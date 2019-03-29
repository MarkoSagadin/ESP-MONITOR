#include "display.h"

int initDisplay()
{
    if(display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    {
        display.clearDisplay();         //Clear buffer from Adafruit logo
        return 1;
    }
    else
        return 0;
}

void greetScreenDisplay()
{
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.println(F("    ESP"));
    display.println(F("  MONITOR"));
    display.display();
    delay(2000); // Pause for 2 seconds
}

void checkingEepromDisplay()
{
    display.clearDisplay();                        
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(F("Checking for old"));
    display.print(F("network in memory"));
    display.display();
    delay(1000);
    for(int i = 0; i<3; i++)
    {
        display.print(F("."));
        display.display();
        delay(1000);
    }
}

void instructConnDisplay()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("No old network found!"));
    display.println(" ");
    display.println("Connect to");
    display.println(ssid);
    display.println("to setup wifi!");
    display.println("Pass is the same.");
    display.println("IP is: ");
    display.print(WiFi.softAPIP());
    display.display();
}

void connTimeoutDisplay()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Connection timeout!"));
    display.println(F(" "));
    display.println(F("Either reset module"));
    display.println(F("or connect to:"));
    display.println(ssid);
    display.println("to setup wifi!");
    display.println("IP is: ");
    display.print(WiFi.softAPIP());
    display.display();
}

void connectedDisplay(char * mqttId)
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("Connected!"));
    display.println(F(" "));
    display.println(F("IP: "));
    display.println(WiFi.localIP()); // Display ESP8266 IP Address
    display.println(F(" "));
    display.println(F("My DeviceID tag in"));
    display.println(F("Grafana is: "));
    display.print(mqttId);
    display.display();
    delay(3000);
}
void flashErasedDisplay()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(F("Flash has been "));
    display.println(F("deleted!"));
    display.println(F(""));
    display.println(F("Reset module to"));
    display.println(F("start again."));
    display.display();
}