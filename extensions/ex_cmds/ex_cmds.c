/*
 * @Author: your name
 * @Date: 2021-10-11 13:40:10
 * @LastEditTime: 2021-10-11 13:41:15
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/extensions/ex_cmds/ex_cmds.c
 */
#include "ex_cmds.h"

#include "xcmd.h"

#define HELP_RUN ("Run cmd. Usage: run cmd")

static int cmd_run(int argc, char* argv[]) {
    if (argc >= 2) {
        xcmd_exec(argv[1]);
    } else {
        xcmd_print("%s\r\n", HELP_RUN);
    }
    return 0;
}

XCMD_EXPORT_CMD(run, cmd_run, HELP_RUN)

static xcmd_t cmds[] = {
#ifndef ENABLE_XCMD_EXPORT
    {"run", cmd_run, HELP_RUN, NULL},
#endif
};

void ex_cmds_init(void) {
    xcmd_cmd_register(cmds, sizeof(cmds) / sizeof(xcmd_t));
}
