/*
 * sht30_config.h
 *
 *  Created on: May 25, 2026
 *      Author: MSI GF63
 */

#ifndef INC_SHT30_CONFIG_H_
#define INC_SHT30_CONFIG_H_

#include "main.h"
#include "utils.h"

/* SHT30 I2C Address (0x44 << 1) */
#define SHT30_I2C_ADDR (0x44 << 1)

/* Sensor Reading Structure */
typedef struct {
    uint16_t humidity;    /* Scaled by 100 (e.g., 5512 = 55.12%) */
    int16_t  temperature; /* Scaled by 100 (e.g., 2450 = 24.50 C) */
} SHT30_SensorReading_t;

/* Function Prototypes */
HAL_StatusTypeDef SHT30_Init(void);
HAL_StatusTypeDef SHT30_ReadSensor(SHT30_SensorReading_t *reading);

#endif /* INC_SHT30_CONFIG_H_ */
