// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2019-2021 by Chernov A.A.                               *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#include "mcu_defs.h"
#include "st7735_hwspi.h"
#include "st7735_cmds.h"
#include "systick_delay_ms.h"
#include "gfx.h"
//#include "eeprom_i2c.h"

#include "gfx.h"
#include "fonts.h"

#include "app_state.h"
#include "form_defs.h"

#include "form_test_text_freesans_hints_mono.h"
#include "form_test_text_freesans_hints_auto.h"
#include "form_test_text_freeserif_hints_mono.h"
#include "form_test_text_freeserif_hints_auto.h"
#include "form_test_text_notosans_hints_mono.h"
#include "form_test_text_notosans_hints_auto.h"
#include "form_test_text_notoserif_hints_mono.h"
#include "form_test_text_notoserif_hints_auto.h"
#include "form_test_image_wb.h"
#include "form_bench.h"

#include <stdint.h>
#include <stdio.h>

struct op_status
{
	volatile uint8_t button_code;
	volatile uint8_t status;
};

#if 0
#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif
#endif

#define UNUSED(x) { x = x; }

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP
#define LCD1_BUFFER_SZ		2673			// 132x162 @ 1bpp
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP
#define LCD1_BUFFER_SZ		10692			// 132x162 @ 4bpp
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP
#define LCD1_BUFFER_SZ		21384			// 132x162 @ 8bpp
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565
#define LCD1_BUFFER_SZ		42768			// 132x162 @ 16bpp
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888
#define LCD1_BUFFER_SZ		64152			// 132x162 @ 24bpp
#else
#error Invalid color format value!
#endif


#define STATUS_BTN_RDY_Pos			0							// button ready flag
#define STATUS_BTN_RDY_Msk			(1 << STATUS_BTN_RDY_Pos)
#define STATUS_BTN_CPT_BT1_Pos		1							// BT1 capture flag
#define STATUS_BTN_CPT_BT1_Msk		(1 << STATUS_BTN_CPT_BT1_Pos)
#define STATUS_BTN_CPT_BT2_Pos		2							// BT2 capture flag
#define STATUS_BTN_CPT_BT2_Msk		(1 << STATUS_BTN_CPT_BT2_Pos)
#define STATUS_BTN_CPT_BT3_Pos		3							// BT3 capture flag
#define STATUS_BTN_CPT_BT3_Msk		(1 << STATUS_BTN_CPT_BT3_Pos)
#define STATUS_BTN_CPT_Msk			(STATUS_BTN_CPT_BT1_Msk | STATUS_BTN_CPT_BT2_Msk | STATUS_BTN_CPT_BT3_Msk)
#define STATUS_BTN_PRE_Pos			4							// button pressed flag
#define STATUS_BTN_PRE_Msk			(1 << STATUS_BTN_PRE_Pos)
#define STATUS_BTN_REP_Pos			5							// button repeat flag
#define STATUS_BTN_REP_Msk			(1 << STATUS_BTN_REP_Pos)
//#define STATUS_RTC_TO_Pos			7							// RTC timeout flag
//#define STATUS_RTC_TO_Msk			(1 << STATUS_RTC_TO_Pos)


#if RGBCOLOR_AS_NUMBER==1

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP

static const RGBColor lcd1_1bpp_palette[2] = 	// 1bpp palette for LCD
{
	0xFF000000,			// 0 - black
	0xFFFFFFFF,			// 1 - white
};

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP

static const RGBColor lcd1_4bpp_palette[16] = 	// 4bpp palette for LCD
{
	// Standard VGA 16 color palette
	0xFF000000,			// 0  - black
	0xFF0000AA,			// 1  - blue
	0xFF00AA00,			// 2  - green
	0xFF00AAAA,			// 3  - cyan
	0xFFAA0000,			// 4  - red
	0xFFAA00AA,			// 5  - magenta
	0xFFAA5500,			// 6  - brown
	0xFFAAAAAA,			// 7  - gray
	0xFF555555,			// 8  - dark gray
	0xFF5555FF,			// 9  - bright blue
	0xFF55FF55,			// 10 - bright green
	0xFF55FFFF,			// 11 - bright cyan
	0xFFFF5555,			// 12 - bright red
	0xFFFF55FF,			// 13 - bright magenta
	0xFFFFFF55,			// 14 - yellow
	0xFFFFFFFF,			// 15 - white
};		// 64 bytes

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP

