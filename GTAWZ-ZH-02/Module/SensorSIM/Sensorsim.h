#ifndef __ADC_H
#define __ADC_H	
#include "IO_Map.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define DIO  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)//PB6

void DI_Init(void);
void Adc_Init(void);
uint16_t Get_Adc(uint8_t ch) ; 

void SensorSIM_Init(void);

void SenSimMS_Thread(const void *argument);
void SensorSIMMS(void);
 
#endif 
