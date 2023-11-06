#include <stdint.h>
#include <Arduino.h>
#pragma once

typedef enum{
    LONG_SCAN,
    SHORT_SCAN,
    NO_SCAN,
} scanType_e;

typedef struct {
    String tagID;
    scanType_e scanType;
} scannedTag_t;

void nfcBegin();
boolean nfcTask(scannedTag_t*);
String readNFC();

