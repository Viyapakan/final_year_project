#ifndef APP_TYPES_H
#define APP_TYPES_H

#include <Arduino.h>

#define LORA_MAX_PAYLOAD 48
#define SENSOR_TYPE_SOIL 0x01
#define SENSOR_TYPE_ENV  0x02 // For future expansion

// 1. The Gateway "Envelope" (Total header size: 6 bytes)
typedef struct __attribute__((packed)) {
    uint32_t device_id;                   // 4 bytes: Unique STM32 ID
    uint8_t  sensor_type;                 // 1 byte: Identifier for payload type
    uint8_t  payload_length;              // 1 byte: Exact size of trailing data
    uint8_t  payload[LORA_MAX_PAYLOAD];   // Variable buffer
} lora_packet_t;

// 2. Specific Payload Format for Soil Sensor (Total size: 6 bytes)
typedef struct __attribute__((packed)) {
    uint16_t humidity;
    int16_t  temperature;
    uint16_t ec;
} soil_payload_t;

#endif // APP_TYPES_H