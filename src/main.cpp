#include <Arduino.h>
#include "user_eeprom.h"
#include "nfc.h"
#include "FBServer.h"
#include "locker.h"

#define DEBUG
#include "SerialDebug.h"

#define EEPROM_SIZE				2048

String UUID = "";
CMD_TYPE_E solenoid_cmd_nfc = CMD_DONT_OPEN;
CMD_TYPE_E solenoid_cmd_web = CMD_DONT_OPEN;

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
	initLockerDevs();

	NfcBegin(&UUID);
	Beep(150);

	ConnectWifi("");
	ConnectFirebase();

	BeginEEPROM(EEPROM_SIZE);

	SaveUserInfoEEPROM();
	initInterruptMagnet();
}

void loop()
{
	SaveUserInfoEEPROM();

	// Only use NFC specific tasks when solenoid and door are closed
	if (solenoid_state == CLOSED && door_state == CLOSED) {
		NfcTask(&UUID);
		IndicateScan(UUID);

		solenoid_cmd_nfc = FireBaseTask(&UUID);

		if(solenoid_cmd_nfc == CMD_OPEN || solenoid_cmd_web == CMD_KEEP_OPEN) {
			if(SWSerial.availableForWrite() > 0){
				SWSerialWrite(SEND_OPEN_LOCK);
			}
			delay(200);
			DBGL("Opening");
			SolenoidOpen();
		}
		else if(solenoid_cmd_nfc == CMD_DONT_OPEN && !UUID.isEmpty()){
			if(SWSerial.availableForWrite() > 0){
				SWSerialWrite(SEND_NO_ACCESS);
				DBGL("No access");
			}
		}
	}

	solenoid_cmd_web = FireBaseCheckSolenoid();

	FireBaseUpdateDoorState(door_state);

	NfcConnCheck(&UUID);

	SolenoidStateChange((uint8_t)solenoid_cmd_web);

	SendDeviceStatuses();

	CheckDoorState();

	UUID = "";
}
