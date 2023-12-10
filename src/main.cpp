#include <Arduino.h>
#include "SerialDebug.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <LittleFS.h>

#include "SatoshiBlack24.h"
#include "SatoshiBlack32.h"
#include "SatoshiBlack36.h"
#include "SatoshiBlack46.h"
#include "SatoshiMedium24.h"
#include "SatoshiMedium20.h"

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
#define RECEIVE_ASK_INPUT 				'a'
#define RECEIVE_OPEN_LOCK 				'o'
#define RECEIVE_OPEN_DOOR 				'd'
#define RECEIVE_CLOSE_DOOR 				'c'
#define RECEIVE_NO_ACCESS 				'n'
#define RECEIVE_ADDING_UNSUCCESSFUL  	'e'
#define RECEIVE_ADDING_SUCCESSFUL  		'l'

#define SEND_OPEN_LOCK					's'
#define SEND_ADD_STUDENT 				'a'
#define SEND_CLOSE_ADMIN				'c'
// Add button size
#define ADDBUTTON_X 					40
#define ADDBUTTON_Y 					80
#define ADDBUTTON_W 					240
#define ADDBUTTON_H 					65

// Log  button size
#define LOGBUTTON_X 					40
#define LOGBUTTON_Y 					160
#define LOGBUTTON_W 					240
#define LOGBUTTON_H 					65

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
byte rec = 0;
uint16_t x, y;

int rgb(unsigned char r, unsigned char g, unsigned char b) {
    if (r < 0 || 255 < r || g < 0 || 255 < g || b < 0 || b > 255)
        return -1;

    unsigned char red = r >> 3;
    unsigned char green = g >> 2;
    unsigned char blue = b >> 3;

    int result = (red << (5 + 6)) | (green << 5) | blue;
    return result;
}
// 7 132 181
void normalView()
{
	adminViewOn = false;
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextDatum(MC_DATUM);
	tft.loadFont(SatoshiBlack46);
	tft.drawString("Scan card", 160, 120);
	tft.unloadFont();
}

void Boot()
{
	adminViewOn = false;
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextDatum(MC_DATUM);
	tft.loadFont(SatoshiBlack46);
	tft.drawString("Boot success", 160, 120);
	tft.unloadFont();
}

void drawCloseButton()
{
	int closeButtonSize = 15;
	int closeButtonX = tft.width() - closeButtonSize - 40;
	int closeButtonY = 20;
	int lineWidth = 5;
	for (int i = 0; i < lineWidth; ++i)
	{
		tft.drawLine(closeButtonX + i, closeButtonY, closeButtonX + closeButtonSize + i, closeButtonY + closeButtonSize, rgb(190,0,0));
		tft.drawLine(closeButtonX + i, closeButtonY + closeButtonSize, closeButtonX + closeButtonSize + i, closeButtonY, rgb(190,0,0));
	}
}

void addingEndedView(boolean added)
{
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
	tft.setTextDatum(MC_DATUM);
	tft.setTextWrap(true);
	if(added){
		tft.drawString("Successfully", 160, 105);
		tft.drawString("added user", 160, 145);
		delay(3000);
		normalView();
	}
	else{
		tft.drawString("No card scanned", 160, 105);
		delay(3000);
		normalView();
	}
}

void addBubble(int32_t x,int32_t y,int32_t w,int32_t h, int32_t fill, const char* str){
	tft.fillScreen(TFT_BLACK);
	tft.drawSmoothRoundRect(x, y, 10, 8, w, h, TFT_WHITE, TFT_BLACK, 0x0F);
	tft.fillSmoothRoundRect(x+2,y+2,w-3,h-3,8, fill, TFT_WHITE);
	tft.loadFont(SatoshiBlack32);
    tft.setTextColor(TFT_WHITE, fill);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(str, x + (w / 2), y + (h / 1.75));
	tft.unloadFont();
}

void addBtnRound()
{
	//rgb(59, 130, 246)
	//rgb(36,36,36)
	int32_t x = ADDBUTTON_X;
    int32_t y = ADDBUTTON_Y;
    int32_t r = 10;  // Adjust the radius of the outer arc
    int32_t ir = 8;  // Adjust the radius of the inner arc (for anti-aliasing)
    int32_t w = ADDBUTTON_W;
    int32_t h = ADDBUTTON_H;
    uint32_t fg_color = TFT_WHITE;  // Adjust color as needed
    uint32_t bg_color = TFT_BLACK;  // Adjust color as needed
    uint8_t quadrants = 0x0F;  // Draw all four quadrants (0x0F for top-left, top-right, bottom-left, bottom-right)
	
	tft.drawSmoothRoundRect(x, y, r, ir, w, h, fg_color, bg_color, quadrants);
	tft.fillSmoothRoundRect(x+2,y+2,w-3,h-3,ir,rgb(7, 132, 181), TFT_WHITE);

    tft.setTextColor(TFT_WHITE, rgb(7, 132, 181));
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Add cards", ADDBUTTON_X + (ADDBUTTON_W / 2), ADDBUTTON_Y + (ADDBUTTON_H / 1.75));
}

