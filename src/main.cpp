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

void drawCloseButton()
{
	int closeButtonSize = 15;
	int closeButtonX = tft.width() - closeButtonSize - 40;
	int closeButtonY = 20;
	int lineWidth = 5;
	for (int i = 0; i < lineWidth; ++i)
	{
		tft.drawLine(closeButtonX + i, closeButtonY, closeButtonX + closeButtonSize + i, closeButtonY + closeButtonSize, TFT_MAROON);
		tft.drawLine(closeButtonX + i, closeButtonY + closeButtonSize, closeButtonX + closeButtonSize + i, closeButtonY, TFT_MAROON);
	}
}

void addingEndedView(boolean added)
{
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextDatum(MC_DATUM);
	if(added){
		tft.drawString("Successfully added user", 160, 105);
	}
	else{
		tft.drawString("No card scanned", 160, 105);
	}
	
	delay(3000);
	tft.unloadFont();
	normalView();
}

void addBubble(int32_t x,int32_t y,int32_t w,int32_t h, int32_t fill, const char* str){
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
	static uint32_t now = millis();
	tft.loadFont(SatoshiBlack32);
	tft.fillScreen(TFT_BLACK);
	tft.setTextColor(TFT_WHITE, TFT_BLACK);
	tft.setTextDatum(MC_DATUM);
	tft.drawString("Scan card", 160, 100);
	tft.drawString("to add user", 160, 140);
	Serial.write('a');
	tft.setTextDatum(ML_DATUM);
	while (!Serial.available()){
		if((millis() - now) > 3000) {
			// tft.fillScreen(TFT_BLACK);
			break;
		}
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
	Serial.println("HELLO");
	tft.init();

	tft.setRotation(1);

	touch_calibrate();

	tft.fillScreen(TFT_BLACK);
	// tft.loadFont(NotoSansBold36);
	// draw normal view
	normalView();
	Serial.flush();
}

void displaySreenMiddle(const char* str, uint16_t bgc){
	tft.loadFont(SatoshiBlack32);
	tft.fillScreen(bgc);
	tft.setTextColor(TFT_WHITE, bgc);
	tft.setTextDatum(MC_DATUM);
	tft.drawString(str, 160, 120);
	tft.unloadFont();
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
		// unsigned char chars[5] = {'a','o','d','n','c'};
		// if(i == 5) i = 0;
		// rec = chars[i];

		if (rec == 'a')
		{
			adminView();
		}
		else if (rec == 'o')
		{
			addBubble(40,80,240,65,TFT_DARKGREEN, "Lock opened");
			delay(2000);
		}
		else if (rec == 'd')
		{
			addBubble(40,80,240,65,TFT_DARKGREEN, "Door opened");
			delay(2000);
		}
		else if (rec == 'n')
		{
			addBubble(40,80,240,65,TFT_MAROON, "No access");
			delay(2000);
		}
		else if (rec == 'c')
		{
			addBubble(40,80,240,65,TFT_DARKGREY, "Door closed");
			delay(2000);
		}
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
				if (x > 270 && y < 20)
				{
					Serial.write('c');
					normalView();
				}
				if ((x > ADDBUTTON_X) && (x < (ADDBUTTON_X + ADDBUTTON_W)))
				{
					if ((y > ADDBUTTON_Y) && (y <= (ADDBUTTON_Y + ADDBUTTON_H)))
					{
						// addUserView();
						displaySreenMiddle("User Add", TFT_DARKGREEN);
						delay(2000);
						normalView();
					}
				}
				if ((x > LOGBUTTON_X) && (x < (LOGBUTTON_X + LOGBUTTON_W)))
				{
					if ((y > LOGBUTTON_Y) && (y <= (LOGBUTTON_Y + LOGBUTTON_H)))
					{
						displaySreenMiddle("Door open", TFT_DARKGREY);
						delay(2000);
						normalView();
					}
				}
			}
	}
}
