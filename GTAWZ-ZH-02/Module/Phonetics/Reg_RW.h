
// Reg_RW.h  ��дLD3320оƬ�ļĴ������������ַ�����
#ifndef REG_RW_H
#define REG_RW_H

#define osObjectsPublic                     // define objects in main module

#define uint8 unsigned char
#define uint16 unsigned int
#define uint32 unsigned long

/*************�˿���Ϣ********************
 * ����˵��
 * RST      PB6
 * CS   	  PB8    --PB12
 * WR/SPIS  PB9
 * P2/SDCK  PB3
 * P1/SDO   PB4
 * P0/SDI   PB5
 * IRQ      PC1
 * CLK  �뵥Ƭ���ľ�����

*****************************************/
#define LD_RST_H() GPIO_SetBits(GPIOC, GPIO_Pin_13)
#define LD_RST_L() GPIO_ResetBits(GPIOC, GPIO_Pin_13)

#define LD_CS_H()	GPIO_SetBits(GPIOA, GPIO_Pin_4)
#define LD_CS_L()	GPIO_ResetBits(GPIOA, GPIO_Pin_4)

#define LD_SPIS_H()  GPIO_SetBits(GPIOC, GPIO_Pin_6)
#define LD_SPIS_L()  GPIO_ResetBits(GPIOC, GPIO_Pin_6)



/****************************************************************
�������� LD_WriteReg
���ܣ�дLD3320оƬ�ļĴ���
������  address, 8λ�޷�����������ַ
		dataout��8λ�޷���������Ҫд�������
����ֵ����
****************************************************************/ 
void LD_WriteReg( unsigned char address, unsigned char dataout );

/****************************************************************
�������� LD_ReadReg
���ܣ���LD3320оƬ�ļĴ���
������  address, 8λ�޷�����������ַ
����ֵ��8λ�޷�����������ȡ�Ľ��
****************************************************************/ 
unsigned char LD_ReadReg( unsigned char address );



#endif
