#include <Arduino.h>
#include "FBServer.h"
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include "time.h"
#include "nfc.h"

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define WIFI_SSID "CGA2121_2.4GHZ"
#define WIFI_PASSWORD "kell1234"
#define API_KEY "AIzaSyDAprMNQ1yMMMm0AFMB_KIQuyNl7WLr7Cw"
#define DATABASE_URL "nutikapp-default-rtdb.europe-west1.firebasedatabase.app"

#define NTP_SERVER "pool.ntp.org"
#define TIMEZONE_OFFSET 7200 // GMT+2 (2 hours in seconds)
#define DAY_LIGHT_SAVINGS 0	 // 1 Hour Estonia DST (Day Light Savings)

#define ADD_STUDENT '1'
#define OPEN_DOOR '2'

FirebaseData fbdo;
FirebaseJson fjson;

FirebaseAuth auth;
FirebaseConfig config;

uint32_t sendDataPrev = 0;
uint32_t nextEntryNumber = 1;
int count = 0;
bool signupOK = false;

static void PrintTime(struct tm timeinfo)
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

// Get_Epoch_Time() Function that gets current epoch time
struct tm Get_Epoch_Time()
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

struct tm ConvertFromTimeStamp(unsigned long long tstamp)
{
	time_t now = static_cast<time_t>(tstamp);
	struct tm timeinfo;

	if (!getLocalTime(&timeinfo))
	{
		Serial.println("Failed to obtain time");
		return timeinfo;
	}
	time(&now);
	PrintTime(timeinfo);
	return timeinfo;
}

static unsigned long long ConvertToTimeStamp(struct tm timeinfo)
{
	time_t timestamp = mktime(&timeinfo);

	if (timestamp != -1)
	{
		return static_cast<unsigned long long>(timestamp);
	}
	else
	{
		// Handle the error case, e.g., by returning an error code
		Serial.println("ERROR: Convert to time stamp failed");
		return 0;
	}
}

// Sygis semester: ~ 04.09 - 21.01 | kuud: 7, 8, 9, 10, 11, 12, 1
// Kevad semester: ~ 29.01 - 12.6  | kuud: 1, 2, 3, 4, 5, 6

struct tm GetSemesterEnd()
{ // Returns struct tm of semester end

