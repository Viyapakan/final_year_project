#include "mqtt_manager.h"
#include "mqtt_config.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>

// -----------------------------------------------------------------------------
// Global Instances
// -----------------------------------------------------------------------------
WiFiClientSecure secure_client;
PubSubClient mqtt_client(secure_client);

// -----------------------------------------------------------------------------
// Initialize MQTT
// -----------------------------------------------------------------------------
bool mqtt_init()
{
    Serial.println("[MQTT] Initializing Secure MQTT Client...");

    // secure_client.setInsecure(); // Keep this on for now to rule out certificates
    secure_client.setCACert(ca_cert);
    // Increase timeouts for Cellular/4G networks
    secure_client.setTimeout(15000); 

    mqtt_client.setServer(MQTT_BROKER, MQTT_PORT);
    
    // Increase KeepAlive to prevent dropping over mobile networks
    mqtt_client.setKeepAlive(60); 
    
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
bool mqtt_publish(const char* topic, const char* payload) {
    if (mqtt_client.connected()) {
        return mqtt_client.publish(topic, payload);
    }
    Serial.println("[MQTT] Cannot publish, client disconnected.");
    return false;
}

// -----------------------------------------------------------------------------
// MQTT Monitor Task (FreeRTOS)
// -----------------------------------------------------------------------------
void mqtt_monitor_task(void *parameter)
{
    while (true)
    {
        // Only attempt MQTT operations if WiFi is connected
        if (WiFi.status() == WL_CONNECTED) {
            
            if (!mqtt_client.connected()) {
                Serial.println("[MQTT] Connection lost or not started. Reconnecting...");
                // NEW: Give the ESP32 network stack a brief moment before firing TLS
                vTaskDelay(pdMS_TO_TICKS(1000));
                if (connect_to_broker()) {
                    // Optional: Subscribe to topics here if needed later (e.g., Downlinks to LoRa nodes)
                    // mqtt_client.subscribe("gateway/downlink");
                }
            } else {
                // Keep the MQTT connection alive and process incoming messages
                mqtt_client.loop();
            }
        }
        
        // Brief delay to yield to the FreeRTOS scheduler
        // If disconnected, wait longer before retrying to prevent spamming the broker
        if (mqtt_client.connected()) {
            vTaskDelay(pdMS_TO_TICKS(10)); 
        } else {
            vTaskDelay(pdMS_TO_TICKS(MQTT_RETRY_INTERVAL_MS));
        }
    }
}