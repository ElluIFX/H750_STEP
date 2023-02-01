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

#include "uart_pack.h"

/**
 * @brief 初始化步进电机
 * @param  step             步进电机控制结构体
 * @param  timMaster        主定时器句柄(用于PWM输出)
 * @param  timSlave         从定时器句柄(用于脉冲计数)
 * @param  timMasterCh      主定时器PWM通道
 * @param  dirPort          方向控制端口
 * @param  dirPin           方向控制引脚
 * @param  dirLogic         方向控制逻辑
 */
void Step_Init(step_ctrl_t *step, TIM_HandleTypeDef *timMaster,
               TIM_HandleTypeDef *timSlave, uint32_t timMasterCh,
               GPIO_TypeDef *dirPort, uint16_t dirPin, uint8_t dirLogic) {
  step->speed = 0;
  step->angle = 0;
  step->angleTarget = 0;
  step->rotating = 0;
  step->dir = 0;
  step->slaveTimReload = 0;
  step->slaveTimITCnt = 0;
  step->timMaster = timMaster;
  step->timSlave = timSlave;
  step->timMasterCh = timMasterCh;
  step->dirPort = dirPort;
  step->dirPin = dirPin;
  step->dirLogic = dirLogic;
  __HAL_TIM_SET_COUNTER(step->timMaster, 0);
  __HAL_TIM_SET_COUNTER(step->timSlave, 0);
}

/**
 * @brief 设置步进电机速度
 * @param  step             步进电机控制结构体
 * @param  speed            速度(单位:度/秒)
 */
void Step_Set_Speed(step_ctrl_t *step, double speed) {
  if (speed < 0.01 && speed > -0.01) {
    LOG_E("[STEP] setspeed=0");
    return;
  }
  speed = fabs(speed);
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
  // LOG_D("Step_Set_Speed: speed = %f, pwmFreq = %f, PSC = %d, ARR = %d",
  // speed,
  //       pulsePerSec, prescaler, period);
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
      if (step->rotating) {
        if (step->slaveTimITCnt == 0) {  // 旋转结束
        LRotateFinish:
          HAL_TIM_PWM_Stop_IT(step->timMaster, step->timMasterCh);
          HAL_TIM_Base_Stop_IT(step->timSlave);
          step->rotating = 0;
          step->angle = step->angleTarget;
          LOG_D("[STEP] Stop");
          return;
        } else {  // 从定时器溢出
          step->slaveTimITCnt--;
          if (step->slaveTimITCnt == 0) {  // 不存在下一次溢出
            if (step->slaveTimReload != 0) {
              __HAL_TIM_SET_AUTORELOAD(step->timSlave, step->slaveTimReload);
            } else {
              goto LRotateFinish;
            }
          }
        }
      }
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
    LOG_E("[STEP] step busy");
    return;
  }
  step->dir = angle > 0 ? 1 : 0;
  if (!step->dirLogic)
    HAL_GPIO_WritePin(step->dirPort, step->dirPin,
                      step->dir ? GPIO_PIN_RESET : GPIO_PIN_SET);
  else
    HAL_GPIO_WritePin(step->dirPort, step->dirPin,
                      step->dir ? GPIO_PIN_SET : GPIO_PIN_RESET);
  angle = fabs(angle);
  uint32_t targetPulse = angle * STEP_PULSE_PER_ROUND / 360;
  if (targetPulse < 2) {
    LOG_E("[STEP] targetPulse<2");
    return;
  }
  // step->angleTarget = step->angle + angle; // 存在累加误差
  step->angleTarget = (double)targetPulse * 360 / STEP_PULSE_PER_ROUND;
  step->angleTarget = step->dir ? step->angle + step->angleTarget
                                : step->angle - step->angleTarget;
  // LOG_D("Step_Rotate: angle = %f, pulse = %ld, dir = %d, angleT = %f", angle,
  //       targetPulse, step->dir, step->angleTarget);
  step->slaveTimITCnt = targetPulse / (STEP_SLAVE_TIM_MAX_CNT + 1);
  if (step->slaveTimITCnt > 0) {
    step->slaveTimReload = targetPulse % (STEP_SLAVE_TIM_MAX_CNT + 1);
    targetPulse = STEP_SLAVE_TIM_MAX_CNT;
    // LOG_D("Step_Rotate: slaveTim ITCnt = %d, Reload = %d",
    // step->slaveTimITCnt,
    //       step->slaveTimReload);
  }
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

/**
 * @brief 手动停止步进电机
 * @param  step           步进电机控制结构体
 */
void Step_Stop(step_ctrl_t *step) {
  HAL_TIM_PWM_Stop_IT(step->timMaster, step->timMasterCh);
  HAL_TIM_Base_Stop_IT(step->timSlave);
  step->angle = Step_Get_Angle(step);
  __HAL_TIM_SET_COUNTER(step->timSlave, 0);
  step->rotating = 0;
  LOG_I("[STEP] manully stop");
}

/**
 * @brief 获取步进电机当前角度
 * @param  step             步进电机控制结构体
 * @retval
 */
double Step_Get_Angle(step_ctrl_t *step) {
  if (!step->rotating) return step->angle;
  double progress = (double)__HAL_TIM_GET_COUNTER(step->timSlave) /
                    (double)__HAL_TIM_GET_AUTORELOAD(step->timSlave);
  return (step->angleTarget - step->angle) * progress + step->angle;
}
