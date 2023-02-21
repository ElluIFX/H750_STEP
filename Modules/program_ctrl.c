/**
 * @file program_ctrl.c
 * @brief 顺序执行程序控制器，控制程序的执行顺序，以及程序的暂停和恢复
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-13
 *
 * THINK DIFFERENTLY
 */

#include "program_ctrl.h"
#include "stdio.h"
// variables
user_task_ctrl_t user_task_ctrl_word;
static uint32_t taskRunTime = 0;
static uint8_t taskRunIndex = 0;

/******************** user tasks ************************/
// task list
static user_task_t userTaskList[] = {
    {User_Task_1, 1000, 0, 0},
    {User_Task_2, 1000, 0, 0},
    {User_Task_3, 1000, 0, 0},
};
const uint8_t USER_TASK_COUNT = sizeof(userTaskList) / sizeof(user_task_t);

void User_Task_1(void) {
  // do something
}

void User_Task_2(void) {
  // do something
}

void User_Task_3(void) {
  // do something
}

/******************** user tasks end ************************/

/******************** safety tasks ************************/

/**
 * @brief Run when user task breakout
 */
void Task_Breakout_Handler(void) {
  // do something
}

/**
 * @brief Run after user task done
 */
void Task_Done_Handler(void) {
  // do somethin
}

/******************** safety tasks end ************************/
// user task control functions
/**
 * @brief handle user task control
 */
void User_Task_Ctrl(void) {
  if (user_task_ctrl_word.runFlag == 1) {
    if (taskRunIndex == 0 && userTaskList[0].runFlag == 0) {  // init
      userTaskList[0].runFlag = 1;
    }
    if (taskRunIndex < USER_TASK_COUNT) {
      if (user_task_ctrl_word.breakFlag == 1) {  // break
        user_task_ctrl_word.breakFlag = 0;
        user_task_ctrl_word.runFlag = 0;
        user_task_ctrl_word.continueFlag = 0;
        user_task_ctrl_word.taskListDone = 1;
        userTaskList[taskRunIndex].runFlag = 0;
        userTaskList[taskRunIndex].endFlag = 0;
        taskRunTime = 0;
        taskRunIndex = 0;
        Task_Breakout_Handler();
      }
      if (user_task_ctrl_word.continueFlag == 1) {  // continue
        user_task_ctrl_word.continueFlag = 0;
        userTaskList[taskRunIndex].endFlag = 1;
        userTaskList[taskRunIndex].runFlag = 0;
      }

      if (userTaskList[taskRunIndex].runFlag == 1) {  // run task
        static uint32_t dT;
        user_task_ctrl_word.taskId = taskRunIndex;
        user_task_ctrl_word.taskRunning = 1;
        dT = HAL_GetTick();
        userTaskList[taskRunIndex].task();
        taskRunTime += HAL_GetTick() - dT;
      }
      if (taskRunTime >= userTaskList[taskRunIndex].runTimeMs) {  // end
        userTaskList[taskRunIndex].endFlag = 1;
        userTaskList[taskRunIndex].runFlag = 0;
        user_task_ctrl_word.taskRunning = 0;
      }
      if (userTaskList[taskRunIndex].endFlag == 1) {  // next
        userTaskList[taskRunIndex].endFlag = 0;
        if (++taskRunIndex < USER_TASK_COUNT) {
          userTaskList[taskRunIndex].runFlag = 1;
        }
        taskRunTime = 0;
      }
    } else {  // all done
      user_task_ctrl_word.taskListDone = 1;
      user_task_ctrl_word.continueFlag = 0;
      user_task_ctrl_word.breakFlag = 0;
      user_task_ctrl_word.runFlag = 0;
      taskRunTime = 0;
      taskRunIndex = 0;
      Task_Done_Handler();
    }
  } else {
    user_task_ctrl_word.taskRunning = 0;
  }
}

/**
 * @brief reset user task related variables
 */
void Reset_User_Task(void) {
  user_task_ctrl_word.runFlag = 0;
  user_task_ctrl_word.breakFlag = 0;
  user_task_ctrl_word.continueFlag = 0;
  user_task_ctrl_word.taskListDone = 0;
  user_task_ctrl_word.taskId = 0;
  user_task_ctrl_word.taskRunning = 0;
  taskRunTime = 0;
  taskRunIndex = 0;
}
