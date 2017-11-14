#include "bmp180_iic.h"
#include "delay.h"

//��ʼ��IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC, ENABLE );	
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
 
	
	M10_IIC_SCL=1;
	M10_IIC_SDA=1;
}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	M10_SDA_OUT();     //sda�����
	M10_IIC_SDA=1;	  	  
	M10_IIC_SCL=1;
	delay_us(4);
 	M10_IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	M10_IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	  
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	M10_SDA_OUT();//sda�����
	M10_IIC_SCL=0;
	M10_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	M10_IIC_SCL=1; 
	M10_IIC_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	M10_SDA_IN();      //SDA����Ϊ����  
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
	M10_IIC_SCL=0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC_Ack(void)
{
	M10_IIC_SCL=0;
	M10_SDA_OUT();//����Ϊ���
	M10_IIC_SDA=0;
	delay_us(2);
	M10_IIC_SCL=1;
	delay_us(2);
	M10_IIC_SCL=0;
}
//������ACKӦ��		    
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
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC_Send_Byte(uint8_t txd)
{                        
    uint8_t t;   
	M10_SDA_OUT(); 	    //����Ϊ�����
    M10_IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        M10_IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		M10_IIC_SCL=1;
		delay_us(2); 
		M10_IIC_SCL=0;	
		delay_us(2);
    }	 
} 	    
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	M10_SDA_IN();//SDA����Ϊ����                                   ///////////���ڰ��� ���롢��������������
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
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK   
    return receive;
}





















