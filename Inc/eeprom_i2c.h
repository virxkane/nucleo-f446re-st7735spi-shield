// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020 by Chernov A.A.                                    *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef EEPROM_I2C_H
#define EEPROM_I2C_H

#include "stm32f4xx_i2c_simple.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Write one byte into EEPROM IC. For example, M24C08 or, 24AA04/24LC04B.
 * @param  i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_address IC device address, 0xA0.
 * @param mem_address memory address 0 - 1024. High bits (8 & 9) added info devices address.
 * @param data data bytes.
 * @return The number of bytes written, if the operation was successful, otherwise error code (value < 0).
 */
int16_t eeprom_i2c_byte_write(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, uint8_t data);

/**
 * @brief Write one page into EEPROM IC. For example, M24C08 or, 24AA04/24LC04B.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_address IC device address, 0xA0.
 * @param mem_address page memory address 0 - 1024. High bits (8 & 9) added info devices address.
 * @param data pointer to buffer with data
 * @param sz size of data buffer. Can't be more than page size (16).
 * @return The number of bytes written, if the operation was successful, otherwise error code (value < 0).
 *
 * If the page memory address is not aligned to the page size and the size of the data buffer
 * is such that the data cannot fit in this memory page, the buffer size is truncated to prevent this.
 */
int16_t eeprom_i2c_page_write(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, const void* data, uint8_t sz);

/**
 * @brief Convenience method for data writing. Write some data into EEPROM IC. For example, M24C08 or, 24AA04/24LC04B.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_address IC device address, 0xA0.
 * @param mem_address page memory address 0 - 1024. High bits (8 & 9) added info devices address.
 * @param data pointer to buffer with data
 * @param sz size of data buffer.
 * @return The number of bytes written, if the operation was successful, otherwise error code (value < 0).
 *
 * If the page memory address is not aligned to the page size and the size of the data buffer
 * is such that the data cannot fit in this memory page, data automatically writes into next page(s).
 */
int16_t eeprom_i2c_write_wrap(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, const void* data, uint16_t sz);

/**
 * @brief Read one byte from EEPROM IC. For example, M24C08 or, 24AA04/24LC04B.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_address IC device address, 0xA0.
 * @param mem_address memory address 0 - 1024. High bits (8 & 9) added info devices address.
 * @param pdata pointer buffer.
 * @return The number of bytes read, if the operation was successful, otherwise error code (value < 0).
 */
int16_t eeprom_i2c_read_byte(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, uint8_t* pdata);

/**
 * @brief Read one byte from EEPROM IC. For example, M24C08 or, 24AA04/24LC04B.
 * @param i2c pointer to a hardware I2C module (I2C_TypeDef structure), for example I2C1 or I2C2.
 * @param dev_address IC device address, 0xA0.
 * @param mem_address memory address 0 - 1024. High bits (8 & 9) added info devices address.
 * @param pdata pointer buffer.
 * @param sz number of bytes requested for reading.
 * @return The number of bytes read, if the operation was successful, otherwise error code (value < 0).
 */
int16_t eeprom_i2c_read(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, void* pdata, uint16_t sz);

#ifdef __cplusplus
}
#endif

#endif // EEPROM_I2C_H
