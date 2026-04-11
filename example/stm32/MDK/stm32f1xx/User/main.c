#include <string.h>
#include "stm32f10x.h"
#include "delay.h"
#include "uart.h"
#include <stdio.h>
#include "xcmd_cmd.h"
#include "xcmd_io.h"
#include "xcmd_session.h"
#include "xcmd_define.h"
#include "test.h"

int cmd_get_char(uint8_t *ch)
{
    return uartRdChar(ch);
}

int cmd_put_char(uint8_t ch)
{
    return uartWrChar(ch);
}

static xcmd_io_term_ctx_t term_ctx;
static xcmd_session_t session;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(72);
    uartInit(115200);
    
    xcmd_io_t *io = XCMD_IO_TERM_STATIC(&term_ctx, cmd_get_char, cmd_put_char);
    if (!io) {
        return -1;
    }
    
    xcmd_session_init(&session, io, xcmd_term_get_ansi_ops());
    test_cmd_init();
    test_keys_init();
    
    while(1)
    {
        xcmd_session_task(&session);
    }
}
