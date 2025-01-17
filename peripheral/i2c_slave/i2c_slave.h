#ifndef I2C_SLAVE_H_
#define I2C_SLAVE_H_

#ifdef __cplusplus
extern "C" {
#endif
#include "modules.h"

/**
 * @brief I2C从机硬件初始化
 * @param  addr 从机地址
 */
extern void Slave_I2C_Init(I2C_TypeDef* I2Cx);

/**
 * @brief I2C从机设置从机地址
 * @param  addr 从机地址
 */
extern void Slave_I2C_SetAddress(I2C_TypeDef* I2Cx, uint8_t addr);

/**
 * @brief I2C从机设置从机使能
 * @param  state 1:使能 0:关闭
 */
extern void Slave_I2C_SetEnable(I2C_TypeDef* I2Cx, uint8_t state);

/**
 * @brief I2C从机LL事件中断回调函数
 * @note 在I2Cx_EV_IRQHandler中调用(stm32xxxx_it.c)
 */
extern void Slave_I2C_IRQHandler(I2C_TypeDef* I2Cx);

/**
 * @brief I2C从机LL错误中断回调函数
 * @note 在I2Cx_ER_IRQHandler中调用(stm32xxxx_it.c)
 */
extern void Slave_I2C_Error_IRQHandler(I2C_TypeDef* I2Cx);

/**
 * @brief I2C从机传输结束回调函数(由用户实现)
 */
extern void Slave_I2C_TransmitEnd_Callback(I2C_TypeDef* I2Cx);

/**
 * @brief I2C从机传输开始回调函数(由用户实现)
 */
extern void Slave_I2C_TransmitBegin_Callback(I2C_TypeDef* I2Cx);

/**
 * @brief I2C从机获取接收数据回调函数(由用户实现)
 * @param[in]  data 刚刚接收到的数据
 */
extern void Slave_I2C_TransmitIn_Callback(I2C_TypeDef* I2Cx, uint8_t data);

/**
 * @brief I2C从机准备发送数据回调函数(由用户实现)
 * @param[out]  data 即将发送的数据
 */
extern void Slave_I2C_TransmitOut_Callback(I2C_TypeDef* I2Cx, uint8_t* data);

/**
 * @brief I2C从机设置中断状态
 * @param  state 1:使能中断 0:关闭中断
 */
extern void Slave_I2C_SetITEnable(I2C_TypeDef* I2Cx, uint8_t state);

#ifdef __cplusplus
}
#endif
#endif
