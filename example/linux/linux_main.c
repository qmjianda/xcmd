/*
 * @Author: your name
 * @Date: 2021-09-14 23:58:24
 * @LastEditTime: 2021-10-11 20:14:26
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/linux/linux_main.c
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xcmd_platform.h"
#include "xcmd.h"
// #include "test.h"
// #include "ex_keys.h"
// #include "ex_cmds.h"
// #include "ex_list.h"
// #include "fs_cmds.h"
// #include "socket_cmds.h"
// #include "ff.h"
// #include "fatfs_disk_mmc.h"
// #include "fatfs_disk_ram.h"

// FIL g_fp;

int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;

    if (tcgetattr(fd, &tm) < 0)
    { //保存现在的终端设置
        return -1;
    }
    tm_old = tm;
    cfmakeraw(&tm); //更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    { //设置上更改之后的设置
        return -1;
    }

    ch = getchar();
    // xcmd_print("%d\n", ch);
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    { //更改设置为最初的样子
        return -1;
    }

    return ch;
}


int io_write_t(int fd, const char *buf, size_t len)
{
    return write(fd, buf, len);
}

int io_read_t(int fd, char *buf, size_t len)
{
    for (int i = 0; i < len; i++)
    {
        buf[i] = getch();
    }
    return len;
}

static int cmd_ls(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    int fd = xnr_io_out_fd(&xcmder->io);
    if(fd != STDOUT_FILENO)
    {
        // 保存原始的标准输出文件描述符
        int stdout_backup = dup(STDOUT_FILENO);
        dup2(fd, STDOUT_FILENO);
        close(fd);
        system("ls");
        // 恢复标准输出为终端
        dup2(stdout_backup, STDOUT_FILENO);
    }
    else
    {
        system("ls");
    }
    return 0;
}

static int key_ctr_c(int argc, char* argv[])
{
    exit(0);
}

static xcmd_key_t keys[] =
{
        {KEY_CTR_C, key_ctr_c, "ctr+c", NULL},
};

void user_keys_init(void)
{
    xcmd_key_register(keys, sizeof(keys) / sizeof(xcmd_key_t));
}

// void fatfs_test(char* path)
// {
//     FRESULT res;
//     res = f_open(&g_fp, path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
//     if(res == FR_OK)
//     {
//         char buf[128];
//         strcpy(buf, "hello world");
//         int len = strlen(buf);
//         UINT bw;
//         UINT br;
//         res = f_write(&g_fp, buf, len, &bw);
//         memset(buf, 0, len);
//         f_lseek(&g_fp,0);
//         res = f_read(&g_fp, buf, len, &br);
//         printf("test file:%s\r\n", buf);
//         f_close(&g_fp);
//     }
// }

static int pre_cmd(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    for(int i=0; i< argc; i++)
    {
        if(strcmp(argv[i], ">") == 0)
        {
            if(i < argc-1)
            {
                int fd = open(argv[i+1], O_WRONLY | O_TRUNC | O_CREAT, 0644);
                xnr_io_set_out_fd(&xcmder->io, fd);
            }
            return i;
        }
    }
    return argc;
}

static int after_cmd(int argc, char* argv[])
{
    xcmder_t *xcmder = XCMD_CURRENT();
    int fd = xnr_io_out_fd(&xcmder->io);
    if(STDOUT_FILENO != fd)
    {
        close(fd);
        xnr_io_set_out_fd(&xcmder->io, STDOUT_FILENO);
    }
    return argc;
}

#define HELP_LS    ("like linux \"ls\"")

XCMD_EXPORT_CMD(ls, cmd_ls, HELP_LS)

static xcmd_t cmds[] = 
{
#ifndef ENABLE_XCMD_EXPORT
    {"ls", cmd_ls, HELP_LS, NULL},
#endif
};

void linux_cmds_init(void)
{
    xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_t));
}

int main(void)
{
    xcmder_t xcmder;
    xcmd_init(&xcmder, io_write_t, io_read_t);
    xcmd_reg_pre_cmd_cbk(&xcmder, pre_cmd);
    xcmd_reg_after_cmd_cbk(&xcmder, after_cmd);
    user_keys_init();
    linux_cmds_init();
    // ram_disk_init();
    // mmc_disk_init();
    // test_cmd_init();
    // test_keys_init();
    // ex_keys_init();
    // ex_cmds_init();
    // socket_cmds_init();
    // fs_cmds_init();
    // ex_list_init();
    while (1)
    {
        xcmd_task(&xcmder);
    }
}