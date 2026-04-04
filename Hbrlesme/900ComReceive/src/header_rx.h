#ifndef HEADER_RX_H
#define HEADER_RX_H

#define LED              PC13
#define RX               PA3
#define TX               PA2

#define LORA_NSS   PA4
#define LORA_DIO1  PB1
#define LORA_BUSY  PB2
#define LORA_NRST  PB0
#define LORA_TXEN  PA10
#define LORA_RXEN  PA9

#define LORA_FREQ  868.0

#include <Arduino.h>
#include <SPI.h>
#include <RadioLib.h>
#include <SoftwareSerial.h>

struct SensorData {
    float altitude, pressure, temperature;
    float yaw, pitch, roll;
    float linAccX, linAccY, linAccZ;
    double lat, lng;
    uint32_t satellites;
    bool gpsValid;
};

void loraInit_rx();
void loraReceive();

#endif