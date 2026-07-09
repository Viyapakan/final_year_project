#include "utils.h"
#include "usart.h"

void led_off(void)
{
//    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void led_on(void)
{
//    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void led_toggle(uint32_t delay_ms, uint8_t times)
{
//    for (uint8_t i = 0; i < times; i++)
//    {
//        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//        HAL_Delay(delay_ms);
//        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
//        HAL_Delay(delay_ms);
//    }
}



uint32_t Get_STM32_UniqueID(void)
{
    // STM32F1xx Unique Device ID register base address
    uint32_t uid_word0 = *(uint32_t*)0x1FFFF7E8;
    uint32_t uid_word1 = *(uint32_t*)0x1FFFF7EC;
    uint32_t uid_word2 = *(uint32_t*)0x1FFFF7F0;

    // Compress 96-bits into a 32-bit ID using XOR
    return (uid_word0 ^ uid_word1 ^ uid_word2);
}


///* utils.c */
//bool switch_operation(SwitchTarget_t target, SwitchState_t state)
//{
//    GPIO_TypeDef* port;
//    uint16_t pin;
//
//    // 1. Map target to the CubeMX generated pins
//    if (target == TARGET_LORA) {
//        port = LORA_SWITCH_GPIO_Port;
//        pin  = LORA_SWITCH_Pin;
//    } else if (target == TARGET_SENSOR) {
//        port = SENSOR_SWITCH_GPIO_Port;
//        pin  = SENSOR_SWITCH_Pin;
//    } else {
//        return false;
//    }
//
//    // 2. Execute switch (P-Channel Logic: LOW = ON, HIGH = OFF)
//    if (state == SWITCH_ON) {
//        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
//    } else {
//        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
//    }
//
//    // 3. Give the capacitors on the LoRa/Sensor modules time to fill up
//    HAL_Delay(20);
//
//    // 4. Verify the pin actually changed state
//    GPIO_PinState current_state = HAL_GPIO_ReadPin(port, pin);
//
//    if ((state == SWITCH_ON) && (current_state == GPIO_PIN_RESET)) {
//        return true;
//    }
//    else if ((state == SWITCH_OFF) && (current_state == GPIO_PIN_SET)) {
//        return true;
//    }
//
//    return false;
//}


bool switch_operation(SwitchTarget_t target, SwitchState_t state)
{
    GPIO_TypeDef* port;
    uint16_t pin;

    // 1. Map target to the CubeMX generated pins
    if (target == TARGET_LORA) {
        port = LORA_SWITCH_GPIO_Port;
        pin  = LORA_SWITCH_Pin;
    } else if (target == TARGET_SENSOR) {
        port = SENSOR_SWITCH_GPIO_Port;
        pin  = SENSOR_SWITCH_Pin;
    } else {
        printf("[ERROR] switch_operation: Invalid target!\r\n");
        return false;
    }

    // 2. Execute switch (P-Channel Logic: LOW = ON, HIGH = OFF)
    if (state == SWITCH_ON) {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
    }

    // 3. Give the capacitors on the LoRa/Sensor modules time to fill up
    HAL_Delay(20);

    // 4. Verify the pin actually changed state
    GPIO_PinState current_state = HAL_GPIO_ReadPin(port, pin);

    // Debug Print
    printf("[DEBUG] switch_operation: Target=%s, Requested=%s, PinRead=%s\r\n",
           (target == TARGET_LORA) ? "LORA" : "SENSOR",
           (state == SWITCH_ON) ? "ON" : "OFF",
           (current_state == GPIO_PIN_RESET) ? "LOW (ON)" : "HIGH (OFF)");

    if ((state == SWITCH_ON) && (current_state == GPIO_PIN_RESET)) {
        return true;
    }
    else if ((state == SWITCH_OFF) && (current_state == GPIO_PIN_SET)) {
        return true;
    }

    printf("[ERROR] switch_operation: Pin state verification failed!\r\n");
    return false;
}
/* ========================================================== */
/* ============ PRINTF REDIRECTION TO USART1 ================ */
/* ========================================================== */

// This tells the GCC compiler how to route standard character output
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    // Transmit one character at a time over USART1
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
