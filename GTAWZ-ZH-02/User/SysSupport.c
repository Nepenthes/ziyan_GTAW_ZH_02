#include "SysSupport.h"

osMutexDef(uart1_mutex);   
osMutexId (uart1_mutex_id); // Mutex ID

void mutexUSART1_Active(void){

	uart1_mutex_id = osMutexCreate(osMutex(uart1_mutex));
}

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  

//本工程不支持printf，请使用sprintf+Driver_USART1.Send
#if 0												
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
_sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
