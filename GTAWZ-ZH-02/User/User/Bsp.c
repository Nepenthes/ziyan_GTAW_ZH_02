#include <Bsp.h>
/**********************************************************************
1.	4x4�������
2.	RFID
3.	����
4.	������ģ��
5.	����ǿ��
6.	CO2����
7.	�������͵�
8.	��ˮ���
9.	��ʪ��
10.����ѹǿ
11.����

12.˫ɫ����
13.4.3�紥��LCD
14.����ʪ�ȼ��
15.�綯��������
16.�������
17.�ŷ����
18.�������ȿ���
19.�����ƿ���
20.��Դ�Զ��л�����

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

	//LEDTest		();	Ӳ����֧��
	USART1Debug();
	USART2Trans();
	keyTest ();
	LCD_144_test();
	
	moudleTestForm[MOUDLE_ID-1]();
}
