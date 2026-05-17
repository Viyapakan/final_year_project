#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// -----------------------------------------------------------------------------
// WiFi Credentials
// -----------------------------------------------------------------------------
#define WIFI_SSID "Dialog 4G 5545"
#define WIFI_PASSWORD "9672745B"

// -----------------------------------------------------------------------------
// WiFi Timing Configuration
// -----------------------------------------------------------------------------
constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 15000;
constexpr uint32_t WIFI_RETRY_INTERVAL_MS  = 10000;

#endif // WIFI_CONFIG_H