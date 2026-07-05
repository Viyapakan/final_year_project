#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// -----------------------------------------------------------------------------
// WiFi Credentials
// -----------------------------------------------------------------------------
#define WIFI_SSID "Dialog 4G 554"
#define WIFI_PASSWORD "9672745B"
// #define WIFI_PASSWORD "my_router_24"

// -----------------------------------------------------------------------------
// WiFi Timing Configuration
// -----------------------------------------------------------------------------
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr uint32_t WIFI_RETRY_INTERVAL_MS  = 10000;

#endif // WIFI_CONFIG_H