#include "uart_pack.h"

#include <stdarg.h>
#include <string.h>

#include "lfbb.h"
#include "lwprintf.h"
#include "ulist.h"

#if 0  // memcpy 实现
static inline void uart_memcpy(void *dst, const void *src, size_t len) {
  uint8_t *dst8 = (uint8_t *)dst;
  uint8_t *src8 = (uint8_t *)src;
  while (len--) {
    *dst8++ = *src8++;
  }
}
#else
#define uart_memcpy memcpy
#endif

#if UART_CFG_TX_USE_DMA
#define UART_CFG_NOT_READY                  \
  (huart->gState != HAL_UART_STATE_READY || \
   (huart->hdmatx && huart->hdmatx->State != HAL_DMA_STATE_READY))
#else
#define UART_CFG_NOT_READY huart->gState != HAL_UART_STATE_READY
#endif

static inline void _send_func(UART_HandleTypeDef *huart, const uint8_t *data,
                              size_t len) {
#if UART_CFG_TX_USE_DMA
  if (huart->hdmatx) {
#if UART_CFG_DCACHE_COMPATIBLE
    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)data, len);
#endif
    HAL_UART_Transmit_DMA(huart, data, len);
  } else
#endif
#if UART_CFG_TX_USE_IT
    HAL_UART_Transmit_IT(huart, data, len);
#else
  HAL_UART_Transmit(huart, data, len, 0xFFFF);
#endif
}

#if UART_CFG_ENABLE_FIFO_TX
typedef struct {              // FIFO串口发送控制结构体
  LFBB_Inst_Type lfbb;        // 发送缓冲区
  size_t sending;             // 正在发送的数据长度
  UART_HandleTypeDef *huart;  // 串口句柄
#if !MOD_CFG_USE_OS_NONE
  MOD_MUTEX_HANDLE mutex;  // 互斥锁
#endif
} uart_fifo_tx_t;

static ulist_t fifo_tx_list = {
    .data = NULL,
    .cap = 0,
    .num = 0,
    .isize = sizeof(uart_fifo_tx_t),
    .cfg = ULIST_CFG_NO_ALLOC_EXTEND | ULIST_CFG_CLEAR_DIRTY_REGION,
};

int Uart_FifoTxInit(UART_HandleTypeDef *huart, uint8_t *buf, size_t bufSize) {
  if (!bufSize) return -1;
  uart_fifo_tx_t *ctrl = (uart_fifo_tx_t *)ulist_append(&fifo_tx_list);
  if (!ctrl) return -1;
  if (!buf) {
    ctrl->lfbb.data = m_alloc(bufSize);
    if (!ctrl->lfbb.data) {
      ulist_remove(&fifo_tx_list, ctrl);
      return NULL;
    }
    buf = ctrl->lfbb.data;
  }
  LFBB_Init(&ctrl->lfbb, buf, bufSize);
  ctrl->huart = huart;
  ctrl->sending = 0;
#if !MOD_CFG_USE_OS_NONE
  ctrl->mutex = MOD_MUTEX_CREATE();
#endif
  return 0;
}

void Uart_FifoTxDeInit(UART_HandleTypeDef *huart) {
  ulist_foreach(&fifo_tx_list, uart_fifo_tx_t, ctrl) {
    if (ctrl->huart == huart) {
      HAL_UART_DMAStop(ctrl->huart);
      HAL_UART_AbortTransmit_IT(ctrl->huart);
      MOD_MUTEX_FREE(ctrl->mutex);
      m_free(ctrl->lfbb.data);
      ulist_remove(&fifo_tx_list, ctrl);
      return;
    }
  }
}

static uart_fifo_tx_t *is_fifo_tx(UART_HandleTypeDef *huart) {
  if (!fifo_tx_list.num) return NULL;
  ulist_foreach(&fifo_tx_list, uart_fifo_tx_t, ctrl) {
    if (ctrl->huart == huart) return ctrl;
  }
  return NULL;
}

