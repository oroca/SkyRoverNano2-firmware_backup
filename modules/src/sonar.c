/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie Firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */
#include <math.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "config.h"
#include "system.h"
#include "pm.h"
#include "commander.h"
#include "log.h"
#include "param.h"
#include "lps25h.h"
#include "debug.h"
#include "uart_sonar.h"




static bool isInit;
uint16_t	sonarRange;

static volatile uint16_t isr_counter = 0;


volatile uint16_t pulse_flag = 0;
volatile uint16_t pulse_start;
volatile uint16_t pulse_end;
volatile uint16_t pulse_time;



static void sonarTask(void* param);





void sonarInit(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef extiInit;


	if(isInit)
		return;

	//uartSonar_Init();


	//-- 타이머 설
	//
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	//Timer configuration
	TIM_TimeBaseStructure.TIM_Period    		= 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler 		= (84-1);
	TIM_TimeBaseStructure.TIM_ClockDivision 	= 0;
	TIM_TimeBaseStructure.TIM_CounterMode 		= TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);


	TIM_Cmd( TIM6, ENABLE );


	//-- 외부 인터럽트 설
	//
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	bzero(&GPIO_InitStructure, sizeof(GPIO_InitStructure));
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	extiInit.EXTI_Line    = EXTI_Line11;
	extiInit.EXTI_Mode    = EXTI_Mode_Interrupt;
	extiInit.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	extiInit.EXTI_LineCmd = ENABLE;
	EXTI_Init(&extiInit);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource11);




	NVIC_EnableIRQ(EXTI15_10_IRQn);


	xTaskCreate(sonarTask, (const signed char * const)SONAR_TASK_NAME,
                SONAR_TASK_STACKSIZE, NULL, SONAR_TASK_PRI, NULL);

	isInit = true;
}


bool sonarTest(void)
{
	bool pass = true;

	pass &= uartSonar_Test();

	return pass;
}


static void sonarTask(void* param)
{
	uint16_t timer_start;
	uint16_t timer_end;
	uint16_t timer_time;

	uint8_t rxState = 0;
	uint8_t c;
	uint8_t dataIndex = 0;
	uint8_t counter = 0;

	TIM_TypeDef* TIM = TIM6;


	DEBUG_PRINT("sonarTask Start\n");

#if 1
	while(1)
	{
		vTaskDelay(M2T(1000));
	}

#else
	while(1)
	{
		timer_start = TIM->CNT;
		vTaskDelay(M2T(10));
		timer_end = TIM->CNT;
		timer_time = timer_end-timer_start;
		DEBUG_PRINT("Cnt : %d %d\n", timer_time, isr_counter);
		vTaskDelay(M2T(1000));
	}


	while(1)
	{
		if ( uartSonar_GetDataWithTimout(&c) )
	    {
			DEBUG_PRINT("Sonar : %X : %c\n", c, c);

			counter++;
			switch(rxState)
			{
				case 0:
					rxState = 1;
					break;

				case 1:
					rxState = 0;
					break;
				default:
					ASSERT(0);
					break;
			}
	    }
	    else
	    {
	    	// Timeout
	    	rxState = 0;
	    }
	}
#endif
}


void sonarExtiInterruptHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line11) == SET)
	{
		isr_counter++;


		if( pulse_flag == 0 )
		{
			pulse_start = TIM6->CNT;
		}
		else
		{
			pulse_end = TIM6->CNT;
			pulse_time = pulse_end-pulse_start;
			sonarRange = pulse_time/57;
		}

		pulse_flag ^= 1;

		EXTI_ClearITPendingBit(EXTI_Line11);
	}
}

LOG_GROUP_START(sonar)
LOG_ADD(LOG_UINT16, sonarRange, &sonarRange)
LOG_GROUP_STOP(sonar)

