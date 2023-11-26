#pragma once
#include <stdint.h>
#include <Arduino.h>

void ConfigTime();
unsigned long long ConvertToTimeStamp(struct tm);
struct tm GetEpochTime();
struct tm ConvertTimeStamp(unsigned long);
struct tm ConvertFromTimeStamp(unsigned long long);
struct tm GetSemesterEnd();
void PrintTime(struct tm);

