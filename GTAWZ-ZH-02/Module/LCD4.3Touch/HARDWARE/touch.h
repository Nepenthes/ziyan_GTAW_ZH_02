#ifndef __TOUCH_H__
#define __TOUCH_H__
#include "string.h"
#include "lcd5510.h"
#include "delay.h"
#include "stdlib.h"
#include "math.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//触摸屏驱动（支持ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A/GT9147等） 代码
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/3/11
//版本：V2.1
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//修改说明
//V2.0 20140311
//增加对电容触摸屏的支持(需要添加:ctiic.c和ott2001a.c两个文件)
//V2.1 20141028
//电容触摸屏增加GT9147的支持
//////////////////////////////////////////////////////////////////////////////////

#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//读取按键0
#define KEY1  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_3)//读取按键1
#define KEY2  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_2)//读取按键2 
#define KEY3  GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//读取按键3(WK_UP) 

#define KEY_UP_TP 	4
#define KEY_LEFT_TP	3
#define KEY_DOWN_TP	2
#define KEY_RIGHT_TP	1

void KEY_Init(void);//IO初始化
u8 KEY_Scan(u8);  	//按键扫描函数		

#define TP_PRES_DOWN 0x80  //触屏被按下	  
#define TP_CATH_PRES 0x40  //有按键按下了 
#define CT_MAX_TOUCH  5    //电容屏支持的点数,固定为5点

//IO方向设置
#define CT_SDA_IN()  {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0X80000000;}
#define CT_SDA_OUT() {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=0X30000000;}

//IO操作函数
#define CT_IIC_SCL    PBout(6) 			//SCL     
#define CT_IIC_SDA    PBout(7) 			//SDA	 
#define CT_READ_SDA   PBin(7)  			//输入SDA 

//IO操作函数
#define OTT_RST    		PBout(2)	//OTT2001A复位引脚
#define OTT_INT    		PFin(10)	//OTT2001A中断引脚	

//通过OTT_SET_REG指令,可以查询到这个信息
//注意,这里的X,Y和屏幕的坐标系刚好是反的.
#define OTT_MAX_X 		2700	 	//TP X方向的最大值(竖方向)
#define OTT_MAX_Y 		1500    	//TP Y方向的最大值(横方向)

//缩放因子
#define OTT_SCAL_X		0.2963		//屏幕的 纵坐标/OTT_MAX_X		
#define OTT_SCAL_Y		0.32		//屏幕的 横坐标/OTT_MAX_Y		

//I2C读写命令
#define OTT_CMD_WR 		0XB2     	//写命令
#define OTT_CMD_RD 		0XB3		//读命令

//寄存器地址
#define OTT_GSTID_REG 	0X0000   	//OTT2001A当前检测到的触摸情况
#define OTT_TP1_REG 	0X0100  	//第一个触摸点数据地址
#define OTT_TP2_REG 	0X0500		//第二个触摸点数据地址
#define OTT_TP3_REG 	0X1000		//第三个触摸点数据地址
#define OTT_TP4_REG 	0X1400		//第四个触摸点数据地址
#define OTT_TP5_REG 	0X1800		//第五个触摸点数据地址  
#define OTT_SET_REG   	0X0900   	//分辨率设置寄存器地址
#define OTT_CTRL_REG  	0X0D00   	//传感器控制(开/关)  

//IO操作函数
#define GT_RST    		PAout(0)	//GT9147复位引脚
#define GT_INT    		PCin(4)	//GT9147中断引脚	

//I2C读写命令
#define GT_CMD_WR 		0X28     	//写命令
#define GT_CMD_RD 		0X29		//读命令

//GT9147 部分寄存器定义
#define GT_CTRL_REG 	0X8040   	//GT9147控制寄存器
#define GT_CFGS_REG 	0X8047   	//GT9147配置起始地址寄存器
#define GT_CHECK_REG 	0X80FF   	//GT9147校验和寄存器
#define GT_PID_REG 		0X8140   	//GT9147产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//GT9147当前检测到的触摸情况
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址   

//IO方向设置
#define SDA_IN()  {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=8<<12;}
#define SDA_OUT() {GPIOB->CRH&=0XFFFF0FFF;GPIOB->CRH|=3<<12;}

//IO操作函数
#define IIC_SCL    PBout(10) //SCL
#define IIC_SDA    PBout(11) //SDA	 
#define READ_SDA   PBin(11)  //输入SDA 

#define AT24C01		127
#define AT24C02		255
#define AT24C04		511
#define AT24C08		1023
#define AT24C16		2047
#define AT24C32		4095
#define AT24C64	    8191
#define AT24C128	16383
#define AT24C256	32767
//Mini STM32开发板使用的是24c02，所以定义EE_TYPE为AT24C02
#define EE_TYPE AT24C02

