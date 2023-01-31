/**
 * @file app.c
 * @brief 用户代码
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2023-01-31
 *
 * THINK DIFFERENTLY
 */

#include "app.h"

#include "uart_pack.h"
#include "usart.h"

void UserCom_DataAnl(uint8_t* data_buf, uint8_t data_len);
void UserCom_DataExchange(void);
void UserCom_SendData(uint8_t* dataToSend, uint8_t Length);
void UserCom_SendAck(uint8_t ack_data);
void UserCom_CalcAck(uint8_t option, uint8_t* data_p, uint8_t data_len);

static uint8_t user_connected = 0;           // 用户下位机是否连接
static uint16_t user_heartbeat_cnt = 0;      // 用户下位机心跳计数
_to_user_un to_user_data;                    // 回传状态数据
_user_ack_st user_ack;                       // ACK数据

/**
 * @brief 用户协议数据获取,在串口中断中调用,解析完成后调用UserCom_DataAnl
 * @param  data             数据
 */
void UserCom_GetOneByte(uint8_t data) {
  static uint8_t _user_data_temp[128];
  static uint8_t _user_data_cnt = 0;
  static uint8_t _data_len = 0;
  static uint8_t state = 0;
  if (state == 0 && data == 0xAA) {
    state = 1;
    _user_data_temp[0] = data;
  } else if (state == 1 && data == 0x22) {
    state = 2;
    _user_data_temp[1] = data;
  } else if (state == 2)  // 功能字
  {
    state = 3;
    _user_data_temp[2] = data;
  } else if (state == 3)  // 长度
  {
    state = 4;
    _user_data_temp[3] = data;
    _data_len = data;  // 数据长度
    _user_data_cnt = 0;
    // if (_data_len == 1) state = 5;
  } else if (state == 4 && _data_len > 0) {
    _data_len--;
    _user_data_temp[4 + _user_data_cnt++] = data;  // 数据
    if (_data_len == 0) state = 5;
  } else if (state == 5) {
    state = 0;
    _user_data_temp[4 + _user_data_cnt] = data;  // check sum
    _user_data_temp[5 + _user_data_cnt] = 0;
    UserCom_DataAnl(_user_data_temp, 4 + _user_data_cnt);
  } else
    state = 0;
}

/**
 * @brief 用户命令解析执行,数据接收完成后自动调用
 * @param  data_buf         数据缓存
 * @param  data_len         数据长度
 */
void UserCom_DataAnl(uint8_t* data_buf, uint8_t data_len) {
  static uint8_t option;
  static uint8_t suboption;
  static uint8_t recv_check;
  static uint8_t calc_check;
  static uint8_t len;
  static uint8_t* p_data;
  static int16_t* p_int16_t;
  static int32_t* p_int32_t;
  static uint32_t* p_uint32_t;
  static uint8_t uint8_t_temp;
  static uint32_t uint32_t_temp;

  p_data = (uint8_t*)(data_buf + 4);
  option = data_buf[2];
  len = data_buf[3];
  recv_check = data_buf[data_len];
  calc_check = 0;
  for (uint8_t i = 0; i < len + 4; i++) {
    calc_check += data_buf[i];
  }
  if (calc_check != recv_check) {
    LOG_E("usercom checksum error");
    return;
  }
  switch (option) {
    case 0x00:  // 心跳包
      if (p_data[0] == 0x01) {
        if (!user_connected) {
          user_connected = 1;
          LOG_I("user connected");
        }
        user_heartbeat_cnt = 0;
        break;
      }
    case 0x01:  // 本地处理
      suboption = p_data[0];
      switch (suboption) {
        case 0x01:                  // WS2812控制
          if (p_data[4] == 0x11) {  // 帧结尾，确保接收完整
            uint32_t_temp = 0xff000000;
            uint8_t_temp = p_data[1];  // R
            uint32_t_temp |= uint8_t_temp << 16;
            uint8_t_temp = p_data[2];  // G
            uint32_t_temp |= uint8_t_temp << 8;
            uint8_t_temp = p_data[3];  // B
            uint32_t_temp |= uint8_t_temp;
            LOG_D("WS2812 color:#%08x", uint32_t_temp);
          }
          break;
        case 0x02:
          break;
        case 0x03:
          break;
        case 0x04:
          break;
        case 0x05:
          break;
        case 0x06:
          break; 
      }
      break;
    case 0x02:
      break;
    default:
      break;
  }
}

