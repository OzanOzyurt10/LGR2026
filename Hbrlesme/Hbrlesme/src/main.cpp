#include "header.h"

SoftwareSerial MySerial(RX, TX);
HardwareSerial GpsSerial(PA10, PA9);

Adafruit_BME280 bme;
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);
TinyGPSPlus gps;

void setup() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(200);
    digitalWrite(BUZZER_PIN, LOW);

    MySerial.begin(9600);
    GpsSerial.begin(38400); 
    delay(500);

    MySerial.println("=== SISTEM BASLATILIYOR ===");

    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();
    MySerial.println("I2C OK");

    bme.begin(0x76, &Wire);
    MySerial.println("BME280 OK");

    bno.begin();
    MySerial.println("BNO055 OK");

    loraInit();

    MySerial.println("=== HAZIR ===");
}

void loop() {
    SensorData data = readSensors();
    printData(data);
    loraSend(data);
    smartDelay(1000);
}