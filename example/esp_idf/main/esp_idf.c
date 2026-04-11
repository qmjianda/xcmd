#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xcmd_cmd.h"
#include "xcmd_io.h"
#include "xcmd_session.h"
#include "xcmd_define.h"
#include "test.h"

int cmd_get_char(unsigned char *ch)
{
    *ch = getc(stdin);
    return 1;
}

int cmd_put_char(unsigned char ch)
{
    putchar(ch);
    return 1;
}

static xcmd_io_term_ctx_t term_ctx;
static xcmd_session_t session;

void app_main(void)
{
    xcmd_io_t *io = XCMD_IO_TERM_STATIC(&term_ctx, cmd_get_char, cmd_put_char);
    if (!io) {
        return;
    }
    
    xcmd_session_init(&session, io, xcmd_term_get_ansi_ops());
    test_cmd_init();
    test_keys_init();
    
    while(1)
    {
        xcmd_session_task(&session);
    }
}
