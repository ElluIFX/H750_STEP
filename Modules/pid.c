/**
 * @file pid.c
 * @brief PID控制器，目前整合了位置PID和速度PID，以及一个没啥用的增量式PID，
 * 封装了电机控制函数，包括编码器读速度和读角度，需要根据实际情况修改的参量都在pid.h里面
 * 位置环可以不使能，只运行速度环的RUN函数就是单速度环。
 * 我也不知道为什么我写注释一下英文一下中文，麻了
 * @author Ellu (lutaoyu@163.com)
 * @version 2.0
 * @date 2021-12-18
 *
 * THINK DIFFERENTLY
 */

#include "pid.h"

#include "candy.h"
/****************** Position PID Functions ******************/

/**
 * @brief Initialize Position PID struct
 * @param  PIDx             Target
 */
void Pos_PID_Param_Init(pos_pid_t *PIDx) {
  PIDx->setPoint = 0;
  PIDx->sumError = 0;
  PIDx->error_1 = 0;
  PIDx->error_2 = 0;
  PIDx->proportion = POS_KP;
  PIDx->integral = POS_KI;
  PIDx->derivative = POS_KD;
  PIDx->deadBand = POS_DEAD_BAND;
  PIDx->maxI = POS_MAX_I;
}

/**
 * @brief Calculate Position PID
 * @param  PIDx             Target
 * @param  nextPoint        Current input
 * @retval double           PID output
 */
double Pos_PID_Calc(pos_pid_t *PIDx, int32_t nextPoint) {
  static int32_t error_0 = 0;
  static double output = 0;
  output = 0;
  error_0 = PIDx->setPoint - nextPoint;
  /* Dead band */
  if (error_0 > PIDx->deadBand || error_0 < -PIDx->deadBand) {
    /* Proportion */
    output += PIDx->proportion * error_0;
    /* Integral */
    if (error_0 < PIDx->maxI && error_0 > -PIDx->maxI) {
      PIDx->sumError += error_0;
      if (PIDx->sumError > PIDx->maxI) {
        PIDx->sumError = PIDx->maxI;
      } else if (PIDx->sumError < -PIDx->maxI) {
        PIDx->sumError = -PIDx->maxI;
      }
    }
    output += PIDx->integral * PIDx->sumError;
  } else {
    PIDx->sumError = 0;
    error_0 = 0;
  }
  /* Derivative */
  output += PIDx->derivative * (error_0 - PIDx->error_1);

  PIDx->error_2 = PIDx->error_1;
  PIDx->error_1 = error_0;
  return output;
}

/****************** Speed PID Functions ******************/

/**
 * @brief Initialize Speed PID struct
 * @param  PIDx             Target
 */
void Spd_PID_Param_Init(spd_pid_t *PIDx) {
  PIDx->setPoint = 0;
  PIDx->sumError = 0;
  PIDx->error_1 = 0;
  PIDx->error_2 = 0;
  PIDx->proportion = SPD_KP;
  PIDx->integral = SPD_KI;
  PIDx->derivative = SPD_KD;
  PIDx->deadBand = SPD_DEAD_BAND;
  PIDx->maxIMult = SPD_MAX_I_MULTI;
}

/**
 * @brief Calculate Speed PID
 * @param  PIDx             Target
 * @param  nextPoint        Current input
 * @retval double           PID output
 */
double Spd_PID_Calc(spd_pid_t *PIDx, double nextPoint) {
  static double error_0 = 0;
  static double output = 0;
  output = 0;
  error_0 = PIDx->setPoint - nextPoint;
  /* Dead band */
  if (error_0 < PIDx->deadBand && error_0 > -PIDx->deadBand) {
    error_0 = 0;
    PIDx->sumError = 0;
  }
  /* Proportion */
  PIDx->sumError += error_0;
  output += PIDx->proportion * error_0;
  /* Integral */
  if (PIDx->setPoint >= 0 && PIDx->sumError > PIDx->maxIMult * PIDx->setPoint) {
    PIDx->sumError = PIDx->maxIMult * PIDx->setPoint;
  } else if (PIDx->setPoint < 0 &&
             PIDx->sumError < PIDx->maxIMult * PIDx->setPoint) {
    PIDx->sumError = PIDx->maxIMult * PIDx->setPoint;
  }
  output += PIDx->integral * PIDx->sumError;
  /* Derivative */
  output += PIDx->derivative * (error_0 - PIDx->error_1);

  PIDx->error_2 = PIDx->error_1;
  PIDx->error_1 = error_0;
  return output;
}

/****************** Incremental PID Functions ******************/

/**
 * @brief Initialize Incremental PID struct
 * @param  PIDx             Target
 */
void Inc_PID_Param_Init(inc_pid_t *PIDx) {
  PIDx->setPoint = 0;
  PIDx->sumError = 0;
  PIDx->error_1 = 0;
  PIDx->error_2 = 0;
  PIDx->proportion = INC_KP;
  PIDx->integral = INC_KI;
  PIDx->derivative = INC_KD;
  PIDx->max_inc = INC_MAX_INC;
}

/**
 * @brief Calculate Incremental PID
 * @param  PIDx             Target
 * @param  nextPoint        Current input
 * @retval double           PID increment
 */
