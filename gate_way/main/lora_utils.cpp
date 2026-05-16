#include "lora_utils.h"

bool init_lora() {
    // 1. Configure custom SPI pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    
    // 2. Set control pins
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
    
    // 3. Initialize hardware at target frequency
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("[ERR] LoRa Hardware Error: Could not communicate with SX1278.");
        return false;
    }
    
    // 4. Apply custom radio configurations
    LoRa.setSpreadingFactor(LORA_SPREAD_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE); 
    LoRa.setTxPower(LORA_TX_POWER);        // The library defaults to PA_BOOST, suitable for 17dBm
    LoRa.setOCP(LORA_OCP);
    LoRa.setPreambleLength(LORA_PREAMBLE_LEN);
    LoRa.setSyncWord(LORA_SYNC_WORD);
    
    Serial.println("[INFO] LoRa parameters (SF7, 125kHz, CR4/5, 17dBm) applied successfully.");
    
    return true;
}

void debug_lora_registers() {
    Serial.println("\n--- SX1278 Hardware Register Dump ---");
    // This library function reads every register via SPI and prints the Hex values
    LoRa.dumpRegisters(Serial);
    Serial.println("-------------------------------------\n");
}