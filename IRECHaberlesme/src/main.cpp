#include "../include/hardware_defs.h"
#include "../include/MS5607_SPI.h"
#include "../include/ICM42688_SPI.h"

// ========================================================
// SENSÖR SEÇİM PANELİ 
#define AKTIF_BARO Baro1  
#define AKTIF_IMU  IMU1   
// ========================================================

HardwareSerial DebugSerial(RX_PIN, TX_PIN);

// Donanımsal SPI Hatları
SPIClass SPI_Baro1(BARO1_MOSI, BARO1_MISO, BARO1_SCK);
SPIClass SPI_Baro2(BARO2_MOSI, BARO2_MISO, BARO2_SCK);
SPIClass SPI_IMU1(IMU1_MOSI, IMU1_MISO, IMU1_SCK);
SPIClass SPI_IMU2(IMU2_MOSI, IMU2_MISO, IMU2_SCK);

// Sensör Objeleri
MS5607 Baro1(&SPI_Baro1, BARO1_CS);
MS5607 Baro2(&SPI_Baro2, BARO2_CS);
ICM42688 IMU1(&SPI_IMU1, IMU1_CS);
ICM42688 IMU2(&SPI_IMU2, IMU2_CS);

void setup() {
  DebugSerial.begin(9600);
  
  SPI_Baro1.begin(); SPI_Baro2.begin(); SPI_IMU1.begin(); SPI_IMU2.begin();
  Baro1.begin(); Baro2.begin(); IMU1.begin(); IMU2.begin();

  pinMode(BUZZER, OUTPUT);
  for(int i=0; i<3; i++) { tone(BUZZER, 2731); delay(80); noTone(BUZZER); delay(80); }
}

void loop() {
  float p, t, alt;
  float ax, ay, az;

  
  AKTIF_BARO.read(p, t);
  alt = AKTIF_BARO.getAltitude(p);
  AKTIF_IMU.read(ax, ay, az);

  
  DebugSerial.print("ALT: "); DebugSerial.println(alt);
  DebugSerial.print(ax); DebugSerial.print(", "); DebugSerial.print(ay); DebugSerial.print(", "); DebugSerial.println(az);
  

  delay(100); 
}