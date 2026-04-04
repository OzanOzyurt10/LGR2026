#ifndef HEADER_H
#define HEADER_H

// ===================== PIN TANIMLARI =====================
#define LED              PC13
#define SDA_PIN          PB7
#define SCL_PIN          PA15
#define RX               PA1
#define TX               PA0
#define BUZZER_PIN       PB9

#define LORA_NSS   PA4
#define LORA_DIO1  PB1
#define LORA_BUSY  PC4
#define LORA_NRST  PB0
#define LORA_TXEN  PA3
#define LORA_RXEN  PA2


#define LORA_FREQ  868.0

// ===================== SABİTLER =====================
#define SEALEVELPRESSURE_HPA (1013.25)

// ===================== KÜTÜPHANeler =====================
#include <Arduino.h>
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <TinyGPS++.h>
#include <RadioLib.h>

// ===================== VERİ YAPISI =====================
struct SensorData {
    // BME280
    float altitude;
    float pressure;
    float temperature;

    // BNO055
    float yaw, pitch, roll;
    float linAccX, linAccY, linAccZ;

    // GPS
    double lat, lng;
    uint32_t satellites;
    bool gpsValid;
};

// ===================== FONKSİYON BİLDİRİMLERİ =====================
void feedGPS();
void smartDelay(unsigned long ms);
SensorData readSensors();
void printData(const SensorData& d);
void loraInit();
void loraSend(const SensorData& d);

#endif // HEADER_H