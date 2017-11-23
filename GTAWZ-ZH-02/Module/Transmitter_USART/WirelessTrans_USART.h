#ifndef WIRELESSTRABS_USART2_H
#define WIRELESSTRABS_USART2_H

#define osObjectsPublic                     // define objects in main module

#include "stm32f10x.h"
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_USART.h"
#include "string.h"
#include "math.h"

#include "LED_Array.h"

#include "USART_STM32F10x.h"

#include <MoudleDats.h>

#define FRAME_DATS_SIZE	10		//����Э��֡�����⣬���������ݻ�����г���

#define FRAME_TX_SIZE	24		//���Ͷ�����֡����
#define FRAME_RX_SIZE	100	//���ն�����֡����

#define FRAME_HEAD	0x7E		//֡ͷ
#define FRAME_TAIL	0x0D		//֡β

#define GATEWAY_ADDR	0x01		//���ص�ַ
#define NODE_ADDR		0x02		//�ڵ��ַ
#define MODULE_ADDR	0x03		//ģ���ַ

#define CMD_DATA_TX		0x10	
#define CMD_DATA_REQ		0x20
#define CMD_SETCONFIGA	0x30
#define CMD_SETCONFIGB	0x31
#define ACK					0x30

#define GTA_GGI01		0x11	//4x4�������
#define GTA_GGI02		0x12	//RFID
#define GTA_GGI03		0x13	//����
#define GTA_GGS01		0x14	//������ģ��
#define GTA_GGS02		0x15	//����ǿ��
#define GTA_GGS03		0x16	//CO2����
#define GTA_GGS04		0x17	//�������͵�
#define GTA_GGS05		0x18	//��ˮ���
#define GTA_GGS06		0x19	//��ʪ��
#define GTA_GGS07		0x1A	//����ѹǿ

#define GTA_GG001		0x1B	/*���ٴ�����*///�̵���
#define GTA_GID01		0x20	//˫ɫ����
#define GTA_GID02		0x21	//4.3�紥��LCD
#define GTA_GID03		0x10	/*����ˮ��*///1.44��LCD
#define GTA_PB002		0x30	//�綯��������
#define GTA_PB003		0x31	//�������
#define GTA_PB004		0x32	//�ŷ����
#define GTA_PB005		0x33	//�������ȿ���
#define GTA_PB006		0x34	//�����ƿ���
#define GTA_PBP01		0x35	//��Դ�Զ��л�����

void myUSART1_callback(uint32_t event);
void myUSART2_callback(uint32_t event);

void USART1Init1(void);
void USART1Init2(void);

void USART2Init1(void);
void USART2Init2(void);

void USART1Debug_Thread(const void *argument);
void USART2Trans_Thread(const void *argument);

void USART1Debug(void);
void USART2Trans(void);

void FRAME_TX_DATSLOAD(uint8_t dats[],uint8_t length);

#endif
