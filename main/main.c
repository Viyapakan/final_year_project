#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Define the onboard LED pin (usually GPIO 2 for ESP32)
#define BLINK_GPIO 2

void app_main(void)
{
    // 1. Hardware Setup
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    printf("--- ESP32 Gateway Booting ---\n");

    // 2. The Infinite FreeRTOS Task Loop
    while (1) {
        printf("System Alive! Hello from ESP-IDF Phase 1.\n");
        
        gpio_set_level(BLINK_GPIO, 1); // LED ON
        vTaskDelay(pdMS_TO_TICKS(1000)); // FreeRTOS delay for 1000ms
        
        gpio_set_level(BLINK_GPIO, 0); // LED OFF
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}