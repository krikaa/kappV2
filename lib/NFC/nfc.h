#include <stdint.h>
#include <Arduino.h>
#pragma once

void nfcBegin();
boolean nfcTask(String* tagID);
String readNFC();