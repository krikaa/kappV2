#include "nfc.h"
#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define NFC_TIMEOUT_MS 1000
#define NFC_RESTART_MS 2000

// SPI:
// PN532_SPI interface(SPI, 15);
// NfcAdapter nfc = NfcAdapter(interface); 

// I2C:
PN532_I2C interface(Wire);
NfcAdapter nfc = NfcAdapter(interface);

boolean nfc_connected = true;

String tagUUID = "";
uint32_t nfc_last_conn = 0;
boolean restart_required = true;
boolean verbose = false;

void NfcBegin()
{
	if (nfc.begin(0))
	{
		Serial.println("NFC Begin successful");
		nfc_connected = true;
	}
	else
	{
		Serial.println("NFC Begin failed");
		nfc_connected = false;
	}
}

boolean ReadNFC(String *UUID)
{
	if (nfc.tagPresent(UUID, 500)) // Read until present 10ms timeout
	{
		// NfcTag nfcCard = nfc.read();
		// tagUUID = nfcCard.getUidString();
		Serial.println(*UUID);
	}

	if (*UUID == "")
	{
		return false;
	}

	return true;
}

// When teacher adds student card
boolean ReadNewCard(String *UUID)
{
	Serial.println("Reading new card");
	for (int i = 0; i < 50; i++) // TODO: Change Hardcoded 50.
	{
		if (nfc.connected(0))
		{
			if (ReadNFC(UUID))
			{
				break;
			}
			restart_required = false;
		}
		else
		{
			restart_required = true;
			NfcBegin();
		}
	}
	return !UUID->isEmpty();
}

boolean NfcTask(String *UUID)
{
	if ((millis() - nfc_last_conn) > 1000 || !nfc_connected)
	{
		if (nfc.connected(0)) // (0) - No debug messages, (1 or empty) - Debug messages
		{
			ReadNFC(UUID);
			nfc_last_conn = millis();
		}
		else // Tries to restart
		{
			NfcBegin();
		}
	}
	return !UUID->isEmpty();
}
