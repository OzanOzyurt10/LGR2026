#include "../include/lora.h"

// Donanımsal seri portumuzu tanımlıyoruz
HardwareSerial LoRaSerial(LORA_RX, LORA_TX);

// Kütüphaneye HardwareSerial referansını, AUX, M0 ve M1 pinlerini veriyoruz!
LoRa_E22 e22(&LoRaSerial, LORA_AUX, LORA_M0, LORA_M1);

extern HardwareSerial DebugSerial; // main.cpp'deki Debug ekranına hata basmak için

bool loraBaslat() {
    // 1. Önce UART portunu başlat
    LoRaSerial.begin(9600);
    delay(100);

    // 2. Kütüphaneyi başlat (M0 ve M1 pinlerini otomatik komut moduna alır)
    if (!e22.begin()) {
        DebugSerial.println("[LORA HATA] Kutuphane baslatilamadi!");
        return false;
    }

    // 3. Mevcut ayarları oku (Modül yanıt veriyor mu testi)
    ResponseStructContainer c;
    c = e22.getConfiguration();
    
    // Eğer okuma başarısızsa modül ile TX/RX iletişimi yoktur
    if (c.status.code != 1) { 
        DebugSerial.print("[LORA HATA] Modul okunmadi! Hata Kodu: ");
        DebugSerial.println(c.status.code);
        return false;
    }

    Configuration config = *(Configuration*) c.data;

    // 4. İlgili parametreleri değiştir (Sadece ihtiyacımız olanları eziyoruz)
    config.ADDL = LORA_ADRES_L;
    config.ADDH = LORA_ADRES_H;
    config.CHAN = LORA_KANAL;
    
    config.SPED.uartBaudRate = UART_BPS_9600;
    config.SPED.airDataRate  = LORA_HAVA_HIZI;
    
    config.OPTION.transmissionPower = LORA_GUC; 
    
    config.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;

    // 5. Ayarları kalıcı olarak karta yaz
    ResponseStatus rs = e22.setConfiguration(config, WRITE_CFG_PWR_DWN_SAVE);
    c.close();

    if (rs.code != 1) {
        DebugSerial.println("[LORA HATA] Ayarlar modula yazilamadi!");
        return false;
    }

    DebugSerial.println("[LORA OK] Ayarlar basariyla yazildi ve modul hazir!");
    return true;
}

void loraGonder(float alt, float gx, float gy, float gz, double lat, double lon, uint8_t fix) {
    LoRaPacket pkt;
    pkt.alt = alt;
    pkt.gx = gx;
    pkt.gy = gy;
    pkt.gz = gz;
    pkt.lat = lat;
    pkt.lon = lon;
    pkt.fixType = fix;

    // Veriyi kütüphane üzerinden gönderiyoruz
    // e22 kütüphanesi AUX pininin durumunu otomatik kontrol edip bekler
    ResponseStatus rs = e22.sendMessage(&pkt, sizeof(LoRaPacket));
    
    // Debug ekranında gönderimin durumunu teyit et
    if(rs.code != 1) {
        DebugSerial.println("[TX HATA] Paket havaya basilamadi!");
    }
}