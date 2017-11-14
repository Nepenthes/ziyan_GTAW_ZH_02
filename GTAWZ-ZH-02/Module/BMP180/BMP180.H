#ifndef __BMP180_H__
#define __BMP180_H__

#define osObjectsPublic                     // define objects in main module

#include "stdio.h"
#include "string.h"

#include <MoudleDats.h>

#include "Driver_USART.h"
#include "delay.h"
#include "bmp180_iic.h"
#include "IO_Map.h"
#include "math.h"

#define	BMP180_SlaveAddress     0xee	//气压传感器器件地址

#define   OSS 0	 //	BMP085使用
#define   REG_Pressure 0x34 // oss=0 4.5


void BMP180_Init(void);//初始化IO   读取校准参数
void bmp180Convert(void);	//校准温度和气压 ，将结果分别存入 result_UP result_UT
double BMP085_Get_Altitude(float pressure);
void BMP180MS_Thread(const void *argument);
void BMP180MS(void);

#endif

