#include "GUI.h"

	void GUI_TOUCH_X_ActivateX(void)
	{
//		ADS7843_CS_LOW();
//		SPI_WriteByte(0x90);
//		ADS7843_CS_HIGH();
	}
	void GUI_TOUCH_X_ActivateY(void)
	{
//		ADS7843_CS_LOW();
//		SPI_WriteByte(0xD0);
//		ADS7843_CS_HIGH();
	}

	int GUI_TOUCH_X_MeasureX(void)
	{
//		return ReadAdc_Flitery();
		//用户触摸函数添加
		return 0;
	}
	int GUI_TOUCH_X_MeasureY(void)
	{
//		return 4095-ReadAdc_Fliterx();
		//用户触摸函数添加
		return 0;
	}
