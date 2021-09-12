#include "xcmd_default_keys.h"

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
    xcmder_t *cmder = (xcmder_t*)pv;
	xcmd_display_delete_char(cmder);
    return 0;
}

static int xcmd_cursor_left(void *pv)
{
    xcmder_t *cmder = (xcmder_t*)pv;
	if(cmder->parser.cursor > 0)
	{
		cmder->parser.cursor--;
        xcmd_print_str(cmder, STR_LEFT);
	}
    return 0;
}

static int xcmd_cursor_right(void *pv)
{
    xcmder_t *cmder = (xcmder_t*)pv;
	if(cmder->parser.cursor < cmder->parser.byte_num)
	{
		cmder->parser.cursor++;
        xcmd_print_str(cmder, STR_RIGHT);
	}
    return 0;
}

static int xcmd_history_dw(void *pv)
{
    xcmder_t *cmder = (xcmder_t*)pv;
    char *line = xcmd_history_prev(cmder);
    char *display_line = xcmd_display_get(cmder);
    if(line)
    {
        xcmd_display_clear(cmder);
        uint16_t len = strlen(line);
        if (len)
        {
            strncpy(display_line, line, cmder->parser.line_len);
            xcmd_print_str(cmder, line);
            cmder->parser.byte_num = len;
            cmder->parser.cursor = len;
        }
    }
    else
    {
        xcmd_display_clear(cmder);
    }
    return 0;
}

static int xcmd_history_up(void *pv)
{
    xcmder_t *cmder = (xcmder_t*)pv;
    char *line = xcmd_history_next(cmder);
    char *display_line = xcmd_display_get(cmder);
    if(line)
    {
        xcmd_display_clear(cmder);
        uint16_t len = strlen(line);
        if (len)
        {
            strncpy(display_line, line, cmder->parser.line_len);
            xcmd_print_str(cmder, line);
            cmder->parser.byte_num = len;
            cmder->parser.cursor = len;
        }
    }
    return 0;
}

static xcmd_key_t default_keys[] = 
{
    {L_DELETE, xcmd_del_char,  NULL},
    {LEFT, xcmd_cursor_left, NULL},
    {RIGHT, xcmd_cursor_right, NULL},
    {DW, xcmd_history_dw, NULL},
    {UP, xcmd_history_up, NULL},
};

void default_keys_init(xcmder_t *cmder)
{
    xcmd_key_register(cmder, default_keys, sizeof(default_keys)/sizeof(xcmd_key_t));
}