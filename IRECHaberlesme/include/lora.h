#ifndef LORA_H
#define LORA_H

#include <Arduino.h>
#include <LoRa_E22.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Wire.h>
#include <IST8310.h>
#include "../include/hardware_defs.h"

// ========================================================
// LORA KONFIGÜRASYON PANELİ
#define LORA_ADRES_H    0x00
#define LORA_ADRES_L    0x01
#define LORA_KANAL      0x00  //Actual Frequency = 850.125 + CH*1M
#define LORA_HAVA_HIZI  AIR_DATA_RATE_100_96
#define LORA_GUC        POWER_33
// ========================================================

/*
 * Paket yapısı (toplam 58 byte):
 *  header1/2  : 0xAA 0xBB  – senkronizasyon baytları
 *  alt        : irtifa (m)      – barometreden
 *  p          : basınç (hPa)    – barometreden
 *  heading    : pusula yönü (°) – 0-360
 *  ax,ay,az   : ivmeölçer (g)
 *  gx,gy,gz   : jiroskop (dps)
 *  lat,lon    : GPS koordinatları (double)
 *  gps_alt    : GPS irtifası (m, double)
 *  status     : bit0 = flight status (0=yerde, 1=uçuşta)
 */
struct __attribute__((packed)) LoRaPacket {
    uint8_t header1;        
    uint8_t header2;        
    float   alt;            
    float   p;              
    float   heading;        
    float   ax, ay, az;     
    float   gx, gy, gz;     
    double  lat, lon;       
    double  gps_alt;        
    uint8_t status;         
};

bool loraBaslat();

void loraGonder(float   alt,
                float   p,
                float   heading,
                float   ax,  float ay,  float az,
                float   gx,  float gy,  float gz,
                double  lat, double lon, double gps_alt,
                uint8_t status);

#endif // LORA_H