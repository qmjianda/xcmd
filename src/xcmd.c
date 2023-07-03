#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "xcmd.h"
#include "xcmd_cmds.h"
#include "xcmd_keys.h"

#ifndef ENABLE_XCMD_EXPORT
static struct
{
    xcmd_t head;
} g_cmd_list;

static struct
{
    xcmd_key_t head;
} g_key_list;
#endif

int xcmd_key_register(xcmd_key_t *keys, uint16_t number)
{
#ifndef ENABLE_XCMD_EXPORT
    uint16_t i = 0;
    xcmd_key_t *temp;

    while (i < number)
    {
        temp = g_key_list.head.next;
        g_key_list.head.next = &keys[i];
        keys[i].next = temp;
        ++i;
    }
#endif
    return 0;
}

int xcmd_cmd_register(xcmd_t *cmds, uint16_t number)
{
#ifndef ENABLE_XCMD_EXPORT
    xcmd_t *temp;
    uint16_t i = 0;
    while (i < number)
    {
        temp = g_cmd_list.head.next;
        g_cmd_list.head.next = &cmds[i];
        cmds[i].next = temp;
        ++i;
    }
#endif
    return 0;
}

int xcmd_unregister_key(char *key)
{
#ifndef ENABLE_XCMD_EXPORT
    xcmd_key_t *bk = &g_key_list.head;
    xcmd_key_t *p = NULL;
    XCMD_KEY_FOR_EACH(p)
    {
        if (strcmp(key, p->name) == 0)
        {
            bk->next = p->next;
            return 0;
        }
        bk = p;
    }
#endif
    return -1;
}

int xcmd_unregister_cmd(char *cmd)
{
#ifndef ENABLE_XCMD_EXPORT
    xcmd_t *bk = &g_cmd_list.head;
    xcmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
    {
        if (strcmp(cmd, p->name) == 0)
        {
            bk->next = p->next;
            return 0;
        }
        bk = p;
    }
#endif
    return -1;
}

xcmd_key_t *xcmd_keylist_get(void)
{
#ifndef ENABLE_XCMD_EXPORT
    return g_key_list.head.next;
#else
    return &_xcmd_key_list_start;
#endif
}

xcmd_t *xcmd_cmdlist_get(void)
{
#ifndef ENABLE_XCMD_EXPORT
    return g_cmd_list.head.next;
#else
    return &_xcmd_cmd_list_start;
#endif
}

static void xcmd_key_match(xcmder_t *xcmder, char *key)
{
    xcmd_key_t *p = NULL;
    XCMD_KEY_FOR_EACH(p)
    {
        if (strcmp(key, p->name) == 0)
        {
            if (p->func(0, (char **)&xcmder) == 0)
            {
                break;
            }
        }
    }
}

static int xcmd_cmd_match(xcmder_t *xcmder, int argc, char *argv[])
{
    uint8_t flag = 0;
    int ret = -1;
    xcmd_t *p = NULL;
    XCMD_CMD_FOR_EACH(p)
    {
        if (strcmp(p->name, argv[0]) == 0)
        {
            flag = 1;
            if (argc > 1)
            {
                if (p->help && ((strcmp(argv[1], "?") == 0) || (strcmp(argv[1], "-h") == 0)))
                {
                    xcmd_print(xcmder, "%s\r\n", p->help);
                    break;
                }
            }
            if (xcmder->pre_cmd_cbk)
            {
                argc = xcmder->pre_cmd_cbk(argc, argv);
                argv[argc] = (char *)xcmder;
            }
            ret = p->func(argc, argv);
            if (xcmder->after_cmd_cbk)
                argc = xcmder->after_cmd_cbk(argc, argv);
            break;
        }
    }
    if (0 == flag)
    {
        if (NULL == xcmder->unknow_cmd_cbk)
            xcmd_print(xcmder, "cmd \"%s\" does not exist\r\n", argv[0]);
        else
            xcmder->unknow_cmd_cbk(argc, argv);
    }
    return ret;
}

