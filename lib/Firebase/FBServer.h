#pragma once
#include <stdint.h>
#include <Arduino.h>

#define MAX_TAG_LEN 21
#define MAX_USERS 97 // Max size of user tag is 21 bytes - Max EEPROM usage 2037 bytes

typedef enum {
    CMD_OPEN,
    CMD_DONT_OPEN,
    CMD_KEEP_OPEN,
    CMD_NO_OP,
} CMD_TYPE_E;

typedef struct {
    String owner;
    String type;
    String UUID;
    String action;
    unsigned long long date_added;
    unsigned long long date_valid;
} authorized_user_t;

struct User {
    char tag[MAX_TAG_LEN];
};

void ConnectWifi();
void ConnectFirebase();

CMD_TYPE_E FireBaseTask(String *);
CMD_TYPE_E FireBaseCheckSolenoid();

void FireBaseUpdateDoorState(boolean);

boolean GetAllUsersFireBase();
void printUserTagsFirebase();

void SendDeviceStatuses();

extern boolean wifi_connected;
extern const char *userTagsFirebase[MAX_USERS];
extern size_t numUsersFireBase;