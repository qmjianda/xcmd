/*
 * @Author: your name
 * @Date: 2021-09-15 00:11:50
 * @LastEditTime: 2021-10-12 14:58:23
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/src/xcmd_default_keys.c
 */
#include "xcmd_confg.h"
#include "xcmd.h"
#include "xcmd_default_keys.h"

static int xcmd_str_match(const char* str1, const char* str2)
{
    int i=0;
    for(i=0; str1[i] && str2[i]; i++)
    {
        if(str1[i] != str2[i])
        {
            break;
        }
    }
    return i;
}

static int xcmd_del_char(void *pv)
{
	xcmd_display_delete_char();
    return 0;
}

static int xcmd_enter(void *pv)
{
    char *cmd = xcmd_display_line_end();
    xcmd_print("\n\r");
    if(cmd[0])
    {
        xcmd_exec(cmd);
        cmd[0] = '\0';
    }
#ifndef XCMD_DEFAULT_PROMPT_CLOLR
        xcmd_print("%s", xcmd_get_prompt());
#else
        xcmd_print(XCMD_DEFAULT_PROMPT_CLOLR "%s" TX_DEF, xcmd_get_prompt());
#endif
    
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

#if XCMD_HISTORY_MAX_NUM
static int xcmd_history_dw(void *pv)
{
    char *line = xcmd_history_prev();
    if(line)
    {
        xcmd_display_print(line);
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
        xcmd_display_print(line);
    }
    return 0;
}
#endif

static int xcmd_auto_completion(void *pv)
{
    xcmd_t *match_cmd_first = NULL;
    uint16_t match_num = 0;
    uint16_t match_subscript_min = 0;
    xcmd_t *p = xcmd_cmdlist_get();
    char *display_line = xcmd_display_get();
    uint16_t cursor_pos = xcmd_display_cursor_get();
    while(p)
    {
        if(strncmp(display_line, p->name, cursor_pos) == 0)
        {
            if(match_num == 0)
            {
                match_cmd_first = p;
                match_subscript_min = strlen(p->name);
            }
            else if(match_num == 1)
            {
                xcmd_print("\r\n%-15s%-15s", match_cmd_first->name, p->name);
            }
            else
            {
                xcmd_print("%-15s", p->name);
                if((match_num%4) == 0)
                {
                    xcmd_print("\r\n");
                }
            }
            uint16_t subscript = xcmd_str_match(match_cmd_first->name, p->name);
            if( subscript < match_subscript_min)
            {
                match_subscript_min = subscript;
            }
            match_num++;
        }
        p = p->next;
    }

    if(match_num == 1)
    {
        xcmd_display_print("%s ", match_cmd_first->name);
    }
    else if(match_num > 1)
    {
        xcmd_print("\r\n");
        xcmd_display_write(match_cmd_first->name, match_subscript_min);
    }
    return 0;
}

static xcmd_key_t default_keys[] = 
{
    {KEY_CTR_M,     xcmd_enter,             "enter", NULL},
    {KEY_CTR_J,     xcmd_enter,             "enter", NULL},
    {KEY_CTR_H,     xcmd_del_char,          "backspace", NULL},
    {KEY_BACKSPACE, xcmd_del_char,          "delete", NULL},
    {KEY_LEFT,      xcmd_cursor_left,       "left", NULL},
    {KEY_RIGHT,     xcmd_cursor_right,      "right", NULL},
    {KEY_TAB,       xcmd_auto_completion,   "tab", NULL},
#if XCMD_HISTORY_MAX_NUM
    {KEY_DW,        xcmd_history_dw,        "down", NULL},
    {KEY_UP,        xcmd_history_up,        "up", NULL},
#endif
};

void default_keys_init(void)
{
    xcmd_key_register(default_keys, sizeof(default_keys)/sizeof(xcmd_key_t));
}
