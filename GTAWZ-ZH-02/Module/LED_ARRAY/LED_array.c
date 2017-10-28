#include "LED_Array.h"

osThreadId tid_LEDArrayTest_Thread;

osThreadDef(LEDArrayTest_Thread,osPriorityNormal,1,1024);

unsigned char gImage_dat1[4][32] = { 
	
	{0x00,0x80,0x00,0x00,0x01,0x00,0x08,0x80,0x00,0x80,0x20,0x80,0x21,0x08,0x08,0x80,0xFC,0x80,0x13,0x3C,0x11,0x08,0x08,0x80,0x04,0xFC,0x12,0x24,0x09,0x10,0x49,0xFE},
	{0x05,0x04,0x02,0x24,0x09,0x20,0x49,0x40,0x49,0x08,0x02,0x24,0x01,0x00,0x4A,0x20,0x2A,0x40,0xF2,0x24,0xFF,0xFE,0x4C,0x20,0x14,0x40,0x12,0x24,0x04,0x40,0x48,0x00},
	{0x10,0x40,0x12,0x24,0x04,0x40,0x49,0xFC,0x28,0xA0,0x12,0xB4,0x04,0x40,0x49,0x24,0x24,0xA0,0x13,0x28,0x04,0x40,0x49,0x24,0x45,0x10,0x12,0x20,0x08,0x42,0x49,0x24},
	{0x81,0x10,0x10,0x20,0x08,0x42,0x49,0x24,0x02,0x08,0x28,0x20,0x10,0x42,0x09,0xFC,0x04,0x04,0x47,0xFE,0x20,0x3E,0x09,0x04,0x08,0x02,0x00,0x00,0xC0,0x00,0x08,0x00}
};

unsigned char gImage_dat[8][16]=
{
	{0x00,0x80,0x00,0x80,0xFC,0x80,0x04,0xFC,0x05,0x04,0x49,0x08,0x2A,0x40,0x14,0x40},
	{0x10,0x40,0x28,0xA0,0x24,0xA0,0x45,0x10,0x81,0x10,0x02,0x08,0x04,0x04,0x08,0x02},/*"欢,0*/
	{0x00,0x00,0x20,0x80,0x13,0x3C,0x12,0x24,0x02,0x24,0x02,0x24,0xF2,0x24,0x12,0x24},
	{0x12,0x24,0x12,0xB4,0x13,0x28,0x12,0x20,0x10,0x20,0x28,0x20,0x47,0xFE,0x00,0x00},/*"迎,1*/
	{0x01,0x00,0x21,0x08,0x11,0x08,0x09,0x10,0x09,0x20,0x01,0x00,0xFF,0xFE,0x04,0x40},
	{0x04,0x40,0x04,0x40,0x04,0x40,0x08,0x42,0x08,0x42,0x10,0x42,0x20,0x3E,0xC0,0x00},/*" 光 ",2*/
	{0x08,0x80,0x08,0x80,0x08,0x80,0x49,0xFE,0x49,0x40,0x4A,0x20,0x4C,0x20,0x48,0x00},
	{0x49,0xFC,0x49,0x24,0x49,0x24,0x49,0x24,0x49,0x24,0x09,0xFC,0x09,0x04,0x08,0x00}/*"临",3*/
};

void DIO1_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOD, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6;//
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_12|GPIO_Pin_13;				 //DI INPUT
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.
	
	
}

void DIO2_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_12|GPIO_Pin_13;//
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOC, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;				 //DI INPUT
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;				 //DI INPUT
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.
	
	
	//GPIO_Write(GPIOB,0XFF00);
// GPIO_ResetBits(GPIOB,GPIO_Pin_7);						
// GPIO_ResetBits(GPIOB,GPIO_Pin_8);		
	
}

void RELAY_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8;				 //
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5|GPIO_Pin_6;				 //DI INPUT
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

	//GPIO_Write(GPIOB,0XFF00);
 GPIO_ResetBits(GPIOB,GPIO_Pin_7);						
 GPIO_ResetBits(GPIOB,GPIO_Pin_8);		
	
}