void UserCom_CalcAck(uint8_t option, uint8_t* data_p, uint8_t data_len) {
  user_ack.ack_data = option;
  for (uint8_t i = 0; i < data_len; i++) {
    user_ack.ack_data += data_p[i];
  }
  user_ack.WTS = 1;
}

/**
 * @brief 用户通讯持续性任务，在调度器中调用
 * @param  dT_s
 */
void UserCom_Task() {
  const float dT_s = 0.01f;
  static uint16_t data_exchange_cnt = 0;
  if (user_connected) {
    // 心跳超时检查
    user_heartbeat_cnt++;
    if (user_heartbeat_cnt * dT_s >= USER_HEARTBEAT_TIMEOUT_S) {
      user_connected = 0;
      LOG_W("user disconnected");
    }

    // ACK发送检查
    if (user_ack.WTS == 1) {
      user_ack.WTS = 0;
      UserCom_SendAck(user_ack.ack_data);
      user_ack.ack_data = 0;
    }

    // 数据交换
    data_exchange_cnt++;
    if (data_exchange_cnt * dT_s >= USER_DATA_EXCHANGE_TIMEOUT_S) {
      data_exchange_cnt = 0;
      UserCom_DataExchange();
    }
  }
}

/**
 * @brief 交换实时数据
 */
void UserCom_DataExchange(void) {
  static uint8_t user_data_size = sizeof(to_user_data.byte_data);

  // 初始化数据
  to_user_data.st_data.head1 = 0xAA;
  to_user_data.st_data.head2 = 0x55;
  to_user_data.st_data.length = user_data_size - 4;
  to_user_data.st_data.cmd = 0x01;

  // 数据赋值
  to_user_data.st_data.test = 233;

  // 校验和
  to_user_data.st_data.check_sum = 0;
  for (uint8_t i = 0; i < user_data_size - 1; i++) {
    to_user_data.st_data.check_sum += to_user_data.byte_data[i];
  }

  UserCom_SendData(to_user_data.byte_data, user_data_size);
}

static uint8_t data_to_send[12];

void UserCom_SendAck(uint8_t ack_data) {
  data_to_send[0] = 0xAA;      // head1
  data_to_send[1] = 0x55;      // head2
  data_to_send[2] = 0x02;      // length
  data_to_send[3] = 0x02;      // cmd
  data_to_send[4] = ack_data;  // data
  data_to_send[5] = 0;         // check_sum
  for (uint8_t i = 0; i < 5; i++) {
    data_to_send[5] += data_to_send[i];
  }
  UserCom_SendData(data_to_send, 6);
}

/**
 * @brief 发送事件
 * @param  event            事件代码
 * @param  op               操作代码
 */
void UserCom_SendEvent(uint8_t event, uint8_t op) {
  data_to_send[0] = 0xAA;   // head1
  data_to_send[1] = 0x55;   // head2
  data_to_send[2] = 0x03;   // length
  data_to_send[3] = 0x03;   // cmd
  data_to_send[4] = event;  // event code
  data_to_send[5] = op;     // op code
  data_to_send[6] = 0;      // check_sum
  for (uint8_t i = 0; i < 6; i++) {
    data_to_send[6] += data_to_send[i];
  }
  UserCom_SendData(data_to_send, 7);
}

/**
 * @brief 用户通讯数据发送
 */
void UserCom_SendData(uint8_t* dataToSend, uint8_t Length) {
  HAL_UART_Transmit_IT(&USER_COM_UART, dataToSend, Length);
}
