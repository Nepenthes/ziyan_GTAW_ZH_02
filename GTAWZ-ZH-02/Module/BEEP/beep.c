#include "beep.h"

void BEEP_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��GPIOA�˿�ʱ��
 
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //BEEP-->PC.5 �˿�����
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	 //�ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);	 //���ݲ�����ʼ��GPIOA.0
 
 GPIO_SetBits(GPIOC,GPIO_Pin_5);//���1���رշ��������
}

void Beep_time(u16 time)
{
	GPIO_WriteBit(GPIOC, GPIO_Pin_5, Bit_RESET);
	osDelay(time);
	GPIO_WriteBit(GPIOC, GPIO_Pin_5, Bit_SET);
	osDelay(time);
}	



