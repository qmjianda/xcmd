/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-09-16 23:00:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_keys.c
 */
#include "../inc/xcmd_default_keys.h"
#include "xcmd_confg.h"
#include "xcmd.h"

#define STR_UP     "\x1B\x5B\x41"
#define STR_DW     "\x1B\x5B\x42"
#define STR_RIGHT  "\x1B\x5B\x43"
#define STR_LEFT   "\x1B\x5B\x44"
#define STR_END    "\x1B\x5B\x46"
#define STR_HOME   "\x1B\x5B\x48"
#define STR_INSERT "\x1B\x5B\x32\x7E"
#define STR_DELETE "\x1B\x5B\x33\x7E"
#define STR_PGUP   "\x1B\x5B\x35\x7E"
#define STR_PGDW   "\x1B\x5B\x36\x7E"

static int xcmd_del_char(void *pv)
{
	xcmd_display_delete_char();
    return 0;
}

static int xcmd_cursor_left(void *pv)
{
    uint16_t pos = xcmd_display_cursor_get();
	if(pos > 0)
	{
		pos--;
        xcmd_display_cursor_set(pos);
	}
    return 0;
}

static int xcmd_cursor_right(void *pv)
{
    uint16_t pos = xcmd_display_cursor_get();
    pos++;
    xcmd_display_cursor_set(pos);
    return 0;
}

static int xcmd_history_dw(void *pv)
{
    char *line = xcmd_history_prev();
    char *display_line = xcmd_display_get();
    if(line)
    {
        xcmd_display_set(line);
    }
    else
    {
        xcmd_display_clear();
    }
    return 0;
}

static int xcmd_history_up(void *pv)
{
    char *line = xcmd_history_next();
    char *display_line = xcmd_display_get();
    if(line)
    {
        xcmd_display_set(line);
    }
    return 0;
}

static int xcmd_auto_completion(void *pv)
{
}


static xcmd_key_t default_keys[] = 
{
    {BACKSPACE,     xcmd_del_char,          NULL},
    {L_DELETE,      xcmd_del_char,          NULL},
    {LEFT,          xcmd_cursor_left,       NULL},
    {RIGHT,         xcmd_cursor_right,      NULL},
    {DW,            xcmd_history_dw,        NULL},
    {UP,            xcmd_history_up,        NULL},
    {TAB,           xcmd_auto_completion,   NULL},
};

void default_keys_init(void)
{
    xcmd_key_register(default_keys, sizeof(default_keys)/sizeof(xcmd_key_t));
}