/**
 * @file uartPack.h
 * @brief see uartPack.c for details.
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-19
 *
 * THINK DIFFERENTLY
 */

#ifndef _UART_PACK_H_
#define _UART_PACK_H_
#include "candy.h"
#include "main.h"
#include "stdio.h"
#include "usart.h"
// private define
// 调试信息设置
#define _DEBUG_UART_PORT huart1  // 调试串口
#define _ENABLE_LOG 1            // 是否输出调试信息
#define _ENABLE_LOG_TIMESTAMP 0  // 调试信息是否添加时间戳
#define _ENABLE_LOG_COLOR 1      // 调试信息是否按等级添加颜色
#define _ENABLE_LOG_ASSERT 1     // 是否开启ASSERT
// 调试信息等级
#define _ENABLE_LOG_DEBUG 1  // 是否输出DEBUG信息
#define _ENABLE_LOG_INFO 1   // 是否输出INFO信息
#define _ENABLE_LOG_WARN 1   // 是否输出WARN信息
#define _ENABLE_LOG_ERROR 1  // 是否输出ERROR信息

#define printf(...) printft(&_DEBUG_UART_PORT, __VA_ARGS__)
#define printf_flush() printft_flush(&_DEBUG_UART_PORT)
// constants
#define _UART_RECV_BUFFER_SIZE 256  // 串口接收缓冲区大小
#define _UART_SEND_BUFFER_SIZE 256  // 串口发送缓冲区大小
#define _RX_DEFAILT_TIMEOUT 10
#define _RX_DEFAILT_ENDBIT '\n'
#define _UART_SEND_TIMEOUT 100  // 串口发送超时时间
#define _UART_PRINT_SAFE 1  // 超时后永久关闭Printf避免主程序卡死
#define _UART_PRINT_PINGPONG 1  // 是否使用双缓冲区
#define _UART_PRINT_DMA 1       // 是否使用DMA发送

// typedef
typedef struct {                              // 超时型UART控制结构体
  uint8_t rxBuf[_UART_RECV_BUFFER_SIZE];      // 接收缓冲区
  uint8_t rxSaveBuf[_UART_RECV_BUFFER_SIZE];  // 接收保存缓冲区
  __IO uint8_t rxBufIndex;                    // 接收缓冲区索引
  __IO uint8_t rxSaveFlag;                    // 接收完成标志位
  __IO uint8_t rxSaveCounter;                 // 接收保存区计数器
  __IO uint32_t rxTick;                       // 接收超时计时器
  uint32_t rxTimeout;                         // 接收超时时间
  UART_HandleTypeDef *huart;                  // 串口句柄
} uart_o_ctrl_t;

typedef struct {                          // 单结束位型UART控制结构体
  uint8_t rxBuf[_UART_RECV_BUFFER_SIZE];  // 接收缓冲区
  uint8_t rxSaveBuf[_UART_RECV_BUFFER_SIZE];  // 接收保存缓冲区
  __IO uint8_t rxSaveFlag;                    // 接收完成标志位
  __IO uint8_t rxBufIndex;                    // 接收缓冲区索引
  __IO uint8_t rxSaveCounter;                 // 接收保存区计数器
  uint8_t rxEndBit;                           // 接收结束位
  UART_HandleTypeDef *huart;                  // 串口句柄
} uart_e_ctrl_t;

typedef struct {                              // DMA型UART控制结构体
  uint8_t rxBuf[_UART_RECV_BUFFER_SIZE];      // 接收缓冲区
  uint8_t rxSaveBuf[_UART_RECV_BUFFER_SIZE];  // 接收保存缓冲区
  __IO uint8_t rxSaveFlag;                    // 接收完成标志位
  __IO uint8_t rxSaveCounter;                 // 接收保存区计数器
  UART_HandleTypeDef *huart;                  // 串口句柄
} uart_dma_ctrl_t;

// defines
#define RX_DONE(uart_t) uart_t.rxSaveFlag
#define RX_DATA(uart_t) ((char *)uart_t.rxSaveBuf)
#define RX_COUNT(uart_t) uart_t.rxSaveCounter
#define RX_CLEAR(uart_t) (uart_t.rxSaveFlag = 0)

// debug functions
#define _GET_SYS_TICK HAL_GetTick
#define _LOG_PRINTF printf