static const RGBColor lcd1_8bpp_palette[256] = 	// 8bpp palette for LCD
{
	// Standard VGA 256 color palette
	// row 0 (R)
	0xFF000000,	// 0  - black
	0xFF0000AA,	// 1  - blue
	0xFF00AA00,	// 2  - green
	0xFF00AAAA,	// 3  - cyan
	0xFFAA0000,	// 4  - red
	0xFFAA00AA,	// 5  - magenta
	0xFFAA5500,	// 6  - brown
	0xFFAAAAAA,	// 7  - gray
	0xFF555555,	// 8  - dark gray
	0xFF5555FF,	// 9  - bright blue
	0xFF55FF55,	// 10 - bright green
	0xFF55FFFF,	// 11 - bright cyan
	0xFFFF5555,	// 12 - bright red
	0xFFFF55FF,	// 13 - bright magenta
	0xFFFFFF55,	// 14 - yellow
	0xFFFFFFFF,	// 15 - white
	// row 1, gray
	0xFF000000,	// 0, 16
	0xFF141414,	// 1, 17
	0xFF202020,	// 2, 18
	0xFF303030,	// 3, 19
	0xFF383838,	// 4, 20
	0xFF444444,	// 5, 21
	0xFF505050,	// 6, 22
	0xFF606060,	// 7, 23
	0xFF707070,	// 8, 24
	0xFF808080,	// 9, 25
	0xFF909090,	// 10,26
	0xFFA0A0A0,	// 11,27
	0xFFB4B4B4,	// 12,28
	0xFFC8C8C8,	// 13,29
	0xFFE0E0E0,	// 14,30
	0xFFFFFFFF,	// 15,31
	// row 2 (R)
	0xFF0000FF,	// 0, 32
	0xFF4000FF,	// 1, 33
	0xFF8000FF,	// 2, 34
	0xFFC000FF,	// 3, 35
	0xFFFF00FF,	// 4, 36
	0xFFFF00C0,	// 5, 37
	0xFFFF0080,	// 6, 38
	0xFFFF0040,	// 7, 39
	0xFFFF0000,	// 8, 40
	0xFFFF4000,	// 9, 41
	0xFFFF8000,	// 10,42
	0xFFFFC000,	// 11,43
	0xFFFFFF00,	// 12,44
	0xFFC0FF00,	// 13,45
	0xFF80FF00,	// 14,46
	0xFF40FF00,	// 15,47
	// row 3 (R)
	0xFF00FF00,	// 0, 48
	0xFF00FF40,	// 1, 49
	0xFF00FF80,	// 2, 50
	0xFF00FFC0,	// 3, 51
	0xFF00FFFF,	// 4, 52
	0xFF00C0FF,	// 5, 53
	0xFF0080FF,	// 6, 54
	0xFF0040FF,	// 7, 55
	0xFF8080FF,	// 8, 56
	0xFFA080FF,	// 9, 57
	0xFFC080FF,	// 10,58
	0xFFE080FF,	// 11,59
	0xFFFF80FF,	// 12,60
	0xFFFF80E0,	// 13,61
	0xFFFF80C0,	// 14,62
	0xFFFF80A0,	// 15,63
	// row 4 (R)
	0xFFFF8080,	// 0, 64
	0xFFFFA080,	// 1, 65
	0xFFFFC080,	// 2, 66
	0xFFFFE080,	// 3, 67
	0xFFFFFF80,	// 4, 68
	0xFFE0FF80,	// 5, 69
	0xFFC0FF80,	// 6, 70
	0xFFA0FF80,	// 7, 71
	0xFF80FF80,	// 8, 72
	0xFF80FFA0,	// 9, 73
	0xFF80FFC0,	// 10,74
	0xFF80FFE0,	// 11,75
	0xFF80FFFF,	// 12,76
	0xFF80E0FF,	// 13,77
	0xFF80C0FF,	// 14,78
	0xFF80A0FF,	// 15,79
	// row 5 (R)
	0xFFB4B4FF,	// 0, 80
	0xFFC4B4FF,	// 1, 81
	0xFFD8B4FF,	// 2, 82
	0xFFE8B4FF,	// 3, 83
	0xFFFFB4FF,	// 4, 84
	0xFFFFB4E8,	// 5, 85
	0xFFFFB4D8,	// 6, 86
	0xFFFFB4C4,	// 7, 87
	0xFFFFB4B4,	// 8, 88
	0xFFFFC4B4,	// 9, 89
	0xFFFFD8B4,	// 10,90
	0xFFFFE8B4,	// 11,91
	0xFFFFFFB4,	// 12,92
	0xFFE8FFB4,	// 13,93
	0xFFD8FFB4,	// 14,94
	0xFFC4FFB4,	// 15,95
	// row 6 (R)
	0xFFB4FFB4,	// 0, 96
	0xFFB4FFC4,	// 1, 97
	0xFFB4FFD8,	// 2, 98
	0xFFB4FFE8,	// 3, 99
	0xFFB4FFFF,	// 4, 100
	0xFFB4E8FF,	// 5, 101
	0xFFB4D8FF,	// 6, 102
	0xFFB4C4FF,	// 7, 103
	0xFF000070,	// 8, 104
	0xFF200070,	// 9, 105
	0xFF380070,	// 10,106
	0xFF540070,	// 11,107
	0xFF700070,	// 12,108
	0xFF700054,	// 13,109
	0xFF700038,	// 14,110
	0xFF700020,	// 15,111
	// row 7 (R)
	0xFF700000,	// 0, 112
	0xFF702000,	// 1, 113
	0xFF703800,	// 2, 114
	0xFF705400,	// 3, 115
	0xFF707000,	// 4, 116
	0xFF547000,	// 5, 117
	0xFF387000,	// 6, 118
	0xFF207000,	// 7, 119
	0xFF007000,	// 8, 120
	0xFF007020,	// 9, 121
	0xFF007038,	// 10,122
	0xFF007054,	// 11,123
	0xFF007070,	// 12,124
	0xFF005470,	// 13,125
	0xFF003870,	// 14,126
	0xFF002070,	// 15,127
	// row 8 (R)
	0xFF383870,	// 0, 128
	0xFF443870,	// 1, 129
	0xFF543870,	// 2, 130
	0xFF603870,	// 3, 131
	0xFF703870,	// 4, 132
	0xFF703860,	// 5, 133
	0xFF703854,	// 6, 134
	0xFF703844,	// 7, 135
	0xFF703838,	// 8, 136
	0xFF704438,	// 9, 137
	0xFF705438,	// 10,138
	0xFF706038,	// 11,139
	0xFF707038,	// 12,140
	0xFF607038,	// 13,141
	0xFF547038,	// 14,142
	0xFF447038,	// 15,143
	// row 9 (R)
	0xFF387038,	// 0, 144
	0xFF387044,	// 1, 145
	0xFF387054,	// 2, 145
	0xFF387060,	// 3, 147
	0xFF387070,	// 4, 148
	0xFF386070,	// 5, 149
	0xFF385470,	// 6, 150
	0xFF384470,	// 7, 151
	0xFF505070,	// 8, 152
	0xFF585070,	// 9, 153
	0xFF605070,	// 10,154
	0xFF685070,	// 11,155
	0xFF705070,	// 12,156
	0xFF705068,	// 13,157
	0xFF705060,	// 14,158
	0xFF705058,	// 15,159
	// row 10 (R)
	0xFF705050,	// 0, 160
	0xFF705850,	// 1, 161
	0xFF706050,	// 2, 162
	0xFF706850,	// 3, 163
	0xFF707050,	// 4, 164
	0xFF687050,	// 5, 165
	0xFF607050,	// 6, 166
	0xFF587050,	// 7, 167
	0xFF507050,	// 8, 168
	0xFF507058,	// 9, 169
	0xFF507060,	// 10,170
	0xFF507068,	// 11,171
	0xFF507070,	// 12,172
	0xFF506870,	// 13,173
	0xFF506070,	// 14,174
	0xFF505870,	// 15,175
	// row 11 (R)
	0xFF000040,	// 0, 176
	0xFF100040,	// 1, 177
	0xFF200040,	// 2, 178
	0xFF300040,	// 3, 179
	0xFF400040,	// 4, 180
	0xFF400030,	// 5, 181
	0xFF400020,	// 6, 182
	0xFF400010,	// 7, 183
	0xFF400000,	// 8, 184
	0xFF401000,	// 9, 185
	0xFF402000,	// 10,186
	0xFF403000,	// 11,187
	0xFF404000,	// 12,188
	0xFF304000,	// 13,189
	0xFF204000,	// 14,190
	0xFF104000,	// 15,191
	// row 12 (R)
	0xFF004000,	// 0, 192
	0xFF004010,	// 1, 193
	0xFF004020,	// 2, 194
	0xFF004030,	// 3, 195
	0xFF004040,	// 4, 196
	0xFF003040,	// 5, 197
	0xFF002040,	// 6, 198
	0xFF001040,	// 7, 199
	0xFF202040,	// 8, 200
	0xFF282040,	// 9, 201
	0xFF302040,	// 10,202
	0xFF382040,	// 11,203
	0xFF402040,	// 12,204
	0xFF402038,	// 13,205
	0xFF402030,	// 14,206
	0xFF402028,	// 15,207
	// row 13 (R)
	0xFF402020,	// 0, 208
	0xFF402820,	// 1, 209
	0xFF403020,	// 2, 210
	0xFF403820,	// 3, 211
	0xFF404020,	// 4, 212
	0xFF384020,	// 5, 213
	0xFF304020,	// 6, 214
	0xFF284020,	// 7, 215
	0xFF204020,	// 8, 216
	0xFF204028,	// 9, 217
	0xFF204030,	// 10,218
	0xFF204038,	// 11,219
	0xFF204040,	// 12,220
	0xFF203840,	// 13,221
	0xFF203040,	// 14,222
	0xFF202840,	// 15,223
	// row 14 (R)
	0xFF2C2C40,	// 0, 224
	0xFF302C40,	// 1, 225
	0xFF342C40,	// 2, 226
	0xFF3C2C40,	// 3, 227
	0xFF402C40,	// 4, 228
	0xFF402C3C,	// 5, 229
	0xFF402C34,	// 6, 230
	0xFF402C30,	// 7, 231
	0xFF402C2C,	// 8, 232
	0xFF40302C,	// 9, 233
	0xFF40342C,	// 10,234
	0xFF403C2C,	// 11,235
	0xFF40402C,	// 12,236
	0xFF3C402C,	// 13,237
	0xFF34402C,	// 14,238
	0xFF30402C,	// 15,239
	// row 15 (R)
	0xFF2C402C,	// 0, 240
	0xFF2C4030,	// 1, 241
	0xFF2C4034,	// 2, 242
	0xFF2C403C,	// 3, 243
	0xFF2C4040,	// 4, 244
	0xFF2C3C40,	// 5, 245
	0xFF2C3440,	// 6, 246
	0xFF000000,	// 7, 247
	0xFF000000,	// 8, 248 - black
	0xFF000000,	// 9, 249 - black
	0xFF000000,	// 10,250 - black
	0xFF000000,	// 11,251 - black
	0xFF000000,	// 12,252 - black
	0xFF000000,	// 13,253 - black
	0xFF000000,	// 14,254 - black
	0xFF000000,	// 15,255 - black
};

#endif	// LCD_COLOR_FORMAT

#else	// RGBCOLOR_AS_NUMBER==1

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP

static const RGBColor lcd1_1bpp_palette[2] = 	// 1bpp palette for LCD
{
	{ 0x00, 0x00, 0x00 },	// 0  - black
	{ 0xFF, 0xFF, 0xFF },	// 1  - white
};		// 6 bytes

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP

static const RGBColor lcd1_4bpp_palette[16] = 	// 4bpp palette for LCD
{
	// Standard VGA 16 color palette
	{ 0x00, 0x00, 0x00 },	// 0  - black
	{ 0x00, 0x00, 0xAA },	// 1  - blue
	{ 0x00, 0xAA, 0x00 },	// 2  - green
	{ 0x00, 0xAA, 0xAA },	// 3  - cyan
	{ 0xAA, 0x00, 0x00 },	// 4  - red
	{ 0xAA, 0x00, 0xAA },	// 5  - magenta
	{ 0xAA, 0x55, 0x00 },	// 6  - brown
	{ 0xAA, 0xAA, 0xAA },	// 7  - gray
	{ 0x55, 0x55, 0x55 },	// 8  - dark gray
	{ 0x55, 0x55, 0xFF },	// 9  - bright blue
	{ 0x55, 0xFF, 0x55 },	// 10 - bright green
	{ 0x55, 0xFF, 0xFF },	// 11 - bright cyan
	{ 0xFF, 0x55, 0x55 },	// 12 - bright red
	{ 0xFF, 0x55, 0xFF },	// 13 - bright magenta
	{ 0xFF, 0xFF, 0x55 },	// 14 - yellow
	{ 0xFF, 0xFF, 0xFF },	// 15 - white
};		// 48 bytes

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP

