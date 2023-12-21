#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include "FBServer.h"
#include "timeutils.h"
#include "nfc.h"
#include "user_eeprom.h"

#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DEBUG
#include "SerialDebug.h"

#define WIFI_SSID 						"TalTech" 	// Change into your own WiFi SSID
#define WIFI_PASSWORD 					""	// Change the password

#define API_KEY 						"API_KEY"
#define DATABASE_URL 					"nutikapp-default-rtdb.europe-west1.firebasedatabase.app"

#define UPDATE_DOOR_TIMEOUT_MS			SEC_TO_MS(1)
#define CHECK_SOLENOID_TIMEOUT_MS		SEC_TO_MS(5)
#define WIFI_TIMEOUT_MS 				SEC_TO_MS(30)			
#define SEND_NFC_STATUS_TIMEOUT_MS		MIN_TO_MS(5)
#define SEND_WIFI_STATUS_TIMEOUT_MS		MIN_TO_MS(5)


FirebaseAuth auth;
FirebaseConfig config;
FirebaseData fbdo;

uint32_t sendDataPrev = 0;
uint32_t sendDataPrevWifi = 0;
uint32_t sendDataPrevSolenoid = 0;
uint32_t sendDataPrevDoor = 0;
uint32_t sendDataPrevNFC = 0;

boolean wifi_connected = false;

const char *userTagsFirebase[MAX_USERS] = {nullptr};
size_t numUsersFireBase = 0;

bool signupOK = false;

static inline boolean IsFireBaseReady(uint32_t sendDataPrev)
{
	if (Firebase.ready() && signupOK && (millis() - sendDataPrev > 200 || sendDataPrev == 0))
	{
		return true;
	}
	return false;
}

static void PrintUser(authorized_user_t user) // For debugging
{
	DBG("UUID: ");
	DBGL(user.UUID);
	DBG("Added:");
	PrintTime(ConvertFromTimeStamp(user.date_added));
	DBG("Owner: ");
	DBGL(user.owner);
	DBG("Type: ");
	DBGL(user.type);
}

// Returns JSON format log entry
static FirebaseJson SetLogEntryJSON(String tagUUID, String name, String type, String action) 
{
	FirebaseJson fbjson;
	fbjson.add("name", name);
	fbjson.add("UUID", tagUUID);
	fbjson.add("type", type);
	fbjson.add("Event", action);

	return fbjson;
}

// Inits and returns JSON format student
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

// Gets user info from Firebase database by tagUUID
// saves into *user
static boolean GetUserInfo(String tagUUID, authorized_user_t *user) {

	// Default values when card doesnt exist
	user->UUID = tagUUID;
	user->owner = "TBD";
	user->type = "unknown";
	user->action = "Attempted open";

	FirebaseJson fbjson;
	FirebaseJsonData result;


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
			DBGL("The user's card has expired");
			user->action = "Attempted open";
			return false;
		}

		return true;
	}
	DBGL("User not found");
	return false;
}

// Adds *user to log
static CMD_TYPE_E AddToLog(authorized_user_t *user)
{
	FirebaseJson fbjson = SetLogEntryJSON(user->UUID, user->owner, user->type, user->action);

	if (Firebase.RTDB.pushJSON(&fbdo, "log", &fbjson))
	{
		if (Firebase.RTDB.setTimestamp(&fbdo, fbdo.dataPath() + "/" + fbdo.pushName() + "/timestamp"))
		{
			DBGL("Log added");
			return CMD_OPEN;
		}
	}
	else
	{
		DBGL("ERROR: Log not added");
	}
	return CMD_DONT_OPEN;
}

// Lets a new card to be scanned
// If card is scanned, adds students rights to Firebase database
static CMD_TYPE_E AddStudentRights(String tagUUID) // Reads new card and saves into cards/
{	
	DBGL("Adding student");
	String studentTag;
	if (!ReadNewCard(&studentTag)) // Changes value of tagUUID
	{
		DBGL("Failed to read new card");
		SWSerialWrite(SEND_ADDING_UNSUCCESSFUL);
		return CMD_NO_OP;
	}
	FirebaseJson fbjson = SetStudentJSON();
	if (Firebase.RTDB.setJSON(&fbdo, "cards/" + studentTag, &fbjson)) // Adds new scanned card
	{
		if (Firebase.RTDB.setTimestamp(&fbdo, "cards/" + studentTag + "/date_added"))
		{
			SWSerialWrite(SEND_ADDING_SUCCESSFUL);
			DBGL("Student added");
			return CMD_NO_OP;
		}
	}
	DBGL("ERROR: Student not added");
	SWSerialWrite(SEND_ADDING_UNSUCCESSFUL);
	return CMD_NO_OP;
}

