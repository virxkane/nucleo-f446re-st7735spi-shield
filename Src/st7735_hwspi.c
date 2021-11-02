// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2019-2021 by Chernov A.A.                               *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#include "st7735_hwspi.h"
#include "st7735_cmds.h"

#include "systick_delay_ms.h"

#include <stdlib.h>

struct _st7735_handle
{
	SPI_TypeDef *hspi;
	GPIO_TypeDef *gpio_cs;
	GPIO_TypeDef *gpio_dc;
	uint16_t gpio_cs_pin_msk;
	uint16_t gpio_dc_pin_msk;
	uint8_t ipf;
	uint8_t width;
	uint8_t height;
	uint8_t _pad;
	uint32_t _1us_delay_cnt;
	uint32_t _1ms_delay_cnt;
	uint32_t _10ms_delay_cnt;
};	// size: 28

#define UNUSED(x) { x = x; }

#define _SET_CS_LOW(handle)		{ handle->gpio_cs->BSRR = (handle->gpio_cs_pin_msk << 16); }
#define _SET_CS_HIGH(handle)	{ handle->gpio_cs->BSRR = handle->gpio_cs_pin_msk; }
#define _SET_DC_LOW(handle)		{ handle->gpio_dc->BSRR = (handle->gpio_dc_pin_msk << 16); }
#define _SET_DC_HIGH(handle)	{ handle->gpio_dc->BSRR = handle->gpio_dc_pin_msk; }

#if 0
#define _DELAY_US	1

static inline void _delay_us(struct _st7735_handle* handle, uint32_t us)
{
	volatile uint32_t cnt = handle->_1us_delay_cnt * us;
	while (cnt--)
		;
}
#endif

#if 0
static inline void _delay_ms(struct _st7735_handle* handle, uint32_t ms)
{
	volatile uint32_t cnt = handle->_1ms_delay_cnt * ms;
	while (cnt--)
		;
}
#endif

