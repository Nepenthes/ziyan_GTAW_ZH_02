#ifndef __KEY_H
#define __KEY_H	

#include "IO_Map.h" 
#include "delay.h"
#include "Driver_USART.h"

#include "stdio.h"
#include "string.h"

#define ROW1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_12)//��ȡ
#define ROW2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)//��ȡ 
#define ROW3  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_6)//��ȡ
#define ROW4  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_7)//��ȡ 

#define COL4  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_8)//��ȡ
#define COL3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_8)//��ȡ 
#define COL2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_8)//��ȡ
#define COL1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_9)//��ȡ 


#define GPIO1_KB PCout(12)
#define GPIO2_KB PCout(13)
#define GPIO3_KB PCout(6)
#define GPIO4_KB PCout(7)

#define GPIO5_KB PCout(8)
#define GPIO6_KB PAout(8)
#define GPIO7_KB PBout(8)
#define GPIO8_KB PBout(9)

void  keydown(void);			

void keyBoard_Init(void);
void KeyBoardMS_Thread(const void *argument);
void KeyBoardMS(void);

#endif
