#include "../include/hardware_defs.h"
#include "../include/MS5607_SPI.h"
#include "../include/ICM42688_SPI.h"
#include "../include/lora.h"
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <IST8310.h>

// ========================================================
// SENSÖR SEÇİM PANELİ
#define AKTIF_BARO Baro1
#define AKTIF_IMU  IMU1
#define AKTIF_MAG  Mag1
// ========================================================

// ========================================================
// MAG2 EKSEN KALİBRASYONU
#define MAG2_FLIP_XY  false
#define MAG2_NEG_X    false
#define MAG2_NEG_Y    true
// ========================================================

HardwareSerial DebugSerial(RX_PIN, TX_PIN);
HardwareSerial GPSSerial(GPS_RX, GPS_TX);

SPIClass SPI_Baro1(BARO1_MOSI, BARO1_MISO, BARO1_SCK);
SPIClass SPI_Baro2(BARO2_MOSI, BARO2_MISO, BARO2_SCK);
SPIClass SPI_IMU1(IMU1_MOSI, IMU1_MISO, IMU1_SCK);
SPIClass SPI_IMU2(IMU2_MOSI, IMU2_MISO, IMU2_SCK);

TwoWire I2C_Mag1(MAG1_SDA, MAG1_SCL);
TwoWire I2C_Mag2(MAG2_SDA, MAG2_SCL);

MS5607   Baro1(&SPI_Baro1, BARO1_CS);
MS5607   Baro2(&SPI_Baro2, BARO2_CS);
ICM42688 IMU1(&SPI_IMU1, IMU1_CS);
ICM42688 IMU2(&SPI_IMU2, IMU2_CS);

IST8310 Mag1;
IST8310 Mag2;

TinyGPSPlus gps;

static double   lat = 0, lon = 0, gps_alt = 0;
static float    p, t, alt;
static float    ax, ay, az, gx, gy, gz;
static float    heading  = 0;
static uint8_t  flightStatus = 0;
static uint32_t loraTimer = 0;

void setup() {
    DebugSerial.begin(115200);

    pinMode(BUZZER, OUTPUT);
    for (int i = 0; i < 3; i++) {
        tone(BUZZER, 2700); delay(80);
        noTone(BUZZER);     delay(80);
    }

    // GPS
    GPSSerial.begin(38400);
    delay(200);
    DebugSerial.println("[OK] GPS Hazir.");


    // SPI sensörler
    SPI_Baro1.begin(); SPI_Baro2.begin();
    SPI_IMU1.begin();  SPI_IMU2.begin();
    Baro1.begin(); Baro2.begin();
    IMU1.begin();  IMU2.begin();

    // I2C - Mag1
    I2C_Mag1.setSDA(MAG1_SDA);
    I2C_Mag1.setSCL(MAG1_SCL);
    I2C_Mag1.begin();
    delay(100);

    // I2C - Mag2
    I2C_Mag2.setSDA(MAG2_SDA);
    I2C_Mag2.setSCL(MAG2_SCL);
    I2C_Mag2.begin();
    delay(100);

    // Mag1 kurulum
    Mag1.setup(&I2C_Mag1, &DebugSerial);
    Mag1.set_flip_x_y(true);
    Mag1.set_declination_offset_radians(0.096f);
    DebugSerial.println("[OK] Mag1 Hazir.");

    // Mag2 kurulum
    Mag2.setup(&I2C_Mag2, &DebugSerial);
    Mag2.set_flip_x_y(MAG2_FLIP_XY);
    Mag2.set_negate_x(MAG2_NEG_X);
    Mag2.set_negate_y(MAG2_NEG_Y);
    Mag2.set_declination_offset_radians(0.096f);
    DebugSerial.println("[OK] Mag2 Hazir.");


    // LoRa
    if (loraBaslat()) {
        DebugSerial.println("[OK] LoRa Hazir.");
    } else {
        DebugSerial.println("[HATA] LoRa yanitlamadi!");
    }
}

void loop() {
    // Barometrik okuma
    AKTIF_BARO.read(p, t);
    alt = AKTIF_BARO.getAltitude(p);

    // IMU okuma
    AKTIF_IMU.read(ax, ay, az, gx, gy, gz);

    // Manyetometre okuma
    if (AKTIF_MAG.update()) {
        heading = AKTIF_MAG.get_heading_degrees();
    }

    while (GPSSerial.available()) {
    char c = GPSSerial.read();
    gps.encode(c);
    }

    if (gps.location.isValid() && gps.location.isUpdated()) {
        lat     = gps.location.lat();
        lon     = gps.location.lng();
    }
    if (gps.altitude.isValid() ) {
        gps_alt = gps.altitude.meters();
    }

    // LoRa gönder (100ms'de bir)
    if (millis() - loraTimer >= 100) {
        loraTimer = millis();
        loraGonder(alt, p, heading,
                   ax, ay, az,
                   gx, gy, gz,
                   lat, lon, gps_alt,
                   flightStatus);
        DebugSerial.println("LoRa Paketi Gonderildi.");
    }

    // Debug çıktısı
    DebugSerial.print("ALT:");     DebugSerial.print(alt, 1);
    DebugSerial.print(" | P:");    DebugSerial.print(p, 2);    DebugSerial.print("hPa");
    DebugSerial.print(" | HDG:");  DebugSerial.print(heading, 1); DebugSerial.print("deg");
    DebugSerial.print(" | GPS:");  DebugSerial.print(lat, 6);
    DebugSerial.print(",");        DebugSerial.print(lon, 6);
    DebugSerial.print(" GALT:");   DebugSerial.println(gps_alt, 1);
    

    delay(100);
}