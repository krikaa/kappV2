#pragma once
#include <Arduino.h>

#define OPEN 					true
#define CLOSED 					false
#define KEEP_OPEN               2

void initLockerDevs();
void initInterruptMagnet();
void CheckDoorState();
void IndicateScan(String);
void SolenoidStateChange(uint8_t);
void SolenoidOpen();
void Beep(unsigned long);

extern boolean solenoid_state;
extern uint32_t solenoid_last_poll;
extern volatile boolean door_state;