static const RGBColor lcd1_8bpp_palette[256] = 	// 8bpp palette for LCD
{
	// Standard VGA 256 color palette
	// row 0 (R)
	{ 0x00, 0x00, 0x00 },	// 0  - black
	{ 0x00, 0x00, 0xAA },	// 1  - blue
	{ 0x00, 0xAA, 0x00 },	// 2  - green
	{ 0x00, 0xAA, 0xAA },	// 3  - cyan
	{ 0xAA, 0x00, 0x00 },	// 4  - red
	{ 0xAA, 0x00, 0xAA },	// 5  - magenta
	{ 0xAA, 0x55, 0x00 },	// 6  - brown
	{ 0xAA, 0xAA, 0xAA },	// 7  - gray
	{ 0x55, 0x55, 0x55 },	// 8  - dark gray
	{ 0x55, 0x55, 0xFF },	// 9  - bright blue
	{ 0x55, 0xFF, 0x55 },	// 10 - bright green
	{ 0x55, 0xFF, 0xFF },	// 11 - bright cyan
	{ 0xFF, 0x55, 0x55 },	// 12 - bright red
	{ 0xFF, 0x55, 0xFF },	// 13 - bright magenta
	{ 0xFF, 0xFF, 0x55 },	// 14 - yellow
	{ 0xFF, 0xFF, 0xFF },	// 15 - white
	// row 1, gray
	{ 0x00, 0x00, 0x00 },	// 0, 16
	{ 0x14, 0x14, 0x14 },	// 1, 17
	{ 0x20, 0x20, 0x20 },	// 2, 18
	{ 0x30, 0x30, 0x30 },	// 3, 19
	{ 0x38, 0x38, 0x38 },	// 4, 20
	{ 0x44, 0x44, 0x44 },	// 5, 21
	{ 0x50, 0x50, 0x50 },	// 6, 22
	{ 0x60, 0x60, 0x60 },	// 7, 23
	{ 0x70, 0x70, 0x70 },	// 8, 24
	{ 0x80, 0x80, 0x80 },	// 9, 25
	{ 0x90, 0x90, 0x90 },	// 10,26
	{ 0xA0, 0xA0, 0xA0 },	// 11,27
	{ 0xB4, 0xB4, 0xB4 },	// 12,28
	{ 0xC8, 0xC8, 0xC8 },	// 13,29
	{ 0xE0, 0xE0, 0xE0 },	// 14,30
	{ 0xFF, 0xFF, 0xFF },	// 15,31
	// row 2 (R)
	{ 0x00, 0x00, 0xFF },	// 0, 32
	{ 0x40, 0x00, 0xFF },	// 1, 33
	{ 0x80, 0x00, 0xFF },	// 2, 34
	{ 0xC0, 0x00, 0xFF },	// 3, 35
	{ 0xFF, 0x00, 0xFF },	// 4, 36
	{ 0xFF, 0x00, 0xC0 },	// 5, 37
	{ 0xFF, 0x00, 0x80 },	// 6, 38
	{ 0xFF, 0x00, 0x40 },	// 7, 39
	{ 0xFF, 0x00, 0x00 },	// 8, 40
	{ 0xFF, 0x40, 0x00 },	// 9, 41
	{ 0xFF, 0x80, 0x00 },	// 10,42
	{ 0xFF, 0xC0, 0x00 },	// 11,43
	{ 0xFF, 0xFF, 0x00 },	// 12,44
	{ 0xC0, 0xFF, 0x00 },	// 13,45
	{ 0x80, 0xFF, 0x00 },	// 14,46
	{ 0x40, 0xFF, 0x00 },	// 15,47
	// row 3 (R)
	{ 0x00, 0xFF, 0x00 },	// 0, 48
	{ 0x00, 0xFF, 0x40 },	// 1, 49
	{ 0x00, 0xFF, 0x80 },	// 2, 50
	{ 0x00, 0xFF, 0xC0 },	// 3, 51
	{ 0x00, 0xFF, 0xFF },	// 4, 52
	{ 0x00, 0xC0, 0xFF },	// 5, 53
	{ 0x00, 0x80, 0xFF },	// 6, 54
	{ 0x00, 0x40, 0xFF },	// 7, 55
	{ 0x80, 0x80, 0xFF },	// 8, 56
	{ 0xA0, 0x80, 0xFF },	// 9, 57
	{ 0xC0, 0x80, 0xFF },	// 10,58
	{ 0xE0, 0x80, 0xFF },	// 11,59
	{ 0xFF, 0x80, 0xFF },	// 12,60
	{ 0xFF, 0x80, 0xE0 },	// 13,61
	{ 0xFF, 0x80, 0xC0 },	// 14,62
	{ 0xFF, 0x80, 0xA0 },	// 15,63
	// row 4 (R)
	{ 0xFF, 0x80, 0x80 },	// 0, 64
	{ 0xFF, 0xA0, 0x80 },	// 1, 65
	{ 0xFF, 0xC0, 0x80 },	// 2, 66
	{ 0xFF, 0xE0, 0x80 },	// 3, 67
	{ 0xFF, 0xFF, 0x80 },	// 4, 68
	{ 0xE0, 0xFF, 0x80 },	// 5, 69
	{ 0xC0, 0xFF, 0x80 },	// 6, 70
	{ 0xA0, 0xFF, 0x80 },	// 7, 71
	{ 0x80, 0xFF, 0x80 },	// 8, 72
	{ 0x80, 0xFF, 0xA0 },	// 9, 73
	{ 0x80, 0xFF, 0xC0 },	// 10,74
	{ 0x80, 0xFF, 0xE0 },	// 11,75
	{ 0x80, 0xFF, 0xFF },	// 12,76
	{ 0x80, 0xE0, 0xFF },	// 13,77
	{ 0x80, 0xC0, 0xFF },	// 14,78
	{ 0x80, 0xA0, 0xFF },	// 15,79
	// row 5 (R)
	{ 0xB4, 0xB4, 0xFF },	// 0, 80
	{ 0xC4, 0xB4, 0xFF },	// 1, 81
	{ 0xD8, 0xB4, 0xFF },	// 2, 82
	{ 0xE8, 0xB4, 0xFF },	// 3, 83
	{ 0xFF, 0xB4, 0xFF },	// 4, 84
	{ 0xFF, 0xB4, 0xE8 },	// 5, 85
	{ 0xFF, 0xB4, 0xD8 },	// 6, 86
	{ 0xFF, 0xB4, 0xC4 },	// 7, 87
	{ 0xFF, 0xB4, 0xB4 },	// 8, 88
	{ 0xFF, 0xC4, 0xB4 },	// 9, 89
	{ 0xFF, 0xD8, 0xB4 },	// 10,90
	{ 0xFF, 0xE8, 0xB4 },	// 11,91
	{ 0xFF, 0xFF, 0xB4 },	// 12,92
	{ 0xE8, 0xFF, 0xB4 },	// 13,93
	{ 0xD8, 0xFF, 0xB4 },	// 14,94
	{ 0xC4, 0xFF, 0xB4 },	// 15,95
	// row 6 (R)
	{ 0xB4, 0xFF, 0xB4 },	// 0, 96
	{ 0xB4, 0xFF, 0xC4 },	// 1, 97
	{ 0xB4, 0xFF, 0xD8 },	// 2, 98
	{ 0xB4, 0xFF, 0xE8 },	// 3, 99
	{ 0xB4, 0xFF, 0xFF },	// 4, 100
	{ 0xB4, 0xE8, 0xFF },	// 5, 101
	{ 0xB4, 0xD8, 0xFF },	// 6, 102
	{ 0xB4, 0xC4, 0xFF },	// 7, 103
	{ 0x00, 0x00, 0x70 },	// 8, 104
	{ 0x20, 0x00, 0x70 },	// 9, 105
	{ 0x38, 0x00, 0x70 },	// 10,106
	{ 0x54, 0x00, 0x70 },	// 11,107
	{ 0x70, 0x00, 0x70 },	// 12,108
	{ 0x70, 0x00, 0x54 },	// 13,109
	{ 0x70, 0x00, 0x38 },	// 14,110
	{ 0x70, 0x00, 0x20 },	// 15,111
	// row 7 (R)
	{ 0x70, 0x00, 0x00 },	// 0, 112
	{ 0x70, 0x20, 0x00 },	// 1, 113
	{ 0x70, 0x38, 0x00 },	// 2, 114
	{ 0x70, 0x54, 0x00 },	// 3, 115
	{ 0x70, 0x70, 0x00 },	// 4, 116
	{ 0x54, 0x70, 0x00 },	// 5, 117
	{ 0x38, 0x70, 0x00 },	// 6, 118
	{ 0x20, 0x70, 0x00 },	// 7, 119
	{ 0x00, 0x70, 0x00 },	// 8, 120
	{ 0x00, 0x70, 0x20 },	// 9, 121
	{ 0x00, 0x70, 0x38 },	// 10,122
	{ 0x00, 0x70, 0x54 },	// 11,123
	{ 0x00, 0x70, 0x70 },	// 12,124
	{ 0x00, 0x54, 0x70 },	// 13,125
	{ 0x00, 0x38, 0x70 },	// 14,126
	{ 0x00, 0x20, 0x70 },	// 15,127
	// row 8 (R)
	{ 0x38, 0x38, 0x70 },	// 0, 128
	{ 0x44, 0x38, 0x70 },	// 1, 129
	{ 0x54, 0x38, 0x70 },	// 2, 130
	{ 0x60, 0x38, 0x70 },	// 3, 131
	{ 0x70, 0x38, 0x70 },	// 4, 132
	{ 0x70, 0x38, 0x60 },	// 5, 133
	{ 0x70, 0x38, 0x54 },	// 6, 134
	{ 0x70, 0x38, 0x44 },	// 7, 135
	{ 0x70, 0x38, 0x38 },	// 8, 136
	{ 0x70, 0x44, 0x38 },	// 9, 137
	{ 0x70, 0x54, 0x38 },	// 10,138
	{ 0x70, 0x60, 0x38 },	// 11,139
	{ 0x70, 0x70, 0x38 },	// 12,140
	{ 0x60, 0x70, 0x38 },	// 13,141
	{ 0x54, 0x70, 0x38 },	// 14,142
	{ 0x44, 0x70, 0x38 },	// 15,143
	// row 9 (R)
	{ 0x38, 0x70, 0x38 },	// 0, 144
	{ 0x38, 0x70, 0x44 },	// 1, 145
	{ 0x38, 0x70, 0x54 },	// 2, 145
	{ 0x38, 0x70, 0x60 },	// 3, 147
	{ 0x38, 0x70, 0x70 },	// 4, 148
	{ 0x38, 0x60, 0x70 },	// 5, 149
	{ 0x38, 0x54, 0x70 },	// 6, 150
	{ 0x38, 0x44, 0x70 },	// 7, 151
	{ 0x50, 0x50, 0x70 },	// 8, 152
	{ 0x58, 0x50, 0x70 },	// 9, 153
	{ 0x60, 0x50, 0x70 },	// 10,154
	{ 0x68, 0x50, 0x70 },	// 11,155
	{ 0x70, 0x50, 0x70 },	// 12,156
	{ 0x70, 0x50, 0x68 },	// 13,157
	{ 0x70, 0x50, 0x60 },	// 14,158
	{ 0x70, 0x50, 0x58 },	// 15,159
	// row 10 (R)
	{ 0x70, 0x50, 0x50 },	// 0, 160
	{ 0x70, 0x58, 0x50 },	// 1, 161
	{ 0x70, 0x60, 0x50 },	// 2, 162
	{ 0x70, 0x68, 0x50 },	// 3, 163
	{ 0x70, 0x70, 0x50 },	// 4, 164
	{ 0x68, 0x70, 0x50 },	// 5, 165
	{ 0x60, 0x70, 0x50 },	// 6, 166
	{ 0x58, 0x70, 0x50 },	// 7, 167
	{ 0x50, 0x70, 0x50 },	// 8, 168
	{ 0x50, 0x70, 0x58 },	// 9, 169
	{ 0x50, 0x70, 0x60 },	// 10,170
	{ 0x50, 0x70, 0x68 },	// 11,171
	{ 0x50, 0x70, 0x70 },	// 12,172
	{ 0x50, 0x68, 0x70 },	// 13,173
	{ 0x50, 0x60, 0x70 },	// 14,174
	{ 0x50, 0x58, 0x70 },	// 15,175
	// row 11 (R)
	{ 0x00, 0x00, 0x40 },	// 0, 176
	{ 0x10, 0x00, 0x40 },	// 1, 177
	{ 0x20, 0x00, 0x40 },	// 2, 178
	{ 0x30, 0x00, 0x40 },	// 3, 179
	{ 0x40, 0x00, 0x40 },	// 4, 180
	{ 0x40, 0x00, 0x30 },	// 5, 181
	{ 0x40, 0x00, 0x20 },	// 6, 182
	{ 0x40, 0x00, 0x10 },	// 7, 183
	{ 0x40, 0x00, 0x00 },	// 8, 184
	{ 0x40, 0x10, 0x00 },	// 9, 185
	{ 0x40, 0x20, 0x00 },	// 10,186
	{ 0x40, 0x30, 0x00 },	// 11,187
	{ 0x40, 0x40, 0x00 },	// 12,188
	{ 0x30, 0x40, 0x00 },	// 13,189
	{ 0x20, 0x40, 0x00 },	// 14,190
	{ 0x10, 0x40, 0x00 },	// 15,191
	// row 12 (R)
	{ 0x00, 0x40, 0x00 },	// 0, 192
	{ 0x00, 0x40, 0x10 },	// 1, 193
	{ 0x00, 0x40, 0x20 },	// 2, 194
	{ 0x00, 0x40, 0x30 },	// 3, 195
	{ 0x00, 0x40, 0x40 },	// 4, 196
	{ 0x00, 0x30, 0x40 },	// 5, 197
	{ 0x00, 0x20, 0x40 },	// 6, 198
	{ 0x00, 0x10, 0x40 },	// 7, 199
	{ 0x20, 0x20, 0x40 },	// 8, 200
	{ 0x28, 0x20, 0x40 },	// 9, 201
	{ 0x30, 0x20, 0x40 },	// 10,202
	{ 0x38, 0x20, 0x40 },	// 11,203
	{ 0x40, 0x20, 0x40 },	// 12,204
	{ 0x40, 0x20, 0x38 },	// 13,205
	{ 0x40, 0x20, 0x30 },	// 14,206
	{ 0x40, 0x20, 0x28 },	// 15,207
	// row 13 (R)
	{ 0x40, 0x20, 0x20 },	// 0, 208
	{ 0x40, 0x28, 0x20 },	// 1, 209
	{ 0x40, 0x30, 0x20 },	// 2, 210
	{ 0x40, 0x38, 0x20 },	// 3, 211
	{ 0x40, 0x40, 0x20 },	// 4, 212
	{ 0x38, 0x40, 0x20 },	// 5, 213
	{ 0x30, 0x40, 0x20 },	// 6, 214
	{ 0x28, 0x40, 0x20 },	// 7, 215
	{ 0x20, 0x40, 0x20 },	// 8, 216
	{ 0x20, 0x40, 0x28 },	// 9, 217
	{ 0x20, 0x40, 0x30 },	// 10,218
	{ 0x20, 0x40, 0x38 },	// 11,219
	{ 0x20, 0x40, 0x40 },	// 12,220
	{ 0x20, 0x38, 0x40 },	// 13,221
	{ 0x20, 0x30, 0x40 },	// 14,222
	{ 0x20, 0x28, 0x40 },	// 15,223
	// row 14 (R)
	{ 0x2C, 0x2C, 0x40 },	// 0, 224
	{ 0x30, 0x2C, 0x40 },	// 1, 225
	{ 0x34, 0x2C, 0x40 },	// 2, 226
	{ 0x3C, 0x2C, 0x40 },	// 3, 227
	{ 0x40, 0x2C, 0x40 },	// 4, 228
	{ 0x40, 0x2C, 0x3C },	// 5, 229
	{ 0x40, 0x2C, 0x34 },	// 6, 230
	{ 0x40, 0x2C, 0x30 },	// 7, 231
	{ 0x40, 0x2C, 0x2C },	// 8, 232
	{ 0x40, 0x30, 0x2C },	// 9, 233
	{ 0x40, 0x34, 0x2C },	// 10,234
	{ 0x40, 0x3C, 0x2C },	// 11,235
	{ 0x40, 0x40, 0x2C },	// 12,236
	{ 0x3C, 0x40, 0x2C },	// 13,237
	{ 0x34, 0x40, 0x2C },	// 14,238
	{ 0x30, 0x40, 0x2C },	// 15,239
	// row 15 (R)
	{ 0x2C, 0x40, 0x2C },	// 0, 240
	{ 0x2C, 0x40, 0x30 },	// 1, 241
	{ 0x2C, 0x40, 0x34 },	// 2, 242
	{ 0x2C, 0x40, 0x3C },	// 3, 243
	{ 0x2C, 0x40, 0x40 },	// 4, 244
	{ 0x2C, 0x3C, 0x40 },	// 5, 245
	{ 0x2C, 0x34, 0x40 },	// 6, 246
	{ 0x00, 0x00, 0x00 },	// 7, 247
	{ 0x00, 0x00, 0x00 },	// 8, 248 - black
	{ 0x00, 0x00, 0x00 },	// 9, 249 - black
	{ 0x00, 0x00, 0x00 },	// 10,250 - black
	{ 0x00, 0x00, 0x00 },	// 11,251 - black
	{ 0x00, 0x00, 0x00 },	// 12,252 - black
	{ 0x00, 0x00, 0x00 },	// 13,253 - black
	{ 0x00, 0x00, 0x00 },	// 14,254 - black
	{ 0x00, 0x00, 0x00 },	// 15,255 - black
};		// 768 bytes

