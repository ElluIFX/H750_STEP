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

#include <scheduler.h>
#include <stdlib.h>
/************************ scheduler tasks ************************/

// task lists
static scheduler_task_t *schTaskEntry = NULL;
static scheduler_task_t *task_p = NULL;
// @note !redefined in main.c

/************************ scheduler tasks end ************************/

// variables

// scheduler task control functions

/**
 * @brief add a task to scheduler
 * @param  task             task function
 * @param  rateHz           task rate
 * @param  enable           enable or disable at startup
 * @retval uint8_t taskId
 */
uint8_t Add_SchTask(void (*task)(void), float rateHz, uint8_t enable) {
  scheduler_task_t *p = (scheduler_task_t *)malloc(sizeof(scheduler_task_t));
  p->task = task;
  p->rateHz = rateHz;
  p->periodMs = 1000 / p->rateHz;
  if (p->periodMs == 0) p->periodMs = 1;
  p->lastRunMs = 0;
  p->enable = enable;
  p->taskId = 0;
  p->next = NULL;
  if (schTaskEntry == NULL) {
    schTaskEntry = p;
  } else {
    scheduler_task_t *q = schTaskEntry;
    while (q->next != NULL) {
      q = q->next;
    }
    q->next = p;
    p->taskId = q->taskId + 1;
  }
  return p->taskId;
}

#if _ENABLE_SCH_DEBUG
#include <stdio.h>
#include <uart_pack.h>

void Print_Debug_info(void) {
  static char str_buf[100];
  scheduler_task_t *p = schTaskEntry;
  sprintf(str_buf, "SCH INFO ---\r\n");
  while (p != NULL) {
    sprintf(str_buf, "%sTask %d: %dms\r\n", str_buf, p->taskId,
            p->task_consuming);
    p = p->next;
  }
  LOG_D("%s--- INFO END ---", str_buf);
}

#endif  // _ENABLE_SCH_DEBUG

/**
 * @brief scheduler runner, call in main loop
 * @retval None
 **/
void Scheduler_Run(void) {
  static uint32_t currentTime = 0;
#if _ENABLE_SCH_DEBUG
  static uint32_t _sch_debug_task_tick = 0;
  static uint32_t _last_show_debug_info_tick = 0;
#endif  // _ENABLE_SCH_DEBUG

  while (1) {
    if (schTaskEntry == NULL) continue;
    if (task_p == NULL) task_p = schTaskEntry;
    currentTime = HAL_GetTick();
    if (task_p->enable &&
        (currentTime - task_p->lastRunMs >= task_p->periodMs)) {
      task_p->lastRunMs = currentTime;
#if _ENABLE_SCH_DEBUG
      _sch_debug_task_tick = HAL_GetTick();
      task_p->task();
      _sch_debug_task_tick = HAL_GetTick() - _sch_debug_task_tick;
      if (task_p->task_consuming < _sch_debug_task_tick) {
        task_p->task_consuming = _sch_debug_task_tick;
      }
#else
      task_p->task();
#endif  // _ENABLE_SCH_DEBUG
    }
    if (task_p != NULL) task_p = task_p->next;
#if _ENABLE_SCH_DEBUG
    currentTime = HAL_GetTick();
    if (currentTime - _last_show_debug_info_tick >= _SCH_DEBUG_INFO_PERIOD) {
      _last_show_debug_info_tick = currentTime;
      Print_Debug_info();
    }
#endif  // _ENABLE_SCH_DEBUG
  }
}

/**
 * @brief Enable a task by id
 * @param  taskId           Target task id
 */
void _Enable_SchTask_Id(uint8_t taskId) {
  scheduler_task_t *p = schTaskEntry;
  while (p != NULL) {
    if (p->taskId == taskId) {
      p->enable = 1;
      break;
    }
    p = p->next;
  }
}

/**
 * @brief Disable a task by id
 * @param  taskId            Target task id
 */
void _Disable_SchTask_Id(uint8_t taskId) {
  scheduler_task_t *p = schTaskEntry;
  while (p != NULL) {
    if (p->taskId == taskId) {
      p->enable = 0;
      break;
    }
    p = p->next;
  }
}

/**
 * @brief Enable a task by function
 * @param  task             Target task function
 * @note if multiple tasks have the same function, all of them will be enabled
 */
void _Enable_SchTask_Func(void (*task)(void)) {
  scheduler_task_t *p = schTaskEntry;
  while (p != NULL) {
    if (p->task == task) {
      p->enable = 1;
    }
    p = p->next;
  }
}

/**
 * @brief Disable a task by function
 * @param  task             Target task function
 * @note if multiple tasks have the same function, all of them will be
 * disabled
 */
void _Disable_SchTask_Func(void (*task)(void)) {
  scheduler_task_t *p = schTaskEntry;
  while (p != NULL) {
    if (p->task == task) {
      p->enable = 0;
    }
    p = p->next;
  }
}

/**
 * @brief delete a task from scheduler by task id
 * @param  taskId           task id
 * @retval None
 */
void _Del_SchTask_Id(uint8_t taskId) {
  scheduler_task_t *p = schTaskEntry;
  scheduler_task_t *q = NULL;
  while (p != NULL) {
    if (p->taskId == taskId) {
      if (q == NULL) {
        schTaskEntry = p->next;
      } else {
        q->next = p->next;
      }
      if (task_p == p) task_p = p->next;
      free(p);
      break;
    }
    q = p;
    p = p->next;
  }
}

/**
 * @brief delete a task from scheduler by task function
 * @param  task             task function
 * @retval None
 */
void _Del_SchTask_Func(void (*task)(void)) {
  scheduler_task_t *p = schTaskEntry;
  scheduler_task_t *q = NULL;
  scheduler_task_t *k = NULL;
  while (p != NULL) {
    if (p->task == task) {
      if (q == NULL) {
        schTaskEntry = p->next;
      } else {
        q->next = p->next;
      }
      if (task_p == p) task_p = p->next;
      k = p->next;
      free(p);
      p = k;
      continue;
    }
    q = p;
    p = p->next;
  }
}

/**
 * @brief Set a task's rate
 * @param  taskId           Task ID
 * @param  freq             Freq
 */
void _Set_SchTask_Freq_Id(uint8_t taskId, float freq) {
  scheduler_task_t *p = schTaskEntry;
  while (p != NULL) {
    if (p->taskId == taskId) {
      p->rateHz = freq;
      p->periodMs = 1000 / p->rateHz;
      if (p->periodMs == 0) {
        p->periodMs = 1;
      }
      break;
    }
    p = p->next;
  }
}

/**
 * @brief Set a task's rate
 * @param  task             Task function
 * @param  freq             Freq
 */
void _Set_SchTask_Freq_Func(void (*task)(void), float freq) {
  scheduler_task_t *p = schTaskEntry;
  while (p != NULL) {
    if (p->task == task) {
      p->rateHz = freq;
      p->periodMs = 1000 / p->rateHz;
      if (p->periodMs == 0) {
        p->periodMs = 1;
      }
    }
    p = p->next;
  }
}
// debug functions

#if _ENABLE_SCH_DEBUG
#warning You Enabled a Scheduler Debugging Feature, which will cause lower performance.
#endif  // _ENABLE_SCH_DEBUG
