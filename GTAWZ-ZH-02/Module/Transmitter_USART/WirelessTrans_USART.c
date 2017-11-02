#include <WirelessTrans_USART.h>

extern osMutexId (uart1_mutex_id);
extern uint8_t FLG_CLO;

#if(MOUDLE_ID == 9)
extern float SHT11_hum; 
extern float SHT11_temp;
#endif

const uint8_t MOUDLE_TYPE[20] = {
	
	GTA_GGI01,GTA_GGI02,GTA_GGI03,GTA_GGS01,
	GTA_GGS02,GTA_GGS03,GTA_GGS04,GTA_GGS05,
	GTA_GGS06,GTA_GGS07,GTA_GG001,GTA_GID01,
	GTA_GID02,GTA_GID03,GTA_PB002,GTA_PB003,
	GTA_PB004,GTA_PB005,GTA_PB006,GTA_PBP01
};

const uint8_t FRAME_PRT1[5] = {FRAME_HEAD,GATEWAY_ADDR,NODE_ADDR,MODULE_ADDR};

const char *TestCMD[] = {

	"天王盖地虎"
};

const char *TestREP[] = {

	"小鸡炖蘑菇\r\n"
};

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;

osThreadId tid_USART1Test_Thread;
osThreadId tid_USART2Test_Thread;

osThreadDef(USART1Test_Thread,osPriorityNormal,1,128);
osThreadDef(USART2Test_Thread,osPriorityNormal,1,128);

uint8_t FRAME_TX[FRAME_TX_SIZE] = {0};
uint8_t FRAME_RX[FRAME_RX_SIZE] = {0};

void USART1Init1(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	/* config USART1 clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
	
	/* USART1 GPIO config */
	/* Configure USART1 Tx (PA.09) as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);    
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	  
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure); 
	USART_Cmd(USART1, ENABLE);
}

void USART1Init2(void){
	
	/*Initialize the USART driver */
	Driver_USART1.Initialize(myUSART1_callback);
	/*Power up the USART peripheral */
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 115200 Bits/sec */
	Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |
									ARM_USART_PARITY_NONE |
									ARM_USART_STOP_BITS_1 |
							ARM_USART_FLOW_CONTROL_NONE, 115200);

	/* Enable Receiver and Transmitter lines */
	Driver_USART1.Control (ARM_USART_CONTROL_TX, 1);
	Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);

	osMutexWait(uart1_mutex_id,osWaitForever);
	Driver_USART1.Send("i'm usart1,Press Enter to receive a message\r\n", 45);
	//Driver_USART1.Send((char*)(TestREP[0]),strlen((char*)(TestREP[0])));
	osMutexRelease(uart1_mutex_id);
}

void USART2Init1(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);	//使能USART1，GPIOA时钟
	USART_DeInit(USART2);  //复位串口1
	//USART2_TX   PA.2	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//USART2_RX	  PA.3
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;  
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	USART_InitStructure.USART_BaudRate            = 115200;
	USART_InitStructure.USART_WordLength          = USART_WordLength_8b;  
	USART_InitStructure.USART_StopBits            = USART_StopBits_1;     
	USART_InitStructure.USART_Parity              = USART_Parity_No ;     
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART2, &USART_InitStructure);

	//Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器


	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART2, ENABLE);     
}

void USART2Init2(void){

	/*Initialize the USART driver */
	Driver_USART2.Initialize(myUSART2_callback);
	/*Power up the USART peripheral */
	Driver_USART2.PowerControl(ARM_POWER_FULL);
	/*Configure the USART to 115200 Bits/sec */
	Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS |
									ARM_USART_DATA_BITS_8 |
									ARM_USART_PARITY_NONE |
									ARM_USART_STOP_BITS_1 |
							ARM_USART_FLOW_CONTROL_NONE, 115200);

	/* Enable Receiver and Transmitter lines */
	Driver_USART2.Control (ARM_USART_CONTROL_TX, 1);
	Driver_USART2.Control (ARM_USART_CONTROL_RX, 1);

	Driver_USART2.Send("i'm usart2,Press Enter to receive a message\r\n", 45);
}