#endif	// LCD_COLOR_FORMAT

#endif	// RGBCOLOR_AS_NUMBER==1

static uint8_t lcd1_buffer[LCD1_BUFFER_SZ];
static st7735_handle lcd1_handle = 0;

// general status
volatile struct op_status op_stat = { 0, 0 };

// current form data
uint8_t form_data[FORM_DATA_MAX_SZ];

AppState appState;

form_def form_test_text_freesans_hintsmono = { &formTestTextFreeSansHintsMono_reset, &formTestTextFreeSansHintsMono_onDraw, &formTestTextFreeSansHintsMono_onButton };
form_def form_test_text_freesans_hintsauto = { &formTestTextFreeSansHintsAuto_reset, &formTestTextFreeSansHintsAuto_onDraw, &formTestTextFreeSansHintsAuto_onButton };
form_def form_test_text_freeserif_hintsmono = { &formTestTextFreeSerifHintsMono_reset, &formTestTextFreeSerifHintsMono_onDraw, &formTestTextFreeSerifHintsMono_onButton };
form_def form_test_text_freeserif_hintsauto = { &formTestTextFreeSerifHintsAuto_reset, &formTestTextFreeSerifHintsAuto_onDraw, &formTestTextFreeSerifHintsAuto_onButton };
form_def form_test_text_notosans_hintsmono = { &formTestTextNotoSansHintsMono_reset, &formTestTextNotoSansHintsMono_onDraw, &formTestTextNotoSansHintsMono_onButton };
form_def form_test_text_notosans_hintsauto = { &formTestTextNotoSansHintsAuto_reset, &formTestTextNotoSansHintsAuto_onDraw, &formTestTextNotoSansHintsAuto_onButton };
form_def form_test_text_notoserif_hintsmono = { &formTestTextNotoSerifHintsMono_reset, &formTestTextNotoSerifHintsMono_onDraw, &formTestTextNotoSerifHintsMono_onButton };
form_def form_test_text_notoserif_hintsauto = { &formTestTextNotoSerifHintsAuto_reset, &formTestTextNotoSerifHintsAuto_onDraw, &formTestTextNotoSerifHintsAuto_onButton };
form_def form_test_image_wb = { &formTestImageWB_reset, &formTestImageWB_onDraw, &formTestImageWB_onButton };
form_def form_bench = { &formBench_reset, &formBench_onDraw, &formBench_onButton };

form_def* curr_form;


void lcd1_clearBuffer();

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP

static int8_t lcd1_setPixel_fb_1bpp(int16_t x, int16_t y, const uint8_t* raw_color);
static int8_t lcd1_syncBuffer_1bpp();

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP

static int8_t lcd1_setPixel_fb_4bpp(int16_t x, int16_t y, const uint8_t* raw_color);
static int8_t lcd1_syncBuffer_4bpp();

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP

static int8_t lcd1_setPixel_fb_8bpp(int16_t x, int16_t y, const uint8_t* raw_color);
static int8_t lcd1_syncBuffer_8bpp();

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565

static int8_t lcd1_setPixel_fb_16bpp(int16_t x, int16_t y, const uint8_t* raw_color);
static int8_t lcd1_syncBuffer_16bpp();

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888

static int8_t lcd1_setPixel_fb_24bpp(int16_t x, int16_t y, const uint8_t* raw_color);
static int8_t lcd1_syncBuffer_24bpp();

#endif

