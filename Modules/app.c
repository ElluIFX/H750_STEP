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

#include "queue.h"
#include "step.h"
#include "uart_pack.h"
#include "usart.h"
extern step_ctrl_t step_1;
extern step_ctrl_t step_2;
extern step_ctrl_t step_3;

void UserCom_DataAnl(uint8_t* data_buf, uint8_t data_len);
void UserCom_DataExchange(void);
void UserCom_SendData(uint8_t* dataToSend, uint8_t Length);
void UserCom_CheckAck();
void UserCom_SendAck(uint8_t option, uint8_t* data_p, uint8_t data_len);

static uint8_t user_connected = 0;       // 用户下位机是否连接
static uint16_t user_heartbeat_cnt = 0;  // 用户下位机心跳计数
_to_user_un to_user_data;                // 回传状态数据
static uint8_t user_ack_buf[32];         // ACK数据
static queue_t user_ack_queue;           // ACK队列
static uint16_t user_ack_cnt = 0;        // ACK计数
uint8_t user_data_temp[128];             // 数据接受缓存

/**
 * @brief 用户协议数据获取,在串口中断中调用,解析完成后调用UserCom_DataAnl
 * @param  data             数据
 */
void UserCom_GetOneByte(uint8_t data) {
  static uint8_t _user_data_cnt = 0;
  static uint8_t _data_len = 0;
  static uint8_t state = 0;
  if (state == 0 && data == 0xAA) {
    state = 1;
    user_data_temp[0] = data;
  } else if (state == 1 && data == 0x22) {
    state = 2;
    user_data_temp[1] = data;
  } else if (state == 2)  // 功能字
  {
    state = 3;
    user_data_temp[2] = data;
  } else if (state == 3)  // 长度
  {
    state = 4;
    user_data_temp[3] = data;
    _data_len = data;  // 数据长度
    _user_data_cnt = 0;
    // if (_data_len == 1) state = 5;
  } else if (state == 4 && _data_len > 0) {
    _data_len--;
    user_data_temp[4 + _user_data_cnt++] = data;  // 数据
    if (_data_len == 0) state = 5;
  } else if (state == 5) {
    state = 0;
    user_data_temp[4 + _user_data_cnt] = data;  // check sum
    user_data_temp[5 + _user_data_cnt] = 0;
    UserCom_DataAnl(user_data_temp, 4 + _user_data_cnt);
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
  __IO static int32_t int32_t_temp;
  __IO static double double_temp;
  p_data = (uint8_t*)(data_buf + 4);
  option = data_buf[2];
  len = data_buf[3];
  recv_check = data_buf[data_len];
  calc_check = 0;
  for (uint8_t i = 0; i < len + 4; i++) {
    calc_check += data_buf[i];
  }
  if (calc_check != recv_check) {
    LOG_E("[COM] checksum ERR");
    return;
  }
  RGB(0xff, 0xff, 0x02);
  switch (option) {
    case 0x00:  // 心跳包
      if (p_data[0] == 0x01) {
        if (!user_connected) {
          user_connected = 1;
          RGB(0xff, 1, 0xff);
          LOG_I("[COM] connected");
        }
        user_heartbeat_cnt = 0;
        break;
      }
    case 0x01:  // 步进电机速度设置
      uint8_t_temp = p_data[0];
      int32_t_temp = *((int32_t*)(p_data + 1));
      double_temp = (double)(int32_t_temp) / 100.0;
      LOG_D("[COM] set step speed: 0x%02x, %f", uint8_t_temp, double_temp);
      if (uint8_t_temp & 0x01) Step_Set_Speed(&step_1, double_temp);
      if (uint8_t_temp & 0x02) Step_Set_Speed(&step_2, double_temp);
      if (uint8_t_temp & 0x04) Step_Set_Speed(&step_3, double_temp);
      UserCom_SendAck(option, p_data, 5);
      break;
    case 0x02:  // 步进电机角度设置
      uint8_t_temp = p_data[0];
      int32_t_temp = *((int32_t*)(p_data + 1));
      double_temp = (double)(int32_t_temp) / 1000.0;
      LOG_D("[COM] set step angle: 0x%02x, %f", uint8_t_temp, double_temp);
      if (uint8_t_temp & 0x01) Step_Set_Angle(&step_1, double_temp);
      if (uint8_t_temp & 0x02) Step_Set_Angle(&step_2, double_temp);
      if (uint8_t_temp & 0x04) Step_Set_Angle(&step_3, double_temp);
      UserCom_SendAck(option, p_data, 5);
      break;
    case 0x03:  // 步进电机相对旋转
      uint8_t_temp = p_data[0];
      int32_t_temp = *((int32_t*)(p_data + 1));
      double_temp = (double)(int32_t_temp) / 1000.0;
      LOG_D("[COM] rotate: 0x%02x, %f", uint8_t_temp, double_temp);
      if (uint8_t_temp & 0x01) Step_Rotate(&step_1, double_temp);
      if (uint8_t_temp & 0x02) Step_Rotate(&step_2, double_temp);
      if (uint8_t_temp & 0x04) Step_Rotate(&step_3, double_temp);
      UserCom_SendAck(option, p_data, 5);
      break;
    case 0x04:  // 步进电机绝对旋转
      uint8_t_temp = p_data[0];
      int32_t_temp = *((int32_t*)(p_data + 1));
      double_temp = (double)(int32_t_temp) / 1000.0;
      LOG_D("[COM] rotate abs 0x%02x, %f", uint8_t_temp, double_temp);
      if (uint8_t_temp & 0x01) Step_Rotate_Abs(&step_1, double_temp);
      if (uint8_t_temp & 0x02) Step_Rotate_Abs(&step_2, double_temp);
      if (uint8_t_temp & 0x04) Step_Rotate_Abs(&step_3, double_temp);
      UserCom_SendAck(option, p_data, 5);
      break;
    case 0x05:  // 步进电机停止
      uint8_t_temp = p_data[0];
      LOG_D("[COM] stop 0x%02x", uint8_t_temp);
      if (uint8_t_temp & 0x01) Step_Stop(&step_1);
      if (uint8_t_temp & 0x02) Step_Stop(&step_2);
      if (uint8_t_temp & 0x04) Step_Stop(&step_3);
      UserCom_SendAck(option, p_data, 1);
      break;
    default:
      LOG_E("[COM] unknown option: 0x%02x", option);
      break;
  }
}

void UserCom_SendAck(uint8_t option, uint8_t* data_p, uint8_t data_len) {
  static uint8_t ack_data;
  ack_data = option;
  for (uint8_t i = 0; i < data_len; i++) {
    ack_data += data_p[i];
  }
  ENQUEUE(&user_ack_queue, &ack_data, 1, uint8_t);
  user_ack_cnt++;
}

/**
 * @brief 用户通讯持续性任务，在调度器中调用
 * @param  dT_s
 */
void UserCom_Task() {
  const float dT_s = 0.01f;
  static uint16_t data_exchange_cnt = 0;
  static uint8_t ack_queue_inited = 0;

  if (!ack_queue_inited) {
    ack_queue_inited = 1;
    QUEUE_INIT(&user_ack_queue, user_ack_buf, 32);
  }

  if (user_connected) {
    // 心跳超时检查
    user_heartbeat_cnt++;
    if (user_heartbeat_cnt * dT_s >= USER_HEARTBEAT_TIMEOUT_S) {
      user_connected = 0;
      RGB(0xff, 0, 0);
      LOG_W("[COM] disconnected");
    }

    // ACK发送检查
    UserCom_CheckAck();

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
  static uint8_t test_data = 0;
  test_data++;
  static uint8_t user_data_size = sizeof(to_user_data.byte_data);

  // 初始化数据
  to_user_data.st_data.head1 = 0xAA;
  to_user_data.st_data.head2 = 0x55;
  to_user_data.st_data.length = user_data_size - 4;
  to_user_data.st_data.cmd = 0x01;

  // 数据赋值
  to_user_data.st_data.step1_speed = step_1.speed * 100;
  to_user_data.st_data.step1_angle = Step_Get_Angle(&step_1) * 1000;
  to_user_data.st_data.step1_target_angle = step_1.angleTarget * 1000;
  to_user_data.st_data.step1_rotating = step_1.rotating;
  to_user_data.st_data.step1_dir = step_1.dir;

  to_user_data.st_data.step2_speed = step_2.speed * 100;
  to_user_data.st_data.step2_angle = Step_Get_Angle(&step_2) * 1000;
  to_user_data.st_data.step2_target_angle = step_2.angleTarget * 1000;
  to_user_data.st_data.step2_rotating = step_2.rotating;
  to_user_data.st_data.step2_dir = step_2.dir;

  to_user_data.st_data.step3_speed = step_3.speed * 100;
  to_user_data.st_data.step3_angle = Step_Get_Angle(&step_3) * 1000;
  to_user_data.st_data.step3_target_angle = step_3.angleTarget * 1000;
  to_user_data.st_data.step3_rotating = step_3.rotating;
  to_user_data.st_data.step3_dir = step_3.dir;

  // 校验和
  to_user_data.st_data.check_sum = 0;
  for (uint8_t i = 0; i < user_data_size - 1; i++) {
    to_user_data.st_data.check_sum += to_user_data.byte_data[i];
  }

  UserCom_SendData(to_user_data.byte_data, user_data_size);
}

static uint8_t data_to_send[12];

/**
 * @brief 检查ACK队列并发送
 */
void UserCom_CheckAck() {
  while (user_ack_cnt) {
    data_to_send[0] = 0xAA;  // head1
    data_to_send[1] = 0x55;  // head2
    data_to_send[2] = 0x02;  // length
    data_to_send[3] = 0x02;  // cmd
    // data_to_send[4] = ack_data;  // data
    DEQUEUE(&user_ack_queue, (data_to_send + 4), 1);
    data_to_send[5] = 0;  // check_sum
    for (uint8_t i = 0; i < 5; i++) {
      data_to_send[5] += data_to_send[i];
    }
    UserCom_SendData(data_to_send, 6);
    user_ack_cnt--;
  }
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
  // HAL_UART_Transmit_IT(&USER_COM_UART, dataToSend, Length);
  // DMA
  if (USER_COM_UART.gState == HAL_UART_STATE_READY &&
      USER_COM_UART.hdmatx->State == HAL_DMA_STATE_READY)
    HAL_UART_Transmit_DMA(&USER_COM_UART, dataToSend, Length);
}
