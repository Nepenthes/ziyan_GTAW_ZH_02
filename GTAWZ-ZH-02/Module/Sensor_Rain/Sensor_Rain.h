#ifndef SENSOR_RAIN_H
#define SENSOR_RAIN_H

#include "stm32f10x.h"
#include "delay.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define RAIN_DATA  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)

void RAIN_Init(void);
void RAINMS_Thread(const void *argument);
void RAINMS(void);
	
#endif
