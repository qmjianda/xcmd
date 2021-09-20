#include "bsp_peripheral_clk.h"

uint32_t getGpioClk(GPIO_TypeDef* gpiox)
{
    uint32_t clk = RCC_AHB1Periph_GPIOA;
    if(gpiox == GPIOA)
    {
        clk = RCC_AHB1Periph_GPIOA;
    }
    else if(gpiox == GPIOB)
    {
        clk = RCC_AHB1Periph_GPIOB;
    }
    else if(gpiox == GPIOC)
    {
        clk = RCC_AHB1Periph_GPIOC;
    }
    else if(gpiox == GPIOD)
    {
        clk = RCC_AHB1Periph_GPIOD;
    }
    else if(gpiox == GPIOE)
    {
        clk = RCC_AHB1Periph_GPIOE;
    }
    else if(gpiox == GPIOF)
    {
        clk = RCC_AHB1Periph_GPIOF;
    }
    else if(gpiox == GPIOH)
    {
        clk = RCC_AHB1Periph_GPIOH;
    }
    else if(gpiox == GPIOI)
    {
        clk = RCC_AHB1Periph_GPIOI;
    }
    else if(gpiox == GPIOJ)
    {
        clk = RCC_AHB1Periph_GPIOJ;
    }
    else if(gpiox == GPIOK)
    {
        clk = RCC_AHB1Periph_GPIOK;
    }
    else
    {
        setErrno(CLK_UNDEFINE);
    }
    return clk;
}
