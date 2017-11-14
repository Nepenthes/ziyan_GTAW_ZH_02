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

11.�̵���
12.˫ɫ����
13.4.3�紥��LCD
14.1.44��LCD
15.�綯��������
16.�������
17.�ŷ����
18.�������ȿ���
19.�����ƿ���
20.��Դ�Զ��л�����

**********************************************************************/
	
funMoudle *moudleTestForm[] = {
	
	&KeyBoardMS								,&RC522									,(void *)0								,&SensorSIMMS							,&TSL2561MS 							,
	(void *)0								,&RTMS									,&RAINMS									,&SHT11MS								,&BMP180MS  							,
	(void *)0								,&LEDArrayCM							,(void *)0								,(void *)0								,(void *)0								,
	&LEDSpray								,&exAir									,(void *)0								,&LEDGrow								,(void *)0								
};

funMoudle *moudleInitForm[] = {
	
	
	&keyBoard_Init							,&RC522_Init							,(void *)0								,&SensorSIM_Init						,&TSL2561_Init							,
	(void *)0								,&RT_Init								,&RAIN_Init								,&SHT11_Init							,&BMP180_Init			   			,
	(void *)0								,&LEDArray_Init						,(void *)0								,(void *)0								,(void *)0								,
	&LEDSpray_Init							,&exAir_Init							,(void *)0								,&LEDGrow_Init							,(void *)0								
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

void BSP_Test(void){

	//LEDTest		();	Ӳ����֧��
	USART1Debug();
	USART2Trans();
	keyTest ();
	LCD_144_test();
	
	moudleTestForm[MOUDLE_ID-1]();
}
