#include "touch.h"
//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//������������֧��ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A/GT9147�ȣ� ����
//������̳:www.openedv.com
//�޸�����:2014/3/11
//********************************************************************************
//�޸�˵��
//V2.0 20140311
//���ӶԵ��ݴ�������֧��(��Ҫ���:ctiic.c��ott2001a.c�����ļ�)
//V2.1 20141028
//���ݴ���������GT9147��֧��
//////////////////////////////////////////////////////////////////////////////////

//GT9147���ò�����
//��һ���ֽ�Ϊ�汾��(0X60),���뱣֤�µİ汾�Ŵ��ڵ���GT9147�ڲ�
//flashԭ�а汾��,�Ż��������.
const u8 GT9147_CFG_TBL[]=
{
    0X60,0XE0,0X01,0X20,0X03,0X05,0X35,0X00,0X02,0X08,
    0X1E,0X08,0X50,0X3C,0X0F,0X05,0X00,0X00,0XFF,0X67,
    0X50,0X00,0X00,0X18,0X1A,0X1E,0X14,0X89,0X28,0X0A,
    0X30,0X2E,0XBB,0X0A,0X03,0X00,0X00,0X02,0X33,0X1D,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X32,0X00,0X00,
    0X2A,0X1C,0X5A,0X94,0XC5,0X02,0X07,0X00,0X00,0X00,
    0XB5,0X1F,0X00,0X90,0X28,0X00,0X77,0X32,0X00,0X62,
    0X3F,0X00,0X52,0X50,0X00,0X52,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X0F,
    0X0F,0X03,0X06,0X10,0X42,0XF8,0X0F,0X14,0X00,0X00,
    0X00,0X00,0X1A,0X18,0X16,0X14,0X12,0X10,0X0E,0X0C,
    0X0A,0X08,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0X00,0X29,0X28,0X24,0X22,0X20,0X1F,0X1E,0X1D,
    0X0E,0X0C,0X0A,0X08,0X06,0X05,0X04,0X02,0X00,0XFF,
    0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,0X00,
    0X00,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,0XFF,
    0XFF,0XFF,0XFF,0XFF,
};

_m_tp_dev tp_dev=
{
    TP_Init,
    TP_Scan,
    TP_Adjust,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
};
//Ĭ��Ϊtouchtype=0������.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

//������ʼ������
void KEY_Init(void) //IO��ʼ��
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	//��ʼ��KEY0-->GPIOA.13,KEY1-->GPIOA.15  ��������
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//ʹ��PORTA,PORTEʱ��

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//PE2~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4

	//��ʼ�� WK_UP-->GPIOA.0	  ��������
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0

}
//����������
//���ذ���ֵ
//mode:0,��֧��������;1,֧��������;
//0��û���κΰ�������
//1��KEY0����
//2��KEY1����
//3��KEY2���� 
//4��KEY3���� WK_UP
//ע��˺�������Ӧ���ȼ�,KEY0>KEY1>KEY2>KEY3!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==1))
	{
		delay_ms(10);//ȥ���� 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(KEY3==1)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0)key_up=1; 	    
 	return 0;// �ް�������
}

//��ʼ��IIC
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;//��ʹ������IO PORTBʱ��
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11); 						 //PE.5 �����

    IIC_SCL=1;
    IIC_SDA=1;

}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
    SDA_OUT();     //sda�����
    IIC_SDA=1;
    IIC_SCL=1;
    delay_us(4);
    IIC_SDA=0;//START:when CLK is high,DATA change form high to low
    delay_us(4);
    IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ��������
}
//����IICֹͣ�ź�
void IIC_Stop(void)
{
    SDA_OUT();//sda�����
    IIC_SCL=0;
    IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
    delay_us(4);
    IIC_SCL=1;
    IIC_SDA=1;//����I2C���߽����ź�
    delay_us(4);
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime=0;
    SDA_IN();      //SDA����Ϊ����
    IIC_SDA=1;
    delay_us(1);
    IIC_SCL=1;
    delay_us(1);
    while(READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            IIC_Stop();
            return 1;
        }
    }
    IIC_SCL=0;//ʱ�����0
    return 0;
}
//����ACKӦ��
void IIC_Ack(void)
{
    IIC_SCL=0;
    SDA_OUT();
    IIC_SDA=0;
    delay_us(2);
    IIC_SCL=1;
    delay_us(2);
    IIC_SCL=0;
}
//������ACKӦ��
void IIC_NAck(void)
{
    IIC_SCL=0;
    SDA_OUT();
    IIC_SDA=1;
    delay_us(2);
    IIC_SCL=1;
    delay_us(2);
    IIC_SCL=0;
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0; t<8; t++)
    {
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        delay_us(2);   //��TEA5767��������ʱ���Ǳ����
        IIC_SCL=1;
        delay_us(2);
        IIC_SCL=0;
        delay_us(2);
    }
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA����Ϊ����
    for(i=0; i<8; i++ )
    {
        IIC_SCL=0;
        delay_us(2);
        IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;
        delay_us(1);
    }
    if (!ack)
        IIC_NAck();//����nACK
    else
        IIC_Ack(); //����ACK
    return receive;
}

