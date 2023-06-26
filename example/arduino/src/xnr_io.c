#include <stdarg.h>
#include <string.h>
#include "xnr_io.h"

int xnr_io_init(xnr_io_t *io,
              io_write_t write,
              io_read_t read)
{
    if(NULL == io)
        return -1;
    io->in_fd = STDIN_FILENO;
    io->out_fd = STDOUT_FILENO;
    io->read = read;
    io->write = write;
    return 0;
}

int xnr_io_puts(xnr_io_t *io, const char *str)
{
    if(NULL == io || NULL == str)
        return -1;
    return io->write(io->out_fd, str, strlen(str));
}

int xnr_io_putc(xnr_io_t *io, char c)
{
    if(NULL == io)
        return -1;
    return io->write(io->out_fd, &c, 1);
}

int xnr_io_printf(xnr_io_t *io, const char *fmt, ...)
{
    if(NULL == io || NULL == fmt)
        return -1;
    char ucstring[XNR_PRINT_BUF_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    int len = vsnprintf(ucstring, XNR_PRINT_BUF_MAX_LENGTH, fmt, arg);
    va_end(arg);
    return io->write(io->out_fd, ucstring, len);
}

void xnr_io_set_in_fd(xnr_io_t *io, int in_fd)
{
    if(NULL == io)
        return;
    io->in_fd = in_fd;
}

void xnr_io_set_out_fd(xnr_io_t *io, int out_fd)
{
    if(NULL == io)
        return;
    io->out_fd = out_fd;
}

int xnr_io_in_fd(xnr_io_t *io)
{
    if(NULL == io)
        return -1;
    return io->in_fd;
}

int xnr_io_out_fd(xnr_io_t *io)
{
    if(NULL == io)
        return -1;
    return io->out_fd;
}

int xnr_io_getc(xnr_io_t *io)
{
    if(NULL == io)
        return -1;
    char c;
    if (io->read(io->in_fd, &c, 1))
    {
        return c;
    }
    return -1;
}
