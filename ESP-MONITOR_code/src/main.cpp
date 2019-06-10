#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <Wire.h>

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <PubSubClient.h>
#include <INA219.h>

#include "setup/webpage.h"
#include "setup/currentmonitor.h"
#include "display/display.h"
#include "Ticker.h"

#define POWER_CYCLE (15)
#define PUBLISH_LED (12)
#define TEST_LED    (13)
#define BUILT_IN_LED (2)
#define USER_BUTTON (14)

const char *ssid = "ESP-MONITOR";           // AP SSID
const char *password = "ESP-MONITOR";       // AP Password
const char *mqtt_server = "192.168.13.130"; // IP of MQTT broker

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET (16) // Reset pin for OLED, if not used set it to -1
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

// WiFi globals
ESP8266WebServer server(80); // Specify port
WiFiClient client;
String Essid = ""; // EEPROM Network SSID
String Epass = ""; // EEPROM Network Password
String sssid = ""; // Read SSID From Web Page
String passs = ""; // Read Password From Web Page

 
// MQTT globals
PubSubClient Client(client);
String clientId = "ESP";            // String needed for callback and reconnect MQTT functions,
                                    // do not change it, otherwise you have to also change Node-Red code
#define SIZE (200)                  // Maximum size of receving buffer for UART
char recvbuffer[SIZE];              // Buffer for receving strings over UART
char mqttId[15];                    // Devices ID that will be copied into published messages
char formatedSerialData[SIZE];
char formatedCurrentData[120];

// INA219 globals
INA219 ina219;
#define CURRENT_ARRAY_SIZE (1000) //Number of samples that are sent over MQTT after being collected
#define VOLTAGE_ARRAY_SIZE (1000)
int currentArray[CURRENT_ARRAY_SIZE];
int voltageArray[VOLTAGE_ARRAY_SIZE];
int sampleNumber = 0;

// Erasing flash memory with USER button
Ticker flashErase;          //Ticker object for timer interrupt
int eraseCounter = 0;
uint8_t eraseFlag = 0;

void debounceUserButton()
{
    if (digitalRead(USER_BUTTON) == HIGH)
       eraseCounter++;
    else
       eraseCounter = 0;
    // When button is pressed for some time, set the eraseFlag, flash will be cleaned
    if(eraseCounter >= 300)
        eraseFlag = 1;
    else
        eraseFlag = 0;
}

// Function that is called every time when send message from Node-Red
void callback(char *topic, byte *payload, int length)
{
    // Function checks the topic of received topic and toggles power cycle pin depending on the message
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    if (strcmp(topic, "PowerCycle") == 0)
    {
        // Switch on the LED if an 1 was received as first character
        if ((char)payload[0] == '1')
        {
            digitalWrite(POWER_CYCLE, HIGH);
            digitalWrite(BUILT_IN_LED, LOW);
        }
        else
        {
            digitalWrite(POWER_CYCLE, LOW); 
            digitalWrite(BUILT_IN_LED, HIGH);
        }
    }
}

