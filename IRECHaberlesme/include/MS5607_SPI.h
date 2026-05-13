#ifndef MS5607_SPI_H
#define MS5607_SPI_H

#include <Arduino.h>
#include <SPI.h>

class MS5607 {
  private:
    SPIClass* _spi;
    uint8_t _cs;
    uint16_t C[8];

    uint32_t readADC(uint8_t cmd) {
        digitalWrite(_cs, LOW);
        _spi->transfer(cmd);
        digitalWrite(_cs, HIGH);
        delay(10); 

        digitalWrite(_cs, LOW);
        _spi->transfer(0x00); 
        uint32_t val = 0;
        val |= (uint32_t)_spi->transfer(0x00) << 16;
        val |= (uint32_t)_spi->transfer(0x00) << 8;
        val |= (uint32_t)_spi->transfer(0x00);
        digitalWrite(_cs, HIGH);
        return val;
    }

  public:
    MS5607(SPIClass* spiBus, uint8_t csPin) : _spi(spiBus), _cs(csPin) {}

    bool begin() {
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);
        
        digitalWrite(_cs, LOW);
        _spi->transfer(0x1E); 
        digitalWrite(_cs, HIGH);
        delay(20);

        for (uint8_t i = 0; i < 8; i++) {
            digitalWrite(_cs, LOW);
            _spi->transfer(0xA0 + (i * 2));
            C[i] = ((uint16_t)_spi->transfer(0x00) << 8) | _spi->transfer(0x00);
            digitalWrite(_cs, HIGH);
        }
        return (C[1] != 0 && C[1] != 0xFFFF);
    }

    void read(float &P, float &T) {
        uint32_t D1 = readADC(0x48); 
        uint32_t D2 = readADC(0x58); 

        int64_t dT = D2 - (int64_t)C[5] * 256;
        int32_t TEMP = 2000 + (dT * C[6]) / 8388608;
        int64_t OFF = (int64_t)C[2] * 131072 + (int64_t)C[4] * dT / 64;
        int64_t SENS = (int64_t)C[1] * 65536 + (int64_t)C[3] * dT / 128;
        int32_t PRESS = (D1 * SENS / 2097152 - OFF) / 32768;

        P = (float)PRESS / 100.0;
        T = (float)TEMP / 100.0;
    }

    // Basıncı (hPa) verip, irtifayı (Metre) döner
    float getAltitude(float pressure, float seaLevelPressure = 1013.25) {
        return 44330.0 * (1.0 - pow((pressure / seaLevelPressure), 0.1903));
    }
};

#endif