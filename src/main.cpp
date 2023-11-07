#include <Arduino.h>
#include "nfc.h"
#include "FBServer.h"
#include "SerialDebug.h"

#define OPEN true
#define CLOSED false

#define DOOR_OPEN_MS 2500
#define DOOR_LED_PIN 2

String UUID;

String tagID = "";
uint32_t door_last_poll = 0;
boolean door_state = CLOSED;

void Blink() // For debugging.
{
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

void DoorStateChange()
{
	if (digitalRead(DOOR_LED_PIN) == HIGH) // Replace with solenoid later
	{
		// Door is open
		if (!door_state)
		{
			// Door has just been opened
			door_last_poll = millis();
			door_state = OPEN;
			// Serial.println("Door open");
		}

		if (door_state && (millis() - door_last_poll >= DOOR_OPEN_MS))
		{
			// Door has been open for more than DOOR_OPEN_MS
			digitalWrite(DOOR_LED_PIN, LOW); // Indicate door is closed
			door_state = CLOSED;
			// Serial.println("Door closed");
		}
	}
	else
	{
		// Door is closed
		door_state = CLOSED;
	}
}

void setup()
{
	pinMode(DOOR_LED_PIN, OUTPUT);
	pinMode(0, OUTPUT); // For debugging: LED high while NFC reader scanning
	Serial.begin(115200);

	// In FBServer.cpp change these for ConnectWifi()
	// #define WIFI_SSID ""
	// #define WIFI_PASSWORD ""
	
	ConnectWifi(); 
	ConnectFirebase();
	ConfigTime();

	NfcBegin(); // Sometimes fails - Manually reconnect VCC wire for NFC reader.
	Blink(); 	// For debugging - show when setup is done
}

void loop()
{
	// Returns true if tag scanned, saves into UUID
	if (NfcTask(&UUID))
	{ 
		door_last_poll = millis();
		digitalWrite(DOOR_LED_PIN, HIGH); // Open door
	}

	DoorStateChange(); 	// Checks if door was opened
						// If opened, keeps open for DOOR_OPEN_MS
						// Else just keeps it closed

	FireBaseTask(&UUID);
}

// TODO:
// RN it opens door when card is scanned
// Should open when scanned card has access 
// SMTH like:
/*
	NfcTask(&UUID);

	if (FireBaseTask(&UUID) == CMD_OPEN){
		door_last_poll = millis();
		digitalWrite(DOOR_LED_PIN, HIGH); // Open door
	}

	DoorStateChange(); 
*/

/*		MAYBE TODO:
 		1) When NFC reader disconnects add reconnect.
		2) Keep log of statuses of devices.
		Maybe after every 30min wake from sleep and send statuses.
		TBD: Wifi stay connected when sleeping? Power consumption with and without.
		Eg. NFC:
				Connected:		True/false
				Last active:	Timestamp
*/
