#ifndef BSP_H
#define BSP_H

#include <MoudleDats.h>

#include <delay.h>

#include <Test.h>
#include <WirelessTrans_USART.h>
#include <Key&Tips.h>
#include <beep.h>
#include <LCD_1.44.h>
#include <LCD4.3Main.h>

#include <LED_Array.h>
#include <ledSpray.h>
#include <ledGrow.h>
#include <exAir.h>
#include <airWarming.h>
#include <Curtain.h>
#include <SourceTog.h>

#include <SHT11.h>
#include <TSL2561.h>
#include <BMP180.h>
#include <SensorSim.h>
#include <keyBoard.h>
#include <Sensor_RT.h>
#include <Sensor_Rain.h>
#include <rc522_function.h>
#include <windSpeed.h>
#include <contentCO2.h>
#include <windSpeed.h>
#include <soilHum.h>
#include <LDchip.h>
#include <ElecMeter.h>

void	BSP_Init(void);
void 	BSP_SetOFF(void);

#endif 

