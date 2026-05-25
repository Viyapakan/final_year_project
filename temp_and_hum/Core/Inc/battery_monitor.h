/*
 * battery_monitor.h
 *
 *  Created on: May 25, 2026
 *      Author: MSI GF63
 */

#ifndef INC_BATTERY_MONITOR_H_
#define INC_BATTERY_MONITOR_H_

#include "main.h"

/* * @brief Reads the ADC and calculates the true supercapacitor voltage.
 * @param hadc: Pointer to the ADC handle (e.g., &hadc1)
 * @retval float: The calculated voltage in Volts (e.g., 4.25)
 */
float Battery_GetVoltage(ADC_HandleTypeDef *hadc);

#endif /* INC_BATTERY_MONITOR_H_ */
