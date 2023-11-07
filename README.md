# NutiKapp

### TODO:
* Find TODO: in code and fix/change accordingly
* Add Touchscreen drivers
* Display feedback (error, success, options) on display
* Add magnet sensor drivers
* Add door closing logic with magnet sensor

#### The following folders under lib/ are middleware drivers The .h and .cpp files can be freely changed according to need. They contain application logic functions:

* Firebase
* NFC
* Locker - Empty, maybe add solenoid lock, magnet sensor logic and handling.

#### All the other folders under lib/ are low level/device drivers. Should be left as is and not be changed:

* ArduinoJson
* Firebase_Arduino_Client_Library_for_ESP8266_and_ESP32
* NDEF
* PN532
* PN532_I2C - Currently used
* PN532_SPI - Remove later?
* PN532_HSU - Remove later?
* PN532_SWSHU - Remove later?
