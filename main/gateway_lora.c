#include "gateway_lora.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "string.h"

static const char *TAG = "GATEWAY_LORA";

/**
 * SX127x LoRa Register Map
 */
#define REG_FIFO                    0x00
#define REG_OPMODE                  0x01
#define REG_FRF_MSB                 0x06
#define REG_FRF_MID                 0x07
#define REG_FRF_LSB                 0x08
#define REG_PA_CONFIG               0x09
#define REG_PA_RAMP                 0x0A
#define REG_OCP                     0x0B
#define REG_LNA                     0x12
#define REG_RX_CONFIG               0x0D
#define REG_RSSI_CONFIG             0x0E
#define REG_RSSI_COLLISION          0x0F
#define REG_RSSI_THRESH             0x10
#define REG_RSSI_WIDEBAND           0x11
#define REG_MODEM_CONFIG_1          0x1D
#define REG_MODEM_CONFIG_2          0x1E
#define REG_MODEM_CONFIG_3          0x26
#define REG_PREAMBLE_MSB            0x20
#define REG_PREAMBLE_LSB            0x21
#define REG_PAYLOAD_LENGTH          0x22
#define REG_FIFO_RX_CURRENT_ADDR    0x10
#define REG_FIFO_RX_BASE_ADDR       0x0F
#define REG_FIFO_TX_BASE_ADDR       0x0E
#define REG_IRQ_FLAGS               0x12
#define REG_RX_NB_BYTES             0x13
#define REG_RX_HEADER_CNT_VALUE_MSB 0x14
#define REG_RX_HEADER_CNT_VALUE_LSB 0x15
#define REG_RX_PACKET_CNT_VALUE_MSB 0x16
#define REG_RX_PACKET_CNT_VALUE_LSB 0x17
#define REG_MODEM_STAT              0x18
#define REG_PKT_SNR_VALUE           0x19
#define REG_PKT_RSSI_VALUE          0x1A
#define REG_RSSI_VALUE              0x1B
#define REG_HOP_CHANNEL             0x1C
#define REG_VERSION                 0x42

/**
 * LoRa Mode Bits (OpMode register)
 */
#define MODE_SLEEP                  0x00
#define MODE_STANDBY                0x01
#define MODE_TX                     0x03
#define MODE_RXCONT                 0x05
#define MODE_RXSINGLE               0x04
#define MODE_CAD                    0x07
#define MODE_FS_TX                  0x02
#define MODE_FS_RX                  0x04

/**
 * LoRa Interrupt Flags
 */
#define IRQ_TX_DONE                 0x08
#define IRQ_PAYLOAD_CRC_ERROR       0x20
#define IRQ_RX_DONE                 0x40
#define IRQ_RX_TIMEOUT              0x80

/**
 * Helper function: Write register
 */
static esp_err_t _write_register(sx127x_t *dev, uint8_t reg, uint8_t data) {
    uint8_t tx_data[2] = { reg | 0x80, data };
    uint8_t rx_data[2] = { 0 };

    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    return spi_device_transmit(dev->spi, &t);
}

/**
 * Helper function: Set LoRa mode
 */
static esp_err_t _set_opmode(sx127x_t *dev, uint8_t mode) {
    uint8_t reg_val = 0;
    esp_err_t ret = sx127x_read_register(dev, REG_OPMODE, &reg_val);
    if (ret != ESP_OK) return ret;

    reg_val = (reg_val & 0xF8) | (mode & 0x07);
    return _write_register(dev, REG_OPMODE, reg_val);
}

/**
 * Helper function: Set radio frequency
 */
static esp_err_t _set_frequency(sx127x_t *dev, uint32_t frequency) {
    uint64_t frf = ((uint64_t)frequency << 19) / 32000000;

    esp_err_t ret = _write_register(dev, REG_FRF_MSB, (uint8_t)((frf >> 16) & 0xFF));
    if (ret != ESP_OK) return ret;

    ret = _write_register(dev, REG_FRF_MID, (uint8_t)((frf >> 8) & 0xFF));
    if (ret != ESP_OK) return ret;

    ret = _write_register(dev, REG_FRF_LSB, (uint8_t)(frf & 0xFF));
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Frequency set to %lu Hz", frequency);
    return ESP_OK;
}

