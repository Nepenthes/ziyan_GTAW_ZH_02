#include "airWarming.h"

float valDS18B20;
uint8_t SW_airWarming;
uint8_t AWM_STATUS = 0x02;

uint8_t USRKawmTX_FLG = 0;
uint8_t USRKawmRX_FLG = 0;

osThreadId tid_airWarmingCM_Thread;
osThreadDef(airWarmingCM_Thread,osPriorityNormal,1,1024);

extern ARM_DRIVER_USART Driver_USART1;								//�豸�����⴮��һ�豸����

//��λDS18B20
void DS18B20_Rst(void)
{
    CLR_DS18B20();    //����DQ
    delay_us(750);    //����750us
    SET_DS18B20();;   //DQ=1
    delay_us(15);     //15US
}


//�ȴ�DS18B20�Ļ�Ӧ
//����1:δ��⵽ds18b20�Ĵ���
//����0:����
uint8_t DS18B20_Check(void)
{
    uint8_t retry=0;

    while (DS18B20_DQ_IN&&retry<200)
    {
        retry++;
        delay_us(1);
    };
    if(retry>=200)return 1;
    else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
    {
        retry++;
        delay_us(1);
    };
    if(retry>=240)return 1;
    return 0;
}


//��DS18B20��ȡһ��λ
//����ֵ:1/0
uint8_t DS18B20_Read_Bit(void)           // read one bit
{
    uint8_t data;

    CLR_DS18B20();   //����DQ
    delay_us(2);
    SET_DS18B20();;  //DQ=1
    delay_us(12);
    if(DS18B20_DQ_IN)data=1;
    else data=0;
    delay_us(50);

    return data;
}


//��ds18b20��ȡһ���ֽ�
//����ֵ:��ȡ��������
uint8_t DS18B20_Read_Byte(void)    // read one byte
{
    uint8_t i,j,dat;
    dat=0;

    for (i=1; i<=8; i++)
    {
        j=DS18B20_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }
    return dat;
}

//*�������� www.stm32cube.com �ṩ
//дһ���ֽڵ�DS18B20
//dat:Ҫд����ֽ�
void DS18B20_Write_Byte(uint8_t dat)
{
    uint8_t j;
    uint8_t testb;

    for (j=1; j<=8; j++)
    {
        testb=dat&0x01;
        dat=dat>>1;
        if (testb)
        {
            CLR_DS18B20(); //DS18B20_DQ_OUT=0;// Write 1
            delay_us(2);
            SET_DS18B20(); //DS18B20_DQ_OUT=1;
            delay_us(60);
        }
        else
        {
            CLR_DS18B20(); //DS18B20_DQ_OUT=0;// Write 0
            delay_us(60);
            SET_DS18B20(); //DS18B20_DQ_OUT=1;
            delay_us(2);
        }
    }
}


//��ʼ�¶�ת��
void DS18B20_Start(void)// ds1820 start convert
{
    DS18B20_Rst();
    DS18B20_Check();
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
}


//��ʼ��DS18B20��IO��DQͬʱ���DS�Ĵ���
//����1:������
//����0:����
uint8_t DS18B20_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
    SET_DS18B20();        

    DS18B20_Rst();
    return DS18B20_Check();
}

void Warming_Init(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	
	WARMING_OFF;
}

void airWarming_Key_Init(void){

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

//��ds18b20�õ��¶�ֵ
//���ȣ�0.1C
//����ֵ���¶�ֵ ��-550~1250��
float DS18B20_Get_Temp(void)
{
    uint8_t temp;
    uint8_t TL,TH;
    int16_t tem;
    DS18B20_Start ();                    // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert
    TL=DS18B20_Read_Byte(); // LSB
    TH=DS18B20_Read_Byte(); // MSB

    if(TH>7)
    {
        TH=~TH;
        TL=~TL;
        temp=0;//�¶�Ϊ��
    } else temp=1;//�¶�Ϊ��
    tem=TH; //��ø߰�λ
    tem<<=8;
    tem+=TL;//��õͰ�λ
    tem=(float)tem*0.625;//ת��
    if(temp)return tem/10.00; //�����¶�ֵ
    else return -tem/10.00;
}

void airWarming_Init(void){

	DS18B20_Init();
	Warming_Init();
	airWarming_Key_Init();
}

u8 KEY_Scan1_AWM(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(swKey_airWarming==0))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(swKey_airWarming==0)return 1;
	}else if(swKey_airWarming==1)key_up=1; 	    
 	return 0;// �ް�������
}

void airWarmingCM_Thread(const void *argument){

#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	u8 key_in;
	u8 key_flg=0;
	
	for(;;){
		
//		USRKawmTX_FLG = 1;		 //�����������ߣ�������ã�ע�ͼ��ɣ�ע�ͺ��򱻶����ߣ�״̬�ı�ʱ���Ϸ�״̬��Ϣ
		
		key_in = KEY_Scan1_AWM(0);
		
		valDS18B20 	= DS18B20_Get_Temp();
		
		if(valDS18B20 > 40.0){		//�¶����ƣ���ʮ�����²ſ��ſ���Ȩ
		
			AWM_STATUS = 0x02;
			
		}else{
			
			if(key_in == 1){
			
				key_flg = !key_flg; 
				
				if(key_flg){
					 
					AWM_STATUS = 0x01;
				}else{
				
					AWM_STATUS = 0x02;
				}	
				USRKawmTX_FLG = 1;
				Beep_time(80);
			}
			
			if(USRKawmRX_FLG == 1){
				
				USRKawmRX_FLG = 0;
			
				key_flg = !key_flg; 
				AWM_STATUS = SW_airWarming;
				Beep_time(80);
			}
		}

		if(AWM_STATUS == 0x01)WARMING_ON;
		else WARMING_OFF;
		
#if(MOUDLE_DEBUG == 1)	
		sprintf(disp,"\n\rairtemp valDS18B20 : %.1f\n\r", valDS18B20);			
		Driver_USART1.Send(disp,strlen(disp));
#endif	
		
	osDelay(200);
	}
}

void airWarming(void){

	tid_airWarmingCM_Thread = osThreadCreate(osThread(airWarmingCM_Thread),NULL);
}

