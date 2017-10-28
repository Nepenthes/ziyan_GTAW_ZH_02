#ifndef DELAY_H
#define DELAY_H

#include "stm32f10x.h"

#define delay_ms	Osdelay

void delay_us(u32 nus);

#endif
