#ifndef __DELAY_H
#define __DELAY_H 			   
#include <sys.h>	  

#define bsp_DelayUS(xus)   delay_us(xus)
#define bsp_DelayMS(xms)   delay_ms(xms)

void delay_init(u8 SYSCLK);
void delay_us(uint32_t nus);
void delay_ms(uint32_t nms);
void getTick(uint32_t *recTick);
uint32_t millins(void);
uint64_t microseconds(void);
uint32_t getLapseTime(uint32_t oldTime, uint32_t newTime);
void regSysTickCallBackFun(void(*callBackFun)(void));
#endif





























