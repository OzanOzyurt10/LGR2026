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

    Wire.setSDA(SDA_PIN);
    Wire.setSCL(SCL_PIN);
    Wire.begin();

    bme.begin(0x76, &Wire);
    bno.begin();

    MySerial.begin(115200);
    GpsSerial.begin(9600);

    delay(1000);
}


void loop() {
    SensorData data = readSensors();
    printData(data);
    loraSend(data);
    smartDelay(1000);
}
