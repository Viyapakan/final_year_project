#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
bool mqtt_init();
void mqtt_monitor_task(void *parameter);
bool mqtt_publish(const char* topic, const char* payload);
void mqtt_publish_task(void *parameter); // <-- NEW TASK

extern QueueHandle_t mqttPublishQueue;
#endif // MQTT_MANAGER_H