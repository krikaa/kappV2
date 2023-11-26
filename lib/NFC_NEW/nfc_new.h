#pragma once
#include <stdint.h>
#include <Arduino.h>

void NfcBeginNew(String *);
void NfcTaskNew(String *);
void NfcConnCheck(String *);
void handleCardDetected(String *);
boolean ReadNFC(String *);
boolean ReadNewCard(String *);

extern boolean nfc_connected_new;