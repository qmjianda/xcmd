#include <stdio.h>
#include <string.h>
#include "xnr_history.h"

int xnr_history_init(xnr_history_t *history, xnr_io_t *io)
{
    if (NULL == history || NULL == io)
        return -1;
    history->head = 0;
    history->tail = 0;
    history->length = 0;
    history->id = 0;
    history->io = io;
    memset(history->buffer, 0, XNR_HISTORY_BUF_SZIE);
    return 0;
}

int xnr_history_clear(xnr_history_t *history)
{
    return xnr_history_init(history, history->io);
}

int xnr_history_length(xnr_history_t *history)
{
    return history->length;
}

int xnr_history_append(xnr_history_t *history, char *cmd)
{
    if ((history == NULL) || (cmd == NULL))
        return -1;
    int len = strlen(cmd) + 1; // 包括'\0'
    if (len == 1 || len > (XNR_HISTORY_BUF_SZIE - 1))
        return -1;

    if (xnr_history_cmp(history, -1, cmd) != 0) // 最近的一次历史记录不同
    {
        for (int i = 0; i < len; i++)
        {
            history->buffer[history->head++] = cmd[i];
            history->head %= XNR_HISTORY_BUF_SZIE;
            if (history->head == history->tail) // 缓存空间不足
            {
                while (1)
                {
                    history->buffer[history->tail++] = '\0';
                    history->tail %= XNR_HISTORY_BUF_SZIE;
                    if (history->buffer[history->tail] == '\0')
                    {
                        ++history->tail;
                        history->tail %= XNR_HISTORY_BUF_SZIE;
                        break;
                    }
                }
                --history->length;
            }
        }
        ++history->length;
    }
    history->id = 0;
    return 0;
}

int xnr_history_pos(xnr_history_t *history, int id)
{
    if ((history == NULL))
        return -1;
    if (history->head == history->tail)
        return -1; // buffer中没有缓存历史命令

    int pos = history->tail;
    if (id < 0)
    {
        id = -id;
        if (id > (history->length)) // 负id从1开始
        {
            return -1; // id超限
        }
        pos = history->head - 1;
        pos = pos < 0 ? (XNR_HISTORY_BUF_SZIE - 1) : pos;
        for (int i = 0; i < id; i++)
        {
            while (1)
            {
                --pos;
                pos = pos < 0 ? (XNR_HISTORY_BUF_SZIE - 1) : pos;
                if (history->buffer[pos] == '\0')
                {
                    break;
                }
            }
        }
        ++pos;
        pos %= XNR_HISTORY_BUF_SZIE;
    }
    else
    {
        if (id > (history->length - 1)) // 正id从0开始
        {
            return -1; // id超限
        }
        for (int i = 0; i < id; i++)
        {
            while (1)
            {
                ++pos;
                pos %= XNR_HISTORY_BUF_SZIE;
                if (history->buffer[pos] == '\0')
                {
                    ++pos;
                    pos %= XNR_HISTORY_BUF_SZIE;
                    break;
                }
            }
        }
    }
    return pos;
}

int xnr_history_get(xnr_history_t *history, int id, char *buf, int buf_len)
{
    if ((history == NULL) || (buf == NULL) || (buf_len == 0))
        return 0;
    int pos = xnr_history_pos(history, id);
    if (pos < 0)
        return 0; // id超限
    for (int i = 0; i < buf_len; i++)
    {
        buf[i] = history->buffer[pos++];
        pos %= XNR_HISTORY_BUF_SZIE;
        if (buf[i] == '\0')
        {
            return i; // 返回字符串的长度
        }
    }
    buf[buf_len - 1] = '\0';
    return -(buf_len - 1); // 由于buf的size小于历史命令，返回字符串的长度
}

int xnr_history_cmp(xnr_history_t *history, int id, char *cmd)
{
    if ((history == NULL) || (cmd == NULL))
        return -1;
    int pos = xnr_history_pos(history, id);
    if (pos < 0)
        return -1; // id超限
    char c;
    while (1)
    {
        c = history->buffer[pos++];
        pos %= XNR_HISTORY_BUF_SZIE;
        if (c && *cmd && (c == *cmd))
        {
            ++cmd;
        }
        else
        {
            break;
        }
    }
    return *cmd - c;
}

int xnr_history_up(xnr_history_t *history, char *buf, int buf_len)
{
    int id = history->id;
    int ret = 0;
    ret = xnr_history_get(history, --id, buf, buf_len);
    if (ret > 0)
    {
        history->id = id;
    }
    return ret;
}

int xnr_history_dw(xnr_history_t *history, char *buf, int buf_len)
{
    int id = history->id;
    int ret = 0;
    if (id >= -1)
    {
        history->id = 0;
        return ret;
    }
    ret = xnr_history_get(history, ++id, buf, buf_len);
    if (ret > 0)
    {
        history->id = id;
    }
    return ret;
}