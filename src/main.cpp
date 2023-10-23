#include <Arduino.h>
#include "nfc.h"
#include "FBServer.h"
#include "SerialDebug.h"

#define OPEN 			true
#define CLOSED 			false

#define DOOR_OPEN_MS 	5000

String tagID = "";
uint32_t door_last_poll = 0;
boolean door_state = CLOSED;

void Blink(){
	digitalWrite(2, HIGH);
	delay(100);
	digitalWrite(2, LOW);
	delay(100);
	digitalWrite(2, HIGH);
	delay(100);
	digitalWrite(2, LOW);
	delay(100);
	digitalWrite(2, HIGH);
}

void DoorStateChange() {
    if (digitalRead(2) == HIGH) {
        // Door is open
        if (!door_state) {
            // Door has just been opened
			door_last_poll = millis();
            door_state = OPEN;
			Serial.println("Door open");
        }

        if (door_state && (millis() - door_last_poll >= DOOR_OPEN_MS)) {
            // Door has been open for more than 5 seconds
            digitalWrite(2, LOW); // Indicate door is closed
            door_state = CLOSED;
			Serial.println("Door closed");
        }
    } else {
        // Door is closed
        door_state = CLOSED;
    }
}

void setup()
{
	pinMode(2, OUTPUT);
	Serial.begin(115200);
	// ConnectWifi();
	// ConnectFirebase();
	nfcBegin();
	// Blink();
}

void loop()
{

	if(nfcTask(&tagID)) { // Returns scanned tagID, "" if not present
		door_last_poll = millis();
		digitalWrite(2, HIGH); // OPEN
	}
	DoorStateChange();

	// FireBaseTask2(tagID);
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




