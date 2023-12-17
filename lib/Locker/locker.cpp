#include "locker.h"
#define DEBUG
#include "SerialDebug.h"

#define INDICATE_SCAN_LED 		0
#define SOLENOID_PIN     		2
#define MAGNET_SENSOR_PIN 		13
#define BUZZER_PIN 				15

#define SOLENOID_OPEN_MS 		5000
#define INDICATION_DURATION_MS 	1000

#define DEBOUNCE_DELAY_MS       100

uint32_t solenoid_last_poll = 0;
boolean solenoid_state = CLOSED;

volatile boolean door_state = CLOSED;
volatile unsigned long lastDebounceTime = 0;

inline static void initLockerMagnet() {
    pinMode(MAGNET_SENSOR_PIN, INPUT_PULLUP);   
}
inline static void initLockerSolenoid() {
    pinMode(SOLENOID_PIN, OUTPUT);   
}
inline static void initLockerBuzzer() {
    pinMode(BUZZER_PIN, OUTPUT);   
}
inline static void initLockerLED() {
    pinMode(INDICATE_SCAN_LED, OUTPUT);   
}

void IRAM_ATTR ISRoutineMagnet()
{
	unsigned long currentTime = millis();
	if (currentTime - lastDebounceTime > DEBOUNCE_DELAY_MS)
	{
		if (digitalRead(MAGNET_SENSOR_PIN) == HIGH)
		{
            SWSerialWrite(SEND_OPEN_DOOR);
			door_state = OPEN;
		}
		else
		{
            SWSerialWrite(SEND_CLOSE_DOOR);
			door_state = CLOSED;
		}
		lastDebounceTime = currentTime;
	}
}

void Beep(unsigned long duration)
{
	tone(BUZZER_PIN, 2000, duration);
}

void initLockerDevs() {
    initLockerMagnet();
    initLockerSolenoid();
    initLockerBuzzer();
    initLockerBuzzer();
}

void initInterruptMagnet(){
    attachInterrupt(MAGNET_SENSOR_PIN, ISRoutineMagnet, CHANGE);
}

// If door state is faulty, fixes it
// May occur when magnet sensor ISR does not work correctly
// This might happen when door is partially closed, opened and in between 2 states
void CheckDoorState()
{
	static uint32_t last_door_check = 0;

	if ((millis() - last_door_check) > 2000)
	{
		if (digitalRead(MAGNET_SENSOR_PIN) == HIGH && door_state == CLOSED)
		{
			door_state = OPEN;
		}
		else if (digitalRead(MAGNET_SENSOR_PIN) == LOW && door_state == OPEN)
		{
			door_state = CLOSED;
		}
		solenoid_state == CLOSED ? DBGL("Closed") : DBGL("Open");
		door_state == CLOSED ? DBGL("Closed") : DBGL("Open");
		last_door_check = millis();
	}
}

// Checks if solenoid has been open for X amount of time
// Closes if open time is reached
// Leaves open if not reached
void SolenoidStateChange(uint8_t web_state)
{
	if (digitalRead(SOLENOID_PIN) == HIGH) // Replace with solenoid later
	{
		// Solenoid is open
		if (!solenoid_state)
		{
			// Solenoid has just been opened
			digitalWrite(INDICATE_SCAN_LED, HIGH);
			solenoid_last_poll = millis();
			solenoid_state = OPEN;
			DBGL("Solenoid open");
		}

		if (solenoid_state && (millis() - solenoid_last_poll >= SOLENOID_OPEN_MS) && web_state != CMD_KEEP_OPEN)
		{
			// Solenoid has been open for more than SOLENOID_OPEN_MS
			digitalWrite(INDICATE_SCAN_LED, LOW); // Indicate solenoid is closed
			digitalWrite(SOLENOID_PIN, LOW); 
			solenoid_state = CLOSED;
			DBGL("Solenoid closed");
		}
	}
	else
	{
		// Solenoid is closed
		solenoid_state = CLOSED;
	}
}

// Indicates that a card has been scanned with LED and buzzer
void IndicateScan(String UUID)
{
	static unsigned long indicationStartTime = 0;

	if (!UUID.isEmpty())
	{
		Beep(150);
		tone(INDICATE_SCAN_LED, 2000, 150);
		indicationStartTime = millis(); 
	}
	else if (millis() - indicationStartTime >= INDICATION_DURATION_MS && indicationStartTime != 0)
	{
		indicationStartTime = 0;
	}
}

void SolenoidOpen(){
    Beep(150);
    solenoid_last_poll = millis();
    digitalWrite(SOLENOID_PIN, HIGH);
}
