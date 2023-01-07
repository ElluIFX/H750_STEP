/**
 * @file queue.h
 * @brief see queue.c for details.
 * @author Ellu (lutaoyu@163.com)
 * @version 1.0
 * @date 2023-01-07
 *
 * THINK DIFFERENTLY
 */

#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <candy.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "PLOOC/plooc.h"

#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__

#if defined(__BYTE_QUEUE_CLASS_IMPLEMENT__)
#define __PLOOC_CLASS_IMPLEMENT__
#elif defined(__BYTE_QUEUE_CLASS_INHERIT__)
#define __PLOOC_CLASS_INHERIT__
#endif

#include "PLOOC/plooc_class.h"

#ifndef queue_protect
#define queue_protect()
#endif

#define __DEQUEUE_0(__QUEUE)                                         \
  ({                                                                 \
    uint8_t __LINE_NAME(temp);                                       \
    dequeue_bytes((__QUEUE), &__LINE_NAME(temp), (sizeof(uint8_t))); \
    __LINE_NAME(temp);                                               \
  })
#define __DEQUEUE_1(__QUEUE, __ADDR) \
  dequeue_bytes((__QUEUE), __ADDR, (sizeof(typeof(*(__ADDR)))))

#define __DEQUEUE_2(__QUEUE, __ADDR, __ITEM_COUNT) \
  dequeue_bytes((__QUEUE), (__ADDR),               \
                __ITEM_COUNT * (sizeof(typeof((__ADDR[0])))))

#define __DEQUEUE_3(__QUEUE, __ADDR, __ITEM_COUNT, __TYPE) \
  dequeue_bytes((__QUEUE), (__ADDR), (__ITEM_COUNT * sizeof(__TYPE)))

#define __ENQUEUE_0__(__QUEUE, __VALUE)                                 \
  do {                                                                  \
    typeof((__VALUE)) __LINE_NAME(value) = __VALUE;                     \
    enqueue_bytes((__QUEUE), &(__LINE_NAME(value)), (sizeof(__VALUE))); \
  } while (0)

#define __ENQUEUE_0(__QUEUE, __VALUE) __ENQUEUE_0__(__QUEUE, __VALUE)

#define __ENQUEUE_1(__QUEUE, __ADDR, __ITEM_COUNT) \
  enqueue_bytes((__QUEUE), (__ADDR),               \
                __ITEM_COUNT * (sizeof(typeof((__ADDR[0])))))

#define __ENQUEUE_2(__QUEUE, __ADDR, __ITEM_COUNT, __TYPE) \
  enqueue_bytes((__QUEUE), (__ADDR), (__ITEM_COUNT * sizeof(__TYPE)))
#define __PEEK_QUEUE_0(__QUEUE)                                         \
  ({                                                                    \
    uint8_t __LINE_NAME(temp);                                      \
    peek_bytes_queue((__QUEUE), &__LINE_NAME(temp), (sizeof(uint8_t))); \
    __LINE_NAME(temp);                                                  \
  })
#define __PEEK_QUEUE_1(__QUEUE, __ADDR) \
  peek_bytes_queue((__QUEUE), (__ADDR), (sizeof(typeof(*(__ADDR)))))

#define __PEEK_QUEUE_2(__QUEUE, __ADDR, __ITEM_COUNT) \
  peek_bytes_queue((__QUEUE), (__ADDR),               \
                   __ITEM_COUNT * (sizeof(typeof((__ADDR[0])))))

#define __PEEK_QUEUE_3(__QUEUE, __ADDR, __ITEM_COUNT, __TYPE) \
  peek_bytes_queue((__QUEUE), (__ADDR), (__ITEM_COUNT * sizeof(__TYPE)))

#define __QUEUE_INIT_0(__QUEUE, __BUFFER) \
  queue_init_byte((__QUEUE), (__BUFFER), sizeof((__BUFFER)))

#define __QUEUE_INIT_1(__QUEUE, __BUFFER, __SIZE) \
  queue_init_byte((__QUEUE), (__BUFFER), (__SIZE))

