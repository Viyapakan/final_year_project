#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

// Using constexpr instead of #define provides type safety
// GPIO 2 is the standard built-in blue LED on most ESP32 Dev Modules
constexpr uint8_t BUILTIN_LED_PIN = 2;

// Pin initialization function
inline bool init_pins() {
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW); // Default to OFF
    
    // Read back to verify initialization
    return digitalRead(BUILTIN_LED_PIN) == LOW;
}

#endif // PIN_CONFIG_H