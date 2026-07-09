/*
 * battery_monitor.c
 *
 *  Created on: May 25, 2026
 *      Author: MSI GF63
 */


#include "battery_monitor.h"

/* * Hardware Constants
 * VREF: The operating voltage of the STM32 (3.3V)
 * DIVIDER_MULTIPLIER: 2.0 because we use a 1M / 1M divider (cuts voltage in half)
 * ADC_MAX: 4095.0 for a 12-bit ADC
 */
#define VREF 3.31f
#define DIVIDER_MULTIPLIER 2.0f
#define ADC_MAX 4095.0f

float Battery_GetVoltage(ADC_HandleTypeDef *hadc)
{
    uint32_t raw_adc = 0;
    float voltage = 0.0f;

    /* 1. Start the ADC hardware */
    HAL_ADC_Start(hadc);

    /* 2. Wait for the conversion to finish (10ms timeout) */
    if (HAL_ADC_PollForConversion(hadc, 10) == HAL_OK)
    {
        /* 3. Read the raw 12-bit value (0 to 4095) */
        raw_adc = HAL_ADC_GetValue(hadc);

        /* 4. Calculate actual supercapacitor voltage */
        voltage = ((float)raw_adc / ADC_MAX) * VREF * DIVIDER_MULTIPLIER;
    }

    /* 5. Stop ADC to ensure it draws zero power when not in use */
    HAL_ADC_Stop(hadc);

    return voltage;
}
