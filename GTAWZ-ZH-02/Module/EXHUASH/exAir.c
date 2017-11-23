#include "exAir.h"

const uint16_t PWM_exAir_Init=3000;
uint16_t PWM_exAir=0;

uint8_t USRKexaTX_FLG = 0;
uint8_t USRKexaRX_FLG = 0;

osThreadId tid_exAirCM_Thread;
osThreadId tid_exAirCM_DB_Thread;
osThreadDef(exAirCM_Thread,osPriorityNormal,1,256);
osThreadDef(exAirCM_DB_Thread,osPriorityNormal,1,256);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void TIM4_PWM_Init_exAir(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //时钟使能

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD| RCC_APB2Periph_AFIO, ENABLE);  //使能GPIO外设和AFIO复用功能模块时钟
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); 
	//设置该引脚为 输出TIM4 CH4的PWM脉冲波形	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIO


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
	TIM_OC4Init(TIM4, &TIM_OCInitStructure);  //根据T指定的参数初始化外设TIM3 OC2

	TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);  //使能TIM3在CCR2上的预装载寄存器
 
	TIM_Cmd(TIM4, ENABLE);  //使能TIM4		 
}

//按键初始化函数
void KEY_Init_exAir(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOC,ENABLE);//使能PORTC时钟


  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		//设置成上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		//设置成上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOE, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.
	
}

u8 KEY_Scan1_exAir(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY3_exAir==0||KEY4_exAir==0||KEY5_exAir==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY3_exAir==0)return 3;
		else if(KEY4_exAir==0)return 4;
		else if(KEY5_exAir==0)return 5;
	}else if(((KEY3_exAir==1)&&(KEY4_exAir==1)&&(KEY5_exAir==1))==1)key_up=1; 	    
 	return 0;// 无按键按下
}

void exAir_Init(void){

	BEEP_Init();
	KEY_Init_exAir();
	//Beep_time(200);
	TIM_SetCompare4(TIM4,0);	
}

void ex_Air(void){
	
	u8 key_in;
	u8 key3_flg=0;
	
	TIM4_PWM_Init_exAir(PWM_exAir_Init,10);	//不分频。PWM频率=72000000/900=80Khz   ARR=900 
	
	while(1)
	{
		key_in=KEY_Scan1_exAir(0);
		
		if(key_in == 3){
		
			key3_flg = !key3_flg; 
			
			if(key3_flg){
				 
				PWM_exAir = 3000;
				TIM_SetCompare4(TIM4,PWM_exAir/2);
			}else{
			
				PWM_exAir = 0;
				TIM_SetCompare4(TIM4,0);
			}	
			USRKexaTX_FLG = 1;
			Beep_time(80);
		}else if(key_in == 4 && key3_flg){
		
			if(PWM_exAir<6000)PWM_exAir=PWM_exAir+500;		
			TIM_SetCompare4(TIM4,PWM_exAir/2);//50%
			Beep_time(80);

			USRKexaTX_FLG = 1;
		}else if(key_in == 5 && key3_flg){
		
			if(PWM_exAir>=1000)PWM_exAir=PWM_exAir-500;
			TIM_SetCompare4(TIM4,PWM_exAir/2);//50%
			Beep_time(80);
			
			USRKexaTX_FLG = 1;
		}
		
		if(USRKexaRX_FLG == 1){
			
			USRKexaRX_FLG = 0;
		
			TIM_SetCompare4(TIM4,PWM_exAir/2);//50%
			Beep_time(80);
		}
	}
}

void exAirCM_DB_Thread(const void *argument){
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	uint16_t PWM_exAir_DB = PWM_exAir / 60;
	
	for(;;){
		
		PWM_exAir_DB = PWM_exAir / 60;
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rexchange air wind pwm is : %d%\n\r", PWM_exAir_DB);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(1000);
	}
}

void exAirCM_Thread(const void *argument){
	
	for(;;)ex_Air();
}

void exAir(void){
	
	tid_exAirCM_Thread = osThreadCreate(osThread(exAirCM_Thread),NULL);
	tid_exAirCM_DB_Thread = osThreadCreate(osThread(exAirCM_DB_Thread),NULL);
}
