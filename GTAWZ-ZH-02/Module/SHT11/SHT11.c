#include "SHT11.h"

/********************************************************************
 * LOCAL VARIABLES
 */
uint8 tick;

uint8 m_error = 0;

uint16 m_temperature;
uint16 m_humidity;
uint8 m_checksum;
uint8 *p_value;

float SHT11_hum=0.0, SHT11_temp=0.0;

osThreadId tid_SHT11Test_Thread;

osThreadDef(SHT11Test_Thread,osPriorityNormal,1,512);

extern osMutexId (uart1_mutex_id);

extern ARM_DRIVER_USART Driver_USART1;								//设备驱动库串口一设备声明

/*********************************************************************
 * LOCAL FUNCTIONS
 */
void reset(void);
void start(void);
void sendByte(uint8 value);
uint8 recvByte(uint8 ack);


void SHT_WInit(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //DI INPUT
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.


}

void SHT_RInit(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;//
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				 //DI INPUT
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

}


void convert_shtxx(float * hum, float *temp)
{
	const float C1=-4;//-2.0468;           // for 12 Bit RH
	const float C2=+0.0405;//+0.0367;           // for 12 Bit RH
	const float C3=-0.0000028;//-0.0000015955;     // for 12 Bit RH
	const float T1=+0.01;             // for 12 Bit RH
	const float T2=+0.00008;          // for 12 Bit RH

	float rh=*hum;             // rh:      Humidity [Ticks] 12 Bit
	float t=*temp;           // t:       Temperature [Ticks] 14 Bit
	float rh_lin;                     // rh_lin:  Humidity linear
	float rh_true;                    // rh_true: Temperature compensated humidity
	float t_C;                        // t_C   :  Temperature

	t_C     = t*0.01 - 39.66;                //calc. temperature  from 14 bit temp. ticks @ 3.5V
	rh_lin  = C3*rh*rh + C2*rh + C1;     //calc. humidity from ticks to [%RH]
	rh_true = (t_C - 25)*(T1 + T2*rh) + rh_lin;   //calc. temperature compensated humidity [%RH]

	if(rh_true > 100)
		rh_true = 100;       //cut if the value is outside of

	if(rh_true < 0.1)
		rh_true = 0.1;       //the physical possible range

	*temp = t_C;               //return temperature
	*hum  = rh_true;              //return humidity[%RH]
}


/**************************************************************************
 *@fn       readTH
 *
 *@brief    read temperature and humidity sensor data.
 *
 *@param    pData-data buffer.
 *          startIndex- .
 *
 *@return   none
 */



void readTH(uint8 *pData, uint8 *pStartIndex)
{
	/*Reading temperature*/
	reset();
	start();
	sendByte(SHT_MEASURE_TEMP);
	SHT_DATA_IN;

	*pStartIndex = 0;

	while (SHT_RD_DATA());

	pData[(*pStartIndex) + 1] = recvByte(0x1); // send ack to sensor
	pData[(*pStartIndex) + 1] &= 0x3f; /*00111111*/
	pData[(*pStartIndex)] = recvByte(0x1); // send ack to sensor
	*pStartIndex += 2;
	(void)recvByte(0x0);//crc chksum and nack to sensor, end transfer, throw away

	/*Reading humidity*/
	reset();
	start();
	sendByte(SHT_MEASURE_HUMI);
	SHT_DATA_IN;

	while (SHT_RD_DATA());

	pData[(*pStartIndex) + 1] = recvByte(0x1);
	pData[(*pStartIndex) + 1] &= 0x0f; /*00001111*/
	pData[(*pStartIndex)] = recvByte(0x1);
	*pStartIndex += 2;
	(void)recvByte(0x0);//crc chksum and nack to sensor, end transfer, throw away
}

void start()
{
  SHT_DATA_OUT;
  //SHT_SCK_OUT;

  SHT_DATA(1);
  SHT_SCK(0);// = 0;

  udelay();

  SHT_SCK(1);// = 1;
  udelay();

  SHT_DATA(0); // = 0;
  udelay();

  SHT_SCK(0);// = 0;
  udelay();

  SHT_SCK(1);// = 1;
  udelay();

  SHT_DATA(1);// = 1;
  udelay();

  SHT_SCK(0);// = 0;
}

void reset()
//----------------------------------------------------------------------------------
// communication reset: DATA-line=1 and at least 9 SCK cycles followed by transstart
//       _____________________________________________________         ________
// DATA:                                                      |_______|
//          _    _    _    _    _    _    _    _    _        ___     ___
// SCK : __| |__| |__| |__| |__| |__| |__| |__| |__| |______|   |___|   |______
{
  u8 i;

  SHT_DATA_OUT;
  //SHT_SCK_OUT;


  SHT_DATA(1);// = 1;
  SHT_SCK(0);// = 0;                    //Initial state
  udelay();

  for (i=0; i<9; i++)                  //9 SCK cycles
  {
    SHT_SCK(1);// = 1;
    udelay();

    SHT_SCK(0);// = 0;

    udelay();

  }

  start();                   //transmission start

}


void sendByte(uint8 value)
{
  uint8 i;

  SHT_DATA_OUT;
  //SHT_SCK_OUT;

  SHT_SCK(0);// = 0;
   udelay();

  for (i=0x80; i>0; i/=2)
  {


    if (i & value)
    {
      SHT_DATA(1);// = 1;
    }
    else
    {
      SHT_DATA(0);// = 0;
    }

    SHT_SCK(1);// = 1;
    udelay();

    SHT_SCK(0);// = 0;
    	udelay();
  }

  //SHT_DATA(1);// = 1;
  //SHT_SCK(1);// = 1;

  SHT_DATA_IN;
  SHT_SCK(1);// = 1;

  while(SHT_RD_DATA());//wait for ack

  SHT_SCK(0);// = 0;
  SHT_DATA_OUT;
}


uint8 recvByte(uint8 ack)
{
  uint8 i,val = 0;

  //SHT_SCK_OUT;


  SHT_DATA_OUT;
  SHT_DATA(1);// = 1;////////////////////

  SHT_DATA_IN;

  udelay();

  for (i=0x80; i>0; i/=2)
  {
    SHT_SCK(1);// = 1;
    udelay();

    if (SHT_RD_DATA())
    {
      val = (val | i);
    }

    SHT_SCK(0);// = 0;
    udelay();
  }

  SHT_DATA_OUT;

  SHT_DATA(!ack);// = !ack;                     //in case of "ack==1" pull down DATA-Line
  SHT_SCK(1);// = 1;                            //clk #9 for ack
  udelay();

  SHT_SCK(0);// = 0;
  SHT_DATA(1);// = 1;                          //release DATA-line
  return val;
}

void SHT11Test_Thread(const void *argument){

	result_t res;
	u8 len;
	char disp[30];
	
	while(1){
	
		readTH((uint8 *)res.sensor_dat, &len);
		SHT11_temp = res.hum_temp.temp;
		SHT11_hum  = res.hum_temp.hum;

		convert_shtxx(&SHT11_hum, &SHT11_temp);
	
		sprintf(disp,"\n\rCurrent humidity = %.2f percent, temperature = %.2f\n\r", SHT11_hum, SHT11_temp);			
		Driver_USART1.Send(disp,strlen(disp));
		
		osDelay(1000);
	}
}

void SHT11_Init(void){

	;
}

void SHT11Test(void){

	tid_SHT11Test_Thread = osThreadCreate(osThread(SHT11Test_Thread),NULL);
}
