#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include "FBServer.h"
#include "time.h"
#include "nfc_new.h"
#include <EEPROM.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "TalTech" 	// Change into your own WiFi SSID
#define WIFI_PASSWORD ""	// Change the password

#define API_KEY "API_KEY"
#define DATABASE_URL "nutikapp-default-rtdb.europe-west1.firebasedatabase.app"

#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET 7200 // GMT+2 (2 hours in seconds)
#define DAY_LIGHT_SAVINGS 0	 // 1 Hour Estonia DST (Day Light Savings)

#define ADD_STUDENT '1'
#define OPEN_DOOR '2'

#define MAX_TAG_LEN 21

FirebaseData fbdo;
FirebaseJson fjson;

FirebaseAuth auth;
FirebaseConfig config;

uint32_t sendDataPrev = 0;
uint32_t sendDataPrevWifi = 0;
uint32_t sendDataPrevSolenoid = 0;
uint32_t nextEntryNumber = 1;

boolean wifi_connected;

int count = 0;
bool signupOK = false;

static void PrintTime(struct tm timeinfo) // For debugging
{
	int year = timeinfo.tm_year + 1900;
	int month = timeinfo.tm_mon + 1; // Month is 0-based, so add 1 to get the real month (1-12)
	int day = timeinfo.tm_mday;		 // Day of the month
	int hour = timeinfo.tm_hour;
	int minute = timeinfo.tm_min;
	Serial.print(day);
	Serial.print("/");
	Serial.print(month);
	Serial.print("/");
	Serial.print(year);
	Serial.print(" ");
	Serial.print(hour);
	Serial.print(":");
	Serial.print(minute);
	Serial.println(" ");
}

static void PrintUser(authorized_user_t user) // For debugging
{
	Serial.print("UUID: ");
	Serial.println(user.UUID);
	Serial.print("Added:");
	PrintTime(ConvertFromTimeStamp(user.date_added));
	Serial.print("Owner: ");
	Serial.println(user.owner);
	Serial.print("Type: ");
	Serial.println(user.type);
}

struct tm GetEpochTime() // Querys current time from NTP server
{
	time_t now;
	struct tm timeinfo;
	if (!getLocalTime(&timeinfo))
	{
		Serial.println("Failed to obtain time");
		return timeinfo;
	}
	time(&now);
	return timeinfo;
}

struct tm ConvertFromTimeStamp(unsigned long long tstamp) // Converts timestamp E.g: 169593923 into struct tm
{
	time_t now = static_cast<time_t>(tstamp);
	struct tm timeinfo;

	if (!getLocalTime(&timeinfo))
	{
		Serial.println("Failed to obtain time");
		return timeinfo; // TODO: return some error when fails.
	}
	time(&now);
	// PrintTime(timeinfo);
	return timeinfo;
}

unsigned long long ConvertToTimeStamp(struct tm timeinfo) // Converts struct tm into timestamp 169593923 
{
	time_t timestamp = mktime(&timeinfo);

	if (timestamp != -1)
	{
		return static_cast<unsigned long long>(timestamp) * 1000; // * 1000 for timestamp in ms format
	}
	else
	{
		// Handle the error case, e.g., by returning an error code
		Serial.println("ERROR: Convert to time stamp failed");
		return 0;
	}
}

void ConfigTime()
{
	configTime(TIMEZONE_OFFSET, DAY_LIGHT_SAVINGS, NTP_SERVER);
}

// Sygis semester: ~ 04.09 - 21.01 | kuud: 7, 8, 9, 10, 11, 12, 1
// Kevad semester: ~ 29.01 - 12.6  | kuud: 1, 2, 3, 4, 5, 6

struct tm GetSemesterEnd() // Returns struct tm of semester end
{ 

	struct tm current_time = GetEpochTime();

	// UNCOMMENT TO DEBUG/TEST THIS FUNCTION (REMOVE AFTER RELEASE)

	// int month, day;
	// Serial.println("Enter Month (0-11): ");
	// while (!Serial.available()) {} // Wait for input
	// month = Serial.parseInt();
	// Serial.println(month); // Print the entered month

	// Serial.println("Enter Day (1-31): ");
	// while (!Serial.available()) {} // Wait for input
	// day = Serial.parseInt();
	// Serial.println(day); // Print the entered day

	// current_time.tm_mon = month;
	// current_time.tm_mday = day;

	current_time.tm_hour = 23;
	current_time.tm_min = 59;
	current_time.tm_sec = 0;

