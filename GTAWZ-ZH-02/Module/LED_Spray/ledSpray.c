#include "ledSpray.h"

uint8_t SW_SPY = 2;
uint8_t SW_PST = 2;
uint8_t USRKspyTX_FLG = 0;
uint8_t USRKspyRX_FLG = 0;
uint8_t SW_STATUS[2] = {2,2};
 
osThreadId tid_LEDSprayCM_Thread;
osThreadId tid_LEDSprayCM_DB_Thread;
osThreadDef(LEDSprayCM_Thread,osPriorityNormal,1,256);
osThreadDef(LEDSprayCM_DB_Thread,osPriorityNormal,1,256);

osThreadId tid_flash_SPY_Thread;
osThreadId tid_flash_PST_Thread;
osThreadDef(flash_SPY_Thread,osPriorityNormal,1,256);
osThreadDef(flash_PST_Thread,osPriorityNormal,1,256);

extern ARM_DRIVER_USART Driver_USART1;								//�豸�����⴮��һ�豸����
 
void NC595_Init(void)
{
 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOE, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11|GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOD, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_15;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
	GPIO_Init(GPIOD, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

	//GPIO_Write(GPIOB,0XFF00);
	GPIO_ResetBits(GPIOE,GPIO_Pin_0);						
	GPIO_ResetBits(GPIOE,GPIO_Pin_1);		
	GPIO_ResetBits(GPIOD,GPIO_Pin_14);						
	GPIO_ResetBits(GPIOD,GPIO_Pin_11);	

}

//������ʼ������
void KEY_Init_ledSPY(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PORTCʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_14|GPIO_Pin_15;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOC, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4	
}

u8 KEY_Scan1_ledSPY(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY3_ledSPY==0||KEY4_ledSPY==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY3_ledSPY==0)return 3;
		else if(KEY4_ledSPY==0)return 4;
	}else if(KEY3_ledSPY==1&&KEY4_ledSPY==1)key_up=1; 	    
 	return 0;// �ް�������
}


void dats595_clr(uint8_t num){

	uint8_t loop;
	
	EN_ledSPY = 0;	
	
	DS_ledSPY = 0;
	for(loop = 0;loop < num;loop ++){
	
		LCLK_ledSPY = SCLK_ledSPY = 1;
		LCLK_ledSPY = SCLK_ledSPY = 0;
	}
}

void datsin595(uint8_t num,uint8_t time){
	
	uint8_t loop;
	
	EN_ledSPY = 0;
	
	DS_ledSPY = 1;
	for(loop = 0;loop < num;loop ++){
	
		LCLK_ledSPY = SCLK_ledSPY = 1;
		LCLK_ledSPY = SCLK_ledSPY = 0;
		osDelay(time);
	}
}

void LEDSpray_Init(void){

	NC595_Init();
	KEY_Init_ledSPY();
	BEEP_Init();	
	Beep_time(150);
}

