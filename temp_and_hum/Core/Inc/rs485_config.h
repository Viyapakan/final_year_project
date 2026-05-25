#ifndef RS485_CONFIG_H
#define RS485_CONFIG_H

#include "stm32f1xx_hal.h"
#include "gpio.h"
#include "usart.h"
#include <stdint.h>

/* ================= Modbus Configuration ================= */

#define RS485_SLAVE_ADDR        0x01
#define RS485_FUNC_READ_HOLDING 0x03
#define RS485_RX_TIMEOUT_MS     800

/* ================= Register Map ================= */

#define SOIL_REG_MOISTURE       0x0000
#define SOIL_REG_TEMPERATURE    0x0001
#define SOIL_REG_EC             0x0002
#define SOIL_REG_RESERVED       0x0003

/* ================= Public API ================= */

HAL_StatusTypeDef RS485_Init(void);

HAL_StatusTypeDef RS485_ReadHoldingRegisters(uint16_t start_reg,
                                             uint16_t reg_count,
                                             uint8_t *rx_buffer);

uint16_t RS485_GetRegister(const uint8_t *rx_buffer, uint8_t index);

/* ===== High-level Sensor APIs (ONE BY ONE) ===== */

HAL_StatusTypeDef RS485_ReadSoilMoisture(uint16_t *value);
HAL_StatusTypeDef RS485_ReadSoilTemperature(uint16_t *value);
HAL_StatusTypeDef RS485_ReadSoilEC(uint16_t *value);
HAL_StatusTypeDef RS485_ReadSoilReserved(uint16_t *value);

typedef struct
{
    uint16_t moisture;     // raw (x10 %)
    uint16_t temperature;  // raw (x10 °C)
    uint16_t ec;           // µS/cm
} RS485_SensorReading_t;

/**
 * @brief Read all soil sensor values (moisture, temperature, EC)
 *
 * @param reading Pointer to sensor reading structure
 *
 * @return HAL_OK / HAL_ERROR
 */
HAL_StatusTypeDef RS485_ReadSoilSensor(RS485_SensorReading_t *reading);


#endif /* RS485_CONFIG_H */
