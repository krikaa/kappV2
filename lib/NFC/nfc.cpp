#include "nfc.h"
// #include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define NFC_TIMEOUT_MS 500

// PN532_SPI interface(SPI, 15);
PN532_I2C interface(Wire);
NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object
String tagId = "None";

uint32_t nfc_last_conn = 0; 

void nfcBegin(){
	if(nfc.begin()){
		Serial.print("NFC Begin successful");
	}
	else{
		Serial.print("NFC Begin failed");
	}
}

void readNFC(uint32_t timeout)
{	
	if (nfc.tagPresent(1))
	{
		Serial.println("Present");
		NfcTag tag = nfc.read();
		tag.print();
		tagId = tag.getUidString();
	}
	else{
		Serial.println("Not present");
	}
}

void nfcTask(){
	readNFC(NFC_TIMEOUT_MS);
	// if(nfc.connected()){
	// 	readNFC(NFC_TIMEOUT_MS);
	// }
	nfc_last_conn = millis();
}