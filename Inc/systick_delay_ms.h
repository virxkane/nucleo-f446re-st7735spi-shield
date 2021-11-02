// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2019-2020 by Чернов А.А.                                *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef SYSTICK_DELAY_MS_H
#define SYSTICK_DELAY_MS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SYSTICK_TIMERS_MAX		8

void SysTick_Delay_ms(uint32_t ms);
void SysTick_StartPerfCounter();
void SysTick_StopPerfCounter();
uint32_t SysTick_GetPerfCounter();

// Start milliseconds timer with number 'no' for ms milliseconds
void SysTick_StartTimer(uint8_t no, uint32_t ms);
// Check milliseconds timer with number 'no' is done.
uint8_t SysTick_CheckTimerDone(uint8_t no);

#ifdef __cplusplus
}
#endif

#endif	// SYSTICK_DELAY_MS_H
