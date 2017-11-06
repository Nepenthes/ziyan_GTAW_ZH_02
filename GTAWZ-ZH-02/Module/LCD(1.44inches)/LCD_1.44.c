#include "LCD_1.44.h"
  
extern ARM_DRIVER_USART Driver_USART1;

extern const uint8_t MOUDLE_TYPE[20];
  
#if(MOUDLE_ID == 1)

#elif(MOUDLE_ID == 5)
extern uint32_t LUXValue;
#elif(MOUDLE_ID == 9)
extern float SHT11_hum; 
extern float SHT11_temp;
#elif(MOUDLE_ID == 12)
extern uint8_t	DispLAattr; // HA:1   color:2  slip :1  speed:4
#endif
  
//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

osThreadId tid_LCD144Test_Thread;

osThreadDef(LCD144Test_Thread,osPriorityNormal,1,512);
osThreadDef(LCD144_Thread,osPriorityNormal,1,1024);

//画笔颜色,背景颜色
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 

/****************************************************************************
* 名    称：void  SPIv_WriteData(u8 Data)
* 功    能：STM32_模拟SPI写一个字节数据底层函数
* 入口参数：Data
* 出口参数：无
* 说    明：STM32_模拟SPI读写一个字节数据底层函数
****************************************************************************/
void  SPIv_WriteData(u8 Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_SDA_SET; //输出数据
      else LCD_SDA_CLR;
	   
      LCD_SCL_CLR;       
      LCD_SCL_SET;
      Data<<=1; 
	}
}

/****************************************************************************
* 名    称：u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
* 功    能：STM32_硬件SPI读写一个字节数据底层函数
* 入口参数：SPIx,Byte
* 出口参数：返回总线收到的数据
* 说    明：STM32_硬件SPI读写一个字节数据底层函数
****************************************************************************/
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
{
	while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);		//等待发送区空	  
	SPIx->DR=Byte;	 	//发送一个byte   
	while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//等待接收完一个byte  
	return SPIx->DR;          	     //返回收到的数据			
} 

/****************************************************************************
* 名    称：void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
* 功    能：设置SPI的速度
* 入口参数：SPIx,SpeedSet
* 出口参数：无
* 说    明：SpeedSet:1,高速;0,低速;
****************************************************************************/
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
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
//函数名：  LCD_WR_REG
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    向液晶屏总线写入写16位指令
//输入参数：Reg:待写入的指令值
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WR_REG(u16 data)
{ 
   LCD_CS_CLR;
   LCD_RS_CLR;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI2,data);
#else
   SPIv_WriteData(data);
#endif 
   LCD_CS_SET;
}

//******************************************************************
//函数名：  LCD_WR_DATA
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    向液晶屏总线写入写8位数据
//输入参数：Data:待写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WR_DATA(u8 data)
{
	
   LCD_CS_CLR;
   LCD_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI2,data);
#else
   SPIv_WriteData(data);
#endif 
   LCD_CS_SET;

}
//******************************************************************
//函数名：  LCD_DrawPoint_16Bit
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    8位总线下如何写入一个16位数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WR_DATA_16Bit(u16 data)
{	
   LCD_CS_CLR;
   LCD_RS_SET;
#if USE_HARDWARE_SPI   
   SPI_WriteByte(SPI2,data>>8);
   SPI_WriteByte(SPI2,data);
#else
   SPIv_WriteData(data>>8);
   SPIv_WriteData(data);
#endif 
   LCD_CS_SET;
}

//******************************************************************
//函数名：  LCD_WriteReg
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    写寄存器数据
//输入参数：LCD_Reg:寄存器地址
//			LCD_RegValue:要写入的数据
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
	 
//******************************************************************
//函数名：  LCD_WriteRAM_Prepare
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    开始写GRAM
//			在给液晶屏传送RGB数据前，应该发送写GRAM指令
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}	 

//******************************************************************
//函数名：  LCD_DrawPoint
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    在指定位置写入一个像素点数据
//输入参数：(x,y):光标坐标
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);//设置光标位置 
	LCD_WR_DATA_16Bit(POINT_COLOR);
}

