#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "nfc.h"

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

void ConnectWifi();
void ConnectFirebase();
void ConfigTime();
struct tm Get_Epoch_Time();
CMD_TYPE_E FireBaseTask(String *);
CMD_TYPE_E FireBaseCheckDoor();
void DeviceStatusesTask();

struct tm ConvertTimeStamp(unsigned long);
unsigned long long ConvertToTimeStamp(struct tm timeinfo);
struct tm ConvertFromTimeStamp(unsigned long long tstamp);
struct tm GetSemesterEnd();

extern boolean wifi_connected;