/**
 * @file app.h
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2023-01-31
 *
 * THINK DIFFERENTLY
 */

#ifndef __APP_H__
#define __APP_H__

#include "main.h"

#define USER_DATA_EXCHANGE_TIMEOUT_S (0.05f - 0.001f)
#define USER_HEARTBEAT_TIMEOUT_S (1.0f - 0.001f)
#define REALTIME_CONTROL_TIMEOUT_S (1.0f - 0.001f)

#define USER_COM_UART huart3

//事件代码
#define USER_EVENT_KEY_SHORT 0x01
#define USER_EVENT_KEY_LONG 0x02
#define USER_EVENT_KEY_DOUBLE 0x03
//事件操作
#define USER_EVENT_OP_SET 0x01
#define USER_EVENT_OP_CLEAR 0x02

//回传数据结构
typedef struct {
  uint8_t head1;
  uint8_t head2;
  uint8_t length;
  //
  uint8_t cmd;
  //data
  int32_t step1_speed;
  int32_t step1_angle;
  int32_t step1_target_angle;
  uint8_t step1_rotating;
  uint8_t step1_dir;

  int32_t step2_speed;
  int32_t step2_angle;
  int32_t step2_target_angle;
  uint8_t step2_rotating;
  uint8_t step2_dir;

  int32_t step3_speed;
  int32_t step3_angle;
  int32_t step3_target_angle;
  uint8_t step3_rotating;
  uint8_t step3_dir;
  //
  uint8_t check_sum;
} __attribute__((__packed__)) _to_user_st;

typedef union {
  uint8_t byte_data[47];
  _to_user_st st_data;
} _to_user_un;

void UserCom_GetOneByte(uint8_t data);

void UserCom_Task();

void UserCom_SendEvent(uint8_t event, uint8_t op);

#endif  // __APP_H__
