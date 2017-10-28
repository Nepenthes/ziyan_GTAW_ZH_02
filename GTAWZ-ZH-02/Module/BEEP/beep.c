#include "beep.h"

void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能GPIOA端口时钟
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //BEEP-->PC.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);	 //根据参数初始化GPIOA.0
 
 GPIO_SetBits(GPIOC,GPIO_Pin_5);//输出1，关闭蜂鸣器输出
}

void Beep_time(u16 time)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_5, Bit_RESET);
	osDelay(time);
	GPIO_WriteBit(GPIOC, GPIO_Pin_5, Bit_SET);
	osDelay(time);
}	