//��ʼ��IIC�ӿ�
void AT24CXX_Init(void)
{
    IIC_Init();
}
//��AT24CXXָ����ַ����һ������
//ReadAddr:��ʼ�����ĵ�ַ
//����ֵ  :����������
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
    u8 temp=0;
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	   //����д����
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr>>8);//���͸ߵ�ַ
        IIC_Wait_Ack();
    } else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //����������ַ0XA0,д����

    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr%256);   //���͵͵�ַ
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(0XA1);           //�������ģʽ
    IIC_Wait_Ack();
    temp=IIC_Read_Byte(0);
    IIC_Stop();//����һ��ֹͣ����
    return temp;
}
//��AT24CXXָ����ַд��һ������
//WriteAddr  :д�����ݵ�Ŀ�ĵ�ַ
//DataToWrite:Ҫд�������
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	    //����д����
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr>>8);//���͸ߵ�ַ
    } else
    {
        IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //����������ַ0XA0,д����
    }
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr%256);   //���͵͵�ַ
    IIC_Wait_Ack();
    IIC_Send_Byte(DataToWrite);     //�����ֽ�
    IIC_Wait_Ack();
    IIC_Stop();//����һ��ֹͣ����
    delay_ms(10);
}
//��AT24CXX�����ָ����ַ��ʼд�볤��ΪLen������
//�ú�������д��16bit����32bit������.
//WriteAddr  :��ʼд��ĵ�ַ
//DataToWrite:���������׵�ַ
//Len        :Ҫд�����ݵĳ���2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{
    u8 t;
    for(t=0; t<Len; t++)
    {
        AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
    }
}

//��AT24CXX�����ָ����ַ��ʼ��������ΪLen������
//�ú������ڶ���16bit����32bit������.
//ReadAddr   :��ʼ�����ĵ�ַ
//����ֵ     :����
//Len        :Ҫ�������ݵĳ���2,4
u32 AT24CXX_ReadLenByte(u16 ReadAddr,u8 Len)
{
    u8 t;
    u32 temp=0;
    for(t=0; t<Len; t++)
    {
        temp<<=8;
        temp+=AT24CXX_ReadOneByte(ReadAddr+Len-t-1);
    }
    return temp;
}
//���AT24CXX�Ƿ�����
//��������24XX�����һ����ַ(255)���洢��־��.
//���������24Cϵ��,�����ַҪ�޸�
//����1:���ʧ��
//����0:���ɹ�
u8 AT24CXX_Check(void)
{
    u8 temp;
    temp=AT24CXX_ReadOneByte(255);//����ÿ�ο�����дAT24CXX
    if(temp==0X55)return 0;
    else//�ų���һ�γ�ʼ�������
    {
        AT24CXX_WriteOneByte(255,0X55);
        temp=AT24CXX_ReadOneByte(255);
        if(temp==0X55)return 0;
    }
    return 1;
}

//��AT24CXX�����ָ����ַ��ʼ����ָ������������
//ReadAddr :��ʼ�����ĵ�ַ ��24c02Ϊ0~255
//pBuffer  :���������׵�ַ
//NumToRead:Ҫ�������ݵĸ���
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
    while(NumToRead)
    {
        *pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);
        NumToRead--;
    }
}
//��AT24CXX�����ָ����ַ��ʼд��ָ������������
//WriteAddr :��ʼд��ĵ�ַ ��24c02Ϊ0~255
//pBuffer   :���������׵�ַ
//NumToWrite:Ҫд�����ݵĸ���
void AT24CXX_Write(u16 WriteAddr,u8 *pBuffer,u16 NumToWrite)
{
    while(NumToWrite--)
    {
        AT24CXX_WriteOneByte(WriteAddr,*pBuffer);
        WriteAddr++;
        pBuffer++;
    }
}

