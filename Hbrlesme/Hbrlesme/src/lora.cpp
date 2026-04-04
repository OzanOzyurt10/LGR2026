#include "header.h"
#include <SPI.h>

SPIClass SPI_LORA(PA7, PA6, PA5);
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY, SPI_LORA);

extern SoftwareSerial MySerial;

void loraInit() {
    pinMode(LORA_TXEN, OUTPUT);
    pinMode(LORA_RXEN, OUTPUT);
    pinMode(LORA_NRST, OUTPUT);
    pinMode(LORA_BUSY, INPUT);
    pinMode(LORA_NSS,  OUTPUT);
    digitalWrite(LORA_NSS,  HIGH);
    digitalWrite(LORA_TXEN, LOW);
    digitalWrite(LORA_RXEN, LOW);

    pinMode(PA5, OUTPUT);
    pinMode(PA7, OUTPUT);
    pinMode(PA6, INPUT);

    SPI_LORA.begin();
    digitalWrite(LORA_NSS, HIGH);
    delay(50);

    digitalWrite(LORA_NRST, LOW);
    delay(20);
    digitalWrite(LORA_NRST, HIGH);
    delay(200);

    MySerial.print("BUSY durumu: ");
    MySerial.println(digitalRead(LORA_BUSY));

    unsigned long t = millis();
    while (digitalRead(LORA_BUSY) == HIGH) {
        if (millis() - t > 3000) {
            MySerial.println("LoRa BUSY TIMEOUT!");
            return;
        }
    }

    radio.setRfSwitchPins(LORA_RXEN, LORA_TXEN);

    // 900 MHz, diğer parametreler aynı
    int state = radio.begin(LORA_FREQ, 125.0, 9, 7, 0x12, 22, 8);
    radio.setTCXO(1.6);

    if (state == RADIOLIB_ERR_NONE) {
        MySerial.println("LoRa OK!");
    } else {
        MySerial.print("LoRa HATA: ");
        MySerial.println(state);
    }
}

void loraSend(const SensorData& d) {
    unsigned long t = millis();
    while (digitalRead(LORA_BUSY) == HIGH) {
        if (millis() - t > 3000) return;
        delay(1);
    }

    char packet[128];
    snprintf(packet, sizeof(packet),
        "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.6f,%.6f,%lu,%d",
        d.altitude,  d.pressure,   d.temperature,
        d.yaw,       d.pitch,      d.roll,
        d.linAccX,   d.linAccY,    d.linAccZ,
        d.lat,       d.lng,
        d.satellites, d.gpsValid ? 1 : 0
    );

    int state = radio.transmit(packet);

    if (state == RADIOLIB_ERR_NONE) {
        MySerial.println("Paket gonderildi.");
    } else {
        MySerial.print("Gonderim hatasi: ");
        MySerial.println(state);
    }
}