//******************************************************************
//函数名：  LCD_GPIOInit
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    液晶屏IO初始化，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_GPIOInit(void)
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
//函数名：  LCD_Reset
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD复位函数，液晶初始化前要调用此函数
//输入参数：无
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_RESET(void)
{
	LCD_RST_CLR;
	osDelay(100);	
	LCD_RST_SET;
	osDelay(50);
}
 	 
//******************************************************************
//函数名：  LCD_Init
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
	LCD_GPIOInit();//使用模拟SPI
#endif  										 

 	LCD_RESET(); //液晶屏复位

	//************* Start Initial Sequence **********//		
  LCD_WR_REG(0x11); //Exit Sleep
	osDelay(20);
	LCD_WR_REG(0x26); //Set Default Gamma
	LCD_WR_DATA(0x04);
	LCD_WR_REG(0xB1);//Set Frame Rate
	LCD_WR_DATA(0x0e);
	LCD_WR_DATA(0x10);
	LCD_WR_REG(0xC0); //Set VRH1[4:0] & VC[2:0] for VCI1 & GVDD
	LCD_WR_DATA(0x08);
	LCD_WR_DATA(0x00);
	LCD_WR_REG(0xC1); //Set BT[2:0] for AVDD & VCL & VGH & VGL
	LCD_WR_DATA(0x05);
	LCD_WR_REG(0xC5); //Set VMH[6:0] & VML[6:0] for VOMH & VCOML
	LCD_WR_DATA(0x38);
	LCD_WR_DATA(0x40);
	
	LCD_WR_REG(0x3a); //Set Color Format
	LCD_WR_DATA(0x05);
	LCD_WR_REG(0x36); //RGB
	LCD_WR_DATA(0x1C);   //1C//C8
	
	LCD_WR_REG(0x2A); //Set Column Address
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x7F);
	LCD_WR_REG(0x2B); //Set Page Address
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(32);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(127+32);
	
	LCD_WR_REG(0xB4); 
	LCD_WR_DATA(0x00);
	
	LCD_WR_REG(0xf2); //Enable Gamma bit
	LCD_WR_DATA(0x01);
	LCD_WR_REG(0xE0); 
	LCD_WR_DATA(0x3f);//p1
	LCD_WR_DATA(0x22);//p2
	LCD_WR_DATA(0x20);//p3
	LCD_WR_DATA(0x30);//p4
	LCD_WR_DATA(0x29);//p5
	LCD_WR_DATA(0x0c);//p6
	LCD_WR_DATA(0x4e);//p7
	LCD_WR_DATA(0xb7);//p8
	LCD_WR_DATA(0x3c);//p9
	LCD_WR_DATA(0x19);//p10
	LCD_WR_DATA(0x22);//p11
	LCD_WR_DATA(0x1e);//p12
	LCD_WR_DATA(0x02);//p13
	LCD_WR_DATA(0x01);//p14
	LCD_WR_DATA(0x00);//p15
	LCD_WR_REG(0xE1); 
	LCD_WR_DATA(0x00);//p1
	LCD_WR_DATA(0x1b);//p2
	LCD_WR_DATA(0x1f);//p3
	LCD_WR_DATA(0x0f);//p4
	LCD_WR_DATA(0x16);//p5
	LCD_WR_DATA(0x13);//p6
	LCD_WR_DATA(0x31);//p7
	LCD_WR_DATA(0x84);//p8
	LCD_WR_DATA(0x43);//p9
	LCD_WR_DATA(0x06);//p10
	LCD_WR_DATA(0x1d);//p11
	LCD_WR_DATA(0x21);//p12
	LCD_WR_DATA(0x3d);//p13
	LCD_WR_DATA(0x3e);//p14
	LCD_WR_DATA(0x3f);//p15
	
	LCD_WR_REG(0x29); // Display On
	LCD_WR_REG(0x2C);

	LCD_SetParam();//设置LCD参数	 
	///LCD_LED_SET;//点亮背光	 
	//LCD_Clear(WHITE);
}
//******************************************************************
//函数名：  LCD_Clear
//作者：    xiao冯@全动电子
//日期：    2013-02-22
//功能：    LCD全屏填充清屏函数
//输入参数：Color:要清屏的填充色
//返回值：  无
//修改记录：无
//******************************************************************
void LCD_Clear(u16 Color)
{
	u16 i,j;      
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);	  
	for(i=0;i<lcddev.width;i++)
	{
		for(j=0;j<lcddev.height;j++)
		LCD_WR_DATA_16Bit(Color);	//写入数据 	 
	}
} 

