#include "../include/hardware_defs.h"
#include "../include/MS5607_SPI.h"
#include "../include/ICM42688_SPI.h"
#include <SparkFun_u-blox_GNSS_Arduino_Library.h> 

// ========================================================
// SENSÖR SEÇİM PANELİ 
#define AKTIF_BARO Baro2
#define AKTIF_IMU  IMU2    
// ========================================================

HardwareSerial DebugSerial(RX_PIN, TX_PIN);
HardwareSerial GPSSerial(GPS_RX, GPS_TX); 

SPIClass SPI_Baro1(BARO1_MOSI, BARO1_MISO, BARO1_SCK);
SPIClass SPI_Baro2(BARO2_MOSI, BARO2_MISO, BARO2_SCK);
SPIClass SPI_IMU1(IMU1_MOSI, IMU1_MISO, IMU1_SCK);
SPIClass SPI_IMU2(IMU2_MOSI, IMU2_MISO, IMU2_SCK);


MS5607 Baro1(&SPI_Baro1, BARO1_CS);
MS5607 Baro2(&SPI_Baro2, BARO2_CS);
ICM42688 IMU1(&SPI_IMU1, IMU1_CS);
ICM42688 IMU2(&SPI_IMU2, IMU2_CS);

SFE_UBLOX_GNSS myGPS;
static double lat = 0, lon = 0;

void setup() {
  DebugSerial.begin(9600);
  GPSSerial.begin(115200); 

  SPI_Baro1.begin(); SPI_Baro2.begin(); SPI_IMU1.begin(); SPI_IMU2.begin();
  
  Baro1.begin(); Baro2.begin();
  IMU1.begin(); IMU2.begin();
  

  
  if (myGPS.begin(GPSSerial)) {
    myGPS.setNavigationFrequency(10); 
    DebugSerial.println("[OK] GPS Hazir.");
  }


  pinMode(BUZZER, OUTPUT);
  for(int i=0; i<3; i++) { tone(BUZZER, 2731); delay(80); noTone(BUZZER); delay(80); }
}

void loop() {
  float p, t, alt, ax, ay, az, gx, gy, gz;
  
  
  AKTIF_BARO.read(p, t);
  alt = AKTIF_BARO.getAltitude(p);
  AKTIF_IMU.read(ax, ay, az, gx, gy, gz);
 

  
  if (myGPS.getPVT()) {
    lat = myGPS.getLatitude() / 10000000.0;
    lon = myGPS.getLongitude() / 10000000.0;
  }

 
  DebugSerial.print("ALT:"); DebugSerial.print(alt);
  DebugSerial.print(" | GPS:"); DebugSerial.print(lat, 6);
  DebugSerial.print(",");       DebugSerial.println(lon, 6);
  DebugSerial.print(" | GX:"); DebugSerial.print(gx);
  DebugSerial.print(" GY:"); DebugSerial.print(gy);
  DebugSerial.print(" GZ:"); DebugSerial.println(gz);

  delay(10); 
}