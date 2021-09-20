/*********************************************************************************************************
*
*   模块名称 : 串口中断+FIFO驱动模块
*   文件名称 : bsp_uart_fifo.h
*   版    本 : V1.0
*   日    期 : 2017-07-05
*   作    者 : zhangkeyan
*   说    明 : 头文件
*
*********************************************************************************************************/

#ifndef _BSP_USART_H_
#define _BSP_USART_H_

#include "include.h"

/*模式选择*/
/*{*/
    /*接收使能*/
    #define UART_RX_EN          (0x0004)
    #define UART_TX_EN          (0x0008)

    /*校验*/
    #define UART_PARITY_NONE    (0x0000)
    #define UART_PARITY_EVEN    (0x0400)
    #define UART_PARITY_ODD     (0x0600)

    /*停止位*/
    #define UART_STOP_BITS_1    (0x1000)
    #define UART_STOP_BITS_0_5  (0x2000)
    #define UART_STOP_BITS_1_5  (0x3000)
    #define UART_STOP_BITS_2    (0x4000)
/*}*/

/*波特率*/
/*{*/
    #define UART_BAUDRATE_115200    (115200)
    #define UART_BAUDRATE_38400     (38400)
    #define UART_BAUDRATE_9600      (9600)
    #define UART_BAUDRATE_230400    (230400)
/*}*/

#define UART_BUF_SIZE   (128)

/* 定义端口号 */
typedef enum
{
	COM1 = 0,	/* USART1*/
	COM2 = 1,	/* USART2*/
	COM3 = 2,	/* USART3*/
	COM4 = 3,	/* UART4*/
	COM5 = 4,	/* UART5*/
    COM6 = 5,	/* USART6*/
}COM_PORT_E;

/*设备默认配置115200 8 n 1*/
#define UART_MODE       (0)     /*输入模式选择可以选择对应的功能并 以或的形式进行组合*/
#define UART_BAUDRATE   (1)     /*用于输入波特率*/
#define UART_REG_BEFOR_FUN (2)  /*注册发送前回调函数*/
#define UART_REG_AFTER_FUN (3)  /*注册发送后回调函数*/

void uartInit(uint8_t ch);
int uartConfigMode(uint8_t ch, uint16_t config);
int uartConfigBaudRate(uint8_t ch, uint32_t baudRate);

int uartWrChar(uint8_t ch, uint16_t data);
int uartWrCharBlock(uint8_t ch, uint16_t data);
int uartRdChar(uint8_t ch);
int uartRdNChar(int ch, uint8_t* buffer, uint16_t bufferLen);
int uartWrNChar(int ch, uint8_t* buffer, uint16_t bufferLen);
int uartWrNCharBlock(int ch, uint8_t* buffer, uint16_t bufferLen);
int uartCtrl(int ch, uint8_t flag, uint32_t param);


#endif

/************************************ DIHEELEC (END OF FILE) ******************************************/

