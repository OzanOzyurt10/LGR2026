#ifndef MS5607_SPI_H
#define MS5607_SPI_H

#include <Arduino.h>
#include <SPI.h>

// MS5607 SPI ayarları: Mode 0, MSB first, max 20MHz
// STM32 kararlılığı için 10MHz kullanıyoruz
static const SPISettings MS5607_SPI_SETTINGS(10000000, MSBFIRST, SPI_MODE0);

class MS5607 {
  private:
    SPIClass* _spi;
    uint8_t   _cs;
    uint16_t  C[8]; // C[0]=factory, C[1..6]=kalibrasyon, C[7]=CRC

    // -------------------------------------------------------
    // Dönüşüm komutu gönder, bekle, ADC'yi oku (24-bit)
    // -------------------------------------------------------
    uint32_t readADC(uint8_t cmd) {
        // 1) Dönüşüm başlat
        _spi->beginTransaction(MS5607_SPI_SETTINGS);
        digitalWrite(_cs, LOW);
        _spi->transfer(cmd);
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();

        // OSR=4096 için max dönüşüm süresi 9.04ms → 12ms güvenli
        delay(12);

        // 2) ADC oku (0x00 = ADC Read komutu)
        _spi->beginTransaction(MS5607_SPI_SETTINGS);
        digitalWrite(_cs, LOW);
        _spi->transfer(0x00);
        uint32_t val = 0;
        val |= (uint32_t)_spi->transfer(0x00) << 16;
        val |= (uint32_t)_spi->transfer(0x00) << 8;
        val |= (uint32_t)_spi->transfer(0x00);
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();

        return val;
    }

  public:
    MS5607(SPIClass* spiBus, uint8_t csPin) : _spi(spiBus), _cs(csPin) {}

    // -------------------------------------------------------
    // Sensörü sıfırla, PROM kalibrasyon katsayılarını oku
    // false → sensör bulunamadı / iletişim hatası
    // -------------------------------------------------------
    bool begin() {
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);

        // Reset
        _spi->beginTransaction(MS5607_SPI_SETTINGS);
        digitalWrite(_cs, LOW);
        _spi->transfer(0x1E); // Reset komutu
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();
        delay(20); // Datasheet: reset süresi ~2.8ms, 20ms çok güvenli

        // PROM oku: 0xA0, 0xA2 … 0xAE (8 kelime × 2 byte)
        for (uint8_t i = 0; i < 8; i++) {
            _spi->beginTransaction(MS5607_SPI_SETTINGS);
            digitalWrite(_cs, LOW);
            _spi->transfer(0xA0 + (i * 2));
            C[i] = ((uint16_t)_spi->transfer(0x00) << 8) | _spi->transfer(0x00);
            digitalWrite(_cs, HIGH);
            _spi->endTransaction();
        }

        // Basit doğrulama: C[1] (SENS_T1) sıfır ya da 0xFFFF olamaz
        return (C[1] != 0 && C[1] != 0xFFFF);
    }

    // -------------------------------------------------------
    // Basınç (hPa) ve sıcaklık (°C) oku
    // MS5607 datasheet AN520 – ikinci derece kompanzasyon dahil
    // -------------------------------------------------------
    void read(float &P, float &T) {
        uint32_t D1 = readADC(0x48); // Convert D1 (basınç), OSR=4096
        uint32_t D2 = readADC(0x58); // Convert D2 (sıcaklık), OSR=4096

        // --- Birinci derece kompanzasyon ---
        int64_t dT   = (int64_t)D2 - (int64_t)C[5] * 256LL;
        int32_t TEMP = 2000 + (int32_t)((dT * (int64_t)C[6]) / 8388608LL);

        int64_t OFF  = (int64_t)C[2] * 131072LL + ((int64_t)C[4] * dT) / 64LL;
        int64_t SENS = (int64_t)C[1] * 65536LL  + ((int64_t)C[3] * dT) / 128LL;

        // --- İkinci derece kompanzasyon (T < 20°C) ---
        // Yüksek irtifada hava sıcaklığı -50°C'ye kadar düşebilir,
        // bu düzeltme olmadan basınç ölçümü ciddi hata verir.
        if (TEMP < 2000) {
            int64_t T2    = (dT * dT) >> 31;                                    // dT² / 2^31
            int64_t OFF2  = 61LL * (int64_t)(TEMP - 2000) * (int64_t)(TEMP - 2000) / 16LL;
            int64_t SENS2 =  2LL * (int64_t)(TEMP - 2000) * (int64_t)(TEMP - 2000);

            // Çok düşük sıcaklık ek düzeltmesi (T < -15°C)
            if (TEMP < -1500) {
                OFF2  += 15LL * (int64_t)(TEMP + 1500) * (int64_t)(TEMP + 1500);
                SENS2 +=  8LL * (int64_t)(TEMP + 1500) * (int64_t)(TEMP + 1500);
            }

            TEMP -= (int32_t)T2;
            OFF  -= OFF2;
            SENS -= SENS2;
        }

        // --- Nihai basınç hesabı ---
        int32_t PRESS = (int32_t)(((int64_t)D1 * SENS / 2097152LL - OFF) / 32768LL);

        T = (float)TEMP  / 100.0f; // °C
        P = (float)PRESS / 100.0f; // hPa
    }

    // -------------------------------------------------------
    // Basınç (hPa) → irtifa (metre) - Uluslararası Barometre Formülü
    // -------------------------------------------------------
    float getAltitude(float pressure, float seaLevelPressure = 1013.25f) {
        return 44330.0f * (1.0f - powf(pressure / seaLevelPressure, 0.1903f));
    }
};

#endif // MS5607_SPI_H