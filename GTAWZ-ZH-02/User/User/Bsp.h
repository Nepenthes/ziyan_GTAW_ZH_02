#ifndef BSP_H
#define BSP_H

#include <MoudleDats.h>

#include <delay.h>

#include <Test.h>
#include <WirelessTrans_USART.h>
#include <Key&Tips.h>
#include <beep.h>
#include <LCD_1.44.h>

#include <LED_Array.h>

#include <SHT11.h>
#include <TSL2561.h>

void	BSP_Init(void);
void 	BSP_Test(void);

#endif 

