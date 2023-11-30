#include <Arduino.h>
#include "SerialDebug.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <LittleFS.h>

// DISPLAY - D1 mini

// VCC - 3.3V
// GND - GND
// CS - 15
// RESET - 2
// DC - 0
// SDI - 13
// SCK - 14
// LED - 3.3V
// SDO - 12
// T_CLK - 14
// T_CS - 16
// T_DIN - 13
// T_DO - 12

// Add button size
#define ADDBUTTON_X 40
#define ADDBUTTON_Y 80
#define ADDBUTTON_W 240
#define ADDBUTTON_H 65

// Log  button size
#define LOGBUTTON_X 40
#define LOGBUTTON_Y 160
#define LOGBUTTON_W 240
#define LOGBUTTON_H 65

TFT_eSPI tft = TFT_eSPI();

// This is the file name used to store the touch coordinate
// calibration data. Change the name to start a new calibration.
#define CALIBRATION_FILE "/TouchCalData"

// Set REPEAT_CAL to true instead of false to run calibration
// again, otherwise it will only be done once.
// Repeat calibration if you change the screen rotation.
#define REPEAT_CAL false

bool switchOn = false;
bool adminViewOn = false;
uint16_t x, y;

int rgb(unsigned char r, unsigned char g, unsigned char b) {
    if (r < 0 || 255 < r || g < 0 || 255 < g || b < 0 || b > 255)
        return -1;

    unsigned char red = r >> 3;
    unsigned char green = g >> 2;
    unsigned char blue = b >> 3;

    int result = (red << (5 + 6)) | (green << 5) | blue;

//tests
    // printf("red: %x\n", red);
    // printf("green: %x\n", green);
    // printf("blue: %x\n", blue);
    // printf("result: %x\n", result);

    return result;
}
// 7 132 181
void normalView()
{
	adminViewOn = false;
	tft.fillScreen(rgb(36,36,36));
	tft.setTextColor(TFT_WHITE);
	tft.setTextDatum(4);
	tft.setFreeFont(&Satoshi_Black24pt7b);
	tft.drawString("Scan card", 160, 120);
}

void drawCloseButton()
{
	int closeButtonSize = 15;
	int closeButtonX = tft.width() - closeButtonSize - 40;
	int closeButtonY = 20;
	int lineWidth = 5;
	for (int i = 0; i < lineWidth; ++i)
	{
		tft.drawLine(closeButtonX + i, closeButtonY, closeButtonX + closeButtonSize + i, closeButtonY + closeButtonSize, TFT_RED);
		tft.drawLine(closeButtonX + i, closeButtonY + closeButtonSize, closeButtonX + closeButtonSize + i, closeButtonY, TFT_RED);
	}
}

void addingEndedView(boolean added)
{
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&Satoshi_Black24pt7b);
	tft.setTextDatum(MC_DATUM);
	if(added){
		tft.drawString("Successfully added user", 160, 105);
	}
	else{
		tft.drawString("No card scanned", 160, 105);
	}
	
	delay(3000);
	normalView();
}

void addBtn()
{
	tft.fillRect(ADDBUTTON_X, ADDBUTTON_Y, ADDBUTTON_W, ADDBUTTON_H, rgb(7, 132, 181));
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&Satoshi_Black24pt7b);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Add cards", ADDBUTTON_X + (ADDBUTTON_W / 2), ADDBUTTON_Y + (ADDBUTTON_H / 2));
}

void logsBtn()
{
	tft.fillRect(LOGBUTTON_X, LOGBUTTON_Y, LOGBUTTON_W, LOGBUTTON_H, rgb(7, 132, 181));
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&Satoshi_Black24pt7b);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Open lock", LOGBUTTON_X + (LOGBUTTON_W / 2), LOGBUTTON_Y + (LOGBUTTON_H / 2));
}

void adminView()
{
	adminViewOn = true;
	tft.fillScreen(rgb(36,36,36));
	drawCloseButton();
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&Satoshi_Black20pt7b);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Admin view", 100, 40);
	logsBtn();
	addBtn();
}

