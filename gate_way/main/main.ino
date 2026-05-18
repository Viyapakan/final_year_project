#include "pin_config.h"
#include "utils.h"
#include "lora_utils.h"
#include "app_types.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "json_builder.h"


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
    xTaskCreatePinnedToCore(mqtt_monitor_task, "MQTT Monitor Task", 8192, NULL, 1, NULL, 1); 
    xTaskCreatePinnedToCore(mqtt_publish_task, "MQTT Publish Task", 4096, NULL, 2, NULL, 1);

    Serial.println("[INFO] Boot sequence complete.\n");
}

// void loop() {
//     // 1. Service the LoRa radio
//     process_lora_interrupt();

//     // 2. Process data from the FreeRTOS Queue
//     lora_packet_t pending_packet;
    
//     if (xQueueReceive(sensorDataQueue, &pending_packet, 0) == pdPASS) {
        
//         Serial.println("\n--- New Data Pulled from Queue ---");
        
//         // 3. Generate JSON automatically based on the packet envelope
//         // This single line replaces your entire switch() statement!
//         String json_payload = build_sensor_json(&pending_packet);
        
//         // 4. Publish if the JSON was built successfully (not empty)
//         if (json_payload.length() > 0) {
            
//             // Generalized topic structure
//             String topic = "gateway/" + String(pending_packet.device_id);
            
//             Serial.print("[PUBLISH] Topic: ");
//             Serial.println(topic);
            
//             // THIS is where you verify your decoded data!
//             Serial.print("[PUBLISH] Payload: ");
//             Serial.println(json_payload);

//             if (mqtt_publish(topic.c_str(), json_payload.c_str())) {
//                 Serial.println("[PUBLISH] Status: SUCCESS");
//             } else {
//                 Serial.println("[PUBLISH] Status: FAILED (Check connection)");
//             }
//         } else {
//             Serial.println("[ERR] Dropped packet: Unknown type or size mismatch.");
//         }
        
//         Serial.println("----------------------------------");
//     }
// }

void loop() {
    process_lora_interrupt();

    lora_packet_t pending_packet;
    
    if (xQueueReceive(sensorDataQueue, &pending_packet, 0) == pdPASS) {
        
        Serial.println("\n--- New Data Pulled from Queue ---");
        
        String json_payload = build_sensor_json(&pending_packet);
        
        if (json_payload.length() > 0) {
            
            String topic = "gateway/" + String(pending_packet.device_id);
            
            // ---------------------------------------------------------
            // NEW: Package the string into our struct and send to Queue
            // ---------------------------------------------------------
            mqtt_message_t out_msg;
            
            // Safely copy the strings into the character arrays
            strncpy(out_msg.topic, topic.c_str(), MQTT_MAX_TOPIC_LEN - 1);
            out_msg.topic[MQTT_MAX_TOPIC_LEN - 1] = '\0'; // Ensure null termination
            
            strncpy(out_msg.payload, json_payload.c_str(), MQTT_MAX_PAYLOAD_LEN - 1);
            out_msg.payload[MQTT_MAX_PAYLOAD_LEN - 1] = '\0'; // Ensure null termination

            // Push to the Outbound Queue (0 delay means don't block if queue is full)
            if (xQueueSend(mqttPublishQueue, &out_msg, 0) == pdPASS) {
                Serial.println("[MAIN] Dispatched to MQTT Task");
            } else {
                Serial.println("[MAIN-ERR] Outbound MQTT Queue is FULL!");
            }
            // ---------------------------------------------------------

        } else {
            Serial.println("[ERR] Dropped packet: Unknown type or size mismatch.");
        }
        
        Serial.println("----------------------------------");
    }
}