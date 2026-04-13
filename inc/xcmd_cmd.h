#ifndef XCMD_CMD_H
#define XCMD_CMD_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XCMD_CMD_MAX_NAME_LEN    32
#define XCMD_COMPLETION_MAX      16

typedef struct xcmd_cmd xcmd_cmd_t;

xcmd_cmd_t *xcmd_cmdlist_get(void);
#define XCMD_EXPORT_CMD(name, func, help)
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)

typedef int (*xcmd_cmd_func_t)(int argc, char *argv[]);
struct xcmd_line_editor;
typedef int (*xcmd_complete_cb)(struct xcmd_line_editor *le,
                                 const char *text,
                                 int word_id,
                                 char *matches[],
                                 int max);

struct xcmd_cmd {
    const char *name;
    xcmd_cmd_func_t func;
    const char *help;
    xcmd_complete_cb complete;
    xcmd_cmd_t *next;
};

int xcmd_cmd_register(xcmd_cmd_t *cmds, int count);
xcmd_cmd_t* xcmd_cmd_find(const char *name);
xcmd_cmd_t* xcmd_cmd_first(void);
int xcmd_cmd_unregister(const char *name);

const char* xcmd_cmd_get_name(xcmd_cmd_t *cmd);

int xcmd_unregister_cmd(const char *cmd);

int xcmd_complete_default(const char *partial, char *matches[], int max);
void xcmd_register_path_complete(xcmd_complete_cb cb);
xcmd_complete_cb xcmd_get_path_complete(void);

#ifdef __cplusplus
}
#endif

#endif
