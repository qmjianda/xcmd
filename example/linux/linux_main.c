#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "xcmd.h"
#include "xcmd_default_keys.h"
#include "xcmd_default_cmds.h"
#include<errno.h>  
#include<sys/types.h>  
#include<sys/socket.h>  
#include <termio.h>

extern void test_cmd_init(xcmder_t *cmder);

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
     // printf("%d\n", ch);
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

int cmd_ctr_a(void* pv)
{
    printf("this is ctr+a\n");
}

int cmd_ctr_c(void* pv)
{
    exit(0);
}

static xcmd_key_t user_keys[] = 
{
    {CTR_A, cmd_ctr_a,  NULL},
    {CTR_C, cmd_ctr_c, NULL},
};

void user_keys_init(xcmder_t *cmder)
{
    xcmd_key_register(cmder, user_keys, sizeof(user_keys)/sizeof(xcmd_key_t));
}

int main(void)
{
    xcmder_t* cmder = xcmd_create_default(cmd_get_char, cmd_put_char);
    if(cmder)
    {
        test_cmd_init(cmder);
        user_keys_init(cmder);
        default_keys_init(cmder);
        default_cmds_init(cmder);
        xcmd_exec(cmder, "logo");
        while(1)
        {
            xcmd_task(cmder);
        }
    }
    return 1;
}