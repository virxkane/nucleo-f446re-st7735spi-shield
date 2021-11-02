// -*- coding: utf-8 -*-
/***************************************************************************
 *   Copyright (C) 2019-2020 by Chernov A.A.                               *
 *   valexlin@gmail.com                                                    *
 *                                                                         *
 ***************************************************************************/

#ifndef MCU_DEFS_H
#define MCU_DEFS_H

#if !defined  (HSE_VALUE)
  #define HSE_VALUE    ((uint32_t)8000000U) /*!< Value of the External oscillator in Hz */
#endif /* HSE_VALUE */

#if !defined  (HSI_VALUE)
  #define HSI_VALUE    ((uint32_t)16000000U) /*!< Value of the Internal oscillator in Hz*/
#endif /* HSI_VALUE */

#ifdef STM32F446xx
#undef STM32F446xx
#endif
#define STM32F446xx
#include <stm32f4xx.h>

#endif	// MCU_DEFS_H
