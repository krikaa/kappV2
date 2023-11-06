#include "nfc.h"
// #include <Wire.h>
// #include <SPI.h>
// #include <PN532_SPI.h>
// #include <PN532_I2C.h>
// #include <PN532_SWHSU.h>
// #include <SoftwareSerial.h>
// #include <PN532.h>
// #include <NfcAdapter.h>

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

#define LONG_SCAN_THRESHOLD 5  // Number of consecutive scans for a long scan

// PN532_SPI interface(SPI, 15);
// PN532_I2C interface(Wire);
// SoftwareSerial SWSerial( 12, 14 ); // RX, TX
// PN532_SWHSU interface(SWSerial);
// NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object
String tagUUID = "";
uint32_t nfc_last_conn = 0;
boolean restart_required = true;
boolean verbose = false;



void nfcBegin(){
	if(nfc.begin()){
		Serial.print("NFC Begin successful");
	}
	else{
		Serial.print("NFC Begin failed");
	}
}

String readNFC(uint32_t timeout)
{	

	// Serial.println("Present");
	NfcTag tag = nfc.read();
	// tag.print();
	tagUUID = tag.getUidString();
	Serial.println(tagUUID);
	return String();
}

String lastTagID = "";  // Variable to store the last scanned tag
int consecutiveScans = 0;  // Variable to track consecutive scans

boolean readNFC2(scannedTag_t* tag)
{	
	tagUUID = "";
	while (nfc.tagPresent(10)) // Read until present or 20 same scans detected
	{
		digitalWrite(0, HIGH); // Green led while scanning
		NfcTag nfcCard = nfc.read();
		tagUUID = nfcCard.getUidString();
		Serial.println(tagUUID);

		// if (tagUUID == lastTagID && tagUUID != "") {
		// 	consecutiveScans++;
		// } 
		// lastTagID = tagUUID;

		// Serial.println(consecutiveScans);

		// if(consecutiveScans >= 5) {
		// 	Serial.println("Long scan");
		// 	tag->scanType = LONG_SCAN;
		// 	tag->tagID = tagUUID;
		// 	return true;
		// }
	}
	digitalWrite(0, LOW);
	tag->tagID = tagUUID;
	
	if(tagUUID == "") {
		tag->scanType = NO_SCAN;
		return false;
	}

	// Serial.println("Short scan");
	tag->scanType = SHORT_SCAN;
	
	
	return true;

}

boolean nfcTask(scannedTag_t* tag){
	if((millis() - nfc_last_conn) > 0 || restart_required) {
		if (nfc.connected(0)) {
			readNFC2(tag); // Pass pointer, save UUID into tag.tagID 
			nfc_last_conn = millis();
			restart_required = false;
		}
		else{
			restart_required = true;
			nfcBegin();
		}
	}
	consecutiveScans = 0;
	return !tag->tagID.isEmpty();
}

boolean nfcTask2(scannedTag_t* tag){
	if((millis() - nfc_last_conn) > NFC_TIMEOUT_MS || restart_required) {
		if (nfc.connected(0)) {
			tag->tagID = readNFC(NFC_TIMEOUT_MS);
			nfc_last_conn = millis();
			restart_required = false;
			// Serial.println(*tagID);

			if (tag->tagID == lastTagID && tag->tagID != "") {
				consecutiveScans++;
			} else {
				consecutiveScans = 1;
			}

			lastTagID = tag->tagID;

			if (consecutiveScans >= LONG_SCAN_THRESHOLD) {
				// Handle a long scan here
				Serial.println("Long scan detected: " + tag->tagID);
				tag->scanType = LONG_SCAN;
				// You can put your long scan handling logic here
			}
		}
		else{
			restart_required = true;
			nfcBegin();
			// Serial.println(millis());
		}
		Serial.println(tag->tagID);
	}
	return !tag->tagID.isEmpty();
}