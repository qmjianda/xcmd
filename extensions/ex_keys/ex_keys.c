#include "xcmd.h"
#include "ex_keys.h"

#define IS_ALPHA(c) ( (c>='A' && c<='Z') || (c>='a' && c<='z') )
#define IS_NUMBER(c)    (c>='0' && c<='9')

static int xcmd_ctr_a(int argc, char* argv[])
{
    /* 移动光标到头 */
    xcmder_t *xcmder = XCMD_CURRENT();
    xnr_line_cursor_set(&xcmder->line, 0);
    return 0;
}

static int xcmd_ctr_e(int argc, char* argv[])
{
    /* 移动光标到尾 */
    xcmder_t *xcmder = XCMD_CURRENT();
    xnr_line_cursor_set(&xcmder->line, XCMD_LINE_MAX_LENGTH);
    return 0;
}

static int xcmd_ctr_u(int argc, char* argv[])
{
    /* 删除光标左边的所有字符 */
    xcmder_t *xcmder = XCMD_CURRENT();
    int pos = xnr_line_cursor(&xcmder->line);
    for(int i=0; i<pos; i++)
    {
        xnr_line_delete(&xcmder->line);
    }
    return 0;
}


static int xcmd_ctr_k(int argc, char* argv[])
{
    /* 删除光标右边的所有字符 */
    xcmder_t *xcmder = XCMD_CURRENT();
    int pos = xnr_line_cursor(&xcmder->line);
    xnr_line_cursor_set(&xcmder->line, XCMD_LINE_MAX_LENGTH);
    while(1)
    {
        if(xnr_line_cursor(&xcmder->line) == pos)
        {
            break;
        }
        xnr_line_delete(&xcmder->line);
    }
    return 0;
}

static int xcmd_ctr_l(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    xcmd_exec(xcmder, "clear");
    return 0;
}


static int xcmd_ctr_left(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char *line = xnr_line_line(&xcmder->line);
    uint16_t pos = xnr_line_cursor(&xcmder->line);
    while(pos)
    {
        pos--;
        if(IS_ALPHA(line[pos]) || IS_NUMBER(line[pos]))
        {
            break;
        }
    }

    while(pos)
    {
        if( !IS_ALPHA(line[pos-1]) && !IS_NUMBER(line[pos-1]) )
        {
            break;
        }
        pos--;
    }
    xnr_line_cursor_set(&xcmder->line, pos);
    return 0;
}


static int xcmd_ctr_right(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    char *line = xnr_line_line(&xcmder->line);
    uint16_t pos = xnr_line_cursor(&xcmder->line);
    while(line[pos++])
    {
        if(IS_ALPHA(line[pos]) || IS_NUMBER(line[pos]))
        {
            break;
        }
    }

    while(line[pos++])
    {
        if( !IS_ALPHA(line[pos]) && !IS_NUMBER(line[pos]) )
        {
            break;
        }
    }
    xnr_line_cursor_set(&xcmder->line, pos);
    return 0;
}

XCMD_EXPORT_KEY(KEY_CTR_A, xcmd_ctr_a, "ctr+a")
XCMD_EXPORT_KEY(KEY_CTR_E, xcmd_ctr_e, "ctr+e")
XCMD_EXPORT_KEY(KEY_CTR_U, xcmd_ctr_u, "ctr+u")
XCMD_EXPORT_KEY(KEY_CTR_K, xcmd_ctr_k, "ctr+k")
XCMD_EXPORT_KEY(KEY_CTR_L, xcmd_ctr_l, "ctr+l")
XCMD_EXPORT_KEY(KEY_CTR_LEFT, xcmd_ctr_left, "ctr+lelf")
XCMD_EXPORT_KEY(KEY_CTR_RIGHT, xcmd_ctr_right, "ctr+lelf")

static xcmd_key_t ex_keys[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {KEY_CTR_A, xcmd_ctr_a, "ctr+a", NULL},
    {KEY_CTR_E, xcmd_ctr_e, "ctr+e", NULL},
    {KEY_CTR_U, xcmd_ctr_u, "ctr+u", NULL},
    {KEY_CTR_K, xcmd_ctr_k, "ctr+k", NULL},
    {KEY_CTR_L, xcmd_ctr_l, "ctr+l", NULL},
    {KEY_CTR_LEFT, xcmd_ctr_left, "ctr+lelf", NULL},
    {KEY_CTR_RIGHT, xcmd_ctr_right, "ctr+right", NULL},
#endif
};


void ex_keys_init(void)
{
    xcmd_key_register(ex_keys, sizeof(ex_keys)/sizeof(xcmd_key_t));
}
