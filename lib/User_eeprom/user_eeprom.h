#pragma once
#include <Arduino.h>
#include "FBServer.h"

void BeginEEPROM(uint16);

boolean GetAllUsersEEPROM();
boolean SaveUserInfoEEPROM();
boolean UsersUpToDateInEEPROM();
void ClearEEPROM();

CMD_TYPE_E GetTagEEPROM(String);

void printUserTagsEEPROM();

extern const char *userTagsEEPROM[MAX_USERS];
extern size_t numUsersEEPROM;
extern boolean ReadEEPROM;