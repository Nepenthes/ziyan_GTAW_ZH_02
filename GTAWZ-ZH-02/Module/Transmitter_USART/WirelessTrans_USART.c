#include <WirelessTrans_USART.h>

uint8_t TX_RSQ;	//数据发送请求
uint16_t RX_NUM = 0; //剥壳后数据长（去掉帧头尾等地址后的数据长度）

extern osMutexId (uart1_mutex_id);
#if(MOUDLE_ID == 1)
extern uint8_t valKeyBoard;
#elif(MOUDLE_ID == 2)
extern uint8_t RC522IDBUF[4]; 
#elif(MOUDLE_ID == 3)
extern uint8_t phoneticsNUM;
#elif(MOUDLE_ID == 4)
extern uint8_t valAnalog;
extern uint8_t valDigital;
#elif(MOUDLE_ID == 5)
extern uint32_t LUXValue;
#elif(MOUDLE_ID == 6)
extern float valcontentCO2;
#elif(MOUDLE_ID == 7)
extern uint8_t isSomeone;
#elif(MOUDLE_ID == 8)
extern uint8_t isRain;
#elif(MOUDLE_ID == 9)
extern float SHT11_hum; 
extern float SHT11_temp;
#elif(MOUDLE_ID == 10)
extern float result_UP;
extern float result_UA;
#elif(MOUDLE_ID == 11)
extern double valwindSpeed;
#elif(MOUDLE_ID == 12)
extern uint8_t	DispLAattr; // HA:1   color:2  slip :1  speed:4
extern uint8_t Disp_num;
//extern uint8_t DispLABuffer[DISPLA_BUFFER_SIZE];
#elif(MOUDLE_ID == 14)
extern float valsoilHum;
#elif(MOUDLE_ID == 15)
extern uint8_t curAction[2];
extern uint8_t USRKcurTX_FLG;
extern uint8_t USRKcurRX_FLG;
#elif(MOUDLE_ID == 16)
extern uint8_t SW_SPY;
extern uint8_t SW_PST;
extern uint8_t USRKspyTX_FLG;
extern uint8_t USRKspyRX_FLG;
extern uint8_t SW_STATUS[2];
#elif(MOUDLE_ID == 17)
extern uint16_t PWM_exAir;
extern uint8_t USRKexaTX_FLG;
extern uint8_t USRKexaRX_FLG;
#elif(MOUDLE_ID == 18)
extern float valDS18B20;
extern uint8_t SW_airWarming;
extern uint8_t AWM_STATUS;
extern uint8_t USRKawmTX_FLG;
extern uint8_t USRKawmRX_FLG;		//保留暂时不用
#elif(MOUDLE_ID == 19)
extern uint16_t PWM_ledGRW;
extern uint8_t USRKgrwTX_FLG;
extern uint8_t USRKgrwRX_FLG;	
#elif(MOUDLE_ID == 20)
extern float valVoltage;
extern uint8_t SOURCE_TYPE;
extern uint8_t USRKpowTX_FLG;
extern uint8_t USRKpowexaRX_FLG;	//保留暂时不用
#elif(MOUDLE_ID == 21)
extern uint8_t swRelay_lock;
extern uint8_t swRelay_light;
extern uint8_t USRKjdqTX_FLG;
extern uint8_t USRKjdqRX_FLG;
#elif(MOUDLE_ID == 22)
extern uint8_t Elec_Param[13];
#endif

const uint8_t MOUDLE_TYPE[MOUDLE_NUM] = {
	
	GTA_GGI01,GTA_GGI02,GTA_GGI03,GTA_GGS01,
	GTA_GGS02,GTA_GGS03,GTA_GGS04,GTA_GGS05,
	GTA_GGS06,GTA_GGS07,GTA_GG001,GTA_GID01,
	GTA_GID02,GTA_GID03,GTA_PB002,GTA_PB003,
	GTA_PB004,GTA_PB005,GTA_PB006,GTA_PBP01,
	GTA_PBP02,GTA_PBP03
};

const uint8_t FRAME_PRT1[5] = {FRAME_HEAD,GATEWAY_ADDR,NODE_ADDR,MODULE_ADDR};  //本地模块帧头对号入座

