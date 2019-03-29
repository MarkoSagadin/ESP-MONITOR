#pragma once
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

extern Adafruit_SSD1306 display;
extern ESP8266WiFiClass WiFi;
extern const char *ssid;

/* The sole purpose of these functions is to break and 
refactor the code in order to keep the main file clean. 
Function deal only with presenting instructions and 
information on oled display to the user, nothing more.*/
int initDisplay(); 
void greetScreenDisplay();
void checkingEepromDisplay();  
void instructConnDisplay();      
void connTimeoutDisplay();
void connectedDisplay(char * mqttId);
void flashErasedDisplay();