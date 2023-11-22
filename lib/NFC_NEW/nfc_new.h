#pragma once
#include <stdint.h>
#include <Arduino.h>

void NfcBeginNew(String *);
void NfcTaskNew(String *);
void NfcStatusCheck(String *);

extern boolean nfc_connected_new;