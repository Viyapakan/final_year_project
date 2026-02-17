#include "project_includes.h"

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        led_toggle(1, 100); // Visual feedback using your new util
        Serial.print(".");
    }

    led_on(); // Solid light means we are online
    Serial.println("\nWiFi connected!");
}

void loop() {
    // Ready for sensors!
}