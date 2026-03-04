#ifndef ADS1115_H
#define ADS1115_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* -------------------- 配置宏 -------------------- */
#define ADS1115_OS                   (0x01 << 7)

/* 单端 */
#define ADS1115_MUX_AIN0             (0x04 << 4)
#define ADS1115_MUX_AIN1             (0x05 << 4)
#define ADS1115_MUX_AIN2             (0x06 << 4)
#define ADS1115_MUX_AIN3             (0x07 << 4)

/* 差分模式 */
#define ADS1115_MUX_DIFF_0_1         (0x00 << 4)
#define ADS1115_MUX_DIFF_0_3         (0x01 << 4)
#define ADS1115_MUX_DIFF_1_3         (0x02 << 4)
#define ADS1115_MUX_DIFF_2_3         (0x03 << 4)

#define ADS1115_PGA_TWOTHIRDS        (0x00 << 1)
#define ADS1115_PGA_ONE              (0x01 << 1)
#define ADS1115_PGA_TWO              (0x02 << 1)
#define ADS1115_PGA_FOUR             (0x03 << 1)
#define ADS1115_PGA_EIGHT            (0x04 << 1)
#define ADS1115_PGA_SIXTEEN          (0x07 << 1)

#define ADS1115_MODE                 (0x01)

#define ADS1115_DATA_RATE_128        (0x04 << 5)

#define ADS1115_COMP_MODE            (0x00 << 4)
#define ADS1115_COMP_POL             (0x00 << 3)
#define ADS1115_COMP_LAT             (0x00 << 2)
#define ADS1115_COMP_QUE             (0x03)

#define ADS1115_CONVER_REG           0x00
#define ADS1115_CONFIG_REG           0x01

#define ADS1115_TIMEOUT              100

/* -------------------- 函数声明 -------------------- */
HAL_StatusTypeDef ADS1115_Init(I2C_HandleTypeDef *handler, uint16_t setDataRate, uint16_t setPGA);
HAL_StatusTypeDef ADS1115_readSingleEnded(uint16_t muxPort, float *voltage);
HAL_StatusTypeDef ADS1115_readDifferential(uint16_t diffMux, float *voltage);

#ifdef __cplusplus
}
#endif

#endif /* ADS1115_H */
