#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "string.h"
#include "lcd5510.h"
#include "delay.h"
#include "stdlib.h"
#include "math.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//������������֧��ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A/GT9147�ȣ� ����
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/3/11
//�汾��V2.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//�޸�˵��
//V2.0 20140311
//���ӶԵ��ݴ�������֧��(��Ҫ���:ctiic.c��ott2001a.c�����ļ�)
//V2.1 20141028
//���ݴ���������GT9147��֧��
//////////////////////////////////////////////////////////////////////////////////

#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//��ȡ����0
#define KEY1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//��ȡ����1
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)//��ȡ����2 
#define KEY3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//��ȡ����3(WK_UP) 

#define KEY_UP_TP 	4
#define KEY_LEFT_TP	3
#define KEY_DOWN_TP	2
#define KEY_RIGHT_TP	1

void KEY_Init(void);//IO��ʼ��
u8 KEY_Scan(u8);  	//����ɨ�躯��		

#define TP_PRES_DOWN 0x80  //����������	  
#define TP_CATH_PRES 0x40  //�а��������� 
#define CT_MAX_TOUCH  5    //������֧�ֵĵ���,�̶�Ϊ5��

//IO��������
#define CT_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0X80000000;}
#define CT_SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0X30000000;}

//IO��������
#define CT_IIC_SCL    PBout(6) 			//SCL     
#define CT_IIC_SDA    PBout(7) 			//SDA	 
#define CT_READ_SDA   PBin(7)  			//����SDA 

//IO��������
#define OTT_RST    		PBout(2)	//OTT2001A��λ����
#define OTT_INT    		PFin(10)	//OTT2001A�ж�����	

//ͨ��OTT_SET_REGָ��,���Բ�ѯ�������Ϣ
//ע��,�����X,Y����Ļ������ϵ�պ��Ƿ���.
#define OTT_MAX_X 		2700	 	//TP X��������ֵ(������)
#define OTT_MAX_Y 		1500    	//TP Y��������ֵ(�᷽��)

//��������
#define OTT_SCAL_X		0.2963		//��Ļ�� ������/OTT_MAX_X		
#define OTT_SCAL_Y		0.32		//��Ļ�� ������/OTT_MAX_Y		

//I2C��д����
#define OTT_CMD_WR 		0XB2     	//д����
#define OTT_CMD_RD 		0XB3		//������

//�Ĵ�����ַ
#define OTT_GSTID_REG 	0X0000   	//OTT2001A��ǰ��⵽�Ĵ������
#define OTT_TP1_REG 	0X0100  	//��һ�����������ݵ�ַ
#define OTT_TP2_REG 	0X0500		//�ڶ������������ݵ�ַ
#define OTT_TP3_REG 	0X1000		//���������������ݵ�ַ
#define OTT_TP4_REG 	0X1400		//���ĸ����������ݵ�ַ
#define OTT_TP5_REG 	0X1800		//��������������ݵ�ַ  
#define OTT_SET_REG   	0X0900   	//�ֱ������üĴ�����ַ
#define OTT_CTRL_REG  	0X0D00   	//����������(��/��)  

//IO��������
#define GT_RST    		PAout(0)	//GT9147��λ����
#define GT_INT    		PCin(4)	//GT9147�ж�����	

//I2C��д����
#define GT_CMD_WR 		0X28     	//д����
#define GT_CMD_RD 		0X29		//������

//GT9147 ���ּĴ�������
#define GT_CTRL_REG 	0X8040   	//GT9147���ƼĴ���
#define GT_CFGS_REG 	0X8047   	//GT9147������ʼ��ַ�Ĵ���
#define GT_CHECK_REG 	0X80FF   	//GT9147У��ͼĴ���
#define GT_PID_REG 		0X8140   	//GT9147��ƷID�Ĵ���

#define GT_GSTID_REG 	0X814E   	//GT9147��ǰ��⵽�Ĵ������
#define GT_TP1_REG 		0X8150  	//��һ�����������ݵ�ַ
#define GT_TP2_REG 		0X8158		//�ڶ������������ݵ�ַ
#define GT_TP3_REG 		0X8160		//���������������ݵ�ַ
#define GT_TP4_REG 		0X8168		//���ĸ����������ݵ�ַ
#define GT_TP5_REG 		0X8170		//��������������ݵ�ַ   

//IO��������
#define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=8<<12;}
#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}

//IO��������
#define IIC_SCL    PBout(10) //SCL
#define IIC_SDA    PBout(11) //SDA	 
#define READ_SDA   PBin(11)  //����SDA 

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767
//Mini STM32������ʹ�õ���24c02�����Զ���EE_TYPEΪAT24C02
#define EE_TYPE AT24C02