//触摸屏控制器
typedef struct
{
    u8 (*init)(void);			//初始化触摸屏控制器
    u8 (*scan)(u8);				//扫描触摸屏.0,屏幕扫描;1,物理坐标;
    void (*adjust)(void);		//触摸屏校准
    u16 x[CT_MAX_TOUCH]; 		//当前坐标
    u16 y[CT_MAX_TOUCH];		//电容屏有最多5组坐标,电阻屏则用x[0],y[0]代表:此次扫描时,触屏的坐标,用
    //x[4],y[4]存储第一次按下时的坐标.
    u8  sta;					//笔的状态
    //b7:按下1/松开0;
    //b6:0,没有按键按下;1,有按键按下.
    //b5:保留
    //b4~b0:电容触摸屏按下的点数(0,表示未按下,1表示按下)
/////////////////////触摸屏校准参数(电容屏不需要校准)//////////////////////
    float xfac;
    float yfac;
    short xoff;
    short yoff;
//新增的参数,当触摸屏的左右上下完全颠倒时需要用到.
//b0:0,竖屏(适合左右为X坐标,上下为Y坐标的TP)
//   1,横屏(适合左右为Y坐标,上下为X坐标的TP)
//b1~6:保留.
//b7:0,电阻屏
//   1,电容屏
    u8 touchtype;
} _m_tp_dev;

extern _m_tp_dev tp_dev;	 	//触屏控制器在touch.c里面定义

//芯片连接引脚
#define PEN  		PFin(10)  	//PF10 INT
#define DOUT 		PFin(8)   	//PF8  MISO
#define TDIN 		PFout(9)  	//PF9  MOSI
#define TCLK 		PBout(1)  	//PB1  SCLK
#define TCS  		PBout(2)  	//PB2  CS 

//IIC所有操作函数
void IIC_Init(void);                //初始化IIC的IO口
void IIC_Start(void);				//发送IIC开始信号
void IIC_Stop(void);	  			//发送IIC停止信号
void IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC_Wait_Ack(void); 				//IIC等待ACK信号
void IIC_Ack(void);					//IIC发送ACK信号
void IIC_NAck(void);				//IIC不发送ACK信号

void IIC_Write_One_Byte(u8 daddr,u8 addr,u8 data);
u8 IIC_Read_One_Byte(u8 daddr,u8 addr);

u8 AT24CXX_ReadOneByte(u16 ReadAddr);							//指定地址读取一个字节
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite);		//指定地址写入一个字节
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len);//指定地址开始写入指定长度的数据
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len);					//指定地址开始读取指定长度数据
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite);	//从指定地址开始写入指定长度的数据
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead);   	//从指定地址开始读出指定长度的数据

u8 AT24CXX_Check(void);  //检查器件
void AT24CXX_Init(void); //初始化IIC

//IIC所有操作函数
void CT_IIC_Init(void);                	//初始化IIC的IO口
void CT_IIC_Start(void);				//发送IIC开始信号
void CT_IIC_Stop(void);	  				//发送IIC停止信号
void CT_IIC_Send_Byte(u8 txd);			//IIC发送一个字节
u8 CT_IIC_Read_Byte(unsigned char ack);	//IIC读取一个字节
u8 CT_IIC_Wait_Ack(void); 				//IIC等待ACK信号
void CT_IIC_Ack(void);					//IIC发送ACK信号
void CT_IIC_NAck(void);					//IIC不发送ACK信号

u8 OTT2001A_WR_Reg(u16 reg,u8 *buf,u8 len);		//写寄存器(实际无用)
void OTT2001A_RD_Reg(u16 reg,u8 *buf,u8 len);	//读寄存器
void OTT2001A_SensorControl(u8 cmd);//传感器打开/关闭操作
u8 OTT2001A_Init(void); 			//4.3电容触摸屏始化函数
u8 OTT2001A_Scan(u8 mode);			//电容触摸屏扫描函数

u8 GT9147_Send_Cfg(u8 mode);
u8 GT9147_WR_Reg(u16 reg,u8 *buf,u8 len);
void GT9147_RD_Reg(u16 reg,u8 *buf,u8 len);
u8 GT9147_Init(void);
u8 GT9147_Scan(u8 mode);

//电阻屏函数
void TP_Write_Byte(u8 num);						//向控制芯片写入一个数据
u16 TP_Read_AD(u8 CMD);							//读取AD转换值
u16 TP_Read_XOY(u8 xy);							//带滤波的坐标读取(X/Y)
u8 TP_Read_XY(u16 *x,u16 *y);					//双方向读取(X+Y)
u8 TP_Read_XY2(u16 *x,u16 *y);					//带加强滤波的双方向坐标读取
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color);//画一个坐标校准点
void TP_Draw_Big_Point(u16 x,u16 y,u16 color);	//画一个大点
void TP_Save_Adjdata(void);						//保存校准参数
u8 TP_Get_Adjdata(void);						//读取校准参数
void TP_Adjust(void);							//触摸屏校准
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac);//显示校准信息
//电阻屏/电容屏 共用函数
u8 TP_Scan(u8 tp);								//扫描
u8 TP_Init(void);								//初始化

#endif

















