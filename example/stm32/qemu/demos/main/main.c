#define USE_STDPERIPH_DRIVER
#include "stm32_p103.h"
#include "xcmd.h"
#include "test.h"
#include "ex_keys.h"
#include "ex_cmds.h"

int cmd_get_char(uint8_t *ch)
{
    while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
    /* Capture the received byte and print it out. */
    *ch = (USART_ReceiveData(USART2) & 0x7F);
    return 1;
}

int cmd_put_char(uint8_t ch)
{
    send_byte(ch);
    return 1;
}


int main(void)
{
    uint8_t b;

    init_led();

    init_rs232();
    USART_Cmd(USART2, ENABLE);
    xcmd_init(cmd_get_char, cmd_put_char);
    
#ifndef ENABLE_XCMD_EXPORT
    test_cmd_init();
    test_keys_init();
#endif
    while(1) {
        xcmd_task();
    }
}
