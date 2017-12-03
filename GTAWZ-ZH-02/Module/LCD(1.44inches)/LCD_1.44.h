#ifndef __LCD_144_H
#define __LCD_144_H	

#define osObjectsPublic                     // define objects in main module

#include "stdlib.h"
#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "GUI_1.44.h"
#include "MoudleDats.h"
#include "stdio.h"
#include "Driver_USART.h"
#include "string.h"

#include "Bsp.h"

#include "delay.h"

//LCD��Ҫ������
typedef struct  
{										    
	uint16_t width;			//LCD ���
	uint16_t height;			//LCD �߶�
	uint16_t id;				//LCD ID
	uint8_t  dir;			//���������������ƣ�0��������1��������	
	uint16_t	 wramcmd;		//��ʼдgramָ��
	uint16_t  setxcmd;		//����x����ָ��
	uint16_t  setycmd;		//����y����ָ��	 
}_lcd144_dev; 	

//LCD����
extern _lcd144_dev lcd144dev;	//����LCD��Ҫ����
/////////////////////////////////////�û�������///////////////////////////////////	 
//֧�ֺ��������ٶ����л���֧��8/16λģʽ�л�
#define USE_HORIZONTAL  	0	//�����Ƿ�ʹ�ú��� 		0,��ʹ��.1,ʹ��.
//ʹ��Ӳ��SPI ����ģ��SPI��Ϊ����
#define USE_HARDWARE_SPI  1  //1:Enable Hardware SPI;0:USE Soft SPI
//////////////////////////////////////////////////////////////////////////////////	  
//����LCD�ĳߴ�
#if USE_HORIZONTAL==1	//ʹ�ú���
#define LCD_1_44_W 320
#define LCD_1_44_H 240
#else
#define LCD_1_44_W 240
#define LCD_1_44_H 320
#endif

//TFTLCD������Ҫ���õĺ���		   
extern uint16_t  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern uint16_t  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ

////////////////////////////////////////////////////////////////////
//-----------------LCD�˿ڶ���---------------- 
//QDtechȫϵ��ģ������������ܿ��Ʊ��������û�Ҳ���Խ�PWM���ڱ�������
//#define LCD_1_44_LED        	GPIO_Pin_9  //PB9 ������TFT -LED
//�ӿڶ�����Lcd_Driver.h�ڶ��壬����ݽ����޸Ĳ��޸���ӦIO��ʼ��LCD_1_44_GPIO_Init()

//#define LCD_1_44_CTRL   	  	GPIOB		//����TFT���ݶ˿�
//#define LCD_1_44_RS         	GPIO_Pin_10	//PB10������TFT --RS
//#define LCD_1_44_CS        	GPIO_Pin_11 //PB11 ������TFT --CS
//#define LCD_1_44_RST     	GPIO_Pin_12	//PB12������TFT --RST
//#define LCD_1_44_SCL        	GPIO_Pin_13	//PB13������TFT -- CLK
//#define LCD_1_44_SDA        	GPIO_Pin_15	//PB15������TFT - SDI
#define LCD_1_44_CTRL   	  	GPIOB		//����TFT���ݶ˿�
#define LCD_1_44_CTRL2   	  	GPIOE		//����TFT���ݶ˿�

//#define LCD_1_44_LED        	GPIO_Pin_9  //MCU_PB9--->>TFT --BL
#define LCD_1_44_RS         	GPIO_Pin_5	//PB11--->>TFT --RS/DC
#define LCD_1_44_CS        	GPIO_Pin_12  //MCU_PB11--->>TFT --CS/CE
#define LCD_1_44_RST     		GPIO_Pin_6	//PB10--->>TFT --RST
#define LCD_1_44_SCL        	GPIO_Pin_13	//PB13--->>TFT --SCL/SCK
#define LCD_1_44_SDA        	GPIO_Pin_15	//PB15 MOSI--->>TFT --SDA/DIN

							    
//////////////////////////////////////////////////////////////////////
//Һ�����ƿ���1�������궨��
#define	LCD_1_44_CS_SET  	LCD_1_44_CTRL->BSRR=LCD_1_44_CS    
#define	LCD_1_44_RS_SET  	LCD_1_44_CTRL2->BSRR=LCD_1_44_RS    
#define	LCD_1_44_SDA_SET  	LCD_1_44_CTRL->BSRR=LCD_1_44_SDA    
#define	LCD_1_44_SCL_SET  	LCD_1_44_CTRL->BSRR=LCD_1_44_SCL    
#define	LCD_1_44_RST_SET  	LCD_1_44_CTRL2->BSRR=LCD_1_44_RST    
//#define	LCD_1_44_LED_SET  	LCD_1_44_CTRL->BSRR=LCD_1_44_LED   

//Һ�����ƿ���0�������궨��
#define	LCD_1_44_CS_CLR  	LCD_1_44_CTRL->BRR=LCD_1_44_CS    
#define	LCD_1_44_RS_CLR  	LCD_1_44_CTRL2->BRR=LCD_1_44_RS    
#define	LCD_1_44_SDA_CLR  	LCD_1_44_CTRL->BRR=LCD_1_44_SDA    
#define	LCD_1_44_SCL_CLR  	LCD_1_44_CTRL->BRR=LCD_1_44_SCL    
#define	LCD_1_44_RST_CLR  	LCD_1_44_CTRL2->BRR=LCD_1_44_RST    
//#define	LCD_1_44_LED_CLR  	LCD_1_44_CTRL->BRR=LCD_1_44_LED 

//������ɫ
#define WHITE       0xFFFF
#define BLACK      	0x0000	  
#define BLUE       	0x001F  
#define BRED        0XF81F
#define GRED 			 	0XFFE0
#define GBLUE			 	0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 			0XBC40 //��ɫ
#define BRRED 			0XFC07 //�غ�ɫ
#define GRAY  			0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	0X841F //ǳ��ɫ
//#define LIGHTGRAY     0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 		0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE      	0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE          0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)
	    															  
extern uint16_t BACK_COLOR, POINT_COLOR ;  

void LCD144_Init(void);
void LCD_1_44_DisplayOn(void);
void LCD_1_44_DisplayOff(void);
void LCD_1_44_Clear(uint16_t Color);	
void LCD_1_44_ClearS(uint16_t Color,uint16_t x,uint16_t y,uint16_t xx,uint16_t yy);
void LCD_1_44_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_1_44_DrawPoint(uint16_t x,uint16_t y);//����
uint16_t  LCD_1_44_ReadPoint(uint16_t x,uint16_t y); //����
void LCD_1_44_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_1_44_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   
void LCD_1_44_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd);
void LCD_1_44_DrawPoint_16Bit(uint16_t color);
uint16_t LCD_1_44_RD_DATA(void);//��ȡLCD����									    
void LCD_1_44_WriteReg(uint16_t LCD_1_44_Reg, uint16_t LCD_1_44_RegValue);
void LCD_1_44_WR_DATA(uint8_t data);
void LCD_1_44_WR_DATA_16Bit(uint16_t data);
uint16_t LCD_1_44_ReadReg(uint8_t LCD_1_44_Reg);
void LCD_1_44_WriteRAM_Prepare(void);
void LCD_1_44_WriteRAM(uint16_t RGB_Code);
uint16_t LCD_1_44_ReadRAM(void);		   
uint16_t LCD_1_44_BGR2RGB(uint16_t c);
void LCD_1_44_SetParam(void);

void LCD144Test_Thread(const void *argument);
void LCD144_Thread(const void *argument);

void LCD144_test(void);
						  		 
#endif  
	 
	 



