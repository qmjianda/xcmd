#include "delay.h"


static u8  fac_us=0;							//us延时倍乘数			   
static volatile uint32_t timerTick = 0;
static volatile uint64_t sysTimeUs = 0;
static void(*tickIrqCallBackFun)(void) = 0;
 
//systick中断服务函数
void SysTick_Handler(void)
{	
    timerTick++;
    sysTimeUs += 1000;
    if(tickIrqCallBackFun)
    {
        tickIrqCallBackFun();
    }
}
			   
//初始化延迟函数
void delay_init(u8 SYSCLK)
{
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);//SYSTICK使用外部时钟源	 
	fac_us=SYSCLK;						//不论是否使用OS,fac_us都需要使用
    SysTick_Config(SystemCoreClock / 1000);
}								    

void delay_us(uint32_t nus)
{		
	volatile uint32_t ticks;
	volatile uint32_t told,tnow,tcnt=0;
	volatile uint32_t reload=SysTick->LOAD;				//LOAD的值	    	 
	ticks=nus*fac_us; 						//需要的节拍数 
	told=SysTick->VAL;        				//刚进入时的计数器值
	while(1)
	{
		tnow=SysTick->VAL;	
		if(tnow!=told)
		{	    
			if(tnow<told)tcnt+=told-tnow;	//这里注意一下SYSTICK是一个递减的计数器就可以了.
			else tcnt+=reload-tnow+told;	    
			told=tnow;
			if(tcnt>=ticks)break;			//时间超过/等于要延迟的时间,则退出.
		}  
	}
}

//延时nms 
void delay_ms(uint32_t nms)
{	 	 
    volatile uint32_t nextTime;
    nextTime = timerTick+nms;
    if(nextTime < timerTick) /*溢出了*/
    {
        while(nextTime <= timerTick);
    }
    else /*未溢出*/
    {
        while(nextTime >= timerTick);
    }
}

uint64_t now_us(void)
{
    return (sysTimeUs + SysTick->VAL);
}
uint32_t now_ms(void)
{
    return (timerTick);
}

void regSysTickCallBackFun(void(*callBackFun)(void))
{
    tickIrqCallBackFun = callBackFun;
}

