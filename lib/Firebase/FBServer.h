#pragma once
#include <stdint.h>
#include <Arduino.h>
#include "nfc.h"

typedef enum {
    CMD_OPEN,
    CMD_DONT_OPEN,
    CMD_SCAN_NEW,
} CMD_TYPE_E;

typedef struct {
    String owner;
    String type;
    String UUID;
    unsigned long long date_added;
    unsigned long long date_valid;
} authorized_user_t;

void ConnectWifi();
void ConnectFirebase();
void ConfigTime();
struct tm Get_Epoch_Time();
CMD_TYPE_E FireBaseTask(String *);

struct tm ConvertTimeStamp(unsigned long);
struct tm GetSemesterEnd();

