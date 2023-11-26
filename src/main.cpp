#include <Arduino.h>
#include "SerialDebug.h"
#include <SPI.h>
#include <TFT_eSPI.h> 
#include <LittleFS.h>

//DISPLAY - D1 mini

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

void drawCloseButton() {
  int closeButtonSize = 15;
  int closeButtonX = tft.width() - closeButtonSize - 10;
  int closeButtonY = 5; 
  int lineWidth = 5;
  for (int i = 0; i < lineWidth; ++i) {
    tft.drawLine(closeButtonX + i, closeButtonY, closeButtonX + closeButtonSize + i, closeButtonY + closeButtonSize, TFT_RED);
    tft.drawLine(closeButtonX + i, closeButtonY + closeButtonSize, closeButtonX + closeButtonSize + i, closeButtonY, TFT_RED);
  }
}

void addingEndedView(int userCount){
  String userString;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Successfully added", 160, 105);
  if(userCount == 1){
    userString = " user";
  }
  else{
    userString = " users";
  }
  tft.drawString(String(userCount) + userString, 160, 135);
  delay(3000);
}

void addBtn(){
  tft.fillRect(ADDBUTTON_X, ADDBUTTON_Y, ADDBUTTON_W, ADDBUTTON_H, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Add cards", ADDBUTTON_X + (ADDBUTTON_W / 2), ADDBUTTON_Y + (ADDBUTTON_H / 2));
}

void logsBtn(){
  tft.fillRect(LOGBUTTON_X, LOGBUTTON_Y, LOGBUTTON_W, LOGBUTTON_H, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("View logs", LOGBUTTON_X + (LOGBUTTON_W / 2), LOGBUTTON_Y + (LOGBUTTON_H / 2));
}

void normalView()
{
  adminViewOn = false;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansBold24pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Scan card", 160, 120);
}

void adminView()
{
  adminViewOn = true;
  tft.fillScreen(TFT_BLACK);
  drawCloseButton();
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansBold12pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Admin view", 100, 40);
  logsBtn();
  addBtn();
}

void addUserView(){
  int userCount = 0;
  tft.fillScreen(TFT_BLACK);
  drawCloseButton();
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansBold18pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Scan cards", 160, 100);
  tft.drawString("to add users", 160, 140);
  Serial.println("Add user");
  tft.setTextDatum(ML_DATUM);
  tft.setFreeFont(&FreeSansBold12pt7b);
  while(true){
    if (Serial.available()) {
      String receivedData = Serial.readString();
      if (receivedData == "User added\n") {
        userCount++;
        tft.drawString("Users added:", tft.width() - 190, 220);
        tft.fillRect(tft.width() - 30, 200, 30, 40, TFT_BLACK);
        tft.drawString(String(userCount), tft.width() - 30, 220);
      }
      if (receivedData == "Adding ended\n") {
        if(userCount > 0){
          addingEndedView(userCount);
        } 
        userCount = 0;
        normalView();
        break;
      }
    }
    if (tft.getTouch(&x, &y) && (x >= 290) && (y <= 30)){
      Serial.println("Closed");
      if(userCount > 0){
        addingEndedView(userCount);
      }
      userCount = 0;
      normalView();
      break;
    }
  }
}

void seeLogsView(){
  tft.fillScreen(TFT_BLACK);
  drawCloseButton();
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&FreeSansBold9pt7b);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Logs:", 30, 20); 
  Serial.println("Send logs");
  while(true){
    if (Serial.available()) {
      String receivedData = Serial.readString();
      tft.drawString(receivedData, 20, 40);
    }
    if (tft.getTouch(&x, &y) && (x >= 290) && (y <= 30)){
      Serial.println("Closed");
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
  if (!LittleFS.begin()) { 
    Serial.println("Formatting file system");
    LittleFS.format();
    LittleFS.begin();
  }

  // check if calibration file exists and size is correct
  if (LittleFS.exists(CALIBRATION_FILE)) { 
    if (REPEAT_CAL) {
      // Delete if we want to re-calibrate
      LittleFS.remove(CALIBRATION_FILE); 
    } else {
      File f = LittleFS.open(CALIBRATION_FILE, "r"); 
      if (f) {
        if (f.readBytes((char *)calData, 14) == 14)
          calDataOK = 1;
        f.close();
      }
    }
  }

  if (calDataOK && !REPEAT_CAL) {
    // calibration data valid
    tft.setTouch(calData);
  } else {
    // data not valid so recalibrate
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(20, 20);
    tft.setFreeFont(&FreeSansBold9pt7b);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);

    tft.println("Touch corners as indicated");

    tft.setTextFont(1);
    tft.println();

    if (REPEAT_CAL) {
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.println("Set REPEAT_CAL to false to stop this running again!");
    }

    tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.println("Calibration complete!");

    // store data
    File f = LittleFS.open(CALIBRATION_FILE, "w");
    if (f) {
      f.write((const unsigned char *)calData, 14);
      f.close();
    }
  }
} 

void setup(void)
{
  Serial.begin(115200);
  tft.init();

  // Set the rotation before we calibrate
  tft.setRotation(1);

  // call screen calibration
  touch_calibrate();

  // clear screen
  tft.fillScreen(TFT_BLACK);

  // draw normal view
  normalView();
}
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
void loop()
{

  // See if there's any serial communication data from other ESP
  if (Serial.available()) {
    String receivedData = Serial.readString();
    if (receivedData == "Admin view\n") {
      adminView();
    }
    if (receivedData == "Door opened\n") {
      tft.fillScreen(TFT_GREEN);
      tft.setTextColor(TFT_WHITE);
      tft.setFreeFont(&FreeSansBold24pt7b);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Door opened", 160, 120);
      delay(2000);
      normalView();
    }
    if (receivedData == "No access\n") {
      tft.fillScreen(TFT_RED);
      tft.setTextColor(TFT_WHITE);
      tft.setFreeFont(&FreeSansBold24pt7b);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("No access", 160, 120);
      delay(2000);
      normalView();
    }
    if (receivedData == "Door closed\n") {
      tft.fillScreen(TFT_DARKGREY);
      tft.setTextColor(TFT_WHITE);
      tft.setFreeFont(&FreeSansBold24pt7b);
      tft.setTextDatum(MC_DATUM);
      tft.drawString("Door closed", 160, 120);
      delay(2000);
      normalView();
    }
  }

  // See if there's any touch data from display
  if (adminViewOn){
    if (tft.getTouch(&x, &y)){
      if (x > 290 && y < 30) {
          Serial.println("Closed");
          normalView(); 
      }
      if ((x > ADDBUTTON_X) && (x < (ADDBUTTON_X + ADDBUTTON_W))) {
        if ((y > ADDBUTTON_Y) && (y <= (ADDBUTTON_Y + ADDBUTTON_H))) {
          addUserView();
        }
      }
      if ((x > LOGBUTTON_X) && (x < (LOGBUTTON_X + LOGBUTTON_W))) {
        if ((y > LOGBUTTON_Y) && (y <= (LOGBUTTON_Y + LOGBUTTON_H))) {
          seeLogsView();
        }
      }
    }
  }
}


