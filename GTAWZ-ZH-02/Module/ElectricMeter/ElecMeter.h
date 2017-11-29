#ifndef __ELECMETER_H_
#define __ELECMETER_H_

#include "stm32f10x.h"
#include "delay.h"
#include "IO_Map.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define CON485READ	PCout(9) = 0
#define CON485SEND	PCout(9) = 1

void USART4ElecMeter_Init(void);
void USART4ElecMeter_Thread(const void *argument);
void USART4ElecMeter(void);

#endif
