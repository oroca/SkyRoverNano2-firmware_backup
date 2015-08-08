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
 * uart.c - uart CRTP link and raw access functions
 */
#include <string.h>

/*ST includes */
#include "stm32fxxx.h"

/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "config.h"
#include "uart_sonar.h"
#include "crtp.h"
#include "cfassert.h"
#include "nvicconf.h"
#include "config.h"
#include "ledseq.h"


#define UART_DATA_TIMEOUT_MS 1000
#define UART_DATA_TIMEOUT_TICKS (UART_DATA_TIMEOUT_MS / portTICK_RATE_MS)
#define CCR_ENABLE_SET  ((uint32_t)0x00000001)

static bool isInit = false;


static xQueueHandle uartSonar_DataDelivery;







void uartSonar_Init(void)
{

  USART_InitTypeDef USART_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  EXTI_InitTypeDef extiInit;

  /* Enable GPIO and USART clock */
  ENABLE_UART_SONAR_RCC(UART_SONAR_PERIF, ENABLE);

  /* Configure USART Rx as input floating */
  GPIO_InitStructure.GPIO_Pin   = UART_SONAR_GPIO_RX_PIN;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(UART_SONAR_GPIO_PORT, &GPIO_InitStructure);

  /* Configure USART Tx as alternate function */
  GPIO_InitStructure.GPIO_Pin   = UART_SONAR_GPIO_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_Init(UART_SONAR_GPIO_PORT, &GPIO_InitStructure);

  //Map uart to alternate functions
  GPIO_PinAFConfig(UART_SONAR_GPIO_PORT, UART_SONAR_GPIO_AF_TX_PIN, UART_SONAR_GPIO_AF_TX);
  GPIO_PinAFConfig(UART_SONAR_GPIO_PORT, UART_SONAR_GPIO_AF_RX_PIN, UART_SONAR_GPIO_AF_RX);

  USART_InitStructure.USART_BaudRate            = 9600;
  USART_InitStructure.USART_Mode                = USART_Mode_Rx;

  USART_InitStructure.USART_WordLength          = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits            = USART_StopBits_1;
  USART_InitStructure.USART_Parity              = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(UART_SONAR_TYPE, &USART_InitStructure);



  // TODO: Enable
  // Configure Tx buffer empty interrupt
  NVIC_InitStructure.NVIC_IRQChannel = UART_SONAR_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_UART_PRI;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  uartSonar_DataDelivery = xQueueCreate(40, sizeof(uint8_t));

  USART_ITConfig(UART_SONAR_TYPE, USART_IT_RXNE, ENABLE);


  //Enable UART
  USART_Cmd(UART_SONAR_TYPE, ENABLE);
  
  isInit = true;
}


bool uartSonar_Test(void)
{
  return isInit;
}


bool uartSonar_GetDataWithTimout(uint8_t *c)
{
  if (xQueueReceive(uartSonar_DataDelivery, c, UART_DATA_TIMEOUT_TICKS) == pdTRUE)
  {
    return true;
  }
  return false;
}



void uartSonar_Isr(void)
{
  portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
  uint8_t rxDataInterrupt;

  if (USART_GetITStatus(UART_SONAR_TYPE, USART_IT_RXNE))
  {
    rxDataInterrupt = USART_ReceiveData(UART_SONAR_TYPE) & 0x00FF;
    xQueueSendFromISR(uartSonar_DataDelivery, &rxDataInterrupt, &xHigherPriorityTaskWoken);
  }
}

