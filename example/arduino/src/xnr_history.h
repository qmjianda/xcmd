#ifndef XNR_HISTORY_H
#define XNR_HISTORY_H

#include "xnr_conf.h"
#include "xnr_io.h"

#ifdef __cplusplus
extern   "C" {
#endif

#ifndef XNR_HISTORY_BUF_SZIE
#define XNR_HISTORY_BUF_SZIE   (256)
#endif

typedef struct xnr_history //doc/history.drawio
{
    char buffer[XNR_HISTORY_BUF_SZIE];
    int length;
    int head;
    int tail;
    int id;
    xnr_io_t *io;
}xnr_history_t;

/**
 * @description: 初始化
 * @param history：xnr_history实例，io：xnr_io实例
 * @return -1：失败，0：成功
 */
int xnr_history_init(xnr_history_t *history, xnr_io_t *io);

/**
 * @description: 清除历史记录
 * @param history：xnr_history实例
 * @return -1：失败，0：成功
 */
int xnr_history_clear(xnr_history_t *history);

/**
 * @description: 获取历史记录个数
 * @param history：xnr_history实例
 * @return 历史记录个数
 */
int xnr_history_length(xnr_history_t *history);

/**
 * @description: 记录命令
 * @param history：xnr_history实例，cmd：命令
 * @return -1：失败，0：成功
 */
int xnr_history_append(xnr_history_t *history, char* cmd);

/**
 * @description: 获取历史记录编号在历史ringbuffer对应的起始位置
 * @param history：xnr_history实例
 * @return -1：失败，其他：历史记录在ringbuffer对应的起始位置
 */
int xnr_history_pos(xnr_history_t *history, int id);

/**
 * @description: 历史记录与命令比对
 * @param history：xnr_history实例，id：历史记录编号，cmd：命令
 * @return 其他：失败，0：成功
 */
int xnr_history_cmp(xnr_history_t *history, int id, char *cmd);

/**
 * @description: 从历史记录中获取命令
 * @param history：xnr_history实例，id：历史记录编号，buf：命令缓存buf，buf_len：缓存长度
 * @return -1：失败，其他：历史命令长度
 */
int xnr_history_get(xnr_history_t *history, int id, char *buf, int buf_len);

/**
 * @description: 获取上一条历史记录
 * @param history：xnr_history实例，buf：命令缓存buf，buf_len：缓存长度
 * @return -1：失败，其他：历史命令长度
 */
int xnr_history_up(xnr_history_t *history, char *buf, int buf_len);

/**
 * @description: 获取下一条历史记录
 * @param history：xnr_history实例，buf：命令缓存buf，buf_len：缓存长度
 * @return -1：失败，其他：历史命令长度
 */
int xnr_history_dw(xnr_history_t *history, char *buf, int buf_len);

#ifdef __cplusplus
        }
#endif

#endif