double Inc_PID_Calc(inc_pid_t *PIDx, double nextPoint) {
  double error_0, inc;
  error_0 = PIDx->setPoint - nextPoint;
  inc =  //增量计算
      PIDx->proportion * (error_0 - PIDx->error_1) + PIDx->integral * error_0 +
      PIDx->derivative * (error_0 - 2 * PIDx->error_1 + PIDx->error_2);
  if (PIDx->max_inc > 0) {
    if (inc < -PIDx->max_inc) inc = -PIDx->max_inc;
    if (inc > PIDx->max_inc) inc = PIDx->max_inc;
  }
  PIDx->error_2 = PIDx->error_1;
  PIDx->error_1 = error_0;
  return (inc);
}

/****************** All PID Functions End ******************/

/**
 * @brief Setup motor structure and configure Timers.
 * @param  motor            Target
 * @param  timEncoder       Timer Handle for encoder
 * @param  timPWM           Timer Handle for PWM
 * @param  forwardChannel   PWM channel for forward
 * @param  reverseChannel   PWM channel for reverse
 */
void Motor_Setup(motor_t *motor, TIM_HandleTypeDef *timEncoder,
                 TIM_HandleTypeDef *timPWM, uint32_t forwardChannel,
                 uint32_t reverseChannel) {
  Pos_PID_Param_Init(&motor->posPID);
  motor->posPID.setPoint = 0;
  Spd_PID_Param_Init(&motor->spdPID);
  motor->timEncoder = timEncoder;
  motor->timPWM = timPWM;
  motor->forwardChannel = forwardChannel;
  motor->reverseChannel = reverseChannel;
  motor->posTargetSpd = POS_INIT_TARGET_SPEED;
  motor->pwmDuty = 0;
  motor->speed = 0;
  motor->pos = 0;
  motor->lastPos = 0;
  HAL_TIM_Encoder_Start(timEncoder, TIM_CHANNEL_ALL);
  __HAL_TIM_SET_COUNTER(timEncoder, ENCODER_MID_VALUE);
  __HAL_TIM_SET_PRESCALER(timPWM,
                          (SYSTEM_CLOCK_FREQ_HZ / 1000 / MOTOR_PWM_FREQ) - 1);
  __HAL_TIM_SET_AUTORELOAD(timPWM, 1000 - 1);
  __HAL_TIM_SET_COMPARE(timPWM, forwardChannel, 0);
  __HAL_TIM_SET_COMPARE(timPWM, reverseChannel, 0);
  HAL_TIM_PWM_Start(timPWM, forwardChannel);
  HAL_TIM_PWM_Start(timPWM, reverseChannel);
}

/**
 * @brief Get motor speed and position
 * @param  motor            Target
 * @param  runTimeHz        How fast this function is called
 */
void Motor_Update_Speed(motor_t *motor, double runTimeHz) {
  static double speed = 0;
  motor->pos +=
      (int32_t)__HAL_TIM_GET_COUNTER(motor->timEncoder) - ENCODER_MID_VALUE;
  __HAL_TIM_SET_COUNTER(motor->timEncoder, ENCODER_MID_VALUE);
  speed = (double)(motor->pos - motor->lastPos) * 60.0 * runTimeHz /
          PULSE_PER_ROTATION;
  motor->lastPos = motor->pos;
  motor->speed += (speed - motor->speed) * SPEED_FILTER;
}

/**
 * @brief Calculate motor position PID, will set speed PID setpoint!
 * @param  motor            Target
 */
void Motor_Pos_PID_Run(motor_t *motor) {
  double tempSpeed = Pos_PID_Calc(&motor->posPID, motor->pos);
  // Set target speed
  if (tempSpeed > motor->posTargetSpd) {
    tempSpeed = motor->posTargetSpd;
  } else if (tempSpeed < -motor->posTargetSpd) {
    tempSpeed = -motor->posTargetSpd;
  }
  motor->spdPID.setPoint = tempSpeed;
}

/**
 * @brief Calculate motor speed PID, will change motor speed!
 * @param  motor            Target
 */
void Motor_Spd_PID_Run(motor_t *motor) {
  static double pwmDuty = 0;
  pwmDuty = Spd_PID_Calc(&motor->spdPID, motor->speed);
  if (pwmDuty > 0.1)
    // pwmDuty += MOTOR_LAUNCH_PWM_DUTY;
    pwmDuty = fmap(pwmDuty, 0, 100, MOTOR_LAUNCH_PWM_DUTY, 100);
  else if (pwmDuty < -0.1)
    // pwmDuty -= MOTOR_LAUNCH_PWM_DUTY;
    pwmDuty = fmap(pwmDuty, -100, 0, -100, -MOTOR_LAUNCH_PWM_DUTY);
  else
    pwmDuty = 0;
  if (pwmDuty > 100) pwmDuty = 100;
  if (pwmDuty < -100) pwmDuty = -100;
  // Set PWM Output
  if (pwmDuty >= 0) {
    __HAL_TIM_SET_COMPARE(motor->timPWM, motor->forwardChannel, pwmDuty * 10);
    __HAL_TIM_SET_COMPARE(motor->timPWM, motor->reverseChannel, 0);
  } else {
    __HAL_TIM_SET_COMPARE(motor->timPWM, motor->forwardChannel, 0);
    __HAL_TIM_SET_COMPARE(motor->timPWM, motor->reverseChannel, -pwmDuty * 10);
  }
  motor->pwmDuty = pwmDuty;
}
