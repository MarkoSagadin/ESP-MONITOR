#include "setup/currentmonitor.h"

void setupMonitor()
{
    // Default address 0x40
    ina219.begin();
    ina219.configure(INA219::RANGE_32V, INA219::GAIN_8_320MV, INA219::ADC_12BIT, INA219::ADC_12BIT, INA219::CONT_SH_BUS);
    ina219.calibrate(R_SHUNT, V_SHUNT_MAX, V_BUS_MAX, I_MAX_EXPECTED);
}

float max(int samples[], int size, dataINA select)
{
    int max = samples[0];

    for (int i = 0; i < size - 1; i++)
        if (max < samples[i + 1])
            max = samples[i + 1];

    if (select == VOLTAGE_DATA)
        return (float)max / 100.0f; // Divide by 100 to get correct voltage value
    else
        return (float)max / 10.0f; // Divide by 10 to get correct current value
}

float min(int samples[], int size, dataINA select)
{
    int min = samples[0];

    for (int i = 0; i < size - 1; i++)
        if (min > samples[i + 1])
            min = samples[i + 1];

    if (select == VOLTAGE_DATA)
        return (float)min / 100.0f; // Divide by 100 to get correct voltage value
    else
        return (float)min / 10.0f; // Divide by 10 to get correct current value
}

float mean(int samples[], int size, dataINA select)
{
    float mean = 0;

    for (int i = 0; i < size; i++)
        mean += samples[i];

    mean /= size;

    if(select == VOLTAGE_DATA)
        return (float)mean / 100.0f; // Divide by 100 to get correct voltage value
    else
        return (float)mean / 10.0f; // Divide by 10 to get correct current value
}

// Calculation of standard deviation from array
float stdValue(int samples[], float mean, int size, dataINA select)
{
    float std = 0.0;

    /* Both implementations of standard deviation my look wierd,
    but they have been optimized for speed, othwerwise each sample 
    from voltageArray and currentArry would have to be divided by 
    100 and 10 respectively to compensate for float to int conversion 
    which would result in slow execution of equation. */

    if (select == VOLTAGE_DATA)
    {
        for (int i = 0; i < size; ++i)
            std += (samples[i] - 100 * mean) * (samples[i] - 100 * mean);

        return sqrt(std / (1000 * (size - 1))); 
    }
    else
    {
        for (int i = 0; i < size; ++i)
            std += (samples[i] - 10 * mean) * (samples[i] - 10 * mean);

        return sqrt(std / (100 * (size - 1))); 
    }
}

mqttData updateData(int samples[], int size, dataINA select)
{
    struct mqttData data;
    data.maxValue = max(samples, size, select);
    data.minValue = min(samples, size, select);
    data.meanValue = mean(samples, size, select);
    data.stdValue = stdValue(samples, data.meanValue, size, select);

    return data;
}