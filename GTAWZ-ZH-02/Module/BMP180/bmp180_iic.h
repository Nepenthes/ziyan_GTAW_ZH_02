#ifndef __BMPIIC_H
#define __BMPIIC_H
#include "IO_Map.h"

typedef unsigned char uint8_t;
								
//IO��������
#define M10_SDA_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=8<<16;}
#define M10_SDA_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=3<<16;}
//IO��������	 
#define M10_BMPIIC_SCL    PCout(1) //SCL

#define M10_BMPIIC_SDA    PAout(4) //SDA	 
#define M10_READ_SDA   PAin(4)  //����SDA 

//u32 mybmp180 (void);                          //����bmp180 ����

//IIC���в�������
void BMPIIC_Init(void);                //��ʼ��IIC��IO��				 
void BMPIIC_Start(void);				//����IIC��ʼ�ź�
void BMPIIC_Stop(void);	  			//����IICֹͣ�ź�
void BMPIIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t BMPIIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t BMPIIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void BMPIIC_Ack(void);					//IIC����ACK�ź�
void BMPIIC_NAck(void);				//IIC������ACK�ź�

void BMPIIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t BMPIIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	

#endif
