	if (current_time.tm_mon > 5 && current_time.tm_mon <= 11)
	{
		// Sygissemester voi suve kuu
		current_time.tm_mday = 21;
		current_time.tm_mon = 0; // Sygisel lopp
		current_time.tm_year += 1;
		// return 0;
	}
	else if (current_time.tm_mon >= 0 && current_time.tm_mon <= 5)
	{
		// Kevadsemester
		if ((current_time.tm_mon == 5 && current_time.tm_mday >= 12) || // Juuni 12 - ...
			(current_time.tm_mon == 0 && current_time.tm_mday < 21))
		{ // Jaanuar 1-20 -> sygissemester

			if (current_time.tm_mon == 5)
			{
				current_time.tm_year += 1;
			}
			current_time.tm_mday = 21;
			current_time.tm_mon = 0; // Sygisel lopp
		}
		else
		{
			current_time.tm_mday = 12;
			current_time.tm_mon = 5; // Kevadel lopp
		}
	}
	PrintTime(current_time);
	Serial.print("TSTAMP: ");
	Serial.println(ConvertToTimeStamp(current_time));
	return current_time;
}

static inline boolean IsFireBaseReady(uint32_t sendDataPrev)
{
	if (Firebase.ready() && signupOK && (millis() - sendDataPrev > 200 || sendDataPrev == 0))
	{
		return true;
	}
	return false;
}

static FirebaseJson SetLogEntryJSON(String tagUUID, String name, String type, String action) // Returns json of log entry
{
	FirebaseJson fbjson;
	fbjson.add("name", name);
	fbjson.add("UUID", tagUUID);
	fbjson.add("type", type);
	fbjson.add("Event", action);

	return fbjson;
}

static FirebaseJson SetStudentJSON()
{
	FirebaseJson fbjson;
	struct tm semester_end = GetSemesterEnd();									  // Gets semester end date
	unsigned long long semester_end_timestamp = ConvertToTimeStamp(semester_end); // Converts into timestamp
	fbjson.add("owner", "TBD");
	fbjson.add("type", "student"); 						// Default value is student.
	fbjson.add("date_valid", semester_end_timestamp);	// From web interface can change added card type

	return fbjson;
}


// Returns all users from FireBase that should have access
const char** GetAllUsersFireBase() {
	if(!wifi_connected) return NULL;

	FirebaseJson fbjson;
	QueryFilter filter;
	FirebaseJsonData result;

	filter.orderBy("$key");
	if(IsFireBaseReady(sendDataPrev)){
		if (Firebase.RTDB.getJSON(&fbdo, "cards/", &filter)) // Looks if cards contains a node that satisfies nodeName == tagUUID
		{
			fbjson = fbdo.to<FirebaseJson>();
			String jsonstr;
			fbjson.toString(jsonstr);
			StaticJsonDocument <512> doc;
			deserializeJson(doc, jsonstr);
			const char** userTags = (const char**)malloc((doc.size() + 1) * sizeof(const char*));
			if (userTags == NULL) {
				return NULL;
			}

			int i = 0;
			unsigned long long current_timestamp = ConvertToTimeStamp(GetEpochTime());

			for (const auto& element : doc.as<JsonObject>()) {
				const char* userTag = element.key().c_str();

                fbjson.get(result, String(userTag) + "/date_valid");

                unsigned long long date_valid_timestamp = (unsigned long long)result.doubleValue;
				if (current_timestamp <= date_valid_timestamp) {
					userTags[i] = strdup(userTag); // Use strdup to duplicate the string
					Serial.println(userTags[i]);
					i++;
				}
			}
			userTags[i] = NULL;
			sendDataPrev = millis();
			return userTags;
		}
	}
	return NULL;
}

void ClearEEPROM(){
	size_t eepromAddr = 0;
	EEPROM.begin(4096);
    while (eepromAddr < EEPROM.length()) {
		EEPROM.write(eepromAddr, 0);
		eepromAddr++;
	}
	Serial.println("EEPROM Cleared");
	EEPROM.end();
}

void FreeUserTags(const char** userTags) {
    if (userTags == NULL) {
        return; // Nothing to free
    }

    for (int i = 0; userTags[i] != NULL; ++i) {
        free((void*)userTags[i]); // Free each duplicated string
    }
    free(userTags); // Free the array itself
}

struct User {
    char tag[MAX_TAG_LEN];
};

// First check if the current EEPROM has similar users already
// This helps reduce the write amounts to flash memory.
boolean SaveUserInfoEEPROM() {
    const char **userTags = GetAllUsersFireBase();
    if (userTags == NULL) {
        Serial.println("SaveUserInfoEEPROM() ERROR");
        return false;
    }

    int eepromAddr = 0;
    for (int i = 0; userTags[i] != NULL; ++i) {
        User user;
        strncpy(user.tag, userTags[i], MAX_TAG_LEN - 1); // Ensure null-termination
        user.tag[MAX_TAG_LEN - 1] = '\0'; // Ensure null-termination
        Serial.print(user.tag);
        Serial.print(" Len: ");
        Serial.print(strlen(user.tag));
        Serial.print(" Size: ");
        Serial.println(sizeof(user.tag));
        EEPROM.put(eepromAddr, user);
        eepromAddr += sizeof(User);
    }

    EEPROM.commit();
    FreeUserTags(userTags);
    return true;
}

