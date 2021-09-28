/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-09-17 23:18:12
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_keys.c
 */
#include "xcmd_confg.h"
#include "xcmd.h"
#include "xcmd_default_keys.h"

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
    xcmd_t *match_cmd = NULL;
    uint16_t match_num = 0;
    xcmd_t *p = xcmd_cmdlist_get();
    char *display_line = xcmd_display_get();
    char display_backup[XCMD_LINE_MAX_LENGTH];
    strncpy(display_backup, display_line, XCMD_LINE_MAX_LENGTH);
    uint16_t cursor_pos = xcmd_display_cursor_get();
    while(p)
    {
        if(strncmp(display_line, p->name, cursor_pos) == 0)
        {
            if(match_num == 0)
            {
                match_cmd = p;
            }
            else if(match_num == 1)
            {
                xcmd_print("\r\n%-15s%-15s", match_cmd->name, p->name);
            }
            else
            {
                xcmd_print("%-15s", p->name);
                if((match_num%4) == 0)
                {
                    xcmd_print("\r\n");
                }
            }
            match_num++;
        }
        p = p->next;
    }

    if(match_num == 1)
    {
        xcmd_display_set(match_cmd->name);
    }
    else if(match_num > 1)
    {
        xcmd_print("\r\n");
        xcmd_display_set(display_backup);
    }
    return 0;
}


static xcmd_key_t default_keys[] = 
{
    {BACKSPACE,     xcmd_del_char,          "backspace"},
    {L_DELETE,      xcmd_del_char,          "left delete"},
    {LEFT,          xcmd_cursor_left,       "left"},
    {RIGHT,         xcmd_cursor_right,      "right"},
    {TAB,           xcmd_auto_completion,   "tab"},
#if XCMD_HISTORY_MAX_NUM
    {DW,            xcmd_history_dw,        "down"},
    {UP,            xcmd_history_up,        "up"},
#endif
};

void default_keys_init(void)
{
    xcmd_key_register(default_keys, sizeof(default_keys)/sizeof(xcmd_key_t));
}