//������������
typedef struct
{
    u8 (*init)(void);			//��ʼ��������������
    u8 (*scan)(u8);				//ɨ�败����.0,��Ļɨ��;1,��������;
    void (*adjust)(void);		//������У׼
    u16 x[CT_MAX_TOUCH]; 		//��ǰ����
    u16 y[CT_MAX_TOUCH];		//�����������5������,����������x[0],y[0]����:�˴�ɨ��ʱ,����������,��
    //x[4],y[4]�洢��һ�ΰ���ʱ������.
    u8  sta;					//�ʵ�״̬
    //b7:����1/�ɿ�0;
    //b6:0,û�а�������;1,�а�������.
    //b5:����
    //b4~b0:���ݴ��������µĵ���(0,��ʾδ����,1��ʾ����)
/////////////////////������У׼����(����������ҪУ׼)//////////////////////
    float xfac;
    float yfac;
    short xoff;
    short yoff;
//�����Ĳ���,��������������������ȫ�ߵ�ʱ��Ҫ�õ�.
//b0:0,����(�ʺ�����ΪX����,����ΪY�����TP)
//   1,����(�ʺ�����ΪY����,����ΪX�����TP)
//b1~6:����.
//b7:0,������
//   1,������
    u8 touchtype;
} _m_tp_dev;

extern _m_tp_dev tp_dev;	 	//������������touch.c���涨��

//оƬ��������
#define PEN  		PFin(10)  	//PF10 INT
#define DOUT 		PFin(8)   	//PF8  MISO
#define TDIN 		PFout(9)  	//PF9  MOSI
#define TCLK 		PBout(1)  	//PB1  SCLK
#define TCS  		PBout(2)  	//PB2  CS 

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);

u8 AT24CXX_ReadOneByte(u16 ReadAddr);							//ָ����ַ��ȡһ���ֽ�
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//ָ����ַд��һ���ֽ�
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//ָ����ַ��ʼд��ָ�����ȵ�����
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					//ָ����ַ��ʼ��ȡָ����������
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	//��ָ����ַ��ʼд��ָ�����ȵ�����
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	//��ָ����ַ��ʼ����ָ�����ȵ�����

u8 AT24CXX_Check(void);  //�������
void AT24CXX_Init(void); //��ʼ��IIC

//IIC���в�������
void CT_IIC_Init(void);                	//��ʼ��IIC��IO��
void CT_IIC_Start(void);				//����IIC��ʼ�ź�
void CT_IIC_Stop(void);	  				//����IICֹͣ�ź�
void CT_IIC_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 CT_IIC_Read_Byte(unsigned char ack);	//IIC��ȡһ���ֽ�
u8 CT_IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void CT_IIC_Ack(void);					//IIC����ACK�ź�
void CT_IIC_NAck(void);					//IIC������ACK�ź�

u8 OTT2001A_WR_Reg(u16 reg,u8 *buf,u8 len);		//д�Ĵ���(ʵ������)
void OTT2001A_RD_Reg(u16 reg,u8 *buf,u8 len);	//���Ĵ���
void OTT2001A_SensorControl(u8 cmd);//��������/�رղ���
u8 OTT2001A_Init(void); 			//4.3���ݴ�����ʼ������
u8 OTT2001A_Scan(u8 mode);			//���ݴ�����ɨ�躯��

u8 GT9147_Send_Cfg(u8 mode);
u8 GT9147_WR_Reg(u16 reg,u8 *buf,u8 len);
void GT9147_RD_Reg(u16 reg,u8 *buf,u8 len);
u8 GT9147_Init(void);
u8 GT9147_Scan(u8 mode);

//����������
void TP_Write_Byte(u8 num);						//�����оƬд��һ������
u16 TP_Read_AD(u8 CMD);							//��ȡADת��ֵ
u16 TP_Read_XOY(u8 xy);							//���˲��������ȡ(X/Y)
u8 TP_Read_XY(u16 *x,u16 *y);					//˫�����ȡ(X+Y)
u8 TP_Read_XY2(u16 *x,u16 *y);					//����ǿ�˲���˫���������ȡ
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color);//��һ������У׼��
void TP_Draw_Big_Point(u16 x,u16 y,u16 color);	//��һ�����
void TP_Save_Adjdata(void);						//����У׼����
u8 TP_Get_Adjdata(void);						//��ȡУ׼����
void TP_Adjust(void);							//������У׼
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);//��ʾУ׼��Ϣ
//������/������ ���ú���
u8 TP_Scan(u8 tp);								//ɨ��
u8 TP_Init(void);								//��ʼ��

#endif

















