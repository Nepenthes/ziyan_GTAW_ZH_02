#include "LCD_1.44.h"
  
extern ARM_DRIVER_USART Driver_USART1;

extern const uint8_t MOUDLE_TYPE[MOUDLE_NUM];
  
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
#elif(MOUDLE_ID == 14)
extern float valsoilHum;
#elif(MOUDLE_ID == 15)
extern uint8_t curAction[2];
extern uint8_t USRKcurTX_FLG;
extern uint8_t USRKcurRX_FLG;
extern uint16_t cycMotorCnt;  //激励脉冲计数
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
#elif(MOUDLE_ID == 22)
extern uint8_t Elec_Param[10];
extern float 	valDianYa;
extern float 	valDianLiu;
extern float	valGongLv;
extern double 	valDianLiang;
#endif
  
//管理LCD重要参数
//默认为竖屏
_lcd144_dev lcd144dev;

osThreadId tid_LCD144Test_Thread;

osThreadDef(LCD144Test_Thread,osPriorityNormal,1,512);
osThreadDef(LCD144_Thread,osPriorityNormal,1,2048);

//画笔颜色,背景颜色
uint16_t LCD144POINT_COLOR = 0x0000,LCD144BACK_COLOR = 0xFFFF;  
uint16_t DeviceCode;	 

/****************************************************************************
* 名    称：void  SPIv_WriteData(uint8_t Data)
* 功    能：STM32_模拟SPI写一个字节数据底层函数
* 入口参数：Data
* 出口参数：无
* 说    明：STM32_模拟SPI读写一个字节数据底层函数
****************************************************************************/
void  SPIv_WriteData(uint8_t Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_1_44_SDA_SET; //输出数据
      else LCD_1_44_SDA_CLR;
	   
      LCD_1_44_SCL_CLR;       
      LCD_1_44_SCL_SET;
      Data<<=1; 
	}
}

/****************************************************************************
* 名    称：uint8_t SPI_WriteByte(SPI_TypeDef* SPIx,uint8_t Byte)
* 功    能：STM32_硬件SPI读写一个字节数据底层函数
* 入口参数：SPIx,Byte
* 出口参数：返回总线收到的数据
* 说    明：STM32_硬件SPI读写一个字节数据底层函数
****************************************************************************/
uint8_t SPI_WriteByte(SPI_TypeDef* SPIx,uint8_t Byte)
{
	while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);		//等待发送区空	  
	SPIx->DR=Byte;	 	//发送一个byte   
	while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//等待接收完一个byte  
	return SPIx->DR;          	     //返回收到的数据			
} 

/****************************************************************************
* 名    称：void SPI_SetSpeed(SPI_TypeDef* SPIx,uint8_t SpeedSet)
* 功    能：设置SPI的速度
* 入口参数：SPIx,SpeedSet
* 出口参数：无
* 说    明：SpeedSet:1,高速;0,低速;
****************************************************************************/
void SPI_SetSpeed(SPI_TypeDef* SPIx,uint8_t SpeedSet)
{
	SPIx->CR1&=0XFFC7;
	if(SpeedSet==1)//高速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2	
	}
	else//低速
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_32; //Fsck=Fpclk/32
	}
	SPIx->CR1|=1<<6; //SPI设备使能
} 

/****************************************************************************
* 名    称：SPI2_Init(void)
* 功    能：STM32_SPI2硬件配置初始化
* 入口参数：无
* 出口参数：无
* 说    明：STM32_SPI2硬件配置初始化
****************************************************************************/
void SPI2_Init(void)	
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 
	//配置SPI2管脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	//SPI2配置选项
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,ENABLE);
	   
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure);

	//使能SPI2
	SPI_Cmd(SPI2, ENABLE);   
}

//******************************************************************
//函数名：  LCD_1_44_WR_REG
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    向液晶屏总线写入写16位指令
//输入参数：Reg:待写入的指令值
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WR_REG(uint16_t data)
{ 
   LCD_1_44_CS_CLR;
   LCD_1_44_RS_CLR;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI2,data);
#else
   SPIv_WriteData(data);
#endif 
   LCD_1_44_CS_SET;
}

//******************************************************************
//函数名：  LCD_1_44_WR_DATA
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    向液晶屏总线写入写8位数据
//输入参数：Data:待写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WR_DATA(uint8_t data)
{
	
   LCD_1_44_CS_CLR;
   LCD_1_44_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI2,data);
#else
   SPIv_WriteData(data);
#endif 
   LCD_1_44_CS_SET;

}
//******************************************************************
//函数名：  LCD_1_44_DrawPoint_16Bit
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    8位总线下如何写入一个16位数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WR_DATA_16Bit(uint16_t data)
{	
   LCD_1_44_CS_CLR;
   LCD_1_44_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI2,data>>8);
   SPI_WriteByte(SPI2,data);
#else
   SPIv_WriteData(data>>8);
   SPIv_WriteData(data);
#endif 
   LCD_1_44_CS_SET;
}

