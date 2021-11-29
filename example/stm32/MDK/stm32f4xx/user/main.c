#include "include.h"
#include <string.h>
#include "stm32f4xx.h"
#include "delay.h"
#include <stdio.h>
#include "xcmd.h"
#include "test.h"

extern void test_cmd_init(void);

int cmd_get_char(uint8_t *ch)
{
    int rcv = uartRdChar(DENUG_UART);
    if(rcv > 0)
    {
        *ch = rcv;
        return 1;
    }
    return 0;
}

int cmd_put_char(uint8_t ch)
{
    return uartWrChar(DENUG_UART, ch);
}

int main(void)
{
    sysInit();
    
    xcmd_init(cmd_get_char, cmd_put_char);
    test_cmd_init();
    test_keys_init();
    
    while(1)
    {
        xcmd_task();
    }
}

