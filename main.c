/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

/* ???????,????????? */
#include "dac8831.h"
#include "ads1115.h"
#include "usart.h"
#include "gpio.h"
#include "spi.h"
#include "i2c.h"

/* ---------------------------------------------------------------------------*/
/* ?????? (????????????.c???,????)                  */
/* ---------------------------------------------------------------------------*/
extern UART_HandleTypeDef huart2; /* ??? usart.c */
extern SPI_HandleTypeDef hspi1;   /* ??? spi.c */
extern I2C_HandleTypeDef hi2c1;   /* ??? i2c.c */

/* ---------------------------------------------------------------------------*/
/* ??????                                                               */
/* ---------------------------------------------------------------------------*/
uint8_t BT_RX;                 /* ?????? */
volatile uint8_t cur_mode = 0; /* 0:????, 1:????? */

/* ---------------------------------------------------------------------------*/
/* ????                                                                   */
/* ---------------------------------------------------------------------------*/
void SystemClock_Config(void);

/* ---------------------------------------------------------------------------*/
/* ??? printf ? USART2                                                    */
/* ---------------------------------------------------------------------------*/
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 10);
    return ch;
}

/* ---------------------------------------------------------------------------*/
/* ???????? (??????)                                            */
/* ---------------------------------------------------------------------------*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    /* ????? USART2 (?? PA9/PA10) */
    if (huart->Instance == USART2)
    {
        if (BT_RX == '1')
        {
            cur_mode = 0;
            // ?? 1,??????
        }
        else if (BT_RX == '2')
        {
            cur_mode = 1;
            // ?? 2,???????
        }
        
        /* ??:???????? */
        HAL_UART_Receive_IT(&huart2, &BT_RX, 1);
    }
}

/* ?????? (????) */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if(huart->Instance == USART2)
    {
        /* ??????????? */
        __HAL_UART_CLEAR_OREFLAG(huart);
        __HAL_UART_CLEAR_NEFLAG(huart);
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UART_Receive_IT(&huart2, &BT_RX, 1);
    }
}

/* ---------------------------------------------------------------------------*/
/* ???                                                                     */
/* ---------------------------------------------------------------------------*/
int main(void)
{
    /* 1. ??????? */
    HAL_Init();
    SystemClock_Config();

    /* 2. ????? (???? usart.c/gpio.c ???????) */
    MX_GPIO_Init();
    MX_USART2_UART_Init(); 
    MX_SPI1_Init();
    MX_I2C1_Init();

    /* ========================================================== */
    /* ???????? USART2 ??                                 */
    /* ???? usart.c ??????????                         */
    /* ========================================================== */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
    /* ========================================================== */

    /* 3. ???????? */
    DAC8831_Init();
    ADS1115_Init(&hi2c1, ADS1115_DATA_RATE_128, ADS1115_PGA_TWO);

    /* 4. ?????? */
    HAL_UART_Receive_IT(&huart2, &BT_RX, 1);

    printf("\r\n--- BEMS System Ready ---\r\n");

    /* 5. ?????? */
    const uint32_t T_CYCLE = 40000;
    uint32_t t_start = 0;
    float adc_val = 0;
    uint8_t last_mode = 0xFF; 

    /* 6. ??? */
    while (1)
    {
        uint32_t current_tick = HAL_GetTick();

        /* === ???:?????? === */
        if (cur_mode != last_mode)
        {
            if (cur_mode == 1)
            {
                /* ???:????? */
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET); // ??
                t_start = current_tick; // ????
                printf("CMD: Mode 2 (Electrochem)\r\n");
            }
            else
            {
                /* ???:???? */
                HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET); // ??
                DAC8831_WriteCode(32770); // 0V ??
                printf("CMD: Mode 1 (Pressure)\r\n");
            }
            last_mode = cur_mode;
            HAL_Delay(10); 
        }

        /* === ???? === */
        if (cur_mode == 1)
        {
            /* --- ??2:??? --- */
            uint32_t t = (current_tick - t_start) % T_CYCLE;
            float target_v;
            
            // ?????
            if (t < 20000) target_v = -1.0f + ((float)t / 20000.0f) * 2.0f;
            else           target_v = 1.0f - (((float)(t - 20000)) / 20000.0f) * 2.0f;

            // DAC ??
            uint16_t code;
            if (target_v >= 0) code = (uint16_t)(32770 + target_v * (52799 - 32770));
            else               code = (uint16_t)(32770 + target_v * (32770 - 9900));
            DAC8831_WriteCode(code);

            // ADS1115 ????
            static uint32_t last_adc_tick = 0;
            if (current_tick - last_adc_tick > 200)
            {
                last_adc_tick = current_tick;
                float sum = 0;
                for(int i=0; i<5; i++) {
                    float temp;
                    ADS1115_readDifferential(ADS1115_MUX_DIFF_2_3, &temp);
                    sum += temp;
                }
                adc_val = sum / 5.0f;
                printf("M:2|T:%lu|Set:%.2f|V:%.4f\r\n", t/1000, target_v, adc_val/1000.0f);
            }
        }
        else
        {
            /* --- ??1:?? --- */
            static uint32_t last_adc_tick = 0;
            if (current_tick - last_adc_tick > 200)
            {
                last_adc_tick = current_tick;
                float sum = 0;
                for(int i=0; i<5; i++) {
                    float temp;
                    ADS1115_readSingleEnded(ADS1115_MUX_AIN0, &temp);
                    sum += temp;
                }
                adc_val = sum / 5.0f;
                printf("M:1|V:%.4f\r\n", adc_val/1000.0f);
            }
        }
    }
}

/* ---------------------------------------------------------------------------*/
/* ?????? (???????? main.c)                                     */
/* ---------------------------------------------------------------------------*/
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLLMUL_4;
  RCC_OscInitStruct.PLL.PLLDIV = RCC_PLLDIV_2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* ?????? */
void Error_Handler(void)
{
  __disable_irq();
  while (1) {}
}

/* ??:??? MX_USART2_UART_Init, MX_GPIO_Init ???????? */
/* ????????? usart.c, gpio.c ??,?????,??? */