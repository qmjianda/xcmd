#include "xcmd_confg.h"
#include "xcmd.h"
#include "xcmd_keys.h"

static int xcmd_str_match(const char *str1, const char *str2)
{
    int i = 0;
    for (i = 0; str1[i] && str2[i]; i++)
    {
        if (str1[i] != str2[i])
        {
            break;
        }
    }
    return i;
}

static int xcmd_del_char(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xnr_line_delete(&xcmder->line);
    return 0;
}
XCMD_EXPORT_KEY(KEY_CTR_H, xcmd_del_char, "backspace")
XCMD_EXPORT_KEY(KEY_BACKSPACE, xcmd_del_char, "delete")

static int xcmd_enter(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char out_line[XNR_LINE_MAX_LENGTH] = {0};
    xcmd_print(xcmder, "\n\r");
    char *line = xnr_line_line(&xcmder->line);
#if XCMD_HISTORY_BUF_SZIE != 0
    xnr_history_append(&xcmder->history, line);
#endif
#if XCMD_VAR_NUM!=0
    xnr_var_repalce(&xcmder->var_tab, line, out_line, XNR_LINE_MAX_LENGTH);
#else
    strncpy(out_line, line, XNR_LINE_MAX_LENGTH);
#endif

    if (out_line[0])
    {
        xcmd_exec(xcmder, out_line);
    }
    else
    {
        xnr_line_clear(&xcmder->line);
    }
    return 0;
}
XCMD_EXPORT_KEY(KEY_CTR_M, xcmd_enter, "enter")
XCMD_EXPORT_KEY(KEY_CTR_J, xcmd_enter, "enter")

static int xcmd_cursor_left(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xnr_line_cursor_left(&xcmder->line);
    return 0;
}
XCMD_EXPORT_KEY(KEY_LEFT, xcmd_cursor_left, "left")

static int xcmd_cursor_right(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xnr_line_cursor_right(&xcmder->line);
    return 0;
}
XCMD_EXPORT_KEY(KEY_RIGHT, xcmd_cursor_right, "right")

#if XCMD_HISTORY_BUF_SZIE != 0
static int xcmd_history_dw(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char out_line[XNR_LINE_MAX_LENGTH] = {0};
    if (xnr_history_dw(&xcmder->history, out_line, XNR_LINE_MAX_LENGTH) > 0)
    {
        xnr_line_clear(&xcmder->line);
        xnr_line_puts(&xcmder->line, out_line);
    }
    else
    {
        xnr_line_clear(&xcmder->line);
    }
    return 0;
}
XCMD_EXPORT_KEY(KEY_DW, xcmd_history_dw, "down")

static int xcmd_history_up(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char out_line[XNR_LINE_MAX_LENGTH] = {0};
    if (xnr_history_up(&xcmder->history, out_line, XNR_LINE_MAX_LENGTH) > 0)
    {
        xnr_line_clear(&xcmder->line);
        xnr_line_puts(&xcmder->line, out_line);
    }
    return 0;
}
XCMD_EXPORT_KEY(KEY_UP, xcmd_history_up, "up")
#endif

static int xcmd_auto_completion(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xcmd_t *match_cmd_first = NULL;
    uint16_t match_num = 0;
    uint16_t match_subscript_min = 0;
    char *display_line = xnr_line_line(&xcmder->line);
    uint16_t cursor_pos = xnr_line_cursor(&xcmder->line);
    xcmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
    {
        if (strncmp(display_line, p->name, cursor_pos) == 0)
        {
            if (match_num == 0)
            {
                match_cmd_first = p;
                match_subscript_min = strlen(p->name);
            }
            else if (match_num == 1)
            {
                xcmd_print(xcmder, "\r\n%-15s%-15s", match_cmd_first->name, p->name);
            }
            else
            {
                xcmd_print(xcmder, "%-15s", p->name);
                if ((match_num % 4) == 0)
                {
                    xcmd_print(xcmder, "\r\n");
                }
            }
            uint16_t subscript = xcmd_str_match(match_cmd_first->name, p->name);
            if (subscript < match_subscript_min)
            {
                match_subscript_min = subscript;
            }
            match_num++;
        }
    }

    if (match_num == 1)
    {
        xnr_line_clear(&xcmder->line);
        xnr_line_puts(&xcmder->line, match_cmd_first->name);
    }
    else if (match_num > 1)
    {
        xcmd_print(xcmder, "\r\n");
        xnr_line_clear(&xcmder->line);
        xnr_line_write(&xcmder->line, match_cmd_first->name, match_subscript_min);
    }
    return 0;
}
XCMD_EXPORT_KEY(KEY_TAB, xcmd_auto_completion, "tab")

static xcmd_key_t default_keys[] =
    {
#ifndef ENABLE_XCMD_EXPORT
        {KEY_CTR_M, xcmd_enter, "enter", NULL},
        {KEY_CTR_J, xcmd_enter, "enter", NULL},
        {KEY_CTR_H, xcmd_del_char, "backspace", NULL},
        {KEY_BACKSPACE, xcmd_del_char, "delete", NULL},
        {KEY_LEFT, xcmd_cursor_left, "left", NULL},
        {KEY_RIGHT, xcmd_cursor_right, "right", NULL},
        {KEY_TAB, xcmd_auto_completion, "tab", NULL},
#if XCMD_HISTORY_BUF_SZIE != 0
        {KEY_DW, xcmd_history_dw, "down", NULL},
        {KEY_UP, xcmd_history_up, "up", NULL},
#endif
#endif
};

void default_keys_init(void)
{
    xcmd_key_register(default_keys, sizeof(default_keys) / sizeof(xcmd_key_t));
}