static void fifo_exchange(uart_fifo_tx_t *ctrl, uint8_t force) {
  UART_HandleTypeDef *huart = ctrl->huart;
  if (!force && UART_CFG_NOT_READY) return;  // 串口正在发送
  if (ctrl->sending) {
    LFBB_ReadRelease(&ctrl->lfbb, ctrl->sending);
    ctrl->sending = 0;
  }
  uint8_t *data = LFBB_ReadAcquire(&ctrl->lfbb, &ctrl->sending);
  if (data) {
    _send_func(huart, data, ctrl->sending);
  } else {
    ctrl->sending = 0;
  }
}

static inline uint8_t *wait_fifo(uart_fifo_tx_t *ctrl, size_t len) {
  if (len > ctrl->lfbb.size - 1) return NULL;
#if UART_CFG_FIFO_TIMEOUT < 0
  return LFBB_WriteAcquire(&ctrl->lfbb, len);
#else
#if UART_CFG_FIFO_TIMEOUT > 0
  m_time_t _start_time = m_time_ms();
#endif  // UART_CFG_FIFO_TIMEOUT
  uint8_t *data;
  while (1) {
    data = LFBB_WriteAcquire(&ctrl->lfbb, len);
    if (data) return data;
    fifo_exchange(ctrl, 0);
#if UART_CFG_FIFO_TIMEOUT > 0
    m_delay_ms(1);
    if (m_time_ms() - _start_time > UART_CFG_FIFO_TIMEOUT) return NULL;
#endif  // UART_CFG_FIFO_TIMEOUT
  }
  return NULL;
#endif  // UART_CFG_FIFO_TIMEOUT
}

static inline int fifo_send(uart_fifo_tx_t *ctrl, const uint8_t *data,
                            size_t len) {
  MOD_MUTEX_ACQUIRE(ctrl->mutex);
  uint8_t *buf = wait_fifo(ctrl, len);
  if (!buf) {
    MOD_MUTEX_RELEASE(ctrl->mutex);
    return -1;
  }
  uart_memcpy(buf, data, len);
  LFBB_WriteRelease(&ctrl->lfbb, len);
  fifo_exchange(ctrl, 0);
  MOD_MUTEX_RELEASE(ctrl->mutex);
  return 0;
}

static inline size_t fifo_send_va(uart_fifo_tx_t *ctrl, const char *fmt,
                                  va_list ap) {
  size_t sendLen, writeLen;
  MOD_MUTEX_ACQUIRE(ctrl->mutex);
  // 第一次尝试直接获取缓冲区最大可用长度
  uint8_t *buf = LFBB_WriteAcquireAlt(&ctrl->lfbb, &sendLen);
  if (!buf) {
    MOD_MUTEX_RELEASE(ctrl->mutex);
    return 0;
  }
  do {  // 尝试写入数据
    writeLen = lwprintf_vsnprintf((char *)buf, sendLen, fmt, ap);
    if (writeLen >= ctrl->lfbb.size - 1) {  // 需求长度超过缓冲区最大长度
      MOD_MUTEX_RELEASE(ctrl->mutex);
      return 0;
    }
    if (writeLen + 1 >= sendLen) {  // 数据长度超过缓冲区最大长度
      buf = wait_fifo(ctrl, writeLen + 2);  // 等待缓冲区可用
      if (!buf) {
        MOD_MUTEX_RELEASE(ctrl->mutex);
        return 0;
      }
      sendLen = writeLen + 2;
      continue;  // 重新写入数据
    }
    break;
  } while (1);
  LFBB_WriteRelease(&ctrl->lfbb, writeLen + 1);
  fifo_exchange(ctrl, 0);
  MOD_MUTEX_RELEASE(ctrl->mutex);
  return sendLen;
}

#endif  // UART_CFG_ENABLE_FIFO_TX

