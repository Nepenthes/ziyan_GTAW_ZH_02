#ifndef _LCD_4_3_MAIN_H_
#define _LCD_4_3_MAIN_H_

#include "stm32f10x.h"
#include "delay.h"
#include "IO_Map.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"

#include "lcd5510.h"
#include "touch.h"

#include "GUI.h"
#include "GUIDEMO.h"

void LCD4_3_Init(void);
void LCD4_3_Thread(const void *argument);
void LCD4_3_GUICLK_Thread(const void *argument);
void LCD4_3_Main(void);

#endif
