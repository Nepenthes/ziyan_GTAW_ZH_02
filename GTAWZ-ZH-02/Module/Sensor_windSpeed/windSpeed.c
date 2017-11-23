#include "windSpeed.h"
#include "delay.h"

double valwindSpeed;

osThreadId tid_windSpeedMS_Thread;
osThreadDef(windSpeedMS_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void  windSpeed_Adc_Init(void)
{ 	
	ADC_InitTypeDef ADC_InitStructure; 
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_ADC1	, ENABLE );	  //使能ADC1通道时钟
 

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);   //设置ADC分频因子6 72M/6=12,ADC最大时间不能超过14M


	//PC0 1 作为模拟通道输入引脚  ADC12_IN8                       
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;		//模拟输入引脚
	GPIO_Init(GPIOC, &GPIO_InitStructure);	
	
	ADC_DeInit(ADC1);  //复位ADC1,将外设 ADC1 的全部寄存器重设为缺省值

	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;	//ADC工作模式:ADC1和ADC2工作在独立模式
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;	//模数转换工作在单通道模式
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;	//模数转换工作在单次转换模式
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//转换由软件而不是外部触发启动
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;	//ADC数据右对齐
	ADC_InitStructure.ADC_NbrOfChannel = 1;	//顺序进行规则转换的ADC通道的数目
	ADC_Init(ADC1, &ADC_InitStructure);	//根据ADC_InitStruct中指定的参数初始化外设ADCx的寄存器   

  
	ADC_Cmd(ADC1, ENABLE);	//使能指定的ADC1
	
	ADC_ResetCalibration(ADC1);	//使能复位校准  
	 
	while(ADC_GetResetCalibrationStatus(ADC1));	//等待复位校准结束
	
	ADC_StartCalibration(ADC1);	 //开启AD校准
 
	while(ADC_GetCalibrationStatus(ADC1));	 //等待校准结束
 
//	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能

}				  
//获得ADC值
//ch:通道值 0~3
uint16_t windSpeed_Get_Adc(uint8_t ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
}

uint16_t windSpeed_Get_Adc_Average(uint8_t ch,uint8_t times)
{
	u32 temp_val=0;
	uint8_t t;
	
	for(t=0;t<times;t++)
	{
		temp_val+=windSpeed_Get_Adc(ch);
		delay_ms(5);
	}
	return temp_val/times;
} 	 

void windSpeed_Init(void){

	windSpeed_Adc_Init();
}

void windSpeedMS_Thread(const void *argument){

	uint16_t temp_windSpeed;
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	for(;;){
	
		temp_windSpeed = windSpeed_Get_Adc_Average(12,8);
		
		if(temp_windSpeed > 820){
		
			temp_windSpeed -= 820;
			valwindSpeed =  0.0091575 * (double)temp_windSpeed;
		} else valwindSpeed = 0.0;
		
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rwindSpeed is : %.3f m/s\n\r", valwindSpeed);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(100);
	}
}

void windSpeedMS(void){

	tid_windSpeedMS_Thread = osThreadCreate(osThread(windSpeedMS_Thread),NULL);
}






