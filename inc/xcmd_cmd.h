#ifndef XCMD_CMD_H
#define XCMD_CMD_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "xcmd_complete.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XCMD_CMD_MAX_NAME_LEN    32

typedef struct xcmd_cmd xcmd_cmd_t;

xcmd_cmd_t *xcmd_cmdlist_get(void);
#define XCMD_EXPORT_CMD(name, func, help)
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)

typedef int (*xcmd_cmd_func_t)(int argc, char *argv[]);

struct xcmd_cmd {
    const char *name;
    xcmd_cmd_func_t func;
    const char *help;
    xcmd_cmd_t *next;
};

int xcmd_cmd_register(xcmd_cmd_t *cmds, int count);
xcmd_cmd_t* xcmd_cmd_find(const char *name);
xcmd_cmd_t* xcmd_cmd_first(void);
int xcmd_cmd_unregister(const char *name);

int xcmd_cmd_complete_name(const char *partial, xcmd_complete_result_t *result);

const char* xcmd_cmd_get_name(xcmd_cmd_t *cmd);

int xcmd_unregister_cmd(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif
