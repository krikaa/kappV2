#include <stdint.h>
#include <Arduino.h>
#pragma once

void NfcBegin();
boolean NfcTask(String *);
boolean ReadNFC(String *);
boolean ReadNewCard(String *);

extern boolean nfc_connected;  
