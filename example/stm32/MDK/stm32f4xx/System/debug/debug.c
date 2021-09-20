#include "debug.h"
#include "sys.h"
#include "include.h"
#include "stdarg.h"
#include "bsp_uart.h"

static uint8_t debugUart = 0;

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

int _ttywrch(int ch)
{
    return ch = ch;
}

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
int _sys_exit(int x) 
{ 
	return x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
    uartWrChar(debugUart, ch);
	return ch;
}
#endif
 
void debugInit(uint8_t ch, uint32_t baudrate)
{
    uartInit(ch);
    uartCtrl(ch, UART_MODE, UART_RX_EN|UART_TX_EN|UART_PARITY_NONE|UART_STOP_BITS_1);
    uartCtrl(ch, UART_BAUDRATE, baudrate);
}






