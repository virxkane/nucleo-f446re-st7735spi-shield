/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f4xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
//#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "systick_delay_ms.h"
#include "mcu_defs.h"
#include <stdio.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define UNUSED(x) { (void)x; }

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static unsigned long volatile _delay_counter = 0;
static unsigned long volatile _perf_counter = 0xFFFFFFFF;
static volatile uint32_t _timer_counter[SYSTICK_TIMERS_MAX] = { 0 };
// total 40 bytes
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static inline void resetToSafetyMode()
{
	SystemCoreClockUpdate();

	RCC->AHB1ENR = RCC_AHB1ENR_GPIOAEN_Msk |		// Enable clock for GPIO Port A
				   RCC_AHB1ENR_GPIOBEN_Msk;			// Enable clock for GPIO Port B
	// delay, read back
	volatile uint32_t tmp = (RCC->AHB1ENR & RCC_AHB1ENR_GPIOAEN_Msk) == RCC_AHB1ENR_GPIOAEN_Msk;
	UNUSED(tmp);
	// LED1          - PA0
	// LED2          - PA1
	// !CS!          - PB6

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

	// Setup output port PB6 (!CS!)
	GPIOB->MODER &= ~GPIO_MODER_MODER6_Msk;					// PB6: Clear MODE bits
	GPIOB->MODER |= (0x01 << GPIO_MODER_MODER6_Pos);		// PB6: 01b - General purpose output mode
	GPIOB->OSPEEDR |= (0x03 << GPIO_OSPEEDR_OSPEED6_Pos);	// PB6: 11b - High speed
	GPIOB->OTYPER &= ~GPIO_OTYPER_OT6_Msk;					// PB6: 00b - Output push-pull (reset state)
	GPIOB->PUPDR &= ~GPIO_PUPDR_PUPD6_Msk;					// PB6: 00b - No pull-up, pull-down

	// Set safe mode
	GPIOA->BSRR = 1 << (16 + GPIO_BSRR_BS1_Pos);			// Turn off LED2
	GPIOB->BSRR = 1 << GPIO_BSRR_BS6_Pos;					// Set !CS! to High
}

static inline void LED1_blink_inf(uint16_t period)
{
	// Setup output port PA0 (LED1)
	GPIOA->MODER &= ~GPIO_MODER_MODER0_Msk;					// PA0: Clear MODE bits
	GPIOA->MODER |= (0x01 << GPIO_MODER_MODER0_Pos);		// PA0: 01b - General purpose output mode
	GPIOA->OSPEEDR &= ~GPIO_OSPEEDR_OSPEED0_Msk;			// PA0: Clear OSPEEDR bits, 00b - Low speed
	//GPIOA->OSPEEDR |= (0x01 << GPIO_OSPEEDR_OSPEED0_Pos);	// PA0: 01b - Medium speed
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT0_Msk;					// PA0: 00b - Output push-pull (reset state)
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0_Msk;					// PA0: Clear PUPDR bits, 00b - No pull-up, pull-down
	volatile uint32_t _delay_cnt = (period/2)*(SystemCoreClock/4000);		// us * (SystemCoreClock / 1000000) / 4
	volatile uint32_t _cnt;
	while (1)
	{
		// Turn off LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BR0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		// Turn on LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BS0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
	}
}
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
 * @brief This function handles Non maskable interrupt.
 */
