/*
 * power_management.c
 *
 *  Created on: May 25, 2026
 *      Author: MSI GF63
 */

#include "power_management.h"
//
//void Power_DeepSleep(RTC_HandleTypeDef *hrtc, uint32_t seconds)
//{
//    RTC_AlarmTypeDef sAlarm = {0};
//    RTC_TimeTypeDef sTime = {0};
//    RTC_DateTypeDef sDate = {0};
//
//    /* 1. Get current time */
//    HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);
//    HAL_RTC_GetDate(hrtc, &sDate, RTC_FORMAT_BIN); // Must read date to unlock RTC for F1/F4
//
//    /* 2. Calculate Alarm Time (Simple logic for lab testing < 60s) */
//    sAlarm.AlarmTime.Hours = sTime.Hours;
//    sAlarm.AlarmTime.Minutes = sTime.Minutes;
//    sAlarm.AlarmTime.Seconds = sTime.Seconds + seconds;
//
//    /* Handle second overflow (e.g., 55s + 10s = 65s -> 1min 5sec) */
//    if (sAlarm.AlarmTime.Seconds >= 60)
//    {
//        sAlarm.AlarmTime.Seconds -= 60;
//        sAlarm.AlarmTime.Minutes += 1;
//    }
//    if (sAlarm.AlarmTime.Minutes >= 60)
//    {
//        sAlarm.AlarmTime.Minutes -= 60;
//        sAlarm.AlarmTime.Hours += 1;
//    }
//
//    sAlarm.Alarm = RTC_ALARM_A;
//
//    /* 3. Set the RTC Alarm */
//    HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BIN);
//
//    /* 4. Clear the Wake-Up Flag so the system can actually sleep */
//    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
//
//    /* 5. Enter STANDBY Mode (System dies here until alarm rings) */
//    HAL_PWR_EnterSTANDBYMode();
//}

void Power_DeepSleep(RTC_HandleTypeDef *hrtc, uint32_t seconds)
{
    RTC_AlarmTypeDef sAlarm = {0};
    RTC_TimeTypeDef sTime = {0};

    /* 1. Get the current time from the 32-bit hardware counter */
    HAL_RTC_GetTime(hrtc, &sTime, RTC_FORMAT_BIN);

    /* 2. Calculate the future Alarm Time using proper division and modulo math */
    uint32_t total_seconds = sTime.Seconds + seconds;
    uint32_t total_minutes = sTime.Minutes + (total_seconds / 60);
    uint32_t total_hours   = sTime.Hours   + (total_minutes / 60);

    /* Assign properly bounded values to the F1 time sub-structure */
    sAlarm.AlarmTime.Seconds = total_seconds % 60;
    sAlarm.AlarmTime.Minutes = total_minutes % 60;
    sAlarm.AlarmTime.Hours   = total_hours   % 24;

    /* 3. For STM32F1, the Alarm target is selected by assigning the identifier */
    sAlarm.Alarm = RTC_ALARM_A;

    /* 4. Set the RTC Alarm (The F1 HAL driver handles converting this back to a 32-bit match value) */
    HAL_RTC_SetAlarm_IT(hrtc, &sAlarm, RTC_FORMAT_BIN);

    /* 5. Clear the Wake-Up Flag so the system can actually sleep */
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    /* 6. Enter STANDBY Mode */
    HAL_PWR_EnterSTANDBYMode();
}
