#include "LCD4.3Main.h"

osThreadId tid_LCD5510_4_3_Thread;
osThreadDef(LCD4_3_Thread,osPriorityNormal,1,15360);

osTimerDef(Tim_GUIClk_Thread,LCD4_3_GUICLK_Thread);

extern __IO int32_t OS_TimeMS;

void Load_Drow_Dialog(void)
{
    LCD5510_Clear(WHITE);//����
    POINT_COLOR=BLUE;//��������Ϊ��ɫ
    LCD5510_ShowString(lcddev.width-24,0,200,16,16,"RST");//��ʾ��������
    POINT_COLOR=RED;//���û�����ɫ
}
////////////////////////////////////////////////////////////////////////////////
//���ݴ�����ר�в���
//��ˮƽ��
//x0,y0:����
//len:�߳���
//color:��ɫ
void gui_draw_hline(u16 x0,u16 y0,u16 len,u16 color)
{
    if(len==0)return;
    LCD5510_Fill(x0,y0,x0+len-1,y0,color);
}
//��ʵ��Բ
//x0,y0:����
//r:�뾶
//color:��ɫ
void gui_fill_circle(u16 x0,u16 y0,u16 r,u16 color)
{
    u32 i;
    u32 imax = ((u32)r*707)/1000+1;
    u32 sqmax = (u32)r*(u32)r+(u32)r/2;
    u32 x=r;
    gui_draw_hline(x0-r,y0,2*r,color);
    for (i=1; i<=imax; i++)
    {
        if ((i*i+x*x)>sqmax)// draw lines from outside
        {
            if (x>imax)
            {
                gui_draw_hline (x0-i+1,y0+x,2*(i-1),color);
                gui_draw_hline (x0-i+1,y0-x,2*(i-1),color);
            }
            x--;
        }
// draw lines from inside (center)
        gui_draw_hline(x0-x,y0+i,2*x,color);
        gui_draw_hline(x0-x,y0-i,2*x,color);
    }
}
//������֮��ľ���ֵ
//x1,x2����ȡ��ֵ��������
//����ֵ��|x1-x2|
u16 my_abs(u16 x1,u16 x2)
{
    if(x1>x2)return x1-x2;
    else return x2-x1;
}
//��һ������
//(x1,y1),(x2,y2):��������ʼ����
//size�������Ĵ�ϸ�̶�
//color����������ɫ
void lcd_draw_bline(u16 x1, u16 y1, u16 x2, u16 y2,u8 size,u16 color)
{
    u16 t;
    int xerr=0,yerr=0,delta_x,delta_y,distance;
    int incx,incy,uRow,uCol;
    if(x1<size|| x2<size||y1<size|| y2<size)return;
    delta_x=x2-x1; //������������
    delta_y=y2-y1;
    uRow=x1;
    uCol=y1;
    if(delta_x>0)incx=1; //���õ�������
    else if(delta_x==0)incx=0;//��ֱ��
    else {
        incx=-1;
        delta_x=-delta_x;
    }
    if(delta_y>0)incy=1;
    else if(delta_y==0)incy=0;//ˮƽ��
    else {
        incy=-1;
        delta_y=-delta_y;
    }
    if( delta_x>delta_y)distance=delta_x; //ѡȡ��������������
    else distance=delta_y;
    for(t=0; t<=distance+1; t++ ) //�������
    {
        gui_fill_circle(uRow,uCol,size,color);//����
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
////////////////////////////////////////////////////////////////////////////////
//5�����ص����ɫ
const u16 POINT_COLOR_TBL[CT_MAX_TOUCH]= {RED,GREEN,BLUE,BROWN,GRED};
//���败�������Ժ���
void rtp_test(void)
{
    u8 key;
    u8 i=0;
    while(1)
    {
        key=KEY_Scan(0);
        tp_dev.scan(0);
        if(tp_dev.sta&TP_PRES_DOWN)			//������������
        {
            if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height)
            {
                if(tp_dev.x[0]>(lcddev.width-24)&&tp_dev.y[0]<16)Load_Drow_Dialog();//���
                else TP_Draw_Big_Point(tp_dev.x[0],tp_dev.y[0],RED);		//��ͼ
            }
        } else delay_ms(10);	//û�а������µ�ʱ��
        if(key==KEY_RIGHT_TP)	//KEY_RIGHT����,��ִ��У׼����
        {
            LCD5510_Clear(WHITE);//����
            TP_Adjust();  //��ĻУ׼
            TP_Save_Adjdata();
            Load_Drow_Dialog();
        }
        i++;
//if(i%20==0)LED0=!LED0;
    }
}
//���ݴ��������Ժ���
void ctp_test(void)
{
    u8 t=0;
    u8 i=0;
    u16 lastpos[5][2];		//���һ�ε�����
    while(1)
    {
        tp_dev.scan(0);
        for(t=0; t<CT_MAX_TOUCH; t++)
        {
            if((tp_dev.sta)&(1<<t))
            {
                if(tp_dev.x[t]<lcddev.width&&tp_dev.y[t]<lcddev.height)
                {
                    if(lastpos[t][0]==0XFFFF)
                    {
                        lastpos[t][0] = tp_dev.x[t];
                        lastpos[t][1] = tp_dev.y[t];
                    }
                    lcd_draw_bline(lastpos[t][0],lastpos[t][1],tp_dev.x[t],tp_dev.y[t],2,POINT_COLOR_TBL[t]);//����
                    lastpos[t][0]=tp_dev.x[t];
                    lastpos[t][1]=tp_dev.y[t];
                    if(tp_dev.x[t]>(lcddev.width-24)&&tp_dev.y[t]<16)
                    {
                        Load_Drow_Dialog();//���
                    }
                }
            } else lastpos[t][0]=0XFFFF;
        }

        delay_ms(5);
        i++;
//if(i%20==0)LED0=!LED0;
    }
}

void LCD4_3_Test(void) {

    LCD5510_Init();
    tp_dev.init();
    POINT_COLOR=RED;//��������Ϊ��ɫ

    POINT_COLOR = LCD5510_ReadPoint(100,100);
	
    while(1)
    {
        LCD5510_ShowString(60,50,200,16,16," STM32 ");
        LCD5510_ShowString(60,70,200,16,16,"TOUCH TEST");
        LCD5510_ShowString(60,90,200,16,16,"ATOM@ALIENTEK");
        LCD5510_ShowString(60,110,200,16,16,"2014/3/11");

        if(tp_dev.touchtype!=0XFF)
        {
            LCD5510_ShowString(60,130,200,16,16,"Press KEY0 to Adjust");
        }
        delay_ms(1500);
        Load_Drow_Dialog();
        ctp_test();	//����������
    }
}

void LCD4_3_Init(void) {

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    GUI_Init();
//	LCD5510_Init();
}

void LCD4_3_Thread(const void *argument) {

	GUI_EnableAlpha(1);
	GUI_SetBkColor(GUI_CYAN);
	GUI_Clear();
	GUI_SetColor(GUI_BLACK);
	GUI_DispString("Hello World!!!");
	GUI_DispStringHCenterAt("Alphablending", 45, 41);
	GUI_SetColor((0x40uL << 24) | GUI_RED);
	GUI_FillRect(0, 0, 49, 49);
	GUI_SetColor((0x80uL << 24) | GUI_GREEN);
	GUI_FillRect(20, 20, 69, 69);
	GUI_SetColor((0xC0uL << 24) | GUI_BLUE);
	GUI_FillRect(40, 40, 89, 89);
	while(1);

//	GUIDEMO_Main();

// LCD4_3_Test();
}

void LCD4_3_GUICLK_Thread(const void *argument) {

    OS_TimeMS ++;
}

void LCD4_3_Main(void) {

    osTimerId tmrid_GUIClk;

    tid_LCD5510_4_3_Thread = osThreadCreate(osThread(LCD4_3_Thread),NULL);
    tmrid_GUIClk = osTimerCreate(osTimer(Tim_GUIClk_Thread), osTimerPeriodic, &LCD4_3_GUICLK_Thread);

    osTimerStart(tmrid_GUIClk,1);
}