void NMI_Handler(void)
{
	/* USER CODE BEGIN NonMaskableInt_IRQn 0 */

	/* USER CODE END NonMaskableInt_IRQn 0 */
	/* USER CODE BEGIN NonMaskableInt_IRQn 1 */

	/* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
 * @brief This function handles Hard fault interrupt.
 */
void HardFault_Handler(void)
{
	/* USER CODE BEGIN HardFault_IRQn 0 */
#ifdef DEBUG_MSG
	printf("HardFault!\n");
	printf("SCB_CFSR=0x%08lX\n", SCB->CFSR);
	printf("SCB_HFSR=0x%08lX\n", SCB->HFSR);
	if (SCB->CFSR & SCB_CFSR_MMARVALID_Msk)
		printf("SCB_MMFAR=0x%08lX\n", SCB->MMFAR);
	if (SCB->CFSR & SCB_CFSR_BFARVALID_Msk)
		printf("SCB_BFAR=0x%08lX\n", SCB->BFAR);
#endif
	resetToSafetyMode();
	const int period = 2000;
	volatile uint32_t _delay_cnt = (period/2)*(SystemCoreClock/4000);		// us * (SystemCoreClock / 1000000) / 4
	volatile uint32_t _cnt;
	/* USER CODE END HardFault_IRQn 0 */
	while (1)
	{
		/* USER CODE BEGIN W1_HardFault_IRQn 0 */
		// Turn off LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BR0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		// Turn on LED1 (PA3)
		GPIOA->BSRR |= GPIO_BSRR_BS0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		/* USER CODE END W1_HardFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Memory management fault.
 */
void MemManage_Handler(void)
{
	/* USER CODE BEGIN MemoryManagement_IRQn 0 */
#ifdef DEBUG_MSG
	printf("MemFault!\n");
	if (SCB->CFSR & SCB_CFSR_MMARVALID_Msk)
		printf("SCB_MMFAR=0x%08lX\n", SCB->MMFAR);
#endif
	resetToSafetyMode();
	const int period = 400;
	volatile uint32_t _delay_cnt = (period/2)*(SystemCoreClock/4000);		// us * (SystemCoreClock / 1000000) / 4
	volatile uint32_t _cnt;
	/* USER CODE END MemoryManagement_IRQn 0 */
	while (1)
	{
		/* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
		// Turn off LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BR0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		// Turn on LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BS0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		/* USER CODE END W1_MemoryManagement_IRQn 0 */
	}
}

/**
 * @brief This function handles Pre-fetch fault, memory access fault.
 */
void BusFault_Handler(void)
{
	/* USER CODE BEGIN BusFault_IRQn 0 */
#ifdef DEBUG_MSG
	printf("BusFault!\n");
	if (SCB->CFSR & SCB_CFSR_BFARVALID_Msk)
		printf("SCB_MMFAR=0x%08lX\n", SCB->BFAR);
#endif
	resetToSafetyMode();
	const int period = 800;
	volatile uint32_t _delay_cnt = (period/2)*(SystemCoreClock/4000);		// us * (SystemCoreClock / 1000000) / 4
	volatile uint32_t _cnt;
	/* USER CODE END BusFault_IRQn 0 */
	while (1)
	{
		/* USER CODE BEGIN W1_BusFault_IRQn 0 */
		// Turn off LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BR0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		// Turn on LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BS0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		/* USER CODE END W1_BusFault_IRQn 0 */
	}
}

/**
 * @brief This function handles Undefined instruction or illegal state.
 */
void UsageFault_Handler(void)
{
	/* USER CODE BEGIN UsageFault_IRQn 0 */
	resetToSafetyMode();
	const int period = 1600;
	volatile uint32_t _delay_cnt = (period/2)*(SystemCoreClock/4000);		// us * (SystemCoreClock / 1000000) / 4
	volatile uint32_t _cnt;
	/* USER CODE END UsageFault_IRQn 0 */
	while (1)
	{
		/* USER CODE BEGIN W1_UsageFault_IRQn 0 */
		// Turn off LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BR0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		// Turn on LED1 (PA0)
		GPIOA->BSRR |= GPIO_BSRR_BS0_Msk;
		_cnt = _delay_cnt;
		while (--_cnt)
			;
		/* USER CODE END W1_UsageFault_IRQn 0 */
	}
}

/**
 * @brief This function handles System service call via SWI instruction.
 */
void SVC_Handler(void)
{
	/* USER CODE BEGIN SVCall_IRQn 0 */

	/* USER CODE END SVCall_IRQn 0 */
	/* USER CODE BEGIN SVCall_IRQn 1 */

	/* USER CODE END SVCall_IRQn 1 */
}

/**
 * @brief This function handles Debug monitor.
 */
void DebugMon_Handler(void)
{
	/* USER CODE BEGIN DebugMonitor_IRQn 0 */

	/* USER CODE END DebugMonitor_IRQn 0 */
	/* USER CODE BEGIN DebugMonitor_IRQn 1 */

	/* USER CODE END DebugMonitor_IRQn 1 */
}

/**
 * @brief This function handles Pendable request for system service.
 */
void PendSV_Handler(void)
{
	/* USER CODE BEGIN PendSV_IRQn 0 */

	/* USER CODE END PendSV_IRQn 0 */
	/* USER CODE BEGIN PendSV_IRQn 1 */

	/* USER CODE END PendSV_IRQn 1 */
}

/**
 * @brief This function handles System tick timer.
 */
void SysTick_Handler(void)
{
	/* USER CODE BEGIN SysTick_IRQn 0 */

	/* USER CODE END SysTick_IRQn 0 */
	//HAL_IncTick();
	/* USER CODE BEGIN SysTick_IRQn 1 */
	if (_delay_counter != 0)
		_delay_counter--;
	if (_perf_counter != 0xFFFFFFFF)
		_perf_counter++;
	for (uint8_t i = 0; i < SYSTICK_TIMERS_MAX; i++)
	{
		if (_timer_counter[i] != 0)
			--_timer_counter[i];
	}
	/* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/* USER CODE BEGIN 1 */
void SysTick_Delay_ms(unsigned long ms)
{
	_delay_counter = ms;
	while (_delay_counter != 0)
		;
}

void SysTick_StartPerfCounter()
{
	_perf_counter = 0;
}

void SysTick_StopPerfCounter()
{
	_perf_counter = 0xFFFFFFFF;
}

uint32_t SysTick_GetPerfCounter()
{
	return _perf_counter;
}

void SysTick_StartTimer(uint8_t no, uint32_t ms)
{
	if (no < SYSTICK_TIMERS_MAX)
		_timer_counter[no] = ms;
}

uint8_t SysTick_CheckTimerDone(uint8_t no)
{
	if (no < SYSTICK_TIMERS_MAX)
		return _timer_counter[no] == 0;
	return 1;
}

/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