void ReadUserInfoEEPROM() {
    // EEPROM.begin(4096); // Make sure to adjust the size based on your needs
    size_t eepromAddr = 0;

    Serial.println("User Tags:");

    while (eepromAddr < EEPROM.length()) {
        User user;
        EEPROM.get(eepromAddr, user);

        // Check if the tag is empty (all zeros), indicating the end of stored data
        if (user.tag[0] == '\0') {
            break;
        }

        Serial.println(user.tag);

        eepromAddr += sizeof(User);
    }

    // EEPROM.end();
}

static boolean GetUserInfoEEPROM(String tagUUID, authorized_user_t *user) {
	Serial.println("Get user info EEPROM");
	return true;
}

static boolean GetUserInfo(String tagUUID, authorized_user_t *user) {
	if(!wifi_connected) {
		return GetUserInfoEEPROM(tagUUID, user);
	}
	FirebaseJson fbjson;
	FirebaseJsonData result;

	// Default values when card doesnt exist
	user->UUID = tagUUID;
	user->owner = "TBD";
	user->type = "unknown";
	user->action = "Attempted open";

	if (Firebase.RTDB.getJSON(&fbdo, "cards/" + tagUUID)) // Looks if cards contains a node that satisfies nodeName == tagUUID
	{
		fbjson = fbdo.to<FirebaseJson>();
		
		unsigned long long current_timestamp = ConvertToTimeStamp(GetEpochTime());

		fbjson.get(result, "owner");
		user->owner = result.stringValue;

		fbjson.get(result, "type");
		user->type = result.stringValue;

		fbjson.get(result, "date_added");
		user->date_added = result.intValue;

		fbjson.get(result, "date_valid");
		unsigned long long date_valid_timestamp = (unsigned long long)result.doubleValue;

		user->action = "Opened";

		user->date_valid = date_valid_timestamp;

		if(current_timestamp > date_valid_timestamp) {
			Serial.println("The user's card has expired");
			user->action = "Attempted open";
			return false;
		}

		return true;
	}
	Serial.println("User not found");
	return false;
}

static CMD_TYPE_E AddToLog(authorized_user_t *user)
{
	FirebaseJson fbjson = SetLogEntryJSON(user->UUID, user->owner, user->type, user->action);

	if (Firebase.RTDB.pushJSON(&fbdo, "log", &fbjson))
	{
		if (Firebase.RTDB.setTimestamp(&fbdo, fbdo.dataPath() + "/" + fbdo.pushName() + "/timestamp"))
		{
			Serial.println("Log added");
			return CMD_OPEN;
		}
	}
	else
	{
		Serial.println("ERROR: Log not added");
	}
	return CMD_DONT_OPEN;
}

static CMD_TYPE_E AddStudentRights(String tagUUID) // Reads new card and saves into cards/
{
	if (!ReadNewCard(&tagUUID)) // Changes value of tagUUID
	{
		Serial.println("Failed to read new card");
		return CMD_DONT_OPEN;
	}
	FirebaseJson fbjson = SetStudentJSON();
	if (Firebase.RTDB.setJSON(&fbdo, "cards/" + tagUUID, &fbjson)) // Adds new scanned card
	{
		if (Firebase.RTDB.setTimestamp(&fbdo, "cards/" + tagUUID + "/date_added"))
		{
			Serial.println("Student added");
			return CMD_OPEN;
		}
	}
	Serial.println("ERROR: Student not added");
	return CMD_DONT_OPEN;
}

// Temporary function until screen is used
static char AskForInput() 
{ 
	Serial.println("1 - Add student");
	Serial.println("2 - Open door");
	long timeout = millis();
	char inByte = '0';
	do
	{
		if (Serial.available())
		{
			inByte = Serial.read();
			if (inByte == ADD_STUDENT || inByte == OPEN_DOOR) {
				return inByte;
			}
		}
	} while (millis() - timeout < 5000); // Waits for 5 seconds from serial monitor input

	return OPEN_DOOR;
}

static CMD_TYPE_E TeacherTask(authorized_user_t *user)
{
	char cmdAnswer = AskForInput(); // Change to screen input later
	switch (cmdAnswer)
	{
	case ADD_STUDENT:
		return AddStudentRights(user->UUID);
		break;

	case OPEN_DOOR:
		return AddToLog(user);
		break;

	default:
		Serial.println("Wrong input");
		break;
	}
	return CMD_DONT_OPEN;
}

static inline CMD_TYPE_E StudentTask(authorized_user_t *user)
{
	return AddToLog(user);
}

