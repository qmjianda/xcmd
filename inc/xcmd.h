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

#ifdef __cplusplus
extern   "C" {
#endif

typedef void(*cmd_func_t)(int argv, char* argc[]);
typedef int(*cmd_key_func_t)(void *data);

typedef struct __cmd
{
    char* name;
    cmd_func_t func;
    char* help;
    struct  __cmd *next;
}xcmd_t;

typedef struct __key
{
    XCMD_KEY_T key;
    cmd_key_func_t func;
    char* help;
    struct __key *next;
}xcmd_key_t;


/**
 * @description: 接时期初始化
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
 * @description: 获取命令列表，可以通过next指针可以遍历所有指令
 * @param {xcmd_key_t*} keys：快捷键集
 * @param {uint16_t} number：快捷键的个数
 * @return {int}：已经注册的快捷键的个数
 */
xcmd_t *xcmd_cmdlist_get(void);

/**
 * @description: 获取案件列表，可以通过next指针可以遍历所有案件
 * @param {xcmd_key_t*} keys：快捷键集
 * @param {uint16_t} number：快捷键的个数
 * @return {int}：已经注册的快捷键的个数
 */
xcmd_key_t *xcmd_keylist_get(void);

/**
 * @description: 手动执行命令
 * @param {char* } str：命令
 * @return {uint8_t}  返回参数的个数
 */
uint8_t xcmd_exec(char *str);

/**
 * @description: 打印字符串
 * @param {char*} str
 * @return 无
 */
void xcmd_print(const char *fmt, ...);


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
void xcmd_display_print(const char *msg);
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
void  xcmd_history_reset(void);

#ifdef __cplusplus
        }
#endif

#endif /*XCMD_H*/
