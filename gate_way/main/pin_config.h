#ifndef PIN_CONFIG_H
#define PIN_CONFIG_H

#include <Arduino.h>

// Built-in LED
constexpr uint8_t BUILTIN_LED_PIN = 2;

// --- LoRa SX1278 Pin Configuration ---
constexpr uint8_t LORA_NSS  = 5;   // SPI Chip Select
constexpr uint8_t LORA_MOSI = 23;  // SPI MOSI
constexpr uint8_t LORA_MISO = 19;  // SPI MISO
constexpr uint8_t LORA_SCK  = 18;  // SPI Clock
constexpr uint8_t LORA_RST  = 14;  // Hardware Reset
constexpr uint8_t LORA_DIO0 = 26;  // Hardware Interrupt

// Basic pin initialization function
inline bool init_pins() {
    pinMode(BUILTIN_LED_PIN, OUTPUT);
    digitalWrite(BUILTIN_LED_PIN, LOW); 
    
    return digitalRead(BUILTIN_LED_PIN) == LOW;
}

#endif // PIN_CONFIG_H