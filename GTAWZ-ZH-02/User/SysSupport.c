#include "SysSupport.h"

osMutexDef(uart1_mutex);   
osMutexId (uart1_mutex_id); // Mutex ID

void mutexUSART1_Active(void){

	uart1_mutex_id = osMutexCreate(osMutex(uart1_mutex));
}

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  

//�����̲�֧��printf����ʹ��sprintf+Driver_USART1.Send
#if 0												
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
_sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 
