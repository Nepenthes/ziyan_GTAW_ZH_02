#include "touch.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//触摸屏驱动（支持ADS7843/7846/UH7843/7846/XPT2046/TSC2046/OTT2001A/GT9147等） 代码
//技术论坛:www.openedv.com
//修改日期:2014/3/11
//********************************************************************************
//修改说明
//V2.0 20140311
//增加对电容触摸屏的支持(需要添加:ctiic.c和ott2001a.c两个文件)
//V2.1 20141028
//电容触摸屏增加GT9147的支持
//////////////////////////////////////////////////////////////////////////////////

//GT9147配置参数表
//第一个字节为版本号(0X60),必须保证新的版本号大于等于GT9147内部
//flash原有版本号,才会更新配置.
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
//默认为touchtype=0的数据.
u8 CMD_RDX=0XD0;
u8 CMD_RDY=0X90;

//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
	//初始化KEY0-->GPIOA.13,KEY1-->GPIOA.15  上拉输入
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOE,ENABLE);//使能PORTA,PORTE时钟

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;//PE2~4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOE, &GPIO_InitStructure);//初始化GPIOE2,3,4

	//初始化 WK_UP-->GPIOA.0	  下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0

}
//按键处理函数
//返回按键值
//mode:0,不支持连续按;1,支持连续按;
//0，没有任何按键按下
//1，KEY0按下
//2，KEY1按下
//3，KEY2按下 
//4，KEY3按下 WK_UP
//注意此函数有响应优先级,KEY0>KEY1>KEY2>KEY3!!
u8 KEY_Scan(u8 mode)
{	 
	static u8 key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||KEY3==1))
	{
		delay_ms(10);//去抖动 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(KEY3==1)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0)key_up=1; 	    
 	return 0;// 无按键按下
}

//初始化IIC
void IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    //RCC->APB2ENR|=1<<4;//先使能外设IO PORTB时钟
    RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11); 						 //PE.5 输出高

    IIC_SCL=1;
    IIC_SDA=1;

}
//产生IIC起始信号
void IIC_Start(void)
{
    SDA_OUT();     //sda线输出
    IIC_SDA=1;
    IIC_SCL=1;
    delay_us(4);
    IIC_SDA=0;//START:when CLK is high,DATA change form high to low
    delay_us(4);
    IIC_SCL=0;//钳住I2C总线，准备发送或接收数据
}
//产生IIC停止信号
void IIC_Stop(void)
{
    SDA_OUT();//sda线输出
    IIC_SCL=0;
    IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
    delay_us(4);
    IIC_SCL=1;
    IIC_SDA=1;//发送I2C总线结束信号
    delay_us(4);
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC_Wait_Ack(void)
{
    u8 ucErrTime=0;
    SDA_IN();      //SDA设置为输入
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
    IIC_SCL=0;//时钟输出0
    return 0;
}
//产生ACK应答
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
//不产生ACK应答
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
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void IIC_Send_Byte(u8 txd)
{
    u8 t;
    SDA_OUT();
    IIC_SCL=0;//拉低时钟开始数据传输
    for(t=0; t<8; t++)
    {
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        delay_us(2);   //对TEA5767这三个延时都是必须的
        IIC_SCL=1;
        delay_us(2);
        IIC_SCL=0;
        delay_us(2);
    }
}
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 IIC_Read_Byte(unsigned char ack)
{
    unsigned char i,receive=0;
    SDA_IN();//SDA设置为输入
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
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK
    return receive;
}

//初始化IIC接口
void AT24CXX_Init(void)
{
    IIC_Init();
}
//在AT24CXX指定地址读出一个数据
//ReadAddr:开始读数的地址
//返回值  :读到的数据
u8 AT24CXX_ReadOneByte(u16 ReadAddr)
{
    u8 temp=0;
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	   //发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(ReadAddr>>8);//发送高地址
        IIC_Wait_Ack();
    } else IIC_Send_Byte(0XA0+((ReadAddr/256)<<1));   //发送器件地址0XA0,写数据

    IIC_Wait_Ack();
    IIC_Send_Byte(ReadAddr%256);   //发送低地址
    IIC_Wait_Ack();
    IIC_Start();
    IIC_Send_Byte(0XA1);           //进入接收模式
    IIC_Wait_Ack();
    temp=IIC_Read_Byte(0);
    IIC_Stop();//产生一个停止条件
    return temp;
}
//在AT24CXX指定地址写入一个数据
//WriteAddr  :写入数据的目的地址
//DataToWrite:要写入的数据
void AT24CXX_WriteOneByte(u16 WriteAddr,u8 DataToWrite)
{
    IIC_Start();
    if(EE_TYPE>AT24C16)
    {
        IIC_Send_Byte(0XA0);	    //发送写命令
        IIC_Wait_Ack();
        IIC_Send_Byte(WriteAddr>>8);//发送高地址
    } else
    {
        IIC_Send_Byte(0XA0+((WriteAddr/256)<<1));   //发送器件地址0XA0,写数据
    }
    IIC_Wait_Ack();
    IIC_Send_Byte(WriteAddr%256);   //发送低地址
    IIC_Wait_Ack();
    IIC_Send_Byte(DataToWrite);     //发送字节
    IIC_Wait_Ack();
    IIC_Stop();//产生一个停止条件
    delay_ms(10);
}
//在AT24CXX里面的指定地址开始写入长度为Len的数据
//该函数用于写入16bit或者32bit的数据.
//WriteAddr  :开始写入的地址
//DataToWrite:数据数组首地址
//Len        :要写入数据的长度2,4
void AT24CXX_WriteLenByte(u16 WriteAddr,u32 DataToWrite,u8 Len)
{
    u8 t;
    for(t=0; t<Len; t++)
    {
        AT24CXX_WriteOneByte(WriteAddr+t,(DataToWrite>>(8*t))&0xff);
    }
}

