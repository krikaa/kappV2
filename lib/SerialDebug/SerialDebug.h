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

#define REQUEST_ADD_STUDENT         'a'
#define REQUEST_OPEN_LOCK           's'
#define REQUEST_CANCEL              'c'

#define SEND_OPEN_LOCK              'o'
#define SEND_NO_ACCESS              'n'
#define SEND_OPEN_DOOR              'd'
#define SEND_CLOSE_DOOR             'c'

#define SEND_ASK_INPUT              'a'
#define SEND_ADDING_UNSUCCESSFUL    'e'
#define SEND_ADDING_SUCCESSFUL      'l'


extern SoftwareSerial SWSerial;
int SWSerialBegin(uint16_t baud);