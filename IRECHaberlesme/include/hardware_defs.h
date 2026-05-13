#ifndef HARDWARE_DEFS_H
#define HARDWARE_DEFS_H

#include <Arduino.h>

// --- BAROMETRELER ---
#define BARO1_SCK   PC10
#define BARO1_MISO  PC11
#define BARO1_MOSI  PB2
#define BARO1_CS    PA15

#define BARO2_SCK   PB13
#define BARO2_MISO  PB14
#define BARO2_MOSI  PB15
#define BARO2_CS    PB12

// --- IMU (ICM-42688-P) ---
#define IMU1_SCK    PA5
#define IMU1_MISO   PA6
#define IMU1_MOSI   PA7
#define IMU1_CS     PA4

#define IMU2_SCK    PE12
#define IMU2_MISO   PE13
#define IMU2_MOSI   PE14
#define IMU2_CS     PE11

// --- DİĞER ---
#define BUZZER      PE1
#define RX_PIN      PE7
#define TX_PIN      PE8

#endif