//LED IO初始化
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	 //使能PB,PE端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_12;	 //LED0-->PB.5 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.5
 //GPIO_SetBits(GPIOA,GPIO_Pin_8);						 //输出高
 //GPIO_SetBits(GPIOA,GPIO_Pin_12);						 //输出高

 GPIO_ResetBits(GPIOA,GPIO_Pin_8);						
 GPIO_ResetBits(GPIOA,GPIO_Pin_12);	
	
}
 
void NC595_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE, ENABLE);	 //使能PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_14|GPIO_Pin_15;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOD, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOE, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB.
	
//	EN=1;
//	EN1=1;
//	EN2=1;

}

//OE RCLK SRCLK 前端3.3V   -- 5V,    信号反向 
void SH_595(u8 Date_in)  
{  
 u8 j;  
 for (j=0;j<8;j++)  
 {  
	SCLK=1;//上升沿发生移位
	delay_us(1);
	if ((Date_in&0x01)==0x01)
		DS=0;
	else
		DS=1;	 			 
	SCLK=0; 
	delay_us(1);
	Date_in=Date_in>>1;	
 }  
 
} 

void SH_595R(u8 Date_in)  
{  
 u8 j;  
 for (j=0;j<8;j++)  
 {  
	SCLK1=1;//上升沿发生移位
	delay_us(1); 
	if ((Date_in&0x01)==0x01)
		DS1=0;
	else
		DS1=1;	 			 
	SCLK1=0; 
	delay_us(1);
	Date_in=Date_in>>1;	
 }  
 
} 

void SH_595G(u8 Date_in)  
{  
 u8 j;  
 for (j=0;j<8;j++)  
 {  
	SCLK2=1;//上升沿发生移位
	delay_us(1); 	 
	if ((Date_in&0x01)==0x01)
		DS2=0;
	else
		DS2=1;	 			 
	SCLK2=0; 
	delay_us(1); 	
	Date_in=Date_in>>1;	
 }  
 
} 

void OUT_595(void)//移位寄存器内的数据锁存到输出寄存器并显示
{  
  LCLK=1;
	delay_us(1);	
  LCLK=0;//上升沿将数据送到输出锁存器            
	delay_us(1);
  LCLK=1;  
}

void OUT_595R(void)//移位寄存器内的数据锁存到输出寄存器并显示
{  
  LCLK1=1;     
  delay_us(1);
  LCLK1=0;//上升沿将数据送到输出锁存器     
	delay_us(1);
  LCLK1=1;  
}
void OUT_595G(void)//移位寄存器内的数据锁存到输出寄存器并显示
{  
  LCLK2=1;
  delay_us(1);	
  LCLK2=0;//上升沿将数据送到输出锁存器            
  delay_us(1);	
  LCLK2=1;  
}

void	D_DATA(void)
{

  	ROW_SEL(16);
		SH_595R(0x80); 
		SH_595R(0x40); 
		SH_595R(0x20); 
		SH_595R(0x10); 
		SH_595R(0x08); 
		SH_595R(0x04); 
		SH_595R(0x02); 
		SH_595R(0x01); 
		OUT_595R();
		//delay_ms(10);


}	



unsigned char Shift_Data(u8 dat)
{
    u8 i;  
    u8 tmp=0x00;  
  
    for(i=0;i<8;i++)  
    {  
        tmp=((dat>>i)&0x01)|tmp;  
        if(i<7)  
					tmp=tmp<<1;  
    }  	
		return tmp;
		
}	

void SH16_595(u16 Date_in)  
{  
 u8 j;  
 for (j=0;j<16;j++)  
 {  
	SCLK=1;//上升沿发生移位
	delay_us(1);
	if ((Date_in&0x0001)==0x0001)
		DS=0;
	else
		DS=1;	 			 
	SCLK=0; 
	delay_us(1);
	Date_in=Date_in>>1;	
 }  
 
} 

//1-16行 ，  上下8行 反了
void ROW_SEL(u8 dat)
{
	if((dat<=8)&&(dat>=1))
	{
		
		SH_595(~Shift_Data(1<<(dat-1))); 
		SH_595(Shift_Data(0xFF)); 
		OUT_595();
		}	
	else if((dat<=16)&&(dat>=9))
	{
		SH_595(Shift_Data(0xFF)); 
		SH_595(~Shift_Data(1<<(dat-9))); 		
		OUT_595();	
		}	

}	


