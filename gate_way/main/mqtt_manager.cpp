#include "mqtt_manager.h"
#include "app_types.h"
#include "mqtt_config.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>

// -----------------------------------------------------------------------------
// Global Instances
// -----------------------------------------------------------------------------
WiFiClientSecure secure_client;
PubSubClient mqtt_client(secure_client);

// Create the global queue handle
QueueHandle_t mqttPublishQueue;

// -----------------------------------------------------------------------------
// Initialize MQTT
// -----------------------------------------------------------------------------
// bool mqtt_init()
// {
//     Serial.println("[MQTT] Initializing Secure MQTT Client...");

//     // secure_client.setInsecure(); // Keep this on for now to rule out
//     certificates secure_client.setCACert(ca_cert);
//     // Increase timeouts for Cellular/4G networks
//     secure_client.setTimeout(15000);

//     mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);

//     // Increase KeepAlive to prevent dropping over mobile networks
//     mqtt_client.setKeepAlive(60);

//     mqtt_client.setBufferSize(512);

//     return true;
// }
bool mqtt_init() {
  Serial.println("[MQTT] Initializing Secure MQTT Client...");

  // Create the Outbound Queue (Holds up to 10 messages at a time)
  mqttPublishQueue = xQueueCreate(10, sizeof(mqtt_message_t));

  secure_client.setCACert(ca_cert);
  secure_client.setTimeout(15000);
  mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt_client.setKeepAlive(
      30); // [FIX-2] Halved from 60s: faster broker-side disconnect detection
  mqtt_client.setBufferSize(512);

  return true;
}

// -----------------------------------------------------------------------------
// Helper: Attempt Connection
// -----------------------------------------------------------------------------
bool connect_to_broker() {
  // NEW FIX: Manual DNS Resolution Check
  Serial.print("[MQTT] Resolving DNS for ");
  Serial.print(MQTT_BROKER);
  Serial.print("... ");

  IPAddress broker_ip;
  if (WiFi.hostByName(MQTT_BROKER, broker_ip)) {
    Serial.print("Found IP: ");
    Serial.println(broker_ip);
  } else {
    Serial.println("[FAILED] Could not resolve hostname.");
    return false; // Abort connection attempt if DNS fails
  }

  Serial.print("[MQTT] Attempting connection to EMQX Broker...");

  if (mqtt_client.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println(" [SUCCESS]");
    return true;
  } else {
    Serial.print(" [FAILED] State: ");
    Serial.println(mqtt_client.state());
    return false;
  }
}

// -----------------------------------------------------------------------------
// MQTT Publish Wrapper
// -----------------------------------------------------------------------------
bool mqtt_publish(const char *topic, const char *payload) {
  if (mqtt_client.connected()) {
    return mqtt_client.publish(topic, payload);
  }
  Serial.println("[MQTT] Cannot publish, client disconnected.");
  return false;
}

// -----------------------------------------------------------------------------
// MQTT Monitor Task (FreeRTOS)
// -----------------------------------------------------------------------------
void mqtt_monitor_task(void *parameter) {
  while (true) {
    // Only attempt MQTT operations if WiFi is connected
    if (WiFi.status() == WL_CONNECTED) {

      if (!mqtt_client.connected()) {
        Serial.println(
            "[MQTT] Connection lost or not started. Reconnecting...");
        // NEW: Give the ESP32 network stack a brief moment before firing TLS
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (connect_to_broker()) {
          // Optional: Subscribe to topics here if needed later (e.g., Downlinks
          // to LoRa nodes) mqtt_client.subscribe("gateway/downlink");
        }
      } else {
        // Keep the MQTT connection alive and process incoming messages
        mqtt_client.loop();
      }
    }

    // Brief delay to yield to the FreeRTOS scheduler
    // If disconnected, wait longer before retrying to prevent spamming the
    // broker
    if (mqtt_client.connected()) {
      vTaskDelay(pdMS_TO_TICKS(10));
    } else {
      vTaskDelay(pdMS_TO_TICKS(MQTT_RETRY_INTERVAL_MS));
    }
  }
}

// -----------------------------------------------------------------------------
// NEW: Dedicated MQTT Publish Task
// -----------------------------------------------------------------------------
void mqtt_publish_task(void *parameter) {
  mqtt_message_t outgoing_msg;

  while (true) {
    // [FIX-1] Changed portMAX_DELAY -> 30s bounded wait.
    // With portMAX_DELAY the task blocked forever; this allows the watchdog
    // to be fed and prevents a permanent stall if the queue is never filled.
    if (xQueueReceive(mqttPublishQueue, &outgoing_msg, pdMS_TO_TICKS(30000)) ==
        pdPASS) {

      // Only attempt to publish if connected
      if (mqtt_client.connected()) {

        Serial.print("[MQTT-TASK] Publishing to: ");
        Serial.println(outgoing_msg.topic);

        // [FIX-3] Hard-cap the TLS socket write to 5 seconds.
        // Without this, WiFiClientSecure::write() can block INDEFINITELY
        // on a silently-dead TCP/TLS connection, permanently stalling
        // this task and freezing the entire gateway pipeline.
        secure_client.setTimeout(5000);

        if (mqtt_publish(outgoing_msg.topic, outgoing_msg.payload)) {
          Serial.println("[MQTT-TASK] Publish SUCCESS");
        } else {
          Serial.println("[MQTT-TASK] Publish FAILED — forcing reconnect");
          // Force PubSubClient to recognise the dead connection so
          // mqtt_monitor_task will trigger a clean reconnect.
          mqtt_client.disconnect();
        }

      } else {
        // [FIX-4] Re-queue the message instead of silently dropping it.
        // This ensures no sensor reading is lost during a reconnect cycle.
        Serial.println(
            "[MQTT-TASK] Client disconnected. Re-queuing message...");
        xQueueSendToFront(mqttPublishQueue, &outgoing_msg, 0);
        // Give mqtt_monitor_task time to re-establish the connection
        // before we try to dequeue and publish again.
        vTaskDelay(pdMS_TO_TICKS(2000));
      }
    }
    // If the 30s wait timed out with no message, just loop back.
    // This is the normal idle state — no action needed.
  }
}