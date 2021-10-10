#include "ex_cmds.h"
#include "xcmd.h"

static int cmd_run(int argc, char* argv[])
{
    if(argc >= 2)
    {
        xcmd_exec(argv[1]);
    }
    else
    {
        xcmd_print("usage: run \"cmd\"\n");
    }
    return 0;
}

static xcmd_t cmds[] = 
{
    {"run", cmd_run, "run cmd", NULL},
};

void ex_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