// Asks for input from other ESP
// Returns what is requested
static char AskForInput() 
{ 
	SWSerialWrite(SEND_ASK_INPUT);
	long timeout = millis();
	char inByte = '0';

	do
	{
		if (SWSerial.available())
		{
			inByte = SWSerial.read();
			DBGL("Got: " + String(inByte));
			if (inByte == REQUEST_ADD_STUDENT || inByte == REQUEST_OPEN_LOCK || inByte == REQUEST_CANCEL) {
				return inByte;
			}
		}
	} while (millis() - timeout < 5000); // Waits for 5 seconds from serial monitor input

	return REQUEST_OPEN_LOCK;
}

// Teacher choose to add new user or open door
static CMD_TYPE_E TeacherTask(authorized_user_t *user)
{
	char cmdAnswer = AskForInput(); // Change to screen input later
	switch (cmdAnswer)
	{
	case REQUEST_ADD_STUDENT:
		return AddStudentRights(user->UUID);

	case REQUEST_OPEN_LOCK:
		return AddToLog(user);

	case REQUEST_CANCEL:
		return CMD_NO_OP;

	default:
		DBGL("Wrong input");
		break;
	}
	return CMD_DONT_OPEN;
}

// Add to log
static inline CMD_TYPE_E StudentTask(authorized_user_t *user)
{
	return AddToLog(user);
}

static void SendWiFiTimestamp(){
	if(IsFireBaseReady(sendDataPrevWifi)){
		if(Firebase.RTDB.setTimestamp(&fbdo, "lockers/locker1/wifi-last-connected")){
			DBGL("\nWifi timestamp sent");
		}
		sendDataPrevWifi = millis();
	}
}

static void SendWiFiStatus(){
	static uint32_t wifi_last_status_sent = 0;

	if ((millis() - wifi_last_status_sent) > SEND_WIFI_STATUS_TIMEOUT_MS) {	 	// Every 5 Minutes
		if(!wifi_connected) {					// Try to reconnect if disconnected
			ConnectWifi("");
			ConnectFirebase();
		}
		else{
			SendWiFiTimestamp();				// Send status 
		}
		wifi_last_status_sent = millis();
	}
}

static void SendNFCTimetamp(){
	if(IsFireBaseReady(sendDataPrevNFC)){
		if(Firebase.RTDB.setTimestamp(&fbdo, "lockers/locker1/nfc-last-connected")){
			DBGL("\nNFC status sent:");
		}
		sendDataPrevNFC = millis();
	}
}

static void SendNFCStatus(){
	static uint32_t last_time_sent = 0;
	static boolean last_nfc_conn_state = false;


	if (((millis() - last_time_sent) > SEND_NFC_STATUS_TIMEOUT_MS)){
		SendNFCTimetamp();
		last_time_sent = millis();
	}
}

// Prints all user tags from Firebase database
void printUserTagsFirebase(){
	DBGL("FIREBASE USERS:");
	for (size_t i = 0; i < numUsersFireBase;  ++i){
		DBGL(userTagsFirebase[i]);
	}
}

// Returns all users from FireBase database that should have access to open locker
boolean GetAllUsersFireBase() {
	if(!wifi_connected) return false;
	FirebaseJson fbjson;
	QueryFilter filter;
	FirebaseJsonData result;

	filter.orderBy("$key");
	if(IsFireBaseReady(sendDataPrev)){
		if (Firebase.RTDB.getJSON(&fbdo, "cards/", &filter)) // Looks if cards contains a node that satisfies nodeName == tagUUID
		{
			fbjson = fbdo.to<FirebaseJson>();
			String jsonstr;
			fbjson.toString(jsonstr,true);
			size_t size = strlen(jsonstr.c_str());
			DynamicJsonDocument doc(size + 1);
			DeserializationError error = deserializeJson(doc, jsonstr);

			if (error) {
				DBG(F("JSON parsing failed: "));
				DBGL(error.c_str());
				doc.clear();
				return false;  
			}
            numUsersFireBase = 0;
			unsigned long long current_timestamp = ConvertToTimeStamp(GetEpochTime());

			for (const auto& element : doc.as<JsonObject>()) {
				const char* userTag = element.key().c_str();
                fbjson.get(result, String(userTag) + "/date_valid");

                unsigned long long date_valid_timestamp = (unsigned long long)result.doubleValue;

				if (current_timestamp <= date_valid_timestamp) {
					userTagsFirebase[numUsersFireBase] = strdup(userTag); // Use strdup to duplicate the string
					numUsersFireBase++;
				}
			}
			doc.clear();
			userTagsFirebase[numUsersFireBase] = NULL;
			sendDataPrev = millis();
			return true;
		}
	}
	return false;
}

