// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2020-2021 by Chernov A.A.                               *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef FORM_DEFS_H
#define FORM_DEFS_H

#include "app_state.h"

// display forms
#define FORM_TEXT_TEST_FREESANS_MONO	0		// Font drawing test screen, FreeSans hinting=mono
#define FORM_TEXT_TEST_FREESANS_AUTO	1		// Font drawing test screen, FreeSans hinting=auto
#define FORM_TEXT_TEST_FREESERIF_MONO	2		// Font drawing test screen, FreeSerif hinting=mono
#define FORM_TEXT_TEST_FREESERIF_AUTO	3		// Font drawing test screen, FreeSerif hinting=auto
#define FORM_TEXT_TEST_NOTOSANS_MONO	4		// Font drawing test screen, NotoSans hinting=mono
#define FORM_TEXT_TEST_NOTOSANS_AUTO	5		// Font drawing test screen, NotoSans hinting=auto
#define FORM_TEXT_TEST_NOTOSERIF_MONO	6		// Font drawing test screen, NotoSerif hinting=mono
#define FORM_TEXT_TEST_NOTOSERIF_AUTO	7		// Font drawing test screen, NotoSerif hinting=auto
#define FORM_WB_IMAGE_TEST				8		// Black and white images output test screen
#define FORM_BENCH						9		// Graphics performance test screen
#define FORM_MAX						FORM_BENCH

typedef struct form_def
{
	void (*reset_proc)(AppState*);
	void (*onDraw_proc)(AppState*);
	void (*onButton_proc)(AppState*, uint8_t btn);
} form_def;

#ifdef __cplusplus
extern "C" {
#endif

void form_drawHeader(AppState*);

#ifdef __cplusplus
}
#endif

#endif // FORM_DEFS_H
