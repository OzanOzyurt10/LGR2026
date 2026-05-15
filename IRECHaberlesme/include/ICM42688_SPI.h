#ifndef ICM42688_SPI_H
#define ICM42688_SPI_H

#include <Arduino.h>
#include <SPI.h>

// ICM-42688-P SPI ayarları: Mode 0, MSB first, max 24MHz
// STM32 kararlılığı için 10MHz kullanıyoruz
static const SPISettings ICM42688_SPI_SETTINGS(10000000, MSBFIRST, SPI_MODE0);

// --- Sık kullanılan register adresleri ---
#define ICM_REG_WHO_AM_I    0x75  // Beklenen değer: 0x47
#define ICM_REG_PWR_MGMT0   0x4E
#define ICM_REG_ACCEL_CFG0  0x50
#define ICM_REG_GYRO_CFG0   0x4F
#define ICM_REG_ACCEL_X_H   0x1F  // İvmeölçer veri başlangıcı (6 byte)
#define ICM_REG_GYRO_X_H    0x25  // Jiroskop veri başlangıcı  (6 byte)
#define ICM_REG_TEMP_H      0x1D  // Sıcaklık

// --- PWR_MGMT0 bitleri ---
#define ICM_ACCEL_LN_MODE   (0x03 << 2) // Accel Low Noise
#define ICM_GYRO_LN_MODE    (0x03 << 0) // Gyro Low Noise

// --- ACCEL_CONFIG0: ±16g, 1kHz ODR ---
// bits[6:5] = 00 → ±16g  |  bits[3:0] = 0110 → 1kHz
#define ICM_ACCEL_16G_1K    0x06

// --- GYRO_CONFIG0: ±2000dps, 1kHz ODR ---
// bits[6:5] = 00 → ±2000dps  |  bits[3:0] = 0110 → 1kHz
#define ICM_GYRO_2000_1K    0x06

// Ölçek katsayıları (datasheet Table 2)
#define ICM_ACCEL_SCALE     (1.0f / 2048.0f)  // ±16g  → g cinsinden
#define ICM_GYRO_SCALE      (1.0f / 16.4f)    // ±2000dps → dps cinsinden
#define ICM_TEMP_OFFSET     25.0f
#define ICM_TEMP_SCALE      (1.0f / 132.48f)

class ICM42688 {
  private:
    SPIClass* _spi;
    uint8_t   _cs;

    // -------------------------------------------------------
    // Tek register yaz
    // -------------------------------------------------------
    void writeReg(uint8_t reg, uint8_t val) {
        _spi->beginTransaction(ICM42688_SPI_SETTINGS);
        digitalWrite(_cs, LOW);
        _spi->transfer(reg & 0x7F); // MSB=0 → yazma
        _spi->transfer(val);
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();
    }

    // -------------------------------------------------------
    // Tek register oku
    // -------------------------------------------------------
    uint8_t readReg(uint8_t reg) {
        _spi->beginTransaction(ICM42688_SPI_SETTINGS);
        digitalWrite(_cs, LOW);
        _spi->transfer(reg | 0x80); // MSB=1 → okuma
        uint8_t val = _spi->transfer(0x00);
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();
        return val;
    }

    // -------------------------------------------------------
    // Ardışık N byte oku (burst read)
    // -------------------------------------------------------
    void readBurst(uint8_t reg, uint8_t* buf, uint8_t len) {
        _spi->beginTransaction(ICM42688_SPI_SETTINGS);
        digitalWrite(_cs, LOW);
        _spi->transfer(reg | 0x80);
        for (uint8_t i = 0; i < len; i++) {
            buf[i] = _spi->transfer(0x00);
        }
        digitalWrite(_cs, HIGH);
        _spi->endTransaction();
    }

  public:
    ICM42688(SPIClass* spiBus, uint8_t csPin) : _spi(spiBus), _cs(csPin) {}

