#include <Arduino.h>

// --- TEENSY 4.1 YER İSTASYONU (SADECE DİNLEME) ---
#define M0  2
#define M1  3
#define AUX 4

// Pin 7 (RX2) -> LoRa TX | Pin 8 (TX2) -> LoRa RX
#define LoRaPort Serial2 

// Verici ile tam olarak aynı paket yapısı (33 Byte)
struct __attribute__((packed)) LoRaPacket {
    float   alt;
    float   gx, gy, gz;
    double  lat, lon;
    uint8_t fixType;
}; 

void setup() {
    Serial.begin(115200);
    LoRaPort.begin(9600); // Modülü ayarlarken Baud 9600 seçmiştik
    
    pinMode(M0, OUTPUT);
    pinMode(M1, OUTPUT);
    pinMode(AUX, INPUT);

    // MODÜLÜ NORMAL ÇALIŞMA MODUNA ALIYORUZ (Dinleme için M0=0, M1=0 ŞART)
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    
    // Seri port ekranının açılmasına fırsat ver
    delay(1500); 
    Serial.println("\n=====================================");
    Serial.println("🚀 YER ISTASYONU HAZIR VE PUSUDA 🚀");
    Serial.println("=====================================");
    Serial.println("Modul ayarlari disaridan yapildi.");
    Serial.println("Kanal: 0 | Adres: 1 | Hiz: 2.4kbps");
    Serial.println("Telemetri verisi bekleniyor...\n");
}

void loop() {
    // UART buffer'ında tam bir paketlik (33 byte) veri birikmiş mi?
    if (LoRaPort.available() >= sizeof(LoRaPacket)) {
        
        uint8_t buffer[sizeof(LoRaPacket)];
        
        // 33 byte'ı sırayla okuyup buffer dizimize diziyoruz
        for (unsigned int i = 0; i < sizeof(LoRaPacket); i++) {
            buffer[i] = LoRaPort.read();
        }

        // Aldığımız ham byte dizisini roketten gelen struct yapımıza dönüştürüyoruz
        LoRaPacket pkt;
        memcpy(&pkt, buffer, sizeof(LoRaPacket));
        
        // Ekrana jilet gibi yazdır
        Serial.println(">>> YENI PAKET ALINDI <<<");
        Serial.print("Irtifa : "); Serial.print(pkt.alt); Serial.println(" metre");
        Serial.print("Gyro   : X: "); Serial.print(pkt.gx); 
        Serial.print(" | Y: "); Serial.print(pkt.gy); 
        Serial.print(" | Z: "); Serial.println(pkt.gz);
        Serial.print("GPS    : Enlem: "); Serial.print(pkt.lat, 6); 
        Serial.print(" | Boylam: "); Serial.println(pkt.lon, 6);
        Serial.print("GPS Fix: "); Serial.println(pkt.fixType);
        Serial.println("-------------------------------------");
    }
}