// Reconnect function for MQTT connection
void reconnect(String clientId)
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        // Attempt to connect
        Serial.print("Attempting MQTT connection...");
        if (Client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // Once connected, publish an announcement...
            Client.publish("Serial", "hello world");
            // ... and resubscribe
            Client.subscribe("PowerCycle");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(Client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

// Function fills recvbuffer with received UART data
int receivedUartData()
{
    if (Serial.available() == 0)
        return 0;   // Buffer is empty, exit function
    else
    {
        while (Serial.available() > 0)
        {
            Serial.readBytesUntil('\r', recvbuffer, SIZE);  //Should be \r\n, with Arduino's Serial.println() function should work fine
        }
        
        //Serial.print(recvbuffer);

        // Flush buffer
        while (Serial.available() > 0)
        {
            Serial.read();
        }
        return 1;   // Buffer has some data, exit function
    }
}

void setup()
{
    Serial.begin(115200);
    EEPROM.begin(512);
    WiFi.mode(WIFI_AP_STA);

    // Setup power cycle pin for INA219, led on esp module will serve as indicator
    pinMode(POWER_CYCLE, OUTPUT);
    digitalWrite(POWER_CYCLE, HIGH);
    pinMode(BUILT_IN_LED, OUTPUT);
    digitalWrite(BUILT_IN_LED, LOW);

    //Setup led for indicating publiching of current data over MQTT
    pinMode(PUBLISH_LED, OUTPUT);
    digitalWrite(PUBLISH_LED, LOW);

    //Test led
    pinMode(TEST_LED, OUTPUT);
    digitalWrite(TEST_LED, LOW);

    //User button setup for erasing flash
    pinMode(USER_BUTTON, INPUT);
    
    // Setup current monitor INA219
    setupMonitor();

    // Generate mqtt client name based of last 24 bits of MAC address
    // Copy mac address to char array mqttID for recognizing data sent from different devices
    clientId += WiFi.macAddress();
    clientId.remove(4, 9);
    clientId.toCharArray(mqttId, 15);

    // Initialize connection with OLED display
    if (!initDisplay())
    {
        Serial.println(F("SSD1306 allocation failed"));
        while (1);      // Don't proceed, loop forever
            
    }

    // Greet user on display
    greetScreenDisplay();

    // Check EEPROM for past configuration
    checkingEepromDisplay();

    for (int i = 0; i < 32; ++i) //Reading SSID
    {
        Essid += char(EEPROM.read(i));
    }
    for (int i = 32; i < 96; ++i) //Reading Password
    {
        Epass += char(EEPROM.read(i));
    }

    // Check if old network is in flash
    if (EEPROM.read(0) == 0xFF)
    {
        // No old network found, initialize web server
        initWebServer(ssid, password);

        // Display instructions for connection
        instructConnDisplay();

        while (1)
        {
            // Loop here to handle web requests, user should reset ESP
            server.handleClient();
        }
    }
    else
    {
        // Old network found
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Network found:"));
        display.println(Essid);
        display.display();
        delay(1000);

        WiFi.begin(Essid.c_str(), Epass.c_str());
        display.print(F("Connecting"));
        display.display();
        delay(1000);

        // Trying to connect to it
        uint8_t attempt = 0;
        while (WiFi.status() != WL_CONNECTED)
        {
            display.print(F("."));
            display.display();
            delay(1000);

            if (attempt >= 10) // Timeout in 10 seconds
            {
                // Connection timeout reached, setup web server, user can change network or try again with reset
                WiFi.disconnect();
                delay(300);
                initWebServer(ssid, password);
                connTimeoutDisplay();

                while (1)
                {
                    // Loop here to handle web requests, user should reset ESP
                    server.handleClient();
                }
            }
            attempt += 1;
        }
        // Connected
        connectedDisplay(mqttId);

        // Connect to broker
        Client.setServer(mqtt_server, 1883);
        Client.setCallback(callback);
        flashErase.attach(0.01, debounceUserButton);    // Setup and run timer interrupt 
    }
}

void loop()
{
    if (!Client.connected())
        reconnect(clientId);

    if (!Client.loop())
        Client.connect(clientId.c_str());

    if (receivedUartData())
    {
        //When we receive data over UART we send it to MQTT broker
        display.clearDisplay();
        display.setCursor(0, 0);
        display.println(F("Last input received: "));
        display.println(recvbuffer);
        display.display();

        //Serial.print(recvbuffer);

        // Conectate device id with serial input
        strcpy(formatedSerialData, mqttId);
        strcat(formatedSerialData, " & ");
        strcat(formatedSerialData, recvbuffer);

        // Publish message to topic Serial
        Client.publish("Serial", formatedSerialData);
        memset(recvbuffer, 0, SIZE);
    }

    if (sampleNumber == CURRENT_ARRAY_SIZE)
    {
        /* When we fill our array with samples we calculate the max, min, avg and std values
        and we send them over MQTT. In current implementation there is no delay in main loop, 
        as we already need more than a second to collect all samples, get the values and send them over MQTT.
        If this rate of sending messages needs to be changed for any reason, we suggest adjust the size of 
        currentArray with macro CURRENT_ARRAY_SIZE and adding a delay at the end of main loop. */

        // Calculate max, min, avg, std values and save them to struct to struct
        struct mqttData currentData = updateData(currentArray, CURRENT_ARRAY_SIZE, CURRENT_DATA);
        struct mqttData voltageData = updateData(voltageArray, VOLTAGE_ARRAY_SIZE, VOLTAGE_DATA);

        // Conectate device id with current data, do not change this line, othwerwise you will have to fix parsing in Node-Red
        snprintf(formatedCurrentData, 120, "%s & %0.1f, %0.1f, %0.1f, %0.1f, %0.1f, %0.1f, %0.1f, %0.1f", mqttId, 
        currentData.maxValue, currentData.minValue, currentData.meanValue, currentData.stdValue,
        voltageData.maxValue, voltageData.minValue, voltageData.meanValue, voltageData.stdValue);

        // Publish message to topic Current
        Client.publish("CurrentVoltage", formatedCurrentData);

        // Flash led
        digitalWrite(PUBLISH_LED, HIGH);
        delay(10);
        digitalWrite(PUBLISH_LED, LOW);
        
        sampleNumber = 0;
    }
    else
    {
        currentArray[sampleNumber] = (int) (ina219.shuntCurrent() * 10000);     // We multiply with 1000 to get from A to mA,
                                                                                // and by another 10 to get rid of the floating point
                                                                                // so that currentArray can be int and not float

        voltageArray[sampleNumber] = (int) (ina219.busVoltage()*100);           // We multiply with 100 to get rid of the floating point
                                                                                // so that voltageArray can be int and not float
        sampleNumber++;
    }

    // In case that User button is being pressed for longer time the flash gets erased
    if (eraseFlag == 1)
    {
        flashErasedDisplay();
        // Erase Flash!
        EEPROM.write(0, 0xFF);
        EEPROM.end();
        while (1)
        {
        }
    }
}
