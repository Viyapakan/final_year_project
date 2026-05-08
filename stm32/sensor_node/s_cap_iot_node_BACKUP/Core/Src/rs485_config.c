#include "rs485_config.h"

/* ================= PRIVATE ================= */

static uint16_t RS485_CRC16(const uint8_t *data, uint16_t length)
{
    uint16_t crc = 0xFFFF;

    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }
    return crc;
}

static inline void RS485_TX_Enable(void)
{
    HAL_GPIO_WritePin(RS485_CTRL_GPIO_Port, RS485_CTRL_Pin, GPIO_PIN_SET);
}

static inline void RS485_RX_Enable(void)
{
    HAL_GPIO_WritePin(RS485_CTRL_GPIO_Port, RS485_CTRL_Pin, GPIO_PIN_RESET);
}

/* ================= API IMPLEMENTATION ================= */

HAL_StatusTypeDef RS485_Init(void)
{
    /* GPIO already configured in gpio.c */
    RS485_RX_Enable();   // Ensure receive mode
    return HAL_OK;
}

HAL_StatusTypeDef RS485_ReadHoldingRegisters(uint16_t start_reg,
                                             uint16_t reg_count,
                                             uint8_t *rx_buffer)
{
    uint8_t tx_frame[8];
    uint16_t rx_len = 5 + (reg_count * 2);

    /* Build Modbus request */
    tx_frame[0] = RS485_SLAVE_ADDR;
    tx_frame[1] = RS485_FUNC_READ_HOLDING;
    tx_frame[2] = start_reg >> 8;
    tx_frame[3] = start_reg & 0xFF;
    tx_frame[4] = reg_count >> 8;
    tx_frame[5] = reg_count & 0xFF;

    uint16_t crc = RS485_CRC16(tx_frame, 6);
    tx_frame[6] = crc & 0xFF;
    tx_frame[7] = crc >> 8;

    /* Transmit */
    RS485_TX_Enable();
    HAL_UART_Transmit(&huart2, tx_frame, sizeof(tx_frame), 100);
    while (__HAL_UART_GET_FLAG(&huart2, UART_FLAG_TC) == RESET);
    RS485_RX_Enable();

    /* Receive */
    if (HAL_UART_Receive(&huart2, rx_buffer, rx_len, RS485_RX_TIMEOUT_MS) != HAL_OK)
        return HAL_TIMEOUT;

    /* Validate slave address */
    if (rx_buffer[0] != RS485_SLAVE_ADDR)
        return HAL_ERROR;

    /* Check Modbus exception */
    if (rx_buffer[1] & 0x80)
        return HAL_ERROR;

    /* CRC check */
    uint16_t rx_crc = (rx_buffer[rx_len - 1] << 8) |
                       rx_buffer[rx_len - 2];

    uint16_t calc_crc = RS485_CRC16(rx_buffer, rx_len - 2);

    if (rx_crc != calc_crc)
        return HAL_ERROR;

    /* Modbus RTU silent interval (important at 4800 baud) */
    HAL_Delay(5);

    return HAL_OK;
}

uint16_t RS485_GetRegister(const uint8_t *rx_buffer, uint8_t index)
{
    uint8_t hi = rx_buffer[3 + (index * 2)];
    uint8_t lo = rx_buffer[4 + (index * 2)];
    return (hi << 8) | lo;
}


/* ================= Sensor-Level APIs ================= */

HAL_StatusTypeDef RS485_ReadSoilMoisture(uint16_t *value)
{
    uint8_t rx[8];

    if (RS485_ReadHoldingRegisters(SOIL_REG_MOISTURE, 1, rx) != HAL_OK)
        return HAL_ERROR;

    *value = RS485_GetRegister(rx, 0);
    return HAL_OK;
}

HAL_StatusTypeDef RS485_ReadSoilTemperature(uint16_t *value)
{
    uint8_t rx[8];

    if (RS485_ReadHoldingRegisters(SOIL_REG_TEMPERATURE, 1, rx) != HAL_OK)
        return HAL_ERROR;

    *value = RS485_GetRegister(rx, 0);
    return HAL_OK;
}

HAL_StatusTypeDef RS485_ReadSoilEC(uint16_t *value)
{
    uint8_t rx[8];

    if (RS485_ReadHoldingRegisters(SOIL_REG_EC, 1, rx) != HAL_OK)
        return HAL_ERROR;

    *value = RS485_GetRegister(rx, 0);
    return HAL_OK;
}

HAL_StatusTypeDef RS485_ReadSoilReserved(uint16_t *value)
{
    uint8_t rx[8];

    if (RS485_ReadHoldingRegisters(SOIL_REG_RESERVED, 1, rx) != HAL_OK)
        return HAL_ERROR;

    *value = RS485_GetRegister(rx, 0);
    return HAL_OK;
}

HAL_StatusTypeDef RS485_ReadSoilSensor(RS485_SensorReading_t *reading)
{
    uint8_t rx[8];

    /* Read moisture */
    if (RS485_ReadHoldingRegisters(SOIL_REG_MOISTURE, 1, rx) != HAL_OK)
        return HAL_ERROR;
    reading->moisture = RS485_GetRegister(rx, 0);

    /* Read temperature */
    if (RS485_ReadHoldingRegisters(SOIL_REG_TEMPERATURE, 1, rx) != HAL_OK)
        return HAL_ERROR;
    reading->temperature = RS485_GetRegister(rx, 0);

    /* Read EC */
    if (RS485_ReadHoldingRegisters(SOIL_REG_EC, 1, rx) != HAL_OK)
        return HAL_ERROR;
    reading->ec = RS485_GetRegister(rx, 0);

    return HAL_OK;
}
