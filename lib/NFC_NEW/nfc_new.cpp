#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "nfc_new.h"

#define DEBUG
#include "SerialDebug.h"

#define PN532_IRQ (12)
#define PN532_RESET (D3) // Not connected by default on the NFC Shield

boolean nfc_connected_new = false;

const int DELAY_BETWEEN_CARDS = 1000;
long timeLastCardRead = 0;
boolean readerDisabled = false;
int irqCurr;
int irqPrev;
Adafruit_PN532 nfc2(PN532_IRQ, PN532_RESET);

static String convertToString(uint8_t* uid, uint8_t uidLen){
	String UUID;
	for (int i = 0; i < uidLen; i++) {
		if (i > 0) {
			UUID += " ";
		}
		if (uid[i] < 0xF) {
			UUID += "0";
		}
		UUID += String((unsigned int)uid[i], (unsigned char)HEX);
	}
	UUID.toUpperCase();
	return UUID;
}

void handleCardDetected(String *UUID)
{
	uint8_t success = false;
	uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
	uint8_t uidLength;					   // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

	// read the NFC tag's info
	success = nfc2.readDetectedPassiveTargetID(uid, &uidLength);
	DBGL(success ? "Read successful" : "Read failed (not a card?)");

	if (success)
	{
		DBGL("Found an ISO14443A card");
        *UUID = convertToString(uid, uidLength);
        DBGL(*UUID);
		timeLastCardRead = millis();
	}

	// The reader will be enabled again after DELAY_BETWEEN_CARDS ms will pass.
	readerDisabled = true;
}

boolean ReadNFC(String *UUID)
{
	uint8_t success = false;
	uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
	uint8_t uidLength;	

	if (nfc2.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 300)) // Read until present 10ms timeout
	{
		*UUID = convertToString(uid, uidLength);
		Serial.println(*UUID);
	}

	if (*UUID == "")
	{
		return false;
	}

	return true;
}

boolean ReadNewCard(String *UUID)
{
	Serial.println("Reading new card");
	for (int i = 0; i < 30; i++) // TODO: Change Hardcoded 50.
	{
		Serial.println(i);
		if (nfc_connected_new)
		{
			if (ReadNFC(UUID))
			{
				tone(15,2000,150);
				break;
			}
		}
		else
		{
			NfcBeginNew(UUID);
		}
	}
	return !UUID->isEmpty();
}

static void startListeningToNFC(String *UUID){
    irqPrev = irqCurr = HIGH;

	DBGL("Starting passive read for an ISO14443A Card ...");
	if (!nfc2.startPassiveTargetIDDetection(PN532_MIFARE_ISO14443A))
	{
		DBGL("No card found. Waiting...");
	}
	else
	{
		DBGL("Card already present.");
		handleCardDetected(UUID);
	}
}


void NfcTaskNew(String *UUID){
    if (readerDisabled)
	{
		if (millis() - timeLastCardRead > DELAY_BETWEEN_CARDS)
		{
			readerDisabled = false;
			startListeningToNFC(UUID);
		}
	}
	else
	{
		irqCurr = digitalRead(PN532_IRQ);

		// When the IRQ is pulled low - the reader has got something for us.
		if (irqCurr == LOW && irqPrev == HIGH)
		{
			DBGL("Got NFC IRQ");
			handleCardDetected(UUID);
		}

		irqPrev = irqCurr;
	}
}

void NfcConnCheck(String *UUID){
    static uint32_t last_status_check = 0;

	if((millis() - last_status_check) > 300000){ // 5 Min reset
        NfcBeginNew(UUID);

        last_status_check = millis();
    }
}

void NfcBeginNew(String *UUID){
    nfc2.begin();

    uint32_t versiondata = nfc2.getFirmwareVersion();
	if (!versiondata)
	{
		DBG("Didn't find PN53x board");
        nfc_connected_new = false;
        return;
		// while (1);
			 // halt
	}
    nfc_connected_new = true;
	// Got ok data, print it out!
	DBGL("NFC Connected");

    startListeningToNFC(UUID);
}