static int pub_lwprintf_fn(int ch, lwprintf_t *lwobj) {
  if (ch == '\0') return 0;
  UART_HandleTypeDef *huart = (UART_HandleTypeDef *)lwobj->arg;
  if (huart) {
    HAL_UART_Transmit(huart, (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
  }
  return -1;
}

uint8_t disable_printft = 0;  // 关闭printf输出

int printft_block_ap(UART_HandleTypeDef *huart, const char *fmt, va_list ap) {
  lwprintf_t lwp_pub;
  lwp_pub.out_fn = pub_lwprintf_fn;
  lwp_pub.arg = (void *)huart;
  return lwprintf_vprintf_ex(&lwp_pub, fmt, ap);
}

int printft_block(UART_HandleTypeDef *huart, const char *fmt, ...) {
  if (unlikely(disable_printft)) return 0;
  if (UART_CFG_NOT_READY) {
    HAL_UART_DMAStop(huart);
    HAL_UART_AbortTransmit_IT(huart);
  }
  va_list ap;
  va_start(ap, fmt);
  int sendLen = printft_block_ap(huart, fmt, ap);
  va_end(ap);
  return sendLen;
}

int printft(UART_HandleTypeDef *huart, const char *fmt, ...) {
  if (unlikely(disable_printft)) return 0;
  int sendLen;
  va_list ap;
#if UART_CFG_ENABLE_FIFO_TX
  uart_fifo_tx_t *ctrl = is_fifo_tx(huart);
  if (ctrl) {
    va_start(ap, fmt);
    sendLen = fifo_send_va(ctrl, fmt, ap);
    va_end(ap);
    return sendLen;
  }
#endif
  va_start(ap, fmt);
  sendLen = printft_block_ap(huart, fmt, ap);
  va_end(ap);
  return sendLen;
}

void printft_flush(UART_HandleTypeDef *huart) {
#if UART_CFG_ENABLE_FIFO_TX
  uart_fifo_tx_t *ctrl = is_fifo_tx(huart);
  if (ctrl) {
    MOD_MUTEX_ACQUIRE(ctrl->mutex);
    while (!LFBB_IsEmpty(&ctrl->lfbb)) {
      fifo_exchange(ctrl, 0);
    }
    MOD_MUTEX_RELEASE(ctrl->mutex);
    return;
  }
#endif
  while (UART_CFG_NOT_READY) {
    __NOP();
  }
}

static int itm_lwprintf_fn(int ch, lwprintf_t *lwobj) {
  if (ch == '\0') return 0;
  ITM_SendChar(ch);
  return ch;
}

int ITM_Printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  lwprintf_t lwp_pub;
  lwp_pub.out_fn = itm_lwprintf_fn;
  int sendLen = lwprintf_vprintf_ex(&lwp_pub, fmt, ap);
  va_end(ap);
  return sendLen;
}

int Uart_Send(UART_HandleTypeDef *huart, const uint8_t *data, size_t len) {
  if (!len) return 0;
#if UART_CFG_ENABLE_FIFO_TX
  uart_fifo_tx_t *ctrl = is_fifo_tx(huart);
  if (ctrl) return fifo_send(ctrl, data, len);
#endif
#if UART_CFG_TX_TIMEOUT > 0
  m_time_t _start_time = m_time_ms();
  while (UART_CFG_NOT_READY) {
    // m_delay_ms(1);
    if (m_time_ms() - _start_time > UART_CFG_TX_TIMEOUT) return -1;
  }
#elif UART_CFG_TX_TIMEOUT < 0
  if (UART_CFG_NOT_READY) return -1;
#else
  while (UART_CFG_NOT_READY) {
    __NOP();
  }
#endif
  _send_func(huart, data, len);
  return 0;
}

int Uart_SendFast(UART_HandleTypeDef *huart, uint8_t *data, size_t len) {
  if (!len) return 0;
  if (UART_CFG_NOT_READY) return -1;
  _send_func(huart, data, len);
  return 0;
}

typedef struct {                      // 中断FIFO串口接收控制结构体
  lfifo_t fifo;                       // 接收保存区
  uint8_t tmp;                        // 临时变量
  UART_HandleTypeDef *huart;          // 串口句柄
  void (*rxCallback)(lfifo_t *fifo);  // 接收完成回调函数
} uart_fifo_rx_t;

static ulist_t fifo_rx_list = {
    .data = NULL,
    .cap = 0,
    .num = 0,
    .isize = sizeof(uart_fifo_rx_t),
    .cfg = ULIST_CFG_NO_ALLOC_EXTEND | ULIST_CFG_CLEAR_DIRTY_REGION,
};

lfifo_t *Uart_FifoRxInit(UART_HandleTypeDef *huart, uint8_t *buf,
                         size_t bufSize, void (*rxCallback)(lfifo_t *fifo)) {
  if (!bufSize) return NULL;
  uart_fifo_rx_t *ctrl = (uart_fifo_rx_t *)ulist_append(&fifo_rx_list);
  if (!ctrl) return NULL;
  if (!buf) {
    if (LFifo_Init(&ctrl->fifo, bufSize) != 0) {
      ulist_remove(&fifo_rx_list, ctrl);
      return NULL;
    }
  } else {
    LFifo_AssignBuf(&ctrl->fifo, buf, bufSize);
  }
  ctrl->huart = huart;
  ctrl->rxCallback = rxCallback;
  HAL_UART_Receive_IT(huart, &ctrl->tmp, 1);
  return &ctrl->fifo;
}

inline void Uart_RxProcess(UART_HandleTypeDef *huart) {
  ulist_foreach(&fifo_rx_list, uart_fifo_rx_t, ctrl) {
    if (ctrl->huart != huart) continue;
    LFifo_WriteByte(&ctrl->fifo, ctrl->tmp);
    HAL_UART_Receive_IT(ctrl->huart, &ctrl->tmp, 1);
    return;
  }
}

inline void Uart_TxProcess(UART_HandleTypeDef *huart) {
#if UART_CFG_ENABLE_FIFO_TX
  uart_fifo_tx_t *fifo = is_fifo_tx(huart);
  if (fifo) fifo_exchange(fifo, 1);
#endif
}

#if UART_CFG_ENABLE_DMA_RX
static uint8_t dma_tmp;  // DMA垃圾箱

typedef struct {                          // DMA串口接收控制结构体
  LFBB_Inst_Type lfbb;                    // 环形数据缓冲区
  uint8_t *pBuffer;                       // 接收缓冲区指针
  size_t pSize;                           // 接收缓冲区大小
  UART_HandleTypeDef *huart;              // 串口句柄
  void (*rxCallback)(uint8_t *, size_t);  // 接收完成回调函数
  uint8_t cbkInIRQ;  // 回调函数是否在中断中执行
} uart_dma_rx_t;

static ulist_t dma_rx_list = {
    .data = NULL,
    .cap = 0,
    .num = 0,
    .isize = sizeof(uart_dma_rx_t),
    .cfg = ULIST_CFG_NO_ALLOC_EXTEND | ULIST_CFG_CLEAR_DIRTY_REGION,
};

LFBB_Inst_Type *Uart_DmaRxInit(UART_HandleTypeDef *huart, uint8_t *buf,
                               size_t bufSize,
                               void (*rxCallback)(uint8_t *data, size_t len),
                               uint8_t cbkInIRQ) {
  if (!bufSize) return NULL;
  uart_dma_rx_t *ctrl = (uart_dma_rx_t *)ulist_append(&dma_rx_list);
  if (!ctrl) return NULL;
  ctrl->huart = huart;
  ctrl->rxCallback = rxCallback;
  ctrl->cbkInIRQ = cbkInIRQ;
  if (!buf) {
    ctrl->lfbb.data = m_alloc(bufSize);
    if (!ctrl->lfbb.data) {
      ulist_remove(&dma_rx_list, ctrl);
      return NULL;
    }
    buf = ctrl->lfbb.data;
  }
  LFBB_Init(&ctrl->lfbb, buf, bufSize);
  ctrl->pBuffer = LFBB_WriteAcquireAlt(&ctrl->lfbb, &ctrl->pSize);
  if (!ctrl->pBuffer) {
    m_free(ctrl->lfbb.data);
    ulist_remove(&dma_rx_list, ctrl);
    return NULL;
  }
  HAL_UARTEx_ReceiveToIdle_DMA(huart, ctrl->pBuffer, ctrl->pSize);
  return &ctrl->lfbb;
}

inline void Uart_DmaRxProcess(UART_HandleTypeDef *huart, size_t Size) {
  size_t len;
  uint8_t *data;
  ulist_foreach(&dma_rx_list, uart_dma_rx_t, ctrl) {
    if (ctrl->huart != huart) continue;
    if (ctrl->pBuffer != &dma_tmp) {  // 收到有效数据
      LFBB_WriteRelease(&ctrl->lfbb, Size);
#if UART_CFG_DCACHE_COMPATIBLE
      SCB_CleanInvalidateDCache_by_Addr(
          (uint32_t *)ctrl->pBuffer,
          ((ctrl->pSize + 31) / 32) * 32);  // 对齐
#endif
    }
    ctrl->pBuffer = LFBB_WriteAcquireAlt(&ctrl->lfbb, &ctrl->pSize);
    if (!ctrl->pBuffer) {  // 缓冲区已满，丢弃数据
      ctrl->pBuffer = &dma_tmp;
      ctrl->pSize = 1;
    }
    HAL_UARTEx_ReceiveToIdle_DMA(huart, ctrl->pBuffer, ctrl->pSize);
    if (ctrl->rxCallback && ctrl->cbkInIRQ) {
      data = LFBB_ReadAcquire(&ctrl->lfbb, &len);
      if (!data) return;  // 不应该发生
      ctrl->rxCallback(data, len);
      LFBB_ReadRelease(&ctrl->lfbb, len);
    }
    return;
  }
}
#endif  // UART_CFG_ENABLE_DMA_RX

void Uart_CallbackCheck(void) {
  ulist_foreach(&fifo_rx_list, uart_fifo_rx_t, ctrl) {
    if (ctrl->rxCallback == NULL) continue;
    if (!LFifo_IsEmpty(&ctrl->fifo)) {
      ctrl->rxCallback(&ctrl->fifo);
    }
  }
#if UART_CFG_ENABLE_DMA_RX
  size_t len;
  uint8_t *data;
  ulist_foreach(&dma_rx_list, uart_dma_rx_t, ctrl) {
    if (ctrl->rxCallback == NULL || ctrl->cbkInIRQ) continue;
    data = LFBB_ReadAcquire(&ctrl->lfbb, &len);
    if (!data) continue;
    ctrl->rxCallback(data, len);
    LFBB_ReadRelease(&ctrl->lfbb, len);
  }
#endif  // UART_CFG_ENABLE_DMA_RX
#if UART_CFG_ENABLE_CDC
  extern void cdc_check_callback(void);
  cdc_check_callback();
#endif  // UART_CFG_ENABLE_CDC
}

__IO uint8_t uart_error_state = 0;

inline void Uart_ErrorProcess(UART_HandleTypeDef *huart) {
  if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_PE)) != RESET) {  // 奇偶校验错误
    __HAL_UNLOCK(huart);
    __HAL_UART_CLEAR_PEFLAG(huart);
    uart_error_state = 1;
  }
  if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_FE)) != RESET) {  // 帧错误
    __HAL_UNLOCK(huart);
    __HAL_UART_CLEAR_FEFLAG(huart);
    uart_error_state = 2;
  }
  if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_NE)) != RESET) {  // 噪声错误
    __HAL_UNLOCK(huart);
    __HAL_UART_CLEAR_NEFLAG(huart);
    uart_error_state = 3;
  }
  if ((__HAL_UART_GET_FLAG(huart, UART_FLAG_ORE)) != RESET) {  // 接收溢出
    __HAL_UNLOCK(huart);
    __HAL_UART_CLEAR_OREFLAG(huart);
    uart_error_state = 4;
  }
  ulist_foreach(&fifo_rx_list, uart_fifo_rx_t, ctrl) {
    if (ctrl->huart->Instance == huart->Instance) {
      HAL_UART_AbortReceive_IT(ctrl->huart);
      HAL_UART_Receive_IT(ctrl->huart, &ctrl->tmp, 1);
    }
  }
