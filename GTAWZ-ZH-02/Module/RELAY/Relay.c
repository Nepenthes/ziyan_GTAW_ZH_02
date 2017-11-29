#include "Relay.h"

uint8_t swRelay1;
uint8_t swRelay2;

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
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	for(;;){
		
		if(swRelay1 == 1){RELAY1_ON;}
		else RELAY1_OFF;
		
		if(swRelay2 == 1){RELAY2_ON;}
		else RELAY2_OFF;
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rRelay1 status is : %d, Relay2 status is : %d\n\r", swRelay1,swRelay2);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(1000);
	}
} 

void swRelayCM(void){

	tid_swRelayCM_Thread = osThreadCreate(osThread(swRelayCM_Thread),NULL);
}
