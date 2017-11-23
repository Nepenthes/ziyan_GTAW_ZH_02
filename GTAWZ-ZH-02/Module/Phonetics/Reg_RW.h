
// Reg_RW.h  读写LD3320芯片的寄存器，共有四种方法。
#ifndef REG_RW_H
#define REG_RW_H

#define osObjectsPublic                     // define objects in main module

#define uint8 unsigned char
#define uint16 unsigned int
#define uint32 unsigned long

/*************端口信息********************
 * 接线说明
 * RST      PB6
 * CS   	  PB8    --PB12
 * WR/SPIS  PB9
 * P2/SDCK  PB3
 * P1/SDO   PB4
 * P0/SDI   PB5
 * IRQ      PC1
 * CLK  与单片机的晶振共用

*****************************************/
#define LD_RST_H() GPIO_SetBits(GPIOC, GPIO_Pin_13)
#define LD_RST_L() GPIO_ResetBits(GPIOC, GPIO_Pin_13)

#define LD_CS_H()	GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define LD_CS_L()	GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define LD_SPIS_H()  GPIO_SetBits(GPIOC, GPIO_Pin_6)
#define LD_SPIS_L()  GPIO_ResetBits(GPIOC, GPIO_Pin_6)



/****************************************************************
函数名： LD_WriteReg
功能：写LD3320芯片的寄存器
参数：  address, 8位无符号整数，地址
		dataout，8位无符号整数，要写入的数据
返回值：无
****************************************************************/ 
void LD_WriteReg( unsigned char address, unsigned char dataout );

/****************************************************************
函数名： LD_ReadReg
功能：读LD3320芯片的寄存器
参数：  address, 8位无符号整数，地址
返回值：8位无符号整数，读取的结果
****************************************************************/ 
unsigned char LD_ReadReg( unsigned char address );



#endif
