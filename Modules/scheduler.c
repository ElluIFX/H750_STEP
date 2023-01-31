/**
 * @file scheduler.c
 * @brief
 * 时分调度器，目前基本使用__weak修饰，以便在其他文件中可以重写（不用考虑头文件问题）
 * 后续直接整合，使用static修饰提高重入效率
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-11
 *
 * THINK DIFFERENTLY
 */

#include <key.h>
#include <scheduler.h>
#include <uart_pack.h>
#include <stdio.h>
#include "app.h"
/************************ scheduler tasks ************************/

// task lists
static scheduler_task_t schTaskList[] = {
    {UserCom_Task, 100, 0, 0, 1},
    {Task_Uart_Overtime, 100, 0, 0, 1},
    {key_check_all_loop_1ms, 1000, 0, 0, 1},
#if _ENABLE_SCH_DEBUG
    {Show_Sch_Debug_info, 0.2, 0, 0, 1},
#endif
};

__weak void Task_Uart_Overtime(void) { return; }
// @note !redefined in main.c

/************************ scheduler tasks end ************************/

// variables
#if _ENABLE_SCH_DEBUG
uint32_t _sch_debug_task_consuming[SCH_TASK_COUNT - 1];
__IO uint32_t _sch_debug_task_tick = 0;
#endif  // _ENABLE_SCH_DEBUG

// scheduler task control functions

/**
 * @brief Initialize tasklist
 * @retval None
 **/
void Scheduler_Init(void) {
  for (uint8_t i = 0; i < SCH_TASK_COUNT; i++) {
    schTaskList[i].periodMs = 1000 / schTaskList[i].rateHz;
    if (schTaskList[i].periodMs == 0) {
      schTaskList[i].periodMs = 1;
    }
  }
}

/**
 * @brief scheduler runner, call in main loop
 * @retval None
 **/
void Scheduler_Run(void) {
  for (uint8_t i = 0; i < SCH_TASK_COUNT; i++) {
    uint32_t currentTime = HAL_GetTick();
    if (schTaskList[i].enable &&
        (currentTime - schTaskList[i].lastRunMs >= schTaskList[i].periodMs)) {
      schTaskList[i].lastRunMs = currentTime;
#if _ENABLE_SCH_DEBUG
      _sch_debug_task_tick = HAL_GetTick();
      schTaskList[i].task();
      _sch_debug_task_tick = HAL_GetTick() - _sch_debug_task_tick;
      if (_sch_debug_task_consuming[i] < _sch_debug_task_tick) {
        _sch_debug_task_consuming[i] = _sch_debug_task_tick;
      }
#else
      schTaskList[i].task();
#endif  // _ENABLE_SCH_DEBUG
    }
  }
}

/**
 * @brief Enable a task
 * @param  taskId           Target task id
 */
void Enable_SchTask(uint8_t taskId) {
  if (taskId < SCH_TASK_COUNT) {
    schTaskList[taskId].enable = 1;
  }
}

/**
 * @brief Disable a task
 * @param  taskId            Target task id
 */
void Disable_SchTask(uint8_t taskId) {
  if (taskId < SCH_TASK_COUNT) {
    schTaskList[taskId].enable = 0;
  }
}

/**
 * @brief Set a task's rate
 * @param  taskId           Task ID
 * @param  freq             Freq
 */
void Set_SchTask_Freq(uint8_t taskId, float freq) {
  if (taskId < SCH_TASK_COUNT) {
    schTaskList[taskId].rateHz = freq;
    schTaskList[taskId].periodMs = 1000 / schTaskList[taskId].rateHz;
    if (schTaskList[taskId].periodMs == 0) {
      schTaskList[taskId].periodMs = 1;
    }
  }
}

// debug functions

#if _ENABLE_SCH_DEBUG

void Show_Sch_Debug_info(void) {
  static char str_buf[100];
  for (uint8_t i = 0; i < SCH_TASK_COUNT - 1; i++) {
    if (i == 0) {
      sprintf(str_buf, "SCHEDULER INFO:\r\n");
    }
    sprintf(str_buf, "%sTask %d: %ldms\r\n", str_buf, i,
            _sch_debug_task_consuming[i]);
    _sch_debug_task_consuming[i] = 0;
  }
  LOG_D("%s------ INFO END ------", str_buf);
}

#endif  // _ENABLE_SCH_DEBUG
