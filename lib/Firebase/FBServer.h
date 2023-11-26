#pragma once
#include <stdint.h>
#include <Arduino.h>

typedef enum {
    CMD_OPEN,
    CMD_DONT_OPEN,
    CMD_KEEP_OPEN,
} CMD_TYPE_E;

typedef struct {
    String owner;
    String type;
    String UUID;
    String action;
    unsigned long long date_added;
    unsigned long long date_valid;
} authorized_user_t;

void ConnectWifi(const char*);
void ConnectFirebase();

CMD_TYPE_E FireBaseTask(String *);
CMD_TYPE_E FireBaseCheckSolenoid();

void FireBaseUpdateDoorState(boolean);

boolean GetAllUsersFireBase();
boolean GetAllUsersEEPROM();
boolean SaveUserInfoEEPROM();
void ClearEEPROM();

// void printUserTagsFirebase();
// void printUserTagsEEPROM();

void SendDeviceStatuses();

extern boolean wifi_connected;