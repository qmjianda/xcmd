/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-09-17 23:18:12
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_keys.c
 */
#include "../inc/xcmd_default_keys.h"
#include "../inc/xcmd_confg.h"
#include "../inc/xcmd.h"

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
    if(line)
    {
        xcmd_display_set(line);
    }
    return 0;
}

static int xcmd_auto_completion(void *pv)
{
    return 0;
}


static xcmd_key_t default_keys[] = 
{
    {BACKSPACE,     xcmd_del_char,          NULL},
    {L_DELETE,      xcmd_del_char,          NULL},
    {LEFT,          xcmd_cursor_left,       NULL},
    {RIGHT,         xcmd_cursor_right,      NULL},
    {TAB,           xcmd_auto_completion,   NULL},
#if XCMD_HISTORY_MAX_NUM
    {DW,            xcmd_history_dw,        NULL},
    {UP,            xcmd_history_up,        NULL},
#endif
};

void default_keys_init(void)
{
    xcmd_key_register(default_keys, sizeof(default_keys)/sizeof(xcmd_key_t));
}