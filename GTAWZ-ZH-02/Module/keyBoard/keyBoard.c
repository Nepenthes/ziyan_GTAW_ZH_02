#include "keyBoard.h"

uint8_t  valKeyBoard;            //��ʾ����
uint8_t  valKeyBoard_scan;        
uint8_t  key; 

osThreadId tid_KeyBoardMS_Thread;
osThreadDef(KeyBoardMS_Thread,osPriorityNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;						//�豸�����⴮��һ�豸����

//������ʼ������
void KEY_RInit(void) //IO��ʼ��
{ 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PB�˿�ʱ��
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_12|GPIO_Pin_13;//
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
 GPIO_Init(GPIOC, &GPIO_InitStructure);					

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
 GPIO_Init(GPIOA, &GPIO_InitStructure);		
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
 GPIO_Init(GPIOB, &GPIO_InitStructure);		
}

void KEY_CInit(void) //IO��ʼ��
{ 
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_12|GPIO_Pin_13;//
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��GPIOB.

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOC, &GPIO_InitStructure);					

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOA, &GPIO_InitStructure);		

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
}

void keyscan(void)
 { 	

	KEY_RInit();
	GPIO1_KB=0;GPIO2_KB=0;GPIO3_KB=0;GPIO4_KB=0;
	GPIO5_KB=1;GPIO6_KB=1;GPIO7_KB=1;GPIO8_KB=1; 
	delay_us(10);// 
	if(COL1==0)
	{
		key=1;	
	} 
	if(COL2==0)
	{
		key=2;		
	}    
	if(COL3==0)
	{
		key=3;		
	} 
	if(COL4==0)
	{
		key=4;		
	}
	 
	 //temp = 0;
   KEY_CInit();
	GPIO1_KB=1;GPIO2_KB=1;GPIO3_KB=1;GPIO4_KB=1;
	GPIO5_KB=0;GPIO6_KB=0;GPIO7_KB=0;GPIO8_KB=0; 
	delay_us(10);// 
	if(ROW1==0)
	{
		key=key+0;			
	} 
	if(ROW2==0)
	{
		key=key+4;			
	}    
	if(ROW3==0)
	{
		key=key+8;			
	} 
	if(ROW4==0)
	{
		key=key+12;			
	}
	while((ROW1==0)|(ROW2==0)|(ROW3==0)|(ROW4==0));

	
	 valKeyBoard_scan = key;	    //��ֵ����ʾ����
	
 }

void keydown(void)
{   
	KEY_RInit();
	GPIO1_KB=0;GPIO2_KB=0;GPIO3_KB=0;GPIO4_KB=0;
	GPIO5_KB=1;GPIO6_KB=1;GPIO7_KB=1;GPIO8_KB=1; 
	delay_us(10);// 
	
	if((COL1==0)|(COL2==0)|(COL3==0)|(COL4==0))
	{
		keyscan();	
	} 	
}

void keyBoard_Init(void){

	;
}

void KeyBoardMS_Thread(const void *argument){

#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	uint8_t  val_KeyBoard;
	uint8_t  disp_cnt,disp_flg;
	
	for(;;){
		
		if(val_KeyBoard != valKeyBoard_scan){
		
			val_KeyBoard = valKeyBoard_scan;
			valKeyBoard  = valKeyBoard_scan;
			disp_flg = 1;
			disp_cnt = 0;
			
#if(MOUDLE_DEBUG == 1)	
			sprintf(disp,"\n\rvalKeyBard: %d\n\r", valKeyBoard);			
			Driver_USART1.Send(disp,strlen(disp));
#endif	
		}
	
		keydown();

		osDelay(100);
		if(disp_flg){
		
			if(disp_cnt > 10){
			
				disp_flg = 0;
				disp_cnt = 0;
				valKeyBoard = 0;
			}else disp_cnt++;
		}
	}
}

void KeyBoardMS(void){

	tid_KeyBoardMS_Thread = osThreadCreate(osThread(KeyBoardMS_Thread),NULL);
}
