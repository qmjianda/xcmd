#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "xnr_var.h"

int xnr_var_init(xnr_var_tab_t *tab, xnr_var_t *var_buf, int num)
{
    if (NULL == tab || NULL == var_buf || 0 == num)
        return -1;
    tab->capacity = num;
    tab->var_buf = var_buf;
    memset(var_buf, 0, sizeof(xnr_var_t) * num);
    return 0;
}

int xnr_var_set(xnr_var_tab_t *tab, char *name, char *value)
{
    if (NULL == tab ||
        NULL == name ||
        NULL == value ||
        strlen(name) == 0 || strlen(name) >= XNR_VAR_NAME_LEN ||
        strlen(value) == 0 || strlen(value) >= XNR_VAR_VAR_LEN)
        return -1;
    int var_id = -1;                        // 保存需要设置的槽位
    int var_empty_id = -1;                  // 保存空闲槽位
    for (int i = 0; i < tab->capacity; i++) // 查找name是否存在
    {
        if ((-1 == var_empty_id) && (tab->var_buf[i].name[0] == '\0'))
        {
            var_empty_id = i;
        }
        else
        {
            if (strcmp(tab->var_buf[i].name, name) == 0)
            {
                var_id = i;
                break;
            }
        }
    }
    if (-1 == var_id) // 获取一个空闲的var槽位
        var_id = var_empty_id;

    if (-1 == var_id) // 没有空余槽位
        return -1;

    if (strcmp(value, "nil") == 0) // 删除变量
    {
        tab->var_buf[var_id].name[0] = '\0';
        return 0;
    }
    else
    {
        strncpy(tab->var_buf[var_id].value, value, XNR_VAR_VAR_LEN);
        strncpy(tab->var_buf[var_id].name, name, XNR_VAR_NAME_LEN);
        tab->var_buf[var_id].value[XNR_VAR_VAR_LEN - 1] = '\0';
        tab->var_buf[var_id].name[XNR_VAR_NAME_LEN - 1] = '\0';
    }
    return var_id;
}

char *xnr_var_value(xnr_var_tab_t *tab, char *name)
{
    if (NULL == tab || NULL == name)
        return "nil";
    xnr_var_t *var = NULL;
    for (int i = 0; i < tab->capacity; i++) // 查找name是否存在
    {
        if (strcmp(tab->var_buf[i].name, name) == 0)
        {
            var = &tab->var_buf[i];
        }
    }
    if (NULL == var)
        return "nil";
    else
        return var->value;
}

xnr_var_t *xnr_var(xnr_var_tab_t *tab, int id)
{
    if (NULL == tab || (id >= tab->capacity))
        return NULL;
    return &tab->var_buf[id];
}

int xnr_var_repalce(xnr_var_tab_t *tab, char *in, char *out, int out_len)
{
    if (NULL == tab || NULL == in || out == NULL || 0 == out_len)
        return -1;
    int len = strlen(in);
    char name[XNR_VAR_NAME_LEN] = {0};
    int out_i = 0;
    for (unsigned int i = 0; (i < len) && (out_i < (out_len - 1)); i++)
    {
        if (in[i] == '$')
        {
            int k = 0;
            char end = ' ';
            unsigned int j = i + 1;
            if (in[j] == '{')
            {
                end = '}';
                j++;
            }
            while (isprint((int)in[j]) && // 可显示字符
                   in[j] != end      // 变量结尾
            )
            {
                if (k < (XNR_VAR_NAME_LEN - 1))
                {
                    name[k++] = in[j++];
                }
                else
                {
                    return -1; // name长度超限
                }
            }
            name[k] = '\0';
            char *value = xnr_var_value(tab, name); // 获取变量的值
            if (value != NULL)
            {
                int val_len = strlen(value);
                for (k = 0; (k < val_len) && (out_i < (out_len - 1)); k++)
                {
                    out[out_i++] = value[k];
                }
            }
            if (end == '}')
                i = j;
            else
                i = j - 1;
        }
        else
        {
            out[out_i++] = in[i];
        }
    }
    out[out_i] = '\0';
    return out_i;
}