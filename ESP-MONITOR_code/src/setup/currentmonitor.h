#pragma once

#include <Wire.h>
#include <Arduino.h>
#include <INA219.h>
#include <math.h>

// Here you can change parameters for current monitor INA219 
#define R_SHUNT (0.1f)        // Shunt resistor in omhs
#define V_SHUNT_MAX (0.320f)  // Max voltage drop over shunt resistor in volts
#define V_BUS_MAX (32.0f)     // Max bus voltage in volts
#define I_MAX_EXPECTED (1.5f) // Max expected current through shunt resistor in ampers

extern INA219 ina219;

// Struct needed to export data in string that will be send over mqtt
struct mqttData
{
  float minValue;
  float maxValue;
  float meanValue;
  float stdValue;
};

// Needed to differentiate between different equations used for calculating data
typedef enum {
  VOLTAGE_DATA = 0,
  CURRENT_DATA = 1,
}dataINA;

// Setup function, has to be called first as it configures and calibrates INA219. 
void setupMonitor();

// Helper functions to calculate values 
float max(int samples[], int size, dataINA select);
float min(int samples[], int size,  dataINA select);
float mean(int samples[], int size, dataINA select);
float stdValue(int samples[], float mean, int size, dataINA select);

// Main function that calls helper functions and uses correct version of them 
mqttData updateData(int samples[], int size, dataINA select);