#include <Arduino.h>
#include <math.h>

// --- TEENSY 4.1 YER İSTASYONU ---
#define M0  2
#define M1  3
#define AUX 4

#define LoRaPort Serial2

struct __attribute__((packed)) LoRaPacket {
    uint8_t header1;
    uint8_t header2;
    float   alt;
    float   p;
    float   heading;
    float   ax, ay, az;
    float   gx, gy, gz;
    double  lat, lon;
    double  gps_alt;
    uint8_t status;
};

// --- Haversine ile iki GPS noktası arası mesafe (metre) ---
double haversine(double lat1, double lon1, double lat2, double lon2) {
    const double R = 6371000.0;
    double dLat = (lat2 - lat1) * DEG_TO_RAD;
    double dLon = (lon2 - lon1) * DEG_TO_RAD;
    double a = sin(dLat / 2) * sin(dLat / 2)
             + cos(lat1 * DEG_TO_RAD) * cos(lat2 * DEG_TO_RAD)
             * sin(dLon / 2) * sin(dLon / 2);
    return R * 2.0 * atan2(sqrt(a), sqrt(1.0 - a));
}

// Önceki paket verileri (velocity hesabı için)
static double   prevLat     = 0, prevLon = 0;
static uint32_t prevTime    = 0;
static float    velocity    = 0;
static bool     firstPacket = true;

void setup() {
    Serial.begin(115200);
    LoRaPort.begin(9600);

    pinMode(M0, OUTPUT);
    pinMode(M1, OUTPUT);
    pinMode(AUX, INPUT);

    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);

    delay(1500);
    Serial.println("YER ISTASYONU HAZIR");
}

void loop() {
    while (LoRaPort.available() > 0) {

        if (LoRaPort.read() != 0xAA) continue;

        delay(2);
        if (!LoRaPort.available() || LoRaPort.peek() != 0xBB) continue;
        LoRaPort.read();

        const int kalanBoyut = (sizeof(LoRaPacket) - 2) + 1; // +1 RSSI

        uint32_t t = millis();
        while (LoRaPort.available() < kalanBoyut) {
            if (millis() - t > 200) break;
        }
        if (LoRaPort.available() < kalanBoyut) continue;

        uint8_t buffer[kalanBoyut];
        for (int i = 0; i < kalanBoyut; i++) {
            buffer[i] = LoRaPort.read();
        }

        LoRaPacket pkt;
        pkt.header1 = 0xAA;
        pkt.header2 = 0xBB;
        memcpy((uint8_t*)&pkt + 2, buffer, sizeof(LoRaPacket) - 2);

        uint8_t ham_rssi = buffer[kalanBoyut - 1];
        int     rssi_dbm = -(int)ham_rssi;

        // --- Velocity hesabı (m/s, GPS tabanlı Haversine) ---
        uint32_t now = millis();
        if (firstPacket || (pkt.lat == 0.0 && pkt.lon == 0.0)) {
            velocity    = 0;
            firstPacket = false;
        } else {
            double dist = haversine(prevLat, prevLon, pkt.lat, pkt.lon);
            float  dt   = (now - prevTime) / 1000.0f;
            if (dt > 0.05f) velocity = (float)(dist / dt);
        }
        prevLat  = pkt.lat;
        prevLon  = pkt.lon;
        prevTime = now;

        // --- CSV çıktısı: 18 alan, C# TelemetryData sırasıyla ---
        // [0]  Altitude
        // [1]  GPS_lat
        // [2]  GPS_lon
        // [3]  XAngle  → 0 (gönderilmiyor)
        // [4]  YAngle  → 0 (gönderilmiyor)
        // [5]  ZAngle  → heading
        // [6]  X_Acc
        // [7]  Y_Acc
        // [8]  Z_Acc
        // [9]  Pressure
        // [10] FlightStatus
        // [11] Velocity
        // [12] CheckSum → 0
        // [13] RSSI
        // [14] GyroX
        // [15] GyroY
        // [16] GyroZ
        // [17] GpsAlt

        Serial.print(pkt.alt,            2); Serial.print(',');
        Serial.print(pkt.lat,            6); Serial.print(',');
        Serial.print(pkt.lon,            6); Serial.print(',');
        Serial.print(0);                     Serial.print(','); // XAngle
        Serial.print(0);                     Serial.print(','); // YAngle
        Serial.print(pkt.heading,        2); Serial.print(','); // ZAngle = heading
        Serial.print(pkt.ax,             4); Serial.print(',');
        Serial.print(pkt.ay,             4); Serial.print(',');
        Serial.print(pkt.az,             4); Serial.print(',');
        Serial.print(pkt.p,              2); Serial.print(',');
        Serial.print(pkt.status & 0x01);     Serial.print(','); // FlightStatus
        Serial.print(velocity,           2); Serial.print(',');
        Serial.print(0);                     Serial.print(','); // CheckSum
        Serial.print(rssi_dbm);              Serial.print(',');
        Serial.print(pkt.gx,             3); Serial.print(',');
        Serial.print(pkt.gy,             3); Serial.print(',');
        Serial.print(pkt.gz,             3); Serial.print(',');
        Serial.println(pkt.gps_alt,      2);
    }
}