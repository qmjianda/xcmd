#ifndef XCMD_H
#define XCMD_H

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

typedef void(*cmd_func_t)(int argv, char* argc[]);
typedef int (*xcmd_get_char_func_t)(uint8_t *);
typedef int (*xcmd_put_char_func_t)(uint8_t ch);

typedef struct __cmd
{
    char* name;
    cmd_func_t func;
    char* help;
    struct  __cmd *next;
}xcmd_t;

typedef struct
{
    struct
    {
        int (*get_c)(uint8_t*);
        int (*put_c)(uint8_t);
    }io;

    struct
    {
        uint16_t index;   /* 在pool的第几行 */
        uint16_t cur_len; /* 当前行的字符长度 */
        uint16_t cursor;  /* 光标所在位置 */
    }display;

    struct 
    {
        char *buf;
        uint16_t line_len; /* 每一行的最大长度 */
        uint16_t line_num; /* 一共有多少行 */
    }pool;
    
    struct
    {
        char *buf;
        uint16_t paramLen;
        uint16_t historyLen;
        uint16_t bufLen;

        uint16_t historyNum;
        uint16_t bytesNum;

        uint16_t historyCursor;
        uint16_t cursor;
        uint8_t ishistory;
    } parser;

    struct 
    {
        uint16_t len;
        xcmd_t *next;
        xcmd_t *tail;
    }cmd_list;

    uint8_t encodeCaseStu;
    uint8_t _initOK;
} xcmder_t;

int xcmd_register(xcmder_t *cmder, xcmd_t *cmds, uint16_t number);
void xcmd_match(xcmder_t *cmder, int argc, char*argv[]);
void xcmd_print_list(xcmder_t *cmder);
uint8_t xcmd_exec(xcmder_t *cmder, char *str);
void xcmd_destory(xcmder_t* cmder);
xcmder_t *xcmd_create( int (*get_c)(uint8_t*), int (*put_c)(uint8_t), uint16_t cmdLen, uint16_t historyLen, uint16_t paramLen);
#define xcmd_create_default(getchar, putchar)  xcmd_create(getchar,  putchar, 64, 10, 16);
void xcmd_task(xcmder_t *cmder);

#endif /*XCMD_H*/
