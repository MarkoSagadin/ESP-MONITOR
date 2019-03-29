#include "webpage.h"

void initWebServer(const char *ssid, const char *password)
{
  WiFi.softAP(ssid, password); // Turn on AP
  delay(10);                   // For stable AP
  server.on("/", servicePage);
  server.on("/a", getRequest);
  server.begin();
  delay(300);
}

void servicePage()
{
  int Tnetwork = 0;
  String st = "", s = "";
  Tnetwork = WiFi.scanNetworks(); //Scan for total networks available
  st = "<ul>";
  for (int i = 0; i < Tnetwork; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += i + 1;
    st += ": ";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ul>";
  s = "\n\r\n<!DOCTYPE HTML>\r\n<html><h1> ESP-MONITOR</h1> ";
  //s += ipStr;
  s += "<p>";
  s += st;
  s += "<form method='get' action='a'><label>SSID: </label><input name='ssid' length=32><label>Password: </label><input name='pass' length=64><input type='submit'></form>";
  s += "</html>\r\n\r\n";

  server.send(200, "text/html", s);
}

void ClearEeprom()
{
  Serial.println("Clearing Eeprom");
  for (int i = 0; i < 96; ++i)
  {
    EEPROM.write(i, 0);
  }
}

void getRequest()
{

  if (server.hasArg("ssid") && server.hasArg("pass"))
  {
    sssid = server.arg("ssid"); //Get SSID
    passs = server.arg("pass"); //Get Password
  }

  if (sssid.length() > 1 && passs.length() > 1)
  {
    ClearEeprom(); //First Clear Eeprom
    delay(10);
    for (int i = 0; i < sssid.length(); ++i)
    {
      EEPROM.write(i, sssid[i]);
    }

    for (int i = 0; i < passs.length(); ++i)
    {
      EEPROM.write(32 + i, passs[i]);
    }
    EEPROM.commit();

    String s = "\r\n\r\n<!DOCTYPE HTML>\r\n<html><h1> ESP-MONITOR</h1> ";
    s += "<p>Password Saved! Reset module to connect to network!</html>\r\n\r\n";
    server.send(200, "text/html", s);
  }
}


