# Nutikas laborikapp

TalTech Computers and Systems project 2023 \
Authors:
* Markus Visnapuu 
* Rannar Randmäe 
* Siim Tišler
* Markus Veersoo

## Full documentation (in Estonian)

[IAS1420-Aruanne_Laborikapp.pdf](https://github.com/siimtishler/kapp/files/13920218/IAS1420-Aruanne_Laborikapp.1.pdf) \
In the documentation scroll to bottom for pictures, screenshots and use cases.

## Introduction
This is the repo for course "IAS1420 Arvutite ja süsteemide projekt" at TalTech 2023. 
The goal of the project was to add an electronic locking solution to the existing laboratory cabinet, which could be opened using an NFC card. This involved the task of implementing a convenient system for managing access rights to the cabinet and recording the opening data in a log. To facilitate user management and access to the log, a web interface was created.
This repo is only for more convenient code management and project is intended only as a study project.

## Hardware
![Schematic](https://github.com/siimtishler/kapp/blob/master/Schematic.png?raw=true)

## Installation
To use the developed firmware on the chosen hardware for the cabinet, two ESP8266 devices have to be flashed. The main branch has to be flashed to the main ESP, and the display branch has to be flashed to the display ESP. With the MCUs flashed, the solution can be assembled according to the provided schematic. Files for 3D printable enclosures that were used to mount the chosen hardware components are also accessible in the "Enclosures" folder. More detailed information about the project can be found in the included report file.


## Technologies Used
* C and C++ programming languages
* Firebase cloud service
* NFC reader library
* Visual Studio Code as an integrated development environment (IDE)
* Git for version control

## Project layout/format
The branches all contain separate parts of code that should never be merged together
* Master - Contains code for the Master ESP8266
* Display - Contains code for the Slave ESP2866
* Web-interface - Contains code for the web interface, at https://nutikapp.web.app/login

