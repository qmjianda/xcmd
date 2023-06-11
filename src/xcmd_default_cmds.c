#include "xcmd_default_confg.h"
#include "xcmd_default_cmds.h"
#include "xcmd.h"
#include <stdlib.h>

static int cmd_clear(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xcmd_print(xcmder, "\033c");
    return 0;
}

static int cmd_help(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xcmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
    {
        xcmd_print(xcmder, "%-20s %s\r\n",p->name, p->help);
    }
    return 0;
}

static int cmd_keys(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xcmd_key_t *p = NULL;
    XCMD_KEY_FOR_EACH(p)
    {
        xcmd_print(xcmder, "0x%08X\t", p->name);
        xcmd_print(xcmder, "%s\r\n",p->help);
    }
    return 0;
}

static int cmd_logo(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char *log = "\
 _  _  ___  __  __  ____  \r\n\
( \\/ )/ __)(  \\/  )(  _ \\ \r\n\
 )  (( (__  )    (  )(_) )\r\n\
(_/\\_)\\___)(_/\\/\\_)(____/\r\n ";
    xcmd_print(xcmder, "%s", log);
    xcmd_print(xcmder, "\r\n%-10s %s %s\r\n","Build" ,__DATE__,  __TIME__);
    xcmd_print(xcmder, "%-10s %s\r\n","Version", VERSION);
    return 0;
}

static int cmd_set(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    if(argc >= 3)
    {
        xnr_var_set(&xcmder->var_tab, argv[1], argv[2]);
    }
    return 0;
}

static int cmd_var(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    int id = 0;
    xnr_var_t *var;
    while(1)
    {
        var = xnr_var(&xcmder->var_tab, id++);
        if(NULL == var)
            break;
        if(var->name[0] != '\0')
            xcmd_print(xcmder, "%-15s:%s\n", var->name, var->value);
    }
    return 0;
}

XCMD_EXPORT_CMD(clear, cmd_clear, "clear screen")
XCMD_EXPORT_CMD(help, cmd_help, "show this list")
XCMD_EXPORT_CMD(keys, cmd_keys, "show keys")
XCMD_EXPORT_CMD(logo, cmd_logo, "show logo")
XCMD_EXPORT_CMD(set, cmd_set, "set var")
XCMD_EXPORT_CMD(set, cmd_var, "get all var")

static xcmd_t cmds[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"keys", cmd_keys, "show keys", NULL},
    {"logo", cmd_logo, "show logo", NULL},
    {"set", cmd_set, "set var", NULL},
    {"var", cmd_var, "get all var", NULL},
#endif
};

void default_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
