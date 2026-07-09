/*
 * lora_config.h
 *
 *  Created on: Jan 7, 2026
 *      Author: MSI GF63
 */

#ifndef INC_LORA_CONFIG_H_
#define INC_LORA_CONFIG_H_

#include "LoRa.h"
#include "spi.h"
#include "gpio.h"
#include "utils.h"

uint16_t lora_init(void);
uint8_t  lora_send(uint8_t *data, uint8_t length, uint16_t timeout);
uint8_t  lora_receive(uint8_t *data, uint8_t length);
int      lora_get_rssi(void);

void     lora_sleep(void);

#endif /* INC_LORA_CONFIG_H_ */
