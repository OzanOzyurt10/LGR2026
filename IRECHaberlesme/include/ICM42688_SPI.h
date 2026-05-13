#ifndef ICM42688_SPI_H
#define ICM42688_SPI_H

#include <Arduino.h>
#include <SPI.h>

class ICM42688 {
  private:
    SPIClass* _spi;
    uint8_t _cs;

    void writeReg(uint8_t reg, uint8_t val) {
        digitalWrite(_cs, LOW);
        _spi->transfer(reg);
        _spi->transfer(val);
        digitalWrite(_cs, HIGH);
    }

    uint8_t readReg(uint8_t reg) {
        digitalWrite(_cs, LOW);
        _spi->transfer(reg | 0x80);
        uint8_t val = _spi->transfer(0x00);
        digitalWrite(_cs, HIGH);
        return val;
    }

  public:
    ICM42688(SPIClass* spiBus, uint8_t csPin) : _spi(spiBus), _cs(csPin) {}

    bool begin() {
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);
        delay(10);
        if (readReg(0x75) != 0x47) return false; // WHO_AM_I kontrolü
        writeReg(0x4E, 0x0F); // Accel ve Gyro'yu uyandır
        delay(10);
        return true;
    }

    void read(float &ax, float &ay, float &az) {
        digitalWrite(_cs, LOW);
        _spi->transfer(0x1F | 0x80); 
        int16_t rawX = (int16_t)((_spi->transfer(0x00) << 8) | _spi->transfer(0x00));
        int16_t rawY = (int16_t)((_spi->transfer(0x00) << 8) | _spi->transfer(0x00));
        int16_t rawZ = (int16_t)((_spi->transfer(0x00) << 8) | _spi->transfer(0x00));
        digitalWrite(_cs, HIGH);
        ax = (float)rawX / 2048.0; // +-16G varsayılan ayarı
        ay = (float)rawY / 2048.0;
        az = (float)rawZ / 2048.0;
    }
};

#endif