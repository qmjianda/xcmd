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
#include "ex_list.h"
#include "fs_cmds.h"
#include "socket_cmds.h"
#include "ff.h"
#include "fatfs_disk_mmc.h"
#include "fatfs_disk_ram.h"

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

int main(void)
{
    xcmd_init(cmd_get_char, cmd_put_char);
    ram_disk_init();
    mmc_disk_init();
    test_cmd_init();
    test_keys_init();
    user_keys_init();
    ex_keys_init();
    ex_cmds_init();
    socket_cmds_init();
    fs_cmds_init();
    ex_list_init();
    while (1)
    {
        xcmd_task();
    }
}