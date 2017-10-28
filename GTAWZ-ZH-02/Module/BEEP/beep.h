#ifndef __BEEP_H
#define __BEEP_H	   

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions

void BEEP_Init(void);	//≥ı ºªØ
void Beep_time(u16 time);
		 				    
#endif

