# Nutika laborikapi edasiarendus 

Continuation of development of a smart metal locker module, to make it openable with an ISIC card or through a web application.

Continuation by (TalTech Computers and Systems project 2024):

* Kristjan Kaal
* Daniel Erg 
* Hendrik Haabu

[Original 2023 project](https://github.com/siimtishler/kapp) by (TalTech Computers and Systems project 2023):

* Markus Visnapuu 
* Rannar Randmäe 
* Siim Tišler
* Markus Veersoo

## Full documentation (in Estonian)

[Targa laborikapi edasiarendus aruanne.pdf](https://github.com/krikaa/kappV2/blob/master/Targa%20laborikapi%20edasiarendus%20aruanne.pdf) \
The documentation contains details about the improvements, description of the product and instructions on how it should be put together.

## Introduction
This is the repo for course "IAS1420 Arvutite ja süsteemide projekt" at TalTech 2023. 
The goal of the project was to fix some software issues regarding the 2023 year project, add support for multiple lockers and put all the hardware in a single module that is both simpler to install and more universal.
This repo is only for more convenient code management and project is intended only as a study project.

## Hardware
![Schematic](https://github.com/krikaa/kappV2/blob/master/Schematic.png)

## Installation
For fully setting up the device, the following has to be done:

1. Set up the web app:
* Clone the [web-interface](https://github.com/krikaa/kappV2-web-interface) submodule. 
* Set up a Google firebase with email-password user authentication and real-time database. 
* Add the rules found in database.rules.json to the Firebase database access rules. 
* Duplicate example.env, rename it to .env and fill the file in with the environment variables from your Firebase.
* Host the website on Firebase 
* Make the first admin account from Firebases management screen. 

2. Set up the electronics:
* Clone the [door-controller](https://github.com/krikaa/kappV2-door-controller/) submodule.
* Open the code in vscode and wait until necessary third-party libraries are installed through PlatformIO
* Duplicate FirebaseConfig.example.h, rename it to FirebaseConfig.h, and fill it in with your locker, Wi-Fi and Firebase details.
* Change PinConfig.h and platformio.ini to fit your hardware if necessary.
* Flash the code onto an ESP32.
* Build the [circuit](https://github.com/krikaa/kappV2/blob/master/Schematic.png) and test if everything works.

3. Set up the module:
* Print or cut out the models found in the [enclosures](https://github.com/krikaa/kappV2/tree/master/Enclosures) folder. 
NB! Be aware that many pieces require to be out of metal, or they will break!
* Put the module together by the instructions in the [documentation](https://github.com/krikaa/kappV2/blob/master/Report%20file.pdf)
* Install the module on a locker by the instructions.

## Technologies Used
* C and C++ programming languages
* Firebase cloud service
* NFC reader library
* Visual Studio Code as an integrated development environment (IDE)
* Git for version control

## Project layout/format
The project consists of two submodules:
* door-controller - Contains code for the ESP32 in the locker device, driving both the display and locking mechanism.
* web-interface - Contains code for the web interface, at https://nutikapp-v2.web.app

