#pragma once
#include <stdint.h>
#include <Arduino.h>

void NfcBegin(String *);
void NfcTask(String *);
void NfcConnCheck(String *);
// void handleCardDetected(String *);
// boolean ReadNFC(String *);
boolean ReadNewCard(String *);

extern boolean nfc_connected;