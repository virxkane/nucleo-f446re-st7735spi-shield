// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020-2021 by Chernov A.A.                               *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#include "form_test_image_wb.h"
#include "buttons.h"
#include "form_defs.h"
#include "gfx.h"
#include "fonts.h"
#include "vga_palette.h"

#if 0
typedef struct FormData
{
	// volatile form data
	;
} FormData;
#endif

// STM32 Nucleo Logo (XBM format)
#define stm32_nucleo_logo_width 109
#define stm32_nucleo_logo_height 109
static const unsigned char stm32_nucleo_logo_bits[] = {
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0x05, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0xff, 0x03, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff,
   0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
   0x03, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xfc, 0xff, 0x00, 0xfc, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xff, 0x7f, 0x00, 0xf8, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xc0, 0xff, 0x3f, 0x00, 0xf0, 0x3f, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x3f, 0x00, 0xe0, 0x1f, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0x1f, 0x00, 0xe0,
   0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x0f,
   0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff,
   0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0xc0, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x00, 0x00, 0x00,
   0x00, 0x00, 0xe0, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00,
   0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff, 0x0f, 0x00, 0xc0,
   0x0f, 0x00, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x0f,
   0x00, 0xe0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0xe0, 0x0f, 0x00, 0x80, 0x1f, 0x00, 0x00, 0x00, 0x00,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0xf0, 0x07, 0x00, 0x80, 0x3f, 0x00, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0xff, 0x07, 0x00, 0xf8, 0x07, 0x00, 0x80, 0x7f,
   0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0x07, 0x00, 0xfc, 0x07, 0x00,
   0x80, 0xff, 0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0xff, 0x07, 0x00, 0xff,
   0x03, 0xda, 0x00, 0xff, 0x01, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0x03,
   0xff, 0xff, 0xc3, 0xff, 0x1f, 0xfe, 0x03, 0x00, 0x00, 0xf8, 0xff, 0xff,
   0xff, 0xf1, 0xff, 0xff, 0xf9, 0xff, 0x7f, 0xfe, 0x03, 0x00, 0x00, 0xf8,
   0xff, 0xff, 0xff, 0xf9, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xf8, 0x07, 0x00,
   0x00, 0xfc, 0xff, 0xff, 0x7f, 0xfc, 0xff, 0x3f, 0xfe, 0xff, 0xff, 0xf1,
   0x0f, 0x00, 0x00, 0xfe, 0xff, 0xff, 0x3f, 0xfe, 0xff, 0x0f, 0xfe, 0xff,
   0xff, 0x83, 0x0f, 0x00, 0x00, 0xff, 0xff, 0xff, 0x03, 0xff, 0x07, 0x00,
   0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0x00, 0xff, 0xff, 0x07, 0x00, 0xff,
   0x01, 0x80, 0xff, 0x03, 0xff, 0x07, 0x00, 0x00, 0x80, 0xff, 0xff, 0x00,
   0x80, 0x7f, 0x00, 0x80, 0x7f, 0x00, 0xfc, 0x07, 0x00, 0x00, 0x80, 0xff,
   0x7f, 0x00, 0x80, 0x7f, 0x00, 0x80, 0x7f, 0x00, 0xf8, 0x07, 0x00, 0x00,
   0xc0, 0xff, 0x3f, 0x00, 0x80, 0x3f, 0x00, 0x80, 0x3f, 0x00, 0xf0, 0x0f,
   0x00, 0x00, 0xc0, 0xff, 0x3f, 0x00, 0xc0, 0x1f, 0x00, 0xc0, 0x1f, 0x00,
   0xe0, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0xc0,
   0x0f, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0x1f, 0x00, 0xc0, 0x0f,
   0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0xe0, 0xff, 0x0f, 0x00,
   0xc0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0xf0, 0xff,
   0x1f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0x00,
   0xf0, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0x0f,
   0x00, 0x00, 0xf8, 0xff, 0x1f, 0x00, 0xc0, 0x1f, 0x00, 0xc0, 0x0f, 0x00,
   0xe0, 0x0f, 0x00, 0x00, 0xf8, 0xff, 0x1f, 0x00, 0x80, 0x1f, 0x00, 0xc0,
   0x0f, 0x00, 0xc0, 0x0f, 0x00, 0x00, 0xf8, 0xff, 0x1f, 0x00, 0xc0, 0x1f,
   0x00, 0xe0, 0x0f, 0x00, 0xe0, 0x0f, 0x00, 0x00, 0xfc, 0xff, 0x3f, 0x00,
   0x80, 0x3f, 0x00, 0xe0, 0x07, 0x00, 0xf0, 0x07, 0x00, 0x00, 0xfc, 0xff,
   0x3f, 0x00, 0x80, 0x7f, 0x00, 0xf8, 0x07, 0x00, 0xf0, 0x07, 0x00, 0x00,
   0xfc, 0xff, 0xff, 0x00, 0x80, 0xff, 0x00, 0xfc, 0x07, 0x00, 0xf8, 0x07,
   0x00, 0x04, 0xfc, 0xff, 0xff, 0x01, 0x80, 0xff, 0x03, 0xfd, 0x07, 0x00,
   0xfe, 0x03, 0x00, 0x0f, 0xfc, 0xff, 0xff, 0x07, 0x00, 0xff, 0xff, 0xff,
   0x03, 0x40, 0xff, 0x03, 0xff, 0x0f, 0xfe, 0xff, 0xff, 0xff, 0x0f, 0xff,
   0xff, 0xff, 0xc3, 0xff, 0xff, 0xe1, 0xff, 0x0f, 0xfe, 0xff, 0xff, 0xff,
   0x3f, 0xfe, 0xff, 0xff, 0xf1, 0xff, 0xff, 0xf9, 0xff, 0x0f, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0xfc, 0xff, 0xff, 0xfc, 0xff, 0x7f, 0xfe, 0xff, 0x0f,
   0xfe, 0xff, 0xff, 0xff, 0xff, 0xf1, 0xff, 0x7f, 0xfe, 0xff, 0x3f, 0xfe,
   0xff, 0x0f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xc3, 0xff, 0x1f, 0xfe, 0xff,
   0x07, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x07, 0x6a, 0x01,
   0xff, 0x03, 0x80, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x07,
   0x00, 0x80, 0xff, 0x00, 0x80, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff,
   0xff, 0x07, 0x00, 0x80, 0x7f, 0x00, 0x80, 0xff, 0xff, 0x0f, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0x80, 0x3f, 0x00, 0x80, 0xff, 0xff, 0x1f,
   0xfe, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x1f, 0x00, 0xc0, 0xff,
   0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x1f, 0x00,
   0xc0, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xc0,
   0x1f, 0x00, 0xc0, 0xff, 0xff, 0x0f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x0f,
   0x00, 0xc0, 0x0f, 0x00, 0xc0, 0xff, 0xff, 0x1f, 0xfe, 0xff, 0xff, 0xff,
   0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0xff, 0xff, 0x0f, 0xfe, 0xff,
   0xff, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0xff, 0xff, 0x1f,
   0xfe, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00, 0xc0, 0xff,
   0xff, 0x0f, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00, 0xc0, 0x0f, 0x00,
   0xc0, 0xff, 0xff, 0x0f, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0xe0,
   0x1f, 0x00, 0xe0, 0xff, 0xff, 0x0f, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x1f,
   0x00, 0xe0, 0x1f, 0x00, 0xe0, 0xff, 0xff, 0x0f, 0xfc, 0xff, 0xff, 0xff,
   0xff, 0x3f, 0x00, 0xe0, 0x3f, 0x00, 0xf0, 0xff, 0xff, 0x0f, 0xfc, 0xff,
   0xff, 0xff, 0xff, 0x3f, 0x00, 0xf8, 0x7f, 0x00, 0xf8, 0xff, 0xff, 0x0f,
   0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xf8, 0xff, 0x00, 0xfc, 0xff,
   0xff, 0x07, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xfe, 0xff, 0x03,
   0xff, 0xff, 0xff, 0x07, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xef, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0xf8, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03, 0xf8, 0x7f,
   0x70, 0x40, 0x38, 0x1c, 0x1e, 0xfe, 0xff, 0xff, 0xff, 0xff, 0xff, 0x03,
   0xf0, 0x7f, 0x22, 0x61, 0x38, 0xec, 0xcc, 0xfd, 0xff, 0xff, 0xff, 0xff,
   0xff, 0x03, 0xf0, 0x3f, 0xe7, 0xf1, 0x30, 0xec, 0xed, 0xfd, 0xff, 0xff,
   0xff, 0xff, 0xff, 0x01, 0xe0, 0x3f, 0xfe, 0x79, 0x30, 0xfc, 0xed, 0xfd,
   0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0xe0, 0x7f, 0xfc, 0xf1, 0x32, 0xf8,
   0xfd, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0x7f, 0xf0, 0x79,
   0x90, 0x7c, 0xfe, 0xfd, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xc0, 0xff,
   0xe1, 0xf1, 0x94, 0xfc, 0x7d, 0xfe, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00,
   0xc0, 0xff, 0xc7, 0x79, 0x82, 0xfc, 0xbd, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x7f, 0x00, 0x80, 0x3f, 0xce, 0x71, 0x86, 0xf8, 0x9d, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x7f, 0x00, 0x80, 0x3f, 0xc7, 0x79, 0xc4, 0xec, 0xdd, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x3f, 0x00, 0x00, 0x3f, 0xe2, 0xf1, 0xcc, 0xcc,
   0xed, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x7f, 0xf0, 0x79,
   0xc6, 0x1c, 0x0e, 0xfc, 0xff, 0xff, 0xff, 0xff, 0x1f, 0x00, 0x00, 0xfe,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x0f, 0x00,
   0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0x0f, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0x07, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0x03, 0x00, 0x00, 0xf0, 0xff, 0xff, 0xff, 0xff,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0xe0, 0xff, 0xff,
   0xe3, 0xfc, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xc0,
   0xff, 0xff, 0xc3, 0xfc, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00,
   0x00, 0xc0, 0xff, 0xff, 0xc3, 0xfc, 0x7f, 0xcf, 0xff, 0xff, 0xff, 0x7f,
   0x00, 0x00, 0x00, 0x80, 0xff, 0xff, 0xc3, 0xcc, 0x18, 0xcc, 0xe1, 0xe1,
   0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x83, 0xc4, 0xcc, 0xcc,
   0xcc, 0xcc, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0xfe, 0xff, 0x93, 0xcc,
   0xc8, 0xc9, 0x8c, 0xdc, 0xff, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff,
   0x13, 0xc4, 0xcc, 0x4f, 0x40, 0xcc, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00,
   0xf8, 0xff, 0x33, 0xcc, 0xc8, 0xcf, 0xd8, 0xcc, 0xff, 0x03, 0x00, 0x00,
   0x00, 0x00, 0xe0, 0xff, 0x33, 0xcc, 0xcc, 0x4d, 0x5c, 0x9c, 0xff, 0x00,
   0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0x73, 0xcc, 0x88, 0xc8, 0xcc, 0xcc,
   0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xff, 0x73, 0x0c, 0x88, 0xcc,
   0xc0, 0xe0, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x63, 0x1c,
   0x1d, 0xcc, 0xe3, 0xe3, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc,
   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7f, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0x1f,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0xff, 0xff, 0xff,
   0xff, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0xff,
   0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0xfe, 0xff, 0xff, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0xe0, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x00, 0x00 };


