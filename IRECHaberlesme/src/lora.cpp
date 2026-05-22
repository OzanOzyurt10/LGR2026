#include "../include/lora.h"

HardwareSerial LoRaSerial(LORA_RX, LORA_TX);
LoRa_E22 e22(&LoRaSerial, LORA_AUX, LORA_M0, LORA_M1);

extern HardwareSerial DebugSerial;

bool loraBaslat() {
    LoRaSerial.begin(9600);
    delay(100);

    if (!e22.begin()) {
        DebugSerial.println("[LORA HATA] Kutuphane baslatilamadi!");
        return false;
    }

    ResponseStructContainer c;
    c = e22.getConfiguration();

    if (c.status.code != 1) {
        DebugSerial.print("[LORA HATA] Modul okunmadi! Hata Kodu: ");
        DebugSerial.println(c.status.code);
        return false;
    }

    Configuration config = *(Configuration*)c.data;

    config.ADDL = LORA_ADRES_L;
    config.ADDH = LORA_ADRES_H;
    config.CHAN  = LORA_KANAL;

    config.SPED.uartBaudRate = UART_BPS_9600;
    config.SPED.airDataRate  = LORA_HAVA_HIZI;

    config.OPTION.transmissionPower = LORA_GUC;
    config.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;

    ResponseStatus rs = e22.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    c.close();

    if (rs.code != 1) {
        DebugSerial.println("[LORA HATA] Ayarlar modula yazilamadi!");
        return false;
    }

    DebugSerial.println("[LORA OK] Ayarlar basariyla yazildi ve modul hazir!");
    return true;
}

void loraGonder(float   alt,
                float   p,
                float   heading,
                float   ax,  float ay,  float az,
                float   gx,  float gy,  float gz,
                double  lat, double lon, double gps_alt,
                uint8_t status) {

    LoRaPacket pkt;
    pkt.header1  = 0xAA;
    pkt.header2  = 0xBB;
    pkt.alt      = alt;
    pkt.p        = p;
    pkt.heading  = heading;
    pkt.ax       = ax;  pkt.ay = ay;  pkt.az = az;
    pkt.gx       = gx;  pkt.gy = gy;  pkt.gz = gz;
    pkt.lat      = lat;
    pkt.lon      = lon;
    pkt.gps_alt  = gps_alt;
    pkt.status   = status;

    LoRaSerial.write((uint8_t*)&pkt, sizeof(LoRaPacket));
    delay(5);
    while (digitalRead(LORA_AUX) == LOW) {}
}