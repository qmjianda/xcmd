#include "xnr_line.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

/**
 * 在字符串s中查找任意一个属于字符串delim中的字符，
 * 如果找到了，返回该字符在s中的位置，同时忽略双引号
 * 中的内容
 */
static char *_strpbrk(char *s, const char *delim) // 返回s1中第一个满足条件的字符的指针, 并且保留""号内的源格式
{
    char flag = 0; // 用来标记是否在双引号内部，0表示否，1表示是
    for (int i = 0; s[i]; i++)
    {
        if (s[i] == '\"')
        { // 如果当前位置为双引号
            // 从字符串中去掉双引号
            for (int k = i; s[k]; k++)
            {
                s[k] = s[k + 1];
            }
            // 改变flag的值
            flag = !flag;
        }
        else if (flag == 0)
        {
            // 只有不在双引号内部才查找delim中的字符
            for (int j = 0; delim[j]; j++)
            {
                if (s[i] == delim[j])
                {
                    return &s[i]; // 找到了，返回字符位置
                }
            }
        }
    }
    return NULL; // 没找到，返回NULL
}

/**
 * 在字符串s中查找由字符串delim中的字符分隔出来的下一个子串，
 * 如果找到了，返回该子串的头指针，同时更新save_ptr的值，用于
 * 下一次调用时继续切割
 */
char *_strtok(char *s, const char *delim, char **save_ptr)
{
    // 如果s为NULL，则从上一次保存的位置开始切割
    if (s == NULL)
    {
        s = *save_ptr;
    }

    // 跳过连续的分隔符
    s += strspn(s, delim);
    if (*s == '\0')
    {
        // 字符串已经结束
        return NULL;
    }

    // 找到本次切割的子串
    char *token = s;
    s = _strpbrk(token, delim);
    if (s == NULL)
    {
        // 没有找到分隔符，返回剩余的字符串
        *save_ptr = strchr(token, '\0');
    }
    else
    {
        // 找到分隔符，替换为'\0'，并更新save_ptr的值
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}

static int strlen_without_ansi(const char *str)
{
    int len = 0;
    int i = 0;
    while (str[i] != '\0')
    {
        if (str[i] == '\x1b')
        {
            // skip ANSI escape sequence
            while (str[i] != 'm' && str[i] != '\0')
            {
                ++i;
            }
            if (str[i] == '\0')
            {
                break;
            }
        }
        else
        {
            ++len;
        }
        ++i;
    }
    return len;
}

int xnr_line_init(xnr_line_t *le, xnr_io_t *io)
{
    if (NULL == le || NULL == io)
        return -1;
    le->length = 0;
    le->cursor = 0;
    le->prompt = ANSI_COLOR_TXT(ANSI_BLUE, "->");
    le->line[0] = '\0';
    le->io = io;
    return 0;
}

int xnr_line_bind_prompt(xnr_line_t *le, const char *prompt)
{
    if (NULL == le || NULL == prompt)
        return -1;
    le->prompt = prompt;
    return 0;
}

const char *xnr_line_prompt(xnr_line_t *le)
{
    if (NULL == le)
        return NULL;
    return le->prompt;
}

int xnr_line_insert(xnr_line_t *le, char c)
{
    if (NULL == le)
        return -1;

    if (le->length < XNR_LINE_MAX_LENGTH - 1) // 需要保留一个位置存放'\0'
    {
        for (int i = le->length; i > le->cursor; i--)
        {
            le->line[i] = le->line[i - 1];
        }
        ++le->length;
        le->line[le->length] = '\0';
        le->line[le->cursor++] = c;
        xnr_io_printf(le->io, ICH(1));  // 在当前光标位置插入空格
        xnr_io_printf(le->io, "%c", c); // 将空格替换成字符
        return 0;
    }
    return -1;
}

int xnr_line_delete(xnr_line_t *le)
{
    if (NULL == le)
        return -1;
    if (le->cursor > 0)
    {
        for (int i = le->cursor - 1; i < le->length - 1; i++)
        {
            le->line[i] = le->line[i + 1];
        }
        --le->length;
        --le->cursor;
        le->line[le->length] = '\0';
        xnr_io_printf(le->io, CUB(1));
        xnr_io_printf(le->io, DCH(1));
        return 0;
    }
    return -1;
}

char *xnr_line_line(xnr_line_t *le)
{
    if (NULL == le)
        return NULL;
    return le->line;
}

int xnr_line_cursor(xnr_line_t *le)
{
    if (NULL == le)
        return 0;
    return le->cursor;
}

int xnr_line_length(xnr_line_t *le) // 不包含'\0'
{
    if (NULL == le)
        return 0;
    return le->length;
}

int xnr_line_cursor_left(xnr_line_t *le)
{
    if (NULL == le)
        return 0;
    int pos = xnr_line_cursor(le);
    if (pos > 0)
        xnr_line_cursor_set(le, --pos);
    return pos;
}

int xnr_line_cursor_right(xnr_line_t *le)
{
    if (NULL == le)
        return 0;
    int pos = xnr_line_cursor(le);
    if (pos < xnr_line_length(le))
        xnr_line_cursor_set(le, ++pos);
    return pos;
}

int xnr_line_cursor_set(xnr_line_t *le, int pos)
{
    if (NULL == le)
        return 0;
    if (pos > le->length)
    {
        pos = le->length;
    }
    le->cursor = pos;
    pos = le->cursor + strlen_without_ansi(le->prompt) + 1;
    xnr_io_printf(le->io, CHA(pos));
    return pos;
}

void xnr_line_cls(xnr_line_t *le) // 仅清除屏幕显示，clear时也会显示prompt
{
    if (NULL == le)
        return;
    xnr_io_printf(le->io, DL(0));
    xnr_io_printf(le->io, "%s", le->prompt);
    le->length = 0;
    le->cursor = 0;
}

void xnr_line_clear(xnr_line_t *le) // 仅清除屏幕显示和le buffer， clear时也会显示prompt，所以建议新行后都调用此函数
{
    if (NULL == le)
        return;
    xnr_line_cls(le);
    le->line[0] = '\0';
}

char xnr_line_current_char(xnr_line_t *le)
{
    if (NULL == le)
        return '\0';
    if (le->cursor < le->length)
    {
        return le->line[le->cursor];
    }
    return '\0';
}

void xnr_line_write(xnr_line_t *le, const char *buf, int len)
{
    if (NULL == le)
        return;
    if (len > XNR_LINE_MAX_LENGTH - 1)
    {
        len = XNR_LINE_MAX_LENGTH - 1;
    }
    for (int i = 0; i < len; i++)
    {
        xnr_line_insert(le, buf[i]);
    }
}

void xnr_line_puts(xnr_line_t *le, const char *str)
{
    if (NULL == le)
        return;
    xnr_line_write(le, str, strlen(str));
}

void xnr_line_print(xnr_line_t *le, const char *fmt, ...)
{
    if (NULL == le)
        return;
    char ucstring[XNR_LINE_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(ucstring, XNR_LINE_MAX_LENGTH, fmt, arg);
    va_end(arg);
    xnr_line_write(le, ucstring, strlen(ucstring));
}

int xnr_line_tok(char *msg, char *delim, char *get[], int max_num)
{
    int i, ret;
    char *ptr = NULL;
    char *sp = NULL;
    ptr = _strtok(msg, delim, &sp);
    for (i = 0; ptr != NULL && i < max_num; i++)
    {
        get[i] = ptr;
        ptr = _strtok(NULL, delim, &sp);
    }
    ret = i;
    return ret;
}