#include "Curtain.h"

uint8_t curAction[2] = {0x03,0x00}; //状态、开合度

uint16_t cycMotorCnt = 0;  //激励脉冲计数
uint8_t  MotorCntEN  = 0;	//激励脉冲计数使能
uint8_t	IorD = 0;			//上下标记

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
	
	if(CURLIMIT_UP){
	
		CURTAIN_EN 	= 0;
		CURTAIN_DIR	= 0;
		
	}else{
	
		CURTAIN_EN = 1;
		curAction[0] = 0x03;
		USRKcurTX_FLG = 1;
		
		MotorCntEN = 0;
		curAction[1] = 100;
		cycMotorCnt  = 33 * 100;
		
		USRKcurRX_FLG = 0;
		//Beep_time(20);
		osDelay(10);
		//Beep_time(20);
	}
}

void curClose(void){

	if(CURLIMIT_DN){
	
		CURTAIN_EN 	= 0;
		CURTAIN_DIR	= 1;
		
	}else{
	
		CURTAIN_EN = 1;
		curAction[0] = 0x03;
		USRKcurTX_FLG = 1;
		
		MotorCntEN = 0;
		curAction[1] = cycMotorCnt = 0;

		USRKcurRX_FLG = 0;
		//Beep_time(20);
		osDelay(10);
		//Beep_time(20);
	}
}

void curStop(void){
	
	CURTAIN_EN = 1;
}

void Curtain_Init(void){

	BEEP_Init();
	KEY_Init_Curtain();
	////Beep_time(200);	
}

void CurtainCM_CLK_Thread(const void *argument){
	
	CURTAIN_CLK = 1;
	osDelay(1);
	CURTAIN_CLK = 0;
	osDelay(1);
	
	if(MotorCntEN)(IorD)?cycMotorCnt ++:cycMotorCnt --;
	
}

void CurtainCM_Thread(const void *argument){
	
	u8 key_in;
	
	char Disp[30] = {0};
	
	cycMotorCnt = STMFLASH_ReadHalfWord(CURTAIN_POSITION_ID15);
	
	for(;;){
		
		USRKcurTX_FLG = 1;		 //主动保持在线，如果不用，注释即可，注释后则被动在线，状态改变时才上发状态信息
		
		key_in = KEY_Scan1_Curtain(0);
		
		if(key_in == 3){
		
			curAction[0] = 0x01;
			curAction[1] = (uint8_t)(cycMotorCnt / 33);
			//USRKcurRX_FLG = 0;
		
			USRKcurTX_FLG = 1;
			//Beep_time(80);
		}else if(key_in == 4){
		
			curAction[0] = 0x02;
			curAction[1] = (uint8_t)(cycMotorCnt / 33);
			//USRKcurRX_FLG = 0;

			USRKcurTX_FLG = 1;
			//Beep_time(80);
		}else if(key_in == 5){
		
			curAction[0] = 0x03;
			curAction[1] = (uint8_t)(cycMotorCnt / 33);
			USRKcurTX_FLG = 1;
			
			//USRKcurRX_FLG = 0;
			
			STMFLASH_Write(CURTAIN_POSITION_ID15,(u16*)&cycMotorCnt,0x01);
			memset(Disp,0,30 * sizeof(char));
			sprintf(Disp,"累计脉冲：%d \r\n",cycMotorCnt);
			osDelay(10);
			Driver_USART1.Send(Disp,strlen(Disp));
			
			USRKcurTX_FLG = 1;
			//Beep_time(80);
		}
		
		if(USRKcurRX_FLG == 1){
			
//			if(cycMotorCnt != (curAction[1] * 33)){
//			
//				if(cycMotorCnt / 33 >= curAction[1]){   //如果不使用>=  而使用> 电机就会抖动
//				
//					IorD = 0;
//					MotorCntEN = 1;
//					curAction[0] = 0x02;
//				}else{
//				
//					IorD = 1;
//					MotorCntEN = 1;
//					curAction[0] = 0x01;
//				}
//				
//			}else{		
//			
//				MotorCntEN = 0;
//				curAction[0] = 0x03;
//				curAction[1] = cycMotorCnt / 33;
//				USRKcurRX_FLG = 0;
//				USRKcurTX_FLG = 1;
//			}

//******************************************************************************************************************//
			curAction[1] = (uint8_t)(cycMotorCnt / 33);
			USRKcurRX_FLG = 0;

			//Beep_time(80);
		}
		
		switch(curAction[0]){
		
			case CUR_OPEN:		IorD = 1;
									MotorCntEN = 1;
									curOpen();
									break;
			
			case CUR_CLOSE:	IorD = 0;
									MotorCntEN = 1;
									curClose();
									break;
				
			case CUR_STOP:		MotorCntEN = 0;
									curStop();
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
