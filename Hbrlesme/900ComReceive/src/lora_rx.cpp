#include "header_rx.h"

SPIClass SPI_LORA(PA7, PA6, PA5);
SX1262 radio = new Module(LORA_NSS, LORA_DIO1, LORA_NRST, LORA_BUSY, SPI_LORA);

extern SoftwareSerial MySerial;

void loraInit_rx() {
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

    MySerial.print("BUSY: ");
    MySerial.println(digitalRead(LORA_BUSY));

    unsigned long t = millis();
    while (digitalRead(LORA_BUSY) == HIGH) {
        if (millis() - t > 3000) {
            MySerial.println("LoRa BUSY TIMEOUT!");
            return;
        }
    }

    radio.setRfSwitchPins(LORA_RXEN, LORA_TXEN);

    int state = radio.begin(LORA_FREQ, 125.0, 9, 7, 0x12, 22, 8);
    radio.setTCXO(1.6);

    if (state == RADIOLIB_ERR_NONE) {
        MySerial.println("LoRa RX OK!");
    } else {
        MySerial.print("LoRa HATA: ");
        MySerial.println(state);
        return;
    }

    // Sürekli dinleme moduna al
    radio.startReceive();
}

void loraReceive() {
    // Paket geldi mi kontrol et
    if (digitalRead(LORA_DIO1) == LOW) return;

    String received;
    int state = radio.readData(received);

    if (state != RADIOLIB_ERR_NONE) {
        MySerial.print("Alim hatasi: ");
        MySerial.println(state);
        radio.startReceive();
        return;
    }

    // Paketi parse et
    SensorData d;
    int gpsValidInt;

    int parsed = sscanf(received.c_str(),
        "%f,%f,%f,%f,%f,%f,%f,%f,%f,%lf,%lf,%lu,%d",
        &d.altitude,  &d.pressure,   &d.temperature,
        &d.yaw,       &d.pitch,      &d.roll,
        &d.linAccX,   &d.linAccY,    &d.linAccZ,
        &d.lat,       &d.lng,
        &d.satellites, &gpsValidInt
    );
    d.gpsValid = gpsValidInt == 1;

    if (parsed != 2) {
        MySerial.print("Parse hatasi, alan sayisi: ");
        MySerial.println(parsed);
        radio.startReceive();
        return;
    }

    // Veriyi yazdır
    MySerial.println("=== PAKET ALINDI ===");
    MySerial.print("RSSI: ");    MySerial.print(radio.getRSSI());    MySerial.println(" dBm");
    MySerial.print("SNR: ");     MySerial.print(radio.getSNR());     MySerial.println(" dB");
    MySerial.print("Alt: ");     MySerial.print(d.altitude);         MySerial.println(" m");
    MySerial.print("Basinc: ");  MySerial.print(d.pressure);         MySerial.println(" hPa");
    MySerial.print("Sicaklik: "); MySerial.print(d.temperature);     MySerial.println(" C");
    MySerial.print("Yon: ");     MySerial.print(d.yaw);
    MySerial.print(" Egilim: "); MySerial.print(d.pitch);
    MySerial.print(" Roll: ");   MySerial.println(d.roll);
    MySerial.print("LinAcc X: "); MySerial.print(d.linAccX);
    MySerial.print(" Y: ");      MySerial.print(d.linAccY);
    MySerial.print(" Z: ");      MySerial.println(d.linAccZ);
    MySerial.print("GPS: ");
    if (d.gpsValid) {
        MySerial.print(d.lat, 6);
        MySerial.print(", ");
        MySerial.println(d.lng, 6);
    } else {
        MySerial.println("Fix yok");
    }
    MySerial.print("Uydu: ");    MySerial.println(d.satellites);
    MySerial.println("====================");

    // Tekrar dinlemeye al
    radio.startReceive();
}