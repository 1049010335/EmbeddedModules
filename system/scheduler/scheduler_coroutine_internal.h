#include "scheduler.h"
#include "ulist.h"

#define _CR_STATE_READY 0     // 就绪态
#define _CR_STATE_RUNNING 1   // 运行态
#define _CR_STATE_AWAITING 2  // 等待态
#define _CR_STATE_SLEEPING 3  // 睡眠态
#define _CR_STATE_STOPPED 4   // 停止态

#pragma pack(1)
typedef struct {  // 协程句柄结构
  long ptr;       // 协程跳入地址
  void *local;    // 协程局部变量储存区
} __cortn_data_t;

typedef struct {         // 协程句柄结构
  uint8_t state;         // 协程状态
  ulist_t dataList;      // 协程数据列表
  __cortn_data_t *data;  // 协程数据指针
  uint8_t depth;         // 协程当前深度
  uint8_t callDepth;     // 协程嵌套深度
  uint64_t sleepUntil;   // 等待态结束时间(us), 0表示暂停
  void *msg;             // 协程消息指针
  const char *name;      // 协程名
} __cortn_handle_t;
#pragma pack()

#define __chd__ You_forgot___async__
#define __async_check__ You_forgot_ASYNC_INIT

#define __async__ __cortn_handle_t *__chd__
typedef void (*cortn_func_t)(__async__, void *args);  // 协程函数指针类型

extern const char *__Internal_GetName(void);
extern void *__Internal_InitLocal(size_t size);
extern uint8_t __Internal_AwaitEnter(void);
extern uint8_t __Internal_AwaitReturn(void);
extern void __Internal_Delay(uint64_t delayUs);
extern uint8_t __Internal_AcquireMutex(const char *name);
extern void __Internal_ReleaseMutex(const char *name);
extern uint8_t __Internal_WaitBarrier(const char *name);
static void __Internal_AwaitMsg(__async__, void **msgPtr);

#define __ASYNC_INIT                                     \
  __crap:;                                               \
  void *__async_check__ = &&__crap;                      \
  do {                                                   \
    if ((__chd__->data[__chd__->depth].ptr) != 0)        \
      goto *(void *)(__chd__->data[__chd__->depth].ptr); \
  } while (0);

#define __ASYNC_LOCAL_START struct _cr_local {
#define __ASYNC_LOCAL_END                                        \
  }                                                              \
  *_cr_local_p = __Internal_InitLocal(sizeof(struct _cr_local)); \
  if (_cr_local_p == NULL) return;                               \
  ASYNC_INIT

#define __LOCAL(var) (_cr_local_p->var)

#define __YIELD()                                    \
  do {                                               \
    __label__ l;                                     \
    (__chd__->data[__chd__->depth].ptr) = (long)&&l; \
    return;                                          \
  l:;                                                \
    (__chd__->data[__chd__->depth].ptr) = 0;         \
    __async_check__ = __async_check__;               \
  } while (0)

#define __AWAIT(func_cmd, args...)                   \
  do {                                               \
    __label__ l;                                     \
    (__chd__->data[__chd__->depth].ptr) = (long)&&l; \
  l:;                                                \
    if (__Internal_AwaitEnter()) {                   \
      func_cmd(__chd__, ##args);                     \
      if (!__Internal_AwaitReturn()) return;         \
      (__chd__->data[__chd__->depth].ptr) = 0;       \
    }                                                \
    __async_check__ = __async_check__;               \
  } while (0)

#define __AWAIT_DELAY_US(us)             \
  do {                                   \
    __Internal_Delay(us);                \
    __chd__->state = _CR_STATE_SLEEPING; \
    YIELD();                             \
  } while (0)

#define __AWAIT_YIELD_UNTIL(cond) \
  do {                            \
    YIELD();                      \
  } while (!(cond))

#define __AWAIT_DELAY_UNTIL(cond, delayMs) \
  do {                                     \
    AWAIT_DELAY(delayMs);                  \
  } while (!(cond))

#define __AWAIT_ACQUIRE_MUTEX(mutex_name)       \
  do {                                          \
    if (!__Internal_AcquireMutex(mutex_name)) { \
      __chd__->state = _CR_STATE_AWAITING;      \
      YIELD();                                  \
    }                                           \
  } while (0)

#define __AWAIT_BARRIER(barr_name)            \
  do {                                        \
    if (!__Internal_WaitBarrier(barr_name)) { \
      __chd__->state = _CR_STATE_AWAITING;    \
      YIELD();                                \
    }                                         \
  } while (0)
