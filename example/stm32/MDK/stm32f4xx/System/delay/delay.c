#include "delay.h"
#include "sys.h"
#include "include.h"


static u8  fac_us=0;							//us延时倍乘数			   
static u16 fac_ms=0;							//ms延时倍乘数,在os下,代表每个节拍的ms数
static uint32_t timerTick = 0;
 
static void(*tickIrqCallBackFun)(void) = NULL;
 
//systick中断服务函数
void SysTick_Handler(void)
{	
    timerTick++;
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
	fac_ms=(u16)fac_us*1000;				//非OS下,代表每个ms需要的systick时钟数
    SysTick_Config(SystemCoreClock / 1000);
}								    

//延时nus
//nus为要延时的us数.	
//注意:nus的值,不要大于798915us(最大值即2^24/fac_us@fac_us=21)
void delay_us(uint32_t nus)
{		
	uint32_t ticks;
	uint32_t told,tnow,tcnt=0;
	uint32_t reload=SysTick->LOAD;				//LOAD的值	    	 
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
	};
}

//延时nms 
void delay_ms(uint32_t nms)
{	 	 
    uint32_t nextTime;
    nextTime = timerTick+nms;
    
    do
    {
        if(nextTime == timerTick)
        {
            break;
        }
    }while(1);
}

void getTick(uint32_t *recTick)
{
    *recTick = timerTick;
}

uint32_t millins(void)
{
    return timerTick;
}

uint64_t microseconds(void)
{
    return timerTick*1000 + (SysTick->LOAD-SysTick->VAL);
}

uint32_t getLapseTime(uint32_t oldTime, uint32_t newTime)
{
    uint32_t ret;
    if(newTime<oldTime)
    {
        ret = (0xFFFFFFFF - oldTime) + newTime;
    }
    else
    {
        ret = newTime - oldTime;
    }
    return ret;
}

void regSysTickCallBackFun(void(*callBackFun)(void))
{
    tickIrqCallBackFun = callBackFun;
}

