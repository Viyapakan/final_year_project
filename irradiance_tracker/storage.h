#ifndef STORAGE_H
#define STORAGE_H

#include <SPI.h>
#include <SD.h>

const int SD_CS_PIN = 5;

bool init_sd_card() {
    Serial.print("Initializing SD card...");
    
    if (!SD.begin(SD_CS_PIN)) {
        Serial.println(" ERROR: SD Card Mount Failed!");
        return false;
    }
    
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println(" ERROR: No SD card attached!");
        return false;
    }

    Serial.println(" Success! SD Card Ready.");
    return true;
}

#endif