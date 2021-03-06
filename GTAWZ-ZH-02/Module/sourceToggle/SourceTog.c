#include "SourceTog.h"

float valVoltage= 11.01;  //初始化大于11.0  方便跳过首次自检  初始化为主电源
uint8_t SOURCE_TYPE = 1;  //初始化为主电源

uint8_t USRKpowTX_FLG = 0;
uint8_t USRKpowexaRX_FLG = 0;

osThreadId tid_SouTogCM_Thread;
osThreadId tid_PowDetect_Thread;
osThreadDef(SouTogCM_Thread,osPriorityNormal,1,512);
osThreadDef(PowDetect_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void  SourceTog_Adc_Init(void)
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

//获得ADC值
//ch:通道值 0~3
uint16_t SourceTog_Get_Adc(uint8_t ch)   
{
  	//设置指定ADC的规则组通道，一个序列，采样时间
	ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_239Cycles5 );	//ADC1,ADC通道,采样时间为239.5周期	  			    
  
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);		//使能指定的ADC1的软件转换启动功能	
	 
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC ));//等待转换结束

	return ADC_GetConversionValue(ADC1);	//返回最近一次ADC1规则组的转换结果
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
		
		if(valVoltage < 8.0){     //低电识别
	
			BTlow_flg = 1;
			if(SOURCE_TYPE != 2){
				
				SOURCE_TYPE = 2;
				USRKpowTX_FLG = 1;
			}
		}else
		if(valVoltage > 8.5 && BTlow_flg){	//低电后再次切回高电识别条件
		
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
	
		valVoltage = SourceTog_Get_Adc_Average(14,8) * 0.00331; // (3.3 x 4) / 4096 = 0.00344____手动微调消除实际误差到0.00331
																							 /*电阻分压比为1：4*/
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






