void CT_Delay(void)
{
    delay_us(10);
}
//���ݴ���оƬIIC�ӿڳ�ʼ��
void CT_IIC_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	 //ʹ��PB,PF�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // PB1 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_6);	//���1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		 // PF9 �˿�����
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_7);	//���1
}
//����IIC��ʼ�ź�
void CT_IIC_Start(void)
{
    CT_SDA_OUT();     //sda�����
    CT_IIC_SDA=1;
    CT_IIC_SCL=1;
    CT_Delay();
    CT_IIC_SDA=0;//START:when CLK is high,DATA change form high to low
    CT_Delay();
    CT_IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ��������
}
//����IICֹͣ�ź�
void CT_IIC_Stop(void)
{
    CT_SDA_OUT();//sda�����
    CT_IIC_SCL=1;
    CT_Delay();
    CT_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
    CT_Delay();
    CT_IIC_SDA=1;//����I2C���߽����ź�
    CT_Delay();
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 CT_IIC_Wait_Ack(void)
{
    u8 ucErrTime=0;
    CT_SDA_IN();      //SDA����Ϊ����
    CT_IIC_SDA=1;
    delay_us(1);
    CT_IIC_SCL=1;
    delay_us(1);
    while(CT_READ_SDA)
    {
        ucErrTime++;
        if(ucErrTime>250)
        {
            CT_IIC_Stop();
            return 1;
        }
    }
    CT_IIC_SCL=0;//ʱ�����0
    return 0;
}
//����ACKӦ��
void CT_IIC_Ack(void)
{
    CT_IIC_SCL=0;
    CT_SDA_OUT();
    CT_IIC_SDA=0;
    CT_Delay();
    CT_IIC_SCL=1;
    CT_Delay();
    CT_IIC_SCL=0;
}
//������ACKӦ��
void CT_IIC_NAck(void)
{
    CT_IIC_SCL=0;
    CT_SDA_OUT();
    CT_IIC_SDA=1;
    CT_Delay();
    CT_IIC_SCL=1;
    CT_Delay();
    CT_IIC_SCL=0;
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void CT_IIC_Send_Byte(u8 txd)
{
    u8 t;
    CT_SDA_OUT();
    CT_IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0; t<8; t++)
    {
        CT_IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        CT_IIC_SCL=1;
        CT_Delay();
        CT_IIC_SCL=0;
        CT_Delay();
    }
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
u8 CT_IIC_Read_Byte(unsigned char ack)
{
    u8 i,receive=0;
    CT_SDA_IN();//SDA����Ϊ����
    for(i=0; i<8; i++ )
    {
        CT_IIC_SCL=0;
        delay_us(30);
        CT_IIC_SCL=1;
        receive<<=1;
        if(CT_READ_SDA)receive++;
    }
    if (!ack)CT_IIC_NAck();//����nACK
    else CT_IIC_Ack(); //����ACK
    return receive;
}

//��OTT2001Aд��һ������
//reg:��ʼ�Ĵ�����ַ
//buf:���ݻ�������
//len:д���ݳ���
//����ֵ:0,�ɹ�;1,ʧ��.
u8 OTT2001A_WR_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    u8 ret=0;
    CT_IIC_Start();
    CT_IIC_Send_Byte(OTT_CMD_WR);   //����д����
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//���͸�8λ��ַ
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//���͵�8λ��ַ
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        CT_IIC_Send_Byte(buf[i]);  	//������
        ret=CT_IIC_Wait_Ack();
        if(ret)break;
    }
    CT_IIC_Stop();					//����һ��ֹͣ����
    return ret;
}
//��OTT2001A����һ������
//reg:��ʼ�Ĵ�����ַ
//buf:���ݻ�������
//len:�����ݳ���
void OTT2001A_RD_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    CT_IIC_Start();
    CT_IIC_Send_Byte(OTT_CMD_WR);   //����д����
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//���͸�8λ��ַ
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//���͵�8λ��ַ
    CT_IIC_Wait_Ack();
    CT_IIC_Start();
    CT_IIC_Send_Byte(OTT_CMD_RD);   //���Ͷ�����
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //������
    }
    CT_IIC_Stop();//����һ��ֹͣ����
}
//��������/�رղ���
//cmd:1,�򿪴�����;0,�رմ�����
void OTT2001A_SensorControl(u8 cmd)
{
    u8 regval=0X00;
    if(cmd)regval=0X80;
    OTT2001A_WR_Reg(OTT_CTRL_REG,&regval,1);
}
//��ʼ��������
//����ֵ:0,��ʼ���ɹ�;1,��ʼ��ʧ��
u8 OTT2001A_Init(void)
{
    u8 regval=0;

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF,ENABLE);	 //ʹ��PB,PF�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		 // PB2 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_2);	//���1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		 // PF10 �˿�����
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_SetBits(GPIOF,GPIO_Pin_10);//���1

    CT_IIC_Init();      	//��ʼ����������I2C����
    OTT_RST=0;				//��λ
    delay_ms(100);
    OTT_RST=1;				//�ͷŸ�λ
    delay_ms(100);
    OTT2001A_SensorControl(1);	//�򿪴�����
    OTT2001A_RD_Reg(OTT_CTRL_REG,&regval,1);//��ȡ���������мĴ�����ֵ���ж�I2Cͨ���Ƿ�����
    //printf("CTP ID:%x\r\n",regval);
    if(regval==0x80)return 0;
    return 1;
}

