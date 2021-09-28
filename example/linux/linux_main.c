/*
 * @Author: your name
 * @Date: 2021-09-14 23:58:24
 * @LastEditTime: 2021-09-22 23:13:05
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/linux/linux_main.c
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>  
#include <sys/socket.h>  
#include <termio.h>
#include "xcmd.h"
#include "test.h"
#include "ex_keys.h"
#include "ex_cmds.h"

int getch(void)
{
     struct termios tm, tm_old;
     int fd = 0, ch;
 
     if (tcgetattr(fd, &tm) < 0) {//保存现在的终端设置
          return -1;
     }
 
     tm_old = tm;
     cfmakeraw(&tm);//更改终端设置为原始模式，该模式下所有的输入数据以字节为单位被处理
     if (tcsetattr(fd, TCSANOW, &tm) < 0) {//设置上更改之后的设置
          return -1;
     }
 
     ch = getchar();
     // xcmd_print("%d\n", ch);
     if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//更改设置为最初的样子
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

static int key_ctr_c(void* pv)
{
    exit(0);
}

static xcmd_key_t keys[] = 
{
    {CTR_C, key_ctr_c, "ctr+c", NULL},
};

void user_keys_init(void)
{
    xcmd_key_register(keys, sizeof(keys)/sizeof(xcmd_key_t));
}

int main(void)
{
    xcmd_init(cmd_get_char, cmd_put_char);
    test_cmd_init();
    test_keys_init();
    user_keys_init();
    ex_keys_init();
    ex_cmds_init();
    while(1)
    {
        xcmd_task();
    }
}