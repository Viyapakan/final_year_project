#ifndef SX127X_H
#define SX127X_H

#include "esp_err.h"
#include "driver/spi_master.h"

// Register Map
#define REG_VERSION 0x42

typedef struct {
    int rst_pin;
    int cs_pin;
    spi_host_device_t host;
} sx127x_config_t;

typedef struct {
    spi_device_handle_t spi;
    sx127x_config_t cfg;
} sx127x_t;

// API
esp_err_t sx127x_init(sx127x_config_t *config, sx127x_t *dev);
esp_err_t sx127x_read_register(sx127x_t *dev, uint8_t reg, uint8_t *data);

#endif