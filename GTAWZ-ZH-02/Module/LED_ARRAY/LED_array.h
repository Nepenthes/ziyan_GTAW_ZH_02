#ifndef __LEDARRAY_H
#define __LEDARRAY_H	 

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "beep.h"
#include "string.h"

#include <IO_Map.h>
#include "delay.h"

#include "USART_STM32F10x.h"

#define D_NUM 						10
#define DISPLA_BUFFER_SIZE		480*2

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

void LED_Init(void);//初始化
void RELAY_Init(void);//继电器输出
void LEDArray_Init(void);//
void SH_595(uint8_t Date_in);
void OUT_595(void);
void DIO1_Init(void);
void DIO2_Init(void);

void SH_595R(uint8_t Date_in);
void OUT_595R(void);

void SH_595G(uint8_t Date_in);
void OUT_595G(void);

void ROW_SEL(uint8_t dat);
void COLR_DATA(void);

void	D_DATA(void);

void LEDArrayCM(void);
void LEDArrayCM_Thread(const void *argument);

void TaskLAdisp(void const *argument);
void LEDArryDisp(void);

/*--  调入了一幅图像：这是您新建的图像  --*/
/*--  宽度x高度=128x1  --*/

// 4   4*8 
void ROW_OFF(void); 
void DISPLAY_RG(unsigned char gImage_dat[8][16]);
void DISPLAY_R(unsigned char gImage_dat[8][16]);
void DISPLAY_G(unsigned char gImage_dat[8][16]);
void DISPLAY_ASC(unsigned char gImage_dat[8][16],char color);

void DiSP_HANZI(char *str,char color,char speed);
void DiSP_ASC(char *str,char color,char speed);

void DISP_SHIFT(const unsigned char gImage_dat_dest[][16],uint8_t datlen,uint8_t method,uint8_t speed,char color);

#endif

