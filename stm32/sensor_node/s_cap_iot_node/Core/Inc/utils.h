/*
 * utils.h
 *
 *  Created on: Jan 7, 2026
 *      Author: MSI GF63
 */

#ifndef INC_UTILS_H_
#define INC_UTILS_H_
#include "gpio.h"
#include "rtc.h"
#include "stm32f1xx_hal.h"
#include "stdio.h"

void led_on(void);
void led_off(void);
void led_toggle(uint32_t delay_ms, uint8_t times);
uint32_t Get_STM32_UniqueID(void);
void Enter_Standby_RTC(uint32_t seconds);

#endif /* INC_UTILS_H_ */
