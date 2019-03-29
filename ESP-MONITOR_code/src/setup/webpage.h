#pragma once
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <EEPROM.h>

extern ESP8266WebServer server; 
extern WiFiClient client;
extern String Essid;                 //EEPROM Network SSID
extern String Epass;                 //EEPROM Network Password
extern String sssid;                 //Read SSID From Web Page
extern String passs;                 //Read Password From Web Page

void initWebServer(const char *ssid, const char * password);
void servicePage();
void ClearEeprom();
void getRequest();
