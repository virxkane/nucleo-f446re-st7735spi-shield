// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020 by Chernov A.A.                                    *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#include "eeprom_i2c.h"
#include "systick_delay_ms.h"


static inline uint8_t _memdiff_cnt(const uint8_t* s1, const uint8_t* s2, uint8_t sz)
{
	uint8_t res = 0;
	for (uint8_t i = 0; i < sz; i++)
		if (s1[i] != s2[i])
			res++;
	return res;
}


int16_t eeprom_i2c_byte_write(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, uint8_t data)
{
	int16_t res;
	uint8_t mod_dev_addr = dev_address | ((mem_address >> 8) << 1);
	uint8_t mod_mem_addr = (uint8_t)(mem_address & 0xFF);
	uint8_t buff[2];
	// set memory address
	res = i2c_sendDataWrapper(i2c, mod_dev_addr, &mod_mem_addr, 1);
	if (1 == res)
	{
		// To minimized memory wear write to EEPROM ONLY
		// when existing data differs from new data
		res = i2c_readDataWrapper(i2c, mod_dev_addr, buff, 1);
	}
	if (1 == res)
	{
		// compare old & new data byte
		if (buff[0] == data)
			res = 1;
		else
		{
			// Write new data
			buff[0] = mod_mem_addr;
			buff[1] = data;
			res = i2c_sendDataWrapper(i2c, mod_dev_addr, buff, 2);
			if (res > 0)
			{
				SysTick_Delay_ms(5);
				res--;
			}
		}
	}
	return res;
}

int16_t eeprom_i2c_page_write(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, const void* data, uint8_t sz)
{
	int16_t res;
	uint8_t mod_dev_addr = dev_address | ((mem_address >> 8) << 1);
	uint8_t mod_mem_addr = (uint8_t)(mem_address & 0xFF);
	const uint8_t* ptr = (const uint8_t*)data;
	uint8_t old_data[16];
	int16_t ow_res;					// one write operation result
	if (sz > 16)
		sz = 16;
	uint16_t end_address = mem_address + sz - 1;
	if ((mem_address & 0xFFF0) != (end_address & 0xFFF0))
	{
		// truncate buffer size to fit in one page
		//end_address = (end_address & 0xFFF0) - 1;
		end_address = mem_address | 0x000F;
		sz = end_address - mem_address + 1;
	}
	// set memory address
	res = i2c_sendDataWrapper(i2c, mod_dev_addr, &mod_mem_addr, 1);
	if (1 == res)
	{
		// To minimized memory wear write to EEPROM ONLY
		// when existing data differs from new data
		res = i2c_readDataWrapper(i2c, mod_dev_addr, old_data, sz);
	}
	if (sz == res)
	{
		res = _memdiff_cnt(old_data, ptr, sz);
		if (0 == res)
		{
			// nothing to write, no changed
			res = sz;
		}
		else if (res == sz)
		{
			// write all, no matches
			res = i2c_start(i2c);
			if (res)
				res = i2c_send_addr(i2c, mod_dev_addr);
			if (res)
				res = i2c_send_byte(i2c, mod_mem_addr);
			if (res)
				res = i2c_send_packet(i2c, ptr, sz);
			i2c_stop(i2c);
			if (res > 0)
			{
				SysTick_Delay_ms(5);
			}
		}
		else
		{
			res = 0;
			// Compare each byte
			for (uint8_t i = 0; i < sz; i++)
			{
				if (old_data[i] == ptr[i])
				{
					res++;
				}
				else
				{
					// write one byte
					ow_res = i2c_start(i2c);
					if (ow_res)
						ow_res = i2c_send_addr(i2c, mod_dev_addr);
					if (ow_res)
						ow_res = i2c_send_byte(i2c, mod_mem_addr + i);
					if (ow_res)
						ow_res = i2c_send_byte(i2c, ptr[i]);
					i2c_stop(i2c);
					if (ow_res)
					{
						SysTick_Delay_ms(5);
						res++;
					}
					else
					{
						res = -101;
						break;
					}
				}
			}
		}
	}
	else if (res > 0)
		res = -100;
	return res;
}

