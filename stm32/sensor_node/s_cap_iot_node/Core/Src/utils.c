#include "utils.h"

void led_off(void)
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
}

void led_on(void)
{
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
}

void led_toggle(uint32_t delay_ms, uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(delay_ms);
        HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
        HAL_Delay(delay_ms);
    }
}


void Enter_Standby_RTC(uint32_t seconds)
{
    // 1. Enable Power Clock & Backup Access
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // 2. DISABLE the WKUP Pin (PA0) to avoid noise waking us up
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);

    // 3. Wait for RTC registers to synchronize (Critical for F103)
    // We must wait for the RSF bit to be set before reading/writing
    while (!(RTC->CRL & RTC_CRL_RSF));

    // 4. Calculate Alarm Time
    // On F103, CNTH/CNTL are the current counter values
    uint32_t current_counter = (RTC->CNTH << 16) | RTC->CNTL;
    uint32_t alarm_counter = current_counter + seconds;

    // 5. Set the Alarm
    RTC_AlarmTypeDef sAlarm = {0};
    sAlarm.Alarm = alarm_counter;

    // HAL_RTC_SetAlarm_IT automatically handles the RTOFF wait
    if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
    {
        Error_Handler();
    }

    // 6. CLEAR FLAGS (The Source of the "Fast Loop" Bug)

    // A. Clear Power Wakeup Flag
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    // B. Clear RTC Alarm Flag
    // IMPORTANT: For F103, we must wait for the previous write to finish first!
    while (!(RTC->CRL & RTC_CRL_RTOFF));
    __HAL_RTC_ALARM_CLEAR_FLAG(&hrtc, RTC_FLAG_ALRAF);

    // C. Wait for the Clear command to actually finish
    while (!(RTC->CRL & RTC_CRL_RTOFF));

    // 7. Enter Standby Mode
    HAL_PWR_EnterSTANDBYMode();
}
