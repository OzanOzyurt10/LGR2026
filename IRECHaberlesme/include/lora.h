#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E22.h>
#include "../include/hardware_defs.h"

// ========================================================
// LORA KONFIGÜRASYON PANELİ
#define LORA_ADRES_H    0x00
#define LORA_ADRES_L    0x01
#define LORA_KANAL      0x00  
#define LORA_HAVA_HIZI  AIR_DATA_RATE_010_24
#define LORA_GUC        POWER_10 
// ========================================================

struct __attribute__((packed)) LoRaPacket {
    float   alt;
    float   gx, gy, gz;
    double  lat, lon;
    uint8_t fixType;
};

// Fonksiyon prototipleri
bool loraBaslat();
void loraGonder(float alt, float gx, float gy, float gz, double lat, double lon, uint8_t fix);

#endif