const u16 OTT_TPX_TBL[5]= {OTT_TP1_REG,OTT_TP2_REG,OTT_TP3_REG,OTT_TP4_REG,OTT_TP5_REG};
//ɨ�败����(���ò�ѯ��ʽ)
//mode:0,����ɨ��.
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
u8 OTT2001A_Scan(u8 mode)
{
    u8 buf[4];
    u8 i=0;
    u8 res=0;
    static u8 t=0;//���Ʋ�ѯ���,�Ӷ�����CPUռ����
    t++;
    if((t%10)==0||t<10)//����ʱ,ÿ����10��CTP_Scan�����ż��1��,�Ӷ���ʡCPUʹ����
    {
        OTT2001A_RD_Reg(OTT_GSTID_REG,&mode,1);//��ȡ�������״̬
        if(mode&0X1F)
        {
            tp_dev.sta=(mode&0X1F)|TP_PRES_DOWN|TP_CATH_PRES;
            for(i=0; i<5; i++)
            {
                if(tp_dev.sta&(1<<i))	//������Ч?
                {
                    OTT2001A_RD_Reg(OTT_TPX_TBL[i],buf,4);	//��ȡXY����ֵ
                    if(tp_dev.touchtype&0X01)//����
                    {
                        tp_dev.y[i]=(((u16)buf[2]<<8)+buf[3])*OTT_SCAL_Y;
                        tp_dev.x[i]=800-((((u16)buf[0]<<8)+buf[1])*OTT_SCAL_X);
                    } else
                    {
                        tp_dev.x[i]=(((u16)buf[2]<<8)+buf[3])*OTT_SCAL_Y;
                        tp_dev.y[i]=(((u16)buf[0]<<8)+buf[1])*OTT_SCAL_X;
                    }
                    //printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
                }
            }
            res=1;
            if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;	//���������ݶ���0,����Դ˴�����
            t=0;		//����һ��,��������������10��,�Ӷ����������
        }
    }
    if((mode&0X1F)==0)//�޴����㰴��
    {
        if(tp_dev.sta&TP_PRES_DOWN)	//֮ǰ�Ǳ����µ�
        {
            tp_dev.sta&=~(1<<7);	//��ǰ����ɿ�
        } else						//֮ǰ��û�б�����
        {
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
            tp_dev.sta&=0XE0;	//�������Ч���
        }
    }
    if(t>240)t=10;//���´�10��ʼ����
    return res;
}

