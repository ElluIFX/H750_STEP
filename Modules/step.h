/**
 * @file step.h
 * @brief see step.c
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2022-12-12
 *
 * THINK DIFFERENTLY
 */

#ifndef __STEP_H
#define __STEP_H
#include <main.h>
#include <tim.h>

/****************** 常量定义 ******************/
// 电机参数相关
#define STEP_BASE_PULSE 200  // 零细分脉冲数
#define STEP_SUBDIVISION 16  // 细分倍数
#define STEP_PULSE_PER_ROUND (STEP_BASE_PULSE * STEP_SUBDIVISION)  // 一圈脉冲数
#define STEP_DIR_LOGIC_REVERSE \
  0  // 默认逻辑下, 顺时针转动时DIR为高电平, 置1可反转

// 功能相关
#define STEP_TIM_BASE_CLK 240000000  // 定时器时钟频率
#define STEP_PWM_MAX_FREQ 20000      // PWM最大频率(防止丢步)

/****************** 数据类型定义 ******************/

typedef struct {                 // 步进电机控制结构体
  double speed;                  // 速度, deg/s
  double angle;                  // 当前角度, deg
  double angleTarget;            // 目标角度, deg
  uint8_t rotating;              // 是否正在转动
  uint8_t dir;                   // 转动方向 (0:逆时针, 1:顺时针)
  TIM_HandleTypeDef *timMaster;  // 主定时器句柄(用于PWM输出)
  TIM_HandleTypeDef *timSlave;   // 从定时器句柄(用于脉冲计数)
  uint32_t timMasterCh;          // 主定时器通道
  GPIO_TypeDef *dirPort;         // 方向控制端口
  uint16_t dirPin;               // 方向控制引脚
} step_ctrl_t;

/****************** 函数声明 ******************/

void Step_Init(step_ctrl_t *step, TIM_HandleTypeDef *timMaster,
               TIM_HandleTypeDef *timSlave, uint32_t timMasterCh,
               GPIO_TypeDef *dirPort, uint16_t dirPin);
void Step_IT_Handler(step_ctrl_t *step, TIM_HandleTypeDef *htim);
void Step_Set_Speed(step_ctrl_t *step, double speed);
void Step_Rotate(step_ctrl_t *step, double angle);
void Step_Rotate_Abs(step_ctrl_t *step, double angle);
void Step_Set_Angle(step_ctrl_t *step, double angle);
double Step_Get_Angle(step_ctrl_t *step);
#endif