const char *TestCMD[] = {

	"天王盖地虎"
};

const char *TestREP[] = {

	"小鸡炖蘑菇\r\n"
};

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;

osThreadId tid_USART1Debug_Thread;
osThreadId tid_USART2Trans_Thread;

osThreadDef(USART1Debug_Thread,osPriorityNormal,1,256);
osThreadDef(USART2Trans_Thread,osPriorityNormal,1,4096);

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
#if(MOUDLE_ID != 10)
	/* Configure USART1 Rx (PA.10) as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#endif
	  
	/* USART1 mode config */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
#if(MOUDLE_ID != 10)
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
#else
	USART_InitStructure.USART_Mode = USART_Mode_Tx;
#endif
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
#if(MOUDLE_ID != 10)
	Driver_USART1.Control (ARM_USART_CONTROL_RX, 1);
#endif

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

//	uint32_t mask = ARM_USART_EVENT_RX_TIMEOUT;
//	char rxcount;
//	
//	if(event & mask){
//	
//		rxcount = Driver_USART2.GetRxCountNx();
//		Driver_USART1.Send(&rxcount,1);
//	}
}

void USART1Debug_Thread(const void *argument){

	char cmd[30] = "abc";
	
	for(;;){

		osDelay(10);													//必需延时，防乱序
		Driver_USART1.Receive(cmd,strlen(TestCMD[0]));
		//if(strstr(cmd,TestCMD[0])){								//子串比较
		if(!strcmp(TestCMD[0],cmd)){								//全等比较
				
			osMutexWait(uart1_mutex_id,osWaitForever);
			Driver_USART1.Send((char*)(TestREP[0]),strlen((char*)(TestREP[0])));
			osMutexRelease(uart1_mutex_id);
			memset(cmd,0,30*sizeof(char));
		}
	}
}

