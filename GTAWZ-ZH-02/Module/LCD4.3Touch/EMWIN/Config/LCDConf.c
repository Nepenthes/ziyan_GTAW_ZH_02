/*********************************************************************
*                SEGGER Microcontroller GmbH & Co. KG                *
*        Solutions for real time microcontroller applications        *
**********************************************************************
*                                                                    *
*        (c) 1996 - 2015  SEGGER Microcontroller GmbH & Co. KG       *
*                                                                    *
*        Internet: www.segger.com    Support:  support@segger.com    *
*                                                                    *
**********************************************************************

** emWin V5.32 - Graphical user interface for embedded applications **
All  Intellectual Property rights  in the Software belongs to  SEGGER.
emWin is protected by  international copyright laws.  Knowledge of the
source code may not be used to write a similar product.  This file may
only be used in accordance with the following terms:

The software has been licensed to  ARM LIMITED whose registered office
is situated at  110 Fulbourn Road,  Cambridge CB1 9NJ,  England solely
for  the  purposes  of  creating  libraries  for  ARM7, ARM9, Cortex-M
series,  and   Cortex-R4   processor-based  devices,  sublicensed  and
distributed as part of the  MDK-ARM  Professional  under the terms and
conditions  of  the   End  User  License  supplied  with  the  MDK-ARM
Professional. 
Full source code is available at: www.segger.com

We appreciate your understanding and fairness.
----------------------------------------------------------------------
Licensing information

Licensor:                 SEGGER Software GmbH
Licensed to:              ARM Ltd, 110 Fulbourn Road, CB1 9NJ Cambridge, UK
Licensed SEGGER software: emWin
License number:           GUI-00181
License model:            LES-SLA-20007, Agreement, effective since October 1st 2011 
Licensed product:         MDK-ARM Professional
Licensed platform:        ARM7/9, Cortex-M/R4
Licensed number of seats: -
----------------------------------------------------------------------
File        : LCDConf.c
Purpose     : Display controller configuration (single layer)
---------------------------END-OF-HEADER------------------------------
*/

#include "GUI.h"
#include "GUIDRV_FlexColor.h"
#include "lcd5510.h"
#include "touch.h"

/*********************************************************************
*
*       Layer configuration (to be modified)
*
**********************************************************************
*/
//
// Physical display size
//   The display size should be adapted in order to match the size of
//   the target display.
//
#define XSIZE_PHYS lcddev.width
#define YSIZE_PHYS lcddev.height

//
// Color conversion
//   The color conversion functions should be selected according to
//   the color mode of the target display. Detaileds can be found in
//   the chapter "Colors" in the emWin user manual.
//
#define COLOR_CONVERSION GUICC_565

//
// Display driver
//   GUIDRV_WIN32 is for use only within the emWin Simulation
//   environment. In order to use the target display controller, the
//   according emWin display driver should be configured as it is
//   described in the chapter "Display Drivers" in the emWin user
//   manual. Beyond that sample configuration files can be found in
//   The folder "Sample\LCDConf\%DISPLAY_DRIVER%\".
//
#define DISPLAY_DRIVER &GUIDRV_Template_API

/*********************************************************************
*
*       Configuration checking
*
**********************************************************************
*/
#ifndef   XSIZE_PHYS
  #error Physical X size of display is not defined!
#endif
#ifndef   YSIZE_PHYS
  #error Physical Y size of display is not defined!
#endif
#ifndef   COLOR_CONVERSION
  #error Color conversion not defined!
#endif
#ifndef   DISPLAY_DRIVER
  #error No display driver defined!
#endif

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_X_Config
*
* Function description
*   Called during the initialization process in order to set up the
*   display driver configuration.
*/
void LCD_X_Config(void) {
	
	GUI_DEVICE_CreateAndLink(&GUIDRV_Template_API,GUICC_M565,0,0); //创建显示驱动器件
	
	LCD_SetSizeEx(0,lcddev.width,lcddev.height);
	LCD_SetVSizeEx(0,lcddev.width,lcddev.height);
	
	GUI_TOUCH_Calibrate(GUI_COORD_X,0,lcddev.width,0,lcddev.width-1);   
	GUI_TOUCH_Calibrate(GUI_COORD_Y,0,lcddev.height,0,lcddev.height-1);
}

/*********************************************************************
*
*       LCD_X_DisplayDriver
*
* Function description
*   Display driver callback function. This function is called by the
*   Display driver for certain purposes. Using GUIDRV_Win32 it is not
*   required to react to any command.
*/
int LCD_X_DisplayDriver(unsigned LayerIndex, unsigned Cmd, void * pData) {
  int r;
  (void) LayerIndex;
  (void) pData;
  
  switch (Cmd) {
  case LCD_X_INITCONTROLLER: {
	//当初始化的时候被调用,主要是设置显示控制器,如果显示控制器在外部初始化则需要用户初始化
		
	 LCD5510_Init(); //初始化LCD 已经在开始初始化了,所以此处不需要初始化。
	 tp_dev.init();
	 
    return 0;
  }
		default:
    r = -1;
	}
  return r;
}

/*************************** End of file ****************************/
