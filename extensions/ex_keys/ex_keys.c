#include "xcmd.h"
#include "ex_keys.h"

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
    return 0;
}

static xcmd_key_t ex_keys[] = 
{
    {CTR_A, xcmd_ctr_a, "ctr+a"},
    {CTR_E, xcmd_ctr_e, "ctr+e"},
    {CTR_U, xcmd_ctr_u, "ctr+u"},
    {CTR_K, xcmd_ctr_k, "ctr+k"},
    {CTR_L, xcmd_ctr_l, "ctr+l"},
};


void ex_keys_init(void)
{
    xcmd_key_register(ex_keys, sizeof(ex_keys)/sizeof(xcmd_key_t));
}