void USART2Trans_Thread(const void *argument){

	uint8_t dats[FRAME_RX_SIZE];
	
#if(MOUDLE_ID == 2)
	const uint8_t dats_null[20] = {0};
#endif
	
#if(MOUDLE_ID == 1)
	char keyVal_ID1;
#elif(MOUDLE_ID == 2)
	char RC522IDBUF_ID4[4] = {0}; 
#elif(MOUDLE_ID == 3)
	char valPhonenum_ID3;
#elif(MOUDLE_ID == 4)
	char valDigital_ID4 = (char)valDigital;
	char valAnalog_ID4  = (char)valAnalog;
#elif(MOUDLE_ID == 5)	
	char temp_wan_ID5 = (char)(LUXValue % 1000000 / 10000);
	char temp_bai_ID5 = (char)(LUXValue % 10000 / 100);
	char temp_ge_ID5  = (char)(LUXValue % 100);
#elif(MOUDLE_ID == 6)
	char valCO2_CO2_bai_ID6 = (char)((long)valcontentCO2 % 10000 / 100);
	char valCO2_CO2_ge_ID6 = (char)((long)valcontentCO2 % 100);
#elif(MOUDLE_ID == 7)
	char isSomeone_ID7 = (char)isSomeone;
#elif(MOUDLE_ID == 8)
	char isRain_ID8 = (char)isRain;
#elif(MOUDLE_ID == 9)
	char temp_zhengshu_ID9 = (char)SHT11_temp;
	char temp_xiaoshu_ID9  = (char)((SHT11_temp - (float)temp_zhengshu_ID9)*100);
	char hum_zhengshu_ID9  = (char)SHT11_hum;
	char hum_xiaoshu_ID9   = (char)((SHT11_hum - (float)hum_zhengshu_ID9)*100);
#elif(MOUDLE_ID == 10)
	float result_UP_TMP = result_UP / 1000;
	char UP_zhengshu_ID10 = (char)result_UP_TMP;
	char UP_xiaoshu_ID10  = (char)((result_UP_TMP - (float)UP_zhengshu_ID10)*100);
	char UA_zhengshu_ID10 = (char)result_UA;
	char UA_xiaoshu_ID10  = (char)((result_UA - (float)UA_zhengshu_ID10)*100);
#elif(MOUDLE_ID == 11)
	char valwind_zhengshu_ID11 = (char)valwindSpeed;
	char valwind_xiaoshu_ID11 = (char)((valwindSpeed - (double)valwind_zhengshu_ID11) * 100);
#elif(MOUDLE_ID == 12)
	uint16_t num_ID12;
#elif(MOUDLE_ID == 14)
	char valsoil_zhengshu_ID14 = (char)valsoilHum;
	char valsoil_xiaoshu_ID14 = (char)((valsoilHum - (float)valsoil_zhengshu_ID14) * 100);
#elif(MOUDLE_ID == 15)
	char SW_curtain_ID15[2] = {0};
#elif(MOUDLE_ID == 16)
	char SW_ledSPY_ID16[2] = {0};
#elif(MOUDLE_ID == 17)
	char PWM_exAir_ID17  = 0;
#elif(MOUDLE_ID == 18)
	char temp_zhengshu_ID18 = (char)valDS18B20;
	char temp_xiaoshu_ID18 = (char)((valDS18B20 - (float)temp_zhengshu_ID18)*100);
	char sw_awm_ID18;
#elif(MOUDLE_ID == 19)
   char PWM_ledGRW_ID19  = 0;
#elif(MOUDLE_ID == 20)
	char valVol_zhengshu_ID20 = (char)valVoltage;
	char valVol_xiaoshu_ID20 = (char)((valVoltage - (float)valVol_zhengshu_ID20) * 100);
	char SWSource_ID20;
#elif(MOUDLE_ID == 21)
	char swRelay_lock_ID20;
	char swRelay_light_ID20;
#elif(MOUDLE_ID == 22)
   uint8_t Elec_Param_ID22[13] = {0};
#endif
	
	for(;;){
		
#if(MOUDLE_ID > 11)
		char *pt;	
		uint8_t dats_rx[FRAME_DATS_SIZE];
	
		Driver_USART2.Receive(FRAME_RX,72);
		
		pt = strstr((const char*)FRAME_RX,(const char*)FRAME_PRT1);
	
		if(pt){
			
			Driver_USART1.Send(FRAME_RX,pt[6]);
			
			memset(dats,0,FRAME_RX_SIZE*sizeof(uint8_t));
			memcpy(dats,pt,8 + pt[6]);
			RX_NUM = dats[6];
			
			if(dats[4] == MOUDLE_TYPE[MOUDLE_ID - 1] && dats[7 + dats[6]] == 0x0d)
				if(dats[5] == 0x10){
					
					memset(dats_rx,0,FRAME_DATS_SIZE*sizeof(uint8_t));
					memcpy(dats_rx,(const void*)&dats[7],dats[6]);			
				}			
			memset(FRAME_RX,0,FRAME_RX_SIZE*sizeof(uint8_t));
		}	
		osDelay(20);	
#endif
//-----------------------------------------------------------------------传感器，主要为发送数据		
#if(MOUDLE_ID == 1)	 //键盘
		if((keyVal_ID1 != (char)valKeyBoard) && valKeyBoard){
		
			keyVal_ID1 = (char)valKeyBoard;
			dats[0] = keyVal_ID1;
			FRAME_TX_DATSLOAD(dats,1);
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 2)	//RFID
		if(memcmp(RC522IDBUF_ID4,RC522IDBUF,4) && memcmp(dats_null,RC522IDBUF,4)){
		
			memcpy(RC522IDBUF_ID4,RC522IDBUF,4);
			memcpy(dats,RC522IDBUF_ID4,4);
			FRAME_TX_DATSLOAD(dats,4);
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 3)	//语音
		if((valPhonenum_ID3 != phoneticsNUM) && phoneticsNUM){
		
			valPhonenum_ID3 = phoneticsNUM;
			dats[0] = valPhonenum_ID3;
			FRAME_TX_DATSLOAD(dats,1);
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 4)	//信号模拟
		valDigital_ID4 = (char)valDigital;
		valAnalog_ID4 = (char)valAnalog;
		
		dats[0] = valDigital_ID4;
	   dats[1] = valAnalog_ID4;
		FRAME_TX_DATSLOAD(dats,2);
		TX_RSQ = 1;
#elif(MOUDLE_ID == 5)  //光照强度
		temp_wan_ID5 = (char)(LUXValue % 1000000 / 10000);
		temp_bai_ID5 = (char)(LUXValue % 10000 / 100);
		temp_ge_ID5  = (char)(LUXValue % 100);
		
		dats[0] = temp_wan_ID5;
	   dats[1] = temp_bai_ID5;
		dats[2] = temp_ge_ID5;
		FRAME_TX_DATSLOAD(dats,3);	
		TX_RSQ = 1;
#elif(MOUDLE_ID == 6)	//二氧化碳
		valCO2_CO2_bai_ID6 = (char)((long)valcontentCO2 % 10000 / 100);
		valCO2_CO2_ge_ID6 = (char)((long)valcontentCO2 % 100);
		
		dats[0] = valCO2_CO2_bai_ID6;
		dats[1] = valCO2_CO2_ge_ID6;
		FRAME_TX_DATSLOAD(dats,2);	
		TX_RSQ = 1;
#elif(MOUDLE_ID == 7)	//红外热释电
		isSomeone_ID7 = (char)isSomeone;
		
		dats[0] = isSomeone_ID7;
		FRAME_TX_DATSLOAD(dats,1);
		TX_RSQ = 1;	
#elif(MOUDLE_ID == 8)  //雨水
		isRain_ID8 = (char)isRain;
		
		dats[0] = isRain_ID8;
		FRAME_TX_DATSLOAD(dats,1);
		TX_RSQ = 1;
#elif(MOUDLE_ID == 9)	//温湿度
	   temp_zhengshu_ID9 = (char)SHT11_temp;
		temp_xiaoshu_ID9  = (char)((SHT11_temp-(float)temp_zhengshu_ID9)*100);
		hum_zhengshu_ID9  = (char)SHT11_hum;
		hum_xiaoshu_ID9   = (char)((SHT11_hum-(float)hum_zhengshu_ID9)*100);
		
		dats[0] = temp_zhengshu_ID9;
		dats[1] = temp_xiaoshu_ID9;
		dats[2] = hum_zhengshu_ID9;
		dats[3] = hum_xiaoshu_ID9;
		FRAME_TX_DATSLOAD(dats,4);
		TX_RSQ = 1;
#elif(MOUDLE_ID == 10)		//大气压海拔
		result_UP_TMP = result_UP / 1000;
		UP_zhengshu_ID10 = (char)result_UP_TMP;
		UP_xiaoshu_ID10  = (char)((result_UP_TMP - (float)UP_zhengshu_ID10)*100);
		UA_zhengshu_ID10 = (char)result_UA;
		UA_xiaoshu_ID10  = (char)((result_UA - (float)UA_zhengshu_ID10)*100);
			
		dats[0] = UP_zhengshu_ID10;
		dats[1] = UP_xiaoshu_ID10;
		dats[2] = UA_zhengshu_ID10;
		dats[3] = UA_xiaoshu_ID10;
		FRAME_TX_DATSLOAD(dats,4);
		TX_RSQ = 1;
#elif(MOUDLE_ID == 11)	//风速
		valwind_zhengshu_ID11 = (char)valwindSpeed;
		valwind_xiaoshu_ID11 = (char)((valwindSpeed - (double)valwind_zhengshu_ID11) * 100);
		
		dats[0] = valwind_zhengshu_ID11;
		dats[1] = valwind_xiaoshu_ID11;
		FRAME_TX_DATSLOAD(dats,2);	
		TX_RSQ = 1;
//-----------------------------------------------------------------------执行器，主要为接收命令处理		
#elif(MOUDLE_ID == 12) //显示屏

		if(num_ID12 != dats_rx[1]){
		
			num_ID12 	= dats_rx[0];
			DispLAattr 	= dats_rx[0];
			Disp_num		= dats_rx[1];
		}
//		memset(DispLABuffer,0,DISPLA_BUFFER_SIZE*sizeof(char));
//		memcpy(DispLABuffer,(const uint8_t *)&dats_rx[1],num_ID12);

#elif(MOUDLE_ID == 14)
		valsoil_zhengshu_ID14 = (char)valsoilHum;
		valsoil_xiaoshu_ID14 = (char)((valsoilHum - (float)valsoil_zhengshu_ID14) * 100);
		
		dats[0] = valsoil_zhengshu_ID14;
		dats[1] = valsoil_xiaoshu_ID14;
		FRAME_TX_DATSLOAD(dats,2);	
		TX_RSQ = 1;	
#elif(MOUDLE_ID == 15) //窗帘
		if(SW_curtain_ID15[0] != dats_rx[0]){
		
			SW_curtain_ID15[0] = dats_rx[0];
			curAction[0]		 = dats_rx[0];	
			if(curAction[1] > 100)curAction[1] = 100;
			
			USRKcurRX_FLG = 1;
		}
		
		if(USRKcurTX_FLG){
		
			USRKcurTX_FLG = 0;
			
			SW_curtain_ID15[0] = dats_rx[0] = curAction[0];		//更新下行数据缓存，防错乱		
						
			memcpy(dats,curAction,2);
			
			if(dats[0] == 0x03 && !curAction[1])dats[0] = 4;  //开停 、闭停判断
			
			FRAME_TX_DATSLOAD(dats,2);
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 16)  //喷雾
		if(memcmp(SW_ledSPY_ID16,dats_rx,2)){
		
			memcpy(SW_ledSPY_ID16,dats_rx,2);
			
			SW_SPY = dats_rx[0];
			SW_PST = dats_rx[1];
			
			USRKspyRX_FLG = 1;
		}
		
		if(USRKspyTX_FLG){
			
			USRKspyTX_FLG = 0;
			
			memcpy(dats_rx,SW_STATUS,2);		//更新下行数据缓存，防错乱
			memcpy(SW_ledSPY_ID16,SW_STATUS,2);
			
			memcpy(dats,SW_STATUS,2);
			FRAME_TX_DATSLOAD(dats,2);
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 17)	//排风扇
		if(PWM_exAir_ID17 != dats_rx[0]){
		
			PWM_exAir_ID17 = dats_rx[0];
			PWM_exAir	   = ((uint16_t)dats_rx[0]) * 60;
			
			USRKexaRX_FLG = 1;
		}	
		
		if(USRKexaTX_FLG){
			
			USRKexaTX_FLG = 0;
			
			PWM_exAir_ID17 = dats_rx[0] = PWM_exAir / 60;   //更新下行数据缓存，防错乱
			
			dats[0] = (char)(PWM_exAir / 60);
			FRAME_TX_DATSLOAD(dats,1);	
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 18)	//加热管
		temp_zhengshu_ID18 = (char)valDS18B20;
		temp_xiaoshu_ID18 = (char)((valDS18B20 - (float)temp_zhengshu_ID18)*100);
		
		if(sw_awm_ID18 != dats_rx[0]){
		
			sw_awm_ID18 = dats_rx[0];
			SW_airWarming = dats_rx[0];
			
			USRKawmRX_FLG = 1;
		}	
		
		dats[0] = AWM_STATUS;
		dats[1] = temp_zhengshu_ID18;
		dats[2] = temp_xiaoshu_ID18;
		if(USRKawmTX_FLG == 1){
			
			USRKawmTX_FLG = 0;	
			sw_awm_ID18 = dats_rx[0] = AWM_STATUS;	//更新下行数据缓存，防错乱
		}
		FRAME_TX_DATSLOAD(dats,3);
		TX_RSQ = 1;
#elif(MOUDLE_ID == 19)	//生长灯
		if(PWM_ledGRW_ID19 != dats_rx[0]){
		
			PWM_ledGRW_ID19 = dats_rx[0];
			PWM_ledGRW	   = ((uint16_t)dats_rx[0]) * 60;
			
			USRKgrwRX_FLG = 1;
		}	
		
		if(USRKgrwTX_FLG){
			
			USRKgrwTX_FLG = 0;
			
			PWM_ledGRW_ID19 = dats_rx[0] = PWM_ledGRW / 60;   //更新下行数据缓存，防错乱
			
			dats[0] = (char)(PWM_ledGRW / 60);
			FRAME_TX_DATSLOAD(dats,1);	
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 20)		//直流电源切换
		valVol_zhengshu_ID20 = (char)valVoltage;
		valVol_xiaoshu_ID20 = (char)((valVoltage - (float)valVol_zhengshu_ID20) * 100);
		
		dats[0] = valVol_zhengshu_ID20;
		dats[1] = valVol_xiaoshu_ID20;
		dats[2] = SOURCE_TYPE;
		if(USRKpowTX_FLG == 1){
			
			USRKpowTX_FLG = 0;	
			SWSource_ID20 = dats_rx[0] = SOURCE_TYPE;	//更新下行数据缓存，防错乱
		}
		FRAME_TX_DATSLOAD(dats,3);	
		TX_RSQ = 1;
		
		if(SWSource_ID20 != dats_rx[0]){
		
			SWSource_ID20 = dats_rx[0];
			SOURCE_TYPE = dats_rx[0];
		}
#elif(MOUDLE_ID == 21)		//继电器控制
		if(swRelay_lock_ID20 != dats_rx[0] || swRelay_light_ID20 != dats_rx[1]){
		
			if(dats_rx[0])swRelay_lock = swRelay_lock_ID20 = dats_rx[0];
			if(dats_rx[1])swRelay_light = swRelay_light_ID20 = dats_rx[1];
			
			USRKjdqRX_FLG = 1;
		}
		
		if(USRKjdqTX_FLG == 1){
		
			USRKjdqTX_FLG = 0;
			
			dats_rx[0] = dats[0] = swRelay_lock;     //更新下行数据缓存，防错乱
			dats_rx[1] = dats[1] = swRelay_light;
			
			FRAME_TX_DATSLOAD(dats,2);
			TX_RSQ = 1;
		}
#elif(MOUDLE_ID == 22)	//交流总电源检测
		memcpy(Elec_Param_ID22,Elec_Param,13);
		memcpy(dats,Elec_Param_ID22,13);
		FRAME_TX_DATSLOAD(dats,13);	
		TX_RSQ = 1;
#endif
		
//为了兼并执行构件的数据上传，允许数据发送
#if(MOUDLE_ID <= 11 || MOUDLE_ID == 14 || MOUDLE_ID == 15 || MOUDLE_ID == 16 || MOUDLE_ID == 17 || MOUDLE_ID == 18 || MOUDLE_ID == 19 || MOUDLE_ID == 20 || MOUDLE_ID == 21 || MOUDLE_ID == 22)
		if(TX_RSQ){
		
			Driver_USART2.Send((void *)FRAME_TX,8 + FRAME_TX[6]);
			osDelay(500);
			memset(FRAME_TX,0,20*sizeof(char));
			TX_RSQ = 0;
		}
		osDelay(500);	
#endif
	}
}

void FRAME_TX_DATSLOAD(uint8_t dats[],uint8_t datsLength){

	uint8_t framePt	 = 0;
	
	framePt += (sizeof(FRAME_PRT1) / sizeof(FRAME_PRT1[0]) - 1);
	memcpy((uint8_t *)FRAME_TX,(uint8_t *)FRAME_PRT1,framePt);
	FRAME_TX[framePt ++] = (char)MOUDLE_TYPE[MOUDLE_ID - 1];
	FRAME_TX[framePt ++] = CMD_DATA_TX;
	FRAME_TX[framePt ++] = datsLength;
	memcpy((char*)(FRAME_TX+framePt),(const void*)dats,datsLength);
	framePt += datsLength;
	FRAME_TX[framePt] = FRAME_TAIL;
}

void USART1Debug(void){
	
	tid_USART1Debug_Thread = osThreadCreate(osThread(USART1Debug_Thread),NULL);
}

void USART2Trans(void){
	
	tid_USART2Trans_Thread = osThreadCreate(osThread(USART2Trans_Thread),NULL);
}