//******************************************************************
//函数名：  LCD_1_44_WriteReg
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    写寄存器数据
//输入参数：LCD_1_44_Reg:寄存器地址
//			LCD_1_44_RegValue:要写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WriteReg(uint16_t LCD_1_44_Reg, uint16_t LCD_1_44_RegValue)
{	
	LCD_1_44_WR_REG(LCD_1_44_Reg);  
	LCD_1_44_WR_DATA(LCD_1_44_RegValue);	    		 
}	   
	 
//******************************************************************
//函数名：  LCD_1_44_WriteRAM_Prepare
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    开始写GRAM
//			在给液晶屏传送RGB数据前，应该发送写GRAM指令
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_WriteRAM_Prepare(void)
{
	LCD_1_44_WR_REG(lcd144dev.wramcmd);
}	 

//******************************************************************
//函数名：  LCD_1_44_DrawPoint
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    在指定位置写入一个像素点数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_DrawPoint(uint16_t x,uint16_t y)
{
	LCD_1_44_SetCursor(x,y);//设置光标位置 
	LCD_1_44_WR_DATA_16Bit(LCD144POINT_COLOR);
}

//******************************************************************
//函数名：  LCD_1_44_GPIOInit
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    液晶屏IO初始化，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_GPIOInit(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	      
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA |RCC_APB2Periph_GPIOC ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4| GPIO_Pin_5| GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);      
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);  	
}

//******************************************************************
//函数名：  LCD_1_44_Reset
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD复位函数，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_RESET(void)
{
	LCD_1_44_RST_CLR;
	osDelay(100);	
	LCD_1_44_RST_SET;
	osDelay(50);
}
 	 
//******************************************************************
//函数名：  LCD_1_44_Init
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD初始化
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD144_Init(void)
{  
#if USE_HARDWARE_SPI //使用硬件SPI
	SPI2_Init();
#else	
	LCD_1_44_GPIOInit();//使用模拟SPI
#endif  										 

 	LCD_1_44_RESET(); //液晶屏复位

	//************* Start Initial Sequence **********//		
  LCD_1_44_WR_REG(0x11); //Exit Sleep
	osDelay(20);
	LCD_1_44_WR_REG(0x26); //Set Default Gamma
	LCD_1_44_WR_DATA(0x04);
	LCD_1_44_WR_REG(0xB1);//Set Frame Rate
	LCD_1_44_WR_DATA(0x0e);
	LCD_1_44_WR_DATA(0x10);
	LCD_1_44_WR_REG(0xC0); //Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	LCD_1_44_WR_DATA(0x08);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_REG(0xC1); //Set BT[2:0] for AVDD & VCL & VGH & VGL
	LCD_1_44_WR_DATA(0x05);
	LCD_1_44_WR_REG(0xC5); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	LCD_1_44_WR_DATA(0x38);
	LCD_1_44_WR_DATA(0x40);
	
	LCD_1_44_WR_REG(0x3a); //Set Color Format
	LCD_1_44_WR_DATA(0x05);
	LCD_1_44_WR_REG(0x36); //RGB
	LCD_1_44_WR_DATA(0x1C);   //1C//C8
	
	LCD_1_44_WR_REG(0x2A); //Set Column Address
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(0x7F);
	LCD_1_44_WR_REG(0x2B); //Set Page Address
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(32);
	LCD_1_44_WR_DATA(0x00);
	LCD_1_44_WR_DATA(127+32);
	
	LCD_1_44_WR_REG(0xB4); 
	LCD_1_44_WR_DATA(0x00);
	
	LCD_1_44_WR_REG(0xf2); //Enable Gamma bit
	LCD_1_44_WR_DATA(0x01);
	LCD_1_44_WR_REG(0xE0); 
	LCD_1_44_WR_DATA(0x3f);//p1
	LCD_1_44_WR_DATA(0x22);//p2
	LCD_1_44_WR_DATA(0x20);//p3
	LCD_1_44_WR_DATA(0x30);//p4
	LCD_1_44_WR_DATA(0x29);//p5
	LCD_1_44_WR_DATA(0x0c);//p6
	LCD_1_44_WR_DATA(0x4e);//p7
	LCD_1_44_WR_DATA(0xb7);//p8
	LCD_1_44_WR_DATA(0x3c);//p9
	LCD_1_44_WR_DATA(0x19);//p10
	LCD_1_44_WR_DATA(0x22);//p11
	LCD_1_44_WR_DATA(0x1e);//p12
	LCD_1_44_WR_DATA(0x02);//p13
	LCD_1_44_WR_DATA(0x01);//p14
	LCD_1_44_WR_DATA(0x00);//p15
	LCD_1_44_WR_REG(0xE1); 
	LCD_1_44_WR_DATA(0x00);//p1
	LCD_1_44_WR_DATA(0x1b);//p2
	LCD_1_44_WR_DATA(0x1f);//p3
	LCD_1_44_WR_DATA(0x0f);//p4
	LCD_1_44_WR_DATA(0x16);//p5
	LCD_1_44_WR_DATA(0x13);//p6
	LCD_1_44_WR_DATA(0x31);//p7
	LCD_1_44_WR_DATA(0x84);//p8
	LCD_1_44_WR_DATA(0x43);//p9
	LCD_1_44_WR_DATA(0x06);//p10
	LCD_1_44_WR_DATA(0x1d);//p11
	LCD_1_44_WR_DATA(0x21);//p12
	LCD_1_44_WR_DATA(0x3d);//p13
	LCD_1_44_WR_DATA(0x3e);//p14
	LCD_1_44_WR_DATA(0x3f);//p15
	
	LCD_1_44_WR_REG(0x29); // Display On
	LCD_1_44_WR_REG(0x2C);

	LCD_1_44_SetParam();//设置LCD参数	 
	///LCD_1_44_LED_SET;//点亮背光	 
	//LCD_1_44_Clear(WHITE);
}
//******************************************************************
//函数名：  LCD_1_44_Clear
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD全屏填充清屏函数
//输入参数：Color:要清屏的填充色
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_1_44_Clear(uint16_t Color)
{
	uint16_t i,j;      
	LCD_1_44_SetWindows(0,0,lcd144dev.width-1,lcd144dev.height-1);	  
	for(i=0;i<lcd144dev.width;i++)
	{
		for(j=0;j<lcd144dev.height;j++)
		LCD_1_44_WR_DATA_16Bit(Color);	//写入数据 	 
	}
} 

