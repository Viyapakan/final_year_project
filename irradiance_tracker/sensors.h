#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <algorithm> // Needed for sorting

const int LDR_PIN = 34;

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

#endif