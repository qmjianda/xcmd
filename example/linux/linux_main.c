/*
 * @Author: your name
 * @Date: 2021-09-14 23:58:24
 * @LastEditTime: 2021-10-11 20:14:26
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/linux/linux_main.c
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xcmd_platform.h"
#include "xcmd.h"
#include "test.h"
#include "ex_keys.h"
#include "ex_cmds.h"
#include "fs_cmds.h"
#include "socket_cmds.h"
#include "ff.h"

FATFS g_fs_ram;
FATFS g_fs_mmc;
BYTE g_work_ram[FF_MAX_SS]; //一定是一个全局变量
BYTE g_work_mmc[FF_MAX_SS]; //一定是一个全局变量
FIL g_fp;

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

int cmd_get_char(uint8_t *ch)
{
    *ch = getch();
    return 1;
}

int cmd_put_char(uint8_t ch)
{
    putchar(ch);
    return 1;
}

static int key_ctr_c(void *pv)
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

void fatfs_test(char* path)
{
    FRESULT res;
    res = f_open(&g_fp, path, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if(res == FR_OK)
    {
        char buf[128];
        strcpy(buf, "hello world");
        int len = strlen(buf);
        UINT bw;
        UINT br;
        res = f_write(&g_fp, buf, len, &bw);
        memset(buf, 0, len);
        f_lseek(&g_fp,0);
        res = f_read(&g_fp, buf, len, &br);
        printf("test file:%s\r\n", buf);
        f_close(&g_fp);
    }
}

int ram_fatfs_init(void)
{
    FRESULT res;                   //局部变量
    res = f_mount(&g_fs_ram, "0:", 1); //挂载文件系统 ， "1:"就是挂载的设备号为1的设备
    if (res == FR_NO_FILESYSTEM) //FR_NO_FILESYSTEM值为13，表示没有有效的设备
    {
        res = f_mkfs("0:", 0, g_work_ram, sizeof(g_work_ram));
        res = f_mount(NULL, "0:", 1);  //取消文件系统
        res = f_mount(&g_fs_ram, "0:", 1); //挂载文件系统
    }
    
    if(res == FR_OK)
    {
        printf("RAM fs init OK!\r\n");
    }
    else
    {
        printf("RAM fs init ERROR!\r\n");
    }

    return res;
}

int mmc_fatfs_init(void)
{
    FRESULT res;                   //局部变量
    res = f_mount(&g_fs_mmc, "1:", 1); //挂载文件系统 ， "1:"就是挂载的设备号为1的设备
    if (res == FR_NO_FILESYSTEM) //FR_NO_FILESYSTEM值为13，表示没有有效的设备
    {
        res = f_mkfs("1:", 0, g_work_mmc, sizeof(g_work_mmc));
        res = f_mount(NULL, "1:", 1);  //取消文件系统
        res = f_mount(&g_fs_mmc, "1:", 1); //挂载文件系统
    }

    if(res == FR_OK)
    {
        printf("MMC fs init OK!\r\n");
    }
    else
    {
        printf("MMC fs init ERROR!\r\n");
    }
    return res;
}

int main(void)
{
    xcmd_init(cmd_get_char, cmd_put_char);
    ram_fatfs_init();
    mmc_fatfs_init();
    test_cmd_init();
    test_keys_init();
    user_keys_init();
    ex_keys_init();
    ex_cmds_init();
    socket_cmds_init();
    fs_cmds_init();
    linux_cmd_init();
    while (1)
    {
        xcmd_task();
    }
}