#include <stdint.h>
#include <Arduino.h>
#pragma once

void NfcBegin();
boolean NfcTask(String *);
boolean ReadNFC(String *UUID);
boolean ReadNewCard(String *UUID);

extern boolean nfc_connected;  