//在AT24CXX里面的指定地址开始读出长度为Len的数据
//该函数用于读出16bit或者32bit的数据.
//ReadAddr   :开始读出的地址
//返回值     :数据
//Len        :要读出数据的长度2,4
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
//检查AT24CXX是否正常
//这里用了24XX的最后一个地址(255)来存储标志字.
//如果用其他24C系列,这个地址要修改
//返回1:检测失败
//返回0:检测成功
u8 AT24CXX_Check(void)
{
    u8 temp;
    temp=AT24CXX_ReadOneByte(255);//避免每次开机都写AT24CXX
    if(temp==0X55)return 0;
    else//排除第一次初始化的情况
    {
        AT24CXX_WriteOneByte(255,0X55);
        temp=AT24CXX_ReadOneByte(255);
        if(temp==0X55)return 0;
    }
    return 1;
}

//在AT24CXX里面的指定地址开始读出指定个数的数据
//ReadAddr :开始读出的地址 对24c02为0~255
//pBuffer  :数据数组首地址
//NumToRead:要读出数据的个数
void AT24CXX_Read(u16 ReadAddr,u8 *pBuffer,u16 NumToRead)
{
    while(NumToRead)
    {
        *pBuffer++=AT24CXX_ReadOneByte(ReadAddr++);
        NumToRead--;
    }
}
//在AT24CXX里面的指定地址开始写入指定个数的数据
//WriteAddr :开始写入的地址 对24c02为0~255
//pBuffer   :数据数组首地址
//NumToWrite:要写入数据的个数
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
//电容触摸芯片IIC接口初始化
void CT_IIC_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	 //使能PB,PF端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;		 // PB1 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_6);	//输出1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;		 // PF9 端口配置
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_7);	//输出1
}
//产生IIC起始信号
void CT_IIC_Start(void)
{
    CT_SDA_OUT();     //sda线输出
    CT_IIC_SDA=1;
    CT_IIC_SCL=1;
    CT_Delay();
    CT_IIC_SDA=0;//START:when CLK is high,DATA change form high to low
    CT_Delay();
    CT_IIC_SCL=0;//钳住I2C总线，准备发送或接收数据
}
//产生IIC停止信号
void CT_IIC_Stop(void)
{
    CT_SDA_OUT();//sda线输出
    CT_IIC_SCL=1;
    CT_Delay();
    CT_IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
    CT_Delay();
    CT_IIC_SDA=1;//发送I2C总线结束信号
    CT_Delay();
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 CT_IIC_Wait_Ack(void)
{
    u8 ucErrTime=0;
    CT_SDA_IN();      //SDA设置为输入
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
    CT_IIC_SCL=0;//时钟输出0
    return 0;
}
//产生ACK应答
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
//不产生ACK应答
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
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答
void CT_IIC_Send_Byte(u8 txd)
{
    u8 t;
    CT_SDA_OUT();
    CT_IIC_SCL=0;//拉低时钟开始数据传输
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
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK
u8 CT_IIC_Read_Byte(unsigned char ack)
{
    u8 i,receive=0;
    CT_SDA_IN();//SDA设置为输入
    for(i=0; i<8; i++ )
    {
        CT_IIC_SCL=0;
        delay_us(30);
        CT_IIC_SCL=1;
        receive<<=1;
        if(CT_READ_SDA)receive++;
    }
    if (!ack)CT_IIC_NAck();//发送nACK
    else CT_IIC_Ack(); //发送ACK
    return receive;
}

//向OTT2001A写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
u8 OTT2001A_WR_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    u8 ret=0;
    CT_IIC_Start();
    CT_IIC_Send_Byte(OTT_CMD_WR);   //发送写命令
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        CT_IIC_Send_Byte(buf[i]);  	//发数据
        ret=CT_IIC_Wait_Ack();
        if(ret)break;
    }
    CT_IIC_Stop();					//产生一个停止条件
    return ret;
}
//从OTT2001A读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度
void OTT2001A_RD_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    CT_IIC_Start();
    CT_IIC_Send_Byte(OTT_CMD_WR);   //发送写命令
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
    CT_IIC_Wait_Ack();
    CT_IIC_Start();
    CT_IIC_Send_Byte(OTT_CMD_RD);   //发送读命令
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据
    }
    CT_IIC_Stop();//产生一个停止条件
}
//传感器打开/关闭操作
//cmd:1,打开传感器;0,关闭传感器
void OTT2001A_SensorControl(u8 cmd)
{
    u8 regval=0X00;
    if(cmd)regval=0X80;
    OTT2001A_WR_Reg(OTT_CTRL_REG,&regval,1);
}
//初始化触摸屏
//返回值:0,初始化成功;1,初始化失败
u8 OTT2001A_Init(void)
{
    u8 regval=0;

    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF,ENABLE);	 //使能PB,PF端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;		 // PB2 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_2);	//输出1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;		 // PF10 端口配置
    GPIO_Init(GPIOF, &GPIO_InitStructure);
    GPIO_SetBits(GPIOF,GPIO_Pin_10);//输出1

    CT_IIC_Init();      	//初始化电容屏的I2C总线
    OTT_RST=0;				//复位
    delay_ms(100);
    OTT_RST=1;				//释放复位
    delay_ms(100);
    OTT2001A_SensorControl(1);	//打开传感器
    OTT2001A_RD_Reg(OTT_CTRL_REG,&regval,1);//读取传感器运行寄存器的值来判断I2C通信是否正常
    //printf("CTP ID:%x\r\n",regval);
    if(regval==0x80)return 0;
    return 1;
}

