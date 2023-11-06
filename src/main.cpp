#include <Arduino.h>
#include "nfc.h"
#include "FBServer.h"
#include "SerialDebug.h"

#define OPEN 			true
#define CLOSED 			false

#define DOOR_OPEN_MS 	2500
#define DOOR_LED_PIN 	2

scannedTag_t tag = {"", NO_SCAN};

String tagID = "";
uint32_t door_last_poll = 0;
boolean door_state = CLOSED;

void Blink(){
	digitalWrite(DOOR_LED_PIN, HIGH);
	delay(100);
	digitalWrite(DOOR_LED_PIN, LOW);
	delay(100);
	digitalWrite(DOOR_LED_PIN, HIGH);
	delay(100);
	digitalWrite(DOOR_LED_PIN, LOW);
	delay(100);
	digitalWrite(DOOR_LED_PIN, HIGH);
	delay(100);
	digitalWrite(DOOR_LED_PIN, LOW);
}

void DoorStateChange() {
    if (digitalRead(DOOR_LED_PIN) == HIGH) {
        // Door is open
        if (!door_state) {
            // Door has just been opened
			door_last_poll = millis();
            door_state = OPEN;
			// Serial.println("Door open");
        }

        if (door_state && (millis() - door_last_poll >= DOOR_OPEN_MS)) {
            // Door has been open for more than 5 seconds
            digitalWrite(DOOR_LED_PIN, LOW); // Indicate door is closed
            door_state = CLOSED;
			// Serial.println("Door closed");
        }
    } else {
        // Door is closed
        door_state = CLOSED;
    }
}

void setup()
{
	pinMode(DOOR_LED_PIN, OUTPUT);
	pinMode(0, OUTPUT);
	pinMode(16, OUTPUT);
	Serial.begin(115200);
	// ConnectWifi();
	// ConnectFirebase();
	nfcBegin();
	Blink();
}

void loop()
{
	if(nfcTask(&tag)) { // Returns scanned tagID, "" if not present
		door_last_poll = millis();
		digitalWrite(DOOR_LED_PIN, HIGH); // OPEN
	}

	// if(tag.scanType == LONG_SCAN){
	// 	Serial.println("LONG");
	// }
	// else if(tag.scanType == SHORT_SCAN){
	// 	Serial.println("SHORT");
	// }
	DoorStateChange();

	// FireBaseTask(&tag);
	
	tag.scanType = NO_SCAN;
}


/*
TODO: 	1) When NFC reader disconnects add reconnect.
		2) Keep log of statuses of devices.
		Maybe after every 30min wake from sleep and send statuses.
		TBD: Wifi stay connected when sleeping? Power consumption with and without.
		Eg. NFC:
				Connected:		True/false
				Last active:	Timestamp

*/		




