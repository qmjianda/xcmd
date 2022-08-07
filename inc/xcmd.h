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

#ifdef __cplusplus
extern   "C" {
#endif

typedef int(*cmd_func_t)(int, char**);
typedef int(*cmd_key_func_t)(void *);

typedef struct __cmd
{
    const char* name;
    cmd_func_t func;
    const char* help;
#ifndef ENABLE_XCMD_EXPORT
    struct  __cmd *next;
#endif
}xcmd_t;

typedef struct __key
{
    const char* key;
    cmd_key_func_t func;
    const char* help;
#ifndef ENABLE_XCMD_EXPORT
    struct __key *next;
#endif
}xcmd_key_t;


/**
 * @description: 解释器初始化
 * @param {func*} get_c：获取一个字符的函数
 * @param {func*} put_c：发送一个字符的函数
 * @return {*}
 */
void xcmd_init( int (*get_c)(uint8_t*), int (*put_c)(uint8_t));

/**
 * @description: 解释器的主任务
 * @param {*} 
 * @return {*}
 */
void xcmd_task(void);

/**
 * @description: 注册一组指令
 * @param {xcmd_t*} cmds：指令集
 * @param {uint16_t} number：指令个数
 * @return {int} 已经注册的指令的个数
 */
int xcmd_cmd_register(xcmd_t* cmds, uint16_t number);

/**
 * @description: 注册一组按键
 * @param {xcmd_key_t*} keys：快捷键集
 * @param {uint16_t} number：快捷键的个数
 * @return {int}：已经注册的快捷键的个数
 */
int xcmd_key_register(xcmd_key_t* keys, uint16_t number);

/**
 * @description: 删除已经注册的cmd
 * @param {char*} cmd：cmd集
 * @return {int}：0：success； !0：failed
 */
int xcmd_unregister_cmd(char *cmd);

/**
 * @description:删除已经注册的key
 * @param {char*} key：key集
 * @return {int}：0：success； !0：failed
 */
int xcmd_unregister_key(char *key);

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
#define XCMD_EXPORT_KEY(_key, _func, _help)         XCMD_USED const xcmd_key_t XCMD_SECTION("_xcmd_key_list") \
                                                    xcmd_key_##_key={\
                                                        .key=_key, \
                                                        .func=_func, \
                                                        .help=_help\
                                                        };
extern xcmd_t       _xcmd_cmd_list_start;
extern xcmd_t       _xcmd_cmd_list_end;
extern xcmd_key_t   _xcmd_key_list_start;
extern xcmd_key_t   _xcmd_key_list_end;
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = &_xcmd_cmd_list_start; (pos)<&_xcmd_cmd_list_end; ++(pos))
#define XCMD_KEY_FOR_EACH(pos)  for ((pos) = &_xcmd_key_list_start; (pos)<&_xcmd_key_list_end; ++(pos))
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
#define XCMD_CMD_FOR_EACH(pos)  for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)
#define XCMD_KEY_FOR_EACH(pos)  for ((pos) = xcmd_keylist_get(); (pos); (pos) = (pos)->next)
#endif

/**
 * @description: 手动执行命令
 * @param {char* } str：命令
 * @return {uint8_t}  返回执行结果
 */
int xcmd_exec(char *str);

/**
 * @description: 打印字符串
 */
void xcmd_print(const char *fmt, ...);
void xcmd_put_str(const char *str);

/**
 * @description: 向显示器插入一个字符
 * @param {char} c
 * @return 无
 */
void xcmd_display_insert_char(char c);

/**
 * @description: 删除显示器的一个字符
 * @param {*}
 * @return 无
 */
void xcmd_display_delete_char(void);

/**
 * @description: 返回光标当前的字符
 * @param {char*}cha存储返回的字符
 * @return {uint8_t}0光标位置无字符，1有字符
 */
uint8_t xcmd_display_current_char(char *cha);

/**
 * @description: 清除显示器
 * @param {*}
 * @return 无
 */
void xcmd_display_clear(void);

/**
 * @description: 获取显示器的内容
 * @param {*} 
 * @return {char*} *显示器的内容的指针
 */
char* xcmd_display_get(void);

/**
 * @description: 设置显示器的内容
 * @param {char*} 要现实的内容
 * @return 无
 */
void xcmd_display_print(const char *fmt, ...);
void xcmd_display_write(const char* buf, uint16_t len);

/**
 * @description: 光标操作函数
 * @param {*} 
 * @return {*}
 */
void xcmd_display_cursor_set(uint16_t pos);
uint16_t xcmd_display_cursor_get(void);

/**
 * @description: 设置命令行提示字符串，此函数并不拷贝字符串，只是记住了传入的指针
 * @param {char*} prompt
 * @return {*}
 */
void xcmd_set_prompt(const char* prompt);
const char* xcmd_get_prompt(void);

/**
 * @description: 注册解释器接收函数的钩子函数
 * @param {func_p} 钩子函数，返回0则接收到的数据会返回给解释器，返回1则不会
 * @return {*} 无
 */
void xcmd_register_rcv_hook_func(uint8_t(*func_p)(char*));

/**
 * @description: 获取历史记录的个数
 * @param {*}
 * @return {uint16_t} 已经记录的历史个数
 */
uint16_t xcmd_history_len(void);

/**
 * @description: 插入一条历史记录
 * @param {char*} str
 * @return 无
 */
void xcmd_history_insert(char* str);

/**
 * @description: 获取下一条历史记录
 * @param {*}
 * @return 历史命令
 */
char *xcmd_history_next(void);

/**
 * @description: 获取上条历史记录
 * @param {*}
 * @return 历史命令
 */
char *xcmd_history_prev(void);

/**
 * @description: 获取当前历史记录
 * @param {*}
 * @return 历史命令
 */
char *xcmd_history_current(void);

/**
 * @description: 将历史记录指针指向头部
 * @param {*}
 * @return 无
 */
void  xcmd_history_slider_reset(void);

/**
 * @description: 结束输入
 * @param {*}
 * @return {*}
 */
char* xcmd_end_of_input(void);





#ifdef __cplusplus
        }
#endif

#endif /*XCMD_H*/