void myUSART1_callback(uint32_t event)
{
//  uint32_t mask;
//  mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
//         ARM_USART_EVENT_TRANSFER_COMPLETE |
//         ARM_USART_EVENT_SEND_COMPLETE     |
//         ARM_USART_EVENT_TX_COMPLETE       ;
//  if (event & mask) {
////    /* Success: Wakeup Thread */
////    osSignalSet(tid_myUART_Thread, 0x01);
//  }
//  if (event & ARM_USART_EVENT_RX_TIMEOUT) {
//    __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//  }
//  if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
//    __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//  }
}

void myUSART2_callback(uint32_t event){

	;
}

void USART1Test_Thread(const void *argument){

	char cmd[30] = "abc";
	
	for(;;){

		osDelay(10);													//必需延时，防乱序
		Driver_USART1.Receive(cmd,strlen(TestCMD[0]));
		//if(strstr(cmd,TestCMD[0])){								//子串比较
		if(!strcmp(TestCMD[0],cmd)){							//全等比较
				
			osMutexWait(uart1_mutex_id,osWaitForever);
			Driver_USART1.Send((char*)(TestREP[0]),strlen((char*)(TestREP[0])));
			osMutexRelease(uart1_mutex_id);
			memset(cmd,0,30*sizeof(char));
		}
	}
}

void USART2Test_Thread(const void *argument){

	uint8_t dats[20];
	
	for(;;){
		
#if(MOUDLE_ID > 10)
		char *pt;	
		uint8_t dats_rx[10];
	
		Driver_USART2.Receive(FRAME_RX,FRAME_RX_SIZE);
		
		pt = strstr((const char*)FRAME_RX,(const char*)FRAME_PRT1);
	
		if(pt){
			
			memset(dats,0,10*sizeof(uint8_t));
			memset(dats_rx,0,20*sizeof(uint8_t));
			memcpy(dats,pt,8 + pt[6]);
			
			if(dats[4] == MOUDLE_TYPE[MOUDLE_ID - 1] && dats[7 + dats[6]] == 0x0d)
				if(dats[5] == 0x10){
					
					memcpy(dats_rx,(const void*)&dats[7],dats[6]);
					
					FLG_CLO = dats_rx[0];
					
					Driver_USART1.Send(dats_rx,dats[6]);		
				}			
			memset(FRAME_RX,0,30*sizeof(uint8_t));
		}	
		osDelay(500);	
#endif
		
#if(MOUDLE_ID == 9)	
		char temp_shi = (char)SHT11_temp;
		char temp_ge  = (char)((SHT11_temp-(float)temp_shi)*100);
		char hum_shi = (char)SHT11_hum;
		char hum_ge  = (char)((SHT11_hum-(float)hum_shi)*100);
		dats[0] = temp_shi;
		dats[1] = temp_ge;
		dats[2] = hum_shi;
		dats[3] = hum_ge;
		FRAME_TX_DATSLOAD(dats);
		
#elif(MOUDLE_ID == 12)
		
		
#endif

#if(MOUDLE_ID <= 10)
		Driver_USART2.Send((void *)FRAME_TX,strlen((void *)FRAME_TX));
		//USART_SendData(USART2,'a');
		osDelay(500);
		memset(FRAME_TX,0,20*sizeof(char));
		
		osDelay(1500);	
#endif
	}
}

void USART1Test(void){
	
	tid_USART1Test_Thread = osThreadCreate(osThread(USART1Test_Thread),NULL);
}

void USART2Test(void){
	
	tid_USART2Test_Thread = osThreadCreate(osThread(USART2Test_Thread),NULL);
}

void FRAME_TX_DATSLOAD(uint8_t dats[]){

	uint8_t datsLength = strlen((const char*)dats);
	uint8_t framePt	 = 0;
	
	framePt += (sizeof(FRAME_PRT1) / sizeof(FRAME_PRT1[0]) - 1);
	memcpy((uint8_t *)FRAME_TX,(uint8_t *)FRAME_PRT1,framePt);
	FRAME_TX[framePt ++] = (char)MOUDLE_TYPE[MOUDLE_ID - 1];
	FRAME_TX[framePt ++] = CMD_DATA_TX;
	FRAME_TX[framePt ++] = datsLength;
	memcpy((char*)(FRAME_TX+framePt),dats,datsLength);
	framePt += datsLength;
	FRAME_TX[framePt] = FRAME_TAIL;
}