int16_t eeprom_i2c_write_wrap(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, const void* data, uint16_t sz)
{
	int16_t res = 0;
	uint8_t mod_dev_addr;
	uint8_t mod_mem_addr;
	uint16_t end_address;
	uint8_t old_data[16];
	const uint8_t* ptr = (const uint8_t*)data;
	uint16_t pg_sz;					// one page write operation size
	int16_t pg_res;					// one page write operation result
	int16_t ow_res;					// one byte write operation result
	while (sz != 0)
	{
		mod_dev_addr = dev_address | ((mem_address >> 8) << 1);
		mod_mem_addr = (uint8_t)(mem_address & 0xFF);
		pg_sz = sz > 16 ? 16 : (uint8_t)sz;
		end_address = mem_address + pg_sz - 1;
		if ((mem_address & 0xFFF0) != (end_address & 0xFFF0))
		{
			//end_address = (end_address & 0xFFF0) - 1;
			end_address = mem_address | 0x000F;
			pg_sz = end_address - mem_address + 1;
		}
		// set memory address
		pg_res = i2c_sendDataWrapper(i2c, mod_dev_addr, &mod_mem_addr, 1);
		if (1 == pg_res)
		{
			// To minimized memory wear write to EEPROM ONLY
			// when existing data differs from new data
			pg_res = i2c_readDataWrapper(i2c, mod_dev_addr, old_data, pg_sz);
		}
		if (pg_sz == pg_res)
		{
			pg_res = _memdiff_cnt(old_data, ptr, pg_sz);
			if (0 == pg_res)
			{
				// nothing to write, no changed
				pg_res = pg_sz;
			}
			else if (pg_res == pg_sz)
			{
				// write all, no matches
				pg_res = i2c_start(i2c);
				if (pg_res)
					pg_res = i2c_send_addr(i2c, mod_dev_addr);
				if (pg_res)
					pg_res = i2c_send_byte(i2c, mod_mem_addr);
				if (pg_res)
					pg_res = i2c_send_packet(i2c, ptr, pg_sz);
				i2c_stop(i2c);
				if (pg_res == pg_sz)
				{
					SysTick_Delay_ms(5);
				}
				else
					pg_res = -102;
			}
			else
			{
				pg_res = 0;
				// Compare each byte
				for (uint8_t i = 0; i < pg_sz; i++)
				{
					if (old_data[i] == ptr[i])
					{
						pg_res++;
					}
					else
					{
						// write one byte
						ow_res = i2c_start(i2c);
						if (ow_res)
							ow_res = i2c_send_addr(i2c, mod_dev_addr);
						if (ow_res)
							ow_res = i2c_send_byte(i2c, mod_mem_addr + i);
						if (ow_res)
							ow_res = i2c_send_byte(i2c, ptr[i]);
						i2c_stop(i2c);
						if (ow_res)
						{
							SysTick_Delay_ms(5);
							pg_res++;
						}
						else
						{
							pg_res = -101;
							break;
						}
					}
				}
			}
		}
		else if (pg_res > 0)					// не полностью прочитаны данные
			pg_res = -100;
		if (pg_res < 0)
		{
			res = pg_res;
			break;
		}
		res += pg_res;
		sz -= pg_sz;
		mem_address += pg_sz;
		ptr += pg_sz;
	}
	return res;
}

int16_t eeprom_i2c_read_byte(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, uint8_t* pdata)
{
	int16_t res;
	uint8_t mod_dev_addr = dev_address | (mem_address >> 8);
	uint8_t mod_mem_addr = (uint8_t)(mem_address & 0xFF);
	// set memory address
	res = i2c_sendDataWrapper(i2c, mod_dev_addr, &mod_mem_addr, 1);
	if (1 == res)
	{
		// read
		res = i2c_readDataWrapper(i2c, mod_dev_addr, pdata, 1);
	}
	return res;
}

int16_t eeprom_i2c_read(I2C_TypeDef* i2c, uint8_t dev_address, uint16_t mem_address, void* pdata, uint16_t sz)
{
	int16_t res = 0;
	uint8_t mod_dev_addr;
	uint8_t mod_mem_addr;
	uint16_t end_address;
	uint8_t* ptr = (uint8_t*)pdata;
	uint16_t bank_sz;
	int16_t bank_res = 0;

	while (sz != 0)
	{
		bank_sz = sz;
		mod_dev_addr = dev_address | ((mem_address >> 8) << 1);
		mod_mem_addr = (uint8_t)(mem_address & 0xFF);
		end_address = mem_address + bank_sz - 1;
		if ((mem_address & 0xFF00) != (end_address & 0xFF00))
		{
			// truncate buffer size to fit in one memory bank
			end_address = mem_address | 0xFF;
			bank_sz = end_address - mem_address + 1;
		}
		// set memory address
		bank_res = i2c_sendDataWrapper(i2c, mod_dev_addr, &mod_mem_addr, 1);
		if (1 == bank_res)
		{
			// read
			bank_res = i2c_readDataWrapper(i2c, mod_dev_addr, ptr, bank_sz);
		}
		if (bank_res == bank_sz)
			res += bank_res;
		else if (bank_res > 0)
		{
			res += bank_res;
			break;
		}
		else
		{
			res = bank_res;
			break;
		}
		sz -= bank_sz;
		mem_address += bank_sz;
		ptr += bank_sz;
	}
	return res;
}
