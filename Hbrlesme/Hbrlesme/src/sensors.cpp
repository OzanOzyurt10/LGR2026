#include "header.h"

extern SoftwareSerial MySerial;
extern HardwareSerial GpsSerial;
extern Adafruit_BME280  bme;
extern Adafruit_BNO055  bno;
extern TinyGPSPlus       gps;

// ===================== GPS BESLEME =====================
void feedGPS() {
    while (GpsSerial.available()) {
        char c = GpsSerial.read();
        //MySerial.print(c);  // ham NMEA çıktısını gör
        gps.encode(c);
    }
}

// ===================== GPS DELAY =====================
void smartDelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        feedGPS();
    } while (millis() - start < ms);
}

// ===================== VERİ OKUMA =====================
SensorData readSensors() {
    feedGPS();  // Okuma öncesi buffer boşalt

    SensorData data;

    // BME280
    data.altitude    = bme.readAltitude(SEALEVELPRESSURE_HPA);
    data.pressure    = bme.readPressure() / 100.0F;
    data.temperature = bme.readTemperature();

    // BNO055
    imu::Vector<3> euler    = bno.getVector(Adafruit_BNO055::VECTOR_EULER);
    imu::Vector<3> linAccel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);

    data.yaw   = euler.x();
    data.pitch = euler.y();
    data.roll  = euler.z();

    data.linAccX = linAccel.x();
    data.linAccY = linAccel.y();
    data.linAccZ = linAccel.z();

    // GPS — konum ve uydu ayrı kontrol
    data.gpsValid = gps.location.isValid() && gps.location.age() < 2000;

    data.lat = data.gpsValid ? gps.location.lat() : 0.0;
    data.lng = data.gpsValid ? gps.location.lng() : 0.0;

    // Uydu sayısı fix olmasa da okunabilir
    data.satellites = gps.satellites.isValid() ? gps.satellites.value() : 0;

    return data;
}

// ===================== VERİ YAZDIRMA =====================
void printData(const SensorData& d) {
    MySerial.print("Alt:");   MySerial.print(d.altitude);    MySerial.print("m | ");
    MySerial.print("Press:"); MySerial.print(d.pressure);    MySerial.print("hPa | ");
    MySerial.print("Temp:");  MySerial.print(d.temperature); MySerial.print("C | ");

    MySerial.print("Yon:");    MySerial.print(d.yaw);   MySerial.print(" | ");
    MySerial.print("Egilim:"); MySerial.print(d.pitch); MySerial.print(" | ");
    MySerial.print("Roll:");   MySerial.print(d.roll);  MySerial.print(" | ");

    MySerial.print("LinAcc X:"); MySerial.print(d.linAccX);
    MySerial.print(" Y:");       MySerial.print(d.linAccY);
    MySerial.print(" Z:");       MySerial.print(d.linAccZ); MySerial.print(" | ");

    MySerial.print("GPS: ");
    if (d.gpsValid) {
        MySerial.print(d.lat, 6);
        MySerial.print(", ");
        MySerial.print(d.lng, 6);
    } else {
        MySerial.print("Fix yok");
    }
    MySerial.print(" | Uydu: ");
    MySerial.println(d.satellites);
}