//����GT9147���ò���
//mode:0,���������浽flash
//     1,�������浽flash
u8 GT9147_Send_Cfg(u8 mode)
{
    u8 buf[2];
    u8 i=0;
    buf[0]=0;
    buf[1]=mode;	//�Ƿ�д�뵽GT9147 FLASH?  ���Ƿ���籣��
    for(i=0; i<sizeof(GT9147_CFG_TBL); i++)buf[0]+=GT9147_CFG_TBL[i]; //����У���
    buf[0]=(~buf[0])+1;
    GT9147_WR_Reg(GT_CFGS_REG,(u8*)GT9147_CFG_TBL,sizeof(GT9147_CFG_TBL));//���ͼĴ�������
    GT9147_WR_Reg(GT_CHECK_REG,buf,2);//д��У���,�����ø��±��
    return 0;
}
//��GT9147д��һ������
//reg:��ʼ�Ĵ�����ַ
//buf:���ݻ�������
//len:д���ݳ���
//����ֵ:0,�ɹ�;1,ʧ��.
u8 GT9147_WR_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    u8 ret=0;
    CT_IIC_Start();
    CT_IIC_Send_Byte(GT_CMD_WR);   	//����д����
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//���͸�8λ��ַ
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//���͵�8λ��ַ
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        CT_IIC_Send_Byte(buf[i]);  	//������
        ret=CT_IIC_Wait_Ack();
        if(ret)break;
    }
    CT_IIC_Stop();					//����һ��ֹͣ����
    return ret;
}
//��GT9147����һ������
//reg:��ʼ�Ĵ�����ַ
//buf:���ݻ�������
//len:�����ݳ���
void GT9147_RD_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    CT_IIC_Start();
    CT_IIC_Send_Byte(GT_CMD_WR);   //����д����
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//���͸�8λ��ַ
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//���͵�8λ��ַ
    CT_IIC_Wait_Ack();
    CT_IIC_Start();
    CT_IIC_Send_Byte(GT_CMD_RD);   //���Ͷ�����
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //������
    }
    CT_IIC_Stop();//����һ��ֹͣ����
}
//��ʼ��GT9147������
//����ֵ:0,��ʼ���ɹ�;1,��ʼ��ʧ��
u8 GT9147_Init(void)
{
    u8 temp[5];
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);//ʹ��PB,PF�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		 // PB2 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //�������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA,GPIO_Pin_0);	//���1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		 // �˿�����
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_4);//���1

    CT_IIC_Init();      	//��ʼ����������I2C����
    GT_RST=0;				//��λ
    delay_ms(10);
    GT_RST=1;				//�ͷŸ�λ
    delay_ms(10);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 	//��������
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		//PF10 ��������
    GPIO_ResetBits(GPIOC,GPIO_Pin_4);//���0,����
    delay_ms(100);
    GT9147_RD_Reg(GT_PID_REG,temp,4);//��ȡ��ƷID
    temp[4]=0;
    //printf("CTP ID:%s\r\n",temp);	//��ӡID
    if(strcmp((char*)temp,"9147")==0)//ID==9147
    {
        temp[0]=0X02;
        GT9147_WR_Reg(GT_CTRL_REG,temp,1);//��λGT9147
        GT9147_RD_Reg(GT_CFGS_REG,temp,1);//��ȡGT_CFGS_REG�Ĵ���
        if(temp[0]<0X60)//Ĭ�ϰ汾�Ƚϵ�,��Ҫ����flash����
        {
            //printf("Default Ver:%d\r\n",temp[0]);
            GT9147_Send_Cfg(1);//���²���������
        }
        delay_ms(10);
        temp[0]=0X00;
        GT9147_WR_Reg(GT_CTRL_REG,temp,1);	//������λ
        return 0;
    }
    return 1;
}
const u16 GT9147_TPX_TBL[5]= {GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
//ɨ�败����(���ò�ѯ��ʽ)
//mode:0,����ɨ��.
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
u8 GT9147_Scan(u8 mode)
{
    u8 buf[4];
    u8 i=0;
    u8 res=0;
    u8 temp;
    static u8 t=0;//���Ʋ�ѯ���,�Ӷ�����CPUռ����
    t++;
    if((t%10)==0||t<10)//����ʱ,ÿ����10��CTP_Scan�����ż��1��,�Ӷ���ʡCPUʹ����
    {
        GT9147_RD_Reg(GT_GSTID_REG,&mode,1);//��ȡ�������״̬
        if((mode&0XF)&&((mode&0XF)<6))
        {
            temp=0XFF<<(mode&0XF);//����ĸ���ת��Ϊ1��λ��,ƥ��tp_dev.sta����
            tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES;
            for(i=0; i<5; i++)
            {
                if(tp_dev.sta&(1<<i))	//������Ч?
                {
                    GT9147_RD_Reg(GT9147_TPX_TBL[i],buf,4);	//��ȡXY����ֵ
                    if(tp_dev.touchtype&0X01)//����
                    {
                        tp_dev.y[i]=480 - (((u16)buf[1]<<8)+buf[0]);
                        tp_dev.x[i]=(((u16)buf[3]<<8)+buf[2]);
                    } else
                    {
                        tp_dev.x[i]=((u16)buf[1]<<8)+buf[0];
                        tp_dev.y[i]=((u16)buf[3]<<8)+buf[2];
                    }
                    //printf("x[%d]:%d,y[%d]:%d\r\n",i,tp_dev.x[i],i,tp_dev.y[i]);
                }
            }
            res=1;
            if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;	//���������ݶ���0,����Դ˴�����
            t=0;		//����һ��,��������������10��,�Ӷ����������
        }
        if(mode&0X80&&((mode&0XF)<6))
        {
            temp=0;
            GT9147_WR_Reg(GT_GSTID_REG,&temp,1);//���־
        }
    }
    if((mode&0X8F)==0X80)//�޴����㰴��
    {
        if(tp_dev.sta&TP_PRES_DOWN)	//֮ǰ�Ǳ����µ�
        {
            tp_dev.sta&=~(1<<7);	//��ǰ����ɿ�
        } else						//֮ǰ��û�б�����
        {
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
            tp_dev.sta&=0XE0;	//�������Ч���
        }
    }
    if(t>240)t=10;//���´�10��ʼ����
    return res;
}

//SPIд����
//������ICд��1byte����
//num:Ҫд�������
void TP_Write_Byte(u8 num)
{
    u8 count=0;
    for(count=0; count<8; count++)
    {
        if(num&0x80)TDIN=1;
        else TDIN=0;
        num<<=1;
        TCLK=0;
        TCLK=1;		//��������Ч
    }
}
//SPI������
//�Ӵ�����IC��ȡadcֵ
//CMD:ָ��
//����ֵ:����������
u16 TP_Read_AD(u8 CMD)
{
    u8 count=0;
    u16 Num=0;
    TCLK=0;		//������ʱ��
    TDIN=0; 	//����������
    TCS=0; 		//ѡ�д�����IC
    TP_Write_Byte(CMD);//����������
    delay_us(6);//ADS7846��ת��ʱ���Ϊ6us
    TCLK=0;
    delay_us(1);
    TCLK=1;		//��1��ʱ�ӣ����BUSY
    TCLK=0;
    for(count=0; count<16; count++) //����16λ����,ֻ�и�12λ��Ч
    {
        Num<<=1;
        TCLK=0;	//�½�����Ч
        TCLK=1;
        if(DOUT)Num++;
    }
    Num>>=4;   	//ֻ�и�12λ��Ч.
    TCS=1;		//�ͷ�Ƭѡ
    return(Num);
}
//��ȡһ������ֵ(x����y)
//������ȡREAD_TIMES������,����Щ������������,
//Ȼ��ȥ����ͺ����LOST_VAL����,ȡƽ��ֵ
//xy:ָ�CMD_RDX/CMD_RDY��
//����ֵ:����������
#define READ_TIMES 5 	//��ȡ����
#define LOST_VAL 1	  	//����ֵ
u16 TP_Read_XOY(u8 xy)
{
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum=0;
    u16 temp;
    for(i=0; i<READ_TIMES; i++)buf[i]=TP_Read_AD(xy);
    for(i=0; i<READ_TIMES-1; i++) //����
    {
        for(j=i+1; j<READ_TIMES; j++)
        {
            if(buf[i]>buf[j])//��������
            {
                temp=buf[i];
                buf[i]=buf[j];
                buf[j]=temp;
            }
        }
    }
    sum=0;
    for(i=LOST_VAL; i<READ_TIMES-LOST_VAL; i++)sum+=buf[i];
    temp=sum/(READ_TIMES-2*LOST_VAL);
    return temp;
}
//��ȡx,y����
//��Сֵ��������100.
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
u8 TP_Read_XY(u16 *x,u16 *y)
{
    u16 xtemp,ytemp;
    xtemp=TP_Read_XOY(CMD_RDX);
    ytemp=TP_Read_XOY(CMD_RDY);
    //if(xtemp<100||ytemp<100)return 0;//����ʧ��
    *x=xtemp;
    *y=ytemp;
    return 1;//�����ɹ�
}
//����2�ζ�ȡ������IC,�������ε�ƫ��ܳ���
//ERR_RANGE,��������,����Ϊ������ȷ,�����������.
//�ú����ܴ�����׼ȷ��
//x,y:��ȡ��������ֵ
//����ֵ:0,ʧ��;1,�ɹ���
#define ERR_RANGE 50 //��Χ 
u8 TP_Read_XY2(u16 *x,u16 *y)
{
    u16 x1,y1;
    u16 x2,y2;
    u8 flag;
    flag=TP_Read_XY(&x1,&y1);
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);
    if(flag==0)return(0);
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//ǰ�����β�����+-50��
            &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    } else return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//��LCD�����йصĺ���
