#include "nfc.h"
#include <Wire.h>
#include <SPI.h>
#include <PN532_SPI.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define NFC_TIMEOUT_MS 1000
#define NFC_RESTART_MS 2000


// PN532_SPI interface(SPI, 15);
PN532_I2C interface(Wire);
NfcAdapter nfc = NfcAdapter(interface); // create an NFC adapter object
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
	tagUUID = "";
	if (nfc.tagPresent(50)) 
	{
		Serial.println("Present");
		NfcTag tag = nfc.read();
		// tag.print();
		tagUUID = tag.getUidString();
	}
	return tagUUID;
}

boolean nfcTask(String* tagID){
	if((millis() - nfc_last_conn) > NFC_TIMEOUT_MS || restart_required) {
		if (nfc.connected(0)) {
			*tagID = readNFC(NFC_TIMEOUT_MS);
			nfc_last_conn = millis();
			restart_required = false;
			// Serial.println(*tagID);
		}
		else{
			restart_required = true;
			nfcBegin();
			// Serial.println(millis());
		}
	}
	return !tagID->isEmpty();
}