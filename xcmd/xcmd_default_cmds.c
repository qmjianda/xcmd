#include "xcmd_default_cmds.h"
#include <stdlib.h>

static xcmder_t *g_cmder = NULL;

static void cmd_clear(int argc, char* argv[])
{
    xcmd_print(g_cmder, "\033c");
}

static void cmd_help(int argc, char* argv[])
{
    xcmd_t *p = g_cmder->cmd_list.next;
    while(p)
    {
        xcmd_print(g_cmder, "%-20s %s\r\n",p->name, p->help);
        p = p->next;
    }
}

static void cmd_exit(int argc, char* argv[])
{
    exit(0);
}

static void cmd_logo(int argc, char* argv[])
{
    char *log = "\
 _  _  ___  __  __  ____  \n\
( \\/ )/ __)(  \\/  )(  _ \\ \n\
 )  (( (__  )    (  )(_) )\n\
(_/\\_)\\___)(_/\\/\\_)(____/\n ";
    xcmd_print(g_cmder, "%s", log);
}

static xcmd_t cmds[] = 
{
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"exit", cmd_exit, "exit xcmder", NULL},
    {"logo", cmd_logo, "show logo", NULL},
};

void default_cmds_init(xcmder_t *cmder)
{
    xcmd_register(cmder, cmds, sizeof(cmds)/sizeof(xcmd_t));
    g_cmder = cmder;
}
