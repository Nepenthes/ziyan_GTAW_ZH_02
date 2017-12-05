#include "touch.h"
#include "lcd5510.h"
#include "font.h"
//////////////////////////////////////////////////////////////////////////////////
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//2.4寸/2.8寸/3.5寸/4.3寸 TFT液晶驱动
//支持驱动IC型号包括:ILI9341/ILI9325/RM68042/RM68021/ILI9320/ILI9328/LGDP4531/LGDP4535/
//                  SPFD5408/SSD1289/1505/B505/C505/NT35310/NT35510等
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/2/11
//版本：V2.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.2修改说明
//支持了SPFD5408的驱动,另外把液晶ID直接打印成HEX格式.方便查看LCD驱动IC.
//V1.3
//加入了快速IO的支持
//修改了背光控制的极性（适用于V1.8及以后的开发板版本）
//对于1.8版本之前(不包括1.8)的液晶模块,请修改LCD5510_Init函数的LCD5510_LED=1;为LCD5510_LED=1;
//V1.4
//修改了LCD5510_ShowChar函数，使用画点功能画字符。
//加入了横竖屏显示的支持
//V1.5 20110730
//1,修改了B505液晶读颜色有误的bug.
//2,修改了快速IO及横竖屏的设置方式.
//V1.6 20111116
//1,加入对LGDP4535液晶的驱动支持
//V1.7 20120713
//1,增加LCD5510_RD_DATA函数
//2,增加对ILI9341的支持
//3,增加ILI9325的独立驱动代码
//4,增加LCD5510_Scan_Dir函数(慎重使用)
//6,另外修改了部分原来的函数,以适应9341的操作
//V1.8 20120905
//1,加入LCD重要参数设置结构体lcddev
//2,加入LCD5510_Display_Dir函数,支持在线横竖屏切换
//V1.9 20120911
//1,新增RM68042驱动（ID:6804），但是6804不支持横屏显示！！原因：改变扫描方式，
//导致6804坐标设置失效，试过很多方法都不行，暂时无解。
//V2.0 20120924
//在不硬件复位的情况下,ILI9341的ID读取会被误读成9300,修改LCD5510_Init,将无法识别
//的情况（读到ID为9300/非法ID）,强制指定驱动IC为ILI9341，执行9341的初始化。
//V2.1 20120930
//修正ILI9325读颜色的bug。
//V2.2 20121007
//修正LCD5510_Scan_Dir的bug。
//V2.3 20130120
//新增6804支持横屏显示
//V2.4 20131120
//1,新增NT35310（ID:5310）驱动器的支持
//2,新增LCD5510_Set_Window函数,用于设置窗口,对快速填充,比较有用,但是该函数在横屏时,不支持6804.
//V2.5 20140211
//1,新增NT35510（ID:5510）驱动器的支持
//////////////////////////////////////////////////////////////////////////////////

//LCD的画笔颜色和背景色
u16 POINT_COLOR=0x0000;	//画笔颜色
u16 BACK_COLOR=0xFFFF;  //背景色

//管理LCD重要参数
//默认为竖屏
_lcd_dev lcddev;

