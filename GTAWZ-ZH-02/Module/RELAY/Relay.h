#ifndef SENSOR_RAIN_H
#define SENSOR_RAIN_H

#include "stm32f10x.h"
#include "delay.h"
#include "IO_Map.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define RELAY1_ON		PCout(12) = 1;
#define RELAY1_OFF	PCout(12) = 0;
#define RELAY2_ON		PCout(13) = 1;
#define RELAY2_OFF	PCout(13) = 0;

void swRelay_Init(void);
void swRelayCM_Thread(const void *argument);
void swRelayCM(void);
	
#endif