void LED_Spray(void){

	u8 key_in;
	u8 key3_flg=0,key4_flg=0;
	
	static u8 status_spy;
	static u8 status_pst;
	
	while(1)
	{
		USRKspyTX_FLG = 1;     //�����������ߣ�������ã�ע�ͼ��ɣ�ע�ͺ��򱻶����ߣ�״̬�ı�ʱ���Ϸ�״̬��Ϣ
		
		key_in=KEY_Scan1_ledSPY(0); //����ɨ��
/*--------------------------------------------------------------------------*/			
		if(key_in == 3 && !key4_flg){  //������
		
			key3_flg = !key3_flg; 
			
			dats595_clr(50);
			if(key3_flg){
			
				if(SW_STATUS[0] == 2)SW_STATUS[0] = 1;	
			}else{
			
				if(SW_STATUS[0] == 1)SW_STATUS[0] = 2;	
			}	
			USRKspyTX_FLG = 1;
		}else if(key_in == 4 && !key3_flg){		//ɱ�水��
		
			key4_flg = !key4_flg; 
			
			dats595_clr(50);
			if(key4_flg){
			
				if(SW_STATUS[1] == 2)SW_STATUS[1] = 1;	
			}else{
			
				if(SW_STATUS[1] == 1)SW_STATUS[1] = 2;	
			}
			USRKspyTX_FLG = 1;
		}
/*--------------------------------------------------------------------------*/	
		if(USRKspyRX_FLG){
			
			dats595_clr(50);
			
			if(SW_STATUS[0] != SW_SPY){
				
				SW_STATUS[0] = SW_SPY;
				if(SW_STATUS[0] == 1)key3_flg = 1;
				else if(SW_STATUS[0] == 2)key3_flg = 0;
			}
			if(SW_STATUS[1] != SW_PST){
			
				SW_STATUS[1] = SW_PST;
				if(SW_STATUS[1] == 1)key4_flg = 1;
				else if(SW_STATUS[1] == 2)key4_flg = 0;
			}
			
			USRKspyRX_FLG =0;
		}	
/*--------------------------------------------------------------------------*///ģ�⶯����ˮ��ִ��		
		if(status_spy != SW_STATUS[0]){
		
			status_spy = SW_STATUS[0];
			if(status_spy == 1){
			
				Beep_time(80);
				osThreadTerminate(tid_flash_PST_Thread);
				tid_flash_SPY_Thread = osThreadCreate(osThread(flash_SPY_Thread),NULL);

			}else if(status_spy == 2){
			
				Beep_time(80);
				osThreadTerminate(tid_flash_SPY_Thread);
			}
			
			dats595_clr(50);
		}
		if(status_pst != SW_STATUS[1]){
		
			status_pst = SW_STATUS[1];
			if(status_pst == 1){
			
				Beep_time(80);
				osThreadTerminate(tid_flash_SPY_Thread);
				tid_flash_PST_Thread = osThreadCreate(osThread(flash_PST_Thread),NULL);
				
			}else if(status_pst == 2){
			
				Beep_time(80);
				osThreadTerminate(tid_flash_PST_Thread);
			}
			
			dats595_clr(50);
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
		sprintf(disp,"\n\rswitch_SPY : %d switch_PST: %d\n\r", SW_STATUS[0],SW_STATUS[1]);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		osDelay(1000);
	}
}

void flash_SPY_Thread(const void *argument){

	uint8_t loop;
	
	for(;;){
		
		dats595_clr(50);
		datsin595(19, 50);
		datsin595(2, 0);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		
		for(loop = 0;loop < 5;loop ++){
		
			dats595_clr(50);
			datsin595(19, 0);
			datsin595(2, 0);
			datsin595(5, 0);
			osDelay(15);
			datsin595(5, 0);
			osDelay(15);
			datsin595(5, 0);
			osDelay(15);
			datsin595(5, 0);
			osDelay(15);
			datsin595(5, 0);
			osDelay(15);
			datsin595(5, 0);
			osDelay(15);
		}
	}
}

void flash_PST_Thread(const void *argument){

	uint8_t loop;
	
	for(;;){
		
		dats595_clr(50);
		datsin595(19, 50);
		datsin595(2, 0);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		datsin595(5, 0);
		osDelay(10);
		
		for(loop = 0;loop < 15;loop ++){
		
			dats595_clr(50);
			datsin595(19, 0);
			datsin595(2, 0);
			datsin595(5, 0);
			osDelay(35);
			datsin595(5, 0);
			osDelay(35);
			datsin595(5, 0);
			osDelay(35);
			datsin595(5, 0);
			osDelay(35);
			datsin595(5, 0);
			osDelay(35);
			datsin595(5, 0);
			osDelay(35);
		}
	}
}

void LEDSpray(void){

	tid_LEDSprayCM_Thread = osThreadCreate(osThread(LEDSprayCM_Thread),NULL);
	tid_LEDSprayCM_DB_Thread = osThreadCreate(osThread(LEDSprayCM_DB_Thread),NULL);
}
