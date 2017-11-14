#include "bmp180_iic.h"
#include "delay.h"

//初始化IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
 
	
	M10_IIC_SCL=1;
	M10_IIC_SDA=1;
}
//产生IIC起始信号
void IIC_Start(void)
{
	M10_SDA_OUT();     //sda线输出
	M10_IIC_SDA=1;	  	  
	M10_IIC_SCL=1;
	delay_us(4);
 	M10_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	M10_IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	M10_SDA_OUT();//sda线输出
	M10_IIC_SCL=0;
	M10_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	M10_IIC_SCL=1; 
	M10_IIC_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	M10_SDA_IN();      //SDA设置为输入  
	M10_IIC_SDA=1;delay_us(1);	   
	M10_IIC_SCL=1;delay_us(1);	 
	while(M10_READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	M10_IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	M10_IIC_SCL=0;
	M10_SDA_OUT();//设置为输出
	M10_IIC_SDA=0;
	delay_us(2);
	M10_IIC_SCL=1;
	delay_us(2);
	M10_IIC_SCL=0;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	M10_IIC_SCL=0;
	M10_SDA_OUT();
	M10_IIC_SDA=1;
	delay_us(2);
	M10_IIC_SCL=1;
	delay_us(2);
	M10_IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	M10_SDA_OUT(); 	    //设置为输出、
    M10_IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0;t<8;t++)
    {              
        M10_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //对TEA5767这三个延时都是必须的
		M10_IIC_SCL=1;
		delay_us(2); 
		M10_IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	M10_SDA_IN();//SDA设置为输入                                   ///////////对于板子 输入、、、、、、、、
    for(i=0;i<8;i++ )
	{
        M10_IIC_SCL=0; 
        delay_us(2);
		M10_IIC_SCL=1;
        receive<<=1;
        if(M10_READ_SDA)receive++;   
		delay_us(1); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}





















