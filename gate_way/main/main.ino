#include "app_types.h"
#include "json_builder.h"
#include "lora_utils.h"
#include "mqtt_manager.h"
#include "pin_config.h"
#include "utils.h"
#include "wifi_manager.h"

// [FIX] Hardware watchdog: auto-reboots the ESP32 if any watched task
// becomes permanently blocked (e.g. stuck in a dead TLS socket write).
#include <esp_task_wdt.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n--- ESP32 Gateway Booting ---");

  // [FIX] Initialise the hardware Task Watchdog Timer (60 second timeout).
  // If loop() or any watched task stops calling esp_task_wdt_reset() for
  // 60 consecutive seconds, the ESP32 will panic and reboot automatically.
  // This is the final safety net — prevents silent multi-hour hangs even if
  // an unforeseen edge case bypasses all the other fixes.
  //
  // NOTE: ESP32 Arduino core 3.x (ESP-IDF 5.x) changed esp_task_wdt_init()
  // from esp_task_wdt_init(uint32_t, bool) to esp_task_wdt_init(const
  // esp_task_wdt_config_t*). We use the struct form here so the code compiles
  // on both cores correctly.
  const esp_task_wdt_config_t wdt_cfg = {
      .timeout_ms = 60000, // 60 seconds in milliseconds
      .idle_core_mask = 0, // Do NOT watch idle tasks (only our registered task)
      .trigger_panic = true // Trigger panic (reboot) when timeout expires
  };
  esp_task_wdt_init(&wdt_cfg);
  esp_task_wdt_add(NULL); // Watch the current task (loop / Arduino task)

  if (!init_pins()) {
    Serial.println("[CRITICAL] Pin init failed.");
    while (1) {
      delay(100);
    }
  }

  if (!init_lora()) {
    Serial.println("[CRITICAL] LoRa init failed.");
    while (1) {
      led_blink(1, 100);
    }
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
  xTaskCreatePinnedToCore(wifi_monitor_task, "WiFi Monitor Task", 4096, NULL, 1,
                          NULL, 0);
  // xTaskCreatePinnedToCore(mqtt_monitor_task, "MQTT Monitor Task", 8192, NULL,
  // 1, NULL, 1);
  xTaskCreatePinnedToCore(mqtt_monitor_task, "MQTT Monitor Task", 12288, NULL,
                          1, NULL, 1);
  // [FIX] Stack doubled 4096->8192: mqtt_message_t (576 bytes) + deep TLS
  // call stack inside WiFiClientSecure::write() overflowed the 4KB stack,
  // silently corrupting adjacent memory and causing unpredictable hangs.
  xTaskCreatePinnedToCore(mqtt_publish_task, "MQTT Publish Task", 8192, NULL, 2,
                          NULL, 1);

  Serial.println("[INFO] Boot sequence complete.\n");
}

// void loop() {
//     process_lora_interrupt();

//     lora_packet_t pending_packet;

//     if (xQueueReceive(sensorDataQueue, &pending_packet, 0) == pdPASS) {

//         Serial.println("\n--- New Data Pulled from Queue ---");

//         String json_payload = build_sensor_json(&pending_packet);

//         if (json_payload.length() > 0) {

//             String topic = "gateway/" + String(pending_packet.device_id);

//             // ---------------------------------------------------------
//             // NEW: Package the string into our struct and send to Queue
//             // ---------------------------------------------------------
//             mqtt_message_t out_msg;

//             // Safely copy the strings into the character arrays
//             strncpy(out_msg.topic, topic.c_str(), MQTT_MAX_TOPIC_LEN - 1);
//             out_msg.topic[MQTT_MAX_TOPIC_LEN - 1] = '\0'; // Ensure null
//             termination

//             strncpy(out_msg.payload, json_payload.c_str(),
//             MQTT_MAX_PAYLOAD_LEN - 1); out_msg.payload[MQTT_MAX_PAYLOAD_LEN -
//             1] = '\0'; // Ensure null termination

//             // Push to the Outbound Queue (0 delay means don't block if queue
//             is full) if (xQueueSend(mqttPublishQueue, &out_msg, 0) == pdPASS)
//             {
//                 Serial.println("[MAIN] Dispatched to MQTT Task");
//             } else {
//                 Serial.println("[MAIN-ERR] Outbound MQTT Queue is FULL!");
//             }
//             // ---------------------------------------------------------

//         } else {
//             Serial.println("[ERR] Dropped packet: Unknown type or size
//             mismatch.");
//         }

//         Serial.println("----------------------------------");
//     }
// }

void loop() {
  // Feed the hardware watchdog so it knows this task is still alive.
  esp_task_wdt_reset();

  process_lora_interrupt();

  lora_packet_t pending_packet;

  if (xQueueReceive(sensorDataQueue, &pending_packet, 0) == pdPASS) {

    Serial.println("\n--- New Data Pulled from Queue ---");

    // [FIX] Use the buffer-based JSON builder to write directly into
    // the mqtt_message_t struct. This replaces 4-5 heap String allocations
    // per packet cycle, preventing long-term heap fragmentation that would
    // cause malloc() failures after hundreds of 3-minute sensor cycles.
    mqtt_message_t out_msg;
    memset(&out_msg, 0, sizeof(out_msg));

    // Build topic directly into the fixed char array — no String object
    snprintf(out_msg.topic, MQTT_MAX_TOPIC_LEN, "gateway/%lu",
             (unsigned long)pending_packet.device_id);

    // Build JSON payload directly into the fixed char array — no heap
    bool json_ok = build_sensor_json_to_buf(&pending_packet, out_msg.payload,
                                            MQTT_MAX_PAYLOAD_LEN);

    if (json_ok) {
      if (xQueueSend(mqttPublishQueue, &out_msg, 0) == pdPASS) {
        Serial.println("[MAIN] Dispatched to MQTT Task");
      } else {
        Serial.println("[MAIN-ERR] Outbound MQTT Queue is FULL!");
      }
    } else {
      Serial.println("[ERR] Dropped packet: Unknown type or size mismatch.");
    }

    Serial.println("----------------------------------");
  }

  // Yield to the FreeRTOS scheduler
  vTaskDelay(pdMS_TO_TICKS(10));
}