/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-10-11 21:37:09
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_cmds.c
 */
#include "xcmd_default_confg.h"
#include "xcmd_default_cmds.h"
#include "xcmd_cmd.h"
#include "xcmd_session.h"
#include <stdlib.h>

static int cmd_clear(int argc, char* argv[])
{
    xcmd_session_print_current("\033c");
    return 0;
}

static int cmd_help(int argc, char* argv[])
{
    xcmd_cmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
    {
        xcmd_session_print_current("%-20s %s\r\n",p->name, p->help);
    }
    return 0;
}

static int cmd_logo(int argc, char* argv[])
{
    char *log = "\
 _  _  ___  __  __  ____  \r\n\
( \\/ )/ __)(  \\/  )(  _ \\ \r\n\
  ( )  (( (__  )    (  )(_) )\r\n\
 (_/\\_)\\___)(_/\\/\\_)(____/)\r\n ";
    xcmd_session_print_current("%s", log);
    xcmd_session_print_current("\r\n%-10s %s %s\r\n","Build" ,__DATE__,  __TIME__);
    xcmd_session_print_current("%-10s %s\r\n","Version", VERSION);
    return 0;
}

XCMD_EXPORT_CMD(clear, cmd_clear, "clear screen", NULL)
XCMD_EXPORT_CMD(help, cmd_help, "show this list", NULL)
XCMD_EXPORT_CMD(logo, cmd_logo, "show logo", NULL)

static xcmd_cmd_t cmds[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {"clear", cmd_clear, "clear screen", NULL, NULL},
    {"help", cmd_help, "show this list", NULL, NULL},
    {"logo", cmd_logo, "show logo", NULL, NULL},
#endif
};

void default_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_cmd_t));
}
