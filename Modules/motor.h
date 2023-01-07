/**
 * @file motor.h
 * @brief See motor.c for details.
 * @author Ellu (lutaoyu@163.com)
 * @version 2.0
 * @date 2021-12-18
 *
 * THINK DIFFERENTLY
 */

#ifndef MOTOR_H
#define MOTOR_H
#include <main.h>
#include <tim.h>

/****************** 常量定义 ******************/
// 电机参数相关
#define SPEED_RATIO 1.0          // 齿轮组减速比
#define ENCODER_RESOLUTION 16.0  // 编码器线数
#define ENCODER_MULTIPLE 2.0  // 编码器倍频（个数*计数方式）（UP）
#define PULSE_PER_ROTATION \
  (ENCODER_RESOLUTION * SPEED_RATIO * ENCODER_MULTIPLE)  // 每圈编码器脉冲数
#define WHEEL_PERIMETER 0.065                            // 轮子周长（m）
#define PULSE_PER_METER (PULSE_PER_ROTATION / WHEEL_PERIMETER)  // 每米脉冲数
#define MOTOR_LAUNCH_PWM_DUTY 50.0  // 电机启动PWM占空比

// 功能相关
#define MOTOR_PWM_FREQ 20000  // 电机PWM频率
#define SYSTEM_CLOCK_FREQ_HZ 72000000
#define ENCODER_TIM_PERIOD \
  65535  // 编码器TIM周期, 用于计算溢出 16bit:65535 32bit:4294967295
#define ENCODER_MID_VALUE ENCODER_TIM_PERIOD / 2  // 编码器中值
#define SPEED_FILTER 0.8                          // 速度滤波系数

// 增量式PID
#define INC_KP 0.0       // 比例项系数
#define INC_KI 0.0       // 积分项系数
#define INC_KD 0.0       // 微分项系数
#define INC_MAX_INC 0.0  // 单周期最大增量

// 速度环PID
#define SPD_KP 0.24           // 比例项系数
#define SPD_KI 0.004          // 积分项系数
#define SPD_KD 0.002          // 微分项系数
#define SPD_DEAD_BAND 8.0     // 速度死区
#define SPD_MAX_I_MULTI 20.0  // 积分上限乘子
#define SPD_INIT_TARGET 0.0   // 初始目标速度

// 位置环PID
#define POS_KP 0.126   // 比例项系数
#define POS_KI 0.0001  // 积分项系数
#define POS_KD 0.412   // 微分项系数
#define POS_DEAD_BAND (int32_t)(1.0 * PULSE_PER_ROTATION / 360.0)  // 位置死区
#define POS_MAX_I 20000l             // 积分上限
#define POS_INIT_TARGET 0            // 初始目标位置
#define POS_INIT_TARGET_SPEED 110.0  // 初始目标速度

/****************** 数据类型定义 ******************/

typedef struct {           // 增量式PID结构体
  __IO double setPoint;    // 设定目标
  __IO double sumError;    // 误差累计
  __IO double proportion;  // 比例常数
  __IO double integral;    // 积分常数
  __IO double derivative;  // 微分常数
  __IO double error_1;     // e[-1]
  __IO double error_2;     // e[-2]
  __IO double max_inc;     // 单周期最大增量
} inc_pid_t;

typedef struct {           // 速度环PID结构体
  __IO double setPoint;    // 设定目标
  __IO double sumError;    // 误差累计
  __IO double proportion;  // 比例常数
  __IO double integral;    // 积分常数
  __IO double derivative;  // 微分常数
  __IO double error_1;     // e[-1]
  __IO double error_2;     // e[-2]
  __IO double deadBand;    // 速度死区
  __IO double maxIMult;    // 积分上限乘数（与速度）
} spd_pid_t;

typedef struct {           // 位置环PID结构体
  __IO int32_t setPoint;   // 设定目标
  __IO int32_t sumError;   // 误差累计
  __IO double proportion;  // 比例常数
  __IO double integral;    // 积分常数
  __IO double derivative;  // 微分常数
  __IO int32_t error_1;    // e[-1]
  __IO int32_t error_2;    // e[-2]
  __IO int32_t deadBand;   // 位置死区
  __IO int32_t maxI;       // 积分上限
} pos_pid_t;

typedef struct {                  // 电机闭环控制结构体
  double speed;                   // 速度
  int32_t pos;                    // 位置
  int32_t lastPos;                // 上一次位置
  spd_pid_t spdPID;               // 速度环PID
  pos_pid_t posPID;               // 位置环PID
  double posTargetSpd;            // 位置环目标速度
  double pwmDuty;                 // PWM占空比
  TIM_HandleTypeDef *timEncoder;  // 编码器定时器
  TIM_HandleTypeDef *timPWM;      // PWM定时器
  uint32_t forwardChannel;        // 正向通道
  uint32_t reverseChannel;        // 反向通道
} motor_t;

