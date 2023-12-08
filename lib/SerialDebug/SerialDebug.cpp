#include <SerialDebug.h>
#include <SoftwareSerial.h>

#define RX_PIN D5   // Hall
#define TX_PIN D0   // Valge

SoftwareSerial SWSerial(RX_PIN, TX_PIN);

int SWSerialBegin(uint16_t baud) {
    SWSerial.begin(baud);
    delay(100);
    if(!SWSerial.available()){
        return -1;
    }
    return 1;
}