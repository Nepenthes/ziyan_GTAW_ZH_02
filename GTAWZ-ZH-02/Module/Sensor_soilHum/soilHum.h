#ifndef SOILHUM_H
#define SOILHUM_H
#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

void soilHum_Adc_Init(void);
uint16_t soilHum_Get_Adc(uint8_t ch) ; 

void soilHum_Init(void);

void soilHumMS_Thread(const void *argument);
void soilHumMS(void);
 
#endif 