static inline int8_t _st7735_wrcmd(struct _st7735_handle* handle, uint8_t cmd)
{
	volatile uint32_t cnt;
#if 0
	// Clear 'overrun' flag
	cnt = handle->_1ms_delay_cnt;
	volatile uint32_t tmp;
	while ((handle->hspi->SR & SPI_SR_OVR_Msk) == SPI_SR_OVR_Msk && cnt != 0)
	{
		tmp = handle->hspi->DR;
		tmp = handle->hspi->SR;
		UNUSED(tmp);
		cnt--;
	}
	if (cnt == 0)
		return -1;
#endif
	_SET_CS_LOW(handle);
	_SET_DC_LOW(handle);
	if ((handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
	{
		// Now we are in BIDI mode with direction 'receive only'
		// Disable SPI before change direction
		handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
		// Enable master mode and set direction to 'transmit only'
		handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
							 (0x01 << SPI_CR1_BIDIOE_Pos);
	}
	// Enable SPI
	if ((handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
		handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
	// Assumed that any previous transmission completed
	// start transmission
	handle->hspi->DR = cmd;
	// wait this transmissions
	cnt = handle->_1ms_delay_cnt;
	while ((handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
		cnt--;
	while ((handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
		cnt--;
	_SET_CS_HIGH(handle);
	return cnt > 0 ? 0 : -1;
}

static inline int8_t _st7735_wrcmd_args(struct _st7735_handle* handle, uint8_t cmd, uint8_t *args, uint16_t args_sz)
{
	volatile uint32_t cnt;
#if 0
	// Clear 'overrun' flag
	cnt = handle->_1ms_delay_cnt;
	volatile uint32_t tmp;
	while ((handle->hspi->SR & SPI_SR_OVR_Msk) == SPI_SR_OVR_Msk && cnt != 0)
	{
		tmp = handle->hspi->DR;
		tmp = handle->hspi->SR;
		UNUSED(tmp);
		cnt--;
	}
	if (cnt == 0)
		return -1;
#endif
	_SET_CS_LOW(handle);
	_SET_DC_LOW(handle);
	if ((handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
	{
		// Now we are in BIDI mode with direction 'receive only'
		// Disable SPI before change direction
		handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
		// Enable master mode and set direction to 'transmit only'
		handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
							 (0x01 << SPI_CR1_BIDIOE_Pos);
	}
	// Enable SPI
	if ((handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
		handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
	// Assumed that any previous transmission completed
	// start transmission
	handle->hspi->DR = cmd;
	// wait this transmissions
	cnt = handle->_1ms_delay_cnt;
	while ((handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
		cnt--;
	while ((handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
		cnt--;
	if (cnt > 0)
	{
		// set D/CX to 1 - next bytes is data
		_SET_DC_HIGH(handle);
		for (uint16_t i = 0; i < args_sz; i++)
		{
			// start transmission
			handle->hspi->DR = args[i];
			// wait this transmissions
			while ((handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
				cnt--;
			if (0 == cnt)
				break;
		}
		while ((handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
	}
	_SET_CS_HIGH(handle);
	return cnt > 0 ? 0 : -1;
}

static inline int8_t _st7735_rdcmd(struct _st7735_handle* handle, uint8_t cmd, uint8_t *outdata, uint8_t outdata_sz)
{
	volatile uint32_t cnt;
#if 1
	// Clear 'overrun' flag
	cnt = handle->_1ms_delay_cnt;
	volatile uint32_t tmp;
	while ((handle->hspi->SR & SPI_SR_OVR_Msk) == SPI_SR_OVR_Msk && cnt != 0)
	{
		tmp = handle->hspi->DR;
		tmp = handle->hspi->SR;
		UNUSED(tmp);
		cnt--;
	}
	if (cnt == 0)
		return -1;
#endif
	_SET_CS_LOW(handle);
	_SET_DC_LOW(handle);
	if ((handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
	{
		// Now we are in BIDI mode with direction 'receive only'
		// Disable SPI before change direction
		handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
		// Enable master mode and set direction to 'transmit only'
		handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
							 (0x01 << SPI_CR1_BIDIOE_Pos);
	}
	// Enabled SPI
	if ((handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
		handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
	// start transmission
	handle->hspi->DR = cmd;
	// wait this transmissions
	cnt = handle->_1ms_delay_cnt;
	while ((handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
		cnt--;
	while ((handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
		cnt--;
	if (cnt > 0)
	{
		_SET_DC_HIGH(handle);
		// set SPI BIDI receive mode
		handle->hspi->CR1 &= ~(0x01 << SPI_CR1_BIDIOE_Pos);
		uint8_t* ptr = outdata;
		// TODO: dummy clock (ignore first bit?)
		for (uint8_t i = 0; i < outdata_sz; i++, ptr++)
		{
			// wait data from LCD controller
			cnt = handle->_1ms_delay_cnt;
			while ((handle->hspi->SR & SPI_SR_RXNE_Msk) == 0 && cnt != 0)
				cnt--;
			if (cnt > 0)
				*ptr = handle->hspi->DR;
			else
				break;
		}
		cnt = handle->_1ms_delay_cnt;
		while ((handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		while ((handle->hspi->SR & SPI_SR_RXNE_Msk) != 0 && cnt != 0)
			cnt--;
		// Disable SPI before change direction
		handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
		// Set direction to 'transmit only' to stop reading
		handle->hspi->CR1 |= (0x01 << SPI_CR1_BIDIOE_Pos);
	}
	_SET_CS_HIGH(handle);
	return cnt > 0 ? 0 : -1;
}

static inline void RGBColorToNativeColor(uint8_t* dstcolor, RGBColor rgbcolor, uint8_t ipf)
{
	switch (ipf)
	{
	case ST7735_IPF_12BIT:
		// not applicable here!
		break;
	case ST7735_IPF_16BIT:
		RGBColor2RGB565(dstcolor, rgbcolor);
		break;
	default:
	case ST7735_IPF_18BIT:
		RGBColor2RGB888(dstcolor, rgbcolor);
		break;
	}
}


st7735_handle st7735_init_hw(SPI_TypeDef* hspi,
		GPIO_TypeDef* gpio_cs, uint16_t gpio_cs_pin_pos,
		GPIO_TypeDef* gpio_dcx, uint16_t gpio_dcx_pin_pos,
		uint8_t width, uint8_t height, uint8_t ipf)
{
	uint8_t args[3];
	struct _st7735_handle* handle = (struct _st7735_handle*)malloc(sizeof(struct _st7735_handle));
	if (!handle)
		return 0;
	handle->hspi = hspi;
	handle->gpio_cs = gpio_cs;
	handle->gpio_cs_pin_msk = (0x01UL << gpio_cs_pin_pos);
	handle->gpio_dc = gpio_dcx;
	handle->gpio_dc_pin_msk = (0x01UL << gpio_dcx_pin_pos);
	handle->_1us_delay_cnt = SystemCoreClock/4000000;	// us * (SystemCoreClock / 1000000) / 4
	if (0 == handle->_1us_delay_cnt)
		handle->_1us_delay_cnt = 1;
	handle->_1ms_delay_cnt = SystemCoreClock/4000;		// us * (SystemCoreClock / 1000000) / 4
	if (0 == handle->_1ms_delay_cnt)
		handle->_1ms_delay_cnt = 1;
	handle->_10ms_delay_cnt = SystemCoreClock/400;
	if (0 == handle->_10ms_delay_cnt)
		handle->_10ms_delay_cnt = 1;
	// Software reset
	int8_t ret = _st7735_wrcmd(handle, ST7735_CMD_SWRESET);
	if (0 == ret)
	{
		// wait 120 ms.
		SysTick_Delay_ms(120);
		//_delay_ms(handle, 120);
		// Sleep Out
		ret = _st7735_wrcmd(handle, ST7735_CMD_SLPOUT);
	}
	if (0 == ret)
	{
		// wait 120 ms.
		SysTick_Delay_ms(120);
		//_delay_ms(handle, 120);
		args[0] = 0x00;
		ret = _st7735_wrcmd_args(handle, ST7735_CMD_MADCTL, args, 1);
	}
	if (0 == ret)
	{
		args[0] = ipf;
		ret = _st7735_wrcmd_args(handle, ST7735_CMD_COLMOD, args, 1);
	}
	if (0 == ret)
	{
		handle->width = width;
		handle->height = height;
		handle->ipf = ipf;
		// clear RAM
		ret = st7735_cls(handle, COLOR_BLACK);
	}
	if (0 == ret)
	{
		// Display ON
		ret = _st7735_wrcmd(handle, ST7735_CMD_DISPON);
	}
	if (0 != ret)
	{
		free(handle);
		handle = 0;
	}
	return handle;
}

st7735_handle st7735_init_hw_defs(SPI_TypeDef* hspi,
		GPIO_TypeDef* gpio_cs, uint16_t gpio_cs_pin_pos,
		GPIO_TypeDef* gpio_dcx, uint16_t gpio_dcx_pin_pos)
{
	return st7735_init_hw(hspi, gpio_cs, gpio_cs_pin_pos, gpio_dcx, gpio_dcx_pin_pos, 128, 160, ST7735_IPF_18BIT);
}

void st7735_free(st7735_handle handle)
{
	if (handle)
		free(handle);
}

int8_t st7735_cls(st7735_handle handle, RGBColor rgbcolor)
{
	if (!handle)
		return -1;
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	uint8_t ipfcolor[3];
	uint8_t ret = 0;
	uint16_t i;
	uint8_t j;

	ret = st7735_setWindow(handle, 0, 0, _handle->width, _handle->height);
	if (0 == ret)
	{
		volatile uint32_t cnt;
		// set CS to 0
		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								 (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		if (ST7735_IPF_12BIT == _handle->ipf)
		{
			// |RRRRGGGG|BBBBRRRR|GGGGBBBB|
#if RGBCOLOR_AS_NUMBER==1
			ipfcolor[0] = (RGB_GET_R(rgbcolor) & 0xF0) | (RGB_GET_G(rgbcolor) >> 4);
			ipfcolor[1] = (RGB_GET_B(rgbcolor) & 0xF0) | (RGB_GET_R(rgbcolor) >> 4);
			ipfcolor[2] = (RGB_GET_G(rgbcolor) & 0xF0) | (RGB_GET_B(rgbcolor) >> 4);
#else
			ipfcolor[0] = (rgbcolor.r & 0xF0) | (rgbcolor.g >> 4);
			ipfcolor[1] = (rgbcolor.b & 0xF0) | (rgbcolor.r >> 4);
			ipfcolor[2] = (rgbcolor.g & 0xF0) | (rgbcolor.b >> 4);
#endif
		}
		else
			RGBColorToNativeColor(ipfcolor, rgbcolor, _handle->ipf);
		uint8_t color_write_sz = _handle->ipf == ST7735_IPF_16BIT ? 2 : 3;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			//_delay_us(handle, _DELAY_US);
			for (i = 0; i < _handle->width * _handle->height; i++)
			{
				for (j = 0; j < color_write_sz; j++)
				{
					// start trasmission
					_handle->hspi->DR = ipfcolor[j];
					// wait this transmissions
					cnt = _handle->_1ms_delay_cnt;
					while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
						cnt--;
				}
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
				if (ST7735_IPF_12BIT == _handle->ipf)
					i++;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
		}
		else
			ret = -1;
	}
	else
		ret = -1;
	// set CS to 1
	_SET_CS_HIGH(_handle);
	return ret;
}

int8_t st7735_getipf(st7735_handle handle)
{
	if (!handle)
		return -1;
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	return _handle->ipf;
}

int8_t st7735_setipf(st7735_handle handle, uint8_t ipf)
{
	if (!handle)
		return -1;
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	int8_t ret = _st7735_wrcmd_args(_handle, ST7735_CMD_COLMOD, &ipf, 1);
	if (0 == ret)
		_handle->ipf = ipf;
	return ret;
}

int8_t st7735_setWindow(st7735_handle handle, int16_t xs, int16_t ys, int16_t xe, int16_t ye)
{
	if (!handle)
		return -1;
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	int8_t ret;
	uint8_t args[4];
	args[0] = (uint8_t)((uint16_t)xs >> 8);
	args[1] = (uint8_t)((uint16_t)xs & 0xFF);
	args[2] = (uint8_t)((uint16_t)xe >> 8);
	args[3] = (uint8_t)((uint16_t)xe & 0xFF);
	ret = _st7735_wrcmd_args(_handle, ST7735_CMD_CASET, args, 4);
	if (0 == ret)
	{
		args[0] = (uint8_t)((uint16_t)ys >> 8);
		args[1] = (uint8_t)((uint16_t)ys & 0xFF);
		args[2] = (uint8_t)((uint16_t)ye >> 8);
		args[3] = (uint8_t)((uint16_t)ye & 0xFF);
		ret = _st7735_wrcmd_args(_handle, ST7735_CMD_RASET, args, 4);
	}
	return ret;
}

int8_t st7735_setPixel(st7735_handle handle, int16_t x, int16_t y, RGBColor rgbcolor)
{
	int8_t ret = -1;
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	uint8_t ipfcolor[3];
	if (ST7735_IPF_12BIT == _handle->ipf)
	{
		// TODO: Read sibling pixel, combine with this and write back
	}
	else
	{
		ret = st7735_setWindow(handle, x, y, x + 1, y + 1);
		if (0 == ret)
		{
			RGBColorToNativeColor(ipfcolor, rgbcolor, _handle->ipf);
			ret = _st7735_wrcmd_args(_handle, ST7735_CMD_RAMWR, ipfcolor, _handle->ipf == ST7735_IPF_18BIT ? 3 : 2);
		}
	}
	return ret;
}

static int8_t _st7735_drawBitmap_1bpp_12ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz, const RGBColor palette[])
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		int8_t j;
		volatile uint32_t cnt;
		uint8_t pixel_data[3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								 (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t color_index;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < ((w*h) >> 3) && (i < bitmap_sz); i++)
			{
				for (j = 7; j >= 0; j--)
				{
					color_index = (*bptr >> j) & 0x01;
					if (j & 0x01)
					{
						// prepare left/low half-pixel
#if RGBCOLOR_AS_NUMBER==1
						pixel_data[0] = (RGB_GET_R(palette[color_index]) & 0xF0) | (RGB_GET_G(palette[color_index]) >> 4);
						pixel_data[1] = RGB_GET_B(palette[color_index]) & 0xF0;
#else
						pixel_data[0] = (palette[color_index].r & 0xF0) | (palette[color_index].g >> 4);
						pixel_data[1] = palette[color_index].b & 0xF0;
#endif
					}
					else
					{
						// prepare right/high half-pixel
#if RGBCOLOR_AS_NUMBER==1
						pixel_data[1] |= RGB_GET_R(palette[color_index]) >> 4;
						pixel_data[2] = (RGB_GET_G(palette[color_index]) & 0xF0) | (RGB_GET_B(palette[color_index]) >> 4);
#else
						pixel_data[1] |= palette[color_index].r >> 4;
						pixel_data[2] = (palette[color_index].g & 0xF0) | (palette[color_index].b >> 4);
#endif
						// write pair of pixels to RAM
						for (uint8_t k = 0; k < 3; k++)
						{
							// start transmission
							_handle->hspi->DR = pixel_data[k];
							// wait this transmissions
							cnt = _handle->_1ms_delay_cnt;
							while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
								cnt--;
							// Don't check transmission status - it's slow
							//if (0 == cnt)
							//	break;
						}
						if (0 == cnt)
						{
							ret = -1;
							break;
						}
					}
				}
				if (0 != ret)
					break;
				bptr++;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}


int8_t st7735_drawBitmap_1bpp(st7735_handle handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz, const RGBColor palette[])
{
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	if (w > _handle->width - x)
		w = _handle->width - x;
	if (h > _handle->height - y)
		h = _handle->height - y;
	if (ST7735_IPF_12BIT == _handle->ipf)
		return _st7735_drawBitmap_1bpp_12ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz, palette);
	int8_t ret = st7735_setWindow(handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		int8_t j;
		volatile uint32_t cnt;
		uint8_t pal_ipfcolor[2][3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								 (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start trasmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		RGBColorToNativeColor(pal_ipfcolor[0], palette[0], _handle->ipf);
		RGBColorToNativeColor(pal_ipfcolor[1], palette[1], _handle->ipf);
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t color_sz;
			uint8_t color_index;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			//_delay_us(_handle, _DELAY_US);
			color_sz = _handle->ipf == ST7735_IPF_18BIT ? 3 : 2;
			for (i = 0; i < ((w*h) >> 3) && (i < bitmap_sz); i++)
			{
				for (j = 7; j >= 0; j--)
				{
					color_index = (*bptr >> j) & 0x01;
					// write pixel to RAM
					for (uint8_t k = 0; k < color_sz; k++)
					{
						// start transmission
						_handle->hspi->DR = pal_ipfcolor[color_index][k];
						// wait this transmissions
						cnt = _handle->_1ms_delay_cnt;
						while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
							cnt--;
						// Don't check transmission status - it's slow
						//if (0 == cnt)
						//	break;
					}
					//ret = cnt > 0 ? 0 : -1;
					//if (0 != ret)
					//	break;
					if (0 == cnt)
					{
						ret = -1;
						break;
					}
				}
				if (0 != ret)
					break;
				bptr++;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

static int8_t _st7735_drawBitmap_4bpp_12ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz, const RGBColor palette[])
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		uint8_t pixel_data[3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								 (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start trasmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t color_index;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < ((w*h) >> 1) && (i < bitmap_sz); i++)
			{
				// high nibble of pixel data
				color_index = *bptr >> 4;
#if RGBCOLOR_AS_NUMBER==1
				pixel_data[0] = (RGB_GET_R(palette[color_index]) & 0xF0) | (RGB_GET_G(palette[color_index]) >> 4);
				pixel_data[1] = RGB_GET_B(palette[color_index]) & 0xF0;
#else
				pixel_data[0] = (palette[color_index].r & 0xF0) | (palette[color_index].g >> 4);
				pixel_data[1] = palette[color_index].b & 0xF0;
#endif
				// low nibble of pixel data
				color_index = *bptr & 0x0F;
#if RGBCOLOR_AS_NUMBER==1
				pixel_data[1] |= RGB_GET_R(palette[color_index]) >> 4;
				pixel_data[2] = (RGB_GET_G(palette[color_index]) & 0xF0) | (RGB_GET_B(palette[color_index]) >> 4);
#else
				pixel_data[1] |= palette[color_index].r >> 4;
				pixel_data[2] = (palette[color_index].g & 0xF0) | (palette[color_index].b >> 4);
#endif
				// write pair of pixels to RAM
				for (uint8_t k = 0; k < 3; k++)
				{
					// start transmission
					_handle->hspi->DR = pixel_data[k];
					// wait this transmissions
					cnt = _handle->_1ms_delay_cnt;
					while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
						cnt--;
					// Don't check transmission status - it's slow
					//if (0 == cnt)
					//	break;
				}
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
				bptr++;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

int8_t st7735_drawBitmap_4bpp(st7735_handle handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz, const RGBColor palette[])
{
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	if (w > _handle->width - x)
		w = _handle->width - x;
	if (h > _handle->height - y)
		h = _handle->height - y;
	if (ST7735_IPF_12BIT == _handle->ipf)
		return _st7735_drawBitmap_4bpp_12ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz, palette);
	int8_t ret = st7735_setWindow(handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		uint8_t k;
		volatile uint32_t cnt;
		static uint8_t pal_ipfcolor[16][3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								 (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		for (k = 0; k < 16; k++)
			RGBColorToNativeColor(pal_ipfcolor[k], palette[k], _handle->ipf);
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t color_sz;
			uint8_t color_index;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			//_delay_us(_handle, _DELAY_US);
			color_sz = _handle->ipf == ST7735_IPF_18BIT ? 3 : 2;
			for (i = 0; i < ((w*h) >> 1) && (i < bitmap_sz); i++)
			{
				// high nibble of pixel data
				color_index = *bptr >> 4;
				// write pixel to RAM
				for (k = 0; k < color_sz; k++)
				{
					// start transmission
					_handle->hspi->DR = pal_ipfcolor[color_index][k];
					// wait this transmissions
					cnt = _handle->_1ms_delay_cnt;
					while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
						cnt--;
					// Don't check transmission status - it's slow
					//if (0 == cnt)
					//	break;
				}
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
				if (0 != ret)
					break;
				// low nibble of pixel data
				color_index = *bptr & 0x0F;
				// write pixel to RAM
				for (k = 0; k < color_sz; k++)
				{
					// start transmission
					_handle->hspi->DR = pal_ipfcolor[color_index][k];
					// wait this transmissions
					cnt = _handle->_1ms_delay_cnt;
					while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
						cnt--;
					// Don't check transmission status - it's slow
					//if (0 == cnt)
					//	break;
				}
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
				bptr++;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

static int8_t _st7735_drawBitmap_8bpp_12ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz, const RGBColor palette[])
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		uint8_t pixel_data[3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t color_index;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < (w*h) && (i < bitmap_sz); i += 2)
			{
				// high nibble of pixel data
				color_index = *bptr;
#if RGBCOLOR_AS_NUMBER==1
				pixel_data[0] = (RGB_GET_R(palette[color_index]) & 0xF0) | (RGB_GET_G(palette[color_index]) >> 4);
				pixel_data[1] = RGB_GET_B(palette[color_index]) & 0xF0;
#else
				pixel_data[0] = (palette[color_index].r & 0xF0) | (palette[color_index].g >> 4);
				pixel_data[1] = palette[color_index].b & 0xF0;
#endif
				bptr++;
				// low nibble of pixel data
				color_index = *bptr;
#if RGBCOLOR_AS_NUMBER==1
				pixel_data[1] |= RGB_GET_R(palette[color_index]) >> 4;
				pixel_data[2] = (RGB_GET_G(palette[color_index]) & 0xF0) | (RGB_GET_B(palette[color_index]) >> 4);
#else
				pixel_data[1] |= palette[color_index].r >> 4;
				pixel_data[2] = (palette[color_index].g & 0xF0) | (palette[color_index].b >> 4);
#endif
				bptr++;
				// write pair of pixels to RAM (3 bytes)
				_handle->hspi->DR = pixel_data[0];
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[1];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[2];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

int8_t st7735_drawBitmap_8bpp(st7735_handle handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz, const RGBColor palette[])
{
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	if (w > _handle->width - x)
		w = _handle->width - x;
	if (h > _handle->height - y)
		h = _handle->height - y;
	if (ST7735_IPF_12BIT == _handle->ipf)
		return _st7735_drawBitmap_8bpp_12ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz, palette);
	int8_t ret = st7735_setWindow(handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		uint16_t k;
		volatile uint32_t cnt;
		static uint8_t pal_ipfcolor[256][3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// convert palette color table
		for (k = 0; k < 256; k++)
			RGBColorToNativeColor(pal_ipfcolor[k], palette[k], _handle->ipf);
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t color_sz;
			uint8_t color_index;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			color_sz = _handle->ipf == ST7735_IPF_18BIT ? 3 : 2;
			for (i = 0; (i < w*h) && (i < bitmap_sz); i++)
			{
				// write pixel data
				color_index = *bptr;
				cnt = _handle->_1ms_delay_cnt;
				for (k = 0; k < color_sz; k++) {
					_handle->hspi->DR = pal_ipfcolor[color_index][k];
					while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
						cnt--;
				}
				if (cnt == 0)
				{
					ret = -1;
					break;
				}
				bptr++;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

static int8_t _st7735_drawBitmap_16bpp_12ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz)
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		uint8_t pixel_data[3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t r, g, b;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < ((w*h) << 1) && (i < bitmap_sz); i += 4)
			{
				r = *bptr & 0xF8;
				g = (*bptr & 0x07) << 5;
				bptr++;
				g |= ((*bptr & 0xE0) >> 3);
				b = (*bptr & 0x1F) << 3;
				// high nibble of pixel data
				pixel_data[0] = (r & 0xF0) | (g >> 4);
				pixel_data[1] = b & 0xF0;
				bptr++;
				// low nibble of pixel data
				r = *bptr & 0xF8;
				g = (*bptr & 0x07) << 5;
				bptr++;
				g |= ((*bptr & 0xE0) >> 3);
				b = (*bptr & 0x1F) << 3;
				pixel_data[1] |= (r >> 4);
				pixel_data[2] = (g & 0xF0) | (b >> 4);
				bptr++;
				// write pair of pixels to RAM (3 bytes)
				_handle->hspi->DR = pixel_data[0];
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[1];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[2];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

static int8_t _st7735_drawBitmap_16bpp_18ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz)
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t r, g, b;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < ((w*h) << 1) && (i < bitmap_sz); i += 2)
			{
				r = *bptr & 0xF8;
				g = (*bptr & 0x07) << 5;
				bptr++;
				g |= ((*bptr & 0xE0) >> 3);
				b = (*bptr & 0x1F) << 3;
				bptr++;
				// write RGB data for the pixel to RAM (3 bytes)
				_handle->hspi->DR = r;
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = g;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = b;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

int8_t st7735_drawBitmap_16bpp(st7735_handle handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz)
{
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	if (w > _handle->width - x)
		w = _handle->width - x;
	if (h > _handle->height - y)
		h = _handle->height - y;
	if (ST7735_IPF_12BIT == _handle->ipf)
		return _st7735_drawBitmap_16bpp_12ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz);
	else if (ST7735_IPF_18BIT == _handle->ipf)
		return _st7735_drawBitmap_16bpp_18ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz);
	// For IPF 16
	int8_t ret = st7735_setWindow(handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < ((w*h) << 1) && (i < bitmap_sz); i += 2)
			{
				// write pixel to RAM
				_handle->hspi->DR = *bptr++;
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = *bptr++;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

static int8_t _st7735_drawBitmap_24bpp_12ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz)
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		uint8_t pixel_data[3];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			uint8_t r, g, b;
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < (w*h*3) && (i < bitmap_sz); i += 6)
			{
				r = *bptr++;
				g = *bptr++;
				b = *bptr++;
				// high nibble of pixel data
				pixel_data[0] = (r & 0xF0) | (g >> 4);
				pixel_data[1] = b & 0xF0;
				// low nibble of pixel data
				r = *bptr++;
				g = *bptr++;
				b = *bptr++;
				pixel_data[1] |= (r >> 4);
				pixel_data[2] = (g & 0xF0) | (b >> 4);
				// write pair of pixels to RAM (3 bytes)
				_handle->hspi->DR = pixel_data[0];
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[1];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[2];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				if (0 == cnt)
				{
					ret = -1;
					break;
				}
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

static int8_t _st7735_drawBitmap_24bpp_18ipf(struct _st7735_handle* _handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz)
{
	int8_t ret = st7735_setWindow(_handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < (w*h*3) && (i < bitmap_sz); i += 3)
			{
				// write pixel data to RAM
				_handle->hspi->DR = *bptr++;
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = *bptr++;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = *bptr++;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

int8_t st7735_drawBitmap_24bpp(st7735_handle handle, int16_t x, int16_t y, uint16_t w, uint16_t h,
		const uint8_t* bitmap_data, uint16_t bitmap_sz)
{
	struct _st7735_handle* _handle = (struct _st7735_handle*)handle;
	if (w > _handle->width - x)
		w = _handle->width - x;
	if (h > _handle->height - y)
		h = _handle->height - y;
	if (ST7735_IPF_12BIT == _handle->ipf)
		return _st7735_drawBitmap_24bpp_12ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz);
	else if (ST7735_IPF_18BIT == _handle->ipf)
		return _st7735_drawBitmap_24bpp_18ipf(_handle, x, y, w, h, bitmap_data, bitmap_sz);
	// For IPF 16
	int8_t ret = st7735_setWindow(handle, x, y, x + w - 1, y + h - 1);
	if (0 == ret)
	{
		uint16_t i;
		volatile uint32_t cnt;
		uint8_t pixel_data[2];
		const uint8_t* bptr = bitmap_data;

		_SET_CS_LOW(_handle);
		_SET_DC_LOW(_handle);
		if ((_handle->hspi->CR1 & (SPI_CR1_BIDIMODE_Msk | SPI_CR1_BIDIOE_Msk)) == SPI_CR1_BIDIMODE_Msk)
		{
			// Now we are in BIDI mode with direction 'receive only'
			// Disable SPI before change direction
			_handle->hspi->CR1 &= ~SPI_CR1_SPE_Msk;
			// Enable master mode and set direction to 'transmit only'
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
								  (0x01 << SPI_CR1_BIDIOE_Pos);
		}
		// Enable SPI
		if ((_handle->hspi->CR1 & (0x01 << SPI_CR1_SPE_Pos)) == 0)
			_handle->hspi->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// Assumed that any previous transmission completed
		// start transmission
		_handle->hspi->DR = ST7735_CMD_RAMWR;
		// wait this transmissions
		cnt = _handle->_1ms_delay_cnt;
		while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			// set D/CX to 1 - next bytes is data
			_SET_DC_HIGH(_handle);
			for (i = 0; i < (w*h*3) && (i < bitmap_sz); i += 3)
			{
				// convert pixel data
				uint8_t r = *bptr++;
				uint8_t g = *bptr++;
				uint8_t b = *bptr++;
				pixel_data[0] = (r & 0xF8) | (g >> 5);
				pixel_data[1] = ((g & 0x1C) << 3) | (b >> 3);
				// write pixel to RAM
				_handle->hspi->DR = pixel_data[0];
				cnt = _handle->_1ms_delay_cnt;
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
				_handle->hspi->DR = pixel_data[1];
				while ((_handle->hspi->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
					cnt--;
			}
			cnt = _handle->_10ms_delay_cnt;
			while ((_handle->hspi->SR & SPI_SR_BSY_Msk) == SPI_SR_BSY_Msk && cnt != 0)
				cnt--;
			if (0 == cnt)
				ret = -1;
		}
		else
			ret = -1;
		// set CS to 1
		_SET_CS_HIGH(_handle);
	}
	return ret;
}

int8_t st7735_wrcmd(st7735_handle handle, uint8_t cmd)
{
	if (!handle)
		return -1;
	return _st7735_wrcmd((struct _st7735_handle*)handle, cmd);
}

int8_t st7735_wrcmd_args(st7735_handle handle, uint8_t cmd, uint8_t *args, uint16_t args_sz)
{
	if (!handle)
		return -1;
	return _st7735_wrcmd_args((struct _st7735_handle*)handle, cmd, args, args_sz);
}

int8_t st7735_rdcmd(st7735_handle handle, uint8_t cmd, uint8_t *outdata, uint8_t outdata_sz)
{
	if (!handle)
		return -1;
	return _st7735_rdcmd((struct _st7735_handle*)handle, cmd, outdata, outdata_sz);
}
