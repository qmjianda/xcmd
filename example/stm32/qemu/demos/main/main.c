#define USE_STDPERIPH_DRIVER
#include "stm32_p103.h"
#include "xcmd_cmd.h"
#include "xcmd_io.h"
#include "xcmd_session.h"
#include "xcmd_define.h"
#include "test.h"
#include "ex_keys.h"
#include "ex_cmds.h"

int cmd_get_char(uint8_t *ch)
{
    while(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);
    *ch = (USART_ReceiveData(USART2) & 0x7F);
    return 1;
}

int cmd_put_char(uint8_t ch)
{
    send_byte(ch);
    return 1;
}

static xcmd_io_term_ctx_t term_ctx;
static xcmd_session_t session;

int main(void)
{
    init_led();
    init_rs232();
    USART_Cmd(USART2, ENABLE);
    
    xcmd_io_t *io = XCMD_IO_TERM_STATIC(&term_ctx, cmd_get_char, cmd_put_char);
    if (!io) {
        return -1;
    }
    
    xcmd_session_init(&session, io, xcmd_term_get_ansi_ops());
    
    test_cmd_init();
    test_keys_init();
    
    while(1) {
        xcmd_session_task(&session);
    }
}
