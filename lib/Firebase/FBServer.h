#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "nfc.h"

void ConnectWifi();
void ConnectFirebase();
void FireBaseTask(scannedTag_t *);
void FireBaseTask3(scannedTag_t *);