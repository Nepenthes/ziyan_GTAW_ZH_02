#ifndef __LEDGROW_H
#define __LEDGROW_H	 
#include "delay.h"
#include "IO_Map.h"
#include "beep.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define KEY3_ledGRW  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_0)//��ȡ����0
#define KEY4_ledGRW  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_1)//��ȡ����1
#define KEY5_ledGRW  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_11)//��ȡ����2 

void KEY_Init_ledGRW(void);//IO��ʼ��
u8 KEY_Scan1_ledGRW(u8);  	//����ɨ�躯��		

void ledGrow_Init_ledGRW(void);
void ledGrow_ledGRW(void);

void LEDGrow_Init(void);
void LEDGrowCM_Thread(const void *argument);
void LEDGrow(void);
void LEDGrowCM_DB_Thread(const void *argument);
		 				    
#endif
