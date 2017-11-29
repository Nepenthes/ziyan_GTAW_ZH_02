#include "ElecMeter.h"

uint8_t Elec_Param[13];
float 	valDianYa;
float 	valDianLiu;
float		valGongLv;
double 	valDianLiang;

osThreadId tid_USART4ElecMeter_Thread;
osThreadDef(USART4ElecMeter_Thread,osPriorityAboveNormal,1,512);

extern ARM_DRIVER_USART Driver_USART1;
extern ARM_DRIVER_USART Driver_USART2;
extern ARM_DRIVER_USART Driver_USART4;

void myUSART4_callback(uint32_t event) {

//    uint32_t mask;
//	
//    mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
//           ARM_USART_EVENT_TRANSFER_COMPLETE |
//           ARM_USART_EVENT_SEND_COMPLETE     |
//           ARM_USART_EVENT_TX_COMPLETE       ;
	
	if(event & ARM_USART_EVENT_TX_COMPLETE){
	
		CON485READ;
	}
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
	
		//CON485SEND;
	}
//    if (event & mask) {
//        /* Success: Wakeup Thread */
//        osSignalSet(tid_myUART_Thread, 0x01);
//    }
//    if (event & ARM_USART_EVENT_RX_TIMEOUT) {
//        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//    }
//    if (event & (ARM_USART_EVENT_RX_OVERFLOW | ARM_USART_EVENT_TX_UNDERFLOW)) {
//        __breakpoint(0);  /* Error: Call debugger or replace with custom error handling */
//    }
}

void Con485_Init(void) {

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);		//时钟分配

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;	//端口属性分配
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

void USART4ElecMeter_Init(void) {

    Con485_Init();
    /*Initialize the USART driver */
    Driver_USART4.Initialize(myUSART4_callback);
    /*Power up the USART peripheral */
    Driver_USART4.PowerControl(ARM_POWER_FULL);
    /*Configure the USART to 115200 Bits/sec */
    Driver_USART4.Control(ARM_USART_MODE_ASYNCHRONOUS |
                          ARM_USART_DATA_BITS_8 |
                          ARM_USART_PARITY_NONE |
                          ARM_USART_STOP_BITS_1 |
                          ARM_USART_FLOW_CONTROL_NONE, 4800);

    /* Enable Receiver and Transmitter lines */
    Driver_USART4.Control (ARM_USART_CONTROL_TX, 1);
    Driver_USART4.Control (ARM_USART_CONTROL_RX, 1);

    CON485SEND;

    Driver_USART4.Send("i'm usart4 \r\n", 13);
}

uint8_t SumArray(uint8_t array[],uint8_t sum) {

    uint8_t loop,SUM;

    for(loop = 0; loop < sum; loop ++)SUM += array[loop];

    return SUM;
}

