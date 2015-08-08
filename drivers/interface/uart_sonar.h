/**
 *    ||          ____  _ __                           
 * +------+      / __ )(_) /_______________ _____  ___ 
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
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
 * uart.h - uart CRTP link and raw access functions
 */
#ifndef UART_SONAR_H_
#define UART_SONAR_H_

#include <stdbool.h>

#include "crtp.h"
#include "eprintf.h"


#define UART_SONAR_TYPE				USART3
#define UART_SONAR_PERIF			RCC_APB1Periph_USART3
#define ENABLE_UART_SONAR_RCC		RCC_APB1PeriphClockCmd
#define UART_SONAR_IRQ				USART3_IRQn


#define UART_SONAR_GPIO_PERIF		RCC_AHB1Periph_GPIOC
#define UART_SONAR_GPIO_PORT		GPIOC
#define UART_SONAR_GPIO_TX_PIN		GPIO_Pin_10
#define UART_SONAR_GPIO_RX_PIN		GPIO_Pin_11
#define UART_SONAR_GPIO_AF_TX_PIN	GPIO_PinSource10
#define UART_SONAR_GPIO_AF_RX_PIN	GPIO_PinSource11
#define UART_SONAR_GPIO_AF_TX		GPIO_AF_USART3
#define UART_SONAR_GPIO_AF_RX		GPIO_AF_USART3



/**
 * Initialize the UART.
 *
 * @note Initialize CRTP link only if USE_CRTP_UART is defined
 */
void uartSonar_Init(void);


/**
 * Test the UART status.
 *
 * @return true if the UART is initialized
 */
bool uartSonar_Test(void);


/**
 * Get data from rx queue with timeout.
 * @param[out] c  Byte of data
 *
 * @return true if byte received, false if timout reached.
 */
bool uartSonar_GetDataWithTimout(uint8_t *c);




void uartSonar_Isr(void);


#endif
