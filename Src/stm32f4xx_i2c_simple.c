// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020 by Chernov A.A.                                    *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#include "stm32f4xx_i2c_simple.h"

#define UNUSED(x)	((void)x)

int16_t i2c_sendDataWrapper(I2C_TypeDef* i2c, uint8_t dev_addr, const uint8_t* data, uint16_t sz)
{
	int16_t res = 0;
	const uint8_t* ptr = data;
	uint16_t i = 0;
	volatile uint32_t c = 0;
	uint8_t ack_fail = 0;

	while (i2c->SR2 & I2C_SR2_BUSY_Msk && c < 65535)
		c++;
	if (c < 65535)
	{
		i2c->CR1 = I2C_CR1_PE_Msk;						// enable I2C, clear POS, ACK
		i2c->SR1 &= ~I2C_SR1_AF_Msk;					// clear AF bit
		i2c->CR1 |= I2C_CR1_START_Msk;					// START
		// Wait START condition generation
		c = 0;
		while ((i2c->SR1 & I2C_SR1_SB_Msk) == 0 && c < 65535)
			c++;
		if (c < 65535)									// START has been transmitted
		{
			i2c->DR = dev_addr;							// SLA+W, write mode (LSB of dev_addr always is zero)
			// Wait address transmission
			c = 0;
			ack_fail = 0;
			while ((i2c->SR1 & I2C_SR1_ADDR_Msk) == 0 && c < 65535)
			{
				if (i2c->SR1 & I2C_SR1_AF_Msk)
				{
					ack_fail = 1;
					break;
				}
				c++;
			}
			if (!ack_fail && c < 65535)					// SLA+W has been transmitted, ACK has been received
			{
				c = i2c->SR1;							// clear ADDR bits
				UNUSED(c);
				c = i2c->SR2;
				UNUSED(c);
				for (i = 0; i < sz; i++, ptr++)
				{
					c = 0;
					ack_fail = 0;
					while ((i2c->SR1 & I2C_SR1_TXE_Msk) == 0 && c < 65535)
					{
						if (i2c->SR1 & I2C_SR1_AF_Msk)
						{
							ack_fail = 1;
							break;
						}
						c++;
					}
					if (!ack_fail && c < 65535)			// TXE is set
					{
						i2c->DR = *ptr;					// Data
#if 1
						if (i2c->SR1 & I2C_SR1_BTF_Msk && i < sz - 1)
						{
							i++;
							res++;
							ptr++;
							i2c->DR = *ptr;
						}
#endif
						// Wait data transmission
						c = 0;
						ack_fail = 0;
						while ((i2c->SR1 & I2C_SR1_BTF_Msk) == 0 && c < 65535)
						{
							if (i2c->SR1 & I2C_SR1_AF_Msk)
							{
								ack_fail = 1;
								break;
							}
							c++;
						}
						if (!ack_fail && c < 65535)		// data has been transmitted, ACK has been received
							res++;
						else
						{
							if (ack_fail)
								res = I2C_ERR_ACK_FAIL;	// ACK failed after transmiting
							else
								res = I2C_ERR_BTF_FAIL;	// wait transmitions failed
							break;
						}
					}
					else
					{
						if (ack_fail)
							res = I2C_ERR_ACK_FAIL;		// ACK failed before transmiting
						else
							res = I2C_ERR_TXE_FAIL;		// wait previous transmitions failed
						break;
					}
				}
			}
			else
				res = I2C_ERR_ADDR_FAIL;				// address failed
		}
		else
			res = I2C_ERR_START_FAIL;					// START condition failed
	}
	else
		res = I2C_ERR_BUSY;								// bus BUSY
	i2c->CR1 |= I2C_CR1_STOP_Msk;
	return res;
}

int16_t i2c_readDataWrapper(I2C_TypeDef* i2c, uint8_t dev_addr, uint8_t* dst, uint16_t sz)
{
	int16_t res = 0;
	uint8_t* ptr = dst;
	uint16_t i = 0;
	uint16_t c = 0;
	volatile uint32_t tmp;
	uint8_t ack_fail = 0;

	while (i2c->SR2 & I2C_SR2_BUSY_Msk && c < 65535)
		c++;
	if (c < 65535)
	{
		i2c->CR1 = I2C_CR1_PE_Msk;						// enable I2C, clear POS
		i2c->SR1 &= ~I2C_SR1_AF_Msk;					// clear AF bit
		i2c->CR1 |= I2C_CR1_START_Msk;					// START
		// Wait START condition generation
		while ((i2c->SR1 & I2C_SR1_SB_Msk) == 0 && c < 65535)
			c++;
		if (c < 65535)									// START has been transmitted
		{
			i2c->DR = dev_addr | 0x01;					// SLA+R, read mode (LSB of dev_addr always is zero)
			// Wait address transmission
			c = 0;
			ack_fail = 0;
			while ((i2c->SR1 & I2C_SR1_ADDR_Msk) == 0 && c < 65535)
			{
				if (i2c->SR1 & I2C_SR1_AF_Msk)
				{
					ack_fail = 1;
					break;
				}
				c++;
			}
			if (!ack_fail && c < 65535)					// SLA+R has been transmitted, ACK has been received
			{
				i2c->CR1 |= I2C_CR1_ACK_Msk;
				tmp = i2c->SR1;							// clear ADDR bits
				UNUSED(tmp);
				tmp = i2c->SR2;
				UNUSED(tmp);
				for (i = 0; i < sz; i++, ptr++)
				{
					if (i == sz - 1)
						i2c->CR1 &= ~I2C_CR1_ACK_Msk;
					// Wait data reception
					c = 0;
					ack_fail = 0;
					while ((i2c->SR1 & I2C_SR1_RXNE_Msk) == 0 && c < 65535)
					{
						if (i2c->SR1 & I2C_SR1_AF_Msk)
						{
							ack_fail = 1;
							break;
						}
						c++;
					}
					if (!ack_fail && c < 65535)			// Data byte has been received
					{
						*ptr = i2c->DR;					// save data
						res++;
#if 1
						if (i2c->SR1 & I2C_SR1_BTF_Msk)
						{
							i++;
							ptr++;
							*ptr = i2c->DR;
						}
#endif
					}
					else
					{
						if (ack_fail)
							res = I2C_ERR_ACK_FAIL;
						else
							res = I2C_ERR_RXNE_FAIL;
						break;
					}
				}
			}
			else
				res = I2C_ERR_ADDR_FAIL;				// address failed
		}
		else
			res = I2C_ERR_START_FAIL;					// START condition failed
	}
	else
		res = I2C_ERR_BUSY;								// BUSY
	i2c->CR1 |= I2C_CR1_STOP_Msk;						// Transmit STOP condition
	return res;
}

