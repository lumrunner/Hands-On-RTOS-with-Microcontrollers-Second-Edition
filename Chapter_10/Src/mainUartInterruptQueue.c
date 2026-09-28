/*

---------------------------------------------------------------------------------------

Licenses:
- Copyright (c) 2019-2025 Packt Publishing, under the MIT License.
- Based on code copyrighted by Brian Amos, 2019, under the MIT License.
- See the code-repository's license statement for more information:
  - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-Second-Edition

 */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <timers.h>
#include <SEGGER_SYSVIEW.h>

#include <stm32f4xx_hal.h>

#include <Nucleo_F446RE_Init.h>
#include <Nucleo_F446RE_GPIO.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <stdbool.h>
#include <string.h>

/*********************************************
 * A demonstration of a simple receive-only interrupt-driven UART driver,
 * that uses a queue.
 *********************************************/

#define STACK_SIZE 128

#define BAUDRATE 9600

void uartPrintOutTask( void* NotUsed);
void startUart4Traffic( TimerHandle_t xTimer );

static QueueHandle_t usart3_BytesReceived = NULL;

// Indicates USART2 is enabled to receive
static volatile bool rxInProgress = false;

int main(void)
{
	HWInit();
	SEGGER_SYSVIEW_Conf();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); //ensure proper priority grouping for freeRTOS

	// Setup a timer to kick off UART traffic (flowing out of UART4 TX line
	// and into USART3 RX line) 5 seconds after the scheduler starts.
	// The transmission needs to start after the receiver is ready for data.
	TimerHandle_t oneShotHandle =
	xTimerCreate(	"startUart4Traffic",
					5000 / portTICK_PERIOD_MS,
					pdFALSE,
					NULL,
					startUart4Traffic);
	assert_param(oneShotHandle != NULL);
	xTimerStart(oneShotHandle, 0);

    // Create the queue
	usart3_BytesReceived = xQueueCreate(10, sizeof(char));
	assert_param(usart3_BytesReceived != NULL);

    // Setup the task, making sure they have been properly created before moving on
	assert_param(xTaskCreate(uartPrintOutTask, "uartPrint", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Start an interrupt driven receive. This particular ISR is hard-coded
 * to push characters into a queue
 */
void startReceiveInt( void )
{
	rxInProgress = true;
	USART3->CR3 |= USART_CR3_EIE;	//enable error interrupts
	USART3->CR1 |= (USART_CR1_UE | USART_CR1_RXNEIE);
	//all 4 bits are for preemption priority -
	NVIC_SetPriority(USART3_IRQn, 5);
	NVIC_EnableIRQ(USART3_IRQn);
}

void startUart4Traffic( TimerHandle_t xTimer )
{
	SetupUart4ExternalSim(BAUDRATE);
}

void uartPrintOutTask( void* NotUsed)
{
	char nextByte;
	STM_UartInit(USART3, BAUDRATE, NULL, NULL);
	startReceiveInt();

	while(1)
	{
		xQueueReceive(usart3_BytesReceived, &nextByte, portMAX_DELAY);
		// In "%c ", the space is a workaround for an apparent bug in SystemView.
		SEGGER_SYSVIEW_PrintfHost("%c\n", nextByte);
	}
}

void USART3_IRQHandler( void )
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	SEGGER_SYSVIEW_RecordEnterISR();
	// Clear error flags
	USART3->SR &= ~(USART_SR_FE |
					USART_SR_PE |
					USART_SR_NE |
					USART_SR_ORE);

	if(	USART3->SR & USART_SR_RXNE)
	{
		// Read the data register unconditionally to clear
		// the receive-not-empty interrupt if no reception is
		// in progress
		uint8_t tempVal = (uint8_t) USART3->DR;

		if(rxInProgress)
		{
			xQueueSendFromISR(usart3_BytesReceived, &tempVal, &xHigherPriorityTaskWoken);
		}
	}
	SEGGER_SYSVIEW_RecordExitISR();
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
