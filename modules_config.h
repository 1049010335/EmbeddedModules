#ifndef _MODULES_CONF_H_
#define _MODULES_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MOD_CFG_USE_OS_KLITE 1
#define MOD_CFG_TIME_MATHOD_PERF_COUNTER 1
#define MOD_CFG_HEAP_MATHOD_KLITE 1
#define MOD_CFG_DELAY_MATHOD_KLITE 1
#define LOG_CFG_ENABLE 1
#define LOG_CFG_ENABLE_TIMESTAMP 1
#define LOG_CFG_ENABLE_COLOR 1
#define LOG_CFG_ENABLE_FUNC_LINE 1
#define LOG_CFG_ENABLE_ASSERT 1
#define LOG_CFG_ENABLE_DEBUG 1
#define LOG_CFG_ENABLE_PASS 1
#define LOG_CFG_ENABLE_INFO 1
#define LOG_CFG_ENABLE_WARN 1
#define LOG_CFG_ENABLE_ERROR 1
#define LOG_CFG_ENABLE_FATAL 1
#define LOG_CFG_A_COLOR T_RED
#define LOG_CFG_D_COLOR T_CYAN
#define LOG_CFG_P_COLOR T_LGREEN
#define LOG_CFG_I_COLOR T_GREEN
#define LOG_CFG_W_COLOR T_YELLOW
#define LOG_CFG_E_COLOR T_RED
#define LOG_CFG_F_COLOR T_MAGENTA
#define LOG_CFG_L_COLOR T_BLUE
#define LOG_CFG_R_COLOR T_BLUE
#define LOG_CFG_T_COLOR T_YELLOW
#define LOG_CFG_D_STR "DEBUG"
#define LOG_CFG_P_STR "PASS"
#define LOG_CFG_I_STR "INFO"
#define LOG_CFG_W_STR "WARN"
#define LOG_CFG_E_STR "ERROR"
#define LOG_CFG_F_STR "FATAL"
#define LOG_CFG_L_STR "LIMIT"
#define LOG_CFG_R_STR "REFRESH"
#define LOG_CFG_A_STR "ASSERT"
#define LOG_CFG_T_STR "TIMEIT"
#define LOG_CFG_PRINTF printf
#define LOG_CFG_TIMESTAMP ((float)((uint64_t)m_time_ms()) / 1000)
#define LOG_CFG_TIMESTAMP_FMT "%.3fs"
#define LOG_CFG_PREFIX ""
#define LOG_CFG_SUFFIX "\r\n"
#define BOARD_I2C_CFG_USE_NONE 1
#define UART_CFG_ENABLE_DMA_RX 1
#define UART_CFG_ENABLE_FIFO_TX 1
#define UART_CFG_TX_TIMEOUT 5
#define UART_CFG_DCACHE_COMPATIBLE 1
#define UART_CFG_REWRITE_HANLDER 1
#define UART_CFG_TX_USE_DMA 1
#define UART_CFG_TX_USE_IT 1
#define UART_CFG_PRINTF_REDIRECT 1
#define UART_CFG_PRINTF_USE_UART 1
#define UART_CFG_PRINTF_UART_PORT huart1
#define UART_CFG_PRINTF_REDIRECT_PUTX 1
#define VOFA_CFG_ENABLE 1
#define VOFA_CFG_BUFFER_SIZE 32
#define SCH_CFG_ENABLE_TASK 1
#define SCH_CFG_ENABLE_EVENT 1
#define SCH_CFG_ENABLE_COROUTINE 1
#define SCH_CFG_ENABLE_CALLLATER 1
#define SCH_CFG_ENABLE_SOFTINT 1
#define SCH_CFG_STATIC_NAME 1
#define SCH_CFG_STATIC_NAME_LEN 16
#define SCH_CFG_COMP_RANGE_US 1000
#define SCH_CFG_ENABLE_TERMINAL 1

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _MODULES_CONF_H_ */