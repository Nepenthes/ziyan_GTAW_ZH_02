#ifndef __RC522_FUNCTION_H
#define __RC522_FUNCTION_H

#include "rc522_config.h"

#include "stm32f10x_it.h"

#include "stdio.h"
#include "string.h"
#include "Driver_USART.h"
#include "beep.h"

#define          macDummy_Data              0x00


void             PcdReset                   ( void );                       //��λ
void             M500PcdConfigISOType       ( u8 type );                    //������ʽ
char             PcdRequest                 ( u8 req_code, u8 * pTagType ); //Ѱ��
char             PcdAnticoll                ( u8 * pSnr);                   //������

void RC522_Init(void);
void RC522MS_Thread(const void *argument);
void RC522(void);

#endif /* __RC522_FUNCTION_H */

