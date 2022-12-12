/**
 * @file candy.h
 * @brief see candy.c for details.
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-23
 *
 * THINK DIFFERENTLY
 */

#ifndef CANDY_H
#define CANDY_H
/***** settings *****/
#define _RGB_LED_USED_ 1
#define _WS2812_USED_ 0
#define _DELAYUS_USED_ 0

/**** end settings ****/
#include <main.h>

#include "tim.h"

// HMI screen

#define S_END_BIT "\xff\xff\xff"
#define screen(x, ...) printft(&huart2, x, __VA_ARGS__)

// delay_us
#if _DELAYUS_USED_
#define _DELAY_TIM htim6
void delay_us(uint16_t us);
#endif
// single func functions

float fmap(float x, float in_min, float in_max, float out_min, float out_max);

// RGB LED functions
#if _RGB_LED_USED_
void RGB(uint8_t r, uint8_t g, uint8_t b);
#endif  // _RGB_LED_USED_
// WS2812
#if _WS2812_USED_
#define WS2812_PIN_DEF WS2812_GPIO_Port, WS2812_Pin
#define __DELAY_350NS for (uint8_t i = 0; i < 8; i++)
#define __DELAY_600NS for (uint8_t i = 0; i < 13; i++)
#define __DELAY_700NS for (uint8_t i = 0; i < 20; i++)
#define __DELAY_800NS for (uint8_t i = 0; i < 19; i++)
#define __2812_HIGH_BIT                              \
  HAL_GPIO_WritePin(WS2812_PIN_DEF, GPIO_PIN_SET);   \
  __DELAY_700NS;                                     \
  HAL_GPIO_WritePin(WS2812_PIN_DEF, GPIO_PIN_RESET); \
  __DELAY_600NS
#define __2812_LOW_BIT                               \
  HAL_GPIO_WritePin(WS2812_PIN_DEF, GPIO_PIN_SET);   \
  __DELAY_350NS;                                     \
  HAL_GPIO_WritePin(WS2812_PIN_DEF, GPIO_PIN_RESET); \
  __DELAY_800NS
#define __2812_RESET delay_us(70)
#define __SET_2312(d, t) \
  WS2812_SendBit((uint8_t*)d, sizeof(d) / sizeof(uint8_t) / 3, t)
void WS2812_SendBit(uint8_t* data, uint8_t len, float br);
#endif  // _WS2812_USED_

#endif  // CANDY_H