void LCD_1_44_ClearS(uint16_t Color,uint16_t x,uint16_t y,uint16_t xx,uint16_t yy)
{
	uint16_t i,j;      
	LCD_1_44_SetWindows(x,y,xx-1,yy-1);	  
	for(i=x;i<xx;i++)
	{
		for(j=y;j<yy;j++)
		LCD_1_44_WR_DATA_16Bit(Color);	//写入数据 	 
	}
}   	
/*************************************************
函数名：LCD_1_44_SetWindows
功能：设置lcd显示窗口，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void LCD_1_44_SetWindows(uint16_t xStar, uint16_t yStar,uint16_t xEnd,uint16_t yEnd)
{
#if USE_HORIZONTAL==1	//使用横屏

	LCD_1_44_WR_REG(lcd144dev.setxcmd);	
	LCD_1_44_WR_DATA(xStar>>8);
	LCD_1_44_WR_DATA(0x00FF&xStar);		
	LCD_1_44_WR_DATA(xEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&xEnd);

	LCD_1_44_WR_REG(lcd144dev.setycmd);	
	LCD_1_44_WR_DATA(yStar>>8);
	LCD_1_44_WR_DATA(0x00FF&yStar);		
	LCD_1_44_WR_DATA(yEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&yEnd);		
#else
	
	LCD_1_44_WR_REG(lcd144dev.setxcmd);	
	LCD_1_44_WR_DATA(xStar>>8);
	LCD_1_44_WR_DATA(0x00FF&xStar);		
	LCD_1_44_WR_DATA(xEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&xEnd);

	LCD_1_44_WR_REG(lcd144dev.setycmd);	
	LCD_1_44_WR_DATA(yStar>>8);
	LCD_1_44_WR_DATA(0x00FF&yStar+0);		
	LCD_1_44_WR_DATA(yEnd>>8);
	LCD_1_44_WR_DATA(0x00FF&yEnd+0);	
#endif

	LCD_1_44_WriteRAM_Prepare();	//开始写入GRAM				
}   

/*************************************************
函数名：LCD_1_44_SetCursor
功能：设置光标位置
入口参数：xy坐标
返回值：无
*************************************************/
void LCD_1_44_SetCursor(uint16_t Xpos, uint16_t Ypos)
{	  	    			
	LCD_1_44_SetWindows(Xpos,Ypos,Xpos,Ypos);
} 

//设置LCD参数
//方便进行横竖屏模式切换
void LCD_1_44_SetParam(void)
{ 	
	lcd144dev.wramcmd=0x2C;
#if USE_HORIZONTAL==1	//使用横屏	  
	lcd144dev.dir=1;//横屏
	lcd144dev.width=128+3;
	lcd144dev.height=128+2;
	lcd144dev.setxcmd=0x2A;
	lcd144dev.setycmd=0x2B;			
	LCD_1_44_WriteReg(0x36,0xA8);

#else//竖屏
	lcd144dev.dir=0;//竖屏				 	 		
	lcd144dev.width=128+2;
	lcd144dev.height=128+3;
	lcd144dev.setxcmd=0x2A;
	lcd144dev.setycmd=0x2B;	
	LCD_1_44_WriteReg(0x36,0xC8);
	//LCD_1_44_WriteReg(0x36,0x1C);//如其值使用0x1C则LCD_1_44_SetWindows函数中‘+32’偏移量应取0
#endif
}	

