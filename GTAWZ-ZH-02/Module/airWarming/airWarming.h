#ifndef __AIRWARMING_H
#define __AIRWARMING_H
#include "IO_Map.h"
#include "delay.h"
#include "stm32f10x.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "beep.h"

#define swKey_airWarming  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_1)

#define CLR_DS18B20()      PDout(14) = 0
#define SET_DS18B20()      PDout(14) = 1 
#define DS18B20_DQ_IN   	PDin(14) 

#define WARMING_ON			PDout(15) = 1
#define WARMING_OFF			PDout(15) = 0

uint8_t DS18B20_Init(void);          //初始化DS18B20
float DS18B20_Get_Temp(void);   //获取温度
void DS18B20_Start(void);       //开始温度转换
void DS18B20_Write_Byte(uint8_t dat);//写入一个字节
uint8_t DS18B20_Read_Byte(void);     //读出一个字节
uint8_t DS18B20_Read_Bit(void);      //读出一个位
uint8_t DS18B20_Check(void);         //检测是否存在DS18b20
void DS18B20_Rst(void);         //复位DS18B20   

u8 KEY_Scan1_AWM(u8 mode);

void airWarming_Init(void);
void airWarmingCM_Thread(const void *argument);
void airWarming(void);

#endif