// Checks every 5 seconds, 
// Opens solenoid if "Ava kapp" pressed on webpage.
CMD_TYPE_E FireBaseCheckSolenoid() {
	static uint32_t solenoid_last_checked = 0;
	static boolean solenoid_state = false;

	if(millis() - solenoid_last_checked > CHECK_SOLENOID_TIMEOUT_MS) {
		if (IsFireBaseReady(sendDataPrevSolenoid)) {
			if(Firebase.RTDB.getBool(&fbdo,"lockers/locker1/solenoid-activated")){
				solenoid_state = fbdo.to<bool>();
				solenoid_last_checked = millis();
			}
			sendDataPrevSolenoid = millis();
		}
	}
	if(solenoid_state){
		return CMD_KEEP_OPEN;
	}
	return CMD_DONT_OPEN;
}

// Checks if door state has changed and updates it in Firebase database
void FireBaseUpdateDoorState(boolean door_state) {
	static uint32_t last_time_sent = 0;
	static boolean last_door_state = false;

	if (((millis() - last_time_sent) > UPDATE_DOOR_TIMEOUT_MS) && last_door_state != door_state){
		if(IsFireBaseReady(sendDataPrevDoor)){
			if(Firebase.RTDB.setBool(&fbdo, "lockers/locker1/door-open", door_state)){
				DBG("Door state sent: ");
				door_state == false ? DBGL("Closed") : DBGL("Open");
			}
			sendDataPrevDoor = millis();
		}
		last_time_sent = millis();
		last_door_state = door_state;
	}
}

// Sends connection statuses of WiFi and NFC reader to Firebase database
// Additionally tries to reconnect WiFi if it's disconnected
void SendDeviceStatuses(){
	SendWiFiStatus();
	SendNFCStatus();
}

// Opens locker if scanned card has access
// Adds to log if access rights exist
// Additionally lets teacher add users 
CMD_TYPE_E FireBaseTask(String *UUID)
{
	if (*UUID == "") {
		return CMD_DONT_OPEN;
	}

	if(!wifi_connected) {
		return GetTagEEPROM(*UUID); // Returns the CMD if is present in EEPROM
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
			DBGL("Scanned UUID does not have access");
			return CMD_DONT_OPEN;
		}
		sendDataPrev = millis();
	}
	return CMD_DONT_OPEN;
}

// Creates WiFi connection
void ConnectWifi(const char* pass)
{
	static int i = 0;
	DBGL(pass);
	WiFi.begin(WIFI_SSID, pass);
	DBGL("Connecting to Wi-Fi");

	unsigned long startMillis = millis();

	while (WiFi.status() != WL_CONNECTED && millis() - startMillis < WIFI_TIMEOUT_MS) {
		DBG(".");
		delay(100);
	}

	if (WiFi.status() == WL_CONNECTED) {
		ConfigTime();
		wifi_connected = true;
		DBGL();
		DBG("Connected with IP: ");
		DBGL(WiFi.localIP());
		DBGL();
	} else {
		DBGL("\nWi-Fi connection failed. Timeout reached.");
		wifi_connected = false;
		// Handle the failure or retry if needed
	}
	i++;
}

// Creates Firebase connection
void ConnectFirebase()
{
	if(!wifi_connected) return;
	/* Assign the api key (required) */
	config.api_key = API_KEY;

	/* Assign the RTDB URL (required) */
	config.database_url = DATABASE_URL;


	auth.user.email = "FIREBASE_EMAIL";
	auth.user.password = "FIREBASE_PASSWORD";

	signupOK = true;

	/* Assign the callback function for the long running token generation task */
	config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);
}