#include <string.h>
#include "stm32f10x.h"
#include "delay.h"
#include "uart.h"
#include <stdio.h>
#include "xcmd.h"
#include "test.h"

int cmd_get_char(uint8_t *ch)
{
    return uartRdChar(ch);
}

int cmd_put_char(uint8_t ch)
{
    return uartWrChar(ch);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(72);
    uartInit(115200);
    
    xcmd_init(cmd_get_char, cmd_put_char);
    test_cmd_init();
    test_keys_init();
    
    while(1)
    {
        xcmd_task();
    }
}
