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
#include <semphr.h>
#include <timers.h>
#include <stm32f4xx_hal.h>

#include <Nucleo_F446RE_Init.h>
#include <Nucleo_F446RE_GPIO.h>
#include <lookBusy.h>

#define STACK_SIZE 128

void taskStartTimers( void * argument);

void oneShotCallBack( TimerHandle_t xTimer );
void repeatCallBack( TimerHandle_t xTimer );

uint32_t iterationsPerMilliSecond;

int main(void)
{
	HWInit();
	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);	//ensure proper priority grouping for FreeRTOS

    // Get the interation-rate for lookBusy()
    iterationsPerMilliSecond = lookBusyIterationRate();

	// Create the task that will start the timers.
	// Set the task's priority to be higher than the timer-task: (configTIMER_TASK_PRIORITY + 1).
	assert_param(xTaskCreate(taskStartTimers, "startTimersTask",
	                         STACK_SIZE, NULL, (configTIMER_TASK_PRIORITY + 1), NULL) == pdPASS);

	// Start the scheduler - shouldn't return unless there's a problem
	vTaskStartScheduler();

	// If you've wound up here, there is likely an issue with over-running the freeRTOS heap
	while(1)
	{
	}
}

// Task used to start the timers
void taskStartTimers( void* argument )
{
    // Indicate taskStartTimers started
    RedLed.On();
    // Spin until the user starts the SystemView app, in Record mode
    RedLed.Off();

	printf("taskStartTimers: starting\r\n");

    //
    // Create the one-shot timer, and start it
    //

	// Start with Blue LED on - it will be turned off after one-shot fires
	BlueLed.On();
	printf("taskStartTimers: blue LED on\r\n");
	TimerHandle_t oneShotHandle =
		xTimerCreate(	"myOneShotTimer",			//name for timer
						2200 /portTICK_PERIOD_MS,	//period of timer in ticks
						pdFALSE,					//auto-reload flag
						NULL,						//unique ID for timer
						oneShotCallBack);			//callback function
	assert_param(oneShotHandle != NULL);

	printf("taskStartTimers: one-shot timer started (turns off blue LED)\r\n");
	xTimerStart(oneShotHandle, 0);


    //
    // Create the repeat-timer, and start it
    //

    TimerHandle_t repeatHandle =
        xTimerCreate(   "myRepeatTimer",            //name for timer
                        500 /portTICK_PERIOD_MS,    //period of timer in ticks
                        pdTRUE,                     //auto-reload flag
                        NULL,                       //unique ID for timer
                        repeatCallBack);            //callback function
    assert_param(repeatHandle != NULL);

    printf("taskStartTimers: repeating-timer started (blinks the green LED)\r\n");
    xTimerStart(repeatHandle, 0);


	// The task deletes itself
    printf("taskStartTimers: deleting itself\r\n");
	vTaskDelete(NULL);

    // The task never gets to here
	while(1)
	{
	}

}


void oneShotCallBack( TimerHandle_t xTimer )
{
	printf("oneShotCallBack:  blue LED off\r\n");
	BlueLed.Off();
}


void repeatCallBack( TimerHandle_t xTimer )
{
	static uint32_t counter = 0;

	printf("repeatCallBack:  toggle Green LED\r\n");
	// Toggle the green LED
	if(counter++ % 2)
	{
		GreenLed.On();
	}
	else
	{
		GreenLed.Off();
	}
}


