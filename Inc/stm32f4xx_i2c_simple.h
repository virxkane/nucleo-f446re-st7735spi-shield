// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020 by Chernov A.A.                                    *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef STM32F1XX_I2C_SIMPLE_H
#define STM32F1XX_I2C_SIMPLE_H

#include "mcu_defs.h"
#include <stdint.h>

#define I2C_ERR_OK				(0)			// No error
#define I2C_ERR_BUSY			(-1)		// bus busy
#define I2C_ERR_START_FAIL		(-2)		// START condition failed
#define I2C_ERR_ADDR_FAIL		(-3)		// address transmission failed, possible NACK
#define I2C_ERR_TXE_FAIL		(-4)		// wait previous transmitions failed
#define I2C_ERR_BTF_FAIL		(-5)		// wait current byte transmitions failed
#define I2C_ERR_RXNE_FAIL		(-6)		// wait current byte reception failed
#define I2C_ERR_ACK_FAIL		(-7)		// ACK failed after transmiting

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Функция-обертка для передачи данных ведомому устройству.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_addr адрес ведомого устройства.
 * @param data массив передаваемых данных.
 * @param sz размер передаваемых данных.
 * @return Количество переданных байт или значение меньше 0 (код ошибки) в случае ошибки.
 *
 * Данная функция включает в себя операции передачи сигнала START, адреса устройства и
 * самих данных и наконец передачу сигнала STOP, т.е. для отправки одного пакета данных
 * достаточно только этой функции.
 */
int16_t i2c_sendDataWrapper(I2C_TypeDef* i2c, uint8_t dev_addr, const uint8_t* data, uint16_t sz);

/**
 * @brief Функция-обертка для чтения данных с ведомого устройства.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_addr адрес ведомого устройства.
 * @param dst указатель на область памяти, куда необходимо записать данные.
 * @param sz размер буфера dst.
 * @return Количество полученных байт или значение меньше 0 (код ошибки) в случае ошибки.
 *
 * Данная функция включает в себя операции передачи сигнала START, адреса устройства,
 * чтения массива данных и наконец передачу сигнала STOP, т.е. для чтения одного пакета данных
 * достаточно только этой функции.
 */
int16_t i2c_readDataWrapper(I2C_TypeDef* i2c, uint8_t dev_addr, uint8_t* dst, uint16_t sz);

/**
 * @brief Send START condition
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @return 1 в случае успешной операции, в противном случае значение меньше 0 (код ошибки).
 */
int16_t i2c_start(I2C_TypeDef* i2c);

/**
 * @brief Transmit STOP condition
 * @param  i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 */
void i2c_stop(I2C_TypeDef* i2c);

/**
 * @brief Send device address.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param data
 * @return 1 в случае успешной операции, в противном случае значение меньше 0 (код ошибки).
 */
int16_t i2c_send_addr(I2C_TypeDef* i2c, uint8_t addr);

/**
 * @brief Send some data.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param data byte to write.
 * @return 1 в случае успешной операции, в противном случае значение меньше 0 (код ошибки).
 */
int16_t i2c_send_byte(I2C_TypeDef* i2c, uint8_t data);

/**
 * @brief i2c_sendPacket
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param data
 * @param sz
 * @return Количество переданных байт или значение меньше 0 (код ошибки) в случае ошибки.
 */
uint16_t i2c_send_packet(I2C_TypeDef* i2c, const uint8_t* data, uint16_t sz);

#ifdef __cplusplus
}
#endif

#endif // STM32F1XX_I2C_SIMPLE_H
