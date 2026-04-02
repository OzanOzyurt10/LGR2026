#include "header.h"

SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY);

extern SoftwareSerial MySerial;

// ===================== BAŞLATMA =====================
void loraInit() {
    pinMode(LORA_TXEN, OUTPUT);
    pinMode(LORA_RXEN, OUTPUT);
    pinMode(LORA_BUSY, INPUT);
    pinMode(LORA_NRST, OUTPUT);
    
    digitalWrite(LORA_TXEN, LOW);
    digitalWrite(LORA_RXEN, LOW);

    // --- BUSY pin kontrolü ---
    MySerial.print("BUSY pin durumu: ");
    MySerial.println(digitalRead(LORA_BUSY));  // 0 olmalı (hazır)

    // --- Manuel reset ---
    digitalWrite(LORA_NRST, LOW);
    delay(10);
    digitalWrite(LORA_NRST, HIGH);
    delay(20);

    // --- Reset sonrası BUSY bekle ---
    MySerial.print("Reset sonrası BUSY bekleniyor...");
    unsigned long t = millis();
    while (digitalRead(LORA_BUSY) == HIGH) {
        if (millis() - t > 3000) {
            MySerial.println("BUSY TIMEOUT! Modül cevap vermiyor.");
            MySerial.println("→ Güç bağlantısını kontrol edin.");
            MySerial.println("→ BUSY pini doğru mu? (PC4)");
            return;
        }
    }
    MySerial.println("OK, BUSY LOW!");

    // --- SPI kontrol ---
    MySerial.print("SPI başlatılıyor... NSS=PA4, SCK=PB3, MOSI=PB5, MISO=PB4 → ");

    SPI.setMOSI(PB5);
    SPI.setMISO(PB4);
    SPI.setSCLK(PB3);
    SPI.begin();

    int state = radio.begin(
        LORA_FREQ,
        10,       // SF10
        125.0,    // BW
        5,        // CR
        0x12,     // sync word
        22,       // güç
        8         // preamble
    );

    radio.setRfSwitchPins(LORA_RXEN, LORA_TXEN);

    if (state == RADIOLIB_ERR_NONE) {
        MySerial.println("LoRa OK!");
    } else {
        MySerial.print("LoRa HATA kodu: ");
        MySerial.println(state);
        // Hata kodları:
        // -2  = chip not found (SPI sorunu)
        // -705 = SPI timeout
        // -706 = SPI CMD failed
    }
}

// ===================== VERİ GÖNDERME =====================
void loraSend(const SensorData& d) {
    while (digitalRead(LORA_BUSY) == HIGH) {
        delay(1); // modül meşgulse bekle
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

    digitalWrite(LORA_TXEN, HIGH);
    digitalWrite(LORA_RXEN, LOW);

    int state = radio.transmit(packet);

    digitalWrite(LORA_TXEN, LOW);

    if (state == RADIOLIB_ERR_NONE) {
        MySerial.println("Paket gönderildi.");
    } else {
        MySerial.print("Gönderim hatası: ");
        MySerial.println(state);
    }
}
