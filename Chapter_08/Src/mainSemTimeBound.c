/**

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
#include <semphr.h>
#include <stm32f4xx_hal.h>

#include <Nucleo_F446RE_Init.h>
#include <Nucleo_F446RE_GPIO.h>

#define STACK_SIZE 128

static void greenBlink( void );
static void blueTripleBlink( void );

void GreenTaskA( void * argument);
void TaskB( void* argumet );

// Create storage for a pointer to a semaphore
SemaphoreHandle_t semPtr = NULL;

int main(void)
{
	HWInit();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	// Ensure proper priority grouping for freeRTOS

	// Create a semaphore using the FreeRTOS Heap
	semPtr = xSemaphoreCreateBinary();
	assert_param(semPtr != NULL);

	// Create TaskA as a higher priority than TaskB.  In this example, this isn't strictly necessary since the tasks
	// spend nearly all of their time blocked
	assert_param(xTaskCreate(GreenTaskA, "GreenTaskA", STACK_SIZE, NULL, tskIDLE_PRIORITY + 2, NULL) == pdPASS);

	// Using an assert to ensure proper task creation
	assert_param(xTaskCreate(TaskB, "TaskB", STACK_SIZE, NULL, tskIDLE_PRIORITY + 1, NULL) == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

/**
 * Task A periodically 'gives' semaphorePtr.  This version
   has some variability in how often it will give the semaphore
 * NOTES:
 * - This semaphore isn't "given" to any task specifically
 * - Giving the semaphore doesn't prevent taskA from continuing to run.
 * - Notice the green LED continues to blink at all times.
 */
void GreenTaskA( void* argument )
{
	uint_fast8_t count = 0;

	while(1)
	{
	    // Get a random number between 3 and 9, inclusive.
		uint8_t numLoops = StmRand(3,9);
		if(++count >= numLoops)
		{
			count = 0;
			printf("Task A (green LED) gives semPtr\r\n");
			xSemaphoreGive(semPtr);
		}
		greenBlink();
	}
}

/**
 * Wait to receive semPtr and triple blink the Blue LED.
 * If the semaphore isn't available within 500 ms, then
   turn on the RED LED until the semaphore is available.
 */
void TaskB( void* argument )
{
	while(1)
	{
		// 'take' the semaphore with a 500mS timeout
		printf("attempt to take semPtr\r\n");
		if(xSemaphoreTake(semPtr, 500/portTICK_PERIOD_MS) == pdPASS)
		{
			RedLed.Off();
			printf("received semPtr\r\n");
			blueTripleBlink();
		}
		else
		{
			// This code is run when the semaphore wasn't taken in time
			printf("FAILED to receive semPtr in time\r\n");
			RedLed.On();
		}
	}
}

/**
 * Blink the Green LED once using vTaskDelay
 */
static void greenBlink( void )
{
	GreenLed.On();
	vTaskDelay(100/portTICK_PERIOD_MS);
	GreenLed.Off();
	vTaskDelay(100/portTICK_PERIOD_MS);
}

/**
 * blink the Blue LED 3 times in rapid succession using vtaskDelay
 */
static void blueTripleBlink( void )
{
	//triple blink the Blue LED
	for(uint_fast8_t i = 0; i < 3; i++)
	{
		BlueLed.On();
		vTaskDelay(50/portTICK_PERIOD_MS);
		BlueLed.Off();
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
}
