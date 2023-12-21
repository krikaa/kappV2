#pragma once
#include <stdint.h>
#include <Arduino.h>

#define HR_TO_MS(x) 					(x * 3600000)
#define MIN_TO_MS(x) 					(x * 60000)
#define SEC_TO_MS(x) 					(x * 1000)

void ConfigTime();
unsigned long long ConvertToTimeStamp(struct tm);
struct tm GetEpochTime();
struct tm ConvertTimeStamp(unsigned long);
struct tm ConvertFromTimeStamp(unsigned long long);
struct tm GetSemesterEnd();
void PrintTime(struct tm);

