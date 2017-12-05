#include "Relay.h"

uint8_t swRelay_lock = 2;
uint8_t swRelay_light = 2;

uint8_t USRKjdqTX_FLG = 0;
uint8_t USRKjdqRX_FLG = 0;

osThreadId tid_swRelayCM_Thread;
osThreadDef(swRelayCM_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

void swRelay_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13;				
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	PCout(12) = PCout(13) = 0;
}

void swRelayCM_Thread(const void *argument){
	
	uint8_t cnt_DN = 0;
	uint8_t DN_FLG = 0;
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	for(;;){
		
		USRKjdqTX_FLG = 1;		 //主动保持在线，如果不用，注释即可，注释后则被动在线，状态改变时才上发状态信息
		
		if(USRKjdqRX_FLG == 1){
			
			USRKjdqRX_FLG = 0;
		
			if(swRelay_lock == 1){RELAY1_ON;DN_FLG = 1;cnt_DN = 10;}
			else if(swRelay_lock == 2){RELAY1_OFF;}
			
			if(swRelay_light == 1){RELAY2_ON;}
			else if(swRelay_light == 2){RELAY2_OFF;}
		}
		
		if(DN_FLG){
		
			if(cnt_DN){
			
				cnt_DN --;
			}else{
			
				DN_FLG = 0;
				swRelay_lock = 2;
			}
		}
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rRelay1 status is : %d, Relay2 status is : %d\n\r", swRelay_lock,swRelay_light);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(500);
	}
} 

void swRelayCM(void){

	tid_swRelayCM_Thread = osThreadCreate(osThread(swRelayCM_Thread),NULL);
}