#if UART_CFG_ENABLE_DMA_RX
  ulist_foreach(&dma_rx_list, uart_dma_rx_t, ctrl) {
    if (ctrl->huart->Instance == huart->Instance) {
      HAL_UART_DMAStop(ctrl->huart);
      HAL_UARTEx_ReceiveToIdle_DMA(huart, ctrl->pBuffer, ctrl->pSize);
    }
  }
#endif
#if UART_CFG_ENABLE_FIFO_TX
  uart_fifo_tx_t *fifo = is_fifo_tx(huart);
  if (fifo) {
    HAL_UART_DMAStop(fifo->huart);
    HAL_UART_AbortTransmit_IT(fifo->huart);
    fifo->sending = 0;
    fifo_exchange(fifo, 1);
  }
#endif
}

#if UART_CFG_REWRITE_HANLDER  // 重写HAL中断处理函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  Uart_RxProcess(huart);
}
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
  Uart_TxProcess(huart);
}
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
  Uart_ErrorProcess(huart);
}
#if UART_CFG_ENABLE_DMA_RX
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
  Uart_DmaRxProcess(huart, Size);
}
void HAL_UARTEx_TxEventCallback(UART_HandleTypeDef *huart) {
  Uart_TxProcess(huart);
}
#endif  // UART_CFG_ENABLE_DMA_RX
#endif  // UART_CFG_REWRITE_HANLDER

