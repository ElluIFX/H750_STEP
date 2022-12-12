/**
 * @file candy.c
 * @brief 一些有用的小函数
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-23
 *
 * THINK DIFFERENTLY
 */

#include "candy.h"

/**
 * @brief Linear mapping input to the specified range
 * @param  x                input value
 * @param  in_min           input range min
 * @param  in_max           input range max
 * @param  out_min          output range min
 * @param  out_max          output range max
 * @retval Mapping result
 */
float fmap(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#if _DELAYUS_USED_
/**
 * @brief Delay_us, usint a timer prescale to 1Mhz, reloadRegister = 1
 * @param  us               delay time in us
 */
void delay_us(uint16_t us) {
  uint16_t differ = 0xffff - us - 5;
  HAL_TIM_Base_Start(&_DELAY_TIM);
  __HAL_TIM_SetCounter(&_DELAY_TIM, differ);
  while (differ < 0xffff - 5) {
    differ = __HAL_TIM_GetCounter(&_DELAY_TIM);
  }
  HAL_TIM_Base_Stop(&_DELAY_TIM);
}
#endif

#if _WS2812_USED_
/**
 * @brief Send data bits to WS2812, can be used only when MCU's SYSCLK is
 * 168MHz.
 * @param  data             uint32_t LED array, each in Br G R B
 * order(eg.0xffffffff as White), Br is brightness, using OR to combine
 * Total length equals number of LEDs, the first LED in left.
 * @param  len              the number of LEDs
 * @param  br              brightness, 0-1
 */
void WS2812_SendBit(uint8_t* data, uint8_t len, float br) {
  static uint8_t _i = 0;
  static uint8_t _j = 0;
  __IO float div = br;
  static uint8_t temp = 0;
  __disable_irq();  // disable all interrupts
  for (_i = 0; _i < len * 3; _i++) {
    temp = *data++;
    temp *= div;
    for (_j = 0; _j < 8; _j++) {
      if (temp & (128 >> _j)) {
        __2812_HIGH_BIT;
      } else {
        __2812_LOW_BIT;
      }
    }
  }
  __enable_irq();  // enable all interrupts
}
#endif  // _WS2812_USED_

#if _RGB_LED_USED_
/**
 * @brief control RGB led
 * @param uint8_t RGBstat
 * @retval None
 * @note RGBstat = 0x00, RGB off
 **/
void RGB(uint8_t R, uint8_t G, uint8_t B) {
  HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin,
                    R ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin,
                    G ? GPIO_PIN_RESET : GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin,
                    B ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
#endif  // _RGB_LED_USED_