	struct tm current_time = Get_Epoch_Time();

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

void ConfigTime()
{
	configTime(TIMEZONE_OFFSET, DAY_LIGHT_SAVINGS, NTP_SERVER);
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
	if (Firebase.signUp(&config, &auth, "", ""))
	{
		Serial.println("Signup OK");
		signupOK = true;
	}
	else
	{
		Serial.printf("%s\n", config.signer.signupError.message.c_str());
	}

	/* Assign the callback function for the long running token generation task */
	config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

	Firebase.begin(&config, &auth);
	Firebase.reconnectWiFi(true);
}

static inline boolean IsFireBaseReady(uint32_t sendDataPrev)
{
	if (Firebase.ready() && signupOK && (millis() - sendDataPrev > 1000 || sendDataPrev == 0))
	{
		return true;
	}
	return false;
}

static FirebaseJson SetLogEntryJSON(String tagUUID, String name, String type)
{
	FirebaseJson fbjson;
	fbjson.add("name", name);
	fbjson.add("UUID", tagUUID);
	fbjson.add("type", type);
	fbjson.add("Event", "Opened");

	return fbjson;
}

static String GetUserInfo(String tagUUID, authorized_user_t *user)
{
	FirebaseJson fbjson;

	if (Firebase.RTDB.getJSON(&fbdo, "cards/" + tagUUID))
	{
		fbjson = fbdo.to<FirebaseJson>();
		String jsonStr;
		fbjson.toString(jsonStr);
		// Serial.println(jsonStr);
		DynamicJsonDocument doc(1024);
		DeserializationError error = deserializeJson(doc, jsonStr);
		if (error)
		{
			Serial.print(F("JSON parsing failed: "));
			Serial.println(error.c_str());
			return String();
		}

		user->UUID = tagUUID;
		user->type = doc["type"].as<String>();
		user->owner = doc["owner"].as<String>();
		user->date_added = doc["date_added"].as<unsigned long long>();
		user->date_valid = doc["date_valid"].as<unsigned long long>();
		return user->type;
	}
	Serial.print(F("Doesnt exist"));
	return String();
}

static int GetNextLogNumber()
{ // Returns the next entry nr.
	FirebaseJson fbjson;
	QueryFilter query;
	query.orderBy("$key");
	query.limitToLast(1);
	if (Firebase.RTDB.getJSON(&fbdo, "log", &query))
	{
		fbjson = fbdo.to<FirebaseJson>();
		String jsonStr;
		fbjson.toString(jsonStr);
		DynamicJsonDocument doc(1024);
		deserializeJson(doc, jsonStr);

		const char *firstKey = doc.as<JsonObject>().begin()->key().c_str();
		String keyString(firstKey);

		int keyInt = keyString.toInt();

		// Serial.print("Previous entry: ");
		// Serial.println(keyInt);
		if (keyInt == 0)
		{
			keyInt = 1;
		}
		// Serial.println(jsonStr.c_str());
		return keyInt;
	}
	Serial.println("Get next entry error");
	return 0; // Error
}

static CMD_TYPE_E AddToLog(authorized_user_t *user)
{
	FirebaseJson fbjson = SetLogEntryJSON(user->UUID, user->owner, user->type);
	int log_count = GetNextLogNumber();
	std::string log_str = std::to_string(log_count + 1);

	// Incase the database node was deleted.
	// Because the query.limitToLast(1) will return 0
	// if only 1 log in the node.
	if (log_count == 0)
	{
		log_count++;
	}

	if (Firebase.RTDB.setJSON(&fbdo, "log/" + log_str, &fbjson))
	{
		if (Firebase.RTDB.setTimestamp(&fbdo, "log/" + log_str + "/timestamp"))
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

static FirebaseJson SetStudentJSON()
{
	FirebaseJson fbjson;
	struct tm semester_end = GetSemesterEnd();									  // Gets semester end date
	unsigned long long semester_end_timestamp = ConvertToTimeStamp(semester_end); // Converts into timestamp
	fbjson.add("owner", "TBD");
	fbjson.add("type", "teacher"); // UNTILL NEW SCAN SET
	fbjson.add("date_valid", semester_end_timestamp);

	return fbjson;
}

static boolean AddStudentRights(String tagUUID)
{
	if (!ReadNewCard(&tagUUID))
	{
		Serial.println("Failed to read new card");
		return false;
	}
	FirebaseJson fbjson = SetStudentJSON();
	if (Firebase.RTDB.setJSON(&fbdo, "cards/" + tagUUID, &fbjson))
	{
		if (Firebase.RTDB.setTimestamp(&fbdo, "cards/" + tagUUID + "/date_added"))
		{
			Serial.println("Student added");
			return true;
		}
	}
	Serial.println("ERROR: Student not added");
	return false;
}

static char AskForInput()
{ // Temporary function until screen is used
	Serial.println("1 - Add student");
	Serial.println("2 - Open door");
	long timeout = millis();
	char inByte = '0';
	do
	{
		if (Serial.available())
		{
			inByte = Serial.read();
			if (inByte == ADD_STUDENT || inByte == OPEN_DOOR)
			{
				Serial.println("Exiting loop");
				return inByte;
			}
		}
	} while (millis() - timeout < 5000);

	Serial.println("Timeout");
	return '0';
}

static CMD_TYPE_E TeacherTask(authorized_user_t *user)
{
	char cmdAnswer = AskForInput();
	switch (cmdAnswer)
	{
	case ADD_STUDENT:
		AddStudentRights(user->UUID);
		return CMD_SCAN_NEW;
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

static CMD_TYPE_E StudentTask(authorized_user_t *user)
{
	// Serial.println("Adding to log: student");
	if (AddToLog(user))
	{
		return CMD_OPEN;
	}
	return CMD_DONT_OPEN;

	// Timestamp check
	// Open door, add to log
}

// TODO:
// Check status of locker after every 5 seconds.
// to see if from web interface locker has been opened
// returns enum command - Open locker, Dont open and scan again

static void PrintUser(authorized_user_t user)
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

CMD_TYPE_E FireBaseTask(String *UUID)
{

	if (*UUID == "")
	{
		return CMD_DONT_OPEN;
	}
	authorized_user_t user;
	if (IsFireBaseReady(sendDataPrev))
	{
		sendDataPrev = millis();
		String type = GetUserInfo(*UUID, &user);
		PrintUser(user);
		if (type == "teacher")
		{
			return TeacherTask(&user);
		}
		else if (type == "student")
		{
			return StudentTask(&user);
		}
		else
		{
			Serial.println("Scanned UUID does not have access");
			return CMD_DONT_OPEN;
		}
		Serial.println(type);
	}
	return CMD_DONT_OPEN;
}