int main(void)
{
	volatile uint32_t tmp;

	// Enable the FLASH instruction cache.
	FLASH->ACR |= (1 << FLASH_ACR_ICEN_Pos) |
	// Enable the FLASH data cache.
				  (1 << FLASH_ACR_DCEN_Pos) |
	// Enable the FLASH prefetch buffer.
				  (1 << FLASH_ACR_PRFTEN_Pos);

	// 1. Initialize system clock. Set maximum possible frequencies - 180MHz
	// 1.1. Enable system configuration controller clock
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN_Msk;
	// delay, read back
	tmp = (RCC->APB2ENR & RCC_APB2ENR_SYSCFGEN_Msk) == RCC_APB2ENR_SYSCFGEN_Msk;
	UNUSED(tmp);
	// 1.2. Enable power interface clock
	RCC->APB1ENR |= RCC_APB1ENR_PWREN_Msk;
	// delay, read back
	tmp = (RCC->APB1ENR & RCC_APB1ENR_PWREN_Msk) == RCC_APB1ENR_PWREN_Msk;
	UNUSED(tmp);
	// 1.3. Regulator voltage scaling output selection: Scale 1 mode (default value at reset): the maximum value of fHCLK = 168 MHz.
	PWR->CR |= (0x03 << PWR_CR_VOS_Pos);
	// 1.4. HSE clock enable
	RCC->CR |= (1 << RCC_CR_HSEON_Pos);		// (1 << RCC_CR_HSEBYP_Pos)
	// Wait until HSE oscillator is stable
	while ((RCC->CR & RCC_CR_HSERDY_Msk) != RCC_CR_HSERDY_Msk)
		;

	// 1.5. Configure PLL
	// Disable PLL
	RCC->CR &= ~(1 << RCC_CR_PLLON_Pos);
	// Wait until PLL is unlocked.
	while ((RCC->CR & RCC_CR_PLLRDY_Msk) == RCC_CR_PLLRDY_Msk)
		;

	// Configure the main PLL clock source, multiplication and division factors.
	//RCC->PLLCFGR = 0;
	RCC->PLLCFGR =  (0x01 << RCC_PLLCFGR_PLLSRC_HSE_Pos) |	// HSE oscillator clock selected as PLL and PLLI2S clock entry
					(0x04 << RCC_PLLCFGR_PLLM_Pos) |		// PLLM: Division factor for the main PLL (PLL) input clock
					(180 << RCC_PLLCFGR_PLLN_Pos) |			// PLLN: Main PLL (PLL) multiplication factor for VCO
					(0x00 << RCC_PLLCFGR_PLLP_Pos) |		// Main PLL (PLL) division factor for main system clock: 0 => PLLP=2
					(0x02 << RCC_PLLCFGR_PLLQ_Pos) |		// Main PLL (PLL) division factor for USB OTG FS, SDIOclocks
					(0x02 << RCC_PLLCFGR_PLLR_Pos);			// PLLR: Main PLL division factor for I2Ss, SAIs, SYSTEM and SPDIF-Rx clocks
	// Enable PLL
	RCC->CR |= (1 << RCC_CR_PLLON_Pos);
	// Wait until PLL is locked.
	while ((RCC->CR & RCC_CR_PLLRDY_Msk) != RCC_CR_PLLRDY_Msk)
		;

	// 1.6. Enable Over-Drive mode (to achieve 180MHz)
	PWR->CR |= (1 << PWR_CR_ODEN_Pos);
	// Wait ready flag
	while ((PWR->CSR & PWR_CSR_ODRDY_Msk) != PWR_CSR_ODRDY_Msk)
		;
	// Enable the Over-drive switch  (to achieve 180MHz)
	PWR->CR |= (1 << PWR_CR_ODSWEN_Pos);
	// Wait ready flag
	while ((PWR->CSR & PWR_CSR_ODSWRDY_Msk) != PWR_CSR_ODSWRDY_Msk)
		;
	
	// 1.7. Initializes the CPU, AHB and APB busses clocks
	// Set ratio of the CPU clock period to the Flash memory access time
	FLASH->ACR &= ~FLASH_ACR_LATENCY_Msk;
	FLASH->ACR |= FLASH_ACR_LATENCY_5WS;		// 5WS (6 CPU cycles)
	// Set the highest APBx dividers in order to ensure that we do not go through a non-spec phase whatever we decrease or increase HCLK.
	RCC->CFGR = (0x00 << RCC_CFGR_HPRE_Pos) |		// Set AHP Prescaler to 1 (system clock not divided)
				(0x07 << RCC_CFGR_PPRE1_Pos) |		// APB Low speed prescaler (APB1): AHB clock divided by 16
				(0x07 << RCC_CFGR_PPRE2_Pos) |		// APB high-speed prescaler (APB2): AHB clock divided by 16
				(0x02 << RCC_CFGR_SW_Pos);			// PLL_P selected as system clock
	// Wait ready flag
	while ((RCC->CFGR & (0x02 << RCC_CFGR_SWS_Pos)) != (0x02 << RCC_CFGR_SWS_Pos))
		;
	// => SYSCLK = 180 MHz
	// => AHB clock, HCLK = 180 MHz

	RCC->CFGR &= ~(RCC_CFGR_PPRE1_Msk | RCC_CFGR_PPRE2_Msk);
	// Set APB1 & APB2 prescaler
	RCC->CFGR |= (0x05 << RCC_CFGR_PPRE1_Pos) |		// APB Low speed prescaler (APB1): AHB clock divided by 4
				 (0x04 << RCC_CFGR_PPRE2_Pos);		// APB high-speed prescaler (APB2): AHB clock divided by 2

	// => APB1 clock, PCLK1 = 45 MHz
	// => APB2 clock, PCLK2 = 90 MHz

	// 1.8. Update system core clock
	SystemCoreClockUpdate();

	// 2. SysTick initialize
	SysTick_Config(SystemCoreClock / 1000);

	// 3. Enable peripheral clock (APB1, APB2, AHB1ENR)
	// Here disabled "System Configuration Controller Clock", "Power Interface Clock"
	RCC->APB2ENR = RCC_APB2ENR_SPI1EN_Msk | 		// SPI1 clock enable
				   RCC_APB2ENR_SYSCFGEN_Msk;		// System configuration controller clock enabled
	RCC->APB1ENR = RCC_APB1ENR_TIM2EN_Msk |			// TIM2 clock enable
				   RCC_APB1ENR_TIM3EN_Msk;			// TIM3 clock enable
//				   RCC_APB1ENR_I2C1EN_Msk;			// I2C1 clock enable
	RCC->AHB1ENR = RCC_AHB1ENR_GPIOAEN_Msk |		// Enable clock for GPIO Port A
				   RCC_AHB1ENR_GPIOBEN_Msk |		// Enable clock for GPIO Port B
				   RCC_AHB1ENR_GPIOCEN_Msk;			// Enable clock for GPIO Port C
	// pause, to take effect
	tmp = (RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN_Msk) == RCC_AHB1ENR_GPIOAEN_Msk;
	UNUSED(tmp);

	// 4. Init GPIO for LED
	// Setup output port PA0 (LED1)
	GPIOA->MODER &= ~GPIO_MODER_MODER0_Msk;					// PA0: Clear MODE bits
	GPIOA->MODER |= (0x01 << GPIO_MODER_MODER0_Pos);		// PA0: 01b - General purpose output mode
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED0_Msk;			// PA0: Clear OSPEEDR bits, 00b - Low speed
	//GPIOA->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED0_Pos);	// PA0: 01b - Medium speed
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT0_Msk;					// PA0: 00b - Output push-pull (reset state)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0_Msk;					// PA0: Clear PUPDR bits, 00b - No pull-up, pull-down

	// Setup output port PA1 (LED2)
	GPIOA->MODER &= ~GPIO_MODER_MODER1_Msk;					// PA1: Clear MODE bits
	GPIOA->MODER |= (0x01 << GPIO_MODER_MODER1_Pos);		// PA1: 01b - General purpose output mode
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED1_Msk;			// PA1: Clear OSPEEDR bits, 00b - Low speed
	//GPIOA->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED1_Pos);	// PA1: 01b - Medium speed
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT1_Msk;					// PA1: 00b - Output push-pull (reset state)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD1_Msk;					// PA1: Clear PUPDR bits, 00b - No pull-up, pull-down

	// Turn ON LED1 (PA0)
	GPIOA->BSRR |= GPIO_BSRR_BS0_Msk;

#if 0
	// Test SysTick, LED1, LED2
	while (1)
	{
		GPIOA->BSRR = 1 << GPIO_BSRR_BS0_Pos;				// Turn on LED1
		GPIOA->BSRR = 1 << (16 + GPIO_BSRR_BS1_Pos);		// Turn off LED2
		SysTick_Delay_ms(1000);
		GPIOA->BSRR = 1 << (16 + GPIO_BSRR_BS0_Pos);		// Turn off LED1
		GPIOA->BSRR = 1 << GPIO_BSRR_BS1_Pos;				// Turn on LED2
		SysTick_Delay_ms(1000);
	}
#endif

	// 5. Setup data bus for 1.78' LCD ST7735

	// LCD connection: ST7735 GMT177-01
	//  1. NC
	//  2. VSS      - GND
	//  3. LED_K    - GND
	//  4. LED_A    - PB10 via MOSFET (inverted)
	//  5. VSS      - GND
	//  6. RESET    - PA8 or pull-up resistor (selected by jumper JP3)
	//  7. D/CX     - PC7
	//  8. SDA      - PA7 (MOSI1)
	//  9. SCK      - PA5 (SCK1)
	// 10. VDDA     - VCC
	// 11. VDDIO    - VCC
	// 12. !CS!     - PB6
	// 13. VSS      - GND
	// 14. NC

	// ST7735 use bidirectional SPI data mode.
	// 5.1. Init SPI1 GPIO
	// PA5 - SPI1_SCK
	GPIOA->MODER &= ~GPIO_MODER_MODER5_Msk;					// PA5: Clear MODE bits
	GPIOA->MODER |= (0x02 << GPIO_MODER_MODER5_Pos);		// PA5: 10b - Alternate function mode
	GPIOA->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED5_Pos);	// PA5: 11b - High speed
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT5_Msk;					// PA5: 00b - Output push-pull (reset state)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD5_Msk;					// PA5: 00b - No pull-up, pull-down
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL5_Msk;					// PA5: Clear AFRL5 bits
	GPIOA->AFR[0] |= (0x05 << GPIO_AFRL_AFSEL5_Pos);		// PA5: Alternate function selection - AF5 (SPI1)
	// PA7 - SPI1_MOSI
	GPIOA->MODER &= ~GPIO_MODER_MODER7_Msk;					// PA7: Clear MODE bits
	GPIOA->MODER |= (0x02 << GPIO_MODER_MODER7_Pos);		// PA7: 10b - Alternate function mode
	GPIOA->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED7_Pos);	// PA7: 11b - High speed
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT7_Msk;					// PA7: 00b - Output push-pull (reset state)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD7_Msk;					// PA7: 00b - No pull-up, pull-down
	GPIOA->AFR[0] &= ~GPIO_AFRL_AFSEL7_Msk;					// PA7: Clear AFRL7 bits
	GPIOA->AFR[0] |= (0x05 << GPIO_AFRL_AFSEL7_Pos);		// PA7: Alternate function selection - AF5 (SPI1)

	// 5.2. SPI settings, for ST7735 max SPI frequency 15 MBits/s
	SPI1->CR1 &= ~SPI_CR1_SPE_Msk;							// Disable SPI1
	// delay, read back
	tmp = (SPI1->CR1 & SPI_CR1_SPE_Msk);
	SPI1->CR1 = (0x01 << SPI_CR1_MSTR_Pos)		|	// Master selection. 1: Master configuration.
				(0x01 << SPI_CR1_BIDIMODE_Pos)	|	// Bidirectional data mode enable. 1: 1-line bidirectional data mode selected
				(0x00 << SPI_CR1_DFF_Pos)		|	// Data frame format. 0: 8-bit data frame format is selected for transmission/reception
				(0x00 << SPI_CR1_CPOL_Pos)		|	// Clock polarity. 0: CK to 0 when idle
				(0x00 << SPI_CR1_CPHA_Pos)		|	// Clock phase. 0: The first clock transition is the first data capture edge
				(0x01 << SPI_CR1_SSM_Pos)		|	// Software slave management. When the SSM bit is set, the NSS pin input is replaced with the value from the SSI bit. 1: Software slave management enabled.
				(0x01 << SPI_CR1_SSI_Pos)		|	// Internal slave select. This bit has an effect only when the SSM bit is set. The value of this bit is forced onto the NSS pin and the IO value of the NSS pin is ignored.
				//(0x00 << SPI_CR1_BR_Pos)		|	// Baud rate control. 010: fPCLK2/2 -> SPI clock = 45 MBits/s
				//(0x01 << SPI_CR1_BR_Pos)		|	// Baud rate control. 010: fPCLK2/4 -> SPI clock = 22,5 MBits/s
				(0x02 << SPI_CR1_BR_Pos)		|	// Baud rate control. 010: fPCLK2/8 -> SPI clock = 11,25 MBits/s
				//(0x03 << SPI_CR1_BR_Pos)		|	// Baud rate control. 011: fPCLK2/16 -> SPI clock = 5,625 MBits/s
				(0x00 << SPI_CR1_LSBFIRST_Pos)	|	// Frame format. 0: MSB transmitted first
				(0x00 << SPI_CR1_CRCEN_Pos);		// Hardware CRC calculation enable. 0: CRC calculation disabled
	SPI1->CR2 = (0x00 << SPI_CR2_SSOE_Pos);			// SS output enable. 0: SS output is disabled in master mode and the cell can work in multimaster configuration.
	SPI1->CR1 |= (0x01 << SPI_CR1_SPE_Pos);			// Enable SPI1

