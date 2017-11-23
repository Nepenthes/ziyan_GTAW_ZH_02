#ifndef __SOURCETOG_H_
#define __SOURCETOG_H_

#include "IO_Map.h"
#include "stm32f10x.h"
#include "delay.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define SOURCEMAIN_ACTIVE	PEout(1) = 1;PDout(11) = 0
#define SOURCEBACK_ACTIVE	PEout(1) = 0;PDout(11) = 1

void  SourceTog_Adc_Init(void);
uint16_t SourceTog_Get_Adc(uint8_t ch);   
uint16_t SourceTog_Get_Adc_Average(uint8_t ch,uint8_t times);

void SourceTog_Init(void);
void SouTogCM_Thread(const void *argument);
void PowDetect_Thread(const void *argument);
void SourceTogCM(void);
 
#endif 