//写寄存器函数
//regval:寄存器值
void LCD5510_WR_REG(u16 regval)
{
    LCD5510_ADDR->LCD5510_REG=regval;//写入要写的寄存器序号
}
//写LCD数据
//data:要写入的值
void LCD5510_WR_DATA(u16 data)
{
    LCD5510_ADDR->LCD5510_RAM=data;
}
//读LCD数据
//返回值:读到的值
u16 LCD5510_RD_DATA(void)
{
    return LCD5510_ADDR->LCD5510_RAM;
}
//写寄存器
//LCD5510_Reg:寄存器地址
//LCD5510_RegValue:要写入的数据
void LCD5510_WriteReg(u16 LCD5510_Reg, u16 LCD5510_RegValue)
{
    LCD5510_ADDR->LCD5510_REG = LCD5510_Reg;		//写入要写的寄存器序号
    LCD5510_ADDR->LCD5510_RAM = LCD5510_RegValue;//写入数据
}
//读寄存器
//LCD5510_Reg:寄存器地址
//返回值:读到的数据
u16 LCD5510_ReadReg(u16 LCD5510_Reg)
{
    LCD5510_WR_REG(LCD5510_Reg);		//写入要读的寄存器序号
    delay_us(5);
    return LCD5510_RD_DATA();		//返回读到的值
}
//开始写GRAM
void LCD5510_WriteRAM_Prepare(void)
{
    LCD5510_ADDR->LCD5510_REG=lcddev.wramcmd;
}
//LCD写GRAM
//RGB_Code:颜色值
void LCD5510_WriteRAM(u16 RGB_Code)
{
    LCD5510_ADDR->LCD5510_RAM = RGB_Code;//写十六位GRAM
}
//从ILI93xx读出的数据为GBR格式，而我们写入的时候为RGB格式。
//通过该函数转换
//c:GBR格式的颜色值
//返回值：RGB格式的颜色值
u16 LCD5510_BGR2RGB(u16 c)
{
    u16  r,g,b,rgb;
    b=(c>>0)&0x1f;
    g=(c>>5)&0x3f;
    r=(c>>11)&0x1f;
    rgb=(b<<11)+(g<<5)+(r<<0);
    return(rgb);
}
//当mdk -O1时间优化时需要设置
//延时i
void opt_delay(u8 i)
{
    while(i--);
}
//读取个某点的颜色值
//x,y:坐标
//返回值:此点的颜色
u16 LCD5510_ReadPoint(u16 x,u16 y)
{
    u16 r=0,g=0,b=0;
    if(x>=lcddev.width||y>=lcddev.height)return 0;	//超过了范围,直接返回
    LCD5510_SetCursor(x,y);
    if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD5510_WR_REG(0X2E);//9341/6804/3510 发送读GRAM指令
    else if(lcddev.id==0X5510)LCD5510_WR_REG(0X2E00);	//5510 发送读GRAM指令
    else LCD5510_WR_REG(R34);      		 				//其他IC发送读GRAM指令
    if(lcddev.id==0X9320)opt_delay(2);				//FOR 9320,延时2us
    if(LCD5510_ADDR->LCD5510_RAM)r=0;							//dummy Read
    opt_delay(2);
    r=LCD5510_ADDR->LCD5510_RAM;  		  						//实际坐标颜色
    if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510)		//9341/NT35310/NT35510要分2次读出
    {
        opt_delay(2);
        b=LCD5510_ADDR->LCD5510_RAM;
        g=r&0XFF;		//对于9341/5310/5510,第一次读取的是RG的值,R在前,G在后,各占8位
        g<<=8;
    } else if(lcddev.id==0X6804)r=LCD5510_ADDR->LCD5510_RAM;		//6804第二次读取的才是真实值
    if(lcddev.id==0X9325||lcddev.id==0X4535||lcddev.id==0X4531||lcddev.id==0X8989||lcddev.id==0XB505)return r;	//这几种IC直接返回颜色值
    else if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X5510)return (((r>>11)<<11)|((g>>10)<<5)|(b>>11));//ILI9341/NT35310/NT35510需要公式转换一下
    else return LCD5510_BGR2RGB(r);						//其他IC
}
//LCD开启显示
void LCD5510_DisplayOn(void)
{
    if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD5510_WR_REG(0X29);	//开启显示
    else if(lcddev.id==0X5510)LCD5510_WR_REG(0X2900);	//开启显示
    else LCD5510_WriteReg(R7,0x0173); 				 	//开启显示
}
//LCD关闭显示
void LCD5510_DisplayOff(void)
{
    if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)LCD5510_WR_REG(0X28);	//关闭显示
    else if(lcddev.id==0X5510)LCD5510_WR_REG(0X2800);	//关闭显示
    else LCD5510_WriteReg(R7,0x0);//关闭显示
}
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCD5510_SetCursor(u16 Xpos, u16 Ypos)
{
    if(lcddev.id==0X9341||lcddev.id==0X5310)
    {
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(Xpos>>8);
        LCD5510_WR_DATA(Xpos&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(Ypos>>8);
        LCD5510_WR_DATA(Ypos&0XFF);
    } else if(lcddev.id==0X6804)
    {
        if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//横屏时处理
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(Xpos>>8);
        LCD5510_WR_DATA(Xpos&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(Ypos>>8);
        LCD5510_WR_DATA(Ypos&0XFF);
    } else if(lcddev.id==0X5510)
    {
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(Xpos>>8);
        LCD5510_WR_REG(lcddev.setxcmd+1);
        LCD5510_WR_DATA(Xpos&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(Ypos>>8);
        LCD5510_WR_REG(lcddev.setycmd+1);
        LCD5510_WR_DATA(Ypos&0XFF);
    } else
    {
        if(lcddev.dir==1)Xpos=lcddev.width-1-Xpos;//横屏其实就是调转x,y坐标
        LCD5510_WriteReg(lcddev.setxcmd, Xpos);
        LCD5510_WriteReg(lcddev.setycmd, Ypos);
    }
}
//设置LCD的自动扫描方向
//注意:其他函数可能会受到此函数设置的影响(尤其是9341/6804这两个奇葩),
//所以,一般设置为L2R_U2D即可,如果设置为其他扫描方式,可能导致显示不正常.
//dir:0~7,代表8个方向(具体定义见lcd.h)
//9320/9325/9328/4531/4535/1505/b505/8989/5408/9341/5310/5510等IC已经实际测试
void LCD5510_Scan_Dir(u8 dir)
{
    u16 regval=0;
    u16 dirreg=0;
    u16 temp;
    if(lcddev.dir==1&&lcddev.id!=0X6804)//横屏时，对6804不改变扫描方向！
    {
        switch(dir)//方向转换
        {
        case 0:
            dir=6;
            break;
        case 1:
            dir=7;
            break;
        case 2:
            dir=4;
            break;
        case 3:
            dir=5;
            break;
        case 4:
            dir=1;
            break;
        case 5:
            dir=0;
            break;
        case 6:
            dir=3;
            break;
        case 7:
            dir=2;
            break;
        }
    }
    if(lcddev.id==0x9341||lcddev.id==0X6804||lcddev.id==0X5310||lcddev.id==0X5510)//9341/6804/5310/5510,很特殊
    {
        switch(dir)
        {
        case L2R_U2D://从左到右,从上到下
            regval|=(0<<7)|(0<<6)|(0<<5);
            break;
        case L2R_D2U://从左到右,从下到上
            regval|=(1<<7)|(0<<6)|(0<<5);
            break;
        case R2L_U2D://从右到左,从上到下
            regval|=(0<<7)|(1<<6)|(0<<5);
            break;
        case R2L_D2U://从右到左,从下到上
            regval|=(1<<7)|(1<<6)|(0<<5);
            break;
        case U2D_L2R://从上到下,从左到右
            regval|=(0<<7)|(0<<6)|(1<<5);
            break;
        case U2D_R2L://从上到下,从右到左
            regval|=(0<<7)|(1<<6)|(1<<5);
            break;
        case D2U_L2R://从下到上,从左到右
            regval|=(1<<7)|(0<<6)|(1<<5);
            break;
        case D2U_R2L://从下到上,从右到左
            regval|=(1<<7)|(1<<6)|(1<<5);
            break;
        }
        if(lcddev.id==0X5510)dirreg=0X3600;
        else dirreg=0X36;
        if((lcddev.id!=0X5310)&&(lcddev.id!=0X5510))regval|=0X08;//5310/5510不需要BGR
        if(lcddev.id==0X6804)regval|=0x02;//6804的BIT6和9341的反了
        LCD5510_WriteReg(dirreg,regval);
        if((regval&0X20)||lcddev.dir==1)
        {
            if(lcddev.width<lcddev.height)//交换X,Y
            {
                temp=lcddev.width;
                lcddev.width=lcddev.height;
                lcddev.height=temp;
            }
        } else
        {
            if(lcddev.width>lcddev.height)//交换X,Y
            {
                temp=lcddev.width;
                lcddev.width=lcddev.height;
                lcddev.height=temp;
            }
        }
        if(lcddev.id==0X5510)
        {
            LCD5510_WR_REG(lcddev.setxcmd);
            LCD5510_WR_DATA(0);
            LCD5510_WR_REG(lcddev.setxcmd+1);
            LCD5510_WR_DATA(0);
            LCD5510_WR_REG(lcddev.setxcmd+2);
            LCD5510_WR_DATA((lcddev.width-1)>>8);
            LCD5510_WR_REG(lcddev.setxcmd+3);
            LCD5510_WR_DATA((lcddev.width-1)&0XFF);
            LCD5510_WR_REG(lcddev.setycmd);
            LCD5510_WR_DATA(0);
            LCD5510_WR_REG(lcddev.setycmd+1);
            LCD5510_WR_DATA(0);
            LCD5510_WR_REG(lcddev.setycmd+2);
            LCD5510_WR_DATA((lcddev.height-1)>>8);
            LCD5510_WR_REG(lcddev.setycmd+3);
            LCD5510_WR_DATA((lcddev.height-1)&0XFF);
        } else
        {
            LCD5510_WR_REG(lcddev.setxcmd);
            LCD5510_WR_DATA(0);
            LCD5510_WR_DATA(0);
            LCD5510_WR_DATA((lcddev.width-1)>>8);
            LCD5510_WR_DATA((lcddev.width-1)&0XFF);
            LCD5510_WR_REG(lcddev.setycmd);
            LCD5510_WR_DATA(0);
            LCD5510_WR_DATA(0);
            LCD5510_WR_DATA((lcddev.height-1)>>8);
            LCD5510_WR_DATA((lcddev.height-1)&0XFF);
        }
    } else
    {
        switch(dir)
        {
        case L2R_U2D://从左到右,从上到下
            regval|=(1<<5)|(1<<4)|(0<<3);
            break;
        case L2R_D2U://从左到右,从下到上
            regval|=(0<<5)|(1<<4)|(0<<3);
            break;
        case R2L_U2D://从右到左,从上到下
            regval|=(1<<5)|(0<<4)|(0<<3);
            break;
        case R2L_D2U://从右到左,从下到上
            regval|=(0<<5)|(0<<4)|(0<<3);
            break;
        case U2D_L2R://从上到下,从左到右
            regval|=(1<<5)|(1<<4)|(1<<3);
            break;
        case U2D_R2L://从上到下,从右到左
            regval|=(1<<5)|(0<<4)|(1<<3);
            break;
        case D2U_L2R://从下到上,从左到右
            regval|=(0<<5)|(1<<4)|(1<<3);
            break;
        case D2U_R2L://从下到上,从右到左
            regval|=(0<<5)|(0<<4)|(1<<3);
            break;
        }
        if(lcddev.id==0x8989)//8989 IC
        {
            dirreg=0X11;
            regval|=0X6040;	//65K
        } else//其他驱动IC
        {
            dirreg=0X03;
            regval|=1<<12;
        }
        LCD5510_WriteReg(dirreg,regval);
    }
}
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCD5510_DrawPoint(u16 x,u16 y)
{
    LCD5510_SetCursor(x,y);		//设置光标位置
    LCD5510_WriteRAM_Prepare();	//开始写入GRAM
    LCD5510_ADDR->LCD5510_RAM=POINT_COLOR;
}
//快速画点
//x,y:坐标
//color:颜色
void LCD5510_Fast_DrawPoint(u16 x,u16 y,u16 color)
{
    if(lcddev.id==0X9341||lcddev.id==0X5310)
    {
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(x>>8);
        LCD5510_WR_DATA(x&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(y>>8);
        LCD5510_WR_DATA(y&0XFF);
    } else if(lcddev.id==0X5510)
    {
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(x>>8);
        LCD5510_WR_REG(lcddev.setxcmd+1);
        LCD5510_WR_DATA(x&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(y>>8);
        LCD5510_WR_REG(lcddev.setycmd+1);
        LCD5510_WR_DATA(y&0XFF);
    } else if(lcddev.id==0X6804)
    {
        if(lcddev.dir==1)x=lcddev.width-1-x;//横屏时处理
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(x>>8);
        LCD5510_WR_DATA(x&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(y>>8);
        LCD5510_WR_DATA(y&0XFF);
    } else
    {
        if(lcddev.dir==1)x=lcddev.width-1-x;//横屏其实就是调转x,y坐标
        LCD5510_WriteReg(lcddev.setxcmd,x);
        LCD5510_WriteReg(lcddev.setycmd,y);
    }
    LCD5510_ADDR->LCD5510_REG=lcddev.wramcmd;
    LCD5510_ADDR->LCD5510_RAM=color;
}


//设置LCD显示方向
//dir:0,竖屏；1,横屏
void LCD5510_Display_Dir(u8 dir)
{
    if(dir==0)			//竖屏
    {
        lcddev.dir=0;	//竖屏
        lcddev.width=240;
        lcddev.height=320;
        if(lcddev.id==0X9341||lcddev.id==0X6804||lcddev.id==0X5310)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2A;
            lcddev.setycmd=0X2B;
            if(lcddev.id==0X6804||lcddev.id==0X5310)
            {
                lcddev.width=320;
                lcddev.height=480;
            }
        } else if(lcddev.id==0X8989)
        {
            lcddev.wramcmd=R34;
            lcddev.setxcmd=0X4E;
            lcddev.setycmd=0X4F;
        } else if(lcddev.id==0x5510)
        {
            lcddev.wramcmd=0X2C00;
            lcddev.setxcmd=0X2A00;
            lcddev.setycmd=0X2B00;
            lcddev.width=480;
            lcddev.height=800;
        } else
        {
            lcddev.wramcmd=R34;
            lcddev.setxcmd=R32;
            lcddev.setycmd=R33;
        }
    } else 				//横屏
    {
        lcddev.dir=1;	//横屏
        lcddev.width=320;
        lcddev.height=240;
        if(lcddev.id==0X9341||lcddev.id==0X5310)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2A;
            lcddev.setycmd=0X2B;
        } else if(lcddev.id==0X6804)
        {
            lcddev.wramcmd=0X2C;
            lcddev.setxcmd=0X2B;
            lcddev.setycmd=0X2A;
        } else if(lcddev.id==0X8989)
        {
            lcddev.wramcmd=R34;
            lcddev.setxcmd=0X4F;
            lcddev.setycmd=0X4E;
        } else if(lcddev.id==0x5510)
        {
            lcddev.wramcmd=0X2C00;
            lcddev.setxcmd=0X2A00;
            lcddev.setycmd=0X2B00;
            lcddev.width=800;
            lcddev.height=480;
        } else
        {
            lcddev.wramcmd=R34;
            lcddev.setxcmd=R33;
            lcddev.setycmd=R32;
        }
        if(lcddev.id==0X6804||lcddev.id==0X5310)
        {
            lcddev.width=480;
            lcddev.height=320;
        }
    }
    LCD5510_Scan_Dir(DFT_SCAN_DIR);	//默认扫描方向
}
//设置窗口,并自动设置画点坐标到窗口左上角(sx,sy).
//sx,sy:窗口起始坐标(左上角)
//width,height:窗口宽度和高度,必须大于0!!
//窗体大小:width*height.
//68042,横屏时不支持窗口设置!!
void LCD5510_Set_Window(u16 sx,u16 sy,u16 width,u16 height)
{
    u8 hsareg,heareg,vsareg,veareg;
    u16 hsaval,heaval,vsaval,veaval;
    width=sx+width-1;
    height=sy+height-1;
    if(lcddev.id==0X9341||lcddev.id==0X5310||lcddev.id==0X6804)//6804横屏不支持
    {
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(sx>>8);
        LCD5510_WR_DATA(sx&0XFF);
        LCD5510_WR_DATA(width>>8);
        LCD5510_WR_DATA(width&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(sy>>8);
        LCD5510_WR_DATA(sy&0XFF);
        LCD5510_WR_DATA(height>>8);
        LCD5510_WR_DATA(height&0XFF);
    } else if(lcddev.id==0X5510)
    {
        LCD5510_WR_REG(lcddev.setxcmd);
        LCD5510_WR_DATA(sx>>8);
        LCD5510_WR_REG(lcddev.setxcmd+1);
        LCD5510_WR_DATA(sx&0XFF);
        LCD5510_WR_REG(lcddev.setxcmd+2);
        LCD5510_WR_DATA(width>>8);
        LCD5510_WR_REG(lcddev.setxcmd+3);
        LCD5510_WR_DATA(width&0XFF);
        LCD5510_WR_REG(lcddev.setycmd);
        LCD5510_WR_DATA(sy>>8);
        LCD5510_WR_REG(lcddev.setycmd+1);
        LCD5510_WR_DATA(sy&0XFF);
        LCD5510_WR_REG(lcddev.setycmd+2);
        LCD5510_WR_DATA(height>>8);
        LCD5510_WR_REG(lcddev.setycmd+3);
        LCD5510_WR_DATA(height&0XFF);
    } else	//其他驱动IC
    {
        if(lcddev.dir==1)//横屏
        {
            //窗口值
            hsaval=sy;
            heaval=height;
            vsaval=lcddev.width-width-1;
            veaval=lcddev.width-sx-1;
        } else
        {
            hsaval=sx;
            heaval=width;
            vsaval=sy;
            veaval=height;
        }
        if(lcddev.id==0X8989)//8989 IC
        {
            hsareg=0X44;
            heareg=0X44;//水平方向窗口寄存器 (1289的由一个寄存器控制)
            hsaval|=(heaval<<8);	//得到寄存器值.
            heaval=hsaval;
            vsareg=0X45;
            veareg=0X46;//垂直方向窗口寄存器
        } else  //其他驱动IC
        {
            hsareg=0X50;
            heareg=0X51;//水平方向窗口寄存器
            vsareg=0X52;
            veareg=0X53;//垂直方向窗口寄存器
        }
        //设置寄存器值
        LCD5510_WriteReg(hsareg,hsaval);
        LCD5510_WriteReg(heareg,heaval);
        LCD5510_WriteReg(vsareg,vsaval);
        LCD5510_WriteReg(veareg,veaval);
        LCD5510_SetCursor(sx,sy);	//设置光标位置
    }
}
//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试!
void LCD5510_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  readWriteTiming;
    FSMC_NORSRAMTimingInitTypeDef  writeTiming;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC,ENABLE);	//使能FSMC时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOD|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOG|RCC_APB2Periph_AFIO,ENABLE);//使能PORTB,D,E,G以及AFIO复用功能时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;				 //PB0 推挽输出 背光
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    //PORTD复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //PORTE复用推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;				 //	//PORTD复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    //	//PORTG12复用推挽输出 A16
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;	 //	//PORTD复用推挽输出
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 		 //复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	 //	//CS    连接的IO口， 没连接到NE上，所以让CS=0，一直选中。
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    //GPIO_SetBits(GPIOD,GPIO_Pin_6);
    GPIO_ResetBits(GPIOD,GPIO_Pin_6);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;	 //	RST
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
    //GPIO_SetBits(GPIOE,GPIO_Pin_1);
    //GPIO_ResetBits(GPIOE,GPIO_Pin_1);

    readWriteTiming.FSMC_AddressSetupTime = 0x01;	 //地址建立时间（ADDSET）为2个HCLK 1/36M=27ns
    readWriteTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（ADDHLD）模式A未用到
    readWriteTiming.FSMC_DataSetupTime = 0x0f;		 // 数据保存时间为16个HCLK,因为液晶驱动IC的读数据的时候，速度不能太快，尤其对1289这个IC。
    readWriteTiming.FSMC_BusTurnAroundDuration = 0x00;
    readWriteTiming.FSMC_CLKDivision = 0x00;
    readWriteTiming.FSMC_DataLatency = 0x00;
    readWriteTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A


    writeTiming.FSMC_AddressSetupTime = 0x00;	 //地址建立时间（ADDSET）为1个HCLK
    writeTiming.FSMC_AddressHoldTime = 0x00;	 //地址保持时间（A
    writeTiming.FSMC_DataSetupTime = 0x03;		 ////数据保存时间为4个HCLK
    writeTiming.FSMC_BusTurnAroundDuration = 0x00;
    writeTiming.FSMC_CLKDivision = 0x00;
    writeTiming.FSMC_DataLatency = 0x00;
    writeTiming.FSMC_AccessMode = FSMC_AccessMode_A;	 //模式A


    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;//  这里我们使用NE4 ，也就对应BTCR[6],[7]。
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; // 不复用数据地址
    FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//  存储器写使能
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable; // 读写使用不同的时序
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &readWriteTiming; //读写时序
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &writeTiming;  //写时序

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);  // 使能BANK1


    GPIO_ResetBits(GPIOE,GPIO_Pin_1);
    delay_ms(50); // delay 50 ms
    GPIO_SetBits(GPIOE,GPIO_Pin_1);
    delay_ms(50);
    LCD5510_WriteReg(0x0000,0x0001);
    delay_ms(50); // delay 50 ms
    lcddev.id = LCD5510_ReadReg(0x0000);
    if(lcddev.id<0XFF||lcddev.id==0XFFFF||lcddev.id==0X9300)//读到ID不正确,新增lcddev.id==0X9300判断，因为9341在未被复位的情况下会被读成9300
    {
        //尝试9341 ID的读取
        LCD5510_WR_REG(0XD3);
        LCD5510_RD_DATA(); 				//dummy read
        LCD5510_RD_DATA();   	    	//读到0X00
        lcddev.id=LCD5510_RD_DATA();   	//读取93
        lcddev.id<<=8;
        lcddev.id|=LCD5510_RD_DATA();  	//读取41
        if(lcddev.id!=0X9341)		//非9341,尝试是不是6804
        {
            LCD5510_WR_REG(0XBF);
            LCD5510_RD_DATA(); 			//dummy read
            LCD5510_RD_DATA();   	    //读回0X01
            LCD5510_RD_DATA(); 			//读回0XD0
            lcddev.id=LCD5510_RD_DATA();//这里读回0X68
            lcddev.id<<=8;
            lcddev.id|=LCD5510_RD_DATA();//这里读回0X04
            if(lcddev.id!=0X6804)	//也不是6804,尝试看看是不是NT35310
            {
                LCD5510_WR_REG(0XD4);
                LCD5510_RD_DATA(); 				//dummy read
                LCD5510_RD_DATA();   			//读回0X01
                lcddev.id=LCD5510_RD_DATA();	//读回0X53
                lcddev.id<<=8;
                lcddev.id|=LCD5510_RD_DATA();	//这里读回0X10
                if(lcddev.id!=0X5310)		//也不是NT35310,尝试看看是不是NT35510
                {
                    LCD5510_WR_REG(0XDA00);
                    LCD5510_RD_DATA();   		//读回0X00
                    LCD5510_WR_REG(0XDB00);
                    lcddev.id=LCD5510_RD_DATA();//读回0X80
                    lcddev.id<<=8;
                    LCD5510_WR_REG(0XDC00);
                    lcddev.id|=LCD5510_RD_DATA();//读回0X00
                    if(lcddev.id==0x8000)lcddev.id=0x5510;//NT35510读回的ID是8000H,为方便区分,我们强制设置为5510
                }
            }
        }
    }
	 //printf(" LCD ID:%x\r\n",lcddev.id); //打印LCD ID
    if(lcddev.id==0X9341)	//9341初始化
    {
        LCD5510_WR_REG(0xCF);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC1);
        LCD5510_WR_DATA(0X30);
        LCD5510_WR_REG(0xED);
        LCD5510_WR_DATA(0x64);
        LCD5510_WR_DATA(0x03);
        LCD5510_WR_DATA(0X12);
        LCD5510_WR_DATA(0X81);
        LCD5510_WR_REG(0xE8);
        LCD5510_WR_DATA(0x85);
        LCD5510_WR_DATA(0x10);
        LCD5510_WR_DATA(0x7A);
        LCD5510_WR_REG(0xCB);
        LCD5510_WR_DATA(0x39);
        LCD5510_WR_DATA(0x2C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x34);
        LCD5510_WR_DATA(0x02);
        LCD5510_WR_REG(0xF7);
        LCD5510_WR_DATA(0x20);
        LCD5510_WR_REG(0xEA);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_REG(0xC0);    //Power control
        LCD5510_WR_DATA(0x1B);   //VRH[5:0]
        LCD5510_WR_REG(0xC1);    //Power control
        LCD5510_WR_DATA(0x01);   //SAP[2:0];BT[3:0]
        LCD5510_WR_REG(0xC5);    //VCM control
        LCD5510_WR_DATA(0x30); 	 //3F
        LCD5510_WR_DATA(0x30); 	 //3C
        LCD5510_WR_REG(0xC7);    //VCM control2
        LCD5510_WR_DATA(0XB7);
        LCD5510_WR_REG(0x36);    // Memory Access Control
        LCD5510_WR_DATA(0x48);
        LCD5510_WR_REG(0x3A);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_REG(0xB1);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x1A);
        LCD5510_WR_REG(0xB6);    // Display Function Control
        LCD5510_WR_DATA(0x0A);
        LCD5510_WR_DATA(0xA2);
        LCD5510_WR_REG(0xF2);    // 3Gamma Function Disable
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_REG(0x26);    //Gamma curve selected
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_REG(0xE0);    //Set Gamma
        LCD5510_WR_DATA(0x0F);
        LCD5510_WR_DATA(0x2A);
        LCD5510_WR_DATA(0x28);
        LCD5510_WR_DATA(0x08);
        LCD5510_WR_DATA(0x0E);
        LCD5510_WR_DATA(0x08);
        LCD5510_WR_DATA(0x54);
        LCD5510_WR_DATA(0XA9);
        LCD5510_WR_DATA(0x43);
        LCD5510_WR_DATA(0x0A);
        LCD5510_WR_DATA(0x0F);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_REG(0XE1);    //Set Gamma
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x15);
        LCD5510_WR_DATA(0x17);
        LCD5510_WR_DATA(0x07);
        LCD5510_WR_DATA(0x11);
        LCD5510_WR_DATA(0x06);
        LCD5510_WR_DATA(0x2B);
        LCD5510_WR_DATA(0x56);
        LCD5510_WR_DATA(0x3C);
        LCD5510_WR_DATA(0x05);
        LCD5510_WR_DATA(0x10);
        LCD5510_WR_DATA(0x0F);
        LCD5510_WR_DATA(0x3F);
        LCD5510_WR_DATA(0x3F);
        LCD5510_WR_DATA(0x0F);
        LCD5510_WR_REG(0x2B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_DATA(0x3f);
        LCD5510_WR_REG(0x2A);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xef);
        LCD5510_WR_REG(0x11); //Exit Sleep
        delay_ms(120);
        LCD5510_WR_REG(0x29); //display on
    } else if(lcddev.id==0x6804) //6804初始化
    {
        LCD5510_WR_REG(0X11);
        delay_ms(20);
        LCD5510_WR_REG(0XD0);//VCI1  VCL  VGH  VGL DDVDH VREG1OUT power amplitude setting
        LCD5510_WR_DATA(0X07);
        LCD5510_WR_DATA(0X42);
        LCD5510_WR_DATA(0X1D);
        LCD5510_WR_REG(0XD1);//VCOMH VCOM_AC amplitude setting
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X1a);
        LCD5510_WR_DATA(0X09);
        LCD5510_WR_REG(0XD2);//Operational Amplifier Circuit Constant Current Adjust , charge pump frequency setting
        LCD5510_WR_DATA(0X01);
        LCD5510_WR_DATA(0X22);
        LCD5510_WR_REG(0XC0);//REV SM GS
        LCD5510_WR_DATA(0X10);
        LCD5510_WR_DATA(0X3B);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X02);
        LCD5510_WR_DATA(0X11);

        LCD5510_WR_REG(0XC5);// Frame rate setting = 72HZ  when setting 0x03
        LCD5510_WR_DATA(0X03);

        LCD5510_WR_REG(0XC8);//Gamma setting
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X25);
        LCD5510_WR_DATA(0X21);
        LCD5510_WR_DATA(0X05);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X0a);
        LCD5510_WR_DATA(0X65);
        LCD5510_WR_DATA(0X25);
        LCD5510_WR_DATA(0X77);
        LCD5510_WR_DATA(0X50);
        LCD5510_WR_DATA(0X0f);
        LCD5510_WR_DATA(0X00);

        LCD5510_WR_REG(0XF8);
        LCD5510_WR_DATA(0X01);

        LCD5510_WR_REG(0XFE);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X02);

        LCD5510_WR_REG(0X20);//Exit invert mode

        LCD5510_WR_REG(0X36);
        LCD5510_WR_DATA(0X08);//原来是a

        LCD5510_WR_REG(0X3A);
        LCD5510_WR_DATA(0X55);//16位模式
        LCD5510_WR_REG(0X2B);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X01);
        LCD5510_WR_DATA(0X3F);

        LCD5510_WR_REG(0X2A);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X00);
        LCD5510_WR_DATA(0X01);
        LCD5510_WR_DATA(0XDF);
        delay_ms(120);
        LCD5510_WR_REG(0X29);
    } else if(lcddev.id==0x5310)
    {
        LCD5510_WR_REG(0xED);
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_DATA(0xFE);

        LCD5510_WR_REG(0xEE);
        LCD5510_WR_DATA(0xDE);
        LCD5510_WR_DATA(0x21);

        LCD5510_WR_REG(0xF1);
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_REG(0xDF);
        LCD5510_WR_DATA(0x10);

        //VCOMvoltage//
        LCD5510_WR_REG(0xC4);
        LCD5510_WR_DATA(0x8F);	  //5f

        LCD5510_WR_REG(0xC6);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xE2);
        LCD5510_WR_DATA(0xE2);
        LCD5510_WR_DATA(0xE2);
        LCD5510_WR_REG(0xBF);
        LCD5510_WR_DATA(0xAA);

        LCD5510_WR_REG(0xB0);
        LCD5510_WR_DATA(0x0D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x0D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x11);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x19);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x21);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x2D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x5D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x5D);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB1);
        LCD5510_WR_DATA(0x80);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x8B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x96);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x02);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x03);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB4);
        LCD5510_WR_DATA(0x8B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x96);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA1);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB5);
        LCD5510_WR_DATA(0x02);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x03);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x04);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB6);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB7);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3F);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x5E);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x64);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x8C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xAC);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xDC);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x70);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x90);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xEB);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xDC);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xB8);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xBA);
        LCD5510_WR_DATA(0x24);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC1);
        LCD5510_WR_DATA(0x20);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x54);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xFF);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC2);
        LCD5510_WR_DATA(0x0A);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x04);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC3);
        LCD5510_WR_DATA(0x3C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3A);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x39);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x37);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x36);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x32);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x2F);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x2C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x29);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x26);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x24);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x24);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x23);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x36);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x32);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x2F);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x2C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x29);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x26);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x24);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x24);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x23);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC4);
        LCD5510_WR_DATA(0x62);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x05);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x84);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF0);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x18);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA4);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x18);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x50);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x0C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x17);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x95);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xE6);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC5);
        LCD5510_WR_DATA(0x32);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x65);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x76);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x88);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC6);
        LCD5510_WR_DATA(0x20);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x17);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC7);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC8);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xC9);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE0);
        LCD5510_WR_DATA(0x16);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x1C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x21);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x36);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x46);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x52);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x64);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x7A);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x8B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA8);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xB9);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC4);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xCA);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD9);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xE0);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE1);
        LCD5510_WR_DATA(0x16);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x1C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x22);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x36);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x45);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x52);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x64);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x7A);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x8B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA8);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xB9);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC4);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xCA);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD8);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xE0);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE2);
        LCD5510_WR_DATA(0x05);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x0B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x1B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x34);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x4F);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x61);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x79);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x88);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x97);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA6);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xB7);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC7);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD1);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD6);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xDD);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_REG(0xE3);
        LCD5510_WR_DATA(0x05);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x1C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x33);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x50);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x62);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x78);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x88);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x97);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA6);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xB7);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC7);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD1);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD5);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xDD);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE4);
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x01);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x02);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x2A);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x4B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x5D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x74);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x84);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x93);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xB3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xBE);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC4);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xCD);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xDD);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_REG(0xE5);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x02);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x29);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x3C);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x4B);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x5D);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x74);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x84);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x93);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xA2);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xB3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xBE);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xC4);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xCD);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xD3);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xDC);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xF3);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE6);
        LCD5510_WR_DATA(0x11);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x34);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x56);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x76);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x77);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x66);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x88);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xBB);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x66);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x45);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x43);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE7);
        LCD5510_WR_DATA(0x32);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x76);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x66);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x67);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x67);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x87);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xBB);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x77);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x56);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x23);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x33);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x45);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE8);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x87);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x88);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x77);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x66);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x88);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xAA);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0xBB);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x99);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x66);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x44);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x55);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xE9);
        LCD5510_WR_DATA(0xAA);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0x00);
        LCD5510_WR_DATA(0xAA);

        LCD5510_WR_REG(0xCF);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xF0);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x50);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xF3);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0xF9);
        LCD5510_WR_DATA(0x06);
        LCD5510_WR_DATA(0x10);
        LCD5510_WR_DATA(0x29);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0x3A);
        LCD5510_WR_DATA(0x55);	//66

        LCD5510_WR_REG(0x11);
        delay_ms(100);
        LCD5510_WR_REG(0x29);
        LCD5510_WR_REG(0x35);
        LCD5510_WR_DATA(0x00);

        LCD5510_WR_REG(0x51);
        LCD5510_WR_DATA(0xFF);
        LCD5510_WR_REG(0x53);
        LCD5510_WR_DATA(0x2C);
        LCD5510_WR_REG(0x55);
        LCD5510_WR_DATA(0x82);
        LCD5510_WR_REG(0x2c);
    } else if(lcddev.id==0x5510)
    {
        LCD5510_WriteReg(0xF000,0x55);
        LCD5510_WriteReg(0xF001,0xAA);
        LCD5510_WriteReg(0xF002,0x52);
        LCD5510_WriteReg(0xF003,0x08);
        LCD5510_WriteReg(0xF004,0x01);
        //AVDD Set AVDD 5.2V
        LCD5510_WriteReg(0xB000,0x0D);
        LCD5510_WriteReg(0xB001,0x0D);
        LCD5510_WriteReg(0xB002,0x0D);
        //AVDD ratio
        LCD5510_WriteReg(0xB600,0x34);
        LCD5510_WriteReg(0xB601,0x34);
        LCD5510_WriteReg(0xB602,0x34);
        //AVEE -5.2V
        LCD5510_WriteReg(0xB100,0x0D);
        LCD5510_WriteReg(0xB101,0x0D);
        LCD5510_WriteReg(0xB102,0x0D);
        //AVEE ratio
        LCD5510_WriteReg(0xB700,0x34);
        LCD5510_WriteReg(0xB701,0x34);
        LCD5510_WriteReg(0xB702,0x34);
        //VCL -2.5V
        LCD5510_WriteReg(0xB200,0x00);
        LCD5510_WriteReg(0xB201,0x00);
        LCD5510_WriteReg(0xB202,0x00);
        //VCL ratio
        LCD5510_WriteReg(0xB800,0x24);
        LCD5510_WriteReg(0xB801,0x24);
        LCD5510_WriteReg(0xB802,0x24);
        //VGH 15V (Free pump)
        LCD5510_WriteReg(0xBF00,0x01);
        LCD5510_WriteReg(0xB300,0x0F);
        LCD5510_WriteReg(0xB301,0x0F);
        LCD5510_WriteReg(0xB302,0x0F);
        //VGH ratio
        LCD5510_WriteReg(0xB900,0x34);
        LCD5510_WriteReg(0xB901,0x34);
        LCD5510_WriteReg(0xB902,0x34);
        //VGL_REG -10V
        LCD5510_WriteReg(0xB500,0x08);
        LCD5510_WriteReg(0xB501,0x08);
        LCD5510_WriteReg(0xB502,0x08);
        LCD5510_WriteReg(0xC200,0x03);
        //VGLX ratio
        LCD5510_WriteReg(0xBA00,0x24);
        LCD5510_WriteReg(0xBA01,0x24);
        LCD5510_WriteReg(0xBA02,0x24);
        //VGMP/VGSP 4.5V/0V
        LCD5510_WriteReg(0xBC00,0x00);
        LCD5510_WriteReg(0xBC01,0x78);
        LCD5510_WriteReg(0xBC02,0x00);
        //VGMN/VGSN -4.5V/0V
        LCD5510_WriteReg(0xBD00,0x00);
        LCD5510_WriteReg(0xBD01,0x78);
        LCD5510_WriteReg(0xBD02,0x00);
        //VCOM
        LCD5510_WriteReg(0xBE00,0x00);
        LCD5510_WriteReg(0xBE01,0x64);
        //Gamma Setting
        LCD5510_WriteReg(0xD100,0x00);
        LCD5510_WriteReg(0xD101,0x33);
        LCD5510_WriteReg(0xD102,0x00);
        LCD5510_WriteReg(0xD103,0x34);
        LCD5510_WriteReg(0xD104,0x00);
        LCD5510_WriteReg(0xD105,0x3A);
        LCD5510_WriteReg(0xD106,0x00);
        LCD5510_WriteReg(0xD107,0x4A);
        LCD5510_WriteReg(0xD108,0x00);
        LCD5510_WriteReg(0xD109,0x5C);
        LCD5510_WriteReg(0xD10A,0x00);
        LCD5510_WriteReg(0xD10B,0x81);
        LCD5510_WriteReg(0xD10C,0x00);
        LCD5510_WriteReg(0xD10D,0xA6);
        LCD5510_WriteReg(0xD10E,0x00);
        LCD5510_WriteReg(0xD10F,0xE5);
        LCD5510_WriteReg(0xD110,0x01);
        LCD5510_WriteReg(0xD111,0x13);
        LCD5510_WriteReg(0xD112,0x01);
        LCD5510_WriteReg(0xD113,0x54);
        LCD5510_WriteReg(0xD114,0x01);
        LCD5510_WriteReg(0xD115,0x82);
        LCD5510_WriteReg(0xD116,0x01);
        LCD5510_WriteReg(0xD117,0xCA);
        LCD5510_WriteReg(0xD118,0x02);
        LCD5510_WriteReg(0xD119,0x00);
        LCD5510_WriteReg(0xD11A,0x02);
        LCD5510_WriteReg(0xD11B,0x01);
        LCD5510_WriteReg(0xD11C,0x02);
        LCD5510_WriteReg(0xD11D,0x34);
        LCD5510_WriteReg(0xD11E,0x02);
        LCD5510_WriteReg(0xD11F,0x67);
        LCD5510_WriteReg(0xD120,0x02);
        LCD5510_WriteReg(0xD121,0x84);
        LCD5510_WriteReg(0xD122,0x02);
        LCD5510_WriteReg(0xD123,0xA4);
        LCD5510_WriteReg(0xD124,0x02);
        LCD5510_WriteReg(0xD125,0xB7);
        LCD5510_WriteReg(0xD126,0x02);
        LCD5510_WriteReg(0xD127,0xCF);
        LCD5510_WriteReg(0xD128,0x02);
        LCD5510_WriteReg(0xD129,0xDE);
        LCD5510_WriteReg(0xD12A,0x02);
        LCD5510_WriteReg(0xD12B,0xF2);
        LCD5510_WriteReg(0xD12C,0x02);
        LCD5510_WriteReg(0xD12D,0xFE);
        LCD5510_WriteReg(0xD12E,0x03);
        LCD5510_WriteReg(0xD12F,0x10);
        LCD5510_WriteReg(0xD130,0x03);
        LCD5510_WriteReg(0xD131,0x33);
        LCD5510_WriteReg(0xD132,0x03);
        LCD5510_WriteReg(0xD133,0x6D);
        LCD5510_WriteReg(0xD200,0x00);
        LCD5510_WriteReg(0xD201,0x33);
        LCD5510_WriteReg(0xD202,0x00);
        LCD5510_WriteReg(0xD203,0x34);
        LCD5510_WriteReg(0xD204,0x00);
        LCD5510_WriteReg(0xD205,0x3A);
        LCD5510_WriteReg(0xD206,0x00);
        LCD5510_WriteReg(0xD207,0x4A);
        LCD5510_WriteReg(0xD208,0x00);
        LCD5510_WriteReg(0xD209,0x5C);
        LCD5510_WriteReg(0xD20A,0x00);

        LCD5510_WriteReg(0xD20B,0x81);
        LCD5510_WriteReg(0xD20C,0x00);
        LCD5510_WriteReg(0xD20D,0xA6);
        LCD5510_WriteReg(0xD20E,0x00);
        LCD5510_WriteReg(0xD20F,0xE5);
        LCD5510_WriteReg(0xD210,0x01);
        LCD5510_WriteReg(0xD211,0x13);
        LCD5510_WriteReg(0xD212,0x01);
        LCD5510_WriteReg(0xD213,0x54);
        LCD5510_WriteReg(0xD214,0x01);
        LCD5510_WriteReg(0xD215,0x82);
        LCD5510_WriteReg(0xD216,0x01);
        LCD5510_WriteReg(0xD217,0xCA);
        LCD5510_WriteReg(0xD218,0x02);
        LCD5510_WriteReg(0xD219,0x00);
        LCD5510_WriteReg(0xD21A,0x02);
        LCD5510_WriteReg(0xD21B,0x01);
        LCD5510_WriteReg(0xD21C,0x02);
        LCD5510_WriteReg(0xD21D,0x34);
        LCD5510_WriteReg(0xD21E,0x02);
        LCD5510_WriteReg(0xD21F,0x67);
        LCD5510_WriteReg(0xD220,0x02);
        LCD5510_WriteReg(0xD221,0x84);
        LCD5510_WriteReg(0xD222,0x02);
        LCD5510_WriteReg(0xD223,0xA4);
        LCD5510_WriteReg(0xD224,0x02);
        LCD5510_WriteReg(0xD225,0xB7);
        LCD5510_WriteReg(0xD226,0x02);
        LCD5510_WriteReg(0xD227,0xCF);
        LCD5510_WriteReg(0xD228,0x02);
        LCD5510_WriteReg(0xD229,0xDE);
        LCD5510_WriteReg(0xD22A,0x02);
        LCD5510_WriteReg(0xD22B,0xF2);
        LCD5510_WriteReg(0xD22C,0x02);
        LCD5510_WriteReg(0xD22D,0xFE);
        LCD5510_WriteReg(0xD22E,0x03);
        LCD5510_WriteReg(0xD22F,0x10);
        LCD5510_WriteReg(0xD230,0x03);
        LCD5510_WriteReg(0xD231,0x33);
        LCD5510_WriteReg(0xD232,0x03);
        LCD5510_WriteReg(0xD233,0x6D);
        LCD5510_WriteReg(0xD300,0x00);
        LCD5510_WriteReg(0xD301,0x33);
        LCD5510_WriteReg(0xD302,0x00);
        LCD5510_WriteReg(0xD303,0x34);
        LCD5510_WriteReg(0xD304,0x00);
        LCD5510_WriteReg(0xD305,0x3A);
        LCD5510_WriteReg(0xD306,0x00);
        LCD5510_WriteReg(0xD307,0x4A);
        LCD5510_WriteReg(0xD308,0x00);
        LCD5510_WriteReg(0xD309,0x5C);
        LCD5510_WriteReg(0xD30A,0x00);

        LCD5510_WriteReg(0xD30B,0x81);
        LCD5510_WriteReg(0xD30C,0x00);
        LCD5510_WriteReg(0xD30D,0xA6);
        LCD5510_WriteReg(0xD30E,0x00);
        LCD5510_WriteReg(0xD30F,0xE5);
        LCD5510_WriteReg(0xD310,0x01);
        LCD5510_WriteReg(0xD311,0x13);
        LCD5510_WriteReg(0xD312,0x01);
        LCD5510_WriteReg(0xD313,0x54);
        LCD5510_WriteReg(0xD314,0x01);
        LCD5510_WriteReg(0xD315,0x82);
        LCD5510_WriteReg(0xD316,0x01);
        LCD5510_WriteReg(0xD317,0xCA);
        LCD5510_WriteReg(0xD318,0x02);
        LCD5510_WriteReg(0xD319,0x00);
        LCD5510_WriteReg(0xD31A,0x02);
        LCD5510_WriteReg(0xD31B,0x01);
        LCD5510_WriteReg(0xD31C,0x02);
        LCD5510_WriteReg(0xD31D,0x34);
        LCD5510_WriteReg(0xD31E,0x02);
        LCD5510_WriteReg(0xD31F,0x67);
        LCD5510_WriteReg(0xD320,0x02);
        LCD5510_WriteReg(0xD321,0x84);
        LCD5510_WriteReg(0xD322,0x02);
        LCD5510_WriteReg(0xD323,0xA4);
        LCD5510_WriteReg(0xD324,0x02);
        LCD5510_WriteReg(0xD325,0xB7);
        LCD5510_WriteReg(0xD326,0x02);
        LCD5510_WriteReg(0xD327,0xCF);
        LCD5510_WriteReg(0xD328,0x02);
        LCD5510_WriteReg(0xD329,0xDE);
        LCD5510_WriteReg(0xD32A,0x02);
        LCD5510_WriteReg(0xD32B,0xF2);
        LCD5510_WriteReg(0xD32C,0x02);
        LCD5510_WriteReg(0xD32D,0xFE);
        LCD5510_WriteReg(0xD32E,0x03);
        LCD5510_WriteReg(0xD32F,0x10);
        LCD5510_WriteReg(0xD330,0x03);
        LCD5510_WriteReg(0xD331,0x33);
        LCD5510_WriteReg(0xD332,0x03);
        LCD5510_WriteReg(0xD333,0x6D);
        LCD5510_WriteReg(0xD400,0x00);
        LCD5510_WriteReg(0xD401,0x33);
        LCD5510_WriteReg(0xD402,0x00);
        LCD5510_WriteReg(0xD403,0x34);
        LCD5510_WriteReg(0xD404,0x00);
        LCD5510_WriteReg(0xD405,0x3A);
        LCD5510_WriteReg(0xD406,0x00);
        LCD5510_WriteReg(0xD407,0x4A);
        LCD5510_WriteReg(0xD408,0x00);
        LCD5510_WriteReg(0xD409,0x5C);
        LCD5510_WriteReg(0xD40A,0x00);
        LCD5510_WriteReg(0xD40B,0x81);

        LCD5510_WriteReg(0xD40C,0x00);
        LCD5510_WriteReg(0xD40D,0xA6);
        LCD5510_WriteReg(0xD40E,0x00);
        LCD5510_WriteReg(0xD40F,0xE5);
        LCD5510_WriteReg(0xD410,0x01);
        LCD5510_WriteReg(0xD411,0x13);
        LCD5510_WriteReg(0xD412,0x01);
        LCD5510_WriteReg(0xD413,0x54);
        LCD5510_WriteReg(0xD414,0x01);
        LCD5510_WriteReg(0xD415,0x82);
        LCD5510_WriteReg(0xD416,0x01);
        LCD5510_WriteReg(0xD417,0xCA);
        LCD5510_WriteReg(0xD418,0x02);
        LCD5510_WriteReg(0xD419,0x00);
        LCD5510_WriteReg(0xD41A,0x02);
        LCD5510_WriteReg(0xD41B,0x01);
        LCD5510_WriteReg(0xD41C,0x02);
        LCD5510_WriteReg(0xD41D,0x34);
        LCD5510_WriteReg(0xD41E,0x02);
        LCD5510_WriteReg(0xD41F,0x67);
        LCD5510_WriteReg(0xD420,0x02);
        LCD5510_WriteReg(0xD421,0x84);
        LCD5510_WriteReg(0xD422,0x02);
        LCD5510_WriteReg(0xD423,0xA4);
        LCD5510_WriteReg(0xD424,0x02);
        LCD5510_WriteReg(0xD425,0xB7);
        LCD5510_WriteReg(0xD426,0x02);
        LCD5510_WriteReg(0xD427,0xCF);
        LCD5510_WriteReg(0xD428,0x02);
        LCD5510_WriteReg(0xD429,0xDE);
        LCD5510_WriteReg(0xD42A,0x02);
        LCD5510_WriteReg(0xD42B,0xF2);
        LCD5510_WriteReg(0xD42C,0x02);
        LCD5510_WriteReg(0xD42D,0xFE);
        LCD5510_WriteReg(0xD42E,0x03);
        LCD5510_WriteReg(0xD42F,0x10);
        LCD5510_WriteReg(0xD430,0x03);
        LCD5510_WriteReg(0xD431,0x33);
        LCD5510_WriteReg(0xD432,0x03);
        LCD5510_WriteReg(0xD433,0x6D);
        LCD5510_WriteReg(0xD500,0x00);
        LCD5510_WriteReg(0xD501,0x33);
        LCD5510_WriteReg(0xD502,0x00);
        LCD5510_WriteReg(0xD503,0x34);
        LCD5510_WriteReg(0xD504,0x00);
        LCD5510_WriteReg(0xD505,0x3A);
        LCD5510_WriteReg(0xD506,0x00);
        LCD5510_WriteReg(0xD507,0x4A);
        LCD5510_WriteReg(0xD508,0x00);
        LCD5510_WriteReg(0xD509,0x5C);
        LCD5510_WriteReg(0xD50A,0x00);
        LCD5510_WriteReg(0xD50B,0x81);

        LCD5510_WriteReg(0xD50C,0x00);
        LCD5510_WriteReg(0xD50D,0xA6);
        LCD5510_WriteReg(0xD50E,0x00);
        LCD5510_WriteReg(0xD50F,0xE5);
        LCD5510_WriteReg(0xD510,0x01);
        LCD5510_WriteReg(0xD511,0x13);
        LCD5510_WriteReg(0xD512,0x01);
        LCD5510_WriteReg(0xD513,0x54);
        LCD5510_WriteReg(0xD514,0x01);
        LCD5510_WriteReg(0xD515,0x82);
        LCD5510_WriteReg(0xD516,0x01);
        LCD5510_WriteReg(0xD517,0xCA);
        LCD5510_WriteReg(0xD518,0x02);
        LCD5510_WriteReg(0xD519,0x00);
        LCD5510_WriteReg(0xD51A,0x02);
        LCD5510_WriteReg(0xD51B,0x01);
        LCD5510_WriteReg(0xD51C,0x02);
        LCD5510_WriteReg(0xD51D,0x34);
        LCD5510_WriteReg(0xD51E,0x02);
        LCD5510_WriteReg(0xD51F,0x67);
        LCD5510_WriteReg(0xD520,0x02);
        LCD5510_WriteReg(0xD521,0x84);
        LCD5510_WriteReg(0xD522,0x02);
        LCD5510_WriteReg(0xD523,0xA4);
        LCD5510_WriteReg(0xD524,0x02);
        LCD5510_WriteReg(0xD525,0xB7);
        LCD5510_WriteReg(0xD526,0x02);
        LCD5510_WriteReg(0xD527,0xCF);
        LCD5510_WriteReg(0xD528,0x02);
        LCD5510_WriteReg(0xD529,0xDE);
        LCD5510_WriteReg(0xD52A,0x02);
        LCD5510_WriteReg(0xD52B,0xF2);
        LCD5510_WriteReg(0xD52C,0x02);
        LCD5510_WriteReg(0xD52D,0xFE);
        LCD5510_WriteReg(0xD52E,0x03);
        LCD5510_WriteReg(0xD52F,0x10);
        LCD5510_WriteReg(0xD530,0x03);
        LCD5510_WriteReg(0xD531,0x33);
        LCD5510_WriteReg(0xD532,0x03);
        LCD5510_WriteReg(0xD533,0x6D);
        LCD5510_WriteReg(0xD600,0x00);
        LCD5510_WriteReg(0xD601,0x33);
        LCD5510_WriteReg(0xD602,0x00);
        LCD5510_WriteReg(0xD603,0x34);
        LCD5510_WriteReg(0xD604,0x00);
        LCD5510_WriteReg(0xD605,0x3A);
        LCD5510_WriteReg(0xD606,0x00);
        LCD5510_WriteReg(0xD607,0x4A);
        LCD5510_WriteReg(0xD608,0x00);
        LCD5510_WriteReg(0xD609,0x5C);
        LCD5510_WriteReg(0xD60A,0x00);
        LCD5510_WriteReg(0xD60B,0x81);

        LCD5510_WriteReg(0xD60C,0x00);
        LCD5510_WriteReg(0xD60D,0xA6);
        LCD5510_WriteReg(0xD60E,0x00);
        LCD5510_WriteReg(0xD60F,0xE5);
        LCD5510_WriteReg(0xD610,0x01);
        LCD5510_WriteReg(0xD611,0x13);
        LCD5510_WriteReg(0xD612,0x01);
        LCD5510_WriteReg(0xD613,0x54);
        LCD5510_WriteReg(0xD614,0x01);
        LCD5510_WriteReg(0xD615,0x82);
        LCD5510_WriteReg(0xD616,0x01);
        LCD5510_WriteReg(0xD617,0xCA);
        LCD5510_WriteReg(0xD618,0x02);
        LCD5510_WriteReg(0xD619,0x00);
        LCD5510_WriteReg(0xD61A,0x02);
        LCD5510_WriteReg(0xD61B,0x01);
        LCD5510_WriteReg(0xD61C,0x02);
        LCD5510_WriteReg(0xD61D,0x34);
        LCD5510_WriteReg(0xD61E,0x02);
        LCD5510_WriteReg(0xD61F,0x67);
        LCD5510_WriteReg(0xD620,0x02);
        LCD5510_WriteReg(0xD621,0x84);
        LCD5510_WriteReg(0xD622,0x02);
        LCD5510_WriteReg(0xD623,0xA4);
        LCD5510_WriteReg(0xD624,0x02);
        LCD5510_WriteReg(0xD625,0xB7);
        LCD5510_WriteReg(0xD626,0x02);
        LCD5510_WriteReg(0xD627,0xCF);
        LCD5510_WriteReg(0xD628,0x02);
        LCD5510_WriteReg(0xD629,0xDE);
        LCD5510_WriteReg(0xD62A,0x02);
        LCD5510_WriteReg(0xD62B,0xF2);
        LCD5510_WriteReg(0xD62C,0x02);
        LCD5510_WriteReg(0xD62D,0xFE);
        LCD5510_WriteReg(0xD62E,0x03);
        LCD5510_WriteReg(0xD62F,0x10);
        LCD5510_WriteReg(0xD630,0x03);
        LCD5510_WriteReg(0xD631,0x33);
        LCD5510_WriteReg(0xD632,0x03);
        LCD5510_WriteReg(0xD633,0x6D);
        //LV2 Page 0 enable
        LCD5510_WriteReg(0xF000,0x55);
        LCD5510_WriteReg(0xF001,0xAA);
        LCD5510_WriteReg(0xF002,0x52);
        LCD5510_WriteReg(0xF003,0x08);
        LCD5510_WriteReg(0xF004,0x00);
        //Display control
        LCD5510_WriteReg(0xB100, 0xCC);
        LCD5510_WriteReg(0xB101, 0x00);
        //Source hold time
        LCD5510_WriteReg(0xB600,0x05);
        //Gate EQ control
        LCD5510_WriteReg(0xB700,0x70);
        LCD5510_WriteReg(0xB701,0x70);
        //Source EQ control (Mode 2)
        LCD5510_WriteReg(0xB800,0x01);
        LCD5510_WriteReg(0xB801,0x03);
        LCD5510_WriteReg(0xB802,0x03);
        LCD5510_WriteReg(0xB803,0x03);
        //Inversion mode (2-dot)
        LCD5510_WriteReg(0xBC00,0x02);
        LCD5510_WriteReg(0xBC01,0x00);
        LCD5510_WriteReg(0xBC02,0x00);
        //Timing control 4H w/ 4-delay
        LCD5510_WriteReg(0xC900,0xD0);
        LCD5510_WriteReg(0xC901,0x02);
        LCD5510_WriteReg(0xC902,0x50);
        LCD5510_WriteReg(0xC903,0x50);
        LCD5510_WriteReg(0xC904,0x50);
        LCD5510_WriteReg(0x3500,0x00);
        LCD5510_WriteReg(0x3A00,0x55);  //16-bit/pixel
        LCD5510_WR_REG(0x1100);
        delay_us(120);
        LCD5510_WR_REG(0x2900);
    } else if(lcddev.id==0x9325)//9325
    {
        LCD5510_WriteReg(0x00E5,0x78F0);
        LCD5510_WriteReg(0x0001,0x0100);
        LCD5510_WriteReg(0x0002,0x0700);
        LCD5510_WriteReg(0x0003,0x1030);
        LCD5510_WriteReg(0x0004,0x0000);
        LCD5510_WriteReg(0x0008,0x0202);
        LCD5510_WriteReg(0x0009,0x0000);
        LCD5510_WriteReg(0x000A,0x0000);
        LCD5510_WriteReg(0x000C,0x0000);
        LCD5510_WriteReg(0x000D,0x0000);
        LCD5510_WriteReg(0x000F,0x0000);
        //power on sequence VGHVGL
        LCD5510_WriteReg(0x0010,0x0000);
        LCD5510_WriteReg(0x0011,0x0007);
        LCD5510_WriteReg(0x0012,0x0000);
        LCD5510_WriteReg(0x0013,0x0000);
        LCD5510_WriteReg(0x0007,0x0000);
        //vgh
        LCD5510_WriteReg(0x0010,0x1690);
        LCD5510_WriteReg(0x0011,0x0227);
        //delayms(100);
        //vregiout
        LCD5510_WriteReg(0x0012,0x009D); //0x001b
        //delayms(100);
        //vom amplitude
        LCD5510_WriteReg(0x0013,0x1900);
        //delayms(100);
        //vom H
        LCD5510_WriteReg(0x0029,0x0025);
        LCD5510_WriteReg(0x002B,0x000D);
        //gamma
        LCD5510_WriteReg(0x0030,0x0007);
        LCD5510_WriteReg(0x0031,0x0303);
        LCD5510_WriteReg(0x0032,0x0003);// 0006
        LCD5510_WriteReg(0x0035,0x0206);
        LCD5510_WriteReg(0x0036,0x0008);
        LCD5510_WriteReg(0x0037,0x0406);
        LCD5510_WriteReg(0x0038,0x0304);//0200
        LCD5510_WriteReg(0x0039,0x0007);
        LCD5510_WriteReg(0x003C,0x0602);// 0504
        LCD5510_WriteReg(0x003D,0x0008);
        //ram
        LCD5510_WriteReg(0x0050,0x0000);
        LCD5510_WriteReg(0x0051,0x00EF);
        LCD5510_WriteReg(0x0052,0x0000);
        LCD5510_WriteReg(0x0053,0x013F);
        LCD5510_WriteReg(0x0060,0xA700);
        LCD5510_WriteReg(0x0061,0x0001);
        LCD5510_WriteReg(0x006A,0x0000);
        //
        LCD5510_WriteReg(0x0080,0x0000);
        LCD5510_WriteReg(0x0081,0x0000);
        LCD5510_WriteReg(0x0082,0x0000);
        LCD5510_WriteReg(0x0083,0x0000);
        LCD5510_WriteReg(0x0084,0x0000);
        LCD5510_WriteReg(0x0085,0x0000);
        //
        LCD5510_WriteReg(0x0090,0x0010);
        LCD5510_WriteReg(0x0092,0x0600);

        LCD5510_WriteReg(0x0007,0x0133);
        LCD5510_WriteReg(0x00,0x0022);//
    } else if(lcddev.id==0x9328)//ILI9328   OK
    {
        LCD5510_WriteReg(0x00EC,0x108F);// internal timeing
        LCD5510_WriteReg(0x00EF,0x1234);// ADD
        //LCD5510_WriteReg(0x00e7,0x0010);
        //LCD5510_WriteReg(0x0000,0x0001);//开启内部时钟
        LCD5510_WriteReg(0x0001,0x0100);
        LCD5510_WriteReg(0x0002,0x0700);//电源开启
        //LCD5510_WriteReg(0x0003,(1<<3)|(1<<4) ); 	//65K  RGB
        //DRIVE TABLE(寄存器 03H)
        //BIT3=AM BIT4:5=ID0:1
        //AM ID0 ID1   FUNCATION
        // 0  0   0	   R->L D->U
        // 1  0   0	   D->U	R->L
        // 0  1   0	   L->R D->U
        // 1  1   0    D->U	L->R
        // 0  0   1	   R->L U->D
        // 1  0   1    U->D	R->L
        // 0  1   1    L->R U->D 正常就用这个.
        // 1  1   1	   U->D	L->R
        LCD5510_WriteReg(0x0003,(1<<12)|(3<<4)|(0<<3) );//65K
        LCD5510_WriteReg(0x0004,0x0000);
        LCD5510_WriteReg(0x0008,0x0202);
        LCD5510_WriteReg(0x0009,0x0000);
        LCD5510_WriteReg(0x000a,0x0000);//display setting
        LCD5510_WriteReg(0x000c,0x0001);//display setting
        LCD5510_WriteReg(0x000d,0x0000);//0f3c
        LCD5510_WriteReg(0x000f,0x0000);
        //电源配置
        LCD5510_WriteReg(0x0010,0x0000);
        LCD5510_WriteReg(0x0011,0x0007);
        LCD5510_WriteReg(0x0012,0x0000);
        LCD5510_WriteReg(0x0013,0x0000);
        LCD5510_WriteReg(0x0007,0x0001);
        delay_ms(50);
        LCD5510_WriteReg(0x0010,0x1490);
        LCD5510_WriteReg(0x0011,0x0227);
        delay_ms(50);
        LCD5510_WriteReg(0x0012,0x008A);
        delay_ms(50);
        LCD5510_WriteReg(0x0013,0x1a00);
        LCD5510_WriteReg(0x0029,0x0006);
        LCD5510_WriteReg(0x002b,0x000d);
        delay_ms(50);
        LCD5510_WriteReg(0x0020,0x0000);
        LCD5510_WriteReg(0x0021,0x0000);
        delay_ms(50);
        //伽马校正
        LCD5510_WriteReg(0x0030,0x0000);
        LCD5510_WriteReg(0x0031,0x0604);
        LCD5510_WriteReg(0x0032,0x0305);
        LCD5510_WriteReg(0x0035,0x0000);
        LCD5510_WriteReg(0x0036,0x0C09);
        LCD5510_WriteReg(0x0037,0x0204);
        LCD5510_WriteReg(0x0038,0x0301);
        LCD5510_WriteReg(0x0039,0x0707);
        LCD5510_WriteReg(0x003c,0x0000);
        LCD5510_WriteReg(0x003d,0x0a0a);
        delay_ms(50);
        LCD5510_WriteReg(0x0050,0x0000); //水平GRAM起始位置
        LCD5510_WriteReg(0x0051,0x00ef); //水平GRAM终止位置
        LCD5510_WriteReg(0x0052,0x0000); //垂直GRAM起始位置
        LCD5510_WriteReg(0x0053,0x013f); //垂直GRAM终止位置

        LCD5510_WriteReg(0x0060,0xa700);
        LCD5510_WriteReg(0x0061,0x0001);
        LCD5510_WriteReg(0x006a,0x0000);
        LCD5510_WriteReg(0x0080,0x0000);
        LCD5510_WriteReg(0x0081,0x0000);
        LCD5510_WriteReg(0x0082,0x0000);
        LCD5510_WriteReg(0x0083,0x0000);
        LCD5510_WriteReg(0x0084,0x0000);
        LCD5510_WriteReg(0x0085,0x0000);

        LCD5510_WriteReg(0x0090,0x0010);
        LCD5510_WriteReg(0x0092,0x0600);
        //开启显示设置
        LCD5510_WriteReg(0x0007,0x0133);
    } else if(lcddev.id==0x9320)//测试OK.
    {
        LCD5510_WriteReg(0x00,0x0000);
        LCD5510_WriteReg(0x01,0x0100);	//Driver Output Contral.
        LCD5510_WriteReg(0x02,0x0700);	//LCD Driver Waveform Contral.
        LCD5510_WriteReg(0x03,0x1030);//Entry Mode Set.
        //LCD5510_WriteReg(0x03,0x1018);	//Entry Mode Set.

        LCD5510_WriteReg(0x04,0x0000);	//Scalling Contral.
        LCD5510_WriteReg(0x08,0x0202);	//Display Contral 2.(0x0207)
        LCD5510_WriteReg(0x09,0x0000);	//Display Contral 3.(0x0000)
        LCD5510_WriteReg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
        LCD5510_WriteReg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
        LCD5510_WriteReg(0x0d,0x0000);	//Frame Maker Position.
        LCD5510_WriteReg(0x0f,0x0000);	//Extern Display Interface Contral 2.
        delay_ms(50);
        LCD5510_WriteReg(0x07,0x0101);	//Display Contral.
        delay_ms(50);
        LCD5510_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
        LCD5510_WriteReg(0x11,0x0007);								//Power Control 2.(0x0001)
        LCD5510_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3.(0x0138)
        LCD5510_WriteReg(0x13,0x0b00);								//Power Control 4.
        LCD5510_WriteReg(0x29,0x0000);								//Power Control 7.

        LCD5510_WriteReg(0x2b,(1<<14)|(1<<4));
        LCD5510_WriteReg(0x50,0);	//Set X Star
        //水平GRAM终止位置Set X End.
        LCD5510_WriteReg(0x51,239);	//Set Y Star
        LCD5510_WriteReg(0x52,0);	//Set Y End.t.
        LCD5510_WriteReg(0x53,319);	//

        LCD5510_WriteReg(0x60,0x2700);	//Driver Output Control.
        LCD5510_WriteReg(0x61,0x0001);	//Driver Output Control.
        LCD5510_WriteReg(0x6a,0x0000);	//Vertical Srcoll Control.

        LCD5510_WriteReg(0x80,0x0000);	//Display Position? Partial Display 1.
        LCD5510_WriteReg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
        LCD5510_WriteReg(0x82,0x0000);	//RAM Address End-Partial Display 1.
        LCD5510_WriteReg(0x83,0x0000);	//Displsy Position? Partial Display 2.
        LCD5510_WriteReg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
        LCD5510_WriteReg(0x85,0x0000);	//RAM Address End? Partial Display 2.

        LCD5510_WriteReg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
        LCD5510_WriteReg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
        LCD5510_WriteReg(0x93,0x0001);	//Panel Interface Contral 3.
        LCD5510_WriteReg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
        LCD5510_WriteReg(0x97,(0<<8));	//
        LCD5510_WriteReg(0x98,0x0000);	//Frame Cycle Contral.
        LCD5510_WriteReg(0x07,0x0173);	//(0x0173)
    } else if(lcddev.id==0X9331)//OK |/|/|
    {
        LCD5510_WriteReg(0x00E7, 0x1014);
        LCD5510_WriteReg(0x0001, 0x0100); // set SS and SM bit
        LCD5510_WriteReg(0x0002, 0x0200); // set 1 line inversion
        LCD5510_WriteReg(0x0003,(1<<12)|(3<<4)|(1<<3));//65K
        //LCD5510_WriteReg(0x0003, 0x1030); // set GRAM write direction and BGR=1.
        LCD5510_WriteReg(0x0008, 0x0202); // set the back porch and front porch
        LCD5510_WriteReg(0x0009, 0x0000); // set non-display area refresh cycle ISC[3:0]
        LCD5510_WriteReg(0x000A, 0x0000); // FMARK function
        LCD5510_WriteReg(0x000C, 0x0000); // RGB interface setting
        LCD5510_WriteReg(0x000D, 0x0000); // Frame marker Position
        LCD5510_WriteReg(0x000F, 0x0000); // RGB interface polarity
        //*************Power On sequence ****************//
        LCD5510_WriteReg(0x0010, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
        LCD5510_WriteReg(0x0011, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
        LCD5510_WriteReg(0x0012, 0x0000); // VREG1OUT voltage
        LCD5510_WriteReg(0x0013, 0x0000); // VDV[4:0] for VCOM amplitude
        delay_ms(200); // Dis-charge capacitor power voltage
        LCD5510_WriteReg(0x0010, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
        LCD5510_WriteReg(0x0011, 0x0227); // DC1[2:0], DC0[2:0], VC[2:0]
        delay_ms(50); // Delay 50ms
        LCD5510_WriteReg(0x0012, 0x000C); // Internal reference voltage= Vci;
        delay_ms(50); // Delay 50ms
        LCD5510_WriteReg(0x0013, 0x0800); // Set VDV[4:0] for VCOM amplitude
        LCD5510_WriteReg(0x0029, 0x0011); // Set VCM[5:0] for VCOMH
        LCD5510_WriteReg(0x002B, 0x000B); // Set Frame Rate
        delay_ms(50); // Delay 50ms
        LCD5510_WriteReg(0x0020, 0x0000); // GRAM horizontal Address
        LCD5510_WriteReg(0x0021, 0x013f); // GRAM Vertical Address
        // ----------- Adjust the Gamma Curve ----------//
        LCD5510_WriteReg(0x0030, 0x0000);
        LCD5510_WriteReg(0x0031, 0x0106);
        LCD5510_WriteReg(0x0032, 0x0000);
        LCD5510_WriteReg(0x0035, 0x0204);
        LCD5510_WriteReg(0x0036, 0x160A);
        LCD5510_WriteReg(0x0037, 0x0707);
        LCD5510_WriteReg(0x0038, 0x0106);
        LCD5510_WriteReg(0x0039, 0x0707);
        LCD5510_WriteReg(0x003C, 0x0402);
        LCD5510_WriteReg(0x003D, 0x0C0F);
        //------------------ Set GRAM area ---------------//
        LCD5510_WriteReg(0x0050, 0x0000); // Horizontal GRAM Start Address
        LCD5510_WriteReg(0x0051, 0x00EF); // Horizontal GRAM End Address
        LCD5510_WriteReg(0x0052, 0x0000); // Vertical GRAM Start Address
        LCD5510_WriteReg(0x0053, 0x013F); // Vertical GRAM Start Address
        LCD5510_WriteReg(0x0060, 0x2700); // Gate Scan Line
        LCD5510_WriteReg(0x0061, 0x0001); // NDL,VLE, REV
        LCD5510_WriteReg(0x006A, 0x0000); // set scrolling line
        //-------------- Partial Display Control ---------//
        LCD5510_WriteReg(0x0080, 0x0000);
        LCD5510_WriteReg(0x0081, 0x0000);
        LCD5510_WriteReg(0x0082, 0x0000);
        LCD5510_WriteReg(0x0083, 0x0000);
        LCD5510_WriteReg(0x0084, 0x0000);
        LCD5510_WriteReg(0x0085, 0x0000);
        //-------------- Panel Control -------------------//
        LCD5510_WriteReg(0x0090, 0x0010);
        LCD5510_WriteReg(0x0092, 0x0600);
        LCD5510_WriteReg(0x0007, 0x0133); // 262K color and display ON
    } else if(lcddev.id==0x5408)
    {
        LCD5510_WriteReg(0x01,0x0100);
        LCD5510_WriteReg(0x02,0x0700);//LCD Driving Waveform Contral
        LCD5510_WriteReg(0x03,0x1030);//Entry Mode设置
        //指针从左至右自上而下的自动增模式
        //Normal Mode(Window Mode disable)
        //RGB格式
        //16位数据2次传输的8总线设置
        LCD5510_WriteReg(0x04,0x0000); //Scalling Control register
        LCD5510_WriteReg(0x08,0x0207); //Display Control 2
        LCD5510_WriteReg(0x09,0x0000); //Display Control 3
        LCD5510_WriteReg(0x0A,0x0000); //Frame Cycle Control
        LCD5510_WriteReg(0x0C,0x0000); //External Display Interface Control 1
        LCD5510_WriteReg(0x0D,0x0000); //Frame Maker Position
        LCD5510_WriteReg(0x0F,0x0000); //External Display Interface Control 2
        delay_ms(20);
        //TFT 液晶彩色图像显示方法14
        LCD5510_WriteReg(0x10,0x16B0); //0x14B0 //Power Control 1
        LCD5510_WriteReg(0x11,0x0001); //0x0007 //Power Control 2
        LCD5510_WriteReg(0x17,0x0001); //0x0000 //Power Control 3
        LCD5510_WriteReg(0x12,0x0138); //0x013B //Power Control 4
        LCD5510_WriteReg(0x13,0x0800); //0x0800 //Power Control 5
        LCD5510_WriteReg(0x29,0x0009); //NVM read data 2
        LCD5510_WriteReg(0x2a,0x0009); //NVM read data 3
        LCD5510_WriteReg(0xa4,0x0000);
        LCD5510_WriteReg(0x50,0x0000); //设置操作窗口的X轴开始列
        LCD5510_WriteReg(0x51,0x00EF); //设置操作窗口的X轴结束列
        LCD5510_WriteReg(0x52,0x0000); //设置操作窗口的Y轴开始行
        LCD5510_WriteReg(0x53,0x013F); //设置操作窗口的Y轴结束行
        LCD5510_WriteReg(0x60,0x2700); //Driver Output Control
        //设置屏幕的点数以及扫描的起始行
        LCD5510_WriteReg(0x61,0x0001); //Driver Output Control
        LCD5510_WriteReg(0x6A,0x0000); //Vertical Scroll Control
        LCD5510_WriteReg(0x80,0x0000); //Display Position – Partial Display 1
        LCD5510_WriteReg(0x81,0x0000); //RAM Address Start – Partial Display 1
        LCD5510_WriteReg(0x82,0x0000); //RAM address End - Partial Display 1
        LCD5510_WriteReg(0x83,0x0000); //Display Position – Partial Display 2
        LCD5510_WriteReg(0x84,0x0000); //RAM Address Start – Partial Display 2
        LCD5510_WriteReg(0x85,0x0000); //RAM address End – Partail Display2
        LCD5510_WriteReg(0x90,0x0013); //Frame Cycle Control
        LCD5510_WriteReg(0x92,0x0000);  //Panel Interface Control 2
        LCD5510_WriteReg(0x93,0x0003); //Panel Interface control 3
        LCD5510_WriteReg(0x95,0x0110);  //Frame Cycle Control
        LCD5510_WriteReg(0x07,0x0173);
        delay_ms(50);
    }
    else if(lcddev.id==0x1505)//OK
    {
        // second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
        LCD5510_WriteReg(0x0007,0x0000);
        delay_ms(50);
        LCD5510_WriteReg(0x0012,0x011C);//0x011A   why need to set several times?
        LCD5510_WriteReg(0x00A4,0x0001);//NVM
        LCD5510_WriteReg(0x0008,0x000F);
        LCD5510_WriteReg(0x000A,0x0008);
        LCD5510_WriteReg(0x000D,0x0008);
        //伽马校正
        LCD5510_WriteReg(0x0030,0x0707);
        LCD5510_WriteReg(0x0031,0x0007); //0x0707
        LCD5510_WriteReg(0x0032,0x0603);
        LCD5510_WriteReg(0x0033,0x0700);
        LCD5510_WriteReg(0x0034,0x0202);
        LCD5510_WriteReg(0x0035,0x0002); //?0x0606
        LCD5510_WriteReg(0x0036,0x1F0F);
        LCD5510_WriteReg(0x0037,0x0707); //0x0f0f  0x0105
        LCD5510_WriteReg(0x0038,0x0000);
        LCD5510_WriteReg(0x0039,0x0000);
        LCD5510_WriteReg(0x003A,0x0707);
        LCD5510_WriteReg(0x003B,0x0000); //0x0303
        LCD5510_WriteReg(0x003C,0x0007); //?0x0707
        LCD5510_WriteReg(0x003D,0x0000); //0x1313//0x1f08
        delay_ms(50);
        LCD5510_WriteReg(0x0007,0x0001);
        LCD5510_WriteReg(0x0017,0x0001);//开启电源
        delay_ms(50);
        //电源配置
        LCD5510_WriteReg(0x0010,0x17A0);
        LCD5510_WriteReg(0x0011,0x0217);//reference voltage VC[2:0]   Vciout = 1.00*Vcivl
        LCD5510_WriteReg(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
        LCD5510_WriteReg(0x0013,0x0F00);//VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
        LCD5510_WriteReg(0x002A,0x0000);
        LCD5510_WriteReg(0x0029,0x000A);//0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
        LCD5510_WriteReg(0x0012,0x013E);// 0x013C  power supply on
        //Coordinates Control//
        LCD5510_WriteReg(0x0050,0x0000);//0x0e00
        LCD5510_WriteReg(0x0051,0x00EF);
        LCD5510_WriteReg(0x0052,0x0000);
        LCD5510_WriteReg(0x0053,0x013F);
        //Pannel Image Control//
        LCD5510_WriteReg(0x0060,0x2700);
        LCD5510_WriteReg(0x0061,0x0001);
        LCD5510_WriteReg(0x006A,0x0000);
        LCD5510_WriteReg(0x0080,0x0000);
        //Partial Image Control//
        LCD5510_WriteReg(0x0081,0x0000);
        LCD5510_WriteReg(0x0082,0x0000);
        LCD5510_WriteReg(0x0083,0x0000);
        LCD5510_WriteReg(0x0084,0x0000);
        LCD5510_WriteReg(0x0085,0x0000);
        //Panel Interface Control//
        LCD5510_WriteReg(0x0090,0x0013);//0x0010 frenqucy
        LCD5510_WriteReg(0x0092,0x0300);
        LCD5510_WriteReg(0x0093,0x0005);
        LCD5510_WriteReg(0x0095,0x0000);
        LCD5510_WriteReg(0x0097,0x0000);
        LCD5510_WriteReg(0x0098,0x0000);

        LCD5510_WriteReg(0x0001,0x0100);
        LCD5510_WriteReg(0x0002,0x0700);
        LCD5510_WriteReg(0x0003,0x1038);//扫描方向 上->下  左->右
        LCD5510_WriteReg(0x0004,0x0000);
        LCD5510_WriteReg(0x000C,0x0000);
        LCD5510_WriteReg(0x000F,0x0000);
        LCD5510_WriteReg(0x0020,0x0000);
        LCD5510_WriteReg(0x0021,0x0000);
        LCD5510_WriteReg(0x0007,0x0021);
        delay_ms(20);
        LCD5510_WriteReg(0x0007,0x0061);
        delay_ms(20);
        LCD5510_WriteReg(0x0007,0x0173);
        delay_ms(20);
    } else if(lcddev.id==0xB505)
    {
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);

        LCD5510_WriteReg(0x00a4,0x0001);
        delay_ms(20);
        LCD5510_WriteReg(0x0060,0x2700);
        LCD5510_WriteReg(0x0008,0x0202);

        LCD5510_WriteReg(0x0030,0x0214);
        LCD5510_WriteReg(0x0031,0x3715);
        LCD5510_WriteReg(0x0032,0x0604);
        LCD5510_WriteReg(0x0033,0x0e16);
        LCD5510_WriteReg(0x0034,0x2211);
        LCD5510_WriteReg(0x0035,0x1500);
        LCD5510_WriteReg(0x0036,0x8507);
        LCD5510_WriteReg(0x0037,0x1407);
        LCD5510_WriteReg(0x0038,0x1403);
        LCD5510_WriteReg(0x0039,0x0020);

        LCD5510_WriteReg(0x0090,0x001a);
        LCD5510_WriteReg(0x0010,0x0000);
        LCD5510_WriteReg(0x0011,0x0007);
        LCD5510_WriteReg(0x0012,0x0000);
        LCD5510_WriteReg(0x0013,0x0000);
        delay_ms(20);

        LCD5510_WriteReg(0x0010,0x0730);
        LCD5510_WriteReg(0x0011,0x0137);
        delay_ms(20);

        LCD5510_WriteReg(0x0012,0x01b8);
        delay_ms(20);

        LCD5510_WriteReg(0x0013,0x0f00);
        LCD5510_WriteReg(0x002a,0x0080);
        LCD5510_WriteReg(0x0029,0x0048);
        delay_ms(20);

        LCD5510_WriteReg(0x0001,0x0100);
        LCD5510_WriteReg(0x0002,0x0700);
        LCD5510_WriteReg(0x0003,0x1038);//扫描方向 上->下  左->右
        LCD5510_WriteReg(0x0008,0x0202);
        LCD5510_WriteReg(0x000a,0x0000);
        LCD5510_WriteReg(0x000c,0x0000);
        LCD5510_WriteReg(0x000d,0x0000);
        LCD5510_WriteReg(0x000e,0x0030);
        LCD5510_WriteReg(0x0050,0x0000);
        LCD5510_WriteReg(0x0051,0x00ef);
        LCD5510_WriteReg(0x0052,0x0000);
        LCD5510_WriteReg(0x0053,0x013f);
        LCD5510_WriteReg(0x0060,0x2700);
        LCD5510_WriteReg(0x0061,0x0001);
        LCD5510_WriteReg(0x006a,0x0000);
        //LCD5510_WriteReg(0x0080,0x0000);
        //LCD5510_WriteReg(0x0081,0x0000);
        LCD5510_WriteReg(0x0090,0X0011);
        LCD5510_WriteReg(0x0092,0x0600);
        LCD5510_WriteReg(0x0093,0x0402);
        LCD5510_WriteReg(0x0094,0x0002);
        delay_ms(20);

        LCD5510_WriteReg(0x0007,0x0001);
        delay_ms(20);
        LCD5510_WriteReg(0x0007,0x0061);
        LCD5510_WriteReg(0x0007,0x0173);

        LCD5510_WriteReg(0x0020,0x0000);
        LCD5510_WriteReg(0x0021,0x0000);
        LCD5510_WriteReg(0x00,0x22);
    } else if(lcddev.id==0xC505)
    {
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);
        delay_ms(20);
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x0000,0x0000);
        LCD5510_WriteReg(0x00a4,0x0001);
        delay_ms(20);
        LCD5510_WriteReg(0x0060,0x2700);
        LCD5510_WriteReg(0x0008,0x0806);

        LCD5510_WriteReg(0x0030,0x0703);//gamma setting
        LCD5510_WriteReg(0x0031,0x0001);
        LCD5510_WriteReg(0x0032,0x0004);
        LCD5510_WriteReg(0x0033,0x0102);
        LCD5510_WriteReg(0x0034,0x0300);
        LCD5510_WriteReg(0x0035,0x0103);
        LCD5510_WriteReg(0x0036,0x001F);
        LCD5510_WriteReg(0x0037,0x0703);
        LCD5510_WriteReg(0x0038,0x0001);
        LCD5510_WriteReg(0x0039,0x0004);



        LCD5510_WriteReg(0x0090, 0x0015);	//80Hz
        LCD5510_WriteReg(0x0010, 0X0410);	//BT,AP
        LCD5510_WriteReg(0x0011,0x0247);	//DC1,DC0,VC
        LCD5510_WriteReg(0x0012, 0x01BC);
        LCD5510_WriteReg(0x0013, 0x0e00);
        delay_ms(120);
        LCD5510_WriteReg(0x0001, 0x0100);
        LCD5510_WriteReg(0x0002, 0x0200);
        LCD5510_WriteReg(0x0003, 0x1030);

        LCD5510_WriteReg(0x000A, 0x0008);
        LCD5510_WriteReg(0x000C, 0x0000);

        LCD5510_WriteReg(0x000E, 0x0020);
        LCD5510_WriteReg(0x000F, 0x0000);
        LCD5510_WriteReg(0x0020, 0x0000);	//H Start
        LCD5510_WriteReg(0x0021, 0x0000);	//V Start
        LCD5510_WriteReg(0x002A,0x003D);	//vcom2
        delay_ms(20);
        LCD5510_WriteReg(0x0029, 0x002d);
        LCD5510_WriteReg(0x0050, 0x0000);
        LCD5510_WriteReg(0x0051, 0xD0EF);
        LCD5510_WriteReg(0x0052, 0x0000);
        LCD5510_WriteReg(0x0053, 0x013F);
        LCD5510_WriteReg(0x0061, 0x0000);
        LCD5510_WriteReg(0x006A, 0x0000);
        LCD5510_WriteReg(0x0092,0x0300);

        LCD5510_WriteReg(0x0093, 0x0005);
        LCD5510_WriteReg(0x0007, 0x0100);
    } else if(lcddev.id==0x8989)//OK |/|/|
    {
        LCD5510_WriteReg(0x0000,0x0001);//打开晶振
        LCD5510_WriteReg(0x0003,0xA8A4);//0xA8A4
        LCD5510_WriteReg(0x000C,0x0000);
        LCD5510_WriteReg(0x000D,0x080C);
        LCD5510_WriteReg(0x000E,0x2B00);
        LCD5510_WriteReg(0x001E,0x00B0);
        LCD5510_WriteReg(0x0001,0x2B3F);//驱动输出控制320*240  0x6B3F
        LCD5510_WriteReg(0x0002,0x0600);
        LCD5510_WriteReg(0x0010,0x0000);
        LCD5510_WriteReg(0x0011,0x6078); //定义数据格式  16位色 		横屏 0x6058
        LCD5510_WriteReg(0x0005,0x0000);
        LCD5510_WriteReg(0x0006,0x0000);
        LCD5510_WriteReg(0x0016,0xEF1C);
        LCD5510_WriteReg(0x0017,0x0003);
        LCD5510_WriteReg(0x0007,0x0233); //0x0233
        LCD5510_WriteReg(0x000B,0x0000);
        LCD5510_WriteReg(0x000F,0x0000); //扫描开始地址
        LCD5510_WriteReg(0x0041,0x0000);
        LCD5510_WriteReg(0x0042,0x0000);
        LCD5510_WriteReg(0x0048,0x0000);
        LCD5510_WriteReg(0x0049,0x013F);
        LCD5510_WriteReg(0x004A,0x0000);
        LCD5510_WriteReg(0x004B,0x0000);
        LCD5510_WriteReg(0x0044,0xEF00);
        LCD5510_WriteReg(0x0045,0x0000);
        LCD5510_WriteReg(0x0046,0x013F);
        LCD5510_WriteReg(0x0030,0x0707);
        LCD5510_WriteReg(0x0031,0x0204);
        LCD5510_WriteReg(0x0032,0x0204);
        LCD5510_WriteReg(0x0033,0x0502);
        LCD5510_WriteReg(0x0034,0x0507);
        LCD5510_WriteReg(0x0035,0x0204);
        LCD5510_WriteReg(0x0036,0x0204);
        LCD5510_WriteReg(0x0037,0x0502);
        LCD5510_WriteReg(0x003A,0x0302);
        LCD5510_WriteReg(0x003B,0x0302);
        LCD5510_WriteReg(0x0023,0x0000);
        LCD5510_WriteReg(0x0024,0x0000);
        LCD5510_WriteReg(0x0025,0x8000);
        LCD5510_WriteReg(0x004f,0);        //行首址0
        LCD5510_WriteReg(0x004e,0);        //列首址0
    } else if(lcddev.id==0x4531)//OK |/|/|
    {
        LCD5510_WriteReg(0X00,0X0001);
        delay_ms(10);
        LCD5510_WriteReg(0X10,0X1628);
        LCD5510_WriteReg(0X12,0X000e);//0x0006
        LCD5510_WriteReg(0X13,0X0A39);
        delay_ms(10);
        LCD5510_WriteReg(0X11,0X0040);
        LCD5510_WriteReg(0X15,0X0050);
        delay_ms(10);
        LCD5510_WriteReg(0X12,0X001e);//16
        delay_ms(10);
        LCD5510_WriteReg(0X10,0X1620);
        LCD5510_WriteReg(0X13,0X2A39);
        delay_ms(10);
        LCD5510_WriteReg(0X01,0X0100);
        LCD5510_WriteReg(0X02,0X0300);
        LCD5510_WriteReg(0X03,0X1038);//改变方向的
        LCD5510_WriteReg(0X08,0X0202);
        LCD5510_WriteReg(0X0A,0X0008);
        LCD5510_WriteReg(0X30,0X0000);
        LCD5510_WriteReg(0X31,0X0402);
        LCD5510_WriteReg(0X32,0X0106);
        LCD5510_WriteReg(0X33,0X0503);
        LCD5510_WriteReg(0X34,0X0104);
        LCD5510_WriteReg(0X35,0X0301);
        LCD5510_WriteReg(0X36,0X0707);
        LCD5510_WriteReg(0X37,0X0305);
        LCD5510_WriteReg(0X38,0X0208);
        LCD5510_WriteReg(0X39,0X0F0B);
        LCD5510_WriteReg(0X41,0X0002);
        LCD5510_WriteReg(0X60,0X2700);
        LCD5510_WriteReg(0X61,0X0001);
        LCD5510_WriteReg(0X90,0X0210);
        LCD5510_WriteReg(0X92,0X010A);
        LCD5510_WriteReg(0X93,0X0004);
        LCD5510_WriteReg(0XA0,0X0100);
        LCD5510_WriteReg(0X07,0X0001);
        LCD5510_WriteReg(0X07,0X0021);
        LCD5510_WriteReg(0X07,0X0023);
        LCD5510_WriteReg(0X07,0X0033);
        LCD5510_WriteReg(0X07,0X0133);
        LCD5510_WriteReg(0XA0,0X0000);
    } else if(lcddev.id==0x4535)
    {
        LCD5510_WriteReg(0X15,0X0030);
        LCD5510_WriteReg(0X9A,0X0010);
        LCD5510_WriteReg(0X11,0X0020);
        LCD5510_WriteReg(0X10,0X3428);
        LCD5510_WriteReg(0X12,0X0002);//16
        LCD5510_WriteReg(0X13,0X1038);
        delay_ms(40);
        LCD5510_WriteReg(0X12,0X0012);//16
        delay_ms(40);
        LCD5510_WriteReg(0X10,0X3420);
        LCD5510_WriteReg(0X13,0X3038);
        delay_ms(70);
        LCD5510_WriteReg(0X30,0X0000);
        LCD5510_WriteReg(0X31,0X0402);
        LCD5510_WriteReg(0X32,0X0307);
        LCD5510_WriteReg(0X33,0X0304);
        LCD5510_WriteReg(0X34,0X0004);
        LCD5510_WriteReg(0X35,0X0401);
        LCD5510_WriteReg(0X36,0X0707);
        LCD5510_WriteReg(0X37,0X0305);
        LCD5510_WriteReg(0X38,0X0610);
        LCD5510_WriteReg(0X39,0X0610);

        LCD5510_WriteReg(0X01,0X0100);
        LCD5510_WriteReg(0X02,0X0300);
        LCD5510_WriteReg(0X03,0X1030);//改变方向的
        LCD5510_WriteReg(0X08,0X0808);
        LCD5510_WriteReg(0X0A,0X0008);
        LCD5510_WriteReg(0X60,0X2700);
        LCD5510_WriteReg(0X61,0X0001);
        LCD5510_WriteReg(0X90,0X013E);
        LCD5510_WriteReg(0X92,0X0100);
        LCD5510_WriteReg(0X93,0X0100);
        LCD5510_WriteReg(0XA0,0X3000);
        LCD5510_WriteReg(0XA3,0X0010);
        LCD5510_WriteReg(0X07,0X0001);
        LCD5510_WriteReg(0X07,0X0021);
        LCD5510_WriteReg(0X07,0X0023);
        LCD5510_WriteReg(0X07,0X0033);
        LCD5510_WriteReg(0X07,0X0133);
    }
    LCD5510_Display_Dir(1);		 	//默认为竖屏
	 LCD5510_Scan_Dir(3);
    LCD5510_LED=1;					//点亮背光
    LCD5510_Clear(WHITE);
}
//清屏函数
//color:要清屏的填充色
void LCD5510_Clear(u16 color)
{
    u32 index=0;
    u32 totalpoint=lcddev.width;
    totalpoint*=lcddev.height; 			//得到总点数
    if((lcddev.id==0X6804)&&(lcddev.dir==1))//6804横屏的时候特殊处理
    {
        lcddev.dir=0;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;
        LCD5510_SetCursor(0x00,0x0000);		//设置光标位置
        lcddev.dir=1;
        lcddev.setxcmd=0X2B;
        lcddev.setycmd=0X2A;
    } else LCD5510_SetCursor(0x00,0x0000);	//设置光标位置
    LCD5510_WriteRAM_Prepare();     		//开始写入GRAM
    for(index=0; index<totalpoint; index++)
    {
        LCD5510_ADDR->LCD5510_RAM=color;
    }
}
//在指定区域内填充单个颜色
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
//color:要填充的颜色
void LCD5510_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 color)
{
    u16 i,j;
    u16 xlen=0;
    u16 temp;
    if((lcddev.id==0X6804)&&(lcddev.dir==1))	//6804横屏的时候特殊处理
    {
        temp=sx;
        sx=sy;
        sy=lcddev.width-ex-1;
        ex=ey;
        ey=lcddev.width-temp-1;
        lcddev.dir=0;
        lcddev.setxcmd=0X2A;
        lcddev.setycmd=0X2B;
        LCD5510_Fill(sx,sy,ex,ey,color);
        lcddev.dir=1;
        lcddev.setxcmd=0X2B;
        lcddev.setycmd=0X2A;
    } else
    {
        xlen=ex-sx+1;
        for(i=sy; i<=ey; i++)
        {
            LCD5510_SetCursor(sx,i);      				//设置光标位置
            LCD5510_WriteRAM_Prepare();     			//开始写入GRAM
            for(j=0; j<xlen; j++)LCD5510_WR_DATA(color);	//设置光标位置
        }
    }
}
//在指定区域内填充指定颜色块
//(sx,sy),(ex,ey):填充矩形对角坐标,区域大小为:(ex-sx+1)*(ey-sy+1)
//color:要填充的颜色
void LCD5510_Color_Fill(u16 sx,u16 sy,u16 ex,u16 ey,u16 *color)
{
    u16 height,width;
    u16 i,j;
    width=ex-sx+1; 			//得到填充的宽度
    height=ey-sy+1;			//高度
    for(i=0; i<height; i++)
    {
        LCD5510_SetCursor(sx,sy+i);   	//设置光标位置
        LCD5510_WriteRAM_Prepare();     //开始写入GRAM
        for(j=0; j<width; j++)LCD5510_ADDR->LCD5510_RAM=color[i*height+j]; //写入数据
    }
}
//画线
//x1,y1:起点坐标
//x2,y2:终点坐标
void LCD5510_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2)
{
    u16 t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    delta_x=x2-x1; //计算坐标增量
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    if(delta_x>0)incx=1; //设置单步方向
    else if(delta_x==0)incx=0;//垂直线
    else {
        incx=-1;
        delta_x=-delta_x;
    }
    if(delta_y>0)incy=1;
    else if(delta_y==0)incy=0;//水平线
    else {
        incy=-1;
        delta_y=-delta_y;
    }
    if( delta_x>delta_y)distance=delta_x; //选取基本增量坐标轴
    else distance=delta_y;
    for(t=0; t<=distance+1; t++ ) //画线输出
    {
        LCD5510_DrawPoint(uRow,uCol);//画点
        xerr+=delta_x ;
        yerr+=delta_y ;
        if(xerr>distance)
        {
            xerr-=distance;
            uRow+=incx;
        }
        if(yerr>distance)
        {
            yerr-=distance;
            uCol+=incy;
        }
    }
}
//画矩形
//(x1,y1),(x2,y2):矩形的对角坐标
void LCD5510_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD5510_DrawLine(x1,y1,x2,y1);
    LCD5510_DrawLine(x1,y1,x1,y2);
    LCD5510_DrawLine(x1,y2,x2,y2);
    LCD5510_DrawLine(x2,y1,x2,y2);
}
//在指定位置画一个指定大小的圆
//(x,y):中心点
//r    :半径
void Draw_Circle(u16 x0,u16 y0,u8 r)
{
    int a,b;
    int di;
    a=0;
    b=r;
    di=3-(r<<1);             //判断下个点位置的标志
    while(a<=b)
    {
        LCD5510_DrawPoint(x0+a,y0-b);             //5
        LCD5510_DrawPoint(x0+b,y0-a);             //0
        LCD5510_DrawPoint(x0+b,y0+a);             //4
        LCD5510_DrawPoint(x0+a,y0+b);             //6
        LCD5510_DrawPoint(x0-a,y0+b);             //1
        LCD5510_DrawPoint(x0-b,y0+a);
        LCD5510_DrawPoint(x0-a,y0-b);             //2
        LCD5510_DrawPoint(x0-b,y0-a);             //7
        a++;
        //使用Bresenham算法画圆
        if(di<0)di +=4*a+6;
        else
        {
            di+=10+4*(a-b);
            b--;
        }
    }
}
//在指定位置显示一个字符
//x,y:起始坐标
//num:要显示的字符:" "--->"~"
//size:字体大小 12/16
//mode:叠加方式(1)还是非叠加方式(0)
void LCD5510_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode)
{
    u8 temp,t1,t;
    u16 y0=y;
    u16 colortemp=POINT_COLOR;
    //设置窗口
    num=num-' ';//得到偏移后的值
    if(!mode) //非叠加方式
    {
        for(t=0; t<size; t++)
        {
            if(size==12)temp=asc2_1206[num][t];  //调用1206字体
            else temp=asc2_1608[num][t];		 //调用1608字体
            for(t1=0; t1<8; t1++)
            {
                if(temp&0x80)POINT_COLOR=colortemp;
                else POINT_COLOR=BACK_COLOR;
                LCD5510_DrawPoint(x,y);
                temp<<=1;
                y++;
                if(x>=lcddev.width) {
                    POINT_COLOR=colortemp;    //超区域了
                    return;
                }
                if((y-y0)==size)
                {
                    y=y0;
                    x++;
                    if(x>=lcddev.width) {
                        POINT_COLOR=colortemp;    //超区域了
                        return;
                    }
                    break;
                }
            }
        }
    } else//叠加方式
    {
        for(t=0; t<size; t++)
        {
            if(size==12)temp=asc2_1206[num][t];  //调用1206字体
            else temp=asc2_1608[num][t];		 //调用1608字体
            for(t1=0; t1<8; t1++)
            {
                if(temp&0x80)LCD5510_DrawPoint(x,y);
                temp<<=1;
                y++;
                if(x>=lcddev.height) {
                    POINT_COLOR=colortemp;    //超区域了
                    return;
                }
                if((y-y0)==size)
                {
                    y=y0;
                    x++;
                    if(x>=lcddev.width) {
                        POINT_COLOR=colortemp;    //超区域了
                        return;
                    }
                    break;
                }
            }
        }
    }
    POINT_COLOR=colortemp;
}
//m^n函数
//返回值:m^n次方.
u32 LCD5510_Pow(u8 m,u8 n)
{
    u32 result=1;
    while(n--)result*=m;
    return result;
}
//显示数字,高位为0,则不显示
//x,y :起点坐标
//len :数字的位数
//size:字体大小
//color:颜色
//num:数值(0~4294967295);
void LCD5510_ShowNum(u16 x,u16 y,u32 num,u8 len,u8 size)
{
    u8 t,temp;
    u8 enshow=0;
    for(t=0; t<len; t++)
    {
        temp=(num/LCD5510_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                LCD5510_ShowChar(x+(size/2)*t,y,' ',size,0);
                continue;
            } else enshow=1;

        }
        LCD5510_ShowChar(x+(size/2)*t,y,temp+'0',size,0);
    }
}
//显示数字,高位为0,还是显示
//x,y:起点坐标
//num:数值(0~999999999);
//len:长度(即要显示的位数)
//size:字体大小
//mode:
//[7]:0,不填充;1,填充0.
//[6:1]:保留
//[0]:0,非叠加显示;1,叠加显示.
void LCD5510_ShowxNum(u16 x,u16 y,u32 num,u8 len,u8 size,u8 mode)
{
    u8 t,temp;
    u8 enshow=0;
    for(t=0; t<len; t++)
    {
        temp=(num/LCD5510_Pow(10,len-t-1))%10;
        if(enshow==0&&t<(len-1))
        {
            if(temp==0)
            {
                if(mode&0X80)LCD5510_ShowChar(x+(size/2)*t,y,'0',size,mode&0X01);
                else LCD5510_ShowChar(x+(size/2)*t,y,' ',size,mode&0X01);
                continue;
            } else enshow=1;

        }
        LCD5510_ShowChar(x+(size/2)*t,y,temp+'0',size,mode&0X01);
    }
}
//显示字符串
//x,y:起点坐标
//width,height:区域大小
//size:字体大小
//*p:字符串起始地址
void LCD5510_ShowString(u16 x,u16 y,u16 width,u16 height,u8 size,u8 *p)
{
    u8 x0=x;
    width+=x;
    height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符!
    {
        if(x>=width) {
            x=x0;
            y+=size;
        }
        if(y>=height)break;//退出
        LCD5510_ShowChar(x,y,*p,size,0);
        x+=size/2;
        p++;
    }
}






























