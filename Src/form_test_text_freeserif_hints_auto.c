// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020-2021 by Chernov A.A.                               *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#include "form_test_text_freeserif_hints_auto.h"
#include "buttons.h"
#include "form_defs.h"
#include "gfx.h"
#include "fonts.h"
#include "vga_palette.h"

#include <stdio.h>

#if 0
typedef struct FormData
{
	// volatile form data
	;
} FormData;
#endif

void formTestTextFreeSerifHintsAuto_reset(AppState* state)
{
	//FormData* data = (FormData*)state->form_data;
}

void formTestTextFreeSerifHintsAuto_onDraw(AppState* state)
{
	const int16_t base_y = 0;
	int16_t y;

	//form_drawHeader(state);

	//FormData* data = (FormData*)state->form_data;

	// test text drawing functions.

	setTextDrawMode(0);

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP
	uint8_t background_color = 0;
	uint8_t foreground_color = 1;
	uint8_t text_color_1 = 1;
	uint8_t text_color_2 = 1;
	uint8_t text_color_3 = 1;
	uint8_t text_color_4 = 1;
	uint8_t text_color_5 = 1;
	uint8_t text_color_6 = 1;
	uint8_t text_color_7 = 1;
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP
	uint8_t background_color = 0;
	uint8_t foreground_color = 15;
	uint8_t text_color_1 = VGA16_WHITE;
	uint8_t text_color_2 = VGA16_BRIGHT_GREEN;
	uint8_t text_color_3 = VGA16_BRIGHT_RED;
	uint8_t text_color_4 = VGA16_BRIGHT_CYAN;
	uint8_t text_color_5 = VGA16_BRIGHT_BLUE;
	uint8_t text_color_6 = VGA16_WHITE;
	uint8_t text_color_7 = VGA16_BRIGHT_MAGENTA;
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP
	uint8_t background_color = 0x0;
	uint8_t foreground_color = 0x0F;
	uint8_t text_color_1 = 0x0F;
	uint8_t text_color_2 = 0x0A;
	uint8_t text_color_3 = 0x40;
	uint8_t text_color_4 = 0x35;
	uint8_t text_color_5 = 0x4E;
	uint8_t text_color_6 = 0x1F;
	uint8_t text_color_7 = 0x24;
#elif (LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565) || (LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888)
	RGBColor background_color = COLOR_BLACK;
	RGBColor foreground_color = COLOR_WHITE;
	RGBColor text_color_1 = COLOR_WHITE;
	RGBColor text_color_2 = COLOR_GREEN;
#if RGBCOLOR_AS_NUMBER==1
	RGBColor text_color_3 = 0xFFFFA0A0;
	RGBColor text_color_4 = 0xFF00C0FF;
	RGBColor text_color_5 = 0xFF50E0FF;
	RGBColor text_color_6 = 0xFFFFFFFF;
	RGBColor text_color_7 = 0xFFFF00FF;
#else
	RGBColor text_color_3 = { 0xFF, 0xA0, 0xA0 };
	RGBColor text_color_4 = { 0x00, 0xC0, 0xFF };
	RGBColor text_color_5 = { 0x80, 0xE0, 0xFF };
	RGBColor text_color_6 = { 0xFF, 0xFF, 0xFF };
	RGBColor text_color_7 = { 0xFF, 0x00, 0xFF };
#endif
#endif	// LCD_COLOR_FORMAT

#if (LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP) || (LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP) || (LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP)
#define setBackgroundColor	setBackgroundIndexColor
#define setForegroundColor	setForegroundIndexColor
#define setTextColor	setTextIndexColor
#elif (LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565) || (LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888)
#define setBackgroundColor	setBackgroundRGBColor
#define setForegroundColor	setForegroundRGBColor
#define setTextColor	setTextRGBColor
#endif

	y = base_y;

	setBackgroundColor(background_color);
	setForegroundColor(foreground_color);

	setGFXFont(&FreeSerif6pt7b_hintingauto);
	setTextColor(text_color_1);
	drawString(10, y, "FreeSerif 6, 9, auto");
	y += FreeSerif6pt7b_hintingauto.yAdvance - 2;

	setGFXFont(&FreeSerif6pt7b_hintingauto);
	setTextColor(text_color_1);
	drawString(2, y, "0123456789ABCDEF");
	y += FreeSerif6pt7b_hintingauto.yAdvance - 2;

	setTextColor(text_color_2);
	drawString(2, y, "GHIJKLMNOPQRSTU");
	y += FreeSerif6pt7b_hintingauto.yAdvance - 2;

	setTextColor(text_color_3);
	drawString(2, y, "VWXYZabcdefghijklm");
	y += FreeSerif6pt7b_hintingauto.yAdvance - 2;

	setTextColor(text_color_4);
	drawString(2, y, "nopqrstuvwxyz");
	y += FreeSerif6pt7b_hintingauto.yAdvance - 2;

	setGFXFont(&FreeSerifBold6pt7b_hintingauto);
	setTextColor(text_color_5);
	drawString(2, y, "Bold Text");
	y += FreeSerifBold6pt7b_hintingauto.yAdvance - 2;

	setGFXFont(&FreeSerifItalic6pt7b_hintingauto);
	setTextColor(text_color_6);
	drawString(2, y, "Italic Text");
	y += FreeSerifItalic6pt7b_hintingauto.yAdvance - 2;

	setGFXFont(&FreeSerifBoldItalic6pt7b_hintingauto);
	setTextColor(text_color_7);
	drawString(2, y, "Bold Italic Text");
	y += FreeSerifBoldItalic6pt7b_hintingauto.yAdvance - 2;

	// 9pt
	setGFXFont(&FreeSerif9pt7b_hintingauto);
	setTextColor(text_color_1);
	drawString(2, y, "0123ABCabc");
	y += FreeSerif9pt7b_hintingauto.yAdvance - 4;

	setGFXFont(&FreeSerifBold9pt7b_hintingauto);
	setTextColor(text_color_2);
	drawString(2, y, "0123ABCabc");
	y += FreeSerifBold9pt7b_hintingauto.yAdvance - 4;

	setGFXFont(&FreeSerifItalic9pt7b_hintingauto);
	setTextColor(text_color_3);
	drawString(2, y, "0123ABCabc");
	y += FreeSerifItalic9pt7b_hintingauto.yAdvance - 4;

	setGFXFont(&FreeSerifBoldItalic9pt7b_hintingauto);
	setTextColor(text_color_4);
	drawString(2, y, "0123ABCabc");
	//y += FreeSerifBoldItalic9pt7b_hintingauto.yAdvance - 4;
}

void formTestTextFreeSerifHintsAuto_onButton(AppState* state, uint8_t btn)
{
	//FormData* data = (FormData*)state->form_data;
	switch (btn)
	{
	case BUTTON_CODE_2:
		// Change form
		state->form_code = FORM_TEXT_TEST_NOTOSANS_MONO;
		state->changed |= APPSTATE_CHANGED_FORM;
		break;
	}
}