int16_t i2c_start(I2C_TypeDef* i2c)
{
	uint32_t c = 0;
	while (i2c->SR2 & I2C_SR2_BUSY_Msk && c < 65535)
		c++;
	if (c >= 65535)
		return I2C_ERR_BUSY;
	i2c->CR1 |= I2C_CR1_START_Msk | I2C_CR1_PE_Msk;
	// Wait START condition generation
	while ((i2c->SR1 & I2C_SR1_SB) == 0 && c < 65535)
		c++;
	return c < 65535 ? 1 : 0;
}

void i2c_stop(I2C_TypeDef* i2c)
{
	i2c->CR1 |= I2C_CR1_STOP_Msk;
}

static inline int8_t _i2c_send_byte(I2C_TypeDef* i2c, uint8_t data)
{
	uint32_t c = 0;
	uint8_t ack_fail = 0;
	int8_t res = 0;
	while ((i2c->SR1 & I2C_SR1_TXE_Msk) == 0 && c < 65535)
	{
		if (i2c->SR1 & I2C_SR1_AF_Msk)
		{
			ack_fail = 1;
			break;
		}
		c++;
	}
	if (!ack_fail && c < 65535)							// TXE is set
	{
		i2c->DR = data;									// Data
		// Wait data transmission
		c = 0;
		ack_fail = 0;
		while ((i2c->SR1 & I2C_SR1_BTF_Msk) == 0 && c < 65535)
		{
			if (i2c->SR1 & I2C_SR1_AF_Msk)
			{
				ack_fail = 1;
				break;
			}
			c++;
		}
		if (!ack_fail && c < 65535)						// data has been transmitted, ACK has been received
			res = 1;
		else
		{
			if (ack_fail)
				res = I2C_ERR_ACK_FAIL;					// ACK failed after transmiting
			else
				res = I2C_ERR_BTF_FAIL;					// wait transmitions failed
		}
	}
	else
	{
		if (ack_fail)
			res = I2C_ERR_ACK_FAIL;						// ACK failed before transmiting
		else
			res = I2C_ERR_TXE_FAIL;						// wait previous transmitions failed
	}
	return res;
}

int16_t i2c_send_addr(I2C_TypeDef* i2c, uint8_t addr)
{
	uint32_t c = 0;
	uint8_t ack_fail = 0;
	int16_t res = 0;
	volatile uint32_t tmp;
	i2c->DR = addr;										// address
	// Wait address transmission
	c = 0;
	ack_fail = 0;
	while ((i2c->SR1 & I2C_SR1_ADDR_Msk) == 0 && c < 65535)
	{
		if (i2c->SR1 & I2C_SR1_AF_Msk)
		{
			ack_fail = 1;
			break;
		}
		c++;
	}
	if (!ack_fail && c < 65535)							// SLA+[R|W] has been transmitted, ACK has been received
	{
		res = 1;
		tmp = i2c->SR1;									// clear ADDR bits
		UNUSED(tmp);
		tmp = i2c->SR2;
		UNUSED(tmp);
	}
	else
		res = I2C_ERR_ADDR_FAIL;						// address failed
	return res;
}

int16_t i2c_send_byte(I2C_TypeDef* i2c, uint8_t data)
{
	return _i2c_send_byte(i2c, data);
}

uint16_t i2c_send_packet(I2C_TypeDef* i2c, const uint8_t* data, uint16_t sz)
{
	const uint8_t* ptr = data;
	int16_t res = 0;
	int8_t ow_res;					// one byte write operation result
	while (sz != 0)
	{
		ow_res = _i2c_send_byte(i2c, *ptr);
		if (1 == ow_res)
			res++;					// OK
		else
		{
			res = ow_res;			// error occured
			break;
		}
		++ptr;
		--sz;
	}
	return res;
}
