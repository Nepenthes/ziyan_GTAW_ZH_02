#ifndef __LED_GROW_H
#define __LED_GROW_H	 
#include "IO_Map.h"
#include "delay.h" 
#include "beep.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define EN_ledSPY   PEout(0)// 
#define LCLK_ledSPY PEout(1)// 	
#define SCLK_ledSPY PDout(11)// 
#define DS_ledSPY   PDout(14)// 

#define KEY3_ledSPY  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15)//��ȡ����1
#define KEY4_ledSPY  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_0)//��ȡ����2 

void KEY_Init_ledSPY(void);//IO��ʼ��

u8 KEY_Scan1_ledSPY(u8);  	//����ɨ�躯��					    
void LED_Init(void);//��ʼ��
void NC595_Init(void);//
void OUT_595(void);
void LED_Spray(void);

void flash_SPY_Thread(const void *argument);
void flash_PST_Thread(const void *argument);

void LEDSpray_Init(void);
void LEDSprayCM_Thread(const void *argument);
void LEDSprayCM_DB_Thread(const void *argument);
void LEDSpray(void);
		 				    
#endif
