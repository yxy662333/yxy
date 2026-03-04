/**
  ******************************************************************************
  * @file           : dac8831.h
  * @brief          : Header for dac8831.c file.
  *                   This file contains the common defines of the application
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
/* dac8831.h — ??? STM32L031(SPI1 + GPIO ??)*/

#ifndef __DAC8831_H
#define __DAC8831_H

#include "main.h"
#include "stm32l0xx_hal.h"
#include "stm32l0xx_hal_spi.h"
/************ 4 ? SPI ????(???????) ************/

#define DAC8831_CS_GPIO_Port    GPIOA
#define DAC8831_CS_Pin          GPIO_PIN_4

#define DAC8831_SCK_GPIO_Port   GPIOA
#define DAC8831_SCK_Pin         GPIO_PIN_5

#define DAC8831_MISO_GPIO_Port  GPIOA
#define DAC8831_MISO_Pin        GPIO_PIN_6

#define DAC8831_MOSI_GPIO_Port  GPIOA
#define DAC8831_MOSI_Pin        GPIO_PIN_7

void DAC8831_Init(void);
void DAC8831_SetVoltage(float volt);

/************ SPI ??(? DAC8831.c ??) ************/
extern SPI_HandleTypeDef hspi1;


/************ ?????? ************/
void DAC8831_Init(void);
void DAC8831_WriteCode(uint16_t code);
void DAC8831_SetVoltage(float volt);    // ?? +1 ? -1 V ??
uint16_t DAC8831_VoltToCode(float volt);

#endif
