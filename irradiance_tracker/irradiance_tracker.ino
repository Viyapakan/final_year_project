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

    if (!init_sd_card()) {
        // If SD card fails, maybe blink LED fast to warn us?
        led_toggle(10, 50); 
    }

    if (!init_power_sensor()) {
        led_toggle(5, 200); // Blink slowly if INA226 fails
    }
    
    analogSetAttenuation(ADC_11db);
}

void loop() {
    float voltage = 0;
    float current = 0;

    get_power_data(voltage, current);

    Serial.print("Bus Voltage: ");
    Serial.print(voltage, 3);
    Serial.print("V | Current: ");
    Serial.print(current, 3);
    Serial.println("A");

    delay(2000);
}