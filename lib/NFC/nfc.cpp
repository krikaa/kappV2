#include <Wire.h>
#include <Adafruit_PN532.h>
#include "nfc.h"

#define DEBUG
#include "SerialDebug.h"

#define PN532_IRQ 			(12)
#define PN532_RESET 		(D3) // Not connected by default on the NFC Shield

#define NFC_RESET_TIMEOUT 	600000
#define DELAY_BETWEEN_CARDS 1000

boolean nfc_connected = false;
boolean readerDisabled = false;

long timeLastCardRead = 0;
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

// Saves scanned card ID into *UUID 
static void handleCardDetected(String *UUID)
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

static boolean ReadNFC(String *UUID)
{
	uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
	uint8_t uidLength;	

	if (nfc2.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 300)) // Read until present 10ms timeout
	{
		*UUID = convertToString(uid, uidLength);
		DBGL(*UUID);
	}

	if (*UUID == "")
	{
		return false;
	}

	return true;
}

// Reads a single card
// Retries x amount of times
boolean ReadNewCard(String *UUID)
{
	DBGL("Reading new card");
	for (int i = 0; i < 30; i++) // TODO: Change Hardcoded 50.
	{
		DBGL(i);
		if (nfc_connected)
		{
			if (ReadNFC(UUID))
			{
				tone(15,2000,150);
				break;
			}
		}
		else
		{
			NfcBegin(UUID);
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

// Checks if a card is scanned from IRQ pin
void NfcTask(String *UUID){
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

// Resets NFC every 5 minutes
// PN532 can sometimes disconnect due to faulty I2C HAL
// Ensures NFC is active 
void NfcConnCheck(String *UUID){
    static uint32_t last_status_check = 0;

	if((millis() - last_status_check) > NFC_RESET_TIMEOUT){ // 5 Min reset
        NfcBegin(UUID);

        last_status_check = millis();
    }
}

// Starts NFC and card listening
void NfcBegin(String *UUID){
    nfc2.begin();
	digitalWrite(0, LOW);
    uint32_t versiondata = nfc2.getFirmwareVersion();
	if (!versiondata)
	{
		DBG("Didn't find PN53x board");
        nfc_connected = false;
        return;
	}
    nfc_connected = true;
	DBGL("NFC Connected");

    startListeningToNFC(UUID);
}