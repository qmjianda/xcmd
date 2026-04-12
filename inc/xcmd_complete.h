#ifndef XCMD_COMPLETE_H
#define XCMD_COMPLETE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XCMD_COMPLETION_MAX  16

typedef enum {
    XCMD_PARAM_TYPE_NONE,
    XCMD_PARAM_TYPE_STRING,
    XCMD_PARAM_TYPE_PATH,
    XCMD_PARAM_TYPE_ENUM,
} xcmd_param_type_t;

typedef struct {
    xcmd_param_type_t type;
    const char *const *enum_vals;
} xcmd_param_def_t;

typedef struct {
    const char *name;
    int (*func)(int argc, char *argv[]);
    const char *help;
    const xcmd_param_def_t *params;
    int param_count;
} xcmd_subcmd_def_t;

typedef struct {
    const char *name;
    int (*func)(int argc, char *argv[]);
    const char *help;
    const xcmd_subcmd_def_t *subcmds;
    int subcmd_count;
} xcmd_cmd_tree_t;

typedef struct {
    char *candidates[16];
    int count;
} xcmd_complete_result_t;

typedef int (*xcmd_path_complete_cb)(const char *path, xcmd_complete_result_t *result);

#define XCMD_PARAM_NONE()         { XCMD_PARAM_TYPE_NONE, NULL }
#define XCMD_PARAM_STRING()       { XCMD_PARAM_TYPE_STRING, NULL }
#define XCMD_PARAM_PATH()         { XCMD_PARAM_TYPE_PATH, NULL }
#define XCMD_PARAM_ENUM(vals)     { XCMD_PARAM_TYPE_ENUM, vals }

#define XCMD_SUBCMD(n, f, h)         { #n, f, h, NULL, 0 }
#define XCMD_SUBCMD_EX(n, f, h, p, c) { #n, f, h, p, c }

int xcmd_cmd_register_tree(const xcmd_cmd_tree_t *tree);
const xcmd_cmd_tree_t* xcmd_cmd_tree_find(const char *name);
int xcmd_complete_subcmd(const char *partial, const xcmd_subcmd_def_t *subcmds, int count,
                        xcmd_complete_result_t *result);
int xcmd_complete_param(const char *partial, xcmd_param_type_t type,
                        const char *const *enum_vals,
                        xcmd_complete_result_t *result);
int xcmd_complete_cmd(const char *partial, xcmd_complete_result_t *result);
void xcmd_register_path_complete_cb(xcmd_path_complete_cb cb);

#ifdef __cplusplus
}
#endif

#endif
