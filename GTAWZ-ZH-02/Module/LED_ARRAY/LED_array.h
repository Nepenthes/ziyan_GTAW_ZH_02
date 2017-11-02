#ifndef __LED_H
#define __LED_H	 

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "beep.h"

#include <IO_Map.h>
#include "delay.h"

//#define RLY1 PBout(7)// PB7
//#define RLY2 PBout(8)// PB8
#define D_NUM 10

#define GPIO1 PEout(6)//
#define GPIO2 PEout(5)//
#define GPIO3 PEout(4)//
#define GPIO4 PEout(3)//

#define GPIO5 PEout(2)//
#define GPIO6 PDout(13)//
#define GPIO7 PDout(12)//
#define GPIO8 PDout(7)//

#define GPIO9  PCout(12)//
#define GPIO10 PCout(13)//
#define GPIO11 PCout(6)//
#define GPIO12 PCout(7)//

#define GPIO13 PCout(8)//
#define GPIO14 PAout(8)//
#define GPIO15 PBout(8)//
#define GPIO16 PBout(9)//

//#define LED0 PAout(8)// PA5
//#define LED1 PAout(12)// PA12	

#define EN   PDout(11)// 
#define LCLK PEout(1)// 	
#define SCLK PDout(14)// 
#define DS   PDout(15)// 

#define EN1   PDout(1)// 
#define LCLK1 PEout(9)// 	
#define SCLK1 PEout(7)// 
#define DS1   PEout(8)// 

#define EN2   PDout(7)// 
#define LCLK2 PDout(5)// 	
#define SCLK2 PDout(4)// 
#define DS2   PDout(10)// 


#define DI1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_5)//PB5
#define DI2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_6)//PB6


void LED_Init(void);//��ʼ��
void RELAY_Init(void);//�̵������
void LEDArray_Init(void);//
void SH_595(u8 Date_in);
void OUT_595(void);
void DIO1_Init(void);
void DIO2_Init(void);

void SH_595R(u8 Date_in);
void OUT_595R(void);

void SH_595G(u8 Date_in);
void OUT_595G(void);

void ROW_SEL(u8 dat);
//void COLR_DATA(unsigned char *dat);
void COLR_DATA(void);

void	D_DATA(void);

void LEDArrayTest(void);
void LEDArrayTest_Thread(const void *argument);

/*--  ������һ��ͼ���������½���ͼ��  --*/
/*--  ����x�߶�=128x1  --*/
//const 
//	unsigned char gImage_dat1[] = { 
//0xF0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
//	
//};

// 4   4*8 
void ROW_OFF(void);
void DISPLAY_R(void);
void DISPLAY_G(void);
void DISPLAY_RG(void);

#endif