/**
 * Helper function: Set modem configuration
 */
static esp_err_t _configure_modem(sx127x_t *dev, const gateway_lora_config_t *config) {
    uint8_t modem_cfg1, modem_cfg2, modem_cfg3;

    modem_cfg1 = (config->bandwidth << 4) | (config->coding_rate << 1);
    modem_cfg2 = (config->spreading_factor << 4) | 0x04;
    modem_cfg3 = 0x00;

    esp_err_t ret = _write_register(dev, REG_MODEM_CONFIG_1, modem_cfg1);
    if (ret != ESP_OK) return ret;

    ret = _write_register(dev, REG_MODEM_CONFIG_2, modem_cfg2);
    if (ret != ESP_OK) return ret;

    ret = _write_register(dev, REG_MODEM_CONFIG_3, modem_cfg3);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Modem configured - BW:%d, SF:%d, CR:%d",
             config->bandwidth, config->spreading_factor, config->coding_rate);
    return ESP_OK;
}

/**
 * Helper function: Configure preamble length
 */
static esp_err_t _set_preamble(sx127x_t *dev, uint16_t preamble_length) {
    esp_err_t ret = _write_register(dev, REG_PREAMBLE_MSB, (uint8_t)((preamble_length >> 8) & 0xFF));
    if (ret != ESP_OK) return ret;

    ret = _write_register(dev, REG_PREAMBLE_LSB, (uint8_t)(preamble_length & 0xFF));
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Preamble length set to %d symbols", preamble_length);
    return ESP_OK;
}

/**
 * Helper function: Configure transmit power
 */
static esp_err_t _set_tx_power(sx127x_t *dev, uint8_t power) {
    uint8_t pa_config = 0x80 | (power & 0x0F);
    return _write_register(dev, REG_PA_CONFIG, pa_config);
}

/**
 * Helper function: Enter continuous receive mode
 */
static esp_err_t _enter_rx_mode(sx127x_t *dev) {
    return _set_opmode(dev, MODE_RXCONT);
}

/**
 * FreeRTOS task for continuous LoRa receive
 */