//��һ��������
//����У׼�õ�
//x,y:����
//color:��ɫ
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
    POINT_COLOR=color;
    LCD5510_DrawLine(x-12,y,x+13,y);//����
    LCD5510_DrawLine(x,y-12,x,y+13);//����
    LCD5510_DrawPoint(x+1,y+1);
    LCD5510_DrawPoint(x-1,y+1);
    LCD5510_DrawPoint(x+1,y-1);
    LCD5510_DrawPoint(x-1,y-1);
    Draw_Circle(x,y,6);//������Ȧ
}
//��һ�����(2*2�ĵ�)
//x,y:����
//color:��ɫ
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{
    POINT_COLOR=color;
    LCD5510_DrawPoint(x,y);//���ĵ�
    LCD5510_DrawPoint(x+1,y);
    LCD5510_DrawPoint(x,y+1);
    LCD5510_DrawPoint(x+1,y+1);
}
//////////////////////////////////////////////////////////////////////////////////
//��������ɨ��
//tp:0,��Ļ����;1,��������(У׼�����ⳡ����)
//����ֵ:��ǰ����״̬.
//0,�����޴���;1,�����д���
u8 TP_Scan(u8 tp)
{
    if(PEN==0)//�а�������
    {
        if(tp)TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]);//��ȡ��������
        else if(TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))//��ȡ��Ļ����
        {
            tp_dev.x[0]=tp_dev.xfac*tp_dev.x[0]+tp_dev.xoff;//�����ת��Ϊ��Ļ����
            tp_dev.y[0]=tp_dev.yfac*tp_dev.y[0]+tp_dev.yoff;
        }
        if((tp_dev.sta&TP_PRES_DOWN)==0)//֮ǰû�б�����
        {
            tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//��������
            tp_dev.x[4]=tp_dev.x[0];//��¼��һ�ΰ���ʱ������
            tp_dev.y[4]=tp_dev.y[0];
        }
    } else
    {
        if(tp_dev.sta&TP_PRES_DOWN)//֮ǰ�Ǳ����µ�
        {
            tp_dev.sta&=~(1<<7);//��ǰ����ɿ�
        } else//֮ǰ��û�б�����
        {
            tp_dev.x[4]=0;
            tp_dev.y[4]=0;
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
        }
    }
    return tp_dev.sta&TP_PRES_DOWN;//���ص�ǰ�Ĵ���״̬
}
//////////////////////////////////////////////////////////////////////////
//������EEPROM����ĵ�ַ�����ַ,ռ��13���ֽ�(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 40
//����У׼����
void TP_Save_Adjdata(void)
{
    s32 temp;
    //����У�����!
    temp=tp_dev.xfac*100000000;//����xУ������
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);
    temp=tp_dev.yfac*100000000;//����yУ������
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
    //����xƫ����
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,tp_dev.xoff,2);
    //����yƫ����
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,tp_dev.yoff,2);
    //���津������
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,tp_dev.touchtype);
    temp=0X0A;//���У׼����
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp);
}
//�õ�������EEPROM�����У׼ֵ
//����ֵ��1���ɹ���ȡ����
//        0����ȡʧ�ܣ�Ҫ����У׼
u8 TP_Get_Adjdata(void)
{
    s32 tempfac;
    tempfac=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+13);//��ȡ�����,���Ƿ�У׼����
    if(tempfac==0X0A)//�������Ѿ�У׼����
    {
        tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE,4);
        tp_dev.xfac=(float)tempfac/100000000;//�õ�xУ׼����
        tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+4,4);
        tp_dev.yfac=(float)tempfac/100000000;//�õ�yУ׼����
        //�õ�xƫ����
        tp_dev.xoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+8,2);
        //�õ�yƫ����
        tp_dev.yoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+10,2);
        tp_dev.touchtype=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);//��ȡ�������ͱ��
        if(tp_dev.touchtype)//X,Y��������Ļ�෴
        {
            CMD_RDX=0X90;
            CMD_RDY=0XD0;
        } else				   //X,Y��������Ļ��ͬ
        {
            CMD_RDX=0XD0;
            CMD_RDY=0X90;
        }
        return 1;
    }
    return 0;
}
//��ʾ�ַ���
const u8* TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";

