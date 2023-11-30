#pragma once
#include <Arduino.h>
#include <SoftwareSerial.h>


#ifdef DEBUG
#define DBGL(x) Serial.println(x)
#define DBG(x) Serial.print(x)
#else 
#define DBGL(x) ((void)0)
#define DBG(x) ((void)0)
#endif

extern SoftwareSerial SWSerial;
int SWSerialBegin(uint16_t baud);