#ifndef BSP_NUCLEO_F446RE_GPIO_H_
#define BSP_NUCLEO_F446RE_GPIO_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define LD_GPIO_Port GPIOA
#define LD1_Pin GPIO_PIN_5 // Green led on board
#define LD2_Pin GPIO_PIN_6 // pin, check on O-scope
#define LD3_Pin GPIO_PIN_7 // pin, check on O-scope

#define USER_Btn_GPIO_Port GPIOC
#define USER_Btn_Pin GPIO_PIN_13 // physical button on board

//Create a typedef defining a simple function pointer
//to be used for LED's
typedef void (*GPIOFunc)(void);

//this struct holds function pointers to turn each LED
//on and off
typedef struct
{
	const GPIOFunc On;
	const GPIOFunc Off;
}LED;

uint_fast8_t ReadPushButton( void );

extern LED BlueLed;
extern LED GreenLed;
extern LED RedLed;

#endif /* BSP_NUCLEO_F446RE_GPIO_H_ */
