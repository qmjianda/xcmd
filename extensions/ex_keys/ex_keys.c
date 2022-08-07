#include "xcmd.h"
#include "ex_keys.h"

#define IS_ALPHA(c) ( (c>='A' && c<='Z') || (c>='a' && c<='z') )
#define IS_NUMBER(c)    (c>='0' && c<='9')

static int xcmd_ctr_a(void *pv)
{
    /* 移动光标到头 */
    xcmd_display_cursor_set(0);
    return 0;
}

static int xcmd_ctr_e(void *pv)
{
    /* 移动光标到尾 */
    xcmd_display_cursor_set(-1);
    return 0;
}

static int xcmd_ctr_u(void *pv)
{
    /* 删除光标左边的所有字符 */
    uint16_t pos = xcmd_display_cursor_get();
    for(uint16_t i=0; i<pos; i++)
    {
        xcmd_display_delete_char();
    }
    return 0;
}


static int xcmd_ctr_k(void *pv)
{
    /* 删除光标右边的所有字符 */
    uint16_t pos = xcmd_display_cursor_get();
    xcmd_display_cursor_set(-1);
    while(1)
    {
        if(xcmd_display_cursor_get() == pos)
        {
            break;
        }
        xcmd_display_delete_char();
    }
    return 0;
}

static int xcmd_ctr_l(void *pv)
{
    xcmd_exec("clear");
    return 0;
}


static int xcmd_ctr_left(void *pv)
{
    char *line = xcmd_display_get();
    uint16_t pos = xcmd_display_cursor_get();
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
    xcmd_display_cursor_set(pos);
    return 0;
}


static int xcmd_ctr_right(void *pv)
{
    char *line = xcmd_display_get();
    uint16_t pos = xcmd_display_cursor_get();
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
    xcmd_display_cursor_set(pos);
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
