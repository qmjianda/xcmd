#ifndef __DELAY_H
#define __DELAY_H 			   

#include "stm32f10x.h"

uint64_t no_us(void);
uint32_t now_ms(void);

void delay_init(u8 SYSCLK);
void delay_us(uint32_t nus);
void delay_ms(uint32_t nms);
void regSysTickCallBackFun(void(*callBackFun)(void));
#endif





