static void _rx_task(void *pvParameter) {
    gateway_lora_t *gateway = (gateway_lora_t *)pvParameter;
    lora_message_t msg;
    uint8_t irq_flags, rx_bytes;
    uint8_t rssi_raw;

    ESP_LOGI(TAG, "RX Task started - monitoring for incoming messages");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(50));

        if (sx127x_read_register(&gateway->sx_device, REG_IRQ_FLAGS, &irq_flags) != ESP_OK) {
            continue;
        }

        if (irq_flags & IRQ_RX_DONE) {
            ESP_LOGD(TAG, "RX_DONE interrupt detected");

            if (irq_flags & IRQ_PAYLOAD_CRC_ERROR) {
                gateway->stats.crc_errors++;
                ESP_LOGD(TAG, "CRC error (Total: %lu) - noise or incomplete packet",
                         gateway->stats.crc_errors);
                _write_register(&gateway->sx_device, REG_IRQ_FLAGS, 0xFF);
                continue;
            }

            if (sx127x_read_register(&gateway->sx_device, REG_RX_NB_BYTES, &rx_bytes) != ESP_OK) {
                continue;
            }

            if (rx_bytes > LORA_MAX_PAYLOAD_LENGTH) {
                ESP_LOGW(TAG, "Payload size %d exceeds max %d bytes", rx_bytes, LORA_MAX_PAYLOAD_LENGTH);
                _write_register(&gateway->sx_device, REG_IRQ_FLAGS, 0xFF);
                continue;
            }

            /* Read RSSI before any other register access */
            sx127x_read_register(&gateway->sx_device, REG_PKT_RSSI_VALUE, &rssi_raw);
            msg.rssi = rssi_raw - 157;

            /* RSSI Threshold Filtering - ignore weak signals unlikely to be valid packets */
            if (gateway->config.rssi_threshold != 0 && msg.rssi < gateway->config.rssi_threshold) {
                gateway->stats.rssi_filtered++;
                ESP_LOGD(TAG, "RSSI filtered: %d dBm (threshold: %d dBm, Total filtered: %lu)",
                         msg.rssi, gateway->config.rssi_threshold, gateway->stats.rssi_filtered);
                _write_register(&gateway->sx_device, REG_IRQ_FLAGS, 0xFF);
                continue;
            }

            uint8_t fifo_addr = 0;
            sx127x_read_register(&gateway->sx_device, REG_FIFO_RX_CURRENT_ADDR, &fifo_addr);
            _write_register(&gateway->sx_device, REG_FIFO_RX_BASE_ADDR, fifo_addr);

            msg.length = rx_bytes;
            msg.rx_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

            sx127x_read_register(&gateway->sx_device, REG_PKT_SNR_VALUE, (uint8_t *)&msg.snr);
            msg.snr = (msg.snr + 2) >> 2;

            for (uint16_t i = 0; i < rx_bytes; i++) {
                sx127x_read_register(&gateway->sx_device, REG_FIFO, &msg.payload[i]);
            }

            if (xQueueSend(gateway->rx_queue, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
                ESP_LOGW(TAG, "Queue full - dropping message of %d bytes", rx_bytes);
            } else {
                gateway->stats.valid_packets++;
                ESP_LOGI(TAG, "[PKT #%lu] Size:%d bytes | RSSI:%d dBm | SNR:%d dB",
                         gateway->stats.valid_packets, msg.length, msg.rssi, msg.snr);
            }

            _write_register(&gateway->sx_device, REG_IRQ_FLAGS, 0xFF);
        }
    }

    vTaskDelete(NULL);
}

/**
 * Initialize the LoRa gateway
 */
esp_err_t gateway_lora_init(sx127x_config_t *lora_cfg, gateway_lora_t *gateway) {
    if (!lora_cfg || !gateway) {
        ESP_LOGE(TAG, "Invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Initializing LoRa gateway");

    esp_err_t ret = sx127x_init(lora_cfg, &gateway->sx_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SX127x initialization failed");
        return ret;
    }

    uint8_t version = 0;
    ret = sx127x_read_register(&gateway->sx_device, REG_VERSION, &version);
    if (ret != ESP_OK || version != 0x12) {
        ESP_LOGE(TAG, "Invalid SX127x version: 0x%02X (expected 0x12)", version);
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "SX127x verified - Version: 0x%02X", version);

    gateway->rx_queue = xQueueCreate(LORA_RX_QUEUE_SIZE, sizeof(lora_message_t));
    if (!gateway->rx_queue) {
        ESP_LOGE(TAG, "Failed to create RX queue");
        return ESP_FAIL;
    }

    gateway->config.frequency = LORA_DEFAULT_FREQUENCY;
    gateway->config.bandwidth = LORA_DEFAULT_BANDWIDTH;
    gateway->config.spreading_factor = LORA_DEFAULT_SPREADING_FACTOR;
    gateway->config.coding_rate = 1;
    gateway->config.preamble_length = 8;
    gateway->config.tx_power = 17;
    gateway->config.rssi_threshold = -120;    /* Filter very weak signals by default */

    memset(&gateway->stats, 0, sizeof(gateway_lora_stats_t));

    ret = _set_opmode(&gateway->sx_device, MODE_SLEEP);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set sleep mode");
        return ret;
    }

    ESP_LOGI(TAG, "LoRa gateway initialized successfully");
    return ESP_OK;
}

/**
 * Configure LoRa radio parameters
 */