#if 0
	// Test SPI
	while (1)
	{
		// Enable master mode and set output mode for bidirectional data mode
		SPI1->CR1 |= (0x01 << SPI_CR1_MSTR_Pos) |
							 (0x01 << SPI_CR1_BIDIOE_Pos);
		// Enable SPI
		SPI1->CR1 |= (0x01 << SPI_CR1_SPE_Pos);
		// wait previous transmissions
		uint32_t cnt = 10000;
		while ((SPI1->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
			cnt--;
		if (cnt > 0)
		{
			// start transmission
			SPI1->DR = 0x55;
			// wait this transmissions
			cnt = 10000;
			while ((SPI1->SR & SPI_SR_TXE_Msk) == 0 && cnt != 0)
				cnt--;
		}
	}
#endif

	// 5.3. Init GPIO for LCD
	// Setup output port PC7 (D/CX)
	GPIOC->MODER &= ~GPIO_MODER_MODER7_Msk;					// PC7: Clear MODE bits
	GPIOC->MODER |= (0x01 << GPIO_MODER_MODER7_Pos);		// PC7: 01b - General purpose output mode
	GPIOC->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED7_Pos);	// PC7: 11b - High speed
	GPIOC->OTYPER &= ~GPIO_OTYPER_OT7_Msk;					// PC7: 00b - Output push-pull (reset state)
	GPIOC->PUPDR &= ~GPIO_PUPDR_PUPD7_Msk;					// PC7: 00b - No pull-up, pull-down
	// Setup output port PB6 (!CS!)
	GPIOB->MODER &= ~GPIO_MODER_MODER6_Msk;					// PB6: Clear MODE bits
	GPIOB->MODER |= (0x01 << GPIO_MODER_MODER6_Pos);		// PB6: 01b - General purpose output mode
	GPIOB->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED6_Pos);	// PB6: 11b - High speed
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT6_Msk;					// PB6: 00b - Output push-pull (reset state)
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6_Msk;					// PB6: 00b - No pull-up, pull-down
	// Setup output port PA8 (!RESET!)
	GPIOA->MODER &= ~GPIO_MODER_MODER8_Msk;					// PA8: Clear MODE bits
	GPIOA->MODER |= (0x01 << GPIO_MODER_MODER8_Pos);		// PA8: 01b - General purpose output mode
	GPIOA->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED8_Pos);	// PA8: 11b - High speed
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT8_Msk;					// PA8: 00b - Output push-pull (reset state)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD8_Msk;					// PA8: 00b - No pull-up, pull-down

	GPIOB->BSRR = 1 << GPIO_BSRR_BS6_Pos;					// Set !CS! to High (Disable slave devices at initial)
	GPIOA->BSRR = 1 << GPIO_BSRR_BS8_Pos;					// Turn on !LCD_RESET!

#if 1
	// LCD Reset sequence
	GPIOA->BSRR = 1 << GPIO_BSRR_BS8_Pos;			// Turn on !LCD_RESET!
	SysTick_Delay_ms(125);							// delay to wait LCD initialization
	GPIOA->BSRR = 1 << (16 + GPIO_BSRR_BS8_Pos);	// Turn off !LCD_RESET!
	SysTick_Delay_ms(1);							// reset impulse minimum is 10us length
	GPIOA->BSRR = 1 << GPIO_BSRR_BS8_Pos;			// Turn on !LCD_RESET!
	SysTick_Delay_ms(125);							// delay to wait LCD initialization (5-120ms)
#endif

#if 0
	// Test !CS!
	while (1)
	{
		GPIOB->BSRR = 1 << GPIO_BSRR_BS6_Pos;				// Set !CS! to High
		SysTick_Delay_ms(10);
		GPIOB->BSRR = 1 << (16 + GPIO_BSRR_BS6_Pos);		// Set !CS! to Low
		SysTick_Delay_ms(10);
	}
#endif

	// 5.4. Init LCD device module 1.77' LCD display, DPI=116
	lcd1_handle = st7735_init_hw(SPI1, GPIOB, 6, GPIOC, 7, 130, 161, ST7735_IPF_16BIT);
	int8_t ret = lcd1_handle != 0 ? 0 : -1;
	if (ret != 0)
	{
		// LCD init failed, blink LED2 on PA1
		while (1)
		{
			// Turn off PA1 (LED2)
			GPIOA->BSRR |= GPIO_BSRR_BR1_Msk;
			SysTick_Delay_ms(100);
			// Turn on PA1 (LED2)
			GPIOA->BSRR |= GPIO_BSRR_BS1_Msk;
			SysTick_Delay_ms(100);
		}
	}

	// 6. Init Timer TIM2 for LCD brightness control
	GPIOB->MODER &= ~GPIO_MODER_MODER10_Msk;				// PB10: Clear MODE bits
	GPIOB->MODER |= (0x02 << GPIO_MODER_MODER10_Pos);		// PB10: 10b - Alternate function mode
	GPIOB->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED10_Pos);	// PB10: 11b - High speed
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT10_Msk;					// PB10: 00b - Output push-pull (reset state)
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD10_Msk;					// PB10: 00b - No pull-up, pull-down
	GPIOB->AFR[1] &= ~GPIO_AFRH_AFSEL10_Msk;				// PB10: Clear AFRH10 bits
	GPIOB->AFR[1] |= (0x01 << GPIO_AFRH_AFSEL10_Pos);		// PB10: Alternate function selection - AF1 (TIM1, TIM2)
	// TIM2 use APB1 peripheral clock
	// TIM2 internal clock = APB1 clock x 2 = 45MHz x 2 = 90MHz
	TIM2->CR1 = 0;	//(0x01 << TIM_CR1_ARPE_Pos);
	TIM2->CR2 = 0;
	TIM2->SMCR = 0x00 << TIM_SMCR_SMS_Pos;					// Slave mode disabled - if CEN = 1 then the prescaler is clocked directly by the internal clock.
	TIM2->CCMR1 = 0;
	TIM2->CCMR2 = (0x06 << TIM_CCMR2_OC3M_Pos);				// Output compare 3 mode - PWM mode 1
				  //(0x01 << TIM_CCMR2_OC3PE_Pos);			// Output compare 3 preload enable
	TIM2->CCER = (0x01 << TIM_CCER_CC3E_Pos) |				// Capture/Compare 3 output enable
				 (0x01 << TIM_CCER_CC3P_Pos);				// Capture/Compare 3 output polarity, active low
	TIM2->PSC = 0;											// no prescaler
	TIM2->CNT = 0;
	TIM2->ARR = 1000;										// PWM frequency = 90kHz
	TIM2->SR = 0;											// Clear TIM2 status flags
	TIM2->DIER = 0;											// TIM2 interrupts disabled
	TIM2->CCR3 = 50;										// duty cycle, 5%
	TIM2->CR1 |= TIM_CR1_CEN_Msk;							// Start Timer

	// 7. Init GPIO for joystick/buttons
	// BT1 - PA10
	// BT2 - PB5
	// BT3 - PB4
	GPIOA->ODR |= (0x01 << GPIO_ODR_OD10_Pos);		// pull-up
	GPIOA->MODER &= ~GPIO_MODER_MODER10_Msk;		// PA10: Clear MODE bits, input mode

	GPIOB->ODR |= (0x01 << GPIO_ODR_OD5_Pos);		// pull-up
	GPIOB->MODER &= ~GPIO_MODER_MODER5_Msk;			// PB5:  Clear MODE bits, input mode

	GPIOB->ODR |= (0x01 << GPIO_ODR_OD4_Pos);		// pull-up
	GPIOB->MODER &= ~GPIO_MODER_MODER4_Msk;			// PB4:  Clear MODE bits, input mode

	// 7.1. Setting up interrupts for joystick/buttons
	EXTI->IMR = EXTI_IMR_MR10_Msk |					// Interrupt request from Line 10 is not masked
				EXTI_IMR_MR5_Msk |					// Interrupt request from Line 5 is not masked
				EXTI_IMR_MR4_Msk;					// Interrupt request from Line 4 is not masked
