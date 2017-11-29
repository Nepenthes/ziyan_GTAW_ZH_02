#ifndef __CURTAIN_H
#define __CURTAIN_H	 
#include "delay.h"
#include "IO_Map.h"	 
#include "beep.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#define KEY3_Curtain  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_15)//读取按键1
#define KEY4_Curtain  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_0)//读取按键2 
#define KEY5_Curtain  GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_1)//读取按键3 

#define CURTAIN_EN	PDout(14)
#define CURTAIN_CLK	PEout(15)
#define CURTAIN_DIR	PEout(14)

#define CURLIMIT_UP	PEin(0)
#define CURLIMIT_DN	PEin(1)

#define CUR_OPEN 		0x01
#define CUR_CLOSE 	0x02
#define CUR_STOP 		0x03

void KEY_Init_Curtain(void);//IO初始化	
u8 KEY_Scan1_Curtain(u8);  //按键扫描函数	

void Curtain_Init(void);
void CurtainCM_CLK_Thread(const void *argument);
void CurtainCM_Thread(const void *argument);
void Curtain(void);
		 				    
#endif
