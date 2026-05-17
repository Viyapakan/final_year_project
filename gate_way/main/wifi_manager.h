#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>

// -----------------------------------------------------------------------------
// Function Prototypes
// -----------------------------------------------------------------------------
bool wifi_init();
void wifi_monitor_task(void *parameter);

#endif // WIFI_MANAGER_H