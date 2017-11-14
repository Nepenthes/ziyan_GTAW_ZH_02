#include "ledSpray.h"

uint8_t SW_SPY = 0;
uint8_t SW_PST = 0;
 
osThreadId tid_LEDSprayCM_Thread;
osThreadId tid_LEDSprayCM_DB_Thread;
osThreadDef(LEDSprayCM_Thread,osPriorityNormal,1,256);
osThreadDef(LEDSprayCM_DB_Thread,osPriorityNormal,1,256);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明
 
void NC595_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_15;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
	GPIO_Init(GPIOD, &GPIO_InitStructure);//初始化GPIOE2,3,4

	//GPIO_Write(GPIOB,0XFF00);
	GPIO_ResetBits(GPIOE,GPIO_Pin_0);						
	GPIO_ResetBits(GPIOE,GPIO_Pin_1);		
	GPIO_ResetBits(GPIOD,GPIO_Pin_14);						
	GPIO_ResetBits(GPIOD,GPIO_Pin_11);	

}

//按键初始化函数
void KEY_Init_ledSPY(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//使能PORTC时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14|GPIO_Pin_15;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//初始化GPIOE2,3,4	
}

u8 KEY_Scan1_ledSPY(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY3_ledSPY==0||KEY4_ledSPY==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY3_ledSPY==0)return 3;
		else if(KEY4_ledSPY==0)return 4;
	}else if(KEY3_ledSPY==1&&KEY4_ledSPY==1)key_up=1; 	    
 	return 0;// 无按键按下
}

void SH_595_ledSPY(u8 Date_in)  
{  
 u8 j;  
 for (j=0;j<8;j++)  
 {  
	SCLK_ledSPY=0;//上升沿发生移位
	delay_us(1);
	if ((Date_in&0x01)==0x01)
		DS_ledSPY=1;
	else
		DS_ledSPY=0;	 			 
	SCLK_ledSPY=1; 
	delay_us(1);
	Date_in=Date_in>>1;	
 }  
} 

unsigned char Shift_Data_ledSPY(u8 dat)
{
    u8 i;  
    u8 tmp=0x00;  
  
    for(i=0;i<8;i++)  
    {  
        tmp=((dat>>i)&0x01)|tmp;  
        if(i<7)  
					tmp=tmp<<1;  
    }  	
		return tmp;
}	

void CON1_R595(void)
{
	//发送   1  2  4   对应  P2  P3  P4
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);

	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	OUT_595();
	delay_us(10);	
}	

void CON2_R595(void)
{
	//发送   1  2  4   对应  P2  P3  P4
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);

	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	OUT_595();
	delay_us(10);	
}	

void CON3_R595(void)
{
	//发送   1  2  4   对应  P2  P3  P4
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);

	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0X00)); 
	OUT_595();
	delay_us(10);	
}	

void CON4_R595(void)
{
	//发送   1  2  4   对应  P2  P3  P4
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);

	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	//OUT_595();
	delay_us(10);
	
	SH_595_ledSPY(Shift_Data_ledSPY(0XFF)); 
	OUT_595();
	delay_us(10);	
}	

void LEDSpray_Init(void){

	NC595_Init();
	KEY_Init_ledSPY();
	BEEP_Init();	
//	Beep_time(150);
}

void LED_Spray(void){

	u8 t=0,cnt=0,key_in;
	u8 key1_cnt=0,key2_cnt=0;

	CON3_R595();//clr
	
	while(1)
	{
		key_in=KEY_Scan1_ledSPY(0);
		if(key_in==3)
		{
				key1_cnt++;
			  if(key1_cnt%2==1 || SW_SPY)
				{
					CON1_R595();
					Beep_time(150);
				}
				else
				{
					CON3_R595();
					Beep_time(150);
				}		
		}	
		else if(key_in==4 || SW_PST)
		{
				key2_cnt++;
			  if(key2_cnt%2==1)
				{
					CON2_R595();
					Beep_time(150);
				}
				else
				{
					CON3_R595();
					Beep_time(150);
				}	 
		}			
		t++; 
		delay_ms(10);
		if(t==100)
		{
			t=0;
			cnt++;
		}		   
	}
}

void LEDSprayCM_Thread(const void *argument){
	
	for(;;)LED_Spray();
}

void LEDSprayCM_DB_Thread(const void *argument){
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	for(;;){
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rswitch_SPY : %d switch_PST: %d\n\r", &SW_SPY,&SW_PST);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(1000);
	}
}

void LEDSpray(void){

	tid_LEDSprayCM_Thread = osThreadCreate(osThread(LEDSprayCM_Thread),NULL);
	tid_LEDSprayCM_DB_Thread = osThreadCreate(osThread(LEDSprayCM_DB_Thread),NULL);
}
