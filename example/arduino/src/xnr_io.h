#ifndef XNR_IO_H
#define XNR_IO_H

#include <stdio.h>
#include <stddef.h>
#include "xnr_conf.h"

#ifdef __cplusplus
extern   "C" {
#endif

#ifndef XNR_PRINT_BUF_MAX_LENGTH
#define XNR_PRINT_BUF_MAX_LENGTH (128)
#endif

/* 标准文件描述符.  */
#ifndef STDIN_FILENO
#define	STDIN_FILENO	0	/* 标准输入.  */
#endif
#ifndef STDOUT_FILENO
#define	STDOUT_FILENO	1	/* 标准输出.  */
#endif
#ifndef STDERR_FILENO
#define	STDERR_FILENO	2	/* 标准错误.  */
#endif

typedef int (*io_write_t)(int fd, const char *buf, size_t len);
typedef int (*io_read_t)(int fd, char *buf, size_t len);

typedef struct xnr_io
{
    int in_fd;
    int out_fd;
    io_write_t write;
    io_read_t read;
} xnr_io_t;

/**
 * @description: 初始化
 * @param io：xnr_io实例，write：写函数，read：读函数，in_fd：读fd， out_fd：写fd
 * 
 * @return -1：失败，0：成功
 */
int xnr_io_init(xnr_io_t *io,
              io_write_t write,
              io_read_t read);

/**
 * @description: 设置读写fd
 * @param io：xnr_io实例，in_fd：读编号， out_fd：写编号
 * @return 无
 */
void xnr_io_set_in_fd(xnr_io_t *io, int in_fd);
void xnr_io_set_out_fd(xnr_io_t *io, int out_fd);
int xnr_io_in_fd(xnr_io_t *io);
int xnr_io_out_fd(xnr_io_t *io);

/**
 * @description: 打印字符串
 * @param io：xnr_io实例，fmt：格式化字符串，...：参数
 * @return -1：失败，其他：字符串长度
 */
int xnr_io_printf(xnr_io_t *io, const char *fmt, ...);

/**
 * @description: 打印字符串
 * @param io：xnr_io实例，str：字符串
 * @return -1：失败，其他：字符串长度
 */
int xnr_io_puts(xnr_io_t *io, const char *str);

/**
 * @description: 打印字符
 * @param io：xnr_io实例，c：字符
 * @return -1：失败，其他：成功
 */
int xnr_io_putc(xnr_io_t *io, char c);

/**
 * @description: 读取字符
 * @param io：xnr_io实例
 * @return -1：失败，其他：获取到的字符
 */
int xnr_io_getc(xnr_io_t *io);

#ifdef __cplusplus
        }
#endif

#endif