void addUserView()
{
	static uint32_t now = millis();
	int userCount = 0;
	tft.fillScreen(rgb(36,36,36));
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&Satoshi_Regular20pt7b);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Scan card", 160, 100);
	tft.drawString("to add user", 160, 140);
	Serial.write('a');
	tft.setTextDatum(ML_DATUM);
	tft.setFreeFont(&Satoshi_Regular20pt7b);
	while (!Serial.available() || (millis() - now) > 3000){
		now = millis();
		// tft.drawString("Waiting", tft.width() - 190, 220);
	}

	byte rec = Serial.read();
	if (rec == 'l')
	{
		addingEndedView(true);
	}
	if (rec == 'e')
	{
		addingEndedView(false);
	}
}

void seeLogsView()
{
	tft.fillScreen(TFT_BLACK);
	drawCloseButton();
	tft.setTextColor(TFT_WHITE);
	tft.setFreeFont(&FreeSansBold9pt7b);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Logs:", 30, 20);
	Serial.write('s');
	while (true)
	{
		if (Serial.available())
		{
			String receivedData = Serial.readString();
			tft.drawString(receivedData, 20, 40);
		}
		if (tft.getTouch(&x, &y) && (x >= 290) && (y <= 30))
		{
			Serial.write('c');
			normalView();
			break;
		}
	}
}

void touch_calibrate()
{
	uint16_t calData[5];
	uint8_t calDataOK = 0;

	// check file system exists
	if (!LittleFS.begin())
	{
		Serial.println("Formatting file system");
		LittleFS.format();
		LittleFS.begin();
	}

	// check if calibration file exists and size is correct
	if (LittleFS.exists(CALIBRATION_FILE))
	{
		if (REPEAT_CAL)
		{
			// Delete if we want to re-calibrate
			LittleFS.remove(CALIBRATION_FILE);
		}
		else
		{
			File f = LittleFS.open(CALIBRATION_FILE, "r");
			if (f)
			{
				if (f.readBytes((char *)calData, 14) == 14)
					calDataOK = 1;
				f.close();
			}
		}
	}

	if (calDataOK && !REPEAT_CAL)
	{
		// calibration data valid
		tft.setTouch(calData);
	}
	else
	{
		// data not valid so recalibrate
		tft.fillScreen(rgb(36,36,36));
		tft.setCursor(20, 20);
		tft.setFreeFont(&FreeSansBold9pt7b);
		tft.setTextColor(TFT_WHITE, TFT_BLACK);

		tft.println("Touch corners as indicated");

		tft.setTextFont(1);
		tft.println();

		if (REPEAT_CAL)
		{
			tft.setTextColor(TFT_RED, TFT_BLACK);
			tft.println("Set REPEAT_CAL to false to stop this running again!");
		}

		tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

		tft.setTextColor(TFT_GREEN, TFT_BLACK);
		tft.println("Calibration complete!");

		// store data
		File f = LittleFS.open(CALIBRATION_FILE, "w");
		if (f)
		{
			f.write((const unsigned char *)calData, 14);
			f.close();
		}
	}
}

