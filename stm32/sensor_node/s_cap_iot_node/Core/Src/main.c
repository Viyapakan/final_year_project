/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "utils.h"
#include "lora_config.h"
#include "rs485_config.h"

#include <stdio.h>
#include <string.h>
#include <stddef.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define LORA_MAX_PAYLOAD      48
#define SENSOR_TYPE_SOIL      0x01
#define SENSOR_TYPE_ENV       0x02

/* -------------------------------------------------------------------------- */
/*                          General LoRa Packet Format                        */
/* -------------------------------------------------------------------------- */
typedef struct __attribute__((packed))
{
    uint32_t device_id;
    uint8_t  sensor_type;
    uint8_t  payload_length;
    uint8_t  payload[LORA_MAX_PAYLOAD];

} lora_packet_t;

/* -------------------------------------------------------------------------- */
/*                        Soil Sensor Payload Structure                       */
/* -------------------------------------------------------------------------- */
typedef struct __attribute__((packed))
{
    uint16_t humidity;
    int16_t  temperature;
    uint16_t ec;

} soil_payload_t;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  printf("\r\n\r\n=== System Booting ===\r\n");
  // THIS DELAY IS CRITICAL FOR LORA!
      led_on();
      HAL_Delay(500);
      led_off();
      HAL_Delay(500);

    /* ---------------------------------------------------------------------- */
    /*                           Initialize LoRa                              */
    /* ---------------------------------------------------------------------- */
    uint16_t lora_status = lora_init();

    printf("Lora returned Value : %d", lora_status);
    /* LoRa status indication */
    if (lora_status != 0)
    {
        /* Success → Fast LED blink */
        led_toggle(100, 5);
        printf("\r\n\r\n=== LoRa Boot Done! ===\r\n");
    }
    else
    {
        /* Failure → Solid LED ON */
        led_on();
    }

    /* ---------------------------------------------------------------------- */
    /*                         Initialize RS485 Sensor                         */
    /* ---------------------------------------------------------------------- */
    RS485_Init();

    RS485_SensorReading_t currentReading = {0};
    printf("\r\n\r\n=== While Loop Execution from this point! ===\r\n");
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    while (1)
    {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

        /* ------------------------------------------------------------------ */
        /*                         Sensor Read Interval                       */
        /* ------------------------------------------------------------------ */
        HAL_Delay(25000);

        /* ------------------------------------------------------------------ */
        /*                         Read Soil Sensor                           */
        /* ------------------------------------------------------------------ */
        if (RS485_ReadSoilSensor(&currentReading) == HAL_OK)
        {
            /* -------------------------------------------------------------- */
            /*                    Create LoRa Packet                          */
            /* -------------------------------------------------------------- */
            lora_packet_t tx_packet = {0};

            tx_packet.device_id   = Get_STM32_UniqueID();
            tx_packet.sensor_type = SENSOR_TYPE_SOIL;

            /* -------------------------------------------------------------- */
            /*                  Populate Soil Sensor Payload                  */
            /* -------------------------------------------------------------- */
            soil_payload_t soil_data;

            soil_data.temperature = currentReading.temperature;
            soil_data.humidity    = currentReading.moisture;
            soil_data.ec          = currentReading.ec;

            /* -------------------------------------------------------------- */
            /*                Copy Payload into LoRa Packet                   */
            /* -------------------------------------------------------------- */
            tx_packet.payload_length = sizeof(soil_payload_t);

            memcpy(
                tx_packet.payload,
                &soil_data,
                tx_packet.payload_length
            );

            /* -------------------------------------------------------------- */
            /*               Calculate Actual Transmission Size               */
            /* -------------------------------------------------------------- */
            uint8_t tx_size =
                offsetof(lora_packet_t, payload) +
                tx_packet.payload_length;

            /* -------------------------------------------------------------- */
            /*                       Send via LoRa                            */
            /* -------------------------------------------------------------- */
            lora_send((uint8_t *)&tx_packet, tx_size, 1000);

            /* -------------------------------------------------------------- */
            /*                    Transmission Indication                     */
            /* -------------------------------------------------------------- */
            printf("\r\n========== LoRa Packet Sent ==========\r\n");

            printf("Device ID     : %lu\r\n", tx_packet.device_id);
            printf("Sensor Type   : %u\r\n", tx_packet.sensor_type);
            printf("Payload Length: %u\r\n", tx_packet.payload_length);

            printf("\r\n--- Soil Sensor Data ---\r\n");

            printf("Temperature : %d\r\n", soil_data.temperature);
            printf("Humidity    : %u\r\n", soil_data.humidity);
            printf("EC           : %u\r\n", soil_data.ec);

            printf("\r\n--- Raw Packet Bytes ---\r\n");

            for(uint8_t i = 0; i < tx_size; i++)
            {
                printf("0x%02X ", ((uint8_t *)&tx_packet)[i]);
            }

            printf("\r\n======================================\r\n\r\n");
            led_off();
            led_toggle(50, 2);
        }
        else
        {
            /* Sensor Read Failed */
            led_on();
        }

    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */

    __disable_irq();

    while (1)
    {
    }

  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */

    /* Example:
       printf("Wrong parameters value: file %s on line %d\r\n", file, line);
    */

  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
