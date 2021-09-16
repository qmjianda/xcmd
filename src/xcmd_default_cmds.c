/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-09-16 22:58:41
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_cmds.c
 */
#include "../inc/xcmd_default_cmds.h"
#include <stdlib.h>
#include "xcmd.h"

static void cmd_clear(int argc, char* argv[])
{
    xcmd_print("\033c");
}

static void cmd_help(int argc, char* argv[])
{
    xcmd_t *p = xcmd_cmdlist_get();
    while(p)
    {
        xcmd_print("%-20s %s\r\n",p->name, p->help);
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
    xcmd_print("%s", log);
}

static xcmd_t cmds[] = 
{
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"exit", cmd_exit, "exit xcmder", NULL},
    {"logo", cmd_logo, "show logo", NULL},
};

void default_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}