void ROW_OFF(void)
{
		SH16_595(0XFFFF); 
		OUT_595();
	
}	


//gImage_dat1[8][16]=
void DISPLAY_R(void)
{
	u8 i=0,j=0;
	EN=1;
	EN1=1;
	EN2=0;
  for(i=0;i<2;i++)
	{
		for(j=0;j<8;j++)
		{
		ROW_OFF();
		SH_595R((gImage_dat[i+6][1+j*2]));	 
		SH_595R((gImage_dat[i+6][0+j*2]));	

		SH_595R((gImage_dat[i+4][1+j*2]));	
		SH_595R((gImage_dat[i+4][0+j*2]));	 	
		
		SH_595R((gImage_dat[i+2][1+j*2]));	
		SH_595R((gImage_dat[i+2][0+j*2]));	
		
		SH_595R((gImage_dat[i+0][1+j*2]));	 
		SH_595R((gImage_dat[i+0][0+j*2]));	

		OUT_595R();		
			
		ROW_SEL(j+1+8*i);

		delay_us(100);
		//delay_ms(1);			
		}
	}//
}

void DISPLAY_G(void)
{
	u8 i=0,j=0;
	EN=1;
	EN1=0;
	EN2=1;	
  for(i=0;i<2;i++)
	{
		for(j=0;j<8;j++)
		{
		ROW_OFF();
		SH_595G((gImage_dat[i+6][1+j*2]));	 
		SH_595G((gImage_dat[i+6][0+j*2]));	

		SH_595G((gImage_dat[i+4][1+j*2]));	
		SH_595G((gImage_dat[i+4][0+j*2]));	 	
		
		SH_595G((gImage_dat[i+2][1+j*2]));	
		SH_595G((gImage_dat[i+2][0+j*2]));	
		
		SH_595G((gImage_dat[i+0][1+j*2]));	 
		SH_595G((gImage_dat[i+0][0+j*2]));	

		OUT_595G();		
			
		ROW_SEL(j+1+8*i);

		delay_us(100);
		//delay_ms(1);			
		}
	}//
}


void DISPLAY_RG(void)
{
	u8 i=0,j=0;
	EN=1;
	EN1=1;
	EN2=1;	
  for(i=0;i<2;i++)
	{
		for(j=0;j<8;j++)
		{
		ROW_OFF();

		SH_595R((gImage_dat[i+6][1+j*2]));	 
		SH_595R((gImage_dat[i+6][0+j*2]));	

		SH_595R((gImage_dat[i+4][1+j*2]));	
		SH_595R((gImage_dat[i+4][0+j*2]));	 	
		
		SH_595R((gImage_dat[i+2][1+j*2]));	
		SH_595R((gImage_dat[i+2][0+j*2]));	
		
		SH_595R((gImage_dat[i+0][1+j*2]));	 
		SH_595R((gImage_dat[i+0][0+j*2]));	

			
		SH_595G((gImage_dat[i+6][1+j*2]));	 
		SH_595G((gImage_dat[i+6][0+j*2]));	

		SH_595G((gImage_dat[i+4][1+j*2]));	
		SH_595G((gImage_dat[i+4][0+j*2]));	 	
		
		SH_595G((gImage_dat[i+2][1+j*2]));	
		SH_595G((gImage_dat[i+2][0+j*2]));	
		
		SH_595G((gImage_dat[i+0][1+j*2]));	 
		SH_595G((gImage_dat[i+0][0+j*2]));	

		OUT_595R();	
		OUT_595G();		
			
		ROW_SEL(j+1+8*i);

		delay_us(100);
		//delay_ms(1);			
		}
	}//
}

void LEDArrayTest_Thread(const void *argument){

	NC595_Init();
	BEEP_Init();
	Beep_time(150);
	while(1)DISPLAY_R();	
}

void LEDArrayTest(void){

	tid_LEDArrayTest_Thread = osThreadCreate(osThread(LEDArrayTest_Thread),NULL);
}
