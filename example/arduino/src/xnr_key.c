#include "xnr_key.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>


#define _IS_END_KEY(c) ((((c) >= 'A') && ((c) <= 'D')) || (((c) >= 'P') && ((c) <= 'S')) || \
                        ((c) == '~') || ((c) == 'H') || ((c) == 'F'))

char *xnr_key_val(xnr_key_t *key)
{
    if(NULL == key)
        return NULL;
    return key->encode_buf;
}

int xnr_key_encode(xnr_key_t *key, int ch)
{
    if((NULL == key) || (-1 == ch))
        return 0;
    int ret = 0;
    switch (key->case_stu)
    {
    case 0: //开始状态
        key->encode_cnt = 0;
        if (ch == 0x1B) //输入的字符是 ESC
        {
            key->encode_buf[key->encode_cnt++] = ch;
            key->case_stu = 1;
        }
        else
        {
            key->encode_buf[key->encode_cnt++] = ch;
            key->encode_buf[key->encode_cnt] = '\0';
            key->encode_cnt = 0;
            ret = 1;
        }
        break;
    case 1:
        if (_IS_END_KEY(ch))
        {
            key->encode_buf[key->encode_cnt++] = ch;
            key->encode_buf[key->encode_cnt] = '\0';
            ret = key->encode_cnt;
            key->case_stu = 0;
        }
        else
        {
            key->encode_buf[key->encode_cnt++] = ch;
            if (key->encode_cnt >= 6)
            {
                key->case_stu = 0;
                ret = 0;
            }
        }
        break;
    default:
        break;
    }
    return ret;
}

int xnr_key_init(xnr_key_t *key)
{
    if (NULL == key)
        return -1;
    key->case_stu = 0;
    key->encode_cnt = 0;
    return 0;
}