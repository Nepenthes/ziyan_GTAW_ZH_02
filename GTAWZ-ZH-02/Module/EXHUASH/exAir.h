#ifndef __EXAIR_H
#define __EXAIR_H	 
#include "delay.h"
#include "IO_Map.h"	 
#include "beep.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define KEY3_exAir  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_0)//读取按键1
#define KEY4_exAir  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_1)//读取按键2 
#define KEY5_exAir  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_7)//读取按键3 

void TIM4_PWM_Init_exAir(u16 arr,u16 psc);

void KEY_Init_exAir(void);//IO初始化	
u8 KEY_Scan1_exAir(u8);  //按键扫描函数	

void exAir_Init(void);
void exAirCM_Thread(const void *argument);
void exAirCM_DB_Thread(const void *argument);
void exAir(void);
		 				    
#endif
