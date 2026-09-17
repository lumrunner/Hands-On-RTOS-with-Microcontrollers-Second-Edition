/*

---------------------------------------------------------------------------------------

Licenses:
- Copyright (c) 2019-2025 Packt Publishing, under the MIT License.
- Based on code copyrighted by Brian Amos, 2019, under the MIT License.
- See the code-repository's license statement for more information:
  - https://github.com/PacktPublishing/Hands-On-RTOS-with-Microcontrollers-Second-Edition

 */
#include <stdio.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <stm32f4xx_hal.h>

#include <Nucleo_F446RE_Init.h>
#include <Nucleo_F446RE_GPIO.h>
#include <UartQuickDirtyInit.h>
#include "Uart4Setup.h"
#include <lookBusy.h>

/*********************************************
 * A demonstration of a polled UART driver for
 * sending and receiving
 *********************************************/
#define BAUDRATE 9600

#define STACK_SIZE 128

void polledUartReceive ( void* NotUsed );
void uartPrintOutTask( void* NotUsed);
void startUpTask( void* NotUsed);

static QueueHandle_t uart3_BytesReceived = NULL;

uint32_t iterationsPerMilliSecond;

int main(void)
{
    HWInit();

    // Start UART4, and have it continuously send data.
    // UART4 continuously sends the string "data from uart4", including the null-terminator.
    SetupUart4ExternalSim(BAUDRATE);

    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4); //ensure proper priority grouping for freeRTOS

    // Get the interation-rate for lookBusy()
    iterationsPerMilliSecond = lookBusyIterationRate();

    // Setup tasks, making sure they have been properly created before moving on
    assert_param(xTaskCreate(polledUartReceive, "polledUartRx", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);
    assert_param(xTaskCreate(uartPrintOutTask, "uartPrintTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL) == pdPASS);
    assert_param(xTaskCreate(startUpTask, "startUpTask", STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, NULL) == pdPASS);

    // Create the queue.
    uart3_BytesReceived = xQueueCreate(10, sizeof(char));

    // Start the scheduler - shouldn't return unless there's a problem
    vTaskStartScheduler();

    // If you've wound up here, there is likely an issue with over-running the freeRTOS heap
    while(1)
    {
    }
}


// startUpTask() handles system start-up.
// * It is the highest-priority task.
// * It spins until the SystemView app is started in Record-mode.
// * Then, it deletes itself.
void startUpTask( void* NotUsed )
{
    // Indicate startUpTask has started
    BlueLed.On();

    BlueLed.Off();

    vTaskDelete(NULL);
}

// Gets queue-items and displays them on the SystemView app.
void uartPrintOutTask( void* NotUsed)
{
    char nextByte;

    while(1)
    {
        xQueueReceive(uart3_BytesReceived, &nextByte, portMAX_DELAY);

        // newline necessary for characters to be displayed
        printf("%c\n", nextByte);
    }
}

/**
 * This receive task uses a queue to directly monitor
 * the USART2 peripheral.
 */
void polledUartReceive( void* NotUsed )
{
    uint8_t nextByte;
    // Setup USART2
    STM_UartInit(USART3, BAUDRATE, NULL, NULL);

    while(1)
    {
        while(!(USART3->SR & USART_SR_RXNE));
        nextByte = USART3->DR;

        xQueueSend(uart3_BytesReceived, &nextByte, 0);
    }
}

