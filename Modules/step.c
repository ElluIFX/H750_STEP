/**
 * @file step.c
 * @brief  步进电机驱动
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2022-12-12
 *
 * THINK DIFFERENTLY
 */

#include "step.h"

#include "uartPack.h"

/**
 * @brief 初始化步进电机
 * @param  step             步进电机控制结构体
 * @param  timMaster        主定时器句柄(用于PWM输出)
 * @param  timSlave         从定时器句柄(用于脉冲计数)
 * @param  timMasterCh      主定时器PWM通道
 * @param  dirPort          方向控制端口
 * @param  dirPin           方向控制引脚
 */
void Step_Init(step_ctrl_t *step, TIM_HandleTypeDef *timMaster,
               TIM_HandleTypeDef *timSlave, uint32_t timMasterCh,
               GPIO_TypeDef *dirPort, uint16_t dirPin) {
  step->speed = 0;
  step->angle = 0;
  step->angleTarget = 0;
  step->rotating = 0;
  step->dir = 0;
  step->timMaster = timMaster;
  step->timSlave = timSlave;
  step->timMasterCh = timMasterCh;
  step->dirPort = dirPort;
  step->dirPin = dirPin;
  __HAL_TIM_SET_COUNTER(step->timMaster, 0);
  __HAL_TIM_SET_COUNTER(step->timSlave, 0);
}

/**
 * @brief 设置步进电机速度
 * @param  step             步进电机控制结构体
 * @param  speed            速度(单位:度/秒)
 */
void Step_Set_Speed(step_ctrl_t *step, double speed) {
  if (speed == 0) {
    printf("[ERROR] Step_Set_Speed: speed is 0\n");
    return;
  }
  double pulsePerSec = speed / 360 * STEP_PULSE_PER_ROUND;  // 等价于pwm频率
  if (pulsePerSec > STEP_PWM_MAX_FREQ) {
    pulsePerSec = STEP_PWM_MAX_FREQ;
  }
  uint32_t prescaler = STEP_TIM_BASE_CLK / pulsePerSec / 2;
  uint16_t period = 2;
  while (prescaler > 0xFFFF) {
    prescaler /= 2;
    period *= 2;
  }
  printf(
      "[DEBUG] Step_Set_Speed: speed = %f, pulsePerSec = %f, prescaler = %d, "
      "period = %d\n",
      speed, pulsePerSec, prescaler, period);
  __HAL_TIM_SET_PRESCALER(step->timMaster, prescaler - 1);
  __HAL_TIM_SET_AUTORELOAD(step->timMaster, period - 1);
  __HAL_TIM_SET_COMPARE(step->timMaster, step->timMasterCh, period / 2);
  __HAL_TIM_SET_COUNTER(step->timMaster, 0);  // 重置计数器
  step->speed = speed;
}

/**
 * @brief 在从定时器中断中调用
 * @param  step             步进电机控制结构体
 * @param  htim             中断定时器句柄
 */
void Step_IT_Handler(step_ctrl_t *step, TIM_HandleTypeDef *htim) {
  if (htim->Instance == step->timSlave->Instance) {
    if (__HAL_TIM_GET_FLAG(step->timSlave, TIM_FLAG_CC1) != RESET) {
      __HAL_TIM_CLEAR_FLAG(step->timSlave, TIM_FLAG_CC1);
      HAL_TIM_PWM_Stop_IT(step->timMaster, step->timMasterCh);
      HAL_TIM_Base_Stop_IT(step->timSlave);
      __HAL_TIM_SET_COUNTER(step->timSlave, 0);
      step->rotating = 0;
      step->angle = step->angleTarget;
      printf("[DEBUG] Step rotate done\n");
    }
  }
}

/**
 * @brief 旋转步进电机
 * @param  step             步进电机控制结构体
 * @param  angle            旋转角度(单位:度)(正数:顺时针, 负数:逆时针)
 */
void Step_Rotate(step_ctrl_t *step, double angle) {
  if (step->rotating) {
    printf("[ERROR] Step_Rotate: step is rotating\n");
    return;
  }
  if (angle == 0) {
    printf("[ERROR] Step_Rotate: angle is 0\n");
    return;
  }
  step->angleTarget = step->angle + angle;
  step->dir = angle > 0 ? 1 : 0;
#if STEP_DIR_LOGIC_REVERSE
  HAL_GPIO_WritePin(step->dirPort, step->dirPin,
                    step->dir ? GPIO_PIN_RESET : GPIO_PIN_SET);
#else
  HAL_GPIO_WritePin(step->dirPort, step->dirPin,
                    step->dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
#endif
  angle = fabs(angle);
  uint16_t targetPulse = angle * STEP_PULSE_PER_ROUND / 360;
  if (targetPulse < 2) {
    printf("[ERROR] Step_Rotate: targetPulse is too small\n");
    return;
  }
  printf("[DEBUG] Step_Rotate: angle = %f, targetPulse = %d, dir = %d\n", angle,
         targetPulse, step->dir);
  __HAL_TIM_SET_COUNTER(step->timMaster, 0);
  __HAL_TIM_SET_COUNTER(step->timSlave, 0);
  __HAL_TIM_SET_AUTORELOAD(step->timSlave, targetPulse - 1);
  HAL_TIM_Base_Start_IT(step->timSlave);
  HAL_TIM_PWM_Start_IT(step->timMaster, step->timMasterCh);
  step->rotating = 1;
}

/**
 * @brief 重设步进电机当前角度
 * @param  step             步进电机控制结构体
 * @param  angle            角度(单位:度)
 */
void Step_Set_Angle(step_ctrl_t *step, double angle) { step->angle = angle; }

/**
 * @brief 旋转步进电机到指定角度
 * @param  step             步进电机控制结构体
 * @param  angle            目标角度(单位:度)
 */
void Step_Rotate_Abs(step_ctrl_t *step, double angle) {
  Step_Rotate(step, angle - step->angle);
}

void Step_Stop(step_ctrl_t *step) {
  HAL_TIM_PWM_Stop_IT(step->timMaster, step->timMasterCh);
  HAL_TIM_Base_Stop_IT(step->timSlave);
  __HAL_TIM_SET_COUNTER(step->timSlave, 0);
  step->rotating = 0;
  double progress = (double)__HAL_TIM_GET_COUNTER(step->timSlave) /
                    (double)__HAL_TIM_GET_AUTORELOAD(step->timSlave);
  step->angle = (step->angleTarget - step->angle) * progress + step->angle;
}
