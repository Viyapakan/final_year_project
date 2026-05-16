/*
 * lora_config.c
 *
 *  Created on: Jan 7, 2026
 *      Author: MSI GF63
 */

#include "lora_config.h"

/* Private LoRa instance (hidden from main) */
static LoRa myLoRa;

uint16_t lora_init(void)
{
    myLoRa = newLoRa();

    /* NSS (CS) pin */
    myLoRa.CS_port = LORA_NSS_GPIO_Port;
    myLoRa.CS_pin  = LORA_NSS_Pin;

    /* RESET pin */
    myLoRa.reset_port = LORA_RST_GPIO_Port;
    myLoRa.reset_pin  = LORA_RST_Pin;

    /* DIO0 pin (EXTI enabled in gpio.c) */
    myLoRa.DIO0_port = LORA_DIO0_GPIO_Port;
    myLoRa.DIO0_pin  = LORA_DIO0_Pin;

    /* SPI handler */
    myLoRa.hSPIx = &hspi1;

    myLoRa.frequency             = 433;        // MHz
    myLoRa.spredingFactor        = SF_7;
    myLoRa.bandWidth             = BW_125KHz;
    myLoRa.crcRate               = CR_4_5;
    myLoRa.power                 = POWER_17db; // or POWER_17db if supply limited
    myLoRa.overCurrentProtection = 130;
    myLoRa.preamble              = 8;
//    myLoRa.crc = 1;          // ADD THIS! explicitly enable payload CRC


    return LoRa_init(&myLoRa);
}


uint8_t lora_send(uint8_t *data, uint8_t length, uint16_t timeout)
{
    return LoRa_transmit(&myLoRa, data, length, timeout);
}

uint8_t lora_receive(uint8_t *data, uint8_t length)
{
    return LoRa_receive(&myLoRa, data, length);
}

int lora_get_rssi(void)
{
    return LoRa_getRSSI(&myLoRa);
}
