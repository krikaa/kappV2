#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include "nfc_new.h"
#include "FBServer.h"

// #define DEBUG
#include "SerialDebug.h"

#define OPEN true
#define CLOSED false

#define EEPROM_SIZE 2048

#define SOLENOID_OPEN_MS 2500
#define INDICATION_DURATION_MS 1000

#define INDICATE_SCAN_LED 0
#define SOLENOID_LED_PIN 2
#define MAGNET_SENSOR_PIN 13
#define BUZZER_PIN 15

CMD_TYPE_E solenoid_cmd_nfc = CMD_DONT_OPEN;
CMD_TYPE_E solenoid_cmd_web = CMD_DONT_OPEN;

String UUID = "";

uint32_t solenoid_last_poll = 0;
boolean solenoid_state = CLOSED;

volatile boolean door_state = CLOSED;
volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 100;

void IRAM_ATTR ISRoutine()
{
	unsigned long currentTime = millis();
	if (currentTime - lastDebounceTime > debounceDelay)
	{
		if (digitalRead(MAGNET_SENSOR_PIN) == HIGH)
		{
			SWSerial.write('d');
			SWSerial.flush();
			door_state = OPEN;
		}
		else
		{
			SWSerial.write('c');
			SWSerial.flush();
			door_state = CLOSED;
		}
		lastDebounceTime = currentTime;
	}
}

void SolenoidStateChange()
{
	if (digitalRead(SOLENOID_LED_PIN) == HIGH) // Replace with solenoid later
	{
		// Solenoid is open
		if (!solenoid_state)
		{
			// Solenoid has just been opened
			solenoid_last_poll = millis();
			solenoid_state = OPEN;
			DBGL("Solenoid open");
		}

		if (solenoid_state && (millis() - solenoid_last_poll >= SOLENOID_OPEN_MS) && solenoid_cmd_web != CMD_KEEP_OPEN)
		{
			// Solenoid has been open for more than SOLENOID_OPEN_MS
			digitalWrite(SOLENOID_LED_PIN, LOW); // Indicate solenoid is closed
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

static inline void Beep(unsigned long duration)
{
	tone(BUZZER_PIN, 2000, duration);
}

static void IndicateScan()
{
	static unsigned long indicationStartTime = 0;

	if (!UUID.isEmpty())
	{
		// digitalWrite(INDICATE_SCAN_LED, HIGH);
		Beep(150);
		indicationStartTime = millis(); // Record the start time
	}
	else if (millis() - indicationStartTime >= INDICATION_DURATION_MS && indicationStartTime != 0)
	{
		// digitalWrite(INDICATE_SCAN_LED, LOW); // Turn off the LED after 2 seconds
		indicationStartTime = 0; // Reset the start time
	}
}

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
		// Serial.print("Solenoid: ");
		// solenoid_state == CLOSED ? DBGL("Closed") : DBGL("Open");
		// Serial.print("Door: ");
		// door_state == CLOSED ? DBGL("Closed") : DBGL("Open");
		last_door_check = millis();
	}
}

static void initPins()
{
	pinMode(INDICATE_SCAN_LED, OUTPUT);
	pinMode(SOLENOID_LED_PIN, OUTPUT);		  // Solenoid activated LED
	pinMode(MAGNET_SENSOR_PIN, INPUT_PULLUP); // Magnet sensor activated BUTTON
	pinMode(BUZZER_PIN, OUTPUT);			  // Buzzer
}

// TODO: Move
void setup()
{
	Serial.begin(115200);
	if (!Serial)
	{
		delay(100);
	}
	DBGL("\n\n");
	if (SWSerialBegin((uint16_t)9600))
	{
		DBGL("SW serial started");
	}
	SWSerial.write('o');
	Serial.println("Strated");
	initPins();

	ConnectWifi("");
	ConnectFirebase();

	// ClearEEPROM(); 		// Dont use this often
	EEPROM.begin(EEPROM_SIZE);

	SaveUserInfoEEPROM();
	NfcBeginNew(&UUID); 	// Sometimes fails - Manually reconnect VCC wire for NFC reader.
	attachInterrupt(MAGNET_SENSOR_PIN, ISRoutine, CHANGE);
}

// 	TODO: 	(EEPROM) When in setup():
//			1) 	If wifi connected: Check all users in firebase
//			   	Check EEPROM. Compare the users.
//				If same - Dont write to EEPROM
//				Not same - Update EEPROM
//			2)	If not connected: Cant compare - Do nothing. Hope some users already in EEPROM.

//			When in LOOP:
//			3) 	If wifi connected: Do step 1) after every other day.
//			4) 	If not connected: Use EEPROM users to open door.
byte f = 0;
void loop()
{
	// a - Admin view
	// o - Open door
	// c - Close door / Close window (disp)
	// n - No access
	// s - open door
	if(Serial.available()){
		f = Serial.read();
		SWSerial.write(f);
		Serial.println("Sent: "+String(f));
		SWSerial.flush();
		if(f == 'a'){
			delay(10);
			while(!SWSerial.available()){
				;
			}
			byte inc = SWSerial.read();
			Serial.println("Got: " + String(inc));
		}
	}

	SaveUserInfoEEPROM();

	// Only use NFC specific tasks when solenoid and door are closed
	if (solenoid_state == CLOSED && door_state == CLOSED) {
		NfcTaskNew(&UUID);
		IndicateScan();

		solenoid_cmd_nfc = FireBaseTask(&UUID);

		if(solenoid_cmd_nfc == CMD_OPEN || solenoid_cmd_web == CMD_KEEP_OPEN) {
			if(SWSerial.availableForWrite() > 0){
				SWSerial.write('o');
				SWSerial.flush();
				Serial.println("Opening");
			}
			delay(200);
			Beep(150);
			solenoid_last_poll = millis();
			digitalWrite(SOLENOID_LED_PIN, HIGH); // Open solenoid
		}
		else if(solenoid_cmd_nfc == CMD_DONT_OPEN && !UUID.isEmpty()){
			if(SWSerial.availableForWrite() > 0){
				SWSerial.write('n');
				SWSerial.flush();
				Serial.println("No access");
			}
		}
	}

	solenoid_cmd_web = FireBaseCheckSolenoid();

	FireBaseUpdateDoorState(door_state);

	NfcConnCheck(&UUID);

	SolenoidStateChange();

	SendDeviceStatuses();

	CheckDoorState();

	UUID = "";
}
