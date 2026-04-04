#include "header_rx.h"

SoftwareSerial MySerial(RX,TX);

void setup() {
    MySerial.begin(9600);
    delay(500);
    MySerial.println("=== ALICI BASLATILIYOR ===");
    loraInit_rx();
    MySerial.println("=== DİNLENİYOR ===");
}

void loop() {
    loraReceive();
}