//��ʾУ׼���(��������)
void TP_Adj_Info_Show(u16 x0,u16 y0,u16 x1,u16 y1,u16 x2,u16 y2,u16 x3,u16 y3,u16 fac)
{
    POINT_COLOR=RED;
    LCD5510_ShowString(40,160,lcddev.width,lcddev.height,16,"x1:");
    LCD5510_ShowString(40+80,160,lcddev.width,lcddev.height,16,"y1:");
    LCD5510_ShowString(40,180,lcddev.width,lcddev.height,16,"x2:");
    LCD5510_ShowString(40+80,180,lcddev.width,lcddev.height,16,"y2:");
    LCD5510_ShowString(40,200,lcddev.width,lcddev.height,16,"x3:");
    LCD5510_ShowString(40+80,200,lcddev.width,lcddev.height,16,"y3:");
    LCD5510_ShowString(40,220,lcddev.width,lcddev.height,16,"x4:");
    LCD5510_ShowString(40+80,220,lcddev.width,lcddev.height,16,"y4:");
    LCD5510_ShowString(40,240,lcddev.width,lcddev.height,16,"fac is:");
    LCD5510_ShowNum(40+24,160,x0,4,16);		//��ʾ��ֵ
    LCD5510_ShowNum(40+24+80,160,y0,4,16);	//��ʾ��ֵ
    LCD5510_ShowNum(40+24,180,x1,4,16);		//��ʾ��ֵ
    LCD5510_ShowNum(40+24+80,180,y1,4,16);	//��ʾ��ֵ
    LCD5510_ShowNum(40+24,200,x2,4,16);		//��ʾ��ֵ
    LCD5510_ShowNum(40+24+80,200,y2,4,16);	//��ʾ��ֵ
    LCD5510_ShowNum(40+24,220,x3,4,16);		//��ʾ��ֵ
    LCD5510_ShowNum(40+24+80,220,y3,4,16);	//��ʾ��ֵ
    LCD5510_ShowNum(40+56,lcddev.width,fac,3,16); 	//��ʾ��ֵ,����ֵ������95~105��Χ֮��.

}

