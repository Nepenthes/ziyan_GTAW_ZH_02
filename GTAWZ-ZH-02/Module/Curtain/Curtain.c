#include "Curtain.h"

uint8_t curAction = 0x03;

uint32_t cycMotorCnt = 0;
uint8_t 	MotorCntEN 	= 0;

uint8_t USRKcurTX_FLG = 0;
uint8_t USRKcurRX_FLG = 0;

osThreadId tid_CurtainCM_Thread;

osThreadDef(CurtainCM_Thread,osPriorityNormal,1,1024);
osTimerDef(Tim_CurtianClk_Thread,CurtainCM_CLK_Thread);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

//按键初始化函数
void KEY_Init_Curtain(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE,ENABLE);//使能PORTC时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;			//霍尔传感器限位
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;		
	GPIO_Init(GPIOE, &GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;			//步进电机时钟及时钟
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_Init(GPIOE, &GPIO_InitStructure);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_15;	//按键
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		
	GPIO_Init(GPIOD, &GPIO_InitStructure);		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;			//步进电机使能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;			
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;		
	GPIO_Init(GPIOD, &GPIO_InitStructure);	
}

u8 KEY_Scan1_Curtain(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY3_Curtain==0||KEY4_Curtain==0||KEY5_Curtain==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY3_Curtain==0)return 3;
		else if(KEY4_Curtain==0)return 4;
		else if(KEY5_Curtain==0)return 5;
	}else if(((KEY3_Curtain==1)&&(KEY4_Curtain==1)&&(KEY5_Curtain==1))==1)key_up=1; 	    
 	return 0;// 无按键按下
}

void curOpen(void){

	char Disp[30] = {0};
	
	if(CURLIMIT_UP){
	
		CURTAIN_EN 	= 0;
		CURTAIN_DIR	= 0;
		
		MotorCntEN = 1;
		
	}else{
	
		CURTAIN_EN = 1;
		curAction = 0x03;
		Beep_time(20);
		osDelay(10);
		Beep_time(20);
		
		cycMotorCnt = MotorCntEN = 0;
		memset(Disp,0,30 * sizeof(char));
		sprintf(Disp,"累计脉冲：%d",cycMotorCnt);
		Driver_USART1.Send(Disp,strlen(Disp));
	}
}

void curClose(void){
	
	char Disp[30] = {0};

	if(CURLIMIT_DN){
	
		CURTAIN_EN 	= 0;
		CURTAIN_DIR	= 1;
		
		MotorCntEN = 1;
		
	}else{
	
		CURTAIN_EN = 1;
		curAction = 0x03;
		Beep_time(20);
		osDelay(10);
		Beep_time(20);
		
		cycMotorCnt = MotorCntEN = 0;
		memset(Disp,0,30 * sizeof(char));
		sprintf(Disp,"累计脉冲：%d",cycMotorCnt);
		Driver_USART1.Send(Disp,strlen(Disp));
	}
}

void curStop(void){

	CURTAIN_EN = 1;
}

void Curtain_Init(void){

	BEEP_Init();
	KEY_Init_Curtain();
	//Beep_time(200);	
}

void CurtainCM_CLK_Thread(const void *argument){
	
	CURTAIN_CLK = 1;
	osDelay(1);
	CURTAIN_CLK = 0;
	osDelay(1);
	
	if(MotorCntEN)cycMotorCnt ++;
}

void CurtainCM_Thread(const void *argument){
	
	u8 key_in;
	
	for(;;){
		
		key_in = KEY_Scan1_Curtain(0);
		
		if(key_in == 3){
		
			curAction = 0x01;
		
			USRKcurTX_FLG = 1;
			Beep_time(80);
		}else if(key_in == 4){
		
			curAction = 0x02;

			USRKcurTX_FLG = 1;
			Beep_time(80);
		}else if(key_in == 5){
		
			curAction = 0x03;
			
			USRKcurTX_FLG = 1;
			Beep_time(80);
		}
		
		if(USRKcurRX_FLG == 1){
			
			
			
			USRKcurRX_FLG = 0;
			Beep_time(80);
		}
		
		switch(curAction){
		
			case CUR_OPEN:		curOpen();
									break;
			
			case CUR_CLOSE:	curClose();
									break;
				
			case CUR_STOP:		curStop();
									break;
			
					default:		break;				
		}
	}
}

void Curtain(void){
	
	osTimerId timid_curCLk;
	
	tid_CurtainCM_Thread = osThreadCreate(osThread(CurtainCM_Thread),NULL);
	timid_curCLk = osTimerCreate(osTimer(Tim_CurtianClk_Thread), osTimerPeriodic, &CurtainCM_CLK_Thread);
	
	osTimerStart(timid_curCLk,2);
}