// 初始化队列 args: 队列指针 缓冲区指针 [缓冲区大小]
#define QUEUE_INIT(__QUEUE, __BUFFER, ...)   \
  __PLOOC_EVAL(__QUEUE_INIT_, ##__VA_ARGS__) \
  (__QUEUE, __BUFFER, ##__VA_ARGS__)

// 入队 args: 队列指针 数据值/(缓冲区指针 数据个数 [数据类型])
#define ENQUEUE(__QUEUE, __ADDR, ...)     \
  __PLOOC_EVAL(__ENQUEUE_, ##__VA_ARGS__) \
  (__QUEUE, __ADDR, ##__VA_ARGS__)

// 出队 args: 队列指针 [缓冲区指针] [数据个数] [数据类型]
#define DEQUEUE(__QUEUE, ...)             \
  __PLOOC_EVAL(__DEQUEUE_, ##__VA_ARGS__) \
  (__QUEUE, ##__VA_ARGS__)

// 查看队列 args: 队列指针 [缓冲区指针] [数据个数] [数据类型]
#define PEEK_QUEUE(__QUEUE, ...)             \
  __PLOOC_EVAL(__PEEK_QUEUE_, ##__VA_ARGS__) \
  (__QUEUE, ##__VA_ARGS__)

// 队列是否为空
#define IS_ENQUEUE_EMPTY(__QUEUE) is_byte_queue_empty(__QUEUE)

// 重置查看点
#define RESET_PEEK(__QUEUE) reset_peek_byte(__QUEUE)

// 出队所有已查看的数据
#define GET_ALL_PEEKED(__QUEUE) get_all_peeked_byte(__QUEUE)

// 获取查看点
#define GET_PEEK_STATUS(__QUEUE) get_peek_status(__QUEUE)

// 设置查看点
#define RESTORE_PEEK_STATUS(__QUEUE, __COUNT) \
  restore_peek_status(__QUEUE, __COUNT)

// 获取队列中数据个数
#define GET_QUEUE_COUNT(__QUEUE) get_queue_count(__QUEUE)

// 获取队列中剩余空间个数
#define GET_QUEUE_AVAILABLE(__QUEUE) get_queue_available_count(__QUEUE)

declare_class(byte_queue_t)
    def_class(byte_queue_t,

              private_member(uint8_t *pchBuffer; uint16_t hwSize;
                             uint16_t hwHead; uint16_t hwTail;
                             uint16_t hwLength; uint16_t hwPeek;
                             uint16_t hwPeekLength;)

                  ) end_def_class(byte_queue_t);
extern byte_queue_t *queue_init_byte(byte_queue_t *ptObj, void *pBuffer,
                                     uint16_t hwItemSize);
extern bool enqueue_byte(byte_queue_t *ptQueue, uint8_t chByte);
extern int16_t enqueue_bytes(byte_queue_t *ptObj, void *pchByte,
                             uint16_t hwLength);
extern bool dequeue_byte(byte_queue_t *ptQueue, uint8_t *pchByte);
extern int16_t dequeue_bytes(byte_queue_t *ptObj, void *pchByte,
                             uint16_t hwLength);
extern bool is_byte_queue_empty(byte_queue_t *ptQueue);
extern bool peek_byte_queue(byte_queue_t *ptQueue, uint8_t *pchByte);
extern bool reset_peek_byte(byte_queue_t *ptQueue);
extern bool get_all_peeked_byte(byte_queue_t *ptQueue);
extern int16_t peek_bytes_queue(byte_queue_t *ptObj, void *pchByte,
                                uint16_t hwLength);
extern uint16_t get_peek_status(byte_queue_t *ptQueue);
extern bool restore_peek_status(byte_queue_t *ptQueue, uint16_t hwCount);
extern int16_t get_queue_count(byte_queue_t *ptObj);
extern int16_t get_queue_available_count(byte_queue_t *ptObj);

#undef __BYTE_QUEUE_CLASS_INHERIT__
#undef __BYTE_QUEUE_CLASS_IMPLEMENT__

#endif  // __QUEUE_H__
