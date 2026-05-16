#include "utils.h"

bool led_on() {
    digitalWrite(BUILTIN_LED_PIN, HIGH);
    
    // Hardware verification: read the register to ensure it actually went HIGH
    if (digitalRead(BUILTIN_LED_PIN) == HIGH) {
        return true;
    }
    Serial.println("[ERR] Hardware failure: LED pin did not go HIGH");
    return false;
}

bool led_off() {
    digitalWrite(BUILTIN_LED_PIN, LOW);
    
    // Hardware verification: read the register to ensure it actually went LOW
    if (digitalRead(BUILTIN_LED_PIN) == LOW) {
        return true;
    }
    Serial.println("[ERR] Hardware failure: LED pin did not go LOW");
    return false;
}

bool led_blink(uint16_t how_many_blinks, uint32_t in_which_gap_ms) {
    // Input validation
    if (in_which_gap_ms == 0) {
        Serial.println("[WARN] led_blink: Gap cannot be 0ms");
        return false;
    }

    for (uint16_t i = 0; i < how_many_blinks; i++) {
        if (!led_on()) return false;   // Abort if hardware fails
        delay(in_which_gap_ms);
        
        if (!led_off()) return false;  // Abort if hardware fails
        delay(in_which_gap_ms);
    }
    
    return true; // Successfully completed the sequence
}