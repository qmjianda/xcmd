#ifndef XCMD_H
#define XCMD_H

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdarg.h>

typedef void(*cmd_func_t)(int argv, char* argc[]);
typedef int(*cmd_key_func_t)(void *data);

typedef enum
{
    CTR_A = 1,
    CTR_B = 2,
    CTR_C = 3,
    CTR_D = 4,
    CTR_E = 5,
    CTR_F = 6,
    CTR_G = 7,
    CTR_H = 8,
    BACKSPACE = 8,
    CTR_I = 9,
    LF = 9,
    TAB = 9,
    CTR_J = 10,
    CTR_K = 11,
    CTR_L = 12,
    CTR_M = 13,
    CR = 13,
    CTR_N = 14,
    CTR_O = 15,
    CTR_P = 16,
    CTR_Q = 17,
    CTR_R = 18,
    CTR_S = 19,
    CTR_T = 20,
    CTR_U = 21,
    CTR_V = 22,
    CTR_W = 23,
    CTR_X = 24,
    CTR_Y = 25,
    CTR_Z = 26,
    ESC  = 27,
    FS  = 28,
    GS  = 29,
    RS  = 30,
    US  = 31,
    L_DELETE = 127,
    UP = 0x00415B1B,
    DW = 0x00425B1B,
    RIGHT = 0x00435B1B,
    LEFT = 0x00445B1B,
    END = 0x00465B1B,
    HOME = 0x00485B1B,
    INSERT = 0x7E325B1B,
    R_DELETE = 0x7E335B1B,
    PGUP = 0x7E355B1B,
    PGDW = 0x7E365B1B,
}XCMD_KEY_T;

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
    struct __key *next;
}xcmd_key_t;

typedef struct __history
{
    char *line;
    struct __history *next;
    struct __history *prev;
}xcmd_history_t;

typedef struct
{
    struct
    {
        int (*get_c)(uint8_t*);
        int (*put_c)(uint8_t);
    }io;

    struct 
    {
        uint16_t len;
        xcmd_t *next;
        xcmd_t *tail;
    }cmd_list;

    struct
    {
        uint16_t len;
        xcmd_key_t *next;
        
    }key_list;

    struct
    {
        struct
        {
            uint16_t len;
            xcmd_history_t *next;
            xcmd_history_t *handle;
        }history_list;
        char *display_line;  /* 显示区的缓存 */
        uint16_t line_totle; /* 一共有多少行 */
        uint16_t line_len;   /* 每一行的最大长度 */
        uint16_t byte_num;   /* 当前行的字符个数 */
        uint16_t cursor;     /* 光标所在位置 */
        uint8_t  encode_case_stu;
        uint32_t key_val;
        uint16_t param_len;
    }parser;
    uint8_t _initOK;
} xcmder_t;

/**
 * @description: 创建默认的解释器，命令行支持最长字符64个，历史长度最长10个，最长参数支持16个
 * @param {p_func}getchar：获取一个字符的函数
 * @param {p_func}putchar: 发送一个字符的函数
 * @return 返回创建的一个解释器指针
 */
#define  xcmd_create_default(getchar, putchar)  xcmd_create(getchar,  putchar, 64, 10, 16);

/**
 * @description: 
 * @param {p_func}getchar：获取一个字符的函数
 * @param {p_func}putchar: 发送一个字符的函数
 * @param {uint16_t} cmd_len：命令行最长支持的字符数
 * @param {uint16_t} history_len：命令行支持的最多历史数
 * @param {uint16_t} param_len：支持解析的最多参数个数
 * @return 返回创建的一个解释器指针
 */
xcmder_t *xcmd_create( int (*get_c)(uint8_t*), int (*put_c)(uint8_t), uint16_t cmd_len, uint16_t history_len, uint16_t param_len);

/**
 * @description: 删除一个解释器
 * @param {xcmder_t*} cmder：解释器指针
 * @return 无
 */
void xcmd_destory(xcmder_t* cmder);

/**
 * @description: 解释器的主任务
 * @param {xcmder_t*} cmder：解释器指针
 * @return {*}
 */
void xcmd_task(xcmder_t* cmder);

/**
 * @description: 注册一组指令
 * @param {xcmder_t*} cmder：解释器指针
 * @param {xcmd_t*} cmds：指令集
 * @param {uint16_t} number：指令个数
 * @return {int} 已经注册的指令的个数
 */
int xcmd_register(xcmder_t* cmder, xcmd_t* cmds, uint16_t number);

/**
 * @description: 
 * @param {xcmder_t*} cmder：解释器指针
 * @param {xcmd_key_t*} keys：快捷键集
 * @param {uint16_t} number：快捷键的个数
 * @return {int}：已经注册的快捷键的个数
 */
int xcmd_key_register(xcmder_t* cmder, xcmd_key_t* keys, uint16_t number);

/**
 * @description: 手动执行命令
 * @param {xcmder_t} cmder：解释器指针
 * @param {char* } str：命令
 * @return {uint8_t}  返回参数的个数
 */
uint8_t xcmd_exec(xcmder_t *cmder, char *str);

/**
 * @description: 打印字符串
 * @param {xcmder_t *} cmder：解释器指针
 * @param {char*} str
 * @return 无
 */
void xcmd_print(xcmder_t * cmder, const char *fmt, ...);


/**
 * @description: 向显示器插入一个字符
 * @param {xcmder_t} *cmder
 * @param {char} c
 * @return 无
 */
void xcmd_display_insert_char(xcmder_t *cmder, char c);

/**
 * @description: 删除显示器的一个字符
 * @param {xcmder_t} *cmder
 * @return 无
 */
void xcmd_display_delete_char(xcmder_t *cmder);

/**
 * @description: 清除显示器
 * @param {xcmder_t} *cmder
 * @return 无
 */
void xcmd_display_clear(xcmder_t *cmder);

/**
 * @description: 获取显示器的内容
 * @param {xcmder_t} *cmder
 * @return {char*} *显示器的内容的指针
 */
char* xcmd_display_get(xcmder_t *cmder);

/**
 * @description: 获取历史记录的个数
 * @param {xcmder_t} *cmder
 * @return {uint16_t} 已经记录的历史个数
 */
uint16_t xcmd_history_len(xcmder_t *cmder);

/**
 * @description: 插入一条历史记录
 * @param {xcmder_t} *cmder
 * @param {char*} str
 * @return 无
 */
void xcmd_history_insert(xcmder_t *cmder, char* str);

/**
 * @description: 获取下一条历史记录
 * @param {xcmder_t*} cmder
 * @return 历史命令
 */
char *xcmd_history_next(xcmder_t* cmder);

/**
 * @description: 获取上条历史记录
 * @param {xcmder_t*} cmder
 * @return 历史命令
 */
char *xcmd_history_prev(xcmder_t *cmder);

/**
 * @description: 获取当前历史记录
 * @param {xcmder_t*} cmder
 * @return 历史命令
 */
char *xcmd_history_current(xcmder_t *cmder);

/**
 * @description: 将历史记录指针指向头部
 * @param {xcmder_t*} cmder
 * @return 无
 */
void  xcmd_history_reset(xcmder_t *cmder);
#endif /*XCMD_H*/
