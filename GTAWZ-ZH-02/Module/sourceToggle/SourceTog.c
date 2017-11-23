#include "SourceTog.h"

float valVoltage= 11.01;  //��ʼ������11.0  ���������Լ�  ��ʼ��Ϊ����Դ
uint8_t SOURCE_TYPE = 1;  //��ʼ��Ϊ����Դ

uint8_t USRKpowTX_FLG = 0;
uint8_t USRKpowexaRX_FLG = 0;

osThreadId tid_SouTogCM_Thread;
osThreadId tid_PowDetect_Thread;
osThreadDef(SouTogCM_Thread,osPriorityNormal,1,512);
osThreadDef(PowDetect_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;								//�豸�����⴮��һ�豸����

void  SourceTog_Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1	, ENABLE );	  //ʹ��ADC1ͨ��ʱ��
 
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //����ADC��Ƶ����6 72M/6=12,ADC���ʱ�䲻�ܳ���14M

	//PC0 1 ��Ϊģ��ͨ����������  ADC12_IN8                       

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//ģ����������
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);  //��λADC1,������ ADC1 ��ȫ���Ĵ�������Ϊȱʡֵ

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC����ģʽ:ADC1��ADC2�����ڶ���ģʽ
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//ģ��ת�������ڵ�ͨ��ģʽ
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//ģ��ת�������ڵ���ת��ģʽ
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ת��������������ⲿ��������
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC�����Ҷ���
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//˳����й���ת����ADCͨ������Ŀ
	ADC_Init(ADC1, &ADC_InitStructure);	//����ADC_InitStruct��ָ���Ĳ�����ʼ������ADCx�ļĴ���   

  
	ADC_Cmd(ADC1, ENABLE);	//ʹ��ָ����ADC1
	
	ADC_ResetCalibration(ADC1);	//ʹ�ܸ�λУ׼  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//�ȴ���λУ׼����
	
	ADC_StartCalibration(ADC1);	 //����ADУ׼
 
	while(ADC_GetCalibrationStatus(ADC1));	 //�ȴ�У׼����
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������

}			

void PWSwitch_Init(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

//���ADCֵ
//ch:ͨ��ֵ 0~3
uint16_t SourceTog_Get_Adc(uint8_t ch)   
{
  	//����ָ��ADC�Ĺ�����ͨ����һ�����У�����ʱ��
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADCͨ��,����ʱ��Ϊ239.5����	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//ʹ��ָ����ADC1�����ת����������	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//�ȴ�ת������

	return ADC_GetConversionValue(ADC1);	//�������һ��ADC1�������ת�����
}

uint16_t SourceTog_Get_Adc_Average(uint8_t ch,uint8_t times)
{
	u32 temp_val=0;
	uint8_t t;
	
	for(t=0;t<times;t++)
	{
		temp_val+=SourceTog_Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 

void SourceTog_Init(void){

	SourceTog_Adc_Init();
	PWSwitch_Init();
}

void PowDetect_Thread(const void *argument){
	
	uint8_t BTlow_flg = 0;
	
	for(;;){
		
		if(valVoltage < 11.00){
	
			BTlow_flg = 1;
			if(SOURCE_TYPE != 2){
				
				SOURCE_TYPE = 2;
				USRKpowTX_FLG = 1;
			}
		}else
		if(valVoltage > 11.40 && BTlow_flg){
		
			BTlow_flg = 0;
			if(SOURCE_TYPE != 1){
				
				SOURCE_TYPE = 1;
				USRKpowTX_FLG = 1;
			}
		}
		osDelay(10000);
	}
}

void SouTogCM_Thread(const void *argument){

#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif

	SOURCEMAIN_ACTIVE;
	
	for(;;){
		
		if(SOURCE_TYPE == 1){SOURCEMAIN_ACTIVE;}
		else {SOURCEBACK_ACTIVE;}
	
		valVoltage = SourceTog_Get_Adc_Average(14,8) * 0.00331; // (3.3 x 4) / 4096 = 0.00344____�ֶ�΢������ʵ����0.00331
																							 /*�����ѹ��Ϊ1��4*/
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rThe power supply voltage : %.2f V\n\r", valVoltage);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(100);
	}
}

void SourceTogCM(void){

	tid_SouTogCM_Thread = osThreadCreate(osThread(SouTogCM_Thread),NULL);
	tid_PowDetect_Thread = osThreadCreate(osThread(PowDetect_Thread),NULL);
}






















