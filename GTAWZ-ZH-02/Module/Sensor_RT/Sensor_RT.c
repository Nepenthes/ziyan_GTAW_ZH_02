#include "Sensor_RT.h"

uint8_t isSomeone;

osThreadId tid_RTMS_Thread;
osThreadDef(RTMS_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;								//�豸�����⴮��һ�豸����

void RT_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.
}

void RTMS_Thread(const void *argument){
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	for(;;){
		
		isSomeone	= RT_DATA;
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rSomebody here? : %d\n\r", &isSomeone);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(1000);
	}
}

void RTMS(void){

	tid_RTMS_Thread = osThreadCreate(osThread(RTMS_Thread),NULL);
}
