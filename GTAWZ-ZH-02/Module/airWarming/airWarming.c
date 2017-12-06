#include "airWarming.h"

float valDS18B20;
uint8_t SW_airWarming;
uint8_t AWM_STATUS = 0x02;

uint8_t USRKawmTX_FLG = 0;
uint8_t USRKawmRX_FLG = 0;

osThreadId tid_airWarmingCM_Thread;
osThreadDef(airWarmingCM_Thread,osPriorityNormal,1,1024);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

//复位DS18B20
void DS18B20_Rst(void)
{
    CLR_DS18B20();    //拉低DQ
    delay_us(750);    //拉低750us
    SET_DS18B20();;   //DQ=1
    delay_us(15);     //15US
}


//等待DS18B20的回应
//返回1:未检测到ds18b20的存在
//返回0:存在
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


//从DS18B20读取一个位
//返回值:1/0
uint8_t DS18B20_Read_Bit(void)           // read one bit
{
    uint8_t data;

    CLR_DS18B20();   //拉低DQ
    delay_us(2);
    SET_DS18B20();;  //DQ=1
    delay_us(12);
    if(DS18B20_DQ_IN)data=1;
    else data=0;
    delay_us(50);

    return data;
}


//从ds18b20读取一个字节
//返回值:读取到的数据
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

//*本程序由 www.stm32cube.com 提供
//写一个字节到DS18B20
//dat:要写入的字节
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


//开始温度转换
void DS18B20_Start(void)// ds1820 start convert
{
    DS18B20_Rst();
    DS18B20_Check();
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
}


//初始化DS18B20的IO口DQ同时检查DS的存在
//返回1:不存在
//返回0:存在
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

//从ds18b20得到温度值
//精度：0.1C
//返回值：温度值 （-550~1250）
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
        temp=0;//温度为负
    } else temp=1;//温度为正
    tem=TH; //获得高八位
    tem<<=8;
    tem+=TL;//获得低八位
    tem=(float)tem*0.625;//转换
    if(temp)return tem/10.00; //返回温度值
    else return -tem/10.00;
}

void airWarming_Init(void){

	DS18B20_Init();
	Warming_Init();
	airWarming_Key_Init();
}

u8 KEY_Scan1_AWM(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(swKey_airWarming==0))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(swKey_airWarming==0)return 1;
	}else if(swKey_airWarming==1)key_up=1; 	    
 	return 0;// 无按键按下
}

void airWarmingCM_Thread(const void *argument){

#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif
	
	u8 key_in;
	u8 key_flg=0;
	
	for(;;){
		
//		USRKawmTX_FLG = 1;		 //主动保持在线，如果不用，注释即可，注释后则被动在线，状态改变时才上发状态信息
		
		key_in = KEY_Scan1_AWM(0);
		
		valDS18B20 	= DS18B20_Get_Temp();
		
		if(valDS18B20 > 40.0){		//温度限制，三十度以下才开放控制权
		
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

