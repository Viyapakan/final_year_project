#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <algorithm> // Needed for sorting

const int LDR_PIN = 34;
const int LM35_PIN = 35;
int get_raw_irradiance() {
    int samples[15]; 
    // Take 15 quick readings
    for(int i = 0; i < 15; i++) {
        samples[i] = analogRead(LDR_PIN);
        delayMicroseconds(50);
    }
    // Sort them to find the median (removes the random 0s and 4095s)
    std::sort(samples, samples + 15);
    return samples[7]; // Return the middle value
}

float get_brightness_percentage() {
    int raw = get_raw_irradiance();
    return ( (float)raw / 4095.0f ) * 100.0f;
}

float get_temperature() {
    int samples[21];
    for(int i = 0; i < 21; i++) {
        samples[i] = analogRead(LM35_PIN);
        delayMicroseconds(100);
    }
    std::sort(samples, samples + 21);
    int raw_median = samples[10];

    // Conversion Logic:
    // 1. Convert ADC to Voltage (assuming 11dB attenuation ~3.3V range)
    // 2. LM35: 10mV = 1°C -> Voltage(V) * 100 = Temperature(°C)
    float voltage = (raw_median / 4095.0) * 3300.0; // Voltage in mV
    float tempC = voltage / 10.0; 
    
    return tempC;
}

#endif