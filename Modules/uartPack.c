/**
 * @file uartPack.c
 * @brief
 * 封装了一些串口函数，包括串口发送和接收，串口发送和接收的数据类型都是uint8_t
 * ，接受分为两种，一种是超时判定，一种是结束位判定
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-19
 *
 * THINK DIFFERENTLY
 */

#include "uartPack.h"

#include "stdarg.h"
#include "string.h"

/**
 * @brief Send a format string to target UART port
 * @param  huart            UART handle
 * @param  fmt              format string
 * @retval number of bytes sent, -1 if error
 */
int printft(UART_HandleTypeDef *huart, char *fmt, ...) {
  static char sendBuff[_UART_BUFFER_SIZE];  // 发送缓冲区
  static int sendLen = 0;                   // 发送计数
  // 检查串口是否打开
  if (huart->gState != HAL_UART_STATE_READY) {
    return -1;
  }
  va_list ap;         // typedef char *va_list
  va_start(ap, fmt);  // 找到第一个可变形参的地址赋给ap
  sendLen = vsprintf(sendBuff, fmt, ap);
  va_end(ap);
  if (sendLen > 0) {
    HAL_UART_Transmit_IT(huart, (uint8_t *)sendBuff, sendLen);
    while (huart->gState != HAL_UART_STATE_READY) {
    }
  }
  return sendLen;
}

/**
 * @brief Enable a Overtime UART controller, call once before using UART
 * @param  huart            target UART handle
 * @param  ctrl             target UART controller
 */
void Enable_Uart_O_Control(UART_HandleTypeDef *huart, uart_o_ctrl_t *ctrl) {
  // 设置串口接收中断
  HAL_UART_Receive_IT(huart, ctrl->rxBuf, 1);
  if (ctrl->rxTimeout == 0) {
    ctrl->rxTimeout = _RX_DEFAILT_TIMEOUT;
  }
  ctrl->rxSaveFlag = 0;
  ctrl->rxBufIndex = 0;
  ctrl->rxSaveCounter = 0;
  ctrl->huart = huart;
}

/**
 * @brief Enable a Single Ending Bit UART controller, call once before using
 * UART
 * @param  huart            target UART handle
 * @param  ctrl             target UART controller
 */
void Enable_Uart_E_Control(UART_HandleTypeDef *huart, uart_e_ctrl_t *ctrl) {
  // 设置串口接收中断
  HAL_UART_Receive_IT(huart, ctrl->rxBuf, 1);
  if (ctrl->rxEndBit == 0) {
    ctrl->rxEndBit = _RX_DEFAILT_ENDBIT;
  }
  ctrl->rxSaveFlag = 0;
  ctrl->rxBufIndex = 0;
  ctrl->rxSaveCounter = 0;
  ctrl->huart = huart;
}

/**
 * @brief Process Overtime UART data, call in HAL_UART_RxCpltCallback
 * @param  huart            target UART handle
 * @param  ctrl             target UART controller
 * @retval 1: data overflow, 0: no data overflow
 */
uint8_t Uart_O_Data_Process(uart_o_ctrl_t *ctrl) {
  ctrl->rxTick = HAL_GetTick();
  if (++ctrl->rxBufIndex >= _UART_BUFFER_SIZE - 1) {
    memcpy(ctrl->rxSaveBuf, ctrl->rxBuf, ctrl->rxBufIndex);
    ctrl->rxSaveCounter = ctrl->rxBufIndex;
    ctrl->rxSaveBuf[ctrl->rxBufIndex] = 0;
    ctrl->rxSaveFlag = 1;
    ctrl->rxBufIndex = 0;
    return 1;
  }
  HAL_UART_Receive_IT(ctrl->huart, ctrl->rxBuf + ctrl->rxBufIndex, 1);
  return 0;
}

/**
 * @brief Overtime UART timeout check, call in scheduler
 * @param  huart            target UART handle
 * @param  ctrl             target UART controller
 * @retval 1: timeout, 0: not timeout
 */
uint8_t Uart_O_Timeout_Check(uart_o_ctrl_t *ctrl) {
  if (ctrl->rxBufIndex && HAL_GetTick() - ctrl->rxTick > 10) {
    HAL_UART_AbortReceive_IT(ctrl->huart);
    memcpy(ctrl->rxSaveBuf, ctrl->rxBuf, ctrl->rxBufIndex);
    ctrl->rxSaveCounter = ctrl->rxBufIndex;
    ctrl->rxSaveBuf[ctrl->rxBufIndex] = 0;
    ctrl->rxSaveFlag = 1;
    ctrl->rxBufIndex = 0;
    HAL_UART_Receive_IT(ctrl->huart, ctrl->rxBuf, 1);
    return 1;
  }
  return 0;
}

/**
 * @brief Process Single Ending Bit UART data, call in HAL_UART_RxCpltCallback
 * @param  huart            target UART handle
 * @param  ctrl             target UART controller
 * @retval 1: end bit, 0: not end bit
 */
uint8_t Uart_E_Data_Process(uart_e_ctrl_t *ctrl) {
  ctrl->rxBufIndex++;
  if (ctrl->rxBufIndex >= _UART_BUFFER_SIZE - 1 ||
      ctrl->rxBuf[ctrl->rxBufIndex - 1] == ctrl->rxEndBit) {
    ctrl->rxBufIndex--;
    memcpy(ctrl->rxSaveBuf, ctrl->rxBuf, ctrl->rxBufIndex);
    ctrl->rxSaveCounter = ctrl->rxBufIndex;
    ctrl->rxSaveBuf[ctrl->rxBufIndex] = 0;
    ctrl->rxSaveFlag = 1;
    ctrl->rxBufIndex = 0;
    HAL_UART_Receive_IT(ctrl->huart, ctrl->rxBuf, 1);
    return 1;
  }
  HAL_UART_Receive_IT(ctrl->huart, ctrl->rxBuf + ctrl->rxBufIndex, 1);
  return 0;
}
