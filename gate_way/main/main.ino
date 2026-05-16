#include "pin_config.h"
#include "utils.h"

void setup() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    
    // Give the serial monitor a moment to catch up after reset
    delay(1000); 
    Serial.println("\n--- ESP32 System Booting ---");

    // Initialize hardware pins
    if (init_pins()) {
        Serial.println("[OK] Hardware pins initialized successfully.");
    } else {
        Serial.println("[CRITICAL] Failed to initialize pins! Halting.");
        while(1) { delay(100); } // Halt the system safely
    }

    // Run a startup test
    Serial.println("[INFO] Running initial LED tests...");
    
    if (led_on()) {
        Serial.println("[OK] LED ON test passed.");
    }
    delay(1000);
    
    if (led_off()) {
        Serial.println("[OK] LED OFF test passed.");
    }
    delay(1000);
}

void loop() {
    Serial.println("[INFO] Executing blink sequence: 3 blinks, 500ms gap...");
    
    // Test our blink function with debugging
    if (led_blink(3, 500)) {
        Serial.println("[OK] Blink sequence completed.");
    } else {
        Serial.println("[ERR] Blink sequence failed or was interrupted.");
    }

    Serial.println("[INFO] System resting for 3 seconds...\n");
    delay(3000);
}