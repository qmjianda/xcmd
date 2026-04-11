#ifndef XCMD_H
#define XCMD_H

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>
#include "xcmd_define.h"
#include "xcmd_default_confg.h"
#include "xcmd_session.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int(*cmd_func_t)(int, char**);

typedef struct __cmd
{
    const char* name;
    cmd_func_t func;
    const char* help;
#ifndef ENABLE_XCMD_EXPORT
    struct  __cmd *next;
#endif
}xcmd_t;

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

#ifdef ENABLE_XCMD_EXPORT
#define XCMD_EXPORT_CMD(_name, _func, _help)        XCMD_USED const xcmd_t XCMD_SECTION("_xcmd_cmd_list") \
                                                    xcmd_cmd_##_name={\
                                                        .name=#_name, \
                                                        .func=_func, \
                                                        .help=_help\
                                                        };
extern xcmd_t       _xcmd_cmd_list_start;
extern xcmd_t       _xcmd_cmd_list_end;
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = &_xcmd_cmd_list_start; (pos)<&_xcmd_cmd_list_end; ++(pos))
#else
xcmd_t *xcmd_cmdlist_get(void);
#define XCMD_EXPORT_CMD(name, func, help)
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)
#endif

int xcmd_cmd_register(xcmd_t* cmds, uint16_t number);
int xcmd_unregister_cmd(char *cmd);

#ifdef __cplusplus
        }
#endif

#endif /*XCMD_H*/