CMD_TYPE_E FireBaseTask(String *UUID)
{
	if (*UUID == "")
	{
		return CMD_DONT_OPEN;
	}

	authorized_user_t user;

	if (IsFireBaseReady(sendDataPrev))
	{
		// Check if scanned card is one of the authorized user
		if(!GetUserInfo(*UUID, &user)){
			AddToLog(&user);
			return CMD_DONT_OPEN;
		}
		// PrintUser(user); // For debugging
		 
		if (user.type == "teacher") // Card type: teacher
		{
			return TeacherTask(&user); // Has ability to add or open
		}
		else if (user.type == "student") // Card type: student
		{
			return StudentTask(&user); // Has ability to open
		}
		else
		{
			Serial.println("Scanned UUID does not have access");
			return CMD_DONT_OPEN;
		}
		sendDataPrev = millis();
	}
	return CMD_DONT_OPEN;
}

// Checks every 5 seconds, 
// Opens door if "Ava kapp" pressed on webpage.
CMD_TYPE_E FireBaseCheckDoor() {
	static uint32_t door_last_checked = 0;
	static boolean solenoid_state = false;

	if(millis() - door_last_checked > 5000) {
		if (IsFireBaseReady(sendDataPrevSolenoid)) {
			if(Firebase.RTDB.getBool(&fbdo,"lockers/locker1/solenoid-activated")){
				solenoid_state = fbdo.to<bool>();
				door_last_checked = millis();
			}
			sendDataPrevSolenoid = millis();
		}
	}
	if(solenoid_state){
		return CMD_KEEP_OPEN;
	}
	return CMD_DONT_OPEN;
}

static void SendStatusWiFi(){
	if(IsFireBaseReady(sendDataPrevWifi)){
		if(Firebase.RTDB.setTimestamp(&fbdo, "lockers/locker1/wifi-last-connected")){
			Serial.println("\nWifi status sent");
		}
		sendDataPrevWifi = millis();
	}
}

static void SendStatusNFC(boolean status){
	if(IsFireBaseReady(sendDataPrev)){
		if(Firebase.RTDB.setBool(&fbdo, "lockers/locker1/nfc-connected", status)){
			Serial.print("NFC status sent: ");
			status == false ? Serial.println("NOT CONNECTED") : Serial.println("CONNECTED");
		}
		sendDataPrev = millis();
	}
}

static void WiFiStatusTask(){
	static uint32_t wifi_last_status_sent = 0;

	if ((millis() - wifi_last_status_sent) > 300000) {	 	// Every 5 Minutes
		SendStatusWiFi();								 	// Send status 

		if(!WiFi.isConnected()) {							// Try to reconnect also
			wifi_connected = false;
			ConnectWifi();
			ConnectFirebase();
		}
		else{
			wifi_connected = true;
		}
		wifi_last_status_sent = millis();
	}
}

void NFCStatusTask(){
	static uint32_t nfc_last_status_sent = 0;
	static boolean nfc_last_sent_status = false;


	if (((millis() - nfc_last_status_sent) > 1000) && nfc_last_sent_status != nfc_connected_new){
		SendStatusNFC(nfc_connected_new);
		nfc_last_status_sent = millis();
		nfc_last_sent_status = nfc_connected_new;
	}
}

void DeviceStatusesTask(){
	WiFiStatusTask();
	NFCStatusTask();
}



void ConnectWifi()
{
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.println("");
	Serial.println("Connecting to Wi-Fi");
	while (WiFi.status() != WL_CONNECTED) // Add timeout
	{
		Serial.print(".");
		delay(100);
	}
	wifi_connected = true;
	Serial.println();
	Serial.print("Connected with IP: ");
	Serial.println(WiFi.localIP());
	Serial.println();
}

void ConnectFirebase()
{
	/* Assign the api key (required) */
	config.api_key = API_KEY;

	/* Assign the RTDB URL (required) */
	config.database_url = DATABASE_URL;

	/* Sign up */

	auth.user.email = "FIREBASE_EMAIL";
	auth.user.password = "FIREBASE_PASSWORD";
	// if (Firebase.signUp(&config, &auth, "", ""))
	// {
	// 	Serial.println("Signup OK");
	// 	signupOK = true;
	// }
	// else
	// {
	// 	Serial.printf("%s\n", config.signer.signupError.message.c_str());
	// }

	signupOK = true;

	/* Assign the callback function for the long running token generation task */
	config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);

	// if(Firebase.authenticated()){
	// 	Serial.println("Authenticated");
	// } 
	// else {
	// 	Serial.println("Not authenticated");
	// }

	// if (Firebase.isTokenExpired()){
	// 	Firebase.refreshToken(&config);
	// 	Serial.println("Refresh token");
	// }
}