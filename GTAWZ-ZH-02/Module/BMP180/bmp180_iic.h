#ifndef __MYIIC_H
#define __MYIIC_H
#include "IO_Map.h"

typedef unsigned char uint8_t;
								
//IO方向设置
#define M10_SDA_IN()  {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=8<<16;}
#define M10_SDA_OUT() {GPIOA->CRL&=0XFFF0FFFF;GPIOA->CRL|=3<<16;}
//IO操作函数	 
#define M10_IIC_SCL    PCout(1) //SCL

#define M10_IIC_SDA    PAout(4) //SDA	 
#define M10_READ_SDA   PAin(4)  //输入SDA 

//u32 mybmp180 (void);                          //定义bmp180 驱动

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口				 
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(uint8_t txd);			//IIC发送一个字节
uint8_t IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
uint8_t IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	

#endif
















