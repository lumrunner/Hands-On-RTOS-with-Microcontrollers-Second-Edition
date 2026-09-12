/**
 * LED implementation for Red, Green, and Blue discrete LED's on
 * Nucleo-F767ZI
 */

#include <Nucleo_F446RE_GPIO.h>
#include <stm32f4xx_hal.h>

void GreenOn ( void ) {HAL_GPIO_WritePin(LD_GPIO_Port, LD1_Pin, GPIO_PIN_SET);}
void GreenOff ( void ) {HAL_GPIO_WritePin(LD_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);}
LED GreenLed = { GreenOn, GreenOff };

void BlueOn ( void ) {HAL_GPIO_WritePin(LD_GPIO_Port, LD2_Pin, GPIO_PIN_SET);}
void BlueOff ( void ) {HAL_GPIO_WritePin(LD_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);}
LED BlueLed = { BlueOn, BlueOff };

void RedOn ( void ) {HAL_GPIO_WritePin(LD_GPIO_Port, LD3_Pin, GPIO_PIN_SET);}
void RedOff ( void ) {HAL_GPIO_WritePin(LD_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);}
LED RedLed = { RedOn, RedOff };

uint_fast8_t ReadPushButton( void ){ return HAL_GPIO_ReadPin(USER_Btn_GPIO_Port, USER_Btn_Pin);}