#if VOFA_CFG_ENABLE

static float VOFA_Buffer[VOFA_CFG_BUFFER_SIZE + 1];
static uint8_t vofa_index = 0;
static const uint32_t vofa_endbit = 0x7F800000;

__attribute__((always_inline, flatten)) void Vofa_Add(float value) {
  if (vofa_index < VOFA_CFG_BUFFER_SIZE) VOFA_Buffer[vofa_index++] = value;
}

void Vofa_AddSeq(float *value, uint8_t len) {
  if (vofa_index + len >= VOFA_CFG_BUFFER_SIZE) return;
  uart_memcpy(&VOFA_Buffer[vofa_index], value, len * sizeof(float));
  vofa_index += len;
}

void Vofa_Clear(void) { vofa_index = 0; }

void Vofa_Send(UART_HandleTypeDef *huart) {
  if (vofa_index == 0) return;
  Vofa_AddSeq((float *)&vofa_endbit, 1);
  Uart_Send(huart, (uint8_t *)VOFA_Buffer, vofa_index * sizeof(float));
  vofa_index = 0;
}

void Vofa_SendFast(UART_HandleTypeDef *huart) {
  if (vofa_index == 0) return;
  uart_memcpy(&VOFA_Buffer[vofa_index], &vofa_endbit, sizeof(float));
  Uart_SendFast(huart, (uint8_t *)VOFA_Buffer, ++vofa_index * sizeof(float));
  vofa_index = 0;
}

#if UART_CFG_ENABLE_CDC
void Vofa_SendCDC(void) {
  if (vofa_index == 0) return;
  Vofa_AddSeq((float *)&vofa_endbit, 1);
  CDC_Send((uint8_t *)VOFA_Buffer, vofa_index * sizeof(float));
  vofa_index = 0;
}
#endif  // UART_CFG_ENABLE_CDC
#endif  // VOFA_CFG_ENABLE
