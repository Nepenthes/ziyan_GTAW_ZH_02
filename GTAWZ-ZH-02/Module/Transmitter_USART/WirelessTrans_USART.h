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

#define FRAME_DATS_SIZE	10		//除了协议帧数据外，传感器数据缓冲队列长度

#define FRAME_TX_SIZE	24		//发送队列整帧长度
#define FRAME_RX_SIZE	100	//接收队列整帧长度

#define FRAME_HEAD	0x7E		//帧头
#define FRAME_TAIL	0x0D		//帧尾

#define GATEWAY_ADDR	0x01		//网关地址
#define NODE_ADDR		0x02		//节点地址
#define MODULE_ADDR	0x03		//模块地址

#define CMD_DATA_TX		0x10	
#define CMD_DATA_REQ		0x20
#define CMD_SETCONFIGA	0x30
#define CMD_SETCONFIGB	0x31
#define ACK					0x30

#define GTA_GGI01		0x11	//4x4矩阵键盘
#define GTA_GGI02		0x12	//RFID
#define GTA_GGI03		0x13	//语音
#define GTA_GGS01		0x14	//传感器模拟
#define GTA_GGS02		0x15	//光照强度
#define GTA_GGS03		0x16	//CO2含量
#define GTA_GGS04		0x17	//红外热释电
#define GTA_GGS05		0x18	//雨水检测
#define GTA_GGS06		0x19	//温湿度
#define GTA_GGS07		0x1A	//大气压强

#define GTA_GG001		0x1B	/*风速传感器*///继电器
#define GTA_GID01		0x20	//双色点阵
#define GTA_GID02		0x21	//4.3寸触摸LCD
#define GTA_GID03		0x10	/*土壤水分*///1.44寸LCD
#define GTA_PB002		0x30	//电动窗帘控制
#define GTA_PB003		0x31	//喷雾控制
#define GTA_PB004		0x32	//排风控制
#define GTA_PB005		0x33	//空气加热控制
#define GTA_PB006		0x34	//生长灯控制
#define GTA_PBP01		0x35	//电源自动切换控制

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
