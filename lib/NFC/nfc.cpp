#include "nfc.h"
#include <Arduino.h>
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
PN532_I2C interface(Wire);
NfcAdapter nfc = NfcAdapter(interface);

#define NFC_TIMEOUT_MS 1000
#define NFC_RESTART_MS 2000
#define NFC_POWER_PIN 16

// PN532_SPI interface(SPI, 15);
// PN532_I2C interface(Wire);
// SoftwareSerial SWSerial( 12, 14 ); // RX, TX
// PN532_SWHSU interface(SWSerial);
// NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object

String tagUUID = "";
uint32_t nfc_last_conn = 0;
boolean restart_required = true;
boolean verbose = false;

void NfcBegin()
{
	if (nfc.begin())
	{
		Serial.print("NFC Begin successful");
	}
	else
	{
		Serial.print("NFC Begin failed");
	}
}

boolean ReadNFC(String *UUID)
{
	tagUUID = "";
	while (nfc.tagPresent(10)) // Read until present or 20 same scans detected
	{
		digitalWrite(0, HIGH); // Green led while scanning
		NfcTag nfcCard = nfc.read();
		tagUUID = nfcCard.getUidString();
		Serial.println(tagUUID);
	}
	digitalWrite(0, LOW);
	*UUID = tagUUID;

	if (tagUUID == "")
	{
		return false;
	}

	return true;
}

boolean ReadNewCard(String *UUID)
{
	Serial.println("Reading new card");
	for (int i = 0; i < 50; i++)
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
	if ((millis() - nfc_last_conn) > 0 || restart_required)
	{
		if (nfc.connected(0))
		{
			ReadNFC(UUID);
			nfc_last_conn = millis();
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
