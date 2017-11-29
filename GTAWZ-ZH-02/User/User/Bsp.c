#include <Bsp.h>
/**********************************************************************
1.	4x4矩阵键盘
2.	RFID
3.	语音
4.	传感器模拟
5.	光照强度
6.	CO2含量
7.	红外热释电
8.	雨水检测
9.	温湿度
10.大气压强
11.风速

12.双色点阵
13.4.3寸触摸LCD
14.土壤湿度检测
15.电动窗帘控制
16.喷雾控制
17.排风控制
18.空气加热控制
19.生长灯控制
20.电源自动切换控制

**********************************************************************/
	
funMoudle *moudleTestForm[] = {
	
	&KeyBoardMS					,&RC522						,&Phonetics					,&SensorSIMMS				,&TSL2561MS 			,
	&contentCO2MS				,&RTMS						,&RAINMS						,&SHT11MS					,&BMP180MS  			,
	&windSpeedMS				,&LEDArrayCM				,(void *)0					,&soilHumMS					,&Curtain				,
	&LEDSpray					,&exAir						,&airWarming				,&LEDGrow					,&SourceTogCM			,
	(void *)0					,&USART4ElecMeter
};

funMoudle *moudleInitForm[] = {
	
	
	&keyBoard_Init				,&RC522_Init				,&Phonetics_Init			,&SensorSIM_Init			,&TSL2561_Init			,
	&contentCO2_Init			,&RT_Init					,&RAIN_Init					,&SHT11_Init				,&BMP180_Init			,
	&windSpeed_Init			,&LEDArray_Init			,(void *)0					,&soilHum_Init				,&Curtain_Init			,
	&LEDSpray_Init				,&exAir_Init				,&airWarming_Init			,&LEDGrow_Init				,&SourceTog_Init		,
	(void *)0					,&USART4ElecMeter_Init	
};

void  BSP_Init(void){
	
	bsp_delayinit	();
	testInit			();
	USART1Init1		();
	USART1Init2		();
	USART2Init1		();
	USART2Init2		();
	keyInit			();
	BEEP_Init		();
	LCD144_Init		();
	//Beep_time(500);
	
	mutexUSART1_Active();
	
	moudleInitForm[MOUDLE_ID-1]();
}

void BSP_SetOFF(void){

	//LEDTest		();	硬件不支持
	USART1Debug();
	USART2Trans();
	keyTest ();
	LCD_144_test();
	
	moudleTestForm[MOUDLE_ID-1]();
}
