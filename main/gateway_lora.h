#ifndef GATEWAY_LORA_H
#define GATEWAY_LORA_H

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"
#include "sx127x.h"
#include "freertos/queue.h"

/**
 * LoRa Radio Configuration Defaults
 * Easily adjustable for different deployment scenarios
 */
#define LORA_FREQ_433MHZ        433000000
#define LORA_FREQ_868MHZ        868000000
#define LORA_FREQ_915MHZ        915000000

#define LORA_DEFAULT_FREQUENCY  LORA_FREQ_433MHZ
#define LORA_DEFAULT_BANDWIDTH  7    /* 125 kHz */
#define LORA_DEFAULT_SPREADING_FACTOR 7

#define LORA_MAX_PAYLOAD_LENGTH 255
#define LORA_RX_QUEUE_SIZE      32

/**
 * LoRa Message Structure
 * Used for passing received packets through the queue
 */
typedef struct {
    uint8_t payload[LORA_MAX_PAYLOAD_LENGTH];
    uint16_t length;
    int8_t rssi;
    int8_t snr;
    uint32_t rx_time_ms;
} lora_message_t;

/**
 * LoRa Gateway Configuration Structure
 * Contains all parameters for radio operation
 */
typedef struct {
    uint32_t frequency;
    uint8_t bandwidth;
    uint8_t spreading_factor;
    uint8_t coding_rate;
    uint16_t preamble_length;
    uint8_t tx_power;
    int8_t rssi_threshold;      /* Ignore packets weaker than this (dBm) */
} gateway_lora_config_t;

/**
 * LoRa Gateway Statistics Structure
 */
typedef struct {
    uint32_t valid_packets;
    uint32_t crc_errors;
    uint32_t rssi_filtered;
} gateway_lora_stats_t;

/**
 * LoRa Gateway Handle Structure
 * Maintains state for the gateway instance
 */
typedef struct {
    sx127x_t sx_device;
    gateway_lora_config_t config;
    QueueHandle_t rx_queue;
    TaskHandle_t rx_task_handle;
    gateway_lora_stats_t stats;
} gateway_lora_t;

/**
 * Initialize the LoRa gateway with default configuration
 *
 * @param[in]  lora_cfg    SX127x hardware configuration
 * @param[out] gateway     Gateway instance handle
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t gateway_lora_init(sx127x_config_t *lora_cfg, gateway_lora_t *gateway);

/**
 * Configure LoRa radio parameters
 *
 * @param[in] gateway      Gateway instance handle
 * @param[in] config       Radio configuration parameters
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t gateway_lora_configure(gateway_lora_t *gateway, const gateway_lora_config_t *config);

/**
 * Start the continuous receive task
 * Spawns a FreeRTOS task that monitors the radio for incoming messages
 *
 * @param[in] gateway      Gateway instance handle
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t gateway_lora_start_rx(gateway_lora_t *gateway);

/**
 * Stop the receive task
 *
 * @param[in] gateway      Gateway instance handle
 * @return ESP_OK on success, ESP_FAIL on error
 */
esp_err_t gateway_lora_stop_rx(gateway_lora_t *gateway);

/**
 * Get received message from queue (non-blocking)
 *
 * @param[in]  gateway     Gateway instance handle
 * @param[out] message     Received message structure
 * @param[in]  timeout_ms  Queue wait timeout in milliseconds (0 = non-blocking)
 * @return ESP_OK if message received, ESP_ERR_TIMEOUT if no message available
 */
esp_err_t gateway_lora_get_rx_message(gateway_lora_t *gateway, lora_message_t *message, uint32_t timeout_ms);

/**
 * Get current queue status
 *
 * @param[in] gateway      Gateway instance handle
 * @return Number of messages pending in the queue
 */
uint32_t gateway_lora_get_queue_count(gateway_lora_t *gateway);

/**
 * Get gateway statistics
 *
 * @param[in]  gateway           Gateway instance handle
 * @param[out] valid_packets     Number of valid packets received
 * @param[out] crc_errors        Number of packets with CRC errors
 * @param[out] rssi_filtered     Number of packets filtered by RSSI
 * @return ESP_OK on success
 */
esp_err_t gateway_lora_get_stats(gateway_lora_t *gateway, uint32_t *valid_packets,
                                   uint32_t *crc_errors, uint32_t *rssi_filtered);

/**
 * Deinitialize the gateway and free resources
 *
 * @param[in] gateway      Gateway instance handle
 * @return ESP_OK on success
 */
esp_err_t gateway_lora_deinit(gateway_lora_t *gateway);

#endif /* GATEWAY_LORA_H */