void setup(void)
{
	Serial.begin(9600);
	tft.init();

	// Set the rotation before we calibrate
	tft.setRotation(1);

	// call screen calibration
	touch_calibrate();

	// clear screen
	tft.fillScreen(rgb(36,36,36));

	// draw normal view
	normalView();
	Serial.flush();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
int i = 0;
byte rec;
void loop()
{

	// See if there's any serial communication data from other ESP
	if (Serial.available())
	{

		rec = Serial.read();

		if (rec == 'a')
		{
			adminView();
		}
		else if (rec == 'o')
		{
			tft.fillScreen(TFT_GREEN);
			tft.setTextColor(TFT_WHITE);
			tft.setFreeFont(&Satoshi_Black24pt7b);
			tft.setTextDatum(MC_DATUM);
			tft.drawString("Lock opened", 160, 120);
			delay(2000);
			normalView();
		}
		else if (rec == 'd')
		{
			tft.fillScreen(TFT_GREEN);
			tft.setTextColor(TFT_WHITE);
			tft.setFreeFont(&Satoshi_Black24pt7b);
			tft.setTextDatum(MC_DATUM);
			tft.drawString("Door opened", 160, 120);
			delay(2000);
			normalView();
		}
		else if (rec == 'n')
		{
			tft.fillScreen(TFT_RED);
			tft.setTextColor(TFT_WHITE);
			tft.setFreeFont(&Satoshi_Black24pt7b);
			tft.setTextDatum(MC_DATUM);
			tft.drawString("No access", 160, 120);
			delay(2000);
			normalView();
		}
		else if (rec == 'c')
		{
			tft.fillScreen(TFT_DARKGREY);
			tft.setTextColor(TFT_WHITE);
			tft.setFreeFont(&Satoshi_Black24pt7b);
			tft.setTextDatum(MC_DATUM);
			tft.drawString("Door closed", 160, 120);
			delay(2000);
			normalView();
		}
		// else if(rec != 0) {
		//   tft.fillScreen(TFT_BLACK);
		//   tft.setTextColor(TFT_WHITE);
		//   tft.setFreeFont(&FreeSansBold24pt7b);
		//   tft.setTextDatum(MC_DATUM);
		//   tft.drawString(String(rec), 160, 120);
		// }

		// String receivedData = Serial.readString();
		// rec = Serial.read();
		// receivedData.trim();
		// if (receivedData == "Admin view") {
		//   adminView();
		// }
		// else if (receivedData == "Door opened") {
		//   tft.fillScreen(TFT_GREEN);
		//   tft.setTextColor(TFT_WHITE);
		//   tft.setFreeFont(&FreeSansBold24pt7b);
		//   tft.setTextDatum(MC_DATUM);
		//   tft.drawString("Door opened", 160, 120);
		//   delay(2000);
		//   normalView();
		// }
		// else if (receivedData == "No access") {
		//   tft.fillScreen(TFT_RED);
		//   tft.setTextColor(TFT_WHITE);
		//   tft.setFreeFont(&FreeSansBold24pt7b);
		//   tft.setTextDatum(MC_DATUM);
		//   tft.drawString("No access", 160, 120);
		//   delay(2000);
		//   normalView();
		// }
		// else if (receivedData == "Door closed") {
		//   tft.fillScreen(TFT_DARKGREY);
		//   tft.setTextColor(TFT_WHITE);
		//   tft.setFreeFont(&FreeSansBold24pt7b);
		//   tft.setTextDatum(MC_DATUM);
		//   tft.drawString("Door closed", 160, 120);
		//   delay(2000);
		//   normalView();
		// }
	}

	// See if there's any touch data from display
	if (adminViewOn)
	{
		// static uint32_t last = millis();
		// uint32_t now = millis();
		// char sec[3];
		// while(now - last < 5000){
			if (tft.getTouch(&x, &y))
			{
				if (x > 290 && y < 30)
				{
					Serial.write('c');
					normalView();
				}
				if ((x > ADDBUTTON_X) && (x < (ADDBUTTON_X + ADDBUTTON_W)))
				{
					if ((y > ADDBUTTON_Y) && (y <= (ADDBUTTON_Y + ADDBUTTON_H)))
					{
						addUserView();
					}
				}
				if ((x > LOGBUTTON_X) && (x < (LOGBUTTON_X + LOGBUTTON_W)))
				{
					if ((y > LOGBUTTON_Y) && (y <= (LOGBUTTON_Y + LOGBUTTON_H)))
					{
						Serial.write('s');
						return;
					}
				}
			}
		// 	tft.setTextColor(TFT_BLACK);
		// 	tft.drawString(sec, 180, 40); // Clear the previous countdown value
		// 	sprintf(sec, "%d", (5000 - (now - last) + 999) / 1000); // Calculate remaining seconds
		// 	tft.setTextColor(TFT_WHITE);
		// 	tft.drawString(sec, 180, 40);
		// 	now = millis();
		// }
		// last = millis();
	}
}