esp_err_t gateway_lora_configure(gateway_lora_t *gateway, const gateway_lora_config_t *config) {
    if (!gateway || !config) {
        ESP_LOGE(TAG, "Invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Configuring radio parameters");

    memcpy(&gateway->config, config, sizeof(gateway_lora_config_t));

    esp_err_t ret = _set_opmode(&gateway->sx_device, MODE_SLEEP);
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(10));

    ret = _set_frequency(&gateway->sx_device, config->frequency);
    if (ret != ESP_OK) return ret;

    ret = _configure_modem(&gateway->sx_device, config);
    if (ret != ESP_OK) return ret;

    ret = _set_preamble(&gateway->sx_device, config->preamble_length);
    if (ret != ESP_OK) return ret;

    ret = _set_tx_power(&gateway->sx_device, config->tx_power);
    if (ret != ESP_OK) return ret;

    _write_register(&gateway->sx_device, REG_LNA, 0x23);
    _write_register(&gateway->sx_device, REG_IRQ_FLAGS, 0xFF);

    ret = _set_opmode(&gateway->sx_device, MODE_STANDBY);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "Radio configuration complete");
    return ESP_OK;
}

/**
 * Start continuous receive task
 */
esp_err_t gateway_lora_start_rx(gateway_lora_t *gateway) {
    if (!gateway) {
        ESP_LOGE(TAG, "Invalid gateway parameter");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = _enter_rx_mode(&gateway->sx_device);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enter RX mode");
        return ret;
    }

    BaseType_t task_ret = xTaskCreate(
        _rx_task,
        "LoRa_RX",
        4096,
        gateway,
        5,
        &gateway->rx_task_handle
    );

    if (task_ret != pdPASS) {
        ESP_LOGE(TAG, "Failed to create RX task");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Continuous receive mode started");
    return ESP_OK;
}

/**
 * Stop receive task
 */
esp_err_t gateway_lora_stop_rx(gateway_lora_t *gateway) {
    if (!gateway || !gateway->rx_task_handle) {
        ESP_LOGE(TAG, "Invalid gateway parameter");
        return ESP_ERR_INVALID_ARG;
    }

    vTaskDelete(gateway->rx_task_handle);
    gateway->rx_task_handle = NULL;

    esp_err_t ret = _set_opmode(&gateway->sx_device, MODE_STANDBY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set standby mode");
        return ret;
    }

    ESP_LOGI(TAG, "Continuous receive mode stopped");
    return ESP_OK;
}

/**
 * Get received message from queue
 */
esp_err_t gateway_lora_get_rx_message(gateway_lora_t *gateway, lora_message_t *message, uint32_t timeout_ms) {
    if (!gateway || !message) {
        ESP_LOGE(TAG, "Invalid parameter");
        return ESP_ERR_INVALID_ARG;
    }

    TickType_t ticks = (timeout_ms == 0) ? 0 : pdMS_TO_TICKS(timeout_ms);

    if (xQueueReceive(gateway->rx_queue, message, ticks) == pdTRUE) {
        return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}

/**
 * Get queue message count
 */
uint32_t gateway_lora_get_queue_count(gateway_lora_t *gateway) {
    if (!gateway) {
        return 0;
    }
    return uxQueueMessagesWaiting(gateway->rx_queue);
}

/**
 * Get gateway statistics
 */
esp_err_t gateway_lora_get_stats(gateway_lora_t *gateway, uint32_t *valid_packets,
                                   uint32_t *crc_errors, uint32_t *rssi_filtered) {
    if (!gateway) {
        return ESP_ERR_INVALID_ARG;
    }

    if (valid_packets) *valid_packets = gateway->stats.valid_packets;
    if (crc_errors) *crc_errors = gateway->stats.crc_errors;
    if (rssi_filtered) *rssi_filtered = gateway->stats.rssi_filtered;

    return ESP_OK;
}

/**
 * Deinitialize gateway
 */
esp_err_t gateway_lora_deinit(gateway_lora_t *gateway) {
    if (!gateway) {
        return ESP_ERR_INVALID_ARG;
    }

    if (gateway->rx_task_handle) {
        gateway_lora_stop_rx(gateway);
    }

    if (gateway->rx_queue) {
        vQueueDelete(gateway->rx_queue);
        gateway->rx_queue = NULL;
    }

    ESP_LOGI(TAG, "LoRa gateway deinitialized");
    return ESP_OK;
}
