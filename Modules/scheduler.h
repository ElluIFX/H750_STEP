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

#include "candy.h"
#include "main.h"
//  defines
#define _ENABLE_SCH_DEBUG 0
#define _SCH_DEBUG_INFO_PERIOD 5000  // ms

// typedef
typedef struct {       // 用户任务结构
  void (*task)(void);  // task function
  float rateHz;        // task rate
  uint16_t periodMs;   // task period
  uint32_t lastRunMs;  // last run time
  uint8_t enable;      // enable or disable
  uint8_t taskId;      // task id
#if _ENABLE_SCH_DEBUG
  uint32_t task_consuming;  // task consuming time
#endif
  void *next;  // next task
} scheduler_task_t;
// private variables

// private functions

uint8_t Add_SchTask(void (*task)(void), float rateHz, uint8_t enable);
void Scheduler_Run(void);
void Enable_SchTask_Id(uint8_t taskId);
void Disable_SchTask_Id(uint8_t taskId);
void Enable_SchTask_Func(void (*task)(void));
void Disable_SchTask_Func(void (*task)(void));
void Del_SchTask_Id(uint8_t taskId);
void Del_SchTask_Func(void (*task)(void));
void Set_SchTask_Freq(uint8_t taskId, float freq);
#endif  // _SCHEDULER_H_
