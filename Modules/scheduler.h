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

void _Enable_SchTask_Id(uint8_t taskId);
void _Disable_SchTask_Id(uint8_t taskId);
void _Enable_SchTask_Func(void (*task)(void));
void _Disable_SchTask_Func(void (*task)(void));
void _Del_SchTask_Id(uint8_t taskId);
void _Del_SchTask_Func(void (*task)(void));
void _Set_SchTask_Freq_Id(uint8_t taskId, float freq);
void _Set_SchTask_Freq_Func(void (*task)(void), float freq);

// Enable a task by task id or task function
#define Enable_SchTask(_OP)                \
  _Generic((_OP), void (*)(void)           \
           : _Enable_SchTask_Func, default \
           : _Enable_SchTask_Id)(_OP)

// Disable a task by task id or task function
#define Disable_SchTask(_OP)                \
  _Generic((_OP), void (*)(void)            \
           : _Disable_SchTask_Func, default \
           : _Disable_SchTask_Id)(_OP)

// Delete a task by task id or task function
#define Del_SchTask(_OP)                \
  _Generic((_OP), void (*)(void)        \
           : _Del_SchTask_Func, default \
           : _Del_SchTask_Id)(_OP)

// Set task frequency by task id or task function
#define Set_SchTask_Freq(_OP, _FREQ)       \
  _Generic((_OP), , void (*)(void)         \
           : _Set_SchTask_Freq_Id, default \
           : _Set_SchTask_Freq_Id)(_OP, _FREQ)

#endif  // _SCHEDULER_H_
