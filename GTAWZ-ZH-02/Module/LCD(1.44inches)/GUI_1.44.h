#ifndef __LCD144GUI_144_H__
#define __LCD144GUI_144_H__
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//����Ӳ������Ƭ��STM32F103RBT6,��Ƶ72M  ��Ƭ��������ѹ3.3V
//QDtech-TFTҺ������ for STM32 IOģ��
//xiao��@ShenZhen QDtech co.,LTD
//��˾��վ:www.qdtech.net
//�Ա���վ��http://qdtech.taobao.com
//��˾�ṩ����֧�֣��κμ������⻶ӭ��ʱ����ѧϰ
//�̻�(����) :+86 0755-23594567 
//�ֻ�:15989313508���빤�� 
//����:QDtech2008@gmail.com 
//Skype:QDtech2008
//��������QQȺ:324828016
//��������:2013/5/13
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������ȫ�����Ӽ������޹�˾ 2009-2019
//All rights reserved
//////////////////////////////////////////////////////////////////////////////////
void LCD144_LCD144GUI_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void LCD_1_44_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t color);
void LCD_1_44_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_1_44_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD144Draw_Circle(uint16_t x0,uint16_t y0,uint16_t fc,uint8_t r);
void LCD_1_44_ShowChar(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc, uint8_t num,uint8_t size,uint8_t mode);
void LCD_1_44_ShowNum(uint16_t x,uint16_t y,u32 num,uint8_t len,uint8_t size);
void LCD_1_44_Show2Num(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint8_t size,uint8_t mode);
void LCD_1_44_ShowString(uint16_t x,uint16_t y,uint8_t size,uint8_t *p,uint8_t mode);
void LCD144GUI_DrawFont16(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode);
void LCD144GUI_DrawFont24(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode);
void LCD144GUI_DrawFont32(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *s,uint8_t mode);
void Show_Str(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode);
void Gui_Drawbmp16(uint16_t x,uint16_t y,const unsigned char *p); //��ʾ40*40 QQͼƬ
void gui_circle(int xc, int yc,uint16_t c,int r, int fill);
void Gui_StrCenter(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, uint8_t *str,uint8_t size,uint8_t mode);
void LCD_1_44_DrawFillRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_1_44_ShowNum2412(uint16_t x,uint16_t y,uint16_t fc, uint16_t bc,uint8_t *p ,uint8_t size,uint8_t mode);
#endif

