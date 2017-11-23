#ifndef CONTENTCO2_H
#define CONTENTCO2_H
#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

void contentCO2_Adc_Init(void);
uint16_t contentCO2_Get_Adc(uint8_t ch) ; 

void contentCO2_Init(void);

void contentCO2MS_Thread(const void *argument);
void contentCO2MS(void);
 
#endif 
