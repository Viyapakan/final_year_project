#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
bool mqtt_init();
void mqtt_monitor_task(void *parameter);
bool mqtt_publish(const char* topic, const char* payload);
void mqtt_publish_task(void *parameter);

// -----------------------------------------------------------------------------
// Exposed globals — needed by mqtt_publish_task to set socket timeout
// before each publish call, preventing an indefinite TLS block on a
// dead connection (root cause of the silent gateway hang).
// -----------------------------------------------------------------------------
extern WiFiClientSecure secure_client;
extern PubSubClient     mqtt_client;

extern QueueHandle_t mqttPublishQueue;
#endif // MQTT_MANAGER_H