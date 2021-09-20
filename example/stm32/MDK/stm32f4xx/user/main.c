#include "include.h"
#include <string.h>
#include "stm32f4xx.h"
#include "delay.h"
#include <stdio.h>
#include "xcmd.h"

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

int cmd_ctr_a(void* pv)
{
    xcmd_print("this is ctr+a\n");
    return 0;
}

int cmd_ctr_c(void* pv)
{
    return 0;
}

static xcmd_key_t user_keys[] = 
{
    {CTR_A, cmd_ctr_a,  NULL},
    {CTR_C, cmd_ctr_c, NULL},
};

void user_keys_init(void)
{
    xcmd_key_register(user_keys, sizeof(user_keys)/sizeof(xcmd_key_t));
}


int main(void)
{
    sysInit();
    
    xcmd_init(cmd_get_char, cmd_put_char);
    test_cmd_init();
    user_keys_init();
    
    while(1)
    {
        xcmd_task();
    }
}