void doorBtn()
{
	int32_t x = LOGBUTTON_X;
    int32_t y = LOGBUTTON_Y;
    int32_t r = 10;  // Adjust the radius of the outer arc
    int32_t ir = 8;  // Adjust the radius of the inner arc (for anti-aliasing)
    int32_t w = LOGBUTTON_W;
    int32_t h = LOGBUTTON_H;
    uint32_t fg_color = TFT_WHITE;  // Adjust color as needed
    uint32_t bg_color = TFT_BLACK;  // Adjust color as needed
    uint8_t quadrants = 0x0F;  // Draw all four quadrants (0x0F for top-left, top-right, bottom-left, bottom-right)
	
	tft.drawSmoothRoundRect(x, y, r, ir, w, h, fg_color, bg_color, quadrants);
	tft.fillSmoothRoundRect(x+2,y+2,w-3,h-3,ir,rgb(7, 132, 181), TFT_WHITE);

    tft.setTextColor(TFT_WHITE, rgb(7, 132, 181));
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Open door", LOGBUTTON_X + (LOGBUTTON_W / 2), LOGBUTTON_Y + (LOGBUTTON_H / 1.75));

}

void adminView()
{
	tft.loadFont(SatoshiBlack24);
	adminViewOn = true;
	tft.fillScreen(TFT_BLACK);
	drawCloseButton();
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Admin view", 100, 40);
	doorBtn();
	addBtnRound();
	tft.unloadFont();
}

void addUserView()
{
	tft.loadFont(SatoshiBlack32);
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK, true);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Scan card", 160, 100);
	tft.drawString("to add user", 160, 140);
	Serial.write(SEND_ADD_STUDENT);
	tft.setTextDatum(ML_DATUM);

	long timeout = millis();
	long sec_timer = 0;
	long now;
	char inByte = '0';
	int count = 9;
	do
	{
		if(count < 0) break;
		now = millis();
		if((now - sec_timer) >= 1000) {
			tft.fillRect(150,190,27,31,TFT_BLACK);
			tft.setTextColor(TFT_WHITE, TFT_BLACK);
			tft.setCursor(150, 190);
			tft.print(count, 0);
			count--;
			sec_timer = now;
		}
		if (Serial.available())
		{
			inByte = Serial.read();
			if (inByte == RECEIVE_ADDING_SUCCESSFUL)
			{
				addingEndedView(true);
				tft.unloadFont();
				return;
			}
			else if (inByte == RECEIVE_ADDING_SUCCESSFUL)
			{
				addingEndedView(false);
				tft.unloadFont();
				return;
			}	
		}
	} while (now - timeout < 10000); // Waits for 5 seconds from serial monitor input
	addingEndedView(false);

	tft.unloadFont();
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
		tft.fillScreen(TFT_BLACK);
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

	tft.setRotation(1);

	touch_calibrate();

	tft.fillScreen(TFT_BLACK);
	Boot();
	delay(1000);
	normalView();
	Serial.flush();
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

void loop()
{

	// See if there's any serial communication data from other ESP
	if (Serial.available())
	{

		rec = Serial.read();

		if (rec == RECEIVE_ASK_INPUT)
		{
			adminView();
		}
		else if (rec == RECEIVE_OPEN_LOCK)
		{
			addBubble(40,80,240,65,TFT_DARKGREEN, "Lock opened");
			delay(5000);
			normalView();
		}
		else if (rec == RECEIVE_OPEN_DOOR)
		{
			addBubble(40,80,240,65,TFT_DARKGREEN, "Door opened");
			delay(3000);
			normalView();
		}
		else if (rec == RECEIVE_CLOSE_DOOR)
		{
			addBubble(40,80,240,65,TFT_DARKGREY, "Door closed");
			delay(3000);
			normalView();
		}
		else if (rec == RECEIVE_NO_ACCESS)
		{
			addBubble(40,80,240,65,TFT_MAROON, "No access");
			delay(3000);
			normalView();
		}
	}

	// See if there's any touch data from display
	uint32_t now = millis();
	uint32_t last = now;
	if (adminViewOn)
	{
		if(now - last < 5000){
			if (tft.getTouch(&x, &y))
			{
				if (x > 260 && y < 30)
				{
					Serial.write(SEND_CLOSE_ADMIN);
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
						Serial.write(SEND_OPEN_LOCK);
					}
				}
			}
			last = millis();
		}
		else normalView();
	}
}
