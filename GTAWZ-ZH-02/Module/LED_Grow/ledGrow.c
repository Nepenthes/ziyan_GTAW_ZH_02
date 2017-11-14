#include "ledGrow.h"

uint16_t PWM_ledGRW=3000;//pwm

osThreadId tid_LEDGrowCM_Thread;
osThreadId tid_LEDGrowCM_DB_Thread;
osThreadDef(LEDGrowCM_Thread,osPriorityNormal,1,256);
osThreadDef(LEDGrowCM_DB_Thread,osPriorityNormal,1,256);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

//按键初始化函数
void KEY_Init_ledGRW(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	//初始化KEY0-->GPIOA.13,KEY1-->GPIOA.15  上拉输入
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);//使能PORTA,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;//PE2~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4

	//初始化 WK_UP-->GPIOA.0	  下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOA.0
}

void TIM4_PWM_Init_ledGRW(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD| RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); 
	//设置该引脚为复用输出功能,输出TIM3 CH2的PWM脉冲波形	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO
//PB9  TIM4-CH4

   //初始化TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
	
	//初始化TIM3 Channel2 PWM模式	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //选择定时器模式:TIM脉冲宽度调制模式2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //输出极性:TIM输出比较极性高
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 
	TIM_Cmd(TIM4, ENABLE);  //使能TIM4		 
}

u8 KEY_Scan1_ledGRW(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY3_ledGRW==0||KEY4_ledGRW==0||KEY5_ledGRW==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY3_ledGRW==0)return 3;
		else if(KEY4_ledGRW==0)return 4;
		else if(KEY5_ledGRW==0)return 5;
	}else if(((KEY3_ledGRW==1)&&(KEY4_ledGRW==1)&&(KEY5_ledGRW==1))==1)key_up=1; 	    
 	return 0;// 无按键按下
}

void LEDGrow_Init(void){

	BEEP_Init();
	KEY_Init_ledGRW();
}

void led_Grow(void){

	u8 t=0,cnt=0,key_in;
	u8 key_num=0;
	
	TIM4_PWM_Init_ledGRW(PWM_ledGRW,10);	//PWM频率=72000000/900=80Khz   ARR=900 
	
	Beep_time(200);
	TIM_SetCompare3(TIM4,0);	
	
	while(1)
	{
		key_in=KEY_Scan1_ledGRW(0);
		if(key_in==5)
		{
		key_num++;
		if(key_num%2==1)	
			{	
				TIM_SetCompare3(TIM4,PWM_ledGRW/2);		
			}
		else
			{
				TIM_SetCompare3(TIM4,0);	
				PWM_ledGRW = 0;	
			}
		Beep_time(100);
		}	
		else if(key_in==3)
		{
			if(key_num%2==1)	
				{			
					if(PWM_ledGRW<5500)
						PWM_ledGRW=PWM_ledGRW+1000;
					
						TIM_SetCompare3(TIM4,PWM_ledGRW/2);
						Beep_time(100);
				}
		}			
		else if(key_in==4)
		{
			if(key_num%2==1)	
				{			
					if(PWM_ledGRW>500)
					PWM_ledGRW=PWM_ledGRW-500;
					
					TIM_SetCompare3(TIM4,PWM_ledGRW/2);
					Beep_time(100);
				}
		}

		TIM_SetCompare3(TIM4,PWM_ledGRW/2);
		
		t++; 
		delay_ms(10);
		if(t==100)
		{
			t=0;
			cnt++;	
		}		   	
	}	 	 
}

void LEDGrowCM_Thread(const void *argument){
	
	for(;;)led_Grow();
}

void LEDGrowCM_DB_Thread(const void *argument){
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	uint16_t PWM_ledGRW_DB = PWM_ledGRW / 55;
	
	for(;;){
		
		PWM_ledGRW_DB = PWM_ledGRW / 55;
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rLED_Grow pwm is : %d%\n\r", &PWM_ledGRW_DB);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(1000);
	}
}

void LEDGrow(void){

	tid_LEDGrowCM_Thread = osThreadCreate(osThread(LEDGrowCM_Thread),NULL);
	tid_LEDGrowCM_DB_Thread = osThreadCreate(osThread(LEDGrowCM_DB_Thread),NULL);
}
