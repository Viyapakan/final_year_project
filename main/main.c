#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sx127x.h"

// Define your verified pins
#define LORA_MOSI 23
#define LORA_MISO 19
#define LORA_SCK  18
#define LORA_CS   5
#define LORA_RST  14

void app_main(void) {
    // 1. Initialize the SPI Bus
    spi_bus_config_t buscfg = {
        .miso_io_num = LORA_MISO,
        .mosi_io_num = LORA_MOSI,
        .sclk_io_num = LORA_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // 2. Initialize the LoRa Driver Component
    sx127x_config_t lora_cfg = {
        .rst_pin = LORA_RST,
        .cs_pin = LORA_CS,
        .host = VSPI_HOST
    };

    sx127x_t lora_dev;
    if (sx127x_init(&lora_cfg, &lora_dev) == ESP_OK) {
        uint8_t version = 0;
        while(1) {
            if (sx127x_read_register(&lora_dev, REG_VERSION, &version) == ESP_OK) {
                printf("LoRa Check: Register 0x42 = 0x%02X (Expected 0x12)\n", version);
            }
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }
}