    // -------------------------------------------------------
    // Sensörü başlat ve yapılandır
    // false → WHO_AM_I eşleşmedi (bağlantı/donanım hatası)
    // -------------------------------------------------------
    bool begin() {
        pinMode(_cs, OUTPUT);
        digitalWrite(_cs, HIGH);
        delay(10);

        // Kimlik doğrulama
        if (readReg(ICM_REG_WHO_AM_I) != 0x47) return false;

        // Soft reset (opsiyonel ama güvenli başlangıç sağlar)
        writeReg(0x11, 0x01); // DEVICE_CONFIG: soft reset
        delay(10);

        // Accel + Gyro → Low Noise modu
        writeReg(ICM_REG_PWR_MGMT0, ICM_ACCEL_LN_MODE | ICM_GYRO_LN_MODE);
        delay(1); // Mod geçişi için 200µs yeterli, 1ms çok güvenli

        // ±16g, 1kHz ODR
        writeReg(ICM_REG_ACCEL_CFG0, ICM_ACCEL_16G_1K);

        // ±2000dps, 1kHz ODR
        writeReg(ICM_REG_GYRO_CFG0, ICM_GYRO_2000_1K);

        delay(50); // Sensorun ayarlara geçmesi için bekleme
        return true;
    }

    // -------------------------------------------------------
    // İvmeölçer verisi oku (g cinsinden)
    // -------------------------------------------------------
    void readAccel(float &ax, float &ay, float &az) {
        uint8_t buf[6];
        readBurst(ICM_REG_ACCEL_X_H, buf, 6);

        int16_t rawX = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t rawY = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t rawZ = (int16_t)((buf[4] << 8) | buf[5]);

        ax = rawX * ICM_ACCEL_SCALE;
        ay = rawY * ICM_ACCEL_SCALE;
        az = rawZ * ICM_ACCEL_SCALE;
    }

    // -------------------------------------------------------
    // Jiroskop verisi oku (dps cinsinden)
    // -------------------------------------------------------
    void readGyro(float &gx, float &gy, float &gz) {
        uint8_t buf[6];
        readBurst(ICM_REG_GYRO_X_H, buf, 6);

        int16_t rawX = (int16_t)((buf[0] << 8) | buf[1]);
        int16_t rawY = (int16_t)((buf[2] << 8) | buf[3]);
        int16_t rawZ = (int16_t)((buf[4] << 8) | buf[5]);

        gx = rawX * ICM_GYRO_SCALE;
        gy = rawY * ICM_GYRO_SCALE;
        gz = rawZ * ICM_GYRO_SCALE;
    }

    // -------------------------------------------------------
    // Accel + Gyro tek seferde oku (burst read, daha verimli)
    // -------------------------------------------------------
    void read(float &ax, float &ay, float &az,
              float &gx, float &gy, float &gz) {
        // TEMP(2) + ACCEL(6) + GYRO(6) = 14 byte, 0x1D'den başlıyor
        uint8_t buf[14];
        readBurst(ICM_REG_TEMP_H, buf, 14);

        // buf[0..1] = sıcaklık (şimdilik atla)
        int16_t rawAX = (int16_t)((buf[2]  << 8) | buf[3]);
        int16_t rawAY = (int16_t)((buf[4]  << 8) | buf[5]);
        int16_t rawAZ = (int16_t)((buf[6]  << 8) | buf[7]);
        int16_t rawGX = (int16_t)((buf[8]  << 8) | buf[9]);
        int16_t rawGY = (int16_t)((buf[10] << 8) | buf[11]);
        int16_t rawGZ = (int16_t)((buf[12] << 8) | buf[13]);

        ax = rawAX * ICM_ACCEL_SCALE;
        ay = rawAY * ICM_ACCEL_SCALE;
        az = rawAZ * ICM_ACCEL_SCALE;
        gx = rawGX * ICM_GYRO_SCALE;
        gy = rawGY * ICM_GYRO_SCALE;
        gz = rawGZ * ICM_GYRO_SCALE;
    }

    // -------------------------------------------------------
    // Chip sıcaklığı oku (°C) - sensörün kendi ısısı
    // -------------------------------------------------------
    float readTemperature() {
        uint8_t buf[2];
        readBurst(ICM_REG_TEMP_H, buf, 2);
        int16_t rawT = (int16_t)((buf[0] << 8) | buf[1]);
        return (rawT * ICM_TEMP_SCALE) + ICM_TEMP_OFFSET;
    }
};

#endif // ICM42688_SPI_H