/****************** 带参宏定义 ******************/

// 设置速度环速度（前提是没使能位置环）
#define __MOTOR_SET_SPEED(motor, speed) motor.spdPID.setPoint = speed

// 设置位置环位置（以中立位为基准）
#define __MOTOR_SET_POS(motor, pos) motor.posPID.setPoint = pos

// 获取当前位置（以中立位为基准）
#define __MOTOR_GET_POS(motor) (motor.pos)

// 前进一定脉冲数（使能位置环）
#define __MOTOR_GO_POS(motor, pos) motor.posPID.setPoint += pos

// 设置位置环角度（以中立位为基准）
#define __MOTOR_SET_DEGREE(motor, degree) \
  motor.posPID.setPoint = (int32_t)(degree * PULSE_PER_ROTATION / 360.0)

// 获取当前角度（以中立位为基准）
#define __MOTOR_GET_DEGREE(motor) \
  ((double)motor.pos * 360.0 / PULSE_PER_ROTATION)

// 按角度前进(使能位置环)
#define __MOTOR_GO_DEGREE(motor, degree) \
  motor.posPID.setPoint += (int32_t)(degree * PULSE_PER_ROTATION / 360.0)

// 按米前进（使能位置环）
#define __MOTOR_GO_METER(motor, meter) \
  motor.posPID.setPoint += meter * PULSE_PER_METER

// 停止（会使能32的PWM刹车功能（大概？））
#define __MOTOR_PWM_SETZERO(motor)                              \
  __HAL_TIM_SET_COMPARE(motor.timPWM, motor.forwardChannel, 0); \
  __HAL_TIM_SET_COMPARE(motor.timPWM, motor.reverseChannel, 0)

// 设置电机正向PWM输出
#define __MOTOR_PWM_SETFWD(motor, pwm)                                 \
  __HAL_TIM_SET_COMPARE(motor.timPWM, motor.forwardChannel, pwm * 10); \
  __HAL_TIM_SET_COMPARE(motor.timPWM, motor.reverseChannel, 0)

// 设置电机反向PWM输出
#define __MOTOR_PWM_SETREV(motor, pwm)                          \
  __HAL_TIM_SET_COMPARE(motor.timPWM, motor.forwardChannel, 0); \
  __HAL_TIM_SET_COMPARE(motor.timPWM, motor.reverseChannel, pwm * 10)

// 输出停止（会越过32的PWM刹车功能）
#define __MOTOR_STOP(motor) \
  HAL_TIM_PWM_Stop(motor.timPWM, motor.forwardChannel || motor.reverseChannel)

// 输出启动
#define __MOTOR_START(motor) \
  HAL_TIM_PWM_Start(motor.timPWM, motor.forwardChannel || motor.reverseChannel)

// 以当前状态重置位置环中立位
#define __MOTOR_RESET_ENCODER(motor)                          \
  __HAL_TIM_SET_COUNTER(motor.timEncoder, ENCODER_MID_VALUE); \
  motor.lastPos = 0;                                          \
  motor.pos = 0

// 清空任意类型PID的误差累计
#define __CLEAR_PID_ERROR(motor, pid) \
  pid.sumError = 0;                   \
  pid.error_1 = 0;                    \
  pid.error_2 = 0

// 清空双环PID的误差累计
#define __CLEAR_ALL_PID_ERROR(motor)      \
  __CLEAR_PID_ERROR(motor, motor.spdPID); \
  __CLEAR_PID_ERROR(motor, motor.posPID);

/****************** 函数声明 ******************/

double Inc_PID_Calc(inc_pid_t *PIDx, double NextPoint);
void Inc_PID_Param_Init(inc_pid_t *);

double Spd_PID_Calc(spd_pid_t *PIDx, double NextPoint);
void Spd_PID_Param_Init(spd_pid_t *);

double Pos_PID_Calc(pos_pid_t *PIDx, int32_t NextPoint);
void Pos_PID_Param_Init(pos_pid_t *);

void Motor_Setup(motor_t *motor, TIM_HandleTypeDef *timEncoder,
                 TIM_HandleTypeDef *timPWM, uint32_t forwardChannel,
                 uint32_t reverseChannel);
void Motor_Update_Speed(motor_t *motor, double runTimeHz);
void Motor_Encoder_Overflow(motor_t *motor);
void Motor_Pos_PID_Run(motor_t *motor);
void Motor_Spd_PID_Run(motor_t *motor);

#endif  // __MOTOR_H
