#ifndef WINDSPEED_H
#define WINDSPEED_H	
#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

uint16_t windSpeed_Get_Adc_Average(uint8_t ch,uint8_t times);
void windSpeed_Init(void);
void windSpeedMS_Thread(const void *argument);
void windSpeedMS(void);
 
#endif 
