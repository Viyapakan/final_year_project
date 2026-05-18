#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <Arduino.h>

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
bool mqtt_init();
void mqtt_monitor_task(void *parameter);
bool mqtt_publish(const char* topic, const char* payload);

#endif // MQTT_MANAGER_H