void USART4ElecMeter_Thread(const void *argument) {

    const uint8_t BUFTX_SIZE = 8;
    const uint8_t BUFRX_SIZE = 20;
    const char ENQ_CMD[BUFTX_SIZE] = {0x01,0x03,0x00,0x48,0x00,0x05,0x05,0xdf};
    const char ANS_FLG[4] = {0x01,0x03,0x0a};
    char RX_buff[BUFRX_SIZE];
    char debug_str[30];
    uint16_t temp_dianya,temp_dianliu;
//	 uint16_t temp_gonglv; 原功率参数测量不准，转换缓冲暂时不用;
    uint32_t temp_dianliang;

    osDelay(1000);

    for(;;) {

        while(memcmp(ANS_FLG,(const char*)RX_buff,3) != 0) {

            memset(RX_buff,0,BUFRX_SIZE * sizeof(char));
			   CON485SEND;
            Driver_USART4.Send((char*)(ENQ_CMD),BUFTX_SIZE);
			   osDelay(20);	//我也不知道为什么只有20ms，长了不行，短了也不行，醉了
            Driver_USART4.Receive(RX_buff,BUFRX_SIZE);
				osDelay(1000);
        }
#if(MOUDLE_DEBUG == 1)		  
		  Driver_USART1.Send("电参数已更新：\r\n",16);
		  osDelay(200);
#endif
		  
		  temp_dianya  = 0;
        temp_dianya |= (uint16_t)RX_buff[3] << 8;
        temp_dianya |= (uint16_t)RX_buff[4];
        valDianYa = (float)temp_dianya / 100.0;

        Elec_Param[0] = (uint8_t)((uint32_t)valDianYa % 10000 / 100);
        Elec_Param[1] = (uint8_t)((uint32_t)valDianYa % 100);
        Elec_Param[2] = (uint8_t)((uint32_t)(valDianYa * 100.0)) % 100;
#if(MOUDLE_DEBUG == 1)
        sprintf(debug_str,"电压：%.2f V\r\n",valDianYa);
        Driver_USART1.Send((const char*)debug_str,strlen((const char*)debug_str));
        osDelay(200);
#endif
		  
		  temp_dianliu  = 0;
        temp_dianliu |= (uint16_t)RX_buff[5] << 8;
        temp_dianliu |= (uint16_t)RX_buff[6];
        valDianLiu = (float)temp_dianliu / 1000.0;

        Elec_Param[3] = (uint8_t)((uint32_t)valDianLiu % 100);
        Elec_Param[4] = (uint8_t)((uint32_t)(valDianLiu * 100.0)) % 100;
#if(MOUDLE_DEBUG == 1)
        sprintf(debug_str,"电流：%.2f A\r\n",valDianLiu);
        Driver_USART1.Send((const char*)debug_str,strlen((const char*)debug_str));
        osDelay(200);
#endif

//		  	 temp_gonglv  = 0;											//因为电参数测量不准，直接取P = UI.
//        temp_gonglv |= (uint16_t)RX_buff[7] << 8;
//        temp_gonglv |= (uint16_t)RX_buff[8];
//        valGongLv = (float)temp_gonglv / 10.0;
			
		  valGongLv = valDianYa * valDianLiu;

        Elec_Param[5] = (uint8_t)((uint32_t)valGongLv % 10000 / 100);
        Elec_Param[6] = (uint8_t)((uint32_t)valGongLv % 100);
        Elec_Param[7] = (uint8_t)((uint32_t)((valGongLv - Elec_Param[6]) * 100.0)) % 100;
#if(MOUDLE_DEBUG == 1)
        sprintf(debug_str,"功率：%.2f W\r\n",valGongLv);
        Driver_USART1.Send((const char*)debug_str,strlen((const char*)debug_str));
        osDelay(200);
#endif

		  temp_dianliang  = 0;
        temp_dianliang |= (uint32_t)RX_buff[9] << 24;
        temp_dianliang |= (uint32_t)RX_buff[10] << 16;
        temp_dianliang |= (uint32_t)RX_buff[11] << 8;
        temp_dianliang |= (uint32_t)RX_buff[12];
        valDianLiang = (double)temp_dianliang / 100.0;

        Elec_Param[8]  = (uint8_t)((uint64_t)valDianLiang % 100000000 / 1000000);
        Elec_Param[9]  = (uint8_t)((uint64_t)valDianLiang % 1000000 / 10000);
        Elec_Param[10] = (uint8_t)((uint64_t)valDianLiang % 10000 / 100);
        Elec_Param[11] = (uint8_t)((uint64_t)valDianLiang % 100);
        Elec_Param[12] = (uint8_t)((uint64_t)((valDianLiang - Elec_Param[11]) * 100.0)) % 100;
#if(MOUDLE_DEBUG == 1)
        sprintf(debug_str,"用电量：%.2f Wh\r\n",valDianLiang);
        Driver_USART1.Send((const char*)debug_str,strlen((const char*)debug_str));
		  osDelay(20);
		  Driver_USART1.Send("------------------------------\r\n\r\n",34);
        osDelay(200);
#endif
		  memset(RX_buff,0,BUFRX_SIZE * sizeof(char));
        osDelay(200);
    }
}

void USART4ElecMeter(void) {

    tid_USART4ElecMeter_Thread = osThreadCreate(osThread(USART4ElecMeter_Thread),NULL);
}
