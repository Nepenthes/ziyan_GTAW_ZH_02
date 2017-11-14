#ifndef SENSOR_RT_H
#define SENSOR_RT_H

#include "stm32f10x.h"
#include "delay.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define RT_DATA  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_7)

void RT_Init(void);
void RTMS_Thread(const void *argument);
void RTMS(void);

#endif
