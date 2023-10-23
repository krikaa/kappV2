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

static boolean FireBaseReady(uint32_t sendDataPrev){
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
		Serial.println(jsonStr.c_str());
		return keyInt;
	}
	return 0; // Error

	// if(Firebase.RTDB.getJSON(&fbdo,"log", &query)){
	// 	fbjson = fbdo.to<FirebaseJson>();
	// 	String jsonStr;
	// 	fbjson.toString(jsonStr,"\n");
	// 	// Serial.println(jsonStr.c_str());
	// 	DynamicJsonDocument jsonDocument(256);
	// 	DeserializationError error = deserializeJson(jsonDocument, jsonStr);
	// 	if (error) {
	// 		Serial.print("Error parsing JSON: ");
	// 		Serial.println(error.c_str());
	// 	} else {
	// 		const ArduinoJson::JsonObject& entry = jsonDocument.as<ArduinoJson::JsonObject>();
	// 		String nextEntryNumber = "1";
	// 		for (ArduinoJson::JsonPair kv : entry) {
	// 			String entryKey = kv.key().c_str();
	// 			// Extract the entry number from the entry key
	// 			int entryNumber = entryKey.substring(5).toInt();
	// 			if (entryNumber >= nextEntryNumber.toInt()) {
	// 				nextEntryNumber = String(entryNumber + 1);
	// 			}
	// 		}
	// 		return nextEntryNumber;
	// 		Serial.println(nextEntryNumber); Serial.print(" Is the next entry");
	// 		Serial.println();
	// 		// Now you can work with the 'carParkingSlots' JsonObject
	// 	}
	// 	// Get Entry number
	// }
	// return String();
}

static int GetLogCount(){
	if(Firebase.RTDB.getString(&fbdo,("log/count"))){
		int count = fbdo.intData();
		return count;
	}
	return 0;
}

static void IncrementLogCount(int count){
	if(Firebase.RTDB.setInt(&fbdo, "log/count", count)){
		// Serial.println("Incremented");
	}
	else{
		Serial.println("Failed to increment");
	}
}

static void IncrementLogCount(){
	int count = GetLogCount();
    IncrementLogCount(count + 1);
}

void FireBaseTask2(String tagID){
	if (tagID == "") return;

	if (FireBaseReady(sendDataPrev)){
		sendDataPrev = millis();

		FirebaseJson fbjson = SetLogEntry(tagID);
		int log_count = GetNextEntryNumber();

		// Incase the database was deleted. 
		// Because the query.limitToLast(1) will return 0
		// if only 1 log in the node.
		if(log_count == 0){
			log_count++; 
		}
		
		if(Firebase.RTDB.setJSON(&fbdo, "log/" + std::to_string(log_count + 1), &fbjson)){
			// IncrementLogCount(log_count + 1);
			Serial.println("Added");
		}
		else{
			Serial.println("Failed");
		}



		// if (Firebase.RTDB.setJSON(&fbdo, "log/entry1", &fbjson)){
		// 	// if (Firebase.RTDB.pushTimestamp(&fbdo, "log")){
		// 	Serial.println("Success");
		// 	// }
		// }
		// else{
		// 	Serial.println("Failed");
		// }
		// fbjson = SetLogEntry(tagID, count++);

		// if (Firebase.RTDB.setJSON(&fbdo, "log/entry2", &fbjson)){
		// 	// if (Firebase.RTDB.pushTimestamp(&fbdo, "log")){
		// 	Serial.println("Success");
		// 	// }
		// }
		// else{
		// 	Serial.println("Failed");
		// }
	}
}

void FireBaseTask(){
	if (FireBaseReady(sendDataPrev))
	{
		sendDataPrev = millis();
		if(Firebase.RTDB.getJSON(&fbdo,"log/")){
			fjson = fbdo.to<FirebaseJson>();
			std::string str;
			fjson.toString(str, '\n');
			Serial.println("Json");
			// Serial.println(str.c_str());
		}
		else
		{
			Serial.println("FAILED");
			Serial.println("REASON: " + fbdo.errorReason());
		}

		if (Firebase.RTDB.setJSON(&fbdo, "test/int", &fjson)){
			Serial.println("PASSED");
			Serial.println("PATH: " + fbdo.dataPath());
			Serial.println("TYPE: " + fbdo.dataType());
		}
		// Write an Int number on the database path test/int
		// if (Firebase.RTDB.setInt(&fbdo, "test/int", count))
		// {
		// 	Serial.println("PASSED");
		// 	Serial.println("PATH: " + fbdo.dataPath());
		// 	Serial.println("TYPE: " + fbdo.dataType());
		// }
		else
		{
			Serial.println("FAILED");
			Serial.println("REASON: " + fbdo.errorReason());
		}
		count++;

		// Write a Float number on the database path test/float
		if (Firebase.RTDB.pushTimestamp(&fbdo, "test/float"))
		{
			Serial.println("PASSED");
			Serial.println("PATH: " + fbdo.dataPath());
			Serial.println("TYPE: " + fbdo.dataType());
		}
		else
		{
			Serial.println("FAILED");
			Serial.println("REASON: " + fbdo.errorReason());
		}
	}
}