void LCD_ClearS(u16 Color,u16 x,u16 y,u16 xx,u16 yy)
{
	u16 i,j;      
	LCD_SetWindows(x,y,xx-1,yy-1);	  
	for(i=x;i<xx;i++)
	{
		for(j=y;j<yy;j++)
		LCD_WR_DATA_16Bit(Color);	//写入数据 	 
	}
}   	
/*************************************************
函数名：LCD_SetWindows
功能：设置lcd显示窗口，在此区域写点数据自动换行
入口参数：xy起点和终点
返回值：无
*************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{
#if USE_HORIZONTAL==1	//使用横屏

	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd);		
#else
	
	LCD_WR_REG(lcddev.setxcmd);	
	LCD_WR_DATA(xStar>>8);
	LCD_WR_DATA(0x00FF&xStar);		
	LCD_WR_DATA(xEnd>>8);
	LCD_WR_DATA(0x00FF&xEnd);

	LCD_WR_REG(lcddev.setycmd);	
	LCD_WR_DATA(yStar>>8);
	LCD_WR_DATA(0x00FF&yStar+0);		
	LCD_WR_DATA(yEnd>>8);
	LCD_WR_DATA(0x00FF&yEnd+0);	
#endif

	LCD_WriteRAM_Prepare();	//开始写入GRAM				
}   

/*************************************************
函数名：LCD_SetCursor
功能：设置光标位置
入口参数：xy坐标
返回值：无
*************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);
} 

//设置LCD参数
//方便进行横竖屏模式切换
void LCD_SetParam(void)
{ 	
	lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1	//使用横屏	  
	lcddev.dir=1;//横屏
	lcddev.width=128+3;
	lcddev.height=128+2;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;			
	LCD_WriteReg(0x36,0xA8);

#else//竖屏
	lcddev.dir=0;//竖屏				 	 		
	lcddev.width=128+2;
	lcddev.height=128+3;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;	
	LCD_WriteReg(0x36,0xC8);
	//LCD_WriteReg(0x36,0x1C);//如其值使用0x1C则LCD_SetWindows函数中‘+32’偏移量应取0
#endif
}	

void LCD144Test_Thread(const void *argument){

	while(1)
	{
		LCD_Clear(BLACK); //清屏

		POINT_COLOR=GRAY; 

		Show_Str(32,5,BLUE,WHITE,"系统监控",16,0);

		Show_Str(5,25,RED,YELLOW,"温度     ℃",24,1);
		LCD_ShowNum2412(5+48,25,RED,YELLOW,":24",24,1);

		Show_Str(5,50,YELLOW,YELLOW,"湿度     ％",24,1);
		LCD_ShowNum2412(5+48,50,YELLOW,YELLOW,":32",24,1);

		Show_Str(5,75,WHITE,YELLOW,"电压      Ｖ",24,1);
		LCD_ShowNum2412(5+48,75,WHITE,YELLOW,":3.2",24,1);
			
		Show_Str(5,100,GREEN,YELLOW,"电流      Ａ",24,1);
		LCD_ShowNum2412(5+48,100,GREEN,YELLOW,":0.2",24,1);
		
		delay_ms(1500);
	}
}

void LCD144_Thread(const void *argument){
	
	const char MD_ID = MOUDLE_ID;
	char	M_ID[2];
	char  M_ADDR[5] = "0x";
	
	M_ID[0] = MD_ID/10+'0';
	M_ID[1] = MD_ID%10+'0';
	
	M_ADDR[2] = MOUDLE_TYPE[MOUDLE_ID - 1] / 16 +'0';
	M_ADDR[3] = MOUDLE_TYPE[MOUDLE_ID - 1] % 16 +'0';
	
	LCD_Clear(BLACK); //清屏 
	Show_Str(5,3,WHITE,BLACK,"Moudle_ID:",12,1);
	Show_Str(65,3,RED,BLACK,(u8*)&M_ID,12,1);
	Show_Str(82,3,WHITE,BLACK,"ATTR:",12,1);	
	if(MD_ID < 10)Show_Str(112,3,BRED,BLACK,"UP",12,1);
	else Show_Str(112,3,BRED,BLACK,"DN",12,1);
	Show_Str(5,13,WHITE,BLACK,"Moudle_Type:",12,1);
	Show_Str(75,13,BLUE,BLACK,(u8*)M_ADDR,12,1);
	
#if(MOUDLE_ID == 1)
#elif(MOUDLE_ID == 5)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"亮度",24,1);
#elif(MOUDLE_ID == 9)
	Show_Str(5,25,LGRAYBLUE,GREEN,"温度",24,1);
	Show_Str(100,50,GREEN,YELLOW,"℃",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"湿度",24,1);
	Show_Str(100,100,GREEN,YELLOW,"％",24,1);
#elif(MOUDLE_ID == 12)
	Show_Str(5,30,LGRAYBLUE,GREEN,"点阵",24,1);
	Show_Str(10,60,LIGHTBLUE,BLACK,"Disp_type :",24,1);
	Show_Str(10,75,LIGHTBLUE,BLACK,"Disp_color:",24,1);
	Show_Str(10,90,LIGHTBLUE,BLACK,"Disp_slip :",24,1);
	Show_Str(10,105,LIGHTBLUE,BLACK,"Disp_speed:",24,1);
#endif

	while(1)
	{
#if(MOUDLE_ID == 1)
		
#elif(MOUDLE_ID == 5)
		static uint32_t LUX_value;
		char luxDisp[10];
		
		if(LUX_value != LUXValue){
		
			LUX_value = LUXValue;
			LCD_ClearS(BLACK,25,80,120,105);
			sprintf(&luxDisp[1],"%d", LUXValue);
			LCD_ShowNum2412(25,80,GREEN,YELLOW,(u8*)&luxDisp[1],24,1);
			Show_Str(strlen(&luxDisp[1])*16 + 25,90,GREEN,YELLOW,"Lux",24,1);
		}
		
#elif(MOUDLE_ID == 9)			
		static float SHT11hum; 
		static float SHT11temp;
		char 	 hum[6],temp[6];		

		if(SHT11hum != SHT11_hum){
			
			SHT11hum = SHT11_hum;
			LCD_ClearS(BLACK,0,100,100,130);
			sprintf(&hum[1],"%.2f", SHT11hum);
			LCD_ShowNum2412(25,100,GREEN,YELLOW,(u8*)&hum[1],24,1);
		}
		
		if(SHT11temp != SHT11_temp){
		
			SHT11temp = SHT11_temp;
			LCD_ClearS(BLACK,0,50,100,75);
			sprintf(temp,"%.2f", SHT11temp);
			LCD_ShowNum2412(25,50,GREEN,YELLOW,(u8*)temp,24,1);	
		}		
		
#elif(MOUDLE_ID == 10)
		
#elif(MOUDLE_ID == 11)
		
#elif(MOUDLE_ID == 12)
		static uint8_t	Disp_LAattr;
		uint8_t HA;
		uint8_t color;
		uint8_t slip;
		uint8_t speed;
		u8 disp[5];
		
		if(Disp_LAattr != DispLAattr){
			
			Disp_LAattr = DispLAattr;

			HA    = (DispLAattr & 0x80) >> 7;
			color = (DispLAattr & 0x60) >> 5;
			slip  = (DispLAattr & 0x10) >> 4;
			speed = (DispLAattr & 0x0f);
			
			LCD_ClearS(BLACK,100,65,130,130);
			
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
#endif
		delay_ms(500);
	}
}

void LCD_144_test(void){

	tid_LCD144Test_Thread = osThreadCreate(osThread(LCD144_Thread),NULL);
}

