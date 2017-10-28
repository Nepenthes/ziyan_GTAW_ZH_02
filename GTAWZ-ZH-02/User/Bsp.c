#include <Bsp.h>

void  BSP_Init(void){
	
	testInit			();
	USARTInit1		();
	USARTInit2		();
	keyInit			();
	BEEP_Init		();
	LCD144_Init		();
	
	Beep_time(500);
}

void BSP_Test(void){

	USARTTest();
	
	keyTest ();

	//LEDTest();
	
	LCD_144_test();
}