//������У׼����
//�õ��ĸ�У׼����
void TP_Adjust(void)
{
    u16 pos_temp[4][2];//���껺��ֵ
    u8  cnt=0;
    u16 d1,d2;
    u32 tem1,tem2;
    float fac;
    u16 outtime=0;
    cnt=0;
    POINT_COLOR=BLUE;
    BACK_COLOR =WHITE;
    LCD5510_Clear(WHITE);//����
    POINT_COLOR=RED;//��ɫ
    LCD5510_Clear(WHITE);//����
    POINT_COLOR=BLACK;
    LCD5510_ShowString(40,40,160,100,16,(u8*)TP_REMIND_MSG_TBL);//��ʾ��ʾ��Ϣ
    TP_Drow_Touch_Point(20,20,RED);//����1
    tp_dev.sta=0;//���������ź�
    tp_dev.xfac=0;//xfac��������Ƿ�У׼��,����У׼֮ǰ�������!�������
    while(1)//�������10����û�а���,���Զ��˳�
    {
        tp_dev.scan(1);//ɨ����������
        if((tp_dev.sta&0xc0)==TP_CATH_PRES)//����������һ��(��ʱ�����ɿ���.)
        {
            outtime=0;
            tp_dev.sta&=~(1<<6);//��ǰ����Ѿ����������.

            pos_temp[cnt][0]=tp_dev.x[0];
            pos_temp[cnt][1]=tp_dev.y[0];
            cnt++;
            switch(cnt)
            {
            case 1:
                TP_Drow_Touch_Point(20,20,WHITE);				//�����1
                TP_Drow_Touch_Point(lcddev.width-20,20,RED);	//����2
                break;
            case 2:
                TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	//�����2
                TP_Drow_Touch_Point(20,lcddev.height-20,RED);	//����3
                break;
            case 3:
                TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);			//�����3
                TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);	//����4
                break;
            case 4:	 //ȫ���ĸ����Ѿ��õ�
                //�Ա����
                tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
                tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
                tem1*=tem1;
                tem2*=tem2;
                d1=sqrt(tem1+tem2);//�õ�1,2�ľ���

                tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
                tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
                tem1*=tem1;
                tem2*=tem2;
                d2=sqrt(tem1+tem2);//�õ�3,4�ľ���
                fac=(float)d1/d2;
                if(fac<0.95||fac>1.05||d1==0||d2==0)//���ϸ�
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
                    TP_Drow_Touch_Point(20,20,RED);								//����1
                    TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����
                    continue;
                }
                tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
                tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
                tem1*=tem1;
                tem2*=tem2;
                d1=sqrt(tem1+tem2);//�õ�1,3�ľ���

                tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
                tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
                tem1*=tem1;
                tem2*=tem2;
                d2=sqrt(tem1+tem2);//�õ�2,4�ľ���
                fac=(float)d1/d2;
                if(fac<0.95||fac>1.05)//���ϸ�
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
                    TP_Drow_Touch_Point(20,20,RED);								//����1
                    TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����
                    continue;
                }//��ȷ��

                //�Խ������
                tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
                tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
                tem1*=tem1;
                tem2*=tem2;
                d1=sqrt(tem1+tem2);//�õ�1,4�ľ���

                tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
                tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
                tem1*=tem1;
                tem2*=tem2;
                d2=sqrt(tem1+tem2);//�õ�2,3�ľ���
                fac=(float)d1/d2;
                if(fac<0.95||fac>1.05)//���ϸ�
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
                    TP_Drow_Touch_Point(20,20,RED);								//����1
                    TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//��ʾ����
                    continue;
                }//��ȷ��
                //������
                tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//�õ�xfac
                tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//�õ�xoff

                tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//�õ�yfac
                tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//�õ�yoff
                if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//������Ԥ����෴��.
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//�����4
                    TP_Drow_Touch_Point(20,20,RED);								//����1
                    LCD5510_ShowString(40,26,lcddev.width,lcddev.height,16,"TP Need readjust!");
                    tp_dev.touchtype=!tp_dev.touchtype;//�޸Ĵ�������.
                    if(tp_dev.touchtype)//X,Y��������Ļ�෴
                    {
                        CMD_RDX=0X90;
                        CMD_RDY=0XD0;
                    } else				   //X,Y��������Ļ��ͬ
                    {
                        CMD_RDX=0XD0;
                        CMD_RDY=0X90;
                    }
                    continue;
                }
                POINT_COLOR=BLUE;
                LCD5510_Clear(WHITE);//����
                LCD5510_ShowString(35,110,lcddev.width,lcddev.height,16,"Touch Screen Adjust OK!");//У�����
                delay_ms(1000);
                TP_Save_Adjdata();
                LCD5510_Clear(WHITE);//����
                return;//У�����
            }
        }
        delay_ms(10);
        outtime++;
        if(outtime>1000)
        {
            TP_Get_Adjdata();
            break;
        }
    }
}
//��������ʼ��
//����ֵ:0,û�н���У׼
//       1,���й�У׼
u8 TP_Init(void)
{
    if(lcddev.id==0X5510)				//���ݴ�����
    {
        if(GT9147_Init()==0)			//��GT9147
        {
            tp_dev.scan=GT9147_Scan;	//ɨ�躯��ָ��GT9147������ɨ��
        }
        else
        {
            OTT2001A_Init();
            tp_dev.scan=OTT2001A_Scan;	//ɨ�躯��ָ��OTT2001A������ɨ��
        }
        tp_dev.touchtype|=0X80;			//������
        tp_dev.touchtype|=lcddev.dir&0X01;//������������
        return 0;
    }
    else
    {
        //ע��,ʱ��ʹ��֮��,��GPIO�Ĳ�������Ч
        //��������֮ǰ,����ʹ��ʱ��.����ʵ���������������
        GPIO_InitTypeDef  GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF, ENABLE);	 //ʹ��PB,PF�˿�ʱ��

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;				 // PB1,PB2�˿�����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_2);//����


        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 // PF9�˿�����	�������
        GPIO_Init(GPIOF, &GPIO_InitStructure);
        GPIO_SetBits(GPIOF,GPIO_Pin_9);//P9 ����

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10;	// PF8.PF10�˿�����
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //	��������
        GPIO_Init(GPIOF, &GPIO_InitStructure);
        TP_Read_XY(&tp_dev.x[0],&tp_dev.y[0]);//��һ�ζ�ȡ��ʼ��
        AT24CXX_Init();//��ʼ��24CXX
        if(TP_Get_Adjdata())return 0;//�Ѿ�У׼
        else			   //δУ׼?
        {
            LCD5510_Clear(WHITE);//����
            TP_Adjust();  //��ĻУ׼
            TP_Save_Adjdata();
        }
        TP_Get_Adjdata();
    }
    return 1;
}




































