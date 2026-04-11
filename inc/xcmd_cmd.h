#ifndef XCMD_CMD_H
#define XCMD_CMD_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XCMD_CMD_MAX_NAME_LEN    32
#define XCMD_CMD_COMPLETION_MAX 16

#ifndef XCMD_SECTION
    #if defined(__CC_ARM) || defined(__CLANG_ARM)
        #define XCMD_SECTION(x)                __attribute__((section(x)))
    #elif defined (__IAR_SYSTEMS_ICC__)
        #define XCMD_SECTION(x)                @ x
    #elif defined(__GNUC__)
        #define XCMD_SECTION(x)                __attribute__((section(x)))
    #else
        #define XCMD_SECTION(x)
    #endif
#endif

#ifndef XCMD_USED
    #if defined(__CC_ARM) || defined(__CLANG_ARM)
        #define XCMD_USED                      __attribute__((used))
    #elif defined (__IAR_SYSTEMS_ICC__)
        #define XCMD_USED                      __root
    #elif defined(__GNUC__)
        #define XCMD_USED                      __attribute__((used))
    #else
        #define XCMD_USED
    #endif
#endif

typedef struct xcmd_cmd xcmd_cmd_t;

#ifdef ENABLE_XCMD_EXPORT
#define XCMD_EXPORT_CMD(_name, _func, _help, _complete)        XCMD_USED const xcmd_cmd_t XCMD_SECTION("_xcmd_cmd_list") \
                                                      xcmd_cmd_##_name={\
                                                          .name=#_name, \
                                                          .func=_func, \
                                                          .help=_help, \
                                                          .complete=_complete, \
                                                          .next=NULL\
                                                          };
extern xcmd_cmd_t       _xcmd_cmd_list_start;
extern xcmd_cmd_t       _xcmd_cmd_list_end;
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = &_xcmd_cmd_list_start; (pos)<&_xcmd_cmd_list_end; ++(pos))
#else
xcmd_cmd_t *xcmd_cmdlist_get(void);
#define XCMD_EXPORT_CMD(name, func, help, complete)
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)
#endif

typedef struct {
    char *candidates[XCMD_CMD_COMPLETION_MAX];
    int count;
} xcmd_complete_result_t;

typedef int (*xcmd_cmd_func_t)(int argc, char *argv[]);
typedef int (*xcmd_cmd_complete_t)(const char *args, xcmd_complete_result_t *result);

struct xcmd_cmd {
    const char *name;
    xcmd_cmd_func_t func;
    const char *help;
    xcmd_cmd_complete_t complete;
    xcmd_cmd_t *next;
};

int xcmd_cmd_register(xcmd_cmd_t *cmds, int count);
xcmd_cmd_t* xcmd_cmd_find(const char *name);
xcmd_cmd_t* xcmd_cmd_first(void);
int xcmd_cmd_unregister(const char *name);

int xcmd_cmd_complete_name(const char *partial, xcmd_complete_result_t *result);

int xcmd_cmd_complete_prefix(const char *input, 
                            const char *candidates[],
                            xcmd_complete_result_t *result);

int xcmd_cmd_complete_enum(const char *input,
                           const char *enum_vals[],
                           xcmd_complete_result_t *result);

int xcmd_cmd_complete_path(const char *path, xcmd_complete_result_t *result);

const char* xcmd_cmd_get_name(xcmd_cmd_t *cmd);

int xcmd_unregister_cmd(const char *cmd);

#ifdef __cplusplus
}
#endif

#endif
