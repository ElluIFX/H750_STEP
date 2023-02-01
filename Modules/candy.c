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

/**
 * @brief Linear mapping input to the specified range
 * @param  x                input value
 * @param  in_min           input range min
 * @param  in_max           input range max
 * @param  out_min          output range min
 * @param  out_max          output range max
 * @retval Mapping result
 */
double dmap(double x, double in_min, double in_max, double out_min,
            double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#if _DELAYUS_USED_
#include "tim.h"
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

#if _RGB_LED_USED_
/**
 * @brief control RGB led
 * @param uint8_t RGBstat
 * @retval None
 * @note RGBstat = 0x00, RGB off, RGBstat = 0x01, R on, RGBstat = 0xff, ignore
 **/
void RGB(uint8_t R, uint8_t G, uint8_t B) {
  if (R != 0xff)
    HAL_GPIO_WritePin(LED_R_GPIO_Port, LED_R_Pin,
                      R ? GPIO_PIN_RESET : GPIO_PIN_SET);
  if (G != 0xff)
    HAL_GPIO_WritePin(LED_G_GPIO_Port, LED_G_Pin,
                      G ? GPIO_PIN_RESET : GPIO_PIN_SET);
  if (B != 0xff)
    HAL_GPIO_WritePin(LED_B_GPIO_Port, LED_B_Pin,
                      B ? GPIO_PIN_RESET : GPIO_PIN_SET);
}
#endif  // _RGB_LED_USED_
