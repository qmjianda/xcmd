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
}cmd_t;

typedef struct
{
    xcmd_get_char_func_t getchar;
    xcmd_put_char_func_t putchar;
    struct
    {
        char *buf;
        char **param;
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
        cmd_t *next;
    }cmd_list;

    uint8_t encodeCaseStu;
    uint8_t _initOK;
} xcmder_t;

int xcmd_register(xcmder_t *cmder, cmd_t *cmds, uint16_t number);
void xcmd_match(xcmder_t *cmder, int argc, char*argv[]);
void xcmd_print_list(xcmder_t *cmder);
uint8_t xcmd_exec(xcmder_t *cmder, char *str);
void xcmd_destory(xcmder_t* cmder);
void xcmd_task(xcmder_t *cmder);
xcmder_t *xcmd_create(
    xcmd_get_char_func_t getchar,
    xcmd_put_char_func_t putchar,
    uint16_t cmdLen,
    uint16_t historyLen,
    uint16_t paramLen);

#define xcmd_create_default(getchar, putchar)  \
    xcmd_create((xcmd_get_char_func_t)getchar, \
                (xcmd_put_char_func_t)putchar, \
                64,                            \
                10,                            \
                16);

#endif /*XCMD_H*/