void LCD144Test_Thread(const void *argument){

	while(1)
	{
		LCD_1_44_Clear(BLACK); //清屏

		LCD144POINT_COLOR=GRAY; 

		Show_Str(32,5,BLUE,WHITE,"系统监控",16,0);

		Show_Str(5,25,RED,YELLOW,"温度     ℃",24,1);
		LCD_1_44_ShowNum2412(5+48,25,RED,YELLOW,":24",24,1);

		Show_Str(5,50,YELLOW,YELLOW,"湿度     ％",24,1);
		LCD_1_44_ShowNum2412(5+48,50,YELLOW,YELLOW,":32",24,1);

		Show_Str(5,75,WHITE,YELLOW,"电压      Ｖ",24,1);
		LCD_1_44_ShowNum2412(5+48,75,WHITE,YELLOW,":3.2",24,1);
			
		Show_Str(5,100,GREEN,YELLOW,"电流      Ａ",24,1);
		LCD_1_44_ShowNum2412(5+48,100,GREEN,YELLOW,":0.2",24,1);
		
		delay_ms(1500);
	}
}

void LCD144_Thread(const void *argument){
	
	const char MD_ID = MOUDLE_ID;
	char	M_ID[2];
	char  M_ADDR[5] = "0x";
	
	M_ID[0] = MD_ID/10+'0';
	M_ID[1] = MD_ID%10+'0';
	
	M_ADDR[2] = MOUDLE_TYPE[MOUDLE_ID - 1] / 16;
	M_ADDR[3] = MOUDLE_TYPE[MOUDLE_ID - 1] % 16;
	
	if(M_ADDR[2] > 9)M_ADDR[2] += 'A' - 10;else M_ADDR[2] += '0';
	if(M_ADDR[3] > 9)M_ADDR[3] += 'A' - 10;else M_ADDR[3] += '0';
	
	LCD_1_44_Clear(BLACK); //清屏 
	Show_Str(5,3,WHITE,BLACK,"Moudle_ID:",12,1);
	Show_Str(65,3,RED,BLACK,(uint8_t*)&M_ID,12,1);
	Show_Str(82,3,WHITE,BLACK,"ATTR:",12,1);	
	if(MD_ID <= 11 || MD_ID == 22)Show_Str(112,3,BRED,BLACK,"MS",12,1);
	else Show_Str(112,3,BRED,BLACK,"CM",12,1);
	Show_Str(5,13,WHITE,BLACK,"Moudle_Type:",12,1);
	Show_Str(75,13,BLUE,BLACK,(uint8_t*)M_ADDR,12,1);
	
#if(MOUDLE_ID == 1)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"键值鑞",24,1);
#elif(MOUDLE_ID == 2)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"身份卡号鑞",24,1);
#elif(MOUDLE_ID == 3)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"语音识别鑞",24,1);
#elif(MOUDLE_ID == 4)
	Show_Str(5,25,LGRAYBLUE,GREEN,"数字信号鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"模拟信号鑞",24,1);
#elif(MOUDLE_ID == 5)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"光照强度鑞",24,1);
#elif(MOUDLE_ID == 6)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"CO2含量鑞",24,1);
#elif(MOUDLE_ID == 7)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"人体侦测鑞",24,1);
#elif(MOUDLE_ID == 8)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"雨水检测鑞",24,1);
#elif(MOUDLE_ID == 9)
	Show_Str(5,25,LGRAYBLUE,GREEN,"温度鑞",24,1);
	Show_Str(100,50,GREEN,YELLOW,"℃",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"湿度鑞",24,1);
	Show_Str(100,100,GREEN,YELLOW,"％",24,1);
#elif(MOUDLE_ID == 10)
	Show_Str(5,25,LGRAYBLUE,GREEN,"大气压强鑞",24,1);
	Show_Str(100,60,GREEN,YELLOW,"kPa",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"海拔高度鑞",24,1);
#elif(MOUDLE_ID == 11)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"风速检测鑞",24,1);
#elif(MOUDLE_ID == 12)
	Show_Str(5,30,LGRAYBLUE,GREEN,"点阵鑞",24,1);
	Show_Str(10,60,LIGHTBLUE,BLACK,"Disp_type :",24,1);
	Show_Str(10,75,LIGHTBLUE,BLACK,"Disp_color:",24,1);
	Show_Str(10,90,LIGHTBLUE,BLACK,"Disp_slip :",24,1);
	Show_Str(10,105,LIGHTBLUE,BLACK,"Disp_speed:",24,1);
#elif(MOUDLE_ID == 14)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"土壤水分值鑞",24,1);
#elif(MOUDLE_ID == 15)
	Show_Str(5,25,LGRAYBLUE,GREEN,"窗帘状态鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"透光程度鑞",24,1);
#elif(MOUDLE_ID == 16)
	Show_Str(5,25,LGRAYBLUE,GREEN,"喷雾控制鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"杀虫控制鑞",24,1);
#elif(MOUDLE_ID == 17)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"排风量鑞",24,1);
#elif(MOUDLE_ID == 18)
	Show_Str(5,25,LGRAYBLUE,GREEN,"加热控制鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"当前温度鑞",24,1);
#elif(MOUDLE_ID == 19)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"生长灯光度鑞",24,1);
#elif(MOUDLE_ID == 20)
	Show_Str(5,25,LGRAYBLUE,GREEN,"电源控制鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"主电电压鑞",24,1);
#elif(MOUDLE_ID == 21)
	Show_Str(5,25,LGRAYBLUE,GREEN,"门锁控制鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"太阳光源鑞",24,1);
#elif(MOUDLE_ID == 22)
	Show_Str(5,25,LGRAYBLUE,GREEN,"当前功率鑞",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"当前电压鑞",24,1);
#endif

	while(1)
	{
#if(MOUDLE_ID == 1)
		static uint8_t Key_value = 6;
		char keyvalDisp[3];
		
		if(Key_value != valKeyBoard){
		
			Key_value = valKeyBoard;
			LCD_1_44_ClearS(BLACK,25,80,120,105);	
			sprintf(&keyvalDisp[1],"%d",Key_value);
			LCD_1_44_ShowNum2412(60,80,GREEN,YELLOW,(uint8_t*)&keyvalDisp[1],24,1);
		}
#elif(MOUDLE_ID == 2)
		static uint8_t RC522ID_BUF[4] = {6};
		char idDisp[6];
		
		if(memcmp(RC522ID_BUF,RC522IDBUF,4)){
		
			memcpy(RC522ID_BUF,RC522IDBUF,4);
			LCD_1_44_ClearS(BLACK,0,90,130,120);
			sprintf (&idDisp[1], "%02X%02X%02X%02X", RC522IDBUF [ 0 ], RC522IDBUF [ 1 ], RC522IDBUF [ 2 ], RC522IDBUF [ 3 ] );
			Show_Str(30,90,GREEN,YELLOW,(uint8_t*)&idDisp[1],24,1);
		}
#elif(MOUDLE_ID == 3)
		const uint8_t *cmd_tips[10] = {"打开风扇","关闭风扇","开窗","关窗","打开喷雾","关闭喷雾","打开加热","关闭加热","打开生长灯","关闭生长灯"};
		static uint8_t Phonetics_num = 1;
		
		if(Phonetics_num != phoneticsNUM){
		
			Phonetics_num = phoneticsNUM;
			
			if(phoneticsNUM){
			
				LCD_1_44_ClearS(BLACK,0,80,128,128);
				Show_Str(10,90,GREEN,YELLOW,(uint8_t *)cmd_tips[phoneticsNUM - 1],24,1);	
			}else{
			
				LCD_1_44_ClearS(BLACK,0,80,128,128);
				Show_Str(40,90,GREEN,YELLOW,"NO CMD",24,1);	
			}	
		}
#elif(MOUDLE_ID == 4)
		static uint8_t val_Digital = 6;
		static uint8_t val_Analog = 6;
		char valD[3],valA[3];
		
		if(val_Digital != valDigital){
		
			val_Digital = valDigital;
			
			LCD_1_44_ClearS(BLACK,0,50,100,75);
			sprintf(&valD[1],"%d", valDigital);
			LCD_1_44_ShowNum2412(55,50,GREEN,YELLOW,(uint8_t*)&valD[1],24,1);	
		}
		
		if(val_Analog != valAnalog){
		
			val_Analog = valAnalog;
			
			LCD_1_44_ClearS(BLACK,0,100,100,128);
			sprintf(&valA[1],"%d", valAnalog);
			LCD_1_44_ShowNum2412(40,100,GREEN,YELLOW,(uint8_t*)&valA[1],24,1);
			Show_Str(strlen(&valA[1])*16 + 40,105,GREEN,YELLOW,"%",24,1);
		}
#elif(MOUDLE_ID == 5)
		static uint32_t LUX_value = 6;
		char luxDisp[10];
		
		if(LUX_value != LUXValue){
		
			LUX_value = LUXValue;
			LCD_1_44_ClearS(BLACK,25,80,120,105);
			sprintf(&luxDisp[1],"%d",LUXValue);
			LCD_1_44_ShowNum2412(40,80,GREEN,YELLOW,(uint8_t*)&luxDisp[1],24,1);
			Show_Str(strlen(&luxDisp[1])*16 + 40,90,GREEN,YELLOW,"Lux",24,1);
		}
#elif(MOUDLE_ID == 6)
		static float CO2_val;
		char co2Disp[10];
		
		if(valcontentCO2 != CO2_val){
		
			CO2_val = valcontentCO2;
			LCD_1_44_ClearS(BLACK,0,75,120,125);
			sprintf(&co2Disp[1],"%.1f",valcontentCO2);
			LCD_1_44_ShowNum2412(15,80,GREEN,YELLOW,(uint8_t*)&co2Disp[1],24,1);
			Show_Str(strlen(&co2Disp[1])*16+10,90,GREEN,YELLOW,"ppm",24,1);
		}
#elif(MOUDLE_ID == 7)
		static uint8_t is_Someone = 6;
		
		if(is_Someone != isSomeone){
		
			is_Someone = isSomeone;
			LCD_1_44_ClearS(BLACK,0,75,125,125);
			if(isSomeone)Show_Str(40,80,GREEN,YELLOW,"有人",24,1);
			else Show_Str(40,80,GREEN,YELLOW,"没人",24,1);
		}
#elif(MOUDLE_ID == 8)
		static uint8_t is_Rain = 6;
		
		if(is_Rain != isRain){
		
			is_Rain = isRain;
			LCD_1_44_ClearS(BLACK,0,75,120,105);
			if(isRain)Show_Str(40,80,GREEN,YELLOW,"有雨",24,1);
			else Show_Str(40,80,GREEN,YELLOW,"没雨",24,1);
		}
#elif(MOUDLE_ID == 9)			
		static float SHT11hum = 6.0; 
		static float SHT11temp = 6.0;
		char 	 hum[6],temp[6];	

		if(SHT11temp != SHT11_temp){
		
			SHT11temp = SHT11_temp;
			LCD_1_44_ClearS(BLACK,0,50,100,75);
			sprintf(temp,"%.2f", SHT11temp);
			LCD_1_44_ShowNum2412(25,50,GREEN,YELLOW,(uint8_t*)temp,24,1);	
		}			

		if(SHT11hum != SHT11_hum){
			
			SHT11hum = SHT11_hum;
			LCD_1_44_ClearS(BLACK,0,100,100,128);
			sprintf(&hum[1],"%.2f", SHT11hum);
			LCD_1_44_ShowNum2412(25,100,GREEN,YELLOW,(uint8_t*)&hum[1],24,1);
		}		
#elif(MOUDLE_ID == 10)
		static float resultUP = 6.0;
		static float resultUA = 6.0;
		char 	 UP[6],UA[8];
		
		if(resultUP != result_UP){
		
			resultUP = result_UP;
			LCD_1_44_ClearS(BLACK,0,50,100,75);
			sprintf(&UP[1],"%.2f", result_UP / 1000);
			LCD_1_44_ShowNum2412(15,50,GREEN,YELLOW,(uint8_t*)&UP[1],24,1);
		}
		
		if(resultUA != result_UA){
		
			resultUA = result_UA;
			LCD_1_44_ClearS(BLACK,10,100,128,128);
			sprintf(&UA[1],"%.2f", result_UA);
			LCD_1_44_ShowNum2412(15,100,GREEN,YELLOW,(uint8_t*)&UA[1],24,1);
			Show_Str(strlen(&UA[1])*15 + 10,107,GREEN,YELLOW,"m",24,1);
		}
#elif(MOUDLE_ID == 11)
		static float wind_val = 6.0;
		char windDisp[10];
		
		if(wind_val != valwindSpeed){
		
			wind_val = valwindSpeed;
			LCD_1_44_ClearS(BLACK,20,75,120,105);
			sprintf(&windDisp[1],"%.3f",valwindSpeed);
			LCD_1_44_ShowNum2412(20,80,GREEN,YELLOW,(uint8_t*)&windDisp[1],24,1);
			Show_Str(strlen(&windDisp[1])*15 + 20,90,GREEN,YELLOW,"M/s",24,1);
		}
#elif(MOUDLE_ID == 12)
		static uint8_t	Disp_LAattr;
		uint8_t HA;
		uint8_t color;
		uint8_t slip;
		uint8_t speed;
		uint8_t disp[5];
		
		if(Disp_LAattr != DispLAattr){
			
			Disp_LAattr = DispLAattr;

			HA    = (DispLAattr & 0x80) >> 7;
			color = (DispLAattr & 0x60) >> 5;
			slip  = (DispLAattr & 0x10) >> 4;
			speed = (DispLAattr & 0x0f);
			
			LCD_1_44_ClearS(BLACK,100,65,130,130);
			
			if(HA){
				
				Show_Str(100,60,YELLOW,BLACK,"A",24,1);
			}else Show_Str(100,60,YELLOW,BLACK,"B",24,1);
			
			switch(color){
			
				case 0	:	Show_Str(100,75,YELLOW,BLACK,"R",24,1);break;
				case 1	:	Show_Str(100,75,YELLOW,BLACK,"G",24,1);break;
				case 2	:	Show_Str(100,75,YELLOW,BLACK,"RG",24,1);break;
				default	:	Show_Str(100,75,YELLOW,BLACK,"R",24,1);break;
			}
			
			if(slip){
			
				Show_Str(100,90,YELLOW,BLACK,"Yes",24,1);
			}else Show_Str(100,90,YELLOW,BLACK,"No",24,1);
			
			disp[1] = speed / 10 + '0';
			disp[2] = speed % 10 + '0';
			
			Show_Str(100,105,YELLOW,BLACK,&disp[1],24,1);
		}
#elif(MOUDLE_ID == 14)
		static float soil_val;
		char soilDisp[10];
		
		if(soil_val != valsoilHum){
		
			soil_val = valsoilHum;
			LCD_1_44_ClearS(BLACK,0,80,120,105);
			sprintf(&soilDisp[1],"%.2f",valsoilHum);
			LCD_1_44_ShowNum2412(20,80,GREEN,YELLOW,(uint8_t*)&soilDisp[1],24,1);
			Show_Str(strlen(&soilDisp[1])*16 + 20,90,GREEN,YELLOW,"%",24,1);
		}
#elif(MOUDLE_ID == 15)
		static uint8_t cur_Action[2];
		static uint16_t cycMotor_Cnt;
		char curDisp[10];
		
		if(cur_Action[0] != curAction[0]){
		
			cur_Action[0] = curAction[0];
			LCD_1_44_ClearS(BLACK,0,50,128,75);
			switch(curAction[0]){
			
				case 1:	Show_Str(20,50,GREEN,YELLOW,"正在开启",24,1);
							break;
				
				case 2:	Show_Str(20,50,GREEN,YELLOW,"正在关闭",24,1);
							break;
				
				case 3:	if(PEin(1))Show_Str(30,50,GREEN,YELLOW,"已开启",24,1);
							else Show_Str(30,50,GRAY,YELLOW,"已关闭",24,1);
							break;
				
				default: break;
			}
			
			if(cycMotor_Cnt != cycMotorCnt){
			
				cycMotor_Cnt = cycMotorCnt;
				
				LCD_1_44_ClearS(BLACK,0,100,128,128);
				sprintf(&curDisp[1],"%d", cycMotorCnt / 33);
				LCD_1_44_ShowNum2412(40,100,GREEN,YELLOW,(uint8_t*)&curDisp[1],24,1);
				Show_Str(strlen(&curDisp[1])*15 + 40,107,GREEN,YELLOW,"%",24,1);
			}
		}
#elif(MOUDLE_ID == 16)		
		static uint8_t swPST = 6;
		static uint8_t swSPY = 6;
		
		if(swSPY != SW_STATUS[0]){
		
			swSPY = SW_STATUS[0];
			LCD_1_44_ClearS(BLACK,0,50,100,75);
			if(SW_STATUS[0] == 1)Show_Str(40,50,GREEN,YELLOW,"开启",24,1);
			else Show_Str(40,50,GREEN,YELLOW,"关闭",24,1);
		}
		
		if(swPST != SW_STATUS[1]){
		
			swPST = SW_STATUS[1];
			LCD_1_44_ClearS(BLACK,0,100,100,128);
			if(SW_STATUS[1] == 1)Show_Str(40,100,GREEN,YELLOW,"开启",24,1);
			else Show_Str(40,100,GREEN,YELLOW,"关闭",24,1);
		}
#elif(MOUDLE_ID == 17)
		static uint16_t val_PWMexAir = 6;
		
		char valEXAR[5];
		uint16_t dispcolor = GRAY;
		if((PWM_exAir / 60))dispcolor = GREEN;
		else dispcolor = GRAY;
		
		if(val_PWMexAir != PWM_exAir){
		
			val_PWMexAir = PWM_exAir;

			LCD_1_44_ClearS(BLACK,25,75,120,105);
			sprintf(&valEXAR[1],"%d",PWM_exAir / 60);
			LCD_1_44_ShowNum2412(55,80,dispcolor,YELLOW,(uint8_t*)&valEXAR[1],24,1);
			Show_Str(strlen(&valEXAR[1])*16 + 55,90,dispcolor,YELLOW,"%",24,1);	
		}
#elif(MOUDLE_ID == 18)
		static float val_DS18B20 = 6.0;
		static uint8_t statusAWM = 1;
		char ds18b20_Disp[10];
		uint16 dispcolor;
		
		if(AWM_STATUS == 0x01)dispcolor = GREEN;
		else dispcolor = GRAY;
		
		if(statusAWM != AWM_STATUS){
		
			statusAWM = AWM_STATUS;
			LCD_1_44_ClearS(BLACK,0,50,128,75);
			if(AWM_STATUS == 1)Show_Str(40,50,dispcolor,YELLOW,"开启",24,1);
			else Show_Str(40,50,dispcolor,YELLOW,"关闭",24,1);
		}
		
		if(val_DS18B20 != valDS18B20){
		
			val_DS18B20 = valDS18B20;

			LCD_1_44_ClearS(BLACK,0,100,128,128);
			sprintf(&ds18b20_Disp[1],"%.1f",valDS18B20);
			LCD_1_44_ShowNum2412(25,100,GREEN,YELLOW,(uint8_t*)&ds18b20_Disp[1],24,1);
			Show_Str(strlen(&ds18b20_Disp[1])*13 + 25,102,GREEN,YELLOW,"℃",24,1);	
		}
#elif(MOUDLE_ID == 19)			
		static uint16_t val_PWMledGRW = 6;
		char valGRW[5];
		uint16_t dispcolor = GRAY;
		
		if((PWM_ledGRW / 60))dispcolor = GREEN;
		else dispcolor = GRAY;
		
		if(val_PWMledGRW != PWM_ledGRW){
		
			val_PWMledGRW = PWM_ledGRW;

			LCD_1_44_ClearS(BLACK,25,80,120,105);
			sprintf(&valGRW[1],"%d",PWM_ledGRW / 60);
			LCD_1_44_ShowNum2412(55,80,dispcolor,YELLOW,(uint8_t*)&valGRW[1],24,1);
			Show_Str(strlen(&valGRW[1])*16 + 55,90,dispcolor,YELLOW,"%",24,1);	
		}
#elif(MOUDLE_ID == 20)		
		static float val_Voltage;
		static uint8_t sourceType;
		char valVol_Disp[10];
		uint16_t dispcolor = GREEN;
		
		if(SOURCE_TYPE == 1)dispcolor = GREEN;
		else dispcolor = RED;
		
		if(SOURCE_TYPE != sourceType){
		
			sourceType = SOURCE_TYPE;
			LCD_1_44_ClearS(BLACK,0,50,128,75);
			if(SOURCE_TYPE == 1)Show_Str(30,50,dispcolor,YELLOW,"主电源",24,1);
			else Show_Str(10,50,dispcolor,YELLOW,"备用电源",24,1);
		}
		
		if(valVoltage != val_Voltage){
		
			val_Voltage = valVoltage;

			LCD_1_44_ClearS(BLACK,0,100,128,128);
			sprintf(&valVol_Disp[1],"%.2f",valVoltage);
			LCD_1_44_ShowNum2412(25,100,GREEN,YELLOW,(uint8_t*)&valVol_Disp[1],24,1);
			Show_Str(strlen(&valVol_Disp[1])*14 + 25,107,GREEN,YELLOW,"V",24,1);	
		}
#elif(MOUDLE_ID == 21)		
		static uint8_t sw_Relay_lock;
		static uint8_t sw_Relay_light;
		
		if(sw_Relay_lock != swRelay_lock && swRelay_lock){
		
			sw_Relay_lock = swRelay_lock;
			LCD_1_44_ClearS(BLACK,0,50,100,75);
			if(swRelay_lock == 1)Show_Str(40,50,GREEN,YELLOW,"开启",24,1);
			else if(swRelay_lock == 2)Show_Str(40,50,GRAY,YELLOW,"关闭",24,1);
		}
		
		if(sw_Relay_light != swRelay_light && swRelay_lock){
		
			sw_Relay_light = swRelay_light;
			LCD_1_44_ClearS(BLACK,0,100,100,128);
			if(swRelay_light == 1)Show_Str(40,100,GREEN,YELLOW,"开启",24,1);
			else if(swRelay_light == 2)Show_Str(40,100,GRAY,YELLOW,"关闭",24,1);
		}
#elif(MOUDLE_ID == 22)	
		static uint8_t Disp_loop = 0;
		char Meter_disp[20];
		uint8_t dldisp_position = 20;
		
		if((valDianLiang / 1000.0) > 100.0)dldisp_position = 5;
		else dldisp_position = 20;
		
		if(Disp_loop >= 3)Disp_loop = 0;
		
		LCD_1_44_ClearS(BLACK,0,50,128,75);
		memset(Meter_disp,20,20*sizeof(char));
		sprintf(&Meter_disp[1],"%.2f",valGongLv);
		LCD_1_44_ShowNum2412(20,50,GREEN,YELLOW,(uint8_t*)&Meter_disp[1],24,1);
		Show_Str(strlen(&Meter_disp[1])*14+20,59,GREEN,YELLOW,"W",24,1);
	
		LCD_1_44_ClearS(BLACK,0,75,128,128);
		memset(Meter_disp,20,20*sizeof(char));
		sprintf(&Meter_disp[1],"%.2f",valDianYa);
		Show_Str(5,75,LGRAYBLUE,YELLOW,"当前电压鑞",24,1);
		LCD_1_44_ShowNum2412(20,100,GREEN,YELLOW,(uint8_t*)&Meter_disp[1],24,1);
		Show_Str(strlen(&Meter_disp[1])*14+21,107,GREEN,YELLOW,"V",24,1);
		osDelay(2000);Disp_loop ++;
		
		LCD_1_44_ClearS(BLACK,0,75,128,128);
		memset(Meter_disp,20,20*sizeof(char));
		sprintf(&Meter_disp[1],"%.2f",valDianLiu);
		Show_Str(5,75,LGRAYBLUE,YELLOW,"当前电流鑞",24,1);
		LCD_1_44_ShowNum2412(20,100,GREEN,YELLOW,(uint8_t*)&Meter_disp[1],24,1);
		Show_Str(strlen(&Meter_disp[1])*14+20,107,GREEN,YELLOW,"A",24,1);
		osDelay(2000);Disp_loop ++;
		
		LCD_1_44_ClearS(BLACK,0,75,128,128);
		memset(Meter_disp,20,20*sizeof(char));
		sprintf(&Meter_disp[1],"%.2f",valDianLiang / 1000.0);
		Show_Str(5,75,LGRAYBLUE,YELLOW,"总用电量鑞",24,1);
		LCD_1_44_ShowNum2412(dldisp_position,100,GREEN,YELLOW,(uint8_t*)&Meter_disp[1],24,1);
		Show_Str(strlen(&Meter_disp[1])*14+dldisp_position+1,107,GREEN,YELLOW,"kWh",24,1);
		osDelay(2000);Disp_loop ++;	
#endif
		delay_ms(50);
	}
}

void LCD144_test(void){

	tid_LCD144Test_Thread = osThreadCreate(osThread(LCD144_Thread),NULL);
}