const u16 OTT_TPX_TBL[5]= {OTT_TP1_REG,OTT_TP2_REG,OTT_TP3_REG,OTT_TP4_REG,OTT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 OTT2001A_Scan(u8 mode)
{
    u8 buf[4];
    u8 i=0;
    u8 res=0;
    static u8 t=0;//控制查询间隔,从而降低CPU占用率
    t++;
    if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
    {
        OTT2001A_RD_Reg(OTT_GSTID_REG,&mode,1);//读取触摸点的状态
        if(mode&0X1F)
        {
            tp_dev.sta=(mode&0X1F)|TP_PRES_DOWN|TP_CATH_PRES;
            for(i=0; i<5; i++)
            {
                if(tp_dev.sta&(1<<i))	//触摸有效?
                {
                    OTT2001A_RD_Reg(OTT_TPX_TBL[i],buf,4);	//读取XY坐标值
                    if(tp_dev.touchtype&0X01)//横屏
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
            if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;	//读到的数据都是0,则忽略此次数据
            t=0;		//触发一次,则会最少连续监测10次,从而提高命中率
        }
    }
    if((mode&0X1F)==0)//无触摸点按下
    {
        if(tp_dev.sta&TP_PRES_DOWN)	//之前是被按下的
        {
            tp_dev.sta&=~(1<<7);	//标记按键松开
        } else						//之前就没有被按下
        {
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
            tp_dev.sta&=0XE0;	//清除点有效标记
        }
    }
    if(t>240)t=10;//重新从10开始计数
    return res;
}

//发送GT9147配置参数
//mode:0,参数不保存到flash
//     1,参数保存到flash
u8 GT9147_Send_Cfg(u8 mode)
{
    u8 buf[2];
    u8 i=0;
    buf[0]=0;
    buf[1]=mode;	//是否写入到GT9147 FLASH?  即是否掉电保存
    for(i=0; i<sizeof(GT9147_CFG_TBL); i++)buf[0]+=GT9147_CFG_TBL[i]; //计算校验和
    buf[0]=(~buf[0])+1;
    GT9147_WR_Reg(GT_CFGS_REG,(u8*)GT9147_CFG_TBL,sizeof(GT9147_CFG_TBL));//发送寄存器配置
    GT9147_WR_Reg(GT_CHECK_REG,buf,2);//写入校验和,和配置更新标记
    return 0;
}
//向GT9147写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
u8 GT9147_WR_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    u8 ret=0;
    CT_IIC_Start();
    CT_IIC_Send_Byte(GT_CMD_WR);   	//发送写命令
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        CT_IIC_Send_Byte(buf[i]);  	//发数据
        ret=CT_IIC_Wait_Ack();
        if(ret)break;
    }
    CT_IIC_Stop();					//产生一个停止条件
    return ret;
}
//从GT9147读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度
void GT9147_RD_Reg(u16 reg,u8 *buf,u8 len)
{
    u8 i;
    CT_IIC_Start();
    CT_IIC_Send_Byte(GT_CMD_WR);   //发送写命令
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg>>8);   	//发送高8位地址
    CT_IIC_Wait_Ack();
    CT_IIC_Send_Byte(reg&0XFF);   	//发送低8位地址
    CT_IIC_Wait_Ack();
    CT_IIC_Start();
    CT_IIC_Send_Byte(GT_CMD_RD);   //发送读命令
    CT_IIC_Wait_Ack();
    for(i=0; i<len; i++)
    {
        buf[i]=CT_IIC_Read_Byte(i==(len-1)?0:1); //发数据
    }
    CT_IIC_Stop();//产生一个停止条件
}
//初始化GT9147触摸屏
//返回值:0,初始化成功;1,初始化失败
u8 GT9147_Init(void)
{
    u8 temp[5];
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOC,ENABLE);//使能PB,PF端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;		 // PB2 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA,GPIO_Pin_0);	//输出1

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		 // 端口配置
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_SetBits(GPIOC,GPIO_Pin_4);//输出1

    CT_IIC_Init();      	//初始化电容屏的I2C总线
    GT_RST=0;				//复位
    delay_ms(10);
    GT_RST=1;				//释放复位
    delay_ms(10);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 	//下拉输入
    GPIO_Init(GPIOC, &GPIO_InitStructure);	  		//PF10 下拉输入
    GPIO_ResetBits(GPIOC,GPIO_Pin_4);//输出0,下拉
    delay_ms(100);
    GT9147_RD_Reg(GT_PID_REG,temp,4);//读取产品ID
    temp[4]=0;
    //printf("CTP ID:%s\r\n",temp);	//打印ID
    if(strcmp((char*)temp,"9147")==0)//ID==9147
    {
        temp[0]=0X02;
        GT9147_WR_Reg(GT_CTRL_REG,temp,1);//软复位GT9147
        GT9147_RD_Reg(GT_CFGS_REG,temp,1);//读取GT_CFGS_REG寄存器
        if(temp[0]<0X60)//默认版本比较低,需要更新flash配置
        {
            //printf("Default Ver:%d\r\n",temp[0]);
            GT9147_Send_Cfg(1);//更新并保存配置
        }
        delay_ms(10);
        temp[0]=0X00;
        GT9147_WR_Reg(GT_CTRL_REG,temp,1);	//结束复位
        return 0;
    }
    return 1;
}
const u16 GT9147_TPX_TBL[5]= {GT_TP1_REG,GT_TP2_REG,GT_TP3_REG,GT_TP4_REG,GT_TP5_REG};
//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 GT9147_Scan(u8 mode)
{
    u8 buf[4];
    u8 i=0;
    u8 res=0;
    u8 temp;
    static u8 t=0;//控制查询间隔,从而降低CPU占用率
    t++;
    if((t%10)==0||t<10)//空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
    {
        GT9147_RD_Reg(GT_GSTID_REG,&mode,1);//读取触摸点的状态
        if((mode&0XF)&&((mode&0XF)<6))
        {
            temp=0XFF<<(mode&0XF);//将点的个数转换为1的位数,匹配tp_dev.sta定义
            tp_dev.sta=(~temp)|TP_PRES_DOWN|TP_CATH_PRES;
            for(i=0; i<5; i++)
            {
                if(tp_dev.sta&(1<<i))	//触摸有效?
                {
                    GT9147_RD_Reg(GT9147_TPX_TBL[i],buf,4);	//读取XY坐标值
                    if(tp_dev.touchtype&0X01)//横屏
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
            if(tp_dev.x[0]==0 && tp_dev.y[0]==0)mode=0;	//读到的数据都是0,则忽略此次数据
            t=0;		//触发一次,则会最少连续监测10次,从而提高命中率
        }
        if(mode&0X80&&((mode&0XF)<6))
        {
            temp=0;
            GT9147_WR_Reg(GT_GSTID_REG,&temp,1);//清标志
        }
    }
    if((mode&0X8F)==0X80)//无触摸点按下
    {
        if(tp_dev.sta&TP_PRES_DOWN)	//之前是被按下的
        {
            tp_dev.sta&=~(1<<7);	//标记按键松开
        } else						//之前就没有被按下
        {
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
            tp_dev.sta&=0XE0;	//清除点有效标记
        }
    }
    if(t>240)t=10;//重新从10开始计数
    return res;
}

//SPI写数据
//向触摸屏IC写入1byte数据
//num:要写入的数据
void TP_Write_Byte(u8 num)
{
    u8 count=0;
    for(count=0; count<8; count++)
    {
        if(num&0x80)TDIN=1;
        else TDIN=0;
        num<<=1;
        TCLK=0;
        TCLK=1;		//上升沿有效
    }
}
//SPI读数据
//从触摸屏IC读取adc值
//CMD:指令
//返回值:读到的数据
u16 TP_Read_AD(u8 CMD)
{
    u8 count=0;
    u16 Num=0;
    TCLK=0;		//先拉低时钟
    TDIN=0; 	//拉低数据线
    TCS=0; 		//选中触摸屏IC
    TP_Write_Byte(CMD);//发送命令字
    delay_us(6);//ADS7846的转换时间最长为6us
    TCLK=0;
    delay_us(1);
    TCLK=1;		//给1个时钟，清除BUSY
    TCLK=0;
    for(count=0; count<16; count++) //读出16位数据,只有高12位有效
    {
        Num<<=1;
        TCLK=0;	//下降沿有效
        TCLK=1;
        if(DOUT)Num++;
    }
    Num>>=4;   	//只有高12位有效.
    TCS=1;		//释放片选
    return(Num);
}
//读取一个坐标值(x或者y)
//连续读取READ_TIMES次数据,对这些数据升序排列,
//然后去掉最低和最高LOST_VAL个数,取平均值
//xy:指令（CMD_RDX/CMD_RDY）
//返回值:读到的数据
#define READ_TIMES 5 	//读取次数
#define LOST_VAL 1	  	//丢弃值
u16 TP_Read_XOY(u8 xy)
{
    u16 i, j;
    u16 buf[READ_TIMES];
    u16 sum=0;
    u16 temp;
    for(i=0; i<READ_TIMES; i++)buf[i]=TP_Read_AD(xy);
    for(i=0; i<READ_TIMES-1; i++) //排序
    {
        for(j=i+1; j<READ_TIMES; j++)
        {
            if(buf[i]>buf[j])//升序排列
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
//读取x,y坐标
//最小值不能少于100.
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
u8 TP_Read_XY(u16 *x,u16 *y)
{
    u16 xtemp,ytemp;
    xtemp=TP_Read_XOY(CMD_RDX);
    ytemp=TP_Read_XOY(CMD_RDY);
    //if(xtemp<100||ytemp<100)return 0;//读数失败
    *x=xtemp;
    *y=ytemp;
    return 1;//读数成功
}
//连续2次读取触摸屏IC,且这两次的偏差不能超过
//ERR_RANGE,满足条件,则认为读数正确,否则读数错误.
//该函数能大大提高准确度
//x,y:读取到的坐标值
//返回值:0,失败;1,成功。
#define ERR_RANGE 50 //误差范围 
u8 TP_Read_XY2(u16 *x,u16 *y)
{
    u16 x1,y1;
    u16 x2,y2;
    u8 flag;
    flag=TP_Read_XY(&x1,&y1);
    if(flag==0)return(0);
    flag=TP_Read_XY(&x2,&y2);
    if(flag==0)return(0);
    if(((x2<=x1&&x1<x2+ERR_RANGE)||(x1<=x2&&x2<x1+ERR_RANGE))//前后两次采样在+-50内
            &&((y2<=y1&&y1<y2+ERR_RANGE)||(y1<=y2&&y2<y1+ERR_RANGE)))
    {
        *x=(x1+x2)/2;
        *y=(y1+y2)/2;
        return 1;
    } else return 0;
}
//////////////////////////////////////////////////////////////////////////////////
//与LCD部分有关的函数
//画一个触摸点
//用来校准用的
//x,y:坐标
//color:颜色
void TP_Drow_Touch_Point(u16 x,u16 y,u16 color)
{
    POINT_COLOR=color;
    LCD5510_DrawLine(x-12,y,x+13,y);//横线
    LCD5510_DrawLine(x,y-12,x,y+13);//竖线
    LCD5510_DrawPoint(x+1,y+1);
    LCD5510_DrawPoint(x-1,y+1);
    LCD5510_DrawPoint(x+1,y-1);
    LCD5510_DrawPoint(x-1,y-1);
    Draw_Circle(x,y,6);//画中心圈
}
//画一个大点(2*2的点)
//x,y:坐标
//color:颜色
void TP_Draw_Big_Point(u16 x,u16 y,u16 color)
{
    POINT_COLOR=color;
    LCD5510_DrawPoint(x,y);//中心点
    LCD5510_DrawPoint(x+1,y);
    LCD5510_DrawPoint(x,y+1);
    LCD5510_DrawPoint(x+1,y+1);
}
//////////////////////////////////////////////////////////////////////////////////
//触摸按键扫描
//tp:0,屏幕坐标;1,物理坐标(校准等特殊场合用)
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
u8 TP_Scan(u8 tp)
{
    if(PEN==0)//有按键按下
    {
        if(tp)TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]);//读取物理坐标
        else if(TP_Read_XY2(&tp_dev.x[0],&tp_dev.y[0]))//读取屏幕坐标
        {
            tp_dev.x[0]=tp_dev.xfac*tp_dev.x[0]+tp_dev.xoff;//将结果转换为屏幕坐标
            tp_dev.y[0]=tp_dev.yfac*tp_dev.y[0]+tp_dev.yoff;
        }
        if((tp_dev.sta&TP_PRES_DOWN)==0)//之前没有被按下
        {
            tp_dev.sta=TP_PRES_DOWN|TP_CATH_PRES;//按键按下
            tp_dev.x[4]=tp_dev.x[0];//记录第一次按下时的坐标
            tp_dev.y[4]=tp_dev.y[0];
        }
    } else
    {
        if(tp_dev.sta&TP_PRES_DOWN)//之前是被按下的
        {
            tp_dev.sta&=~(1<<7);//标记按键松开
        } else//之前就没有被按下
        {
            tp_dev.x[4]=0;
            tp_dev.y[4]=0;
            tp_dev.x[0]=0xffff;
            tp_dev.y[0]=0xffff;
        }
    }
    return tp_dev.sta&TP_PRES_DOWN;//返回当前的触屏状态
}
//////////////////////////////////////////////////////////////////////////
//保存在EEPROM里面的地址区间基址,占用13个字节(RANGE:SAVE_ADDR_BASE~SAVE_ADDR_BASE+12)
#define SAVE_ADDR_BASE 40
//保存校准参数
void TP_Save_Adjdata(void)
{
    s32 temp;
    //保存校正结果!
    temp=tp_dev.xfac*100000000;//保存x校正因素
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE,temp,4);
    temp=tp_dev.yfac*100000000;//保存y校正因素
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+4,temp,4);
    //保存x偏移量
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+8,tp_dev.xoff,2);
    //保存y偏移量
    AT24CXX_WriteLenByte(SAVE_ADDR_BASE+10,tp_dev.yoff,2);
    //保存触屏类型
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE+12,tp_dev.touchtype);
    temp=0X0A;//标记校准过了
    AT24CXX_WriteOneByte(SAVE_ADDR_BASE+13,temp);
}
//得到保存在EEPROM里面的校准值
//返回值：1，成功获取数据
//        0，获取失败，要重新校准
u8 TP_Get_Adjdata(void)
{
    s32 tempfac;
    tempfac=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+13);//读取标记字,看是否校准过！
    if(tempfac==0X0A)//触摸屏已经校准过了
    {
        tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE,4);
        tp_dev.xfac=(float)tempfac/100000000;//得到x校准参数
        tempfac=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+4,4);
        tp_dev.yfac=(float)tempfac/100000000;//得到y校准参数
        //得到x偏移量
        tp_dev.xoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+8,2);
        //得到y偏移量
        tp_dev.yoff=AT24CXX_ReadLenByte(SAVE_ADDR_BASE+10,2);
        tp_dev.touchtype=AT24CXX_ReadOneByte(SAVE_ADDR_BASE+12);//读取触屏类型标记
        if(tp_dev.touchtype)//X,Y方向与屏幕相反
        {
            CMD_RDX=0X90;
            CMD_RDY=0XD0;
        } else				   //X,Y方向与屏幕相同
        {
            CMD_RDX=0XD0;
            CMD_RDY=0X90;
        }
        return 1;
    }
    return 0;
}
//提示字符串
const u8* TP_REMIND_MSG_TBL="Please use the stylus click the cross on the screen.The cross will always move until the screen adjustment is completed.";