//	EXTI->RTSR = EXTI_RTSR_TR10_Msk |				// Rising trigger enabled (for Event and Interrupt) for input line.
//				 EXTI_RTSR_TR5_Msk |
//				 EXTI_RTSR_TR4_Msk;
	EXTI->FTSR = EXTI_FTSR_TR10_Msk |				// Falling trigger enabled (for Event and Interrupt) for input line.
				 EXTI_FTSR_TR5_Msk |
				 EXTI_FTSR_TR4_Msk;
	SYSCFG->EXTICR[0] = 0;
	SYSCFG->EXTICR[1] = (0x01 << SYSCFG_EXTICR2_EXTI4_Pos) |	// EXTI4 - PB4 pin
						(0x01 << SYSCFG_EXTICR2_EXTI5_Pos);		// EXTI5 - PB5 pin
	SYSCFG->EXTICR[2] = (0x00 << SYSCFG_EXTICR3_EXTI10_Pos);	// EXTI10 - PA10 pin
	SYSCFG->EXTICR[3] = 0;

	// 8. Setup delay timer to suppress contact bounce
	//     and for autorepeat key events
	// TIM3 clocked by APB1 prescaled (prescaler=4) => TIM3 internal clock = APB1 clock x 2 = 90MHz
	TIM3->CR1 = 0;
	TIM3->CR2 = 0;
	TIM3->SMCR = 0x00 << TIM_SMCR_SMS_Pos;			// Slave mode disabled - if CEN = ‘1 then the prescaler is clocked directly by the internal clock.
	TIM3->CCMR1 = 0;
	TIM3->CCMR2 = 0;
	TIM3->CCER = 0;
	// For period 100 ms
	//TIM3->PSC = 8999;		// overflow/update frequency 10kHz (90MHz/9000 = 10kHz)
	TIM3->PSC = SystemCoreClock/20000 - 1;
	TIM3->CNT = 0;
	TIM3->ARR = 999;								// 100 ms
	TIM3->EGR |= TIM_EGR_UG_Msk;					// reload prescaler
	TIM3->SR = 0;									// Clear TIM3 status flags
	TIM3->DIER = (1 << TIM_DIER_UIE_Pos);			// Update interrupt enable

	uint8_t data[4] = { 0, 0, 0, 0 };
	ret = st7735_rdcmd(lcd1_handle, ST7735_CMD_RDDID, data, 4);
	if (0 != ret)
		printf("ST7735: failed to read DeviceID!\n");
	appState.devID = ((uint32_t)(data[0]) << 16) |
					 ((uint32_t)(data[1]) << 8) |
					  (uint32_t)(data[2]);

	appState.gfx_color_format = LCD_COLOR_FORMAT;
	appState.st7735_ipf = st7735_getipf(lcd1_handle);
	appState.form_data = form_data;
	appState.changed = APPSTATE_CHANGED_NEEDREDRAW;

	lcd1_clearBuffer();

	// Setup GFX module
#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP
	setFuncSetPixelColor(&lcd1_setPixel_fb_1bpp);
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP
	setFuncSetPixelColor(&lcd1_setPixel_fb_4bpp);
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP
	setFuncSetPixelColor(&lcd1_setPixel_fb_8bpp);
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565
	setFuncSetPixelColor(&lcd1_setPixel_fb_16bpp);
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888
	setFuncSetPixelColor(&lcd1_setPixel_fb_24bpp);
#endif
	setColorFormat(LCD_COLOR_FORMAT);

	curr_form = &form_test_text_freesans_hintsmono;
	curr_form->reset_proc(&appState);

	// Turn off LED1 (PA0)
	GPIOA->BSRR |= GPIO_BSRR_BR0_Msk;
	// Turn on LED2 (PA1)
	GPIOA->BSRR |= GPIO_BSRR_BS1_Msk;

	// Force clear pending register
	EXTI->PR = (0x01 << EXTI_PR_PR10_Pos) |
			   (0x01 << EXTI_PR_PR5_Pos) |
			   (0x01 << EXTI_PR_PR4_Pos);

	NVIC_EnableIRQ(TIM3_IRQn);
	NVIC_EnableIRQ(EXTI4_IRQn);
	NVIC_EnableIRQ(EXTI9_5_IRQn);
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	__enable_irq();

	while (1)
	{
		// Poll buttons/joystick
		if (op_stat.status & STATUS_BTN_RDY_Msk)
		{
			printf("btn=%u\n", op_stat.button_code);
			if ((op_stat.status & STATUS_BTN_PRE_Msk) == STATUS_BTN_PRE_Msk)
			{
				//curr_form->onButtonPress_proc(&appState, op_stat.button_code);
				curr_form->onButton_proc(&appState, op_stat.button_code);
			}
			else if ((op_stat.status & STATUS_BTN_REP_Msk) == STATUS_BTN_REP_Msk)
				curr_form->onButton_proc(&appState, op_stat.button_code);
			//else
			//	curr_form->onButtonRelease_proc(op_stat.button_code);		// not pressed, not repeated => released
			// Clear for next button event
			op_stat.status &= ~STATUS_BTN_RDY_Msk;
		}
		if (appState.changed & APPSTATE_CHANGED_FORM)
		{
			appState.changed &= ~APPSTATE_CHANGED_FORM;
			// запрошено переключение на другую форму
			switch (appState.form_code)
			{
				case FORM_TEXT_TEST_FREESANS_MONO:
					curr_form = &form_test_text_freesans_hintsmono;
					break;
				case FORM_TEXT_TEST_FREESANS_AUTO:
					curr_form = &form_test_text_freesans_hintsauto;
					break;
				case FORM_TEXT_TEST_FREESERIF_MONO:
					curr_form = &form_test_text_freeserif_hintsmono;
					break;
				case FORM_TEXT_TEST_FREESERIF_AUTO:
					curr_form = &form_test_text_freeserif_hintsauto;
					break;
				case FORM_TEXT_TEST_NOTOSANS_MONO:
					curr_form = &form_test_text_notosans_hintsmono;
					break;
				case FORM_TEXT_TEST_NOTOSANS_AUTO:
					curr_form = &form_test_text_notosans_hintsauto;
					break;
				case FORM_TEXT_TEST_NOTOSERIF_MONO:
					curr_form = &form_test_text_notoserif_hintsmono;
					break;
				case FORM_TEXT_TEST_NOTOSERIF_AUTO:
					curr_form = &form_test_text_notoserif_hintsauto;
					break;
				case FORM_WB_IMAGE_TEST:
					curr_form = &form_test_image_wb;
					break;
				case FORM_BENCH:
					curr_form = &form_bench;
					break;
				default:
					curr_form = &form_test_text_freesans_hintsmono;
					break;
			}
			curr_form->reset_proc(&appState);
			appState.changed |= APPSTATE_CHANGED_NEEDREDRAW;
		}
		if (curr_form == &form_bench) {
			appState.changed |= APPSTATE_CHANGED_NEEDREDRAW;
		}
		if (appState.changed & APPSTATE_CHANGED_NEEDREDRAW)
		{
			appState.changed &= ~APPSTATE_CHANGED_NEEDREDRAW;
			lcd1_clearBuffer();
			SysTick_StartPerfCounter();
			curr_form->onDraw_proc(&appState);
			appState.rendTime = SysTick_GetPerfCounter();
			SysTick_StartPerfCounter();
#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP
			lcd1_syncBuffer_1bpp();
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP
			lcd1_syncBuffer_4bpp();
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP
			lcd1_syncBuffer_8bpp();
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565
			lcd1_syncBuffer_16bpp();
#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888
			lcd1_syncBuffer_24bpp();
#endif
			appState.syncTime = SysTick_GetPerfCounter();
			SysTick_StopPerfCounter();
		}
	}
	st7735_free(lcd1_handle);
}

void lcd1_clearBuffer()
{
	memset(lcd1_buffer, 0, sizeof(lcd1_buffer));
}

#if LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX1BPP

static int8_t lcd1_setPixel_fb_1bpp(int16_t x, int16_t y, const uint8_t* raw_color)
{
	// In 1bpp color mode *raw_color is an integer in range 0-1
	if (x < 0 || x > 131 || y < 0 || y > 161)
		return -1;
	register uint8_t xoff = 7 - x % 8;
	//uint8_t* ptr = lcd1_buffer + y*16 + x/8;
	uint8_t* ptr = lcd1_buffer + (y << 4) + (x >> 3);
	if (*raw_color)
		*ptr |= (1 << xoff);		// white
	else
		*ptr &= ~(1 << xoff);		// black
	return 0;
}

static int8_t lcd1_syncBuffer_1bpp()
{
	return st7735_drawBitmap_1bpp(lcd1_handle, 0, 0, 128, 160, lcd1_buffer, sizeof(lcd1_buffer), lcd1_1bpp_palette);
}

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX4BPP

