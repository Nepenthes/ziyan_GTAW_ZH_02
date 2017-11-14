#ifndef __MYIIC_H
#define __MYIIC_H
#include "IO_Map.h"

typedef unsigned char uint8_t;
								
//IO��������
#define M10_SDA_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=8<<16;}
#define M10_SDA_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=3<<16;}
//IO��������	 
#define M10_IIC_SCL    PCout(1) //SCL

#define M10_IIC_SDA    PAout(4) //SDA	 
#define M10_READ_SDA   PAin(4)  //����SDA 

//u32 mybmp180 (void);                          //����bmp180 ����

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	

#endif
















