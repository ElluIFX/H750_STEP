/**
 * @file candy.h
 * @brief see candy.c for details.
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2021-12-23
 *
 * THINK DIFFERENTLY
 */

#ifndef CANDY_H
#define CANDY_H
/***** settings *****/
#define _RGB_LED_USED_ 1  // 是否使用RGB LED
#define _DELAYUS_USED_ 0  // 是否使用delay_us函数

/**** end settings ****/
#include <main.h>

// delay_us
#if _DELAYUS_USED_
#define _DELAY_TIM htim6  // 开启一个定时器 时基为1Mhz
void delay_us(uint16_t us);
#endif
// single func functions

float fmap(float x, float in_min, float in_max, float out_min, float out_max);
double dmap(double x, double in_min, double in_max, double out_min,
            double out_max);
// RGB LED functions
#if _RGB_LED_USED_
void RGB(uint8_t r, uint8_t g, uint8_t b);
#endif  // _RGB_LED_USED_

/************** 语法糖 *********************/
// 各种数量的连接宏
#define __CONNECT2(__A, __B) __A##__B
#define __CONNECT3(__A, __B, __C) __A##__B##__C
#define __CONNECT4(__A, __B, __C, __D) __A##__B##__C##__D
#define __CONNECT5(__A, __B, __C, __D, __E) __A##__B##__C##__D##__E
#define __CONNECT6(__A, __B, __C, __D, __E, __F) __A##__B##__C##__D##__E##__F
#define __CONNECT7(__A, __B, __C, __D, __E, __F, __G) \
  __A##__B##__C##__D##__E##__F##__G
#define __CONNECT8(__A, __B, __C, __D, __E, __F, __G, __H) \
  _A##__B##__C##__D##__E##__F##__G##__H
#define __CONNECT9(__A, __B, __C, __D, __E, __F, __G, __H, __I) \
  __A##__B##__C##__D##__E##__F##__G##__H##__I
#define CONNECT2(__A, __B) __CONNECT2(__A, __B)
#define CONNECT3(__A, __B, __C) __CONNECT3(__A, __B, __C)
#define CONNECT4(__A, __B, __C, __D) __CONNECT4(__A, __B, __C, __D)
#define CONNECT5(__A, __B, __C, __D, __E) __CONNECT5(__A, __B, __C, __D, __E)
#define CONNECT6(__A, __B, __C, __D, __E, __F) \
  __CONNECT6(__A, __B, __C, __D, __E, __F)
#define CONNECT7(__A, __B, __C, __D, __E, __F, __G) \
  __CONNECT7(__A, __B, __C, __D, __E, __F, __G)
#define CONNECT8(__A, __B, __C, __D, __E, __F, __G, __H) \
  __CONNECT8(__A, __B, __C, __D, __E, __F, __G, __H)
#define CONNECT9(__A, __B, __C, __D, __E, __F, __G, __H, __I)

#define VA_NUM_ARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, __N, ...) __N
// 获取参数个数
#define VA_NUM_ARGS(...) \
  VA_NUM_ARGS_IMPL(__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1)

// 连接宏
#define CONNECT(...) CONNECT2(CONNECT, VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

// using
#define __using1(__declare)                                   \
  for (__declare, *CONNECT3(__using_, __LINE__, _ptr) = NULL; \
       CONNECT3(__using_, __LINE__, _ptr)++ == NULL;)

#define __using2(__declare, __on_leave_expr)                  \
  for (__declare, *CONNECT3(__using_, __LINE__, _ptr) = NULL; \
       CONNECT3(__using_, __LINE__, _ptr)++ == NULL; __on_leave_expr)

#define __using3(__declare, __on_enter_expr, __on_leave_expr)                \
  for (__declare, *CONNECT3(__using_, __LINE__, _ptr) = NULL;                \
       CONNECT3(__using_, __LINE__, _ptr)++ == NULL ? ((__on_enter_expr), 1) \
                                                    : 0;                     \
       __on_leave_expr)

// 局部变量 args: 局部变量 [进入代码] [离开代码]
#define using(...) CONNECT2(__using, VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define __LINE_NAME(__NAME) CONNECT3(__, __NAME, __LINE__)
// 保证原子性的代码块，不会被中断打断
#define SAFE_ATOM_CODE                                   \
  using(uint32_t __LINE_NAME(temp) = ({                  \
          uint32_t __LINE_NAME(temp2) = __get_PRIMASK(); \
          __disable_irq();                               \
          __LINE_NAME(temp2);                            \
        }),                                              \
        __set_PRIMASK(__LINE_NAME(temp)))

#define foreach2(__array, __type)                                            \
  using(__type * _ = __array) for (uint_fast32_t CONNECT2(count, __LINE__) = \
                                       dimof(__array);                       \
                                   CONNECT2(count, __LINE__) > 0;            \
                                   _++, CONNECT2(count, __LINE__)--)

#define foreach3(__array, __type, __pt)                                      \
  using(__type * __pt = __array) for (uint_fast32_t CONNECT2(                \
                                          count, __LINE__) = dimof(__array); \
                                      CONNECT2(count, __LINE__) > 0;         \
                                      __pt++, CONNECT2(count, __LINE__)--)
// 数组长度
#define dimof(__array) (sizeof(__array) / sizeof(__array[0]))

// 遍历数组 args: 数组 元素类型 [元素指针名(默认为_)]
#define foreach(...) CONNECT2(foreach, VA_NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)

/************** 语法糖 END *********************/
#endif  // CANDY_H