//提示校准结果(各个参数)
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
    LCD5510_ShowNum(40+24,160,x0,4,16);		//显示数值
    LCD5510_ShowNum(40+24+80,160,y0,4,16);	//显示数值
    LCD5510_ShowNum(40+24,180,x1,4,16);		//显示数值
    LCD5510_ShowNum(40+24+80,180,y1,4,16);	//显示数值
    LCD5510_ShowNum(40+24,200,x2,4,16);		//显示数值
    LCD5510_ShowNum(40+24+80,200,y2,4,16);	//显示数值
    LCD5510_ShowNum(40+24,220,x3,4,16);		//显示数值
    LCD5510_ShowNum(40+24+80,220,y3,4,16);	//显示数值
    LCD5510_ShowNum(40+56,lcddev.width,fac,3,16); 	//显示数值,该数值必须在95~105范围之内.

}

//触摸屏校准代码
//得到四个校准参数
void TP_Adjust(void)
{
    u16 pos_temp[4][2];//坐标缓存值
    u8  cnt=0;
    u16 d1,d2;
    u32 tem1,tem2;
    float fac;
    u16 outtime=0;
    cnt=0;
    POINT_COLOR=BLUE;
    BACK_COLOR =WHITE;
    LCD5510_Clear(WHITE);//清屏
    POINT_COLOR=RED;//红色
    LCD5510_Clear(WHITE);//清屏
    POINT_COLOR=BLACK;
    LCD5510_ShowString(40,40,160,100,16,(u8*)TP_REMIND_MSG_TBL);//显示提示信息
    TP_Drow_Touch_Point(20,20,RED);//画点1
    tp_dev.sta=0;//消除触发信号
    tp_dev.xfac=0;//xfac用来标记是否校准过,所以校准之前必须清掉!以免错误
    while(1)//如果连续10秒钟没有按下,则自动退出
    {
        tp_dev.scan(1);//扫描物理坐标
        if((tp_dev.sta&0xc0)==TP_CATH_PRES)//按键按下了一次(此时按键松开了.)
        {
            outtime=0;
            tp_dev.sta&=~(1<<6);//标记按键已经被处理过了.

            pos_temp[cnt][0]=tp_dev.x[0];
            pos_temp[cnt][1]=tp_dev.y[0];
            cnt++;
            switch(cnt)
            {
            case 1:
                TP_Drow_Touch_Point(20,20,WHITE);				//清除点1
                TP_Drow_Touch_Point(lcddev.width-20,20,RED);	//画点2
                break;
            case 2:
                TP_Drow_Touch_Point(lcddev.width-20,20,WHITE);	//清除点2
                TP_Drow_Touch_Point(20,lcddev.height-20,RED);	//画点3
                break;
            case 3:
                TP_Drow_Touch_Point(20,lcddev.height-20,WHITE);			//清除点3
                TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,RED);	//画点4
                break;
            case 4:	 //全部四个点已经得到
                //对边相等
                tem1=abs(pos_temp[0][0]-pos_temp[1][0]);//x1-x2
                tem2=abs(pos_temp[0][1]-pos_temp[1][1]);//y1-y2
                tem1*=tem1;
                tem2*=tem2;
                d1=sqrt(tem1+tem2);//得到1,2的距离

                tem1=abs(pos_temp[2][0]-pos_temp[3][0]);//x3-x4
                tem2=abs(pos_temp[2][1]-pos_temp[3][1]);//y3-y4
                tem1*=tem1;
                tem2*=tem2;
                d2=sqrt(tem1+tem2);//得到3,4的距离
                fac=(float)d1/d2;
                if(fac<0.95||fac>1.05||d1==0||d2==0)//不合格
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                    TP_Drow_Touch_Point(20,20,RED);								//画点1
                    TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据
                    continue;
                }
                tem1=abs(pos_temp[0][0]-pos_temp[2][0]);//x1-x3
                tem2=abs(pos_temp[0][1]-pos_temp[2][1]);//y1-y3
                tem1*=tem1;
                tem2*=tem2;
                d1=sqrt(tem1+tem2);//得到1,3的距离

                tem1=abs(pos_temp[1][0]-pos_temp[3][0]);//x2-x4
                tem2=abs(pos_temp[1][1]-pos_temp[3][1]);//y2-y4
                tem1*=tem1;
                tem2*=tem2;
                d2=sqrt(tem1+tem2);//得到2,4的距离
                fac=(float)d1/d2;
                if(fac<0.95||fac>1.05)//不合格
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                    TP_Drow_Touch_Point(20,20,RED);								//画点1
                    TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据
                    continue;
                }//正确了

                //对角线相等
                tem1=abs(pos_temp[1][0]-pos_temp[2][0]);//x1-x3
                tem2=abs(pos_temp[1][1]-pos_temp[2][1]);//y1-y3
                tem1*=tem1;
                tem2*=tem2;
                d1=sqrt(tem1+tem2);//得到1,4的距离

                tem1=abs(pos_temp[0][0]-pos_temp[3][0]);//x2-x4
                tem2=abs(pos_temp[0][1]-pos_temp[3][1]);//y2-y4
                tem1*=tem1;
                tem2*=tem2;
                d2=sqrt(tem1+tem2);//得到2,3的距离
                fac=(float)d1/d2;
                if(fac<0.95||fac>1.05)//不合格
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                    TP_Drow_Touch_Point(20,20,RED);								//画点1
                    TP_Adj_Info_Show(pos_temp[0][0],pos_temp[0][1],pos_temp[1][0],pos_temp[1][1],pos_temp[2][0],pos_temp[2][1],pos_temp[3][0],pos_temp[3][1],fac*100);//显示数据
                    continue;
                }//正确了
                //计算结果
                tp_dev.xfac=(float)(lcddev.width-40)/(pos_temp[1][0]-pos_temp[0][0]);//得到xfac
                tp_dev.xoff=(lcddev.width-tp_dev.xfac*(pos_temp[1][0]+pos_temp[0][0]))/2;//得到xoff

                tp_dev.yfac=(float)(lcddev.height-40)/(pos_temp[2][1]-pos_temp[0][1]);//得到yfac
                tp_dev.yoff=(lcddev.height-tp_dev.yfac*(pos_temp[2][1]+pos_temp[0][1]))/2;//得到yoff
                if(abs(tp_dev.xfac)>2||abs(tp_dev.yfac)>2)//触屏和预设的相反了.
                {
                    cnt=0;
                    TP_Drow_Touch_Point(lcddev.width-20,lcddev.height-20,WHITE);	//清除点4
                    TP_Drow_Touch_Point(20,20,RED);								//画点1
                    LCD5510_ShowString(40,26,lcddev.width,lcddev.height,16,"TP Need readjust!");
                    tp_dev.touchtype=!tp_dev.touchtype;//修改触屏类型.
                    if(tp_dev.touchtype)//X,Y方向与屏幕相反
                    {
                        CMD_RDX=0X90;
                        CMD_RDY=0XD0;
                    } else				   //X,Y方向与屏幕相同
                    {
                        CMD_RDX=0XD0;
                        CMD_RDY=0X90;
                    }
                    continue;
                }
                POINT_COLOR=BLUE;
                LCD5510_Clear(WHITE);//清屏
                LCD5510_ShowString(35,110,lcddev.width,lcddev.height,16,"Touch Screen Adjust OK!");//校正完成
                delay_ms(1000);
                TP_Save_Adjdata();
                LCD5510_Clear(WHITE);//清屏
                return;//校正完成
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
//触摸屏初始化
//返回值:0,没有进行校准
//       1,进行过校准
u8 TP_Init(void)
{
    if(lcddev.id==0X5510)				//电容触摸屏
    {
        if(GT9147_Init()==0)			//是GT9147
        {
            tp_dev.scan=GT9147_Scan;	//扫描函数指向GT9147触摸屏扫描
        }
        else
        {
            OTT2001A_Init();
            tp_dev.scan=OTT2001A_Scan;	//扫描函数指向OTT2001A触摸屏扫描
        }
        tp_dev.touchtype|=0X80;			//电容屏
        tp_dev.touchtype|=lcddev.dir&0X01;//横屏还是竖屏
        return 0;
    }
    else
    {
        //注意,时钟使能之后,对GPIO的操作才有效
        //所以上拉之前,必须使能时钟.才能实现真正的上拉输出
        GPIO_InitTypeDef  GPIO_InitStructure;

        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOF, ENABLE);	 //使能PB,PF端口时钟

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;				 // PB1,PB2端口配置
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_Init(GPIOB, &GPIO_InitStructure);
        GPIO_SetBits(GPIOB,GPIO_Pin_1|GPIO_Pin_2);//上拉


        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;				 // PF9端口配置	推挽输出
        GPIO_Init(GPIOF, &GPIO_InitStructure);
        GPIO_SetBits(GPIOF,GPIO_Pin_9);//P9 上拉

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10;	// PF8.PF10端口配置
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //	上拉输入
        GPIO_Init(GPIOF, &GPIO_InitStructure);
        TP_Read_XY(&tp_dev.x[0],&tp_dev.y[0]);//第一次读取初始化
        AT24CXX_Init();//初始化24CXX
        if(TP_Get_Adjdata())return 0;//已经校准
        else			   //未校准?
        {
            LCD5510_Clear(WHITE);//清屏
            TP_Adjust();  //屏幕校准
            TP_Save_Adjdata();
        }
        TP_Get_Adjdata();
    }
    return 1;
}




































