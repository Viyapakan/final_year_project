#ifndef LORA_UTILS_H
#define LORA_UTILS_H

#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include "pin_config.h"
#include "app_types.h"

// --- LoRa Radio Configurations ---
constexpr long LORA_FREQUENCY      = 433E6; // 433 MHz
constexpr int  LORA_SPREAD_FACTOR  = 7;     // SF_7
constexpr long LORA_BANDWIDTH      = 125E3; // 125 kHz
constexpr int  LORA_CODING_RATE    = 5;     // CR 4/5 (We pass the denominator to the library)
constexpr int  LORA_TX_POWER       = 17;    // 17 dBm
constexpr int  LORA_OCP            = 130;   // 130 mA Over Current Protection
constexpr int  LORA_PREAMBLE_LEN   = 8;     // 8 symbols
constexpr int  LORA_SYNC_WORD      = 0xF3;  // Custom network sync word


// Expose the FreeRTOS Queue handle globally so the Cloud task can read from it
extern QueueHandle_t sensorDataQueue;


// Function prototype
bool init_lora();
void debug_lora_registers();

// Non-blocking RX functions
void start_lora_rx();
void process_lora_interrupt();

#endif // LORA_UTILS_H