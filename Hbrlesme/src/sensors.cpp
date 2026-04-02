#include "header.h"

// ===================== DIŞ DONANIM NESNELERİ =====================
// main.cpp'de tanımlı, burada sadece kullanıyoruz
extern SoftwareSerial MySerial;
extern HardwareSerial GpsSerial;
extern Adafruit_BME280  bme;
extern Adafruit_BNO055  bno;
extern TinyGPSPlus       gps;

// ===================== GPS DELAY =====================
void smartDelay(unsigned long ms) {
    unsigned long start = millis();
    do {
        while (GpsSerial.available()) {
            gps.encode(GpsSerial.read());
        }
    } while (millis() - start < ms);
}

// ===================== VERİ OKUMA =====================
SensorData readSensors() {
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

    // GPS
    data.gpsValid   = gps.location.isValid();
    data.lat        = data.gpsValid ? gps.location.lat() : 0.0;
    data.lng        = data.gpsValid ? gps.location.lng() : 0.0;
    data.satellites = data.gpsValid ? gps.satellites.value() : 0;

    return data;
}

// ===================== VERİ YAZDIRMA =====================
void printData(const SensorData& d) {
    // BME280
    MySerial.print("Alt:");   MySerial.print(d.altitude);    MySerial.print("m | ");
    MySerial.print("Press:"); MySerial.print(d.pressure);    MySerial.print("hPa | ");
    MySerial.print("Temp:");  MySerial.print(d.temperature); MySerial.print("C | ");

    // BNO055 - Yön
    MySerial.print("Yon(X):");    MySerial.print(d.yaw);   MySerial.print(" | ");
    MySerial.print("Egilim(Y):"); MySerial.print(d.pitch); MySerial.print(" | ");
    MySerial.print("Roll(Z):");   MySerial.print(d.roll);  MySerial.print(" | ");

    // BNO055 - Doğrusal ivme
    MySerial.print("LinAcc X:"); MySerial.print(d.linAccX);
    MySerial.print(" Y:");       MySerial.print(d.linAccY);
    MySerial.print(" Z:");       MySerial.print(d.linAccZ); MySerial.print(" | ");

    // GPS
    MySerial.print("GPS: ");
    if (d.gpsValid) {
        MySerial.print(d.lat, 6);
        MySerial.print(", ");
        MySerial.print(d.lng, 6);
        MySerial.print(" Uydular:");
        MySerial.println(d.satellites);
    } else {
        MySerial.println("Uydu Araniyor...");
    }
}