int xcmd_exec(xcmder_t *xcmder, char *str)
{
    int param_num = 0;
    int ret = -1;
    char *cmd_param_buff[XCMD_PARAM_MAX_NUM + 1];
    char temp[XNR_LINE_MAX_LENGTH];
    strncpy(temp, str, XNR_LINE_MAX_LENGTH);
    param_num = xnr_line_tok(temp, " ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
    if (param_num > 0)
    {
        cmd_param_buff[param_num] = (char *)xcmder;
        ret = xcmd_cmd_match(xcmder, param_num, cmd_param_buff);
    }
    xnr_line_clear(&xcmder->line);
    return ret;
}

static void xcmd_key_exec(xcmder_t *xcmder, char *key)
{
    xcmd_key_match(xcmder, key);
}

void xcmd_task(xcmder_t *xcmder)
{
    int c;
    if (xcmder->_initOK)
    {
        c = xnr_io_getc(&xcmder->io);
        if (xnr_key_encode(&xcmder->key, c))
        {
            char *key_val = xnr_key_val(&xcmder->key);
            if (isprint((int)key_val[0u])) // 可显示字符
            {
                xnr_line_insert(&xcmder->line, c); // 向行中插入可显示字符
            }
            else // 不可显示字符，即快捷键
            {
                xcmd_key_exec(xcmder, key_val);
            }
        }
    }
}

void xcmd_reg_unknow_cmd_cbk(xcmder_t *xcmder, cmd_func_t cbk)
{
    xcmder->unknow_cmd_cbk = cbk;
}

void xcmd_reg_pre_cmd_cbk(xcmder_t *xcmder, cmd_func_t cbk)
{
    xcmder->pre_cmd_cbk = cbk;
}

void xcmd_reg_after_cmd_cbk(xcmder_t *xcmder, cmd_func_t cbk)
{
    xcmder->after_cmd_cbk = cbk;
}

int xcmd_getopt(xcmder_t *xcmder, int argc, char *const argv[], const char *optstring)
{
    static char *nextc = NULL; // 字符串中下一个选项字符位置
    char *c = NULL;            // 选项字符
    int has_arg = 0;           // 选项是否有参数
    int is_long_opt = 0;       // 是否是长选项
    int opt_long_index = -1;   // 如果是长选项，存储它的下标

    xcmder->optarg = 0;

    if (nextc == NULL || *nextc == '\0')
    {
        if (xcmder->optind >= argc || argv[xcmder->optind][0] != '-' || argv[xcmder->optind][1] == '\0')
        {
            return -1;
        }

        nextc = argv[xcmder->optind] + 1;

        if (*nextc == '-')
        {
            is_long_opt = 1;
            nextc++;
        }

        xcmder->optind++;
    }

    if (is_long_opt)
    {
        // 如果是长选项，查找选项字符串和操作数
        char buf[XCMD_PARAM_MAX_NUM + 1];
        char *p = strchr(nextc, '=');

        if (p != NULL)
        {
            // 选项字符串和操作数之间有等号
            int len = p - nextc;

            if (len > XCMD_PARAM_MAX_NUM)
            {
                return '?';
            }

            strncpy(buf, nextc, len);
            buf[len] = '\0';
            nextc = p + 1;
            xcmder->optarg = nextc;
            nextc = NULL; // xcmder->optarg 后面不会再有操作数
        }
        else
        {
            // 选项字符串后没有等号
            int len = strlen(nextc);

            if (len > XCMD_PARAM_MAX_NUM)
            {
                return '?';
            }

            strncpy(buf, nextc, XCMD_PARAM_MAX_NUM);
            nextc = NULL;
        }

        // 在 optstring 中查找长选项
        int i;
        for (i = 0; optstring[i] != '\0'; i += 2)
        {
            if (optstring[i] == ':' || optstring[i + 1] == ':')
            {
                // 忽略有错误格式的选项（参见实例代码，有两个冒号的应该可以解析出错）
                continue;
            }

            if (strcmp(buf, optstring + i) == 0)
            {
                opt_long_index = i / 2;
                break;
            }
        }
    }
    else
    {
        c = nextc++;
        has_arg = 0;

        if (*nextc == ':')
        {
            has_arg = 1;
            nextc++;
        }

        if (*c == ':' || strchr(optstring, *c) == NULL)
        {
            return '?';
        }
    }

    if (opt_long_index >= 0 || has_arg)
    {
        if (nextc != NULL)
        {
            xcmder->optarg = nextc;
            nextc = NULL;
        }
        else if (xcmder->optind < argc)
        {
            xcmder->optarg = argv[xcmder->optind];
            xcmder->optind++;
        }
        else
        {
            return ':';
        }
    }

    if (opt_long_index >= 0)
    {
        return optstring[opt_long_index * 2];
    }
    else
    {
        return *c;
    }
}

void xcmd_init(xcmder_t *xcmder, io_write_t write, io_read_t read)
{
    if (write && read && xcmder && 0 == xcmder->_initOK)
    {
        xcmder->unknow_cmd_cbk = NULL;
        xcmder->pre_cmd_cbk = NULL;
        xcmder->after_cmd_cbk = NULL;
        xcmder->user_data = NULL;
        xcmder->optarg = NULL;
        xcmder->optind = 1;
#ifndef ENABLE_XCMD_EXPORT
        g_cmd_list.head.next = NULL;
#endif
        xnr_io_init(&xcmder->io, write, read);
        xnr_key_init(&xcmder->key);
        xnr_line_init(&xcmder->line, &xcmder->io);
#if XCMD_HISTORY_BUF_SZIE != 0
        xnr_history_init(&xcmder->history, &xcmder->io);
#endif
#if XCMD_VAR_NUM!=0
        xnr_var_init(&xcmder->var_tab, xcmder->var_buf, XCMD_VAR_NUM);
#endif
#if XCMD_VAR_NUM==0
        char *var_val = ANSI_COLOR_TXT(XCMD_DEFAULT_PROMPT_CLOLR, XCMD_DEFAULT_PROMPT);
#else
        xnr_var_set(&xcmder->var_tab, "prompt", ANSI_COLOR_TXT(XCMD_DEFAULT_PROMPT_CLOLR, XCMD_DEFAULT_PROMPT));
        char *var_val = xnr_var_value(&xcmder->var_tab, "prompt");
#endif
        xnr_line_bind_prompt(&xcmder->line, var_val);
        default_cmds_init();
        default_keys_init();
        xcmd_exec(xcmder, "logo");
        xcmd_unregister_cmd("logo");
        xnr_line_clear(&xcmder->line);
        xcmder->_initOK = 1;
    }
}