/********************************************************************
 * INCLUDES
 */
#include "TSL2561.h"

/********************************************************************
 * MACROS
 */
osThreadId tid_TSL2561MS_Thread;
osThreadDef(TSL2561MS_Thread,osPriorityNormal,1,256);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

/********************************************************************
 * LOCAL VARIABLES
 */
uint8 Data0_L,Data0_H,Data1_L,Data1_H;
uint16 Channel0,Channel1;
uint32 LUXValue;

/*********************************************************************
 * LOCAL FUNCTIONS
 */

//IO初始化
void IO_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB,PE端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;				 //LED0-->PB.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5

 GPIO_SetBits(GPIOB,GPIO_Pin_0|GPIO_Pin_1); 	//PB10,PB11
}

void TSL2561_Init(void)
{
   IO_Init();
	SDA_OUT();
	IIC_SCL=1;
	IIC_SDA=1;
	TSL2561_Write(CONTROL,0x03);
	delay_ms(100);
	TSL2561_Write(TIMING,0x02);
}

void tsl2561_start(void)
{
	SDA_OUT();     //sda
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(4);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC_SCL=0;//
}


void stop(void)
{
	SDA_OUT();//sda
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC_SCL=1; 
	IIC_SDA=1;//
	delay_us(4);							   	
}


void respons(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
}


void write_byte(uint8 value)
{
    uint8_t t;   
	SDA_OUT(); 	    
    IIC_SCL=0;//
    for(t=0;t<8;t++)
    {              
        //IIC_SDA=(txd&0x80)>>7;
		if((value&0x80)>>7)
			IIC_SDA=1;
		else
			IIC_SDA=0;
		value<<=1; 	  
		delay_us(2);   //
		IIC_SCL=1;
		delay_us(2); 
		IIC_SCL=0;	
		delay_us(2);
    }	 

}


uint8 read_byte(void)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(2);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(1); 
    }					 

	SDA_OUT();
	IIC_SDA=1;//release DATA-line
	return receive;
}


void TSL2561_Write(uint8 command,uint8 data)
{
	tsl2561_start();
	write_byte(SLAVE_ADDR_WR);
	respons();
	write_byte(command);
	respons();
	write_byte(data);
	respons();
	stop();
}


uint8 TSL2561_Read(uint8 command)
{
	uint8 data;
	tsl2561_start();
	write_byte(SLAVE_ADDR_WR);
	respons();
	write_byte(command);
	respons();
	tsl2561_start();
	write_byte(SLAVE_ADDR_RD);
	respons();
	data=read_byte();
	stop();
	return data;
}


void Read_Light(void)
{
	Data0_L=TSL2561_Read(DATA0LOW);
	Data0_H=TSL2561_Read(DATA0HIGH);

	Channel0=(256*Data0_H + Data0_L);
	Data1_L=TSL2561_Read(DATA1LOW);
	Data1_H=TSL2561_Read(DATA1HIGH);
	Channel1=(256*Data1_H + Data1_L);
	LUXValue=calculateLux(Channel0,Channel1);
}

uint32_t calculateLux(uint16_t ch0, uint16_t ch1)
{
		uint32_t chScale;
		uint32_t channel1;
		uint32_t channel0;
		uint32_t temp;
		uint32_t ratio1 = 0;
		uint32_t ratio	;
		uint32_t lux_temp;
		uint16_t b, m;
		chScale=(1 <<TSL2561_LUX_CHSCALE);           //这是时间寄存器为0x02的
		chScale = chScale << 4;                      //这是增益为1的，增益为16不用写这一条
		// scale the channel values
		channel0 = (ch0 * chScale) >> TSL2561_LUX_CHSCALE;
		channel1 = (ch1 * chScale) >> TSL2561_LUX_CHSCALE;
		// find the ratio of the channel values (Channel1/Channel0)
		if (channel0 != 0)
		ratio1 = (channel1 << (TSL2561_LUX_RATIOSCALE+1)) / channel0;
		ratio = (ratio1 + 1) >> 1;	  									 // round the ratio value
		if ((ratio > 0) && (ratio <= TSL2561_LUX_K1T))
			{
				b=TSL2561_LUX_B1T;
				m=TSL2561_LUX_M1T;
			}
		else if (ratio <= TSL2561_LUX_K2T)
			{
				b=TSL2561_LUX_B2T;
				m=TSL2561_LUX_M2T;
			}
		else if (ratio <= TSL2561_LUX_K3T)
			{
				b=TSL2561_LUX_B3T;
				m=TSL2561_LUX_M3T;
			}
		else if (ratio <= TSL2561_LUX_K4T)
			{
				b=TSL2561_LUX_B4T;
				m=TSL2561_LUX_M4T;
			}
		else if (ratio <= TSL2561_LUX_K5T)
			{
				b=TSL2561_LUX_B5T;
				m=TSL2561_LUX_M5T;
			}
		else if (ratio <= TSL2561_LUX_K6T)
			{
				b=TSL2561_LUX_B6T;
				m=TSL2561_LUX_M6T;
			}
		else if (ratio <= TSL2561_LUX_K7T)
			{
				b=TSL2561_LUX_B7T;
				m=TSL2561_LUX_M7T;
			}
		else if (ratio > TSL2561_LUX_K8T)
			{
				b=TSL2561_LUX_B8T;
				m=TSL2561_LUX_M8T;
			}
		temp = ((channel0 * b) - (channel1 * m));
		if (temp < 1)  temp = 0;							// do not allow negative lux value
		temp += (1 << (TSL2561_LUX_LUXSCALE-1));			// round lsb (2^(LUX_SCALE-1))
		lux_temp = temp >> TSL2561_LUX_LUXSCALE;			// strip off fractional portion
		return lux_temp;		  							// Signal I2C had no errors
}

void TSL2561MS_Thread(const void *argument){
	
#if(MOUDLE_DEBUG == 1)
	char disp[30];
#endif	
	
	for(;;){
		
		Read_Light();
#if(MOUDLE_DEBUG == 1)		
		sprintf(disp,"当前光照强度为：%d Lux \r\n",LUXValue);
		Driver_USART1.Send(disp,strlen(disp));
#endif		
		osDelay(1000);
	}
}

void TSL2561MS(void){

	tid_TSL2561MS_Thread = osThreadCreate(osThread(TSL2561MS_Thread),NULL);
}





