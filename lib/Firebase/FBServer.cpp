#include <Arduino.h>
#include "FBServer.h"
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "CGA2121_2.4GHZ"
#define WIFI_PASSWORD "kell1234"
#define API_KEY "AIzaSyDAprMNQ1yMMMm0AFMB_KIQuyNl7WLr7Cw"
#define DATABASE_URL "nutikapp-default-rtdb.europe-west1.firebasedatabase.app"

FirebaseData fbdo;
FirebaseJson fjson;

FirebaseAuth auth;
FirebaseConfig config;

uint32_t sendDataPrev = 0;
uint32_t nextEntryNumber = 1; 
int count = 0;
bool signupOK = false;

void ConnectWifi(){
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.println("");
	Serial.println("Connecting to Wi-Fi");
	while (WiFi.status() != WL_CONNECTED) // Add timeout
	{
		Serial.print(".");
		delay(300);
	}
	Serial.println();
	Serial.print("Connected with IP: ");
	Serial.println(WiFi.localIP());
	Serial.println();
}

void ConnectFirebase(){
	/* Assign the api key (required) */
	config.api_key = API_KEY;

	/* Assign the RTDB URL (required) */
	config.database_url = DATABASE_URL;

	/* Sign up */
	if (Firebase.signUp(&config, &auth, "", "")) {
		Serial.println("Signup OK");
		signupOK = true;
	}
	else {
		Serial.printf("%s\n", config.signer.signupError.message.c_str());
	}

	/* Assign the callback function for the long running token generation task */
	config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);
}

static inline boolean FireBaseReady(uint32_t sendDataPrev){
	if(Firebase.ready() && signupOK && (millis() - sendDataPrev > 1000 || sendDataPrev == 0)) {
		return true;
	}
	return false;
}

static FirebaseJson SetLogEntry(String tagID){
	FirebaseJson fbjson;
	fbjson.add("Event", "Opened");
	fbjson.add("UUID", tagID);
	return fbjson;
}

static String FireBaseGetUUID(String tagID){
	FirebaseJson fbjson;
	QueryFilter query;
	if(Firebase.RTDB.getJSON(&fbdo, "cards/"+tagID)){
		fbjson = fbdo.to<FirebaseJson>();
		String jsonStr;
		fbjson.toString(jsonStr);
		// Serial.println(jsonStr);
		DynamicJsonDocument doc(1024);
		DeserializationError error = deserializeJson(doc, jsonStr);
		if (error) {
			Serial.print(F("JSON parsing failed: "));
			Serial.println(error.c_str());
			return String();
 	 	}
		String type = doc["type"].as<String>();
		return type;
	}
	return String();
}

static int GetNextEntryNumber(){ // Returns the next entry nr.
	FirebaseJson fbjson;
	QueryFilter query;
	query.orderBy("$key");
	query.limitToLast(1);
	if(Firebase.RTDB.getJSON(&fbdo,"log", &query)){
		fbjson = fbdo.to<FirebaseJson>();
		String jsonStr;
		fbjson.toString(jsonStr);
		DynamicJsonDocument doc(1024);
		deserializeJson(doc, jsonStr);

		const char* firstKey = doc.as<JsonObject>().begin()->key().c_str();
		String keyString(firstKey);

		int keyInt = keyString.toInt();
		
		Serial.print("Previous entry: ");
		Serial.println(keyInt);
		if(keyInt == 0){
			keyInt = 1;
		}
		// Serial.println(jsonStr.c_str());
		return keyInt;
	}
	Serial.println("Get next entry error");
	return 0; // Error
}

// static int GetLogCount(){
// 	if(Firebase.RTDB.getString(&fbdo,("log/count"))){
// 		int count = fbdo.intData();
// 		return count;
// 	}
// 	return 0;
// }

// static void IncrementLogCount(int count){
// 	if(Firebase.RTDB.setInt(&fbdo, "log/count", count)){
// 		// Serial.println("Incremented");
// 	}
// 	else{
// 		Serial.println("Failed to increment");
// 	}
// }

// static void IncrementLogCount(){
// 	int count = GetLogCount();
//     IncrementLogCount(count + 1);
// }

static boolean FireBaseAddLog(String tagID) {
	FirebaseJson fbjson = SetLogEntry(tagID);
	int log_count = GetNextEntryNumber();
	std::string log_str = std::to_string(log_count + 1);

	// Incase the database node was deleted. 
	// Because the query.limitToLast(1) will return 0
	// if only 1 log in the node.
	if (log_count == 0){
		log_count++; 
	}
	
	if(Firebase.RTDB.setJSON(&fbdo, "log/" + log_str, &fbjson)){
		if(Firebase.RTDB.setTimestamp(&fbdo, "log/" + log_str + "/timestamp")){
			Serial.println("Log added");
			return true;
		}
	}
	else {
		Serial.println("Failed");
	}
	return false;
}



void FireBaseTask2(String tagID){
	if (tagID == "") return;

	if (FireBaseReady(sendDataPrev)){
		sendDataPrev = millis();

		FireBaseAddLog(tagID);

	}
}

static FirebaseJson SetStudentJSON(){
	FirebaseJson fbjson;
	fbjson.add("owner", "TBD");
	fbjson.add("type", "student");
	return fbjson;
}

static boolean AddStudentRights(String tagID){
	FirebaseJson fbjson = SetStudentJSON();
	if(Firebase.RTDB.setJSON(&fbdo, "cards/" + tagID, &fbjson)){
		if(Firebase.RTDB.setTimestamp(&fbdo, "cards/" + tagID + "/timestamp")){
			Serial.println("Student added");
			return true;
		}
	}
	Serial.println("ERROR: Student not added");
	return false;
}

static void TeacherTask(scannedTag_t *tag) {
	if(tag->scanType == LONG_SCAN) {
		AddStudentRights(tag->tagID);
		Serial.println("Adding to rights: student");

	}
	else {
		FireBaseAddLog(tag->tagID);
		Serial.println("Adding to log: teacher");
		// Open door, add to log
	}
}

static void StudentTask(scannedTag_t *tag) {
	Serial.println("Adding to log: student");
	FireBaseAddLog(tag->tagID);
	// Timestamp check
	// Open door, add to log
}


void FireBaseTask(scannedTag_t *tag){

	if (tag->tagID == "") {
		return;
	}

	if (FireBaseReady(sendDataPrev))
	{
		sendDataPrev = millis();
		String type = FireBaseGetUUID(tag->tagID);
		if (type == "teacher"){
			TeacherTask(tag);
		}
		else if(type == "student"){
			StudentTask(tag);
		}
		else{
			Serial.println("Tag not found");
		}
		Serial.println(type);
	}
}