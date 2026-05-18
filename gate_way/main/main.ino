#include "pin_config.h"
#include "utils.h"
#include "lora_utils.h"
#include "app_types.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n--- ESP32 Gateway Booting ---");

    if (!init_pins()) {
        Serial.println("[CRITICAL] Pin init failed.");
        while(1) { delay(100); }
    }

    if (!init_lora()) {
        Serial.println("[CRITICAL] LoRa init failed.");
        while(1) { led_blink(1, 100); }
    }

    start_lora_rx();

    // -------------------------------------------------------------------------
    // Initialize WiFi
    // -------------------------------------------------------------------------
    if (!wifi_init()) {
        Serial.println("[CRITICAL] WiFi initialization failed.");
    }

    // -------------------------------------------------------------------------
    // Initialize MQTT (THIS WAS MISSING!)
    // -------------------------------------------------------------------------
    if (!mqtt_init()) {
        Serial.println("[CRITICAL] MQTT initialization failed.");
    }

    // -------------------------------------------------------------------------
    // Create Tasks
    // -------------------------------------------------------------------------
    xTaskCreatePinnedToCore(wifi_monitor_task, "WiFi Monitor Task", 4096, NULL, 1, NULL, 0);
    
    // Kept at 8192 for TLS!
    xTaskCreatePinnedToCore(mqtt_monitor_task, "MQTT Monitor Task", 8192, NULL, 1, NULL, 1); 

    Serial.println("[INFO] Boot sequence complete.\n");
}

void loop() {
    // 1. Service the LoRa radio
    process_lora_interrupt();

    // 2. Process data from the FreeRTOS Queue
    lora_packet_t pending_packet;
    
    if (xQueueReceive(sensorDataQueue, &pending_packet, 0) == pdPASS) {
        
        Serial.println("\n--- New Data Pulled from Queue ---");
        Serial.printf("Device ID    : %u\n", pending_packet.device_id);
        Serial.printf("Sensor Type  : 0x%02X\n", pending_packet.sensor_type);
        Serial.printf("Payload Size : %d bytes\n", pending_packet.payload_length);
        
        // 3. Route the data based on Sensor Type
        switch(pending_packet.sensor_type) {
            
            case SENSOR_TYPE_SOIL: {
                // Security check: ensure the payload length matches our expected struct size
                if (pending_packet.payload_length == sizeof(soil_payload_t)) {
                    
                    // Cast the raw payload buffer to our Soil struct
                    soil_payload_t *soil_data = (soil_payload_t *)pending_packet.payload;
                    
                    // Decode Modbus data (x10)
                    float temp_c = soil_data->temperature / 10.0;
                    float hum_pct = soil_data->humidity / 10.0;
                    uint16_t ec_val = soil_data->ec;

                    Serial.printf("Data [SOIL]  : Temp: %.1f C | Hum: %.1f %% | EC: %u uS/cm\n", 
                                  temp_c, hum_pct, ec_val);
                                  
                    // FUTURE: Attach local timestamp here, build JSON, send via WiFi
                } else {
                    Serial.println("[ERR] Payload size mismatch for SOIL sensor!");
                }
                break;
            }

            case SENSOR_TYPE_ENV: {
                Serial.println("[INFO] Environmental sensor data received (Not yet implemented).");
                break;
            }
            
            default:
                Serial.printf("[ERR] Unknown sensor type received: 0x%02X\n", pending_packet.sensor_type);
                break;
        }
        Serial.println("----------------------------------");
    }
}