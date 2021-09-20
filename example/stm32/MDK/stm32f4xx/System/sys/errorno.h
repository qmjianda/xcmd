#ifndef __ERROR_NO_H_
#define __ERROR_NO_H_
#include "include.h"

typedef enum
{
    DISABLED = 1,   /*未使能*/
    NON_EXIST,      /*通道不存在*/
    DEV_UNCONNECT,  /*设备未连接*/
    TIMEOUT,        /*超时*/
    CLK_UNDEFINE,   /*未识别的外设时钟*/
    FIFO_OVERFLOW,  /*缓冲区溢出*/
    PARAM_ERROR,     /*参数错误*/
    
    ERROR_LAST     /*用于标记error no 的总个数*/
}M_Errno;

void setErrno(M_Errno aErrno);
uint32_t getErrno(void);
void showErr(char* msg);

#endif /*__ERROR_NO_H_*/