void formTestImageWB_reset(AppState* state)
{
	//FormData* data = (FormData*)state->form_data;
}

void formTestImageWB_onDraw(AppState* state)
{
	const int16_t base_y = 38;
	int16_t y;

	form_drawHeader(state);

	//FormData* data = (FormData*)state->form_data;

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP
	setForegroundIndexColor(1);
	setBackgroundIndexColor(0);
	setTextIndexColor(1);
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP
	setForegroundIndexColor(VGA16_BRIGHT_BLUE);
	setBackgroundIndexColor(VGA16_WHITE);
	setTextIndexColor(VGA16_WHITE);
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP
	setForegroundIndexColor(0x35);
	setBackgroundIndexColor(0x1F);
	setTextIndexColor(0x1F);
#elif (LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565) || (LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888)
#if RGBCOLOR_AS_NUMBER==1
	RGBColor xbm_fc = 0xFF3EB4E6;
#else
	RGBColor xbm_fc = { 0x3E, 0xB4, 0xE6 };
#endif
	setForegroundRGBColor(xbm_fc);
	setBackgroundRGBColor(COLOR_WHITE);
	setTextRGBColor(COLOR_WHITE);
#endif	// LCD_COLOR_FORMAT

	y = base_y;
	setGFXFont(&NotoSans_Regular7pt_mixed);
	drawString(2, y, "B/W XBitmap");
	y += 13;

	drawXBitmap(0, y, stm32_nucleo_logo_width, stm32_nucleo_logo_height, stm32_nucleo_logo_bits, sizeof(stm32_nucleo_logo_bits));
}

void formTestImageWB_onButton(AppState* state, uint8_t btn)
{
	//FormData* data = (FormData*)state->form_data;
	switch (btn)
	{
	case BUTTON_CODE_2:
		// Change form
		state->form_code = FORM_BENCH;
		state->changed |= APPSTATE_CHANGED_FORM;
		break;
	}
}
