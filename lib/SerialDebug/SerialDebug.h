#pragma once
#include <Arduino.h>

#ifdef DEBUG
#define DBGL(x) Serial.println(x)
#define DBG(x) Serial.print(x)
#else 
#define DBGL(x) 
#define DBG(x)
#endif