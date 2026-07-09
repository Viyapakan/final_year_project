/*
 * lora_config.c
 *
 *  Created on: Jan 7, 2026
 *      Author: MSI GF63
 */

#include "lora_config.h"

/* Private LoRa instance (hidden from main) */
static LoRa myLoRa;
//
//uint16_t lora_init(void)
//{
//    if(switch_operation(TARGET_LORA, SWITCH_ON)){
//
//    myLoRa = newLoRa();
//
//    /* NSS (CS) pin */
//    myLoRa.CS_port = LORA_NSS_GPIO_Port;
//    myLoRa.CS_pin  = LORA_NSS_Pin;
//
//    /* RESET pin */
//    myLoRa.reset_port = LORA_RST_GPIO_Port;
//    myLoRa.reset_pin  = LORA_RST_Pin;
//
//    /* DIO0 pin (EXTI enabled in gpio.c) */
//    myLoRa.DIO0_port = LORA_DIO0_GPIO_Port;
//    myLoRa.DIO0_pin  = LORA_DIO0_Pin;
//
//    /* SPI handler */
//    myLoRa.hSPIx = &hspi1;
//
//    myLoRa.frequency             = 433;        // MHz
//    myLoRa.spredingFactor        = SF_7;
//    myLoRa.bandWidth             = BW_125KHz;
//    myLoRa.crcRate               = CR_4_5;
//    myLoRa.power                 = POWER_14db; // or POWER_17db if supply limited
//    myLoRa.overCurrentProtection = 130;
//    myLoRa.preamble              = 8;
//
//
//
//    return LoRa_init(&myLoRa);
//    } else {
//        printf("[ERROR] LoRa power switch failed. Unable to initialize LoRa module.\r\n");
//        return 1;
//    }
//}

uint16_t lora_init(void)
{
    printf("[DEBUG] lora_init: Requesting LoRa power ON...\r\n");

    if (switch_operation(TARGET_LORA, SWITCH_ON)) {

        // Let the capacitor completely fill up and stabilize
        printf("[DEBUG] lora_init: Power switch is verified ON. Delaying 250ms for voltage stabilization...\r\n");
        HAL_Delay(250);

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
        myLoRa.power                 = POWER_14db;
        myLoRa.overCurrentProtection = 130;
        myLoRa.preamble              = 8;

        printf("[DEBUG] lora_init: Triggering SX1278 hardware initialization...\r\n");
        uint16_t init_status = LoRa_init(&myLoRa);

        printf("[DEBUG] lora_init: LoRa_init finished with status code: %d\r\n", init_status);
        return init_status;

    } else {
        printf("[ERROR] lora_init: LoRa power switch failed. Unable to initialize.\r\n");
        return 1;
    }
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

void lora_sleep(void)
{
    /* Put the SX1278 into its lowest power SLEEP_MODE before cutting power.
     * STANDBY: ~1.6 mA  →  SLEEP: ~0.2 µA
     * This also ensures the chip's internal state machine completes cleanly. */
    LoRa_gotoMode(&myLoRa, SLEEP_MODE);
}
