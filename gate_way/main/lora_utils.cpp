#include "lora_utils.h"

// Define the FreeRTOS Queue
QueueHandle_t sensorDataQueue;

// Volatile flag for the ISR to communicate with the main application
volatile bool lora_packet_ready = false;

// The Interrupt Service Routine (ISR)
// IRAM_ATTR forces this function into fast RAM instead of Flash memory
void IRAM_ATTR lora_isr() {
    lora_packet_ready = true;
}


bool init_lora() {
    // 1. Configure custom SPI pins
    SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_NSS);
    
    // 2. Set control pins
    LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
    
    // 3. Initialize hardware at target frequency

    // 4. Apply custom radio configurations
    LoRa.setSpreadingFactor(LORA_SPREAD_FACTOR);
    LoRa.setSignalBandwidth(LORA_BANDWIDTH);
    LoRa.setCodingRate4(LORA_CODING_RATE); 
    LoRa.setTxPower(LORA_TX_POWER);        // The library defaults to PA_BOOST, suitable for 17dBm
    LoRa.setOCP(LORA_OCP);
    LoRa.setPreambleLength(LORA_PREAMBLE_LEN);
    // LoRa.setSyncWord(LORA_SYNC_WORD);
    
    if (!LoRa.begin(LORA_FREQUENCY)) {
        Serial.println("[ERR] LoRa Hardware Error: Could not communicate with SX1278.");
        return false;
    }
    
    
    
    Serial.println("[INFO] LoRa parameters (SF7, 125kHz, CR4/5, 17dBm) applied successfully.");
    
    return true;
}

void debug_lora_registers() {
    Serial.println("\n--- SX1278 Hardware Register Dump ---");
    // This library function reads every register via SPI and prints the Hex values
    LoRa.dumpRegisters(Serial);
    Serial.println("-------------------------------------\n");
}


// [FIX] SPI mutex definition — guards all LoRa SPI transactions.
SemaphoreHandle_t lora_spi_mutex = NULL;

void start_lora_rx() {
    // 1. Update queue to hold the new Envelope struct (54 bytes max per message)
    sensorDataQueue = xQueueCreate(20, sizeof(lora_packet_t));
    if (sensorDataQueue == NULL) {
        Serial.println("[CRITICAL] Failed to create FreeRTOS Sensor Data Queue!");
        return;
    }

    // [FIX] Create the SPI bus mutex to prevent concurrent SPI access
    // corrupting the SX1278 state machine (root cause of "Dropped packet" errors).
    lora_spi_mutex = xSemaphoreCreateMutex();
    if (lora_spi_mutex == NULL) {
        Serial.println("[CRITICAL] Failed to create LoRa SPI mutex!");
        return;
    }

    pinMode(LORA_DIO0, INPUT);
    attachInterrupt(digitalPinToInterrupt(LORA_DIO0), lora_isr, RISING);

    LoRa.receive();
    Serial.println("[INFO] LoRa RX Interrupt Mode Started. Listening for dynamic packets...");
}

// void process_lora_interrupt() {
//     if (!lora_packet_ready) return;
//     lora_packet_ready = false;

//     int packetSize = LoRa.parsePacket();
    
//     // We calculate the exact header size dynamically (should be 6 bytes)
//     int header_size = offsetof(lora_packet_t, payload);

//     // Ensure the packet is at least large enough to contain the Envelope header
//     if (packetSize >= header_size) {
//         lora_packet_t rx_packet;
//         memset(&rx_packet, 0, sizeof(lora_packet_t)); // Zero out memory first
        
//         // Read exact bytes received into our Envelope struct
//         int bytesToRead = (packetSize > sizeof(lora_packet_t)) ? sizeof(lora_packet_t) : packetSize;
//         LoRa.readBytes((uint8_t*)&rx_packet, bytesToRead);
        
//         int packet_rssi = LoRa.packetRssi();

//         // Push the struct into the FreeRTOS queue
//         if (xQueueSend(sensorDataQueue, &rx_packet, 0) != pdPASS) {
//             Serial.println("[WARN] Data Queue Full! Dropping incoming LoRa packet.");
//         } else {
//             // Optional debug check to verify length math
//             if (packetSize != (header_size + rx_packet.payload_length)) {
//                 Serial.printf("[WARN] Size mismatch. Received %d bytes, but Header + Payload length = %d\n", 
//                               packetSize, (header_size + rx_packet.payload_length));
//             }
//             // Serial.printf("[RX] Packet queued. RSSI: %d dBm\n", packet_rssi);
//         }
        
//     } else if (packetSize > 0) {
//         Serial.printf("[WARN] Rogue packet detected. Size: %d bytes (Too small for header)\n", packetSize);
//         while (LoRa.available()) { LoRa.read(); }
//     }

//     LoRa.receive();
// }



void process_lora_interrupt() {
    // [FIX] Watchdog: track the last time we had any LoRa radio activity.
    // If DIO0 never fires for 10 minutes, the SX1278 has likely frozen in
    // standby mode (happens after a corrupted receive). Re-arm it.
    static uint32_t last_lora_activity_ms = 0;

    // Failsafe: If the pin is physically stuck HIGH but our flag was missed, force a read.
    if (!lora_packet_ready) {
        if (digitalRead(LORA_DIO0) == HIGH) {
            lora_packet_ready = true;
        } else {
            // [FIX] RX Watchdog check (only when radio is idle / no interrupt pending)
            if ((millis() - last_lora_activity_ms) > 600000UL) { // 10 minutes
                Serial.println("[WARN] LoRa RX watchdog: No activity for 10min. Re-arming receiver.");
                LoRa.receive();
                last_lora_activity_ms = millis();
            }
            return;
        }
    }

    lora_packet_ready = false;
    last_lora_activity_ms = millis(); // Reset watchdog on every real interrupt

    // [FIX] Guard the entire SPI transaction with the mutex.
    // A 100ms timeout prevents deadlocks; if we can't get the bus, we skip
    // this interrupt — the watchdog will recover if this happens repeatedly.
    if (xSemaphoreTake(lora_spi_mutex, pdMS_TO_TICKS(100)) != pdTRUE) {
        Serial.println("[WARN] LoRa SPI mutex timeout. Skipping this interrupt.");
        LoRa.receive(); // Still re-arm the radio
        return;
    }

    int packetSize = LoRa.parsePacket();
    int header_size = offsetof(lora_packet_t, payload);

    if (packetSize >= header_size) {
        lora_packet_t rx_packet;
        memset(&rx_packet, 0, sizeof(lora_packet_t)); 
        
        int bytesToRead = (packetSize > sizeof(lora_packet_t)) ? sizeof(lora_packet_t) : packetSize;
        LoRa.readBytes((uint8_t*)&rx_packet, bytesToRead);

        if (xQueueSend(sensorDataQueue, &rx_packet, 0) != pdPASS) {
            Serial.println("[WARN] Data Queue Full! Dropping incoming LoRa packet.");
        }
        
    } else if (packetSize > 0) {
        Serial.printf("[WARN] Rogue packet detected. Size: %d bytes\n", packetSize);
    }

    // CRITICAL FIX: Forcibly empty the hardware FIFO to guarantee DIO0 goes LOW.
    while (LoRa.available()) { 
        LoRa.read(); 
    }

    // Reset the radio state machine back to continuous listening mode
    LoRa.receive();

    xSemaphoreGive(lora_spi_mutex);
}