/**
 * @file programCtrl.h
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-13
 *
 * THINK DIFFERENTLY
 */

#ifndef _PROGRAMCTRL_H_
#define _PROGRAMCTRL_H_
#include "main.h"

// typedef
typedef struct {  //用户任务结构
  void (*task)(void);
  uint32_t runTimeMs;  //任务运行时间
  uint8_t runFlag;     //任务运行标志
  uint8_t endFlag;     //任务结束标志
} user_task_t;

typedef struct {  //用户任务控制字
  //读状态
  uint8_t taskId;   //当前任务ID
  uint8_t taskRunning;  //任务运行状态
  uint8_t taskListDone;  //任务列表结束
  //写任务控制
  uint8_t runFlag;     //运行任务列表
  uint8_t continueFlag;  //结束并跳过当前任务
  uint8_t breakFlag;     //终止任务列表
  //写用户控制（任务参数）
  //留着战未来
} user_task_ctrl_t;

// private variables
extern user_task_ctrl_t user_task_ctrl_word;

// private functions
void User_Task_1(void);
void User_Task_2(void);
void User_Task_3(void);
void User_Task_Ctrl(void);
void Reset_User_Task(void);
void Task_Breakout_Handler(void);
void Task_Done_Handler(void);

#endif  // _PROGRAMCTRL_H_