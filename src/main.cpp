#include <Arduino.h>
#include "nfc.h"
#include "SerialDebug.h"

#include <ESP8266WiFi.h>

#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "CGA2121_2.4GHZ"
#define WIFI_PASSWORD "kell1234"

// Insert Firebase project API Key
#define API_KEY "AIzaSyDAprMNQ1yMMMm0AFMB_KIQuyNl7WLr7Cw"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "nutikapp-default-rtdb.europe-west1.firebasedatabase.app"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseJson fjson;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

void ConnectWifi(){
	WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
	Serial.print("Connecting to Wi-Fi");
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
		Serial.println("ok");
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

void setup()
{
	Serial.begin(115200);
	// ConnectWifi();
	// ConnectFirebase();
	nfcBegin();
}

/*
TODO: 	1) When NFC reader disconnects add reconnect.
		2) Keep log of statuses of devices.
		Maybe after every 30min wake from sleep and send statuses.
		TBD: Wifi stay connected when sleeping? Power consumption with and without.
		Eg. NFC:
				Connected:		True/false
				Last active:	Timestamp

*/		


void FireBaseTask(){
	if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
	{
		sendDataPrevMillis = millis();
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

void loop()
{
	FirebaseJson logJSON;
	logJSON.add("Entry", "Opened Locker");
	nfcTask();
	// FireBaseTask();
	
}