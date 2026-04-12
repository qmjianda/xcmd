#include "ex_redirect.h"
#include <stdio.h>
#include <string.h>
#include "xcmd.h"
#include "xcmd_session.h"

static int cmd_redirect_test(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    xcmd_session_print_current("Testing redirect functionality...\r\n");
    xcmd_session_print_current("This output should go to the redirected target.\r\n");
    xcmd_session_print_current("Redirect test complete.\r\n");
    return 0;
}

static int cmd_redirect_info(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    
    xcmd_session_print_current("VFS Redirect Test Commands:\r\n");
    xcmd_session_print_current("  redirect_test       - Test basic redirect to file\r\n");
    xcmd_session_print_current("  redirect_info       - Show this info\r\n");
    xcmd_session_print_current("\r\n");
    xcmd_session_print_current("Usage examples:\r\n");
    xcmd_session_print_current("  redirect_test > /test.txt        - Redirect output to file\r\n");
    xcmd_session_print_current("  redirect_test > /dev/null        - Redirect to null device\r\n");
    xcmd_session_print_current("  cat /test.txt                    - View redirected output\r\n");
    return 0;
}

static xcmd_cmd_t redirect_cmds[] = {
    {"redirect_test", cmd_redirect_test, "test redirect to file/device", NULL},
    {"redirect_info", cmd_redirect_info, "show redirect test info", NULL},
};

void ex_redirect_init(void)
{
    xcmd_cmd_register(redirect_cmds, sizeof(redirect_cmds)/sizeof(xcmd_cmd_t));
}
