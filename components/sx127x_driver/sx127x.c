#include "sx127x.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

esp_err_t sx127x_init(sx127x_config_t *config, sx127x_t *dev) {
    dev->cfg = *config;

    // 1. Hardware Reset
    gpio_set_direction(dev->cfg.rst_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->cfg.rst_pin, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(dev->cfg.rst_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // 2. Add device to SPI bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000, // 1MHz
        .mode = 0,
        .spics_io_num = dev->cfg.cs_pin,
        .queue_size = 7,
    };

    return spi_bus_add_device(dev->cfg.host, &devcfg, &dev->spi);
}

esp_err_t sx127x_read_register(sx127x_t *dev, uint8_t reg, uint8_t *data) {
    // Bit 7 = 0 for Read
    uint8_t tx_data[2] = { reg & 0x7F, 0xFF };
    uint8_t rx_data[2] = { 0 };

    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    esp_err_t ret = spi_device_transmit(dev->spi, &t);
    *data = rx_data[1];
    return ret;
}