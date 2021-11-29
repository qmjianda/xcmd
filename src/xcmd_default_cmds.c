/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-10-11 21:37:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_cmds.c
 */
#include "xcmd_confg.h"
#include "xcmd_default_cmds.h"
#include "xcmd.h"
#include <stdlib.h>

static int cmd_clear(int argc, char* argv[])
{
    xcmd_print("\033c");
    return 0;
}

static int cmd_help(int argc, char* argv[])
{
    xcmd_t *p = xcmd_cmdlist_get();
    while(p)
    {
        xcmd_print("%-20s %s\r\n",p->name, p->help);
        p = p->next;
    }
    return 0;
}

static int cmd_keys(int argc, char* argv[])
{
    xcmd_key_t *p = xcmd_keylist_get();
    while(p)
    {
        xcmd_print("0x%08X\t", p->key);
        xcmd_print("%s\r\n",p->help);
        p = p->next;
    }
    return 0;
}

static int cmd_logo(int argc, char* argv[])
{
    char *log = "\
 _  _  ___  __  __  ____  \r\n\
( \\/ )/ __)(  \\/  )(  _ \\ \r\n\
 )  (( (__  )    (  )(_) )\r\n\
(_/\\_)\\___)(_/\\/\\_)(____/\r\n ";
    xcmd_print("%s", log);
    xcmd_print("\r\n%-10s %s %s\r\n","Build" ,__DATE__,  __TIME__);
    xcmd_print("%-10s %s\r\n","Version", VERSION);
    return 0;
}

static xcmd_t cmds[] = 
{
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"keys", cmd_keys, "show keys", NULL},
    {"logo", cmd_logo, "show logo", NULL},
};

void default_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
