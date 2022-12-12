/**
 * @file scheduler.h
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-11
 *
 * THINK DIFFERENTLY
 */

#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "main.h"
//  defines
#define _ENABLE_SCH_DEBUG 0

#define UART_CONTROLLER_TASK_ID 0
#define UART_OVERTIME_TASK_ID 1
#define KEY_FUNC_TASK_ID 2
#define KEY_CHECK_ALL_LOOP_1MS_TASK_ID 3

#define SCH_TASK_COUNT sizeof(schTaskList) / sizeof(scheduler_task_t)
// typedef
typedef struct {       //用户任务结构
  void (*task)(void);  // task function
  float rateHz;        // task rate
  uint16_t periodMs;   // task period
  uint32_t lastRunMs;  // last run time
  uint8_t enable;      // enable or disable
} scheduler_task_t;
// private variables
// private functions
__weak void Task_Uart_Controller(void);
__weak void Task_Uart_Overtime(void);
__weak void Task_Key_Func(void);

void Scheduler_Init(void);
void Scheduler_Run(void);
void Enable_SchTask(uint8_t taskId);
void Disable_SchTask(uint8_t taskId);
void Set_SchTask_Freq(uint8_t taskId, float freq);
#if _ENABLE_SCH_DEBUG
void Show_Sch_Debug_info(void);
#warning You Enabled a Schduler Debugging Feature, which will cause lower performance.
#endif  // _ENABLE_SCH_DEBUG
#endif  // _SCHEDULER_H_
