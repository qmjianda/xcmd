#ifndef __UART_H_
#define __UART_H_

#include "stm32f10x.h"

#define UARTx               (USART1)
#define UARTxClk            (RCC_APB2Periph_USART1)
/*UART1*/
#define UART1_RX_PORT       (GPIOA)
#define UART1_RX_PIN        (GPIO_Pin_10)
#define UART1_RX_CLK        (RCC_APB2Periph_GPIOA)
#define UART1_RX_PINSOURCE  (GPIO_PinSource10)
#define UART1_TX_PORT       (GPIOA)
#define UART1_TX_PIN        (GPIO_Pin_9)
#define UART1_TX_CLK        (RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO)
#define UART1_TX_PINSOURCE  (GPIO_PinSource9)

void uartInit(uint32_t baudrate);
int uartWrChar(uint8_t data);
int uartRdChar(uint8_t *cha);
void uartClean(void);
#endif /*__UART_H_*/
