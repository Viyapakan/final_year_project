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
    float voltage = 0, current = 0;
    get_power_data(voltage, current);
    
    float temp = get_temperature();

    Serial.print("V: "); Serial.print(voltage, 3);
    Serial.print("V | Temp: "); Serial.print(temp, 1);
    Serial.println("C");

    // Update storage to include temperature
    // You'll need to update log_data in storage.h to accept 'temp'
    // log_data(voltage, current, temp); 

    delay(5000);
}