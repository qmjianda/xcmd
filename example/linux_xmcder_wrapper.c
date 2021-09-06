#include "linux_xmcder_wrapper.h"
#include <stdio.h>
#include<errno.h>  
#include<sys/types.h>  
#include <termio.h>

xcmder_t *g_cmder = NULL;

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
     if (tcsetattr(fd, TCSANOW, &tm_old) < 0) {//更改设置为最初的样子
          return -1;
     }
    
     return ch;
}

int cmd_get_char(char *ch)
{
    *ch = getch();
    return 1;
}

int cmd_put_char(char ch)
{
    putchar(ch);
    return 1;
}

cmd_t default_cmds[] = 
{
    {"clear", cmd_clear, "clear screen", NULL},
    {"help", cmd_help, "show this list", NULL},
    {"exit", cmd_exit, "exit xcmder", NULL},
};


void linux_cmd_register(cmd_t *cmds, uint16_t number)
{
    if(g_cmder)
    {
        xcmd_register(g_cmder, default_cmds, sizeof(cmds)/sizeof(cmd_t));
    }
}

int linux_xcmder_init(void)
{
    g_cmder = xcmd_create_default(cmd_get_char, cmd_put_char);
    if(g_cmder)
    {
        linux_cmd_register(default_cmds, sizeof(cmds)/sizeof(cmd_t));
        return 0;
    }
    else
    {
        return -1;
    }
}