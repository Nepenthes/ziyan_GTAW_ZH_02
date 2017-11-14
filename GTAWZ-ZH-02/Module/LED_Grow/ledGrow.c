#include "ledGrow.h"

uint16_t PWM_ledGRW=3000;//pwm

osThreadId tid_LEDGrowCM_Thread;
osThreadId tid_LEDGrowCM_DB_Thread;
osThreadDef(LEDGrowCM_Thread,osPriorityNormal,1,256);
osThreadDef(LEDGrowCM_DB_Thread,osPriorityNormal,1,256);

extern ARM_DRIVER_USART Driver_USART1;								//�豸�����⴮��һ�豸����

//������ʼ������
void KEY_Init_ledGRW(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	//��ʼ��KEY0-->GPIOA.13,KEY1-->GPIOA.15  ��������
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1;//PE2~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

	//��ʼ�� WK_UP-->GPIOA.0	  ��������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIOA.0
}

void TIM4_PWM_Init_ledGRW(u16 arr,u16 psc)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE); //ʱ��ʹ��

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD| RCC_APB2Periph_AFIO, ENABLE);  //ʹ��GPIO�����AFIO���ù���ģ��ʱ��
	GPIO_PinRemapConfig(GPIO_Remap_TIM4, ENABLE); 
	//���ø�����Ϊ�����������,���TIM3 CH2��PWM���岨��	GPIOB.5
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14; //TIM_CH2
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIO
//PB9  TIM4-CH4

   //��ʼ��TIM3
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ 
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
	
	//��ʼ��TIM3 Channel2 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low; //�������:TIM����Ƚϼ��Ը�
	TIM_OC3Init(TIM4, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM3 OC2

	TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);  //ʹ��TIM3��CCR2�ϵ�Ԥװ�ؼĴ���
 
	TIM_Cmd(TIM4, ENABLE);  //ʹ��TIM4		 
}

u8 KEY_Scan1_ledGRW(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY3_ledGRW==0||KEY4_ledGRW==0||KEY5_ledGRW==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY3_ledGRW==0)return 3;
		else if(KEY4_ledGRW==0)return 4;
		else if(KEY5_ledGRW==0)return 5;
	}else if(((KEY3_ledGRW==1)&&(KEY4_ledGRW==1)&&(KEY5_ledGRW==1))==1)key_up=1; 	    
 	return 0;// �ް�������
}

void LEDGrow_Init(void){

	BEEP_Init();
	KEY_Init_ledGRW();
}

void led_Grow(void){

	u8 t=0,cnt=0,key_in;
	u8 key_num=0;
	
	TIM4_PWM_Init_ledGRW(PWM_ledGRW,10);	//PWMƵ��=72000000/900=80Khz   ARR=900 
	
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
