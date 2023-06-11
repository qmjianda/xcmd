#ifndef XCMD_H
#define XCMD_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "xnr_line.h"
#include "xnr_history.h"
#include "xnr_io.h"
#include "xnr_key.h"
#include "xnr_var.h"
#include "xcmd_default_confg.h"

typedef int (*cmd_func_t)(int, char **);
typedef struct cmd
{
    const char *name;
    cmd_func_t func;
    const char *help;
#ifndef ENABLE_XCMD_EXPORT
    struct cmd *next;
#endif
} xcmd_t;
typedef xcmd_t xcmd_key_t;

#define XCMD_CURRENT() (xcmder_t *)(argv[argc])

#ifndef XCMD_SECTION
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define XCMD_SECTION(x) __attribute__((section(x)))
#elif defined(__IAR_SYSTEMS_ICC__)
#define XCMD_SECTION(x) @x
#elif defined(__GNUC__)
#define XCMD_SECTION(x) __attribute__((section(x)))
#else
#define XCMD_SECTION(x)
#endif
#endif

#ifndef XCMD_USED
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define XCMD_USED __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__)
#define XCMD_USED __root
#elif defined(__GNUC__)
#define XCMD_USED __attribute__((used))
#else
#define XCMD_USED
#endif
#endif

#ifdef ENABLE_XCMD_EXPORT
#define XCMD_EXPORT_CMD(_name, _func, _help) XCMD_USED const xcmd_t XCMD_SECTION("_xcmd_cmd_list") \
                                                 xcmd_cmd_##_name = {                              \
                                                     .name = #_name,                               \
                                                     .func = _func,                                \
                                                     .help = _help};
#define XCMD_EXPORT_KEY(_key, _func, _help) XCMD_USED const xcmd_key_t XCMD_SECTION("_xcmd_key_list") \
                                                xcmd_key_##_key = {                                   \
                                                    .key = _key,                                      \
                                                    .func = _func,                                    \
                                                    .help = _help};
extern xcmd_t _xcmd_cmd_list_start;
extern xcmd_t _xcmd_cmd_list_end;
extern xcmd_key_t _xcmd_key_list_start;
extern xcmd_key_t _xcmd_key_list_end;
#define XCMD_CMD_FOR_EACH(pos) for ((pos) = &_xcmd_cmd_list_start; (pos) < &_xcmd_cmd_list_end; ++(pos))
#define XCMD_KEY_FOR_EACH(pos) for ((pos) = &_xcmd_key_list_start; (pos) < &_xcmd_key_list_end; ++(pos))
#else
/**
 * @description: 获取命令列表，可以通过next指针可以遍历所有指令
 * @param {*}
 * @param {}
 * @return {xcmd_t *}：指令链表表头
 */
xcmd_t *xcmd_cmdlist_get(void);
/**
 * @description: 获取按键列表，可以通过next指针可以遍历所有按键
 * @param {*}
 * @param {}
 * @return {xcmd_key_t *}：快捷键链表表头
 */
xcmd_key_t *xcmd_keylist_get(void);
#define XCMD_EXPORT_CMD(name, func, help)
#define XCMD_EXPORT_KEY(key, func, help)
#define XCMD_CMD_FOR_EACH(pos) for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)
#define XCMD_KEY_FOR_EACH(pos) for ((pos) = xcmd_keylist_get(); (pos); (pos) = (pos)->next)
#endif

typedef struct xcmd
{
    xnr_io_t io;
    xnr_key_t key;
    xnr_var_tab_t var_tab;
    xnr_history_t history;
    xnr_line_t line;
    cmd_func_t unknow_cmd_cbk; // 输入为命令序列，输出未处理
    cmd_func_t pre_cmd_cbk;    // 输入为命令序列，输出重新赋值给argc
    cmd_func_t after_cmd_cbk;  // 输入为命令序列，输出未处理
    void *user_data;           // 用户数据
    int optind;     // 选项在参数列表中的位置
    char *optarg;   // 选项参数的值
    bool _initOK;
} xcmder_t;

void xcmd_init(xcmder_t *xcmder, _io_write_t write, _io_read_t read);
void xcmd_task(xcmder_t *xcmder);
int xcmd_exec(xcmder_t *xcmder, char *str);
int xcmd_getopt(xcmder_t *xcmder, int argc, char * const argv[], const char *optstring);
void xcmd_reg_unknow_cmd_cbk(xcmder_t *xcmder, cmd_func_t cbk);
void xcmd_reg_pre_cmd_cbk(xcmder_t *xcmder, cmd_func_t cbk);
void xcmd_reg_after_cmd_cbk(xcmder_t *xcmder, cmd_func_t cbk);

int xcmd_print(xcmder_t *xcmder, const char *fmt, ...);
int xcmd_puts(xcmder_t *xcmder, const char *str);
int xcmd_putc(xcmder_t *xcmder, char c);
int xcmd_getc(xcmder_t *xcmder);

int xcmd_key_register(xcmd_key_t *keys, uint16_t number);
int xcmd_unregister_key(char *key);
int xcmd_cmd_register(xcmd_t *cmds, uint16_t number);
int xcmd_unregister_cmd(char *cmd);

#endif