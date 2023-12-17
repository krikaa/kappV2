#include <EEPROM.h>
#include "user_eeprom.h"

#define DEBUG
#include "SerialDebug.h"

const char *userTagsEEPROM[MAX_USERS] = {nullptr};
size_t numUsersEEPROM = 0;
boolean ReadEEPROM = true;

void BeginEEPROM(uint16 size) {
    EEPROM.begin(size);
}

void ClearEEPROM(){
	size_t eepromAddr = 0;
	EEPROM.begin(4096);
    while (eepromAddr < EEPROM.length()) {
		EEPROM.write(eepromAddr, 0);
		eepromAddr++;
	}
	DBGL("EEPROM Cleared");
	EEPROM.end();
}

void printUserTagsEEPROM(){
	DBGL("EEPROM:");
	for (size_t i = 0 ; i < numUsersEEPROM;  ++i){
		DBGL(userTagsEEPROM[i]);
	}
}

// Compares current users in EEPROM and FireBase 
// Users have to be the same to be up to date
boolean UsersUpToDateInEEPROM() {
	// DBGL("Num users EEPROM " + String(numUsersEEPROM) + " Num users Firebase " + String(numUsersFireBase));
	if(!wifi_connected) {
		return true;
	}

	if(numUsersEEPROM != numUsersFireBase) {
		DBGL("User count not the same");
		
		return false; // Different amount of users.
	}
	for (size_t i = 0; i < numUsersFireBase; i++) {	
		if(strcmp(userTagsEEPROM[i], userTagsFirebase[i]) != 0) {
			DBGL("Users not the same");
			return false;
		}
	}
	return true;
}

// First check if the current EEPROM has similar users already
// This helps reduce the write amounts to flash memory.
boolean SaveUserInfoEEPROM() {
	static uint32_t last_check = 0;
	
	// TODO: Change time to ~ 12-24 hr 
	if((millis() - last_check > 300000) || last_check == 0) {

		GetAllUsersFireBase();		// Rewrites userTagsFirebase
		GetAllUsersEEPROM(); 		// Rewrites userTagsEEPROM
		printUserTagsEEPROM();
		printUserTagsFirebase();

		if (UsersUpToDateInEEPROM() == true) 
		{
			DBGL("Users up to date");
			last_check = millis();
			return false;
		}
		DBGL("WRITING EEPROM");
		numUsersEEPROM = 0;
		ReadEEPROM = true;
		int eepromAddr = 0;
		for (size_t i = 0; i < numUsersFireBase; ++i) {
			User user;
			strncpy(user.tag, userTagsFirebase[i], MAX_TAG_LEN - 1); // Ensure null-termination
			user.tag[MAX_TAG_LEN - 1] = '\0'; // Ensure null-termination
			userTagsEEPROM[numUsersEEPROM] = strdup(user.tag);
			numUsersEEPROM = i;
			DBGL(user.tag);
			EEPROM.put(eepromAddr, user);
			eepromAddr += sizeof(User);
		}

		EEPROM.write(eepromAddr, '\0'); // Ends array of tags
		EEPROM.commit();
		last_check = millis();
		return true;
	}
	return false;
}

// Saves all users read from EEPROM into userTagsEEPROM
boolean GetAllUsersEEPROM() {
	if(userTagsEEPROM != nullptr && ReadEEPROM == false){
		DBGL("Value already assigned, wont read EEPROM");
		return false;
	}
    size_t eepromAddr = 0;
	numUsersEEPROM = 0;
	DBGL("READING EEPROM");
    for (size_t i = 0; i < MAX_USERS; ++i) {
        User user;
        EEPROM.get(eepromAddr, user);

        // Check if the tag is empty (all zeros), indicating the end of stored data
        if (user.tag[0] == '\0') {
            break;
        }
        userTagsEEPROM[numUsersEEPROM] = strdup(user.tag);
        numUsersEEPROM++;
        eepromAddr += sizeof(User);
    }
	userTagsEEPROM[numUsersEEPROM] = NULL;
	ReadEEPROM = false;
    return true;
}

// Checks if UUID exists in EEPROm
CMD_TYPE_E GetTagEEPROM(String UUID) {
	DBGL("Get tag EEPROM");
	for (size_t i = 0; i < numUsersEEPROM; i++)
	{
		if(strcmp(UUID.c_str(), userTagsEEPROM[i]) == 0){
			DBGL("Found");
			return CMD_OPEN;
		}
	}
	DBGL("Not found");
	return CMD_DONT_OPEN;
}