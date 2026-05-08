#include "esp_log.h"
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gateway_lora.h"

static const char *TAG = "MAIN";

/**
 * SPI and LoRa Hardware Pin Definitions
 * Verified with register readback (0x42 -> 0x12)
 */
#define LORA_MOSI   23
#define LORA_MISO   19
#define LORA_SCK    18
#define LORA_CS     5
#define LORA_RST    14

void app_main(void) {
    ESP_LOGI(TAG, "LoRa Gateway Firmware Starting");

    /* Initialize SPI Bus */
    spi_bus_config_t buscfg = {
        .miso_io_num = LORA_MISO,
        .mosi_io_num = LORA_MOSI,
        .sclk_io_num = LORA_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_LOGI(TAG, "SPI bus initialized");

    /* Configure LoRa Hardware */
    sx127x_config_t lora_hw_cfg = {
        .rst_pin = LORA_RST,
        .cs_pin = LORA_CS,
        .host = VSPI_HOST
    };

    /* Initialize Gateway */
    gateway_lora_t gateway;
    if (gateway_lora_init(&lora_hw_cfg, &gateway) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LoRa gateway");
        return;
    }

    /* Configure Radio Parameters */
    gateway_lora_config_t radio_cfg = {
        .frequency = LORA_DEFAULT_FREQUENCY,
        .bandwidth = LORA_DEFAULT_BANDWIDTH,
        .spreading_factor = LORA_DEFAULT_SPREADING_FACTOR,
        .coding_rate = 1,
        .preamble_length = 8,
        .tx_power = 17,
        .rssi_threshold = -120     /* Ignore signals weaker than -120 dBm (filters noise) */
    };

    if (gateway_lora_configure(&gateway, &radio_cfg) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure radio");
        return;
    }

    /* Start Continuous Receive Mode */
    if (gateway_lora_start_rx(&gateway) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start RX mode");
        return;
    }

    ESP_LOGI(TAG, "Gateway operational - Entering message processing loop");

    /* Main Message Processing Loop */
    lora_message_t rx_msg;
    uint32_t msg_count = 0;
    uint32_t stat_valid, stat_crc, stat_rssi;

    while (1) {
        /* Non-blocking check for received messages */
        if (gateway_lora_get_rx_message(&gateway, &rx_msg, 100) == ESP_OK) {
            msg_count++;
            
            ESP_LOGI(TAG, "");
            ESP_LOGI(TAG, "═══════════════════════════════════════════");
            ESP_LOGI(TAG, "MESSAGE #%lu RECEIVED", msg_count);
            ESP_LOGI(TAG, "═══════════════════════════════════════════");
            ESP_LOGI(TAG, "Length:     %d bytes", rx_msg.length);
            ESP_LOGI(TAG, "RSSI:       %d dBm", rx_msg.rssi);
            ESP_LOGI(TAG, "SNR:        %d dB", rx_msg.snr);
            ESP_LOGI(TAG, "Timestamp:  %lu ms", rx_msg.rx_time_ms);
            
            ESP_LOGI(TAG, "Payload (HEX):");
            ESP_LOG_BUFFER_HEX(TAG, rx_msg.payload, rx_msg.length);
            
            /* Get and display statistics */
            gateway_lora_get_stats(&gateway, &stat_valid, &stat_crc, &stat_rssi);
            ESP_LOGI(TAG, "---GATEWAY STATISTICS---");
            ESP_LOGI(TAG, "Valid packets:   %lu", stat_valid);
            ESP_LOGI(TAG, "CRC errors:      %lu", stat_crc);
            ESP_LOGI(TAG, "RSSI filtered:   %lu", stat_rssi);
            ESP_LOGI(TAG, "Queue pending:   %lu", gateway_lora_get_queue_count(&gateway));
            ESP_LOGI(TAG, "═══════════════════════════════════════════");
            ESP_LOGI(TAG, "");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}