static int8_t lcd1_setPixel_fb_4bpp(int16_t x, int16_t y, const uint8_t* raw_color)
{
	// In 4bpp color mode *raw_color is an integer in range 0-15
	if (x < 0 || x > 131 || y < 0 || y > 161)
		return -1;
	//uint8_t* ptr = lcd1_buffer + y*64 + x/2;
	uint8_t* ptr = lcd1_buffer + (y << 6) + (x >> 1);
	//uint8_t* ptr = lcd1_buffer + y*65 + x/2;
	if (x & 0x01)
		*ptr = (*ptr & 0xF0) | (*raw_color & 0x0F);
	else
		*ptr = (*ptr & 0x0F) | (*raw_color << 4);
	return 0;
}

static int8_t lcd1_syncBuffer_4bpp()
{
	return st7735_drawBitmap_4bpp(lcd1_handle, 0, 0, 128, 160, lcd1_buffer, sizeof(lcd1_buffer), lcd1_4bpp_palette);
}

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_INDEX8BPP

static int8_t lcd1_setPixel_fb_8bpp(int16_t x, int16_t y, const uint8_t* raw_color)
{
	// In 8bpp color mode *raw_color is whole byte
	if (x < 0 || x > 131 || y < 0 || y > 161)
		return -1;
	//uint8_t* ptr = lcd1_buffer + y*128 + x;
	uint8_t* ptr = lcd1_buffer + (y << 7) + x;
	*ptr = *raw_color;
	return 0;
}

static int8_t lcd1_syncBuffer_8bpp()
{
	return st7735_drawBitmap_8bpp(lcd1_handle, 0, 0, 128, 160, lcd1_buffer, sizeof(lcd1_buffer), lcd1_8bpp_palette);
}

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB565

static int8_t lcd1_setPixel_fb_16bpp(int16_t x, int16_t y, const uint8_t* raw_color)
{
	// In 16bpp (R5B6G5) color mode raw_color is two bytes: |RRRRRGGG|GGGBBBBB|
	if (x < 0 || x > 131 || y < 0 || y > 161)
		return -1;
	//uint8_t* ptr = lcd1_buffer + (y*128 + x)*2;
	uint8_t* ptr = lcd1_buffer + (((y << 7) + x) << 1);
	*ptr++ = *raw_color++;
	*ptr = *raw_color;
	return 0;
}

static int8_t lcd1_syncBuffer_16bpp()
{
	return st7735_drawBitmap_16bpp(lcd1_handle, 0, 0, 128, 160, lcd1_buffer, sizeof(lcd1_buffer));
}

#elif LCD_COLOR_FORMAT == COLOR_FORMAT_RGB888

static int8_t lcd1_setPixel_fb_24bpp(int16_t x, int16_t y, const uint8_t* raw_color)
{
	// In 24bpp (R8B8G8) color mode raw_color is three bytes: |RRRRRRRR|GGGGGGGG|BBBBBBBB|
	if (x < 0 || x > 131 || y < 0 || y > 161)
		return -1;
	//uint8_t* ptr = lcd1_buffer + (y*128 + x)*3;
	uint8_t* ptr = lcd1_buffer + (((y << 7) + x)*3);
	*ptr++ = *raw_color++;
	*ptr++ = *raw_color++;
	*ptr = *raw_color;
	return 0;
}

static int8_t lcd1_syncBuffer_24bpp()
{
	return st7735_drawBitmap_24bpp(lcd1_handle, 0, 0, 128, 160, lcd1_buffer, sizeof(lcd1_buffer));
}

#endif	// LCD_COLOR_FORMAT

// for SWV ITM Data Console
int __io_putchar(int ch)
{
	return ITM_SendChar(ch);
}

// EXTI Line 4 interrupts
void EXTI4_IRQHandler(void)
{
	//printf("EXTI4: PR=0x%04lX\n", EXTI->PR);
	static uint8_t code;
	if (EXTI->PR & EXTI_PR_PR4_Msk)
	{
		EXTI->PR |= EXTI_PR_PR4_Msk;		// clear
		// PB4 - BT3
		if (!(op_stat.status & STATUS_BTN_CPT_BT3_Msk))
		{
			op_stat.status |= STATUS_BTN_CPT_BT3_Msk;
			code = 0;
			if ((GPIOA->IDR & GPIO_IDR_ID10_Msk) == 0)				// BT1
				code |= 0x01;
			if ((GPIOB->IDR & GPIO_IDR_ID5_Msk) == 0)				// BT2
				code |= 0x02;
			if ((GPIOB->IDR & GPIO_IDR_ID4_Msk) == 0)				// BT3
				code |= 0x04;
			if (code > 0)
			{
				// PB4 is low
				op_stat.button_code = code;
				op_stat.status |= STATUS_BTN_PRE_Msk;
			}
			// start delay timer to ignore contact bounce
			TIM3->CNT = 0;
			TIM3->ARR = 1999;		// 200 ms
			TIM3->CR1 |= TIM_CR1_CEN_Msk;
		}
	}
}

// EXTI Line[9:5] interrupts
void EXTI9_5_IRQHandler(void)
{
	//printf("EXTI9_5: PR=0x%04lX\n", EXTI->PR);
	static uint8_t code;
	if (EXTI->PR & EXTI_PR_PR5_Msk)
	{
		EXTI->PR |= EXTI_PR_PR5_Msk;		// clear
		// PB5 - BT2
		if (!(op_stat.status & STATUS_BTN_CPT_BT2_Msk))
		{
			op_stat.status |= STATUS_BTN_CPT_BT2_Msk;
			code = 0;
			if ((GPIOA->IDR & GPIO_IDR_ID10_Msk) == 0)				// BT1
				code |= 0x01;
			if ((GPIOB->IDR & GPIO_IDR_ID5_Msk) == 0)				// BT2
				code |= 0x02;
			if ((GPIOB->IDR & GPIO_IDR_ID4_Msk) == 0)				// BT3
				code |= 0x04;
			if (code > 0)
			{
				// PB5 is low
				op_stat.button_code = code;
				op_stat.status |= STATUS_BTN_PRE_Msk;
			}
			// start delay timer to ignore contact bounce
			TIM3->CNT = 0;
			TIM3->ARR = 1999;		// 200 ms
			TIM3->CR1 |= TIM_CR1_CEN_Msk;
		}
	}
}

// EXTI Line[15:10] interrupts
void EXTI15_10_IRQHandler(void)
{
	//printf("EXTI15_10: PR=0x%04lX\n", EXTI->PR);
	static uint8_t code;
	if (EXTI->PR & EXTI_PR_PR10_Msk)
	{
		EXTI->PR |= EXTI_PR_PR10_Msk;		// clear
		// PA10 - BT1
		if (!(op_stat.status & STATUS_BTN_CPT_BT1_Msk))
		{
			op_stat.status |= STATUS_BTN_CPT_BT1_Msk;
			code = 0;
			if ((GPIOA->IDR & GPIO_IDR_ID10_Msk) == 0)				// BT1
				code |= 0x01;
			if ((GPIOB->IDR & GPIO_IDR_ID5_Msk) == 0)				// BT2
				code |= 0x02;
			if ((GPIOB->IDR & GPIO_IDR_ID4_Msk) == 0)				// BT3
				code |= 0x04;
			if (code > 0)
			{
				// PA10 is low
				op_stat.button_code = code;
				op_stat.status |= STATUS_BTN_PRE_Msk;
			}
			// start delay timer to ignore contact bounce
			TIM3->CNT = 0;
			TIM3->ARR = 1999;		// 200 ms
			TIM3->CR1 |= TIM_CR1_CEN_Msk;
		}
	}
}

void TIM3_IRQHandler(void)
{
	if (TIM3->SR & TIM_SR_UIF_Msk)
	{
		TIM3->SR &= ~TIM_SR_UIF_Msk;
		if ((op_stat.status & STATUS_BTN_CPT_Msk) != 0)
		{
			// handle contact bounce timeout
			op_stat.status = (op_stat.status & ~STATUS_BTN_CPT_Msk) | STATUS_BTN_RDY_Msk;
			// Stop TIM3 (both contact bouncer and autorepeat)
			TIM3->CR1 &= ~TIM_CR1_CEN_Msk;
			if (op_stat.status & STATUS_BTN_PRE_Msk)
			{
				// check if any buttons still is pressed
				if ( (GPIOA->IDR & GPIO_IDR_ID10_Msk) == 0 ||
					 (GPIOB->IDR & GPIO_IDR_ID5_Msk) == 0 ||
					 (GPIOB->IDR & GPIO_IDR_ID4_Msk) == 0 )
				{
					// any buttons in low state - pressed
					// Restart timer (TIM3) for autorepeat key events
					TIM3->CNT = 0;
					TIM3->ARR = 9999;
					TIM3->CR1 |= TIM_CR1_CEN_Msk;
				}
				else
				{
					op_stat.status &= ~STATUS_BTN_REP_Msk;
				}
			}
		}
		else
		{
			// handle key event autorepeat
			if (op_stat.status & STATUS_BTN_PRE_Msk)
			{
				// check if any buttons already is not pressed
				if ( (GPIOA->IDR & GPIO_IDR_ID10_Msk) &&
					 (GPIOB->IDR & GPIO_IDR_ID5_Msk) &&
					 (GPIOB->IDR & GPIO_IDR_ID4_Msk) )
					// all buttons in High state - unpressed
					op_stat.status &= ~(STATUS_BTN_PRE_Msk | STATUS_BTN_REP_Msk);
			}
			if (op_stat.status & STATUS_BTN_PRE_Msk)
			{
				// Button pressed, repeat key event
				op_stat.status |= STATUS_BTN_REP_Msk | STATUS_BTN_RDY_Msk;
				// Set repeat delay to 200ms
				TIM3->CNT = 0;
				TIM3->ARR = 1999;
			}
			else
			{
				// Button released
				op_stat.status |= STATUS_BTN_RDY_Msk;
				// Stop autorepeat timer
				TIM3->CR1 &= ~TIM_CR1_CEN_Msk;
			}
		}
	}
}