#if _ENABLE_LOG
#if _ENABLE_LOG_TIMESTAMP && _ENABLE_LOG_COLOR
#define _DBG_LOG(level, color, fmt, args...)                          \
  _LOG_PRINTF("\033[" #color "m[" level "/%ldms] " fmt "\033[0m\r\n", \
              _GET_SYS_TICK(), ##args)
#elif !_ENABLE_LOG_TIMESTAMP && _ENABLE_LOG_COLOR
#define _DBG_LOG(level, color, fmt, args...) \
  _LOG_PRINTF("\033[" #color "m[" level "] " fmt "\033[0m\r\n", ##args)
#elif _ENABLE_LOG_TIMESTAMP && !_ENABLE_LOG_COLOR
#define _DBG_LOG(level, color, fmt, args...) \
  _LOG_PRINTF("[" level "/%ldms] " fmt "\r\n", _GET_SYS_TICK(), ##args)
#elif !_ENABLE_LOG_TIMESTAMP && !_ENABLE_LOG_COLOR
#define _DBG_LOG(level, color, fmt, args...) \
  _LOG_PRINTF("[" level "] " fmt "\r\n", ##args)
#endif
#if _ENABLE_LOG_DEBUG
#define LOG_D(fmt, args...) _DBG_LOG("D", 36, fmt, ##args)
#endif
#if _ENABLE_LOG_INFO
#define LOG_I(fmt, args...) _DBG_LOG("I", 32, fmt, ##args)
#endif
#if _ENABLE_LOG_WARN
#define LOG_W(fmt, args...) _DBG_LOG("W", 33, fmt, ##args)
#endif
#if _ENABLE_LOG_ERROR
#define LOG_E(fmt, args...) _DBG_LOG("E", 31, fmt, ##args)
#endif
#define LOG_RAW(fmt, args...) _LOG_PRINTF(fmt, ##args)
#endif  // _ENABLE_LOG
#ifndef LOG_D
#define LOG_D(...) ((void)0)
#endif
#ifndef LOG_I
#define LOG_I(...) ((void)0)
#endif
#ifndef LOG_W
#define LOG_W(...) ((void)0)
#endif
#ifndef LOG_E
#define LOG_E(...) ((void)0)
#endif
#ifndef LOG_RAW
#define LOG_RAW(...) ((void)0)
#endif
#if _ENABLE_LOG_ASSERT
#define __ASSERT_0(expr)                                      \
  if (!(expr)) {                                              \
    _DBG_LOG("A", 31, "Failed at %s:%d", __FILE__, __LINE__); \
    Assert_Failed_Handler(__FILE__, __LINE__);                \
  }
#define __ASSERT_1(expr, text)                 \
  if (!(expr)) {                               \
    _DBG_LOG("A", 31, text);                   \
    Assert_Failed_Handler(__FILE__, __LINE__); \
  }
#define __ASSERT_2(expr, text, cmd) \
  if (!(expr)) {                    \
    _DBG_LOG("A", 31, text);        \
    cmd;                            \
  }

// 断言, param: 表达式, 错误信息(可选), 自定义语句(可选)
// 断言失败时默认调用Assert_Failed_Handler
#define ASSERT(expr, ...) EVAL(__ASSERT_, __VA_ARGS__)(expr, ##__VA_ARGS__)
#else
#define ASSERT(expr, ...) ((void)0)
#endif

// public functions

void Assert_Failed_Handler(char *file, uint32_t line);

int printft(UART_HandleTypeDef *huart, char *fmt, ...);
void printft_flush(UART_HandleTypeDef *huart);
void Enable_Uart_O_Control(uart_o_ctrl_t *ctrl, UART_HandleTypeDef *huart);
void Enable_Uart_E_Control(uart_e_ctrl_t *ctrl, UART_HandleTypeDef *huart);
uint8_t Uart_O_Data_Process(uart_o_ctrl_t *ctrl, UART_HandleTypeDef *huart);
uint8_t Uart_O_Timeout_Check(uart_o_ctrl_t *ctrl);
uint8_t Uart_E_Data_Process(uart_e_ctrl_t *ctrl, UART_HandleTypeDef *huart);
void Enable_Uart_DMA_Control(uart_dma_ctrl_t *ctrl, UART_HandleTypeDef *huart);
uint8_t Uart_DMA_Data_Process(uart_dma_ctrl_t *ctrl, UART_HandleTypeDef *huart,
                              uint16_t Size);
#endif
