#ifndef XCMD_H
#define XCMD_H

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

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

#define  xcmd_create_default(getchar, putchar)  xcmd_create(getchar,  putchar, 64, 10, 16);
xcmder_t *xcmd_create( int (*get_c)(uint8_t*), int (*put_c)(uint8_t), uint16_t cmd_len, uint16_t history_len, uint16_t param_len);
void xcmd_destory(xcmder_t* cmder);
void xcmd_task(xcmder_t *cmder);

int xcmd_register(xcmder_t *cmder, xcmd_t *cmds, uint16_t number);
int xcmd_key_register(xcmder_t *cmder, xcmd_key_t *keys, uint16_t number);

uint8_t xcmd_exec(xcmder_t *cmder, char *str);
void xcmd_print_str(xcmder_t * cmder, char* str);

void xcmd_display_insert_char(xcmder_t *cmder, char c);
void xcmd_display_delete_char(xcmder_t *cmder);
void xcmd_display_clear(xcmder_t *cmder);
char*xcmd_display_get(xcmder_t *cmder);

uint16_t xcmd_history_len(xcmder_t *cmder);
void xcmd_history_insert(xcmder_t *cmder, char* str);
char *xcmd_history_next(xcmder_t *cmder);
char *xcmd_history_prev(xcmder_t *cmder);
char *xcmd_history_current(xcmder_t *cmder);
void  xcmd_history_reset(xcmder_t *cmder);
#endif /*XCMD_H*/
