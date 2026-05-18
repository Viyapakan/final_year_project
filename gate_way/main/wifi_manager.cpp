#include "wifi_manager.h"
#include "wifi_config.h"

// -----------------------------------------------------------------------------
// Initialize WiFi
// -----------------------------------------------------------------------------
bool wifi_init()
{
    Serial.println("\n[WIFI] Initializing WiFi...");

    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    uint32_t start_time = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");

        delay(500);

        if ((millis() - start_time) > WIFI_CONNECT_TIMEOUT_MS)
        {
            Serial.println("\n[WIFI] Connection timeout.");
            return false;
        }
    }

    Serial.println("\n[WIFI] Connected successfully.");
    Serial.print("[WIFI] IP Address: ");
    Serial.println(WiFi.localIP());

    // ---------------------------------------------------------
    // NEW: Sync Time via NTP for TLS/SSL Certificates
    // ---------------------------------------------------------
    Serial.println("[WIFI] Syncing time via NTP...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    
    time_t now = time(nullptr);
    while (now < 24 * 3600) {
        Serial.print(".");
        delay(500);
        now = time(nullptr);
    }
    Serial.println("\n[WIFI] Time synchronized.");
    // ---------------------------------------------------------

    return true;
}

// -----------------------------------------------------------------------------
// WiFi Monitor Task
// -----------------------------------------------------------------------------
void wifi_monitor_task(void *parameter)
{
    while (true)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("[WIFI] Connection lost. Reconnecting...");

            WiFi.disconnect();

            WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

            uint32_t start_time = millis();

            while (WiFi.status() != WL_CONNECTED &&
                  ((millis() - start_time) < WIFI_CONNECT_TIMEOUT_MS))
            {
                Serial.print(".");
                vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (WiFi.status() == WL_CONNECTED)
            {
                Serial.println("\n[WIFI] Reconnected.");
                Serial.print("[WIFI] IP Address: ");
                Serial.println(WiFi.localIP());
            }
            else
            {
                Serial.println("\n[WIFI] Reconnect failed.");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(WIFI_RETRY_INTERVAL_MS));
    }
}