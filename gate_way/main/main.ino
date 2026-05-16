#include "pin_config.h"
#include "utils.h"
#include "lora_utils.h"

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    
    // Give the serial monitor a moment to catch up after reset
    delay(1000); 
    Serial.println("\n--- ESP32 System Booting ---");

    //1.  Initialize hardware pins
    if (init_pins()) {
        Serial.println("[OK] Hardware pins initialized successfully.");
    } else {
        Serial.println("[CRITICAL] Failed to initialize pins! Halting.");
        while(1) { delay(100); } // Halt the system safely
    }


    // 2. Initialize and verify LoRa Hardware
    Serial.println("[INFO] Initializing LoRa module and SPI bus...");
    if (init_lora()) {
        Serial.println("[OK] LoRa SX1278 module connected and configured successfully!");
        debug_lora_registers();
    } else {
        Serial.println("[CRITICAL] LoRa initialization failed! Halting system.");
        while(1) { 
            // Panic loop: Use our debuggable LED function to show a hard fault (rapid blinking)
            led_blink(1, 100); 
        }
    }
}

void loop() {
    Serial.println("[INFO] Executing the loop...");
    delay(5000);
}