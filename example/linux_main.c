#include <stdio.h>
#include "xcmd.h"
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

int main(void)
{
    printf("hello\n");
    
    xcmder_t* cmder = xcmd_create_default(cmd_get_char, cmd_put_char);
    if(cmder)
    {
        test_cmd_init(cmder);
        xcmd_exec(cmder, "help");
        while(1)
        {
            xcmd_task(cmder);
        }
    }
    return 1;
}