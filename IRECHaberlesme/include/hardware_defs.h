#ifndef HARDWARE_DEFS_H
#define HARDWARE_DEFS_H

#include <Arduino.h>

// --- BAROMETRELER (SPI) ---
#define BARO1_SCK   PC10
#define BARO1_MISO  PC11
#define BARO1_MOSI  PB2
#define BARO1_CS    PA15

#define BARO2_SCK   PB13
#define BARO2_MISO  PB14
#define BARO2_MOSI  PB15
#define BARO2_CS    PB12

// --- IMU (SPI) ---
#define IMU1_SCK    PA5
#define IMU1_MISO   PA6
#define IMU1_MOSI   PA7
#define IMU1_CS     PA4

#define IMU2_SCK    PE12
#define IMU2_MISO   PE13
#define IMU2_MOSI   PE14
#define IMU2_CS     PE11

// --- MAGNETOMETRELER (I2C) ---
#define MAG1_SCL    PB6
#define MAG1_SDA    PB7
#define MAG2_SCL    PB10
#define MAG2_SDA    PB11
#define MAG2_OFFSET 73.7f 

// --- GPS ---
#define GPS_TX      PD5
#define GPS_RX      PD6

// --- LoRa ---
#define LORA_M0     PC5
#define LORA_M1     PB0
#define LORA_AUX    PB1
#define LORA_RX     PD9
#define LORA_TX     PD8



// --- DİĞER ---
#define BUZZER      PE1
#define RX_PIN      PE7
#define TX_PIN      PE8


#endif