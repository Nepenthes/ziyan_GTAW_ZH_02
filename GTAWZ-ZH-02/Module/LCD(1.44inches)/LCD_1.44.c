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
  
//����LCD��Ҫ����
//Ĭ��Ϊ����
_lcd_dev lcddev;

osThreadId tid_LCD144Test_Thread;

osThreadDef(LCD144Test_Thread,osPriorityNormal,1,512);
osThreadDef(LCD144_Thread,osPriorityNormal,1,1024);

//������ɫ,������ɫ
u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 

/****************************************************************************
* ��    �ƣ�void  SPIv_WriteData(u8 Data)
* ��    �ܣ�STM32_ģ��SPIдһ���ֽ����ݵײ㺯��
* ��ڲ�����Data
* ���ڲ�������
* ˵    ����STM32_ģ��SPI��дһ���ֽ����ݵײ㺯��
****************************************************************************/
void  SPIv_WriteData(u8 Data)
{
	unsigned char i=0;
	for(i=8;i>0;i--)
	{
		if(Data&0x80)	
	  LCD_SDA_SET; //�������
      else LCD_SDA_CLR;
	   
      LCD_SCL_CLR;       
      LCD_SCL_SET;
      Data<<=1; 
	}
}

/****************************************************************************
* ��    �ƣ�u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
* ��    �ܣ�STM32_Ӳ��SPI��дһ���ֽ����ݵײ㺯��
* ��ڲ�����SPIx,Byte
* ���ڲ��������������յ�������
* ˵    ����STM32_Ӳ��SPI��дһ���ֽ����ݵײ㺯��
****************************************************************************/
u8 SPI_WriteByte(SPI_TypeDef* SPIx,u8 Byte)
{
	while((SPIx->SR&SPI_I2S_FLAG_TXE)==RESET);		//�ȴ���������	  
	SPIx->DR=Byte;	 	//����һ��byte   
	while((SPIx->SR&SPI_I2S_FLAG_RXNE)==RESET);//�ȴ�������һ��byte  
	return SPIx->DR;          	     //�����յ�������			
} 

/****************************************************************************
* ��    �ƣ�void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
* ��    �ܣ�����SPI���ٶ�
* ��ڲ�����SPIx,SpeedSet
* ���ڲ�������
* ˵    ����SpeedSet:1,����;0,����;
****************************************************************************/
void SPI_SetSpeed(SPI_TypeDef* SPIx,u8 SpeedSet)
{
	SPIx->CR1&=0XFFC7;
	if(SpeedSet==1)//����
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_2;//Fsck=Fpclk/2	
	}
	else//����
	{
		SPIx->CR1|=SPI_BaudRatePrescaler_32; //Fsck=Fpclk/32
	}
	SPIx->CR1|=1<<6; //SPI�豸ʹ��
} 

/****************************************************************************
* ��    �ƣ�SPI2_Init(void)
* ��    �ܣ�STM32_SPI2Ӳ�����ó�ʼ��
* ��ڲ�������
* ���ڲ�������
* ˵    ����STM32_SPI2Ӳ�����ó�ʼ��
****************************************************************************/
void SPI2_Init(void)	
{
	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	 
	//����SPI2�ܽ�
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
	
	//SPI2����ѡ��
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

	//ʹ��SPI2
	SPI_Cmd(SPI2, ENABLE);   
}

//******************************************************************
//��������  LCD_WR_REG
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    ��Һ��������д��д16λָ��
//���������Reg:��д���ָ��ֵ
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  LCD_WR_DATA
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    ��Һ��������д��д8λ����
//���������Data:��д�������
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  LCD_DrawPoint_16Bit
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    8λ���������д��һ��16λ����
//���������(x,y):�������
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  LCD_WriteReg
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    д�Ĵ�������
//���������LCD_Reg:�Ĵ�����ַ
//			LCD_RegValue:Ҫд�������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_WriteReg(u16 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}	   
	 
//******************************************************************
//��������  LCD_WriteRAM_Prepare
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    ��ʼдGRAM
//			�ڸ�Һ��������RGB����ǰ��Ӧ�÷���дGRAMָ��
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(lcddev.wramcmd);
}	 

//******************************************************************
//��������  LCD_DrawPoint
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    ��ָ��λ��д��һ�����ص�����
//���������(x,y):�������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_DrawPoint(u16 x,u16 y)
{
	LCD_SetCursor(x,y);//���ù��λ�� 
	LCD_WR_DATA_16Bit(POINT_COLOR);
}

//******************************************************************
//��������  LCD_GPIOInit
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    Һ����IO��ʼ����Һ����ʼ��ǰҪ���ô˺���
//�����������
//����ֵ��  ��
//�޸ļ�¼����
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
//��������  LCD_Reset
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    LCD��λ������Һ����ʼ��ǰҪ���ô˺���
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_RESET(void)
{
	LCD_RST_CLR;
	osDelay(100);	
	LCD_RST_SET;
	osDelay(50);
}
 	 
//******************************************************************
//��������  LCD_Init
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    LCD��ʼ��
//�����������
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD144_Init(void)
{  
#if USE_HARDWARE_SPI //ʹ��Ӳ��SPI
	SPI2_Init();
#else	
	LCD_GPIOInit();//ʹ��ģ��SPI
#endif  										 

 	LCD_RESET(); //Һ������λ

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

	LCD_SetParam();//����LCD����	 
	///LCD_LED_SET;//��������	 
	//LCD_Clear(WHITE);
}
//******************************************************************
//��������  LCD_Clear
//���ߣ�    xiao��@ȫ������
//���ڣ�    2013-02-22
//���ܣ�    LCDȫ�������������
//���������Color:Ҫ���������ɫ
//����ֵ��  ��
//�޸ļ�¼����
//******************************************************************
void LCD_Clear(u16 Color)
{
	u16 i,j;      
	LCD_SetWindows(0,0,lcddev.width-1,lcddev.height-1);	  
	for(i=0;i<lcddev.width;i++)
	{
		for(j=0;j<lcddev.height;j++)
		LCD_WR_DATA_16Bit(Color);	//д������ 	 
	}
} 

void LCD_ClearS(u16 Color,u16 x,u16 y,u16 xx,u16 yy)
{
	u16 i,j;      
	LCD_SetWindows(x,y,xx-1,yy-1);	  
	for(i=x;i<xx;i++)
	{
		for(j=y;j<yy;j++)
		LCD_WR_DATA_16Bit(Color);	//д������ 	 
	}
}   	
/*************************************************
��������LCD_SetWindows
���ܣ�����lcd��ʾ���ڣ��ڴ�����д�������Զ�����
��ڲ�����xy�����յ�
����ֵ����
*************************************************/
void LCD_SetWindows(u16 xStar, u16 yStar,u16 xEnd,u16 yEnd)
{
#if USE_HORIZONTAL==1	//ʹ�ú���

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

	LCD_WriteRAM_Prepare();	//��ʼд��GRAM				
}   

/*************************************************
��������LCD_SetCursor
���ܣ����ù��λ��
��ڲ�����xy����
����ֵ����
*************************************************/
void LCD_SetCursor(u16 Xpos, u16 Ypos)
{	  	    			
	LCD_SetWindows(Xpos,Ypos,Xpos,Ypos);
} 

//����LCD����
//������к�����ģʽ�л�
void LCD_SetParam(void)
{ 	
	lcddev.wramcmd=0x2C;
#if USE_HORIZONTAL==1	//ʹ�ú���	  
	lcddev.dir=1;//����
	lcddev.width=128+3;
	lcddev.height=128+2;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;			
	LCD_WriteReg(0x36,0xA8);

#else//����
	lcddev.dir=0;//����				 	 		
	lcddev.width=128+2;
	lcddev.height=128+3;
	lcddev.setxcmd=0x2A;
	lcddev.setycmd=0x2B;	
	LCD_WriteReg(0x36,0xC8);
	//LCD_WriteReg(0x36,0x1C);//����ֵʹ��0x1C��LCD_SetWindows�����С�+32��ƫ����Ӧȡ0
#endif
}	

void LCD144Test_Thread(const void *argument){

	while(1)
	{
		LCD_Clear(BLACK); //����

		POINT_COLOR=GRAY; 

		Show_Str(32,5,BLUE,WHITE,"ϵͳ���",16,0);

		Show_Str(5,25,RED,YELLOW,"�¶�     ��",24,1);
		LCD_ShowNum2412(5+48,25,RED,YELLOW,":24",24,1);

		Show_Str(5,50,YELLOW,YELLOW,"ʪ��     ��",24,1);
		LCD_ShowNum2412(5+48,50,YELLOW,YELLOW,":32",24,1);

		Show_Str(5,75,WHITE,YELLOW,"��ѹ      ��",24,1);
		LCD_ShowNum2412(5+48,75,WHITE,YELLOW,":3.2",24,1);
			
		Show_Str(5,100,GREEN,YELLOW,"����      ��",24,1);
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
	
	LCD_Clear(BLACK); //���� 
	Show_Str(5,3,WHITE,BLACK,"Moudle_ID:",12,1);
	Show_Str(65,3,RED,BLACK,(u8*)&M_ID,12,1);
	Show_Str(82,3,WHITE,BLACK,"ATTR:",12,1);	
	if(MD_ID < 10)Show_Str(112,3,BRED,BLACK,"UP",12,1);
	else Show_Str(112,3,BRED,BLACK,"DN",12,1);
	Show_Str(5,13,WHITE,BLACK,"Moudle_Type:",12,1);
	Show_Str(75,13,BLUE,BLACK,(u8*)M_ADDR,12,1);
	
#if(MOUDLE_ID == 1)
#elif(MOUDLE_ID == 5)
	Show_Str(5,50,LGRAYBLUE,YELLOW,"����",24,1);
#elif(MOUDLE_ID == 9)
	Show_Str(5,25,LGRAYBLUE,GREEN,"�¶�",24,1);
	Show_Str(100,50,GREEN,YELLOW,"��",24,1);
	Show_Str(5,75,LGRAYBLUE,YELLOW,"ʪ��",24,1);
	Show_Str(100,100,GREEN,YELLOW,"��",24,1);
#elif(MOUDLE_ID == 12)
	Show_Str(5,30,LGRAYBLUE,GREEN,"����",24,1);
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

