#include <Arduino.h>
#include "nfc.h"
#include "FBServer.h"
#include "SerialDebug.h"
#include <Wire.h>

#define OPEN true
#define CLOSED false

#define DOOR_OPEN_MS 2500
#define INDICATION_DURATION_MS 1000

#define INDICATE_SCAN_LED			0
#define SOLENOID_LED_PIN 			2
#define MAGNET_SENSOR_PIN			13
#define BUZZER_PIN					15
#define DOOR_OPEN_PIN				16

CMD_TYPE_E solenoid_cmd_nfc = CMD_DONT_OPEN;
CMD_TYPE_E solenoid_cmd_web = CMD_DONT_OPEN;

String UUID = "";

uint32_t solenoid_last_poll = 0;
boolean solenoid_state = CLOSED;

volatile boolean door_state = CLOSED;
volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 150;


void ICACHE_RAM_ATTR ISRoutine(){
	unsigned long currentTime = millis();
	if (currentTime - lastDebounceTime > debounceDelay)
	{
		door_state = !door_state;
		digitalWrite(DOOR_OPEN_PIN, !digitalRead(DOOR_OPEN_PIN)); // Toggle the LED state

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
			Serial.println("Solenoid open");
		}

		if (solenoid_state && (millis() - solenoid_last_poll >= DOOR_OPEN_MS) && solenoid_cmd_web != CMD_KEEP_OPEN)
		{
			// Solenoid has been open for more than DOOR_OPEN_MS
			digitalWrite(SOLENOID_LED_PIN, LOW); // Indicate solenoid is closed
			solenoid_state = CLOSED;
			Serial.println("Solenoid closed");
		}
	}
	else
	{
		// Solenoid is closed
		solenoid_state = CLOSED;
	}
}

static void MagnetSensorStateChange(){

}

static inline void Beep(unsigned long duration) {
	tone(BUZZER_PIN, 2000, duration);
}

static void IndicateScan() {
	static unsigned long indicationStartTime = 0;

	if(!UUID.isEmpty()){
		digitalWrite(INDICATE_SCAN_LED, HIGH);
		Beep(150);
		indicationStartTime = millis(); // Record the start time
	}
	else if (millis() - indicationStartTime >= INDICATION_DURATION_MS && indicationStartTime != 0) {
        digitalWrite(INDICATE_SCAN_LED, LOW); // Turn off the LED after 2 seconds
        indicationStartTime = 0; // Reset the start time
    }
}

void setup()
{
	pinMode(INDICATE_SCAN_LED, OUTPUT);
	pinMode(SOLENOID_LED_PIN, OUTPUT); // Solenoid activated LED
	pinMode(MAGNET_SENSOR_PIN, INPUT_PULLUP); // Magnet sensor activated BUTTON
	pinMode(BUZZER_PIN, OUTPUT); // Buzzer
	pinMode(DOOR_OPEN_PIN, OUTPUT); // Door closed LED

	// In FBServer.cpp change these for ConnectWifi()
	// #define WIFI_SSID ""
	// #define WIFI_PASSWORD ""

	NfcBegin(); // Sometimes fails - Manually reconnect VCC wire for NFC reader.
	Serial.begin(9600);
	if(!Serial.available()) {
		delay(100);
	}
	ConnectWifi(); 
	ConnectFirebase();
	ConfigTime();
	(nfc_connected == true) ? Serial.println("Connected") : Serial.println("Not connected");
	attachInterrupt(MAGNET_SENSOR_PIN, ISRoutine, FALLING);
}



void loop()
{
	// Only use NFC specific tasks when solenoid is closed.
	if (solenoid_state == CLOSED) { 
		// Returns true if tag scanned, saves into UUID
		NfcTask(&UUID);
		IndicateScan();

		solenoid_cmd_nfc = FireBaseTask(&UUID);

		
		if(solenoid_cmd_nfc == CMD_OPEN || solenoid_cmd_web == CMD_KEEP_OPEN) {
			Beep(150);
			solenoid_last_poll = millis();
			digitalWrite(SOLENOID_LED_PIN, HIGH); // Open solenoid
		}
	}

	solenoid_cmd_web = FireBaseCheckDoor();

	SolenoidStateChange(); 	// Checks if solenoid was opened
	 					// If opened, keeps open for DOOR_OPEN_MS
	 					// Else just keeps it closed

	DeviceStatusesTask();


	UUID = "";
}
