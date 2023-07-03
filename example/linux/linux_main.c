#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xcmd_platform.h"
#include "xcmd.h"
#include "linux_cmds.h"
#include "linux_keys.h"
#include "test.h"
#include "ex_keys.h"
#include "ex_cmds.h"
#include "ex_list.h"
#include "socket_cmds.h"

int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;

    if (tcgetattr(fd, &tm) < 0)
    { // 保存现在的终端设置
        return -1;
    }
    tm_old = tm;
    cfmakeraw(&tm); // 更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    { // 设置上更改之后的设置
        return -1;
    }

    ch = getchar();
    // xcmd_print("%d\n", ch);
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    { // 更改设置为最初的样子
        return -1;
    }

    return ch;
}

static int io_write(int fd, const char *buf, size_t len)
{
    int ret = write(fd, buf, len);
    return ret;
}

static int io_read(int fd, char *buf, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        buf[i] = getch();
    }
    return len;
}

static int pre_cmd(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    for (int i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if (i < argc - 1)
            {
                int fd = open(argv[i + 1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                xnr_io_set_out_fd(&xcmder->io, fd);
            }
            return i;
        }
    }
    return argc;
}

static int after_cmd(int argc, char *argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    int fd = xnr_io_out_fd(&xcmder->io);
    if (STDOUT_FILENO != fd)
    {
        close(fd);
        xnr_io_set_out_fd(&xcmder->io, STDOUT_FILENO);
    }
    return argc;
}

int main(void)
{
    xcmder_t xcmder;
    printf("xcmd_t = %ld\r\n", sizeof(xcmd_t));
    xcmd_init(&xcmder, io_write, io_read);
    xcmd_reg_pre_cmd_cbk(&xcmder, pre_cmd);
    xcmd_reg_after_cmd_cbk(&xcmder, after_cmd);
    linux_keys_init();
    linux_cmds_init();
    test_cmd_init();
    test_keys_init();
    ex_keys_init();
    ex_cmds_init();
    socket_cmds_init();
    ex_list_init();
    xcmd_exec(&xcmder, "cd .");
    while (1)
    {
        xcmd_task(&xcmder);
    }
}