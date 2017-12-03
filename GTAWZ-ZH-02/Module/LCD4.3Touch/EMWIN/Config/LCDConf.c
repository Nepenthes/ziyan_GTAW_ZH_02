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
#include "lcd5510.h"

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
#define COLOR_CONVERSION GUICC_M565

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
  //
  // Set display driver and color conversion for 1st layer
  //
  GUI_DEVICE_CreateAndLink(DISPLAY_DRIVER, COLOR_CONVERSION, 0, 0);
  //
  // Display driver configuration
  //
  if (LCD_GetSwapXY()) {
	  
    LCD_SetSizeEx (0, YSIZE_PHYS, XSIZE_PHYS);
    LCD_SetVSizeEx(0, YSIZE_PHYS, XSIZE_PHYS);
  } else {
	  
    LCD_SetSizeEx (0, XSIZE_PHYS, YSIZE_PHYS);
    LCD_SetVSizeEx(0, XSIZE_PHYS, YSIZE_PHYS);
  }
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
	switch (Cmd) 
	{
		case LCD_X_INITCONTROLLER: 
		{
			//
			// Called during the initialization process in order to set up the
			// display controller and put it into operation. If the display
			// controller is not initialized by any external routine this needs
			// to be adapted by the customer...
			//
			// ...
			//  已经在前面初始化了，这里不再初始化
			//  LCD_InitHard();
			LCD5510_Init();
			
			return 0;
		}
		case LCD_X_SETVRAMADDR: 
		{
//			//
//			// Required for setting the address of the video RAM for drivers
//			// with memory mapped video RAM which is passed in the 'pVRAM' element of p
//			//
//			LCD_X_SETVRAMADDR_INFO * p;
//			(void)p;
//			p = (LCD_X_SETVRAMADDR_INFO *)pData;
//			//...
			return 0;
		}
		case LCD_X_SETORG: 
		{
//			//
//			// Required for setting the display origin which is passed in the 'xPos' and 'yPos' element of p
//			//
//			LCD_X_SETORG_INFO * p;
//			(void)p;
//			p = (LCD_X_SETORG_INFO *)pData;

//			//...
			return 0;
		}
		case LCD_X_SHOWBUFFER: 
		{
//			//
//			// Required if multiple buffers are used. The 'Index' element of p contains the buffer index.
//			//
//			LCD_X_SHOWBUFFER_INFO * p;
//			(void)p;
//			p = (LCD_X_SHOWBUFFER_INFO *)pData;
//			//...
			return 0;
		}
		case LCD_X_SETLUTENTRY: 
		{
//			//
//			// Required for setting a lookup table entry which is passed in the 'Pos' and 'Color' element of p
//			//
//			LCD_X_SETLUTENTRY_INFO * p;
//			(void)p;
//			p = (LCD_X_SETLUTENTRY_INFO *)pData;
//			//...
			return 0;
		}
		case LCD_X_ON: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			LCD5510_DisplayOn();
			
			return 0;
		}
		case LCD_X_OFF: 
		{
			//
			// Required if the display controller should support switching on and off
			//
			// ...
			LCD5510_DisplayOff();
			
			return 0;
		}
		default:
		r = -1;
	}
	return r;
}

/*************************** End of file ****************************/
