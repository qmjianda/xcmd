#include "bsp_uart.h"
#include "bsp_ring.h"

#define UART_CH_MAX_NUM (2)
#define IS_UART_CH(s)        (s<UART_CH_MAX_NUM) /*用来判断输入的参数有效性*/
/*UART1*/
#define UART1_RX_PORT       (GPIOA)
#define UART1_RX_PIN        (GPIO_Pin_10)
#define UART1_RX_CLK        (RCC_AHB1Periph_GPIOA)
#define UART1_RX_PINSOURCE  (GPIO_PinSource10)
#define UART1_TX_PORT       (GPIOA)
#define UART1_TX_PIN        (GPIO_Pin_9)
#define UART1_TX_CLK        (RCC_AHB1Periph_GPIOA)
#define UART1_TX_PINSOURCE  (GPIO_PinSource9)

/*UART2*/
#define UART2_RX_PORT       (GPIOD)
#define UART2_RX_PIN        (GPIO_Pin_6)
#define UART2_RX_CLK        (RCC_AHB1Periph_GPIOD)
#define UART2_RX_PINSOURCE  (GPIO_PinSource6)
#define UART2_TX_PORT       (GPIOD)
#define UART2_TX_PIN        (GPIO_Pin_5)
#define UART2_TX_CLK        (RCC_AHB1Periph_GPIOD)
#define UART2_TX_PINSOURCE  (GPIO_PinSource5)

/*UART3*/
#define UART3_RX_PORT       (GPIOD)
#define UART3_RX_PIN        (GPIO_Pin_9)
#define UART3_RX_CLK        (RCC_AHB1Periph_GPIOD)
#define UART3_RX_PINSOURCE  (GPIO_PinSource9)   
#define UART3_TX_PORT       (GPIOD)
#define UART3_TX_PIN        (GPIO_Pin_8)
#define UART3_TX_CLK        (RCC_AHB1Periph_GPIOD)
#define UART3_TX_PINSOURCE  (GPIO_PinSource8)

/*UART4*/
#define UART4_RX_PORT       (GPIOA)
#define UART4_RX_PIN        (GPIO_Pin_1)
#define UART4_RX_CLK        (RCC_AHB1Periph_GPIOA)
#define UART4_RX_PINSOURCE  (GPIO_PinSource1)        
#define UART4_TX_PORT       (GPIOA)
#define UART4_TX_PIN        (GPIO_Pin_0)
#define UART4_TX_CLK        (RCC_AHB1Periph_GPIOA)
#define UART4_TX_PINSOURCE  (GPIO_PinSource0)

/*UART5*/
#define UART5_RX_PORT       (GPIOD)
#define UART5_RX_PIN        (GPIO_Pin_2)
#define UART5_RX_CLK        (RCC_AHB1Periph_GPIOD)
#define UART5_RX_PINSOURCE  (GPIO_PinSource2)        
#define UART5_TX_PORT       (GPIOC)
#define UART5_TX_PIN        (GPIO_Pin_12)
#define UART5_TX_CLK        (RCC_AHB1Periph_GPIOC)
#define UART5_TX_PINSOURCE  (GPIO_PinSource12)

/*UART6*/
#define UART6_RX_PORT       (GPIOC)
#define UART6_RX_PIN        (GPIO_Pin_7)
#define UART6_RX_CLK        (RCC_AHB1Periph_GPIOC)
#define UART6_RX_PINSOURCE  (GPIO_PinSource7)       
#define UART6_TX_PORT       (GPIOC)
#define UART6_TX_PIN        (GPIO_Pin_6)
#define UART6_TX_CLK        (RCC_AHB1Periph_GPIOC)
#define UART6_TX_PINSOURCE  (GPIO_PinSource6)

typedef void(*T_uartCallBackFun)(void);

T_uartCallBackFun uartSendBefor[UART_CH_MAX_NUM] = {0};
T_uartCallBackFun uartSendafter[UART_CH_MAX_NUM] = {0};


//static USART_TypeDef* const usartCH[UART_CH_MAX_NUM] = {USART1, USART2, USART3, UART4, UART5, USART6};
USART_TypeDef* const usartCH[UART_CH_MAX_NUM] = {USART1, USART2};


static char    uartRxBuff[UART_CH_MAX_NUM][UART_BUF_SIZE]; /*串口缓存*/
static char    uartTxBuff[UART_CH_MAX_NUM][UART_BUF_SIZE]; /*串口缓存*/

static T_Ring  uartRxRng[UART_CH_MAX_NUM] = {0}; 
static T_Ring  uartTxRng[UART_CH_MAX_NUM] = {0}; 

static RING_ID uartRxRngID[UART_CH_MAX_NUM] = {0};
static RING_ID uartTxRngID[UART_CH_MAX_NUM] = {0};

static USART_InitTypeDef USART_InitStruct[UART_CH_MAX_NUM];

int uartConfigMode(uint8_t ch, uint16_t config)
{
    if(IS_UART_CH(ch))
    {
        USART_InitStruct[ch].USART_Mode = config & 0x0F;;
        USART_InitStruct[ch].USART_Parity = config & 0x0F00;
        USART_InitStruct[ch].USART_StopBits = config & 0xF000;
        USART_InitStruct[ch].USART_WordLength = USART_WordLength_8b; 
        USART_Init(usartCH[ch], &USART_InitStruct[ch]);
        return 1;
    }
    else
    {
        setErrno(NON_EXIST);
    }
    return -1;
}

int uartConfigBaudRate(uint8_t ch, uint32_t baudRate)
{
    if(IS_UART_CH(ch))
    {
        USART_InitStruct[ch].USART_BaudRate = baudRate;
        USART_Init(usartCH[ch], &USART_InitStruct[ch]);
        return 1;
    }
    else
    {
        setErrno(NON_EXIST);
    }
    return -1;
}

int uartConfigCallBackFunSendBefor(uint8_t ch, void(*before)(void))
{
    if(IS_UART_CH(ch))
    {
        uartSendBefor[ch] = before;
        return 1;
    }
    else
    {
        setErrno(NON_EXIST);
        return -1;
    }
}

int uartConfigCallBackFunSendafter(uint8_t ch, void(*after)(void))
{
    if(IS_UART_CH(ch))
    {
        uartSendafter[ch] = after;
        return 1;
    }
    else
    {
        setErrno(NON_EXIST);
        return -1;
    }
}

void uartInit(uint8_t ch)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    /*接收*/
    uartRxRngID[ch] = xrngInit(&uartRxRng[ch], uartRxBuff[ch], UART_BUF_SIZE);
    
    /*发送*/
    uartTxRngID[ch] = xrngInit(&uartTxRng[ch], uartTxBuff[ch], UART_BUF_SIZE);
    
    
    GPIO_InitStruct.GPIO_Mode   = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd   = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed  = GPIO_Speed_50MHz;
    
    /* 配置UART */
    USART_InitStruct[ch].USART_BaudRate = 115200;
    USART_InitStruct[ch].USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStruct[ch].USART_Mode = USART_Mode_Rx|USART_Mode_Tx;
    USART_InitStruct[ch].USART_Parity = USART_Parity_No;
    USART_InitStruct[ch].USART_StopBits = USART_StopBits_1;
    USART_InitStruct[ch].USART_WordLength = USART_WordLength_8b; 
    
    if(ch == 0) /*UART1 初始化*/
    {
        /* 开启时钟 */
        RCC_AHB1PeriphClockCmd(UART1_RX_CLK|UART1_TX_CLK, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
        /* 配置RX */
        GPIO_InitStruct.GPIO_Pin = UART1_RX_PIN;
        GPIO_Init(UART1_RX_PORT, &GPIO_InitStruct);
        GPIO_PinAFConfig(UART1_RX_PORT, UART1_RX_PINSOURCE, GPIO_AF_USART1);
        /* 配置TX */
        GPIO_InitStruct.GPIO_Pin = UART1_TX_PIN;
        GPIO_Init(UART1_TX_PORT, &GPIO_InitStruct);    
        GPIO_PinAFConfig(UART1_TX_PORT, UART1_TX_PINSOURCE, GPIO_AF_USART1); 
        /* 配置UART的NVIC*/
        NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  
    }
    else if(ch == 1) /*UART2 初始化*/
    {
        /* 开启时钟 */
        RCC_AHB1PeriphClockCmd(UART2_RX_CLK|UART2_TX_CLK, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
        /* 配置RX */
        GPIO_InitStruct.GPIO_Pin = UART2_RX_PIN;
        GPIO_Init(UART2_RX_PORT, &GPIO_InitStruct);
        GPIO_PinAFConfig(UART2_RX_PORT, UART2_RX_PINSOURCE, GPIO_AF_USART2);
        /* 配置TX */
        GPIO_InitStruct.GPIO_Pin = UART2_TX_PIN;
        GPIO_Init(UART2_TX_PORT, &GPIO_InitStruct);    
        GPIO_PinAFConfig(UART2_TX_PORT, UART2_TX_PINSOURCE, GPIO_AF_USART2); 
        /* 配置UART的NVIC*/
        NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;  
    }
    else if(ch == 2)/*UART3 初始化*/
    {
        /* 开启时钟 */
        RCC_AHB1PeriphClockCmd(UART3_RX_CLK|UART3_TX_CLK, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
        /* 配置RX */
        GPIO_InitStruct.GPIO_Pin = UART3_RX_PIN;
        GPIO_Init(UART3_RX_PORT, &GPIO_InitStruct);
        GPIO_PinAFConfig(UART3_RX_PORT, UART3_RX_PINSOURCE, GPIO_AF_USART3);
        /* 配置TX */
        GPIO_InitStruct.GPIO_Pin = UART3_TX_PIN;
        GPIO_Init(UART3_TX_PORT, &GPIO_InitStruct);    
        GPIO_PinAFConfig(UART3_TX_PORT, UART3_TX_PINSOURCE, GPIO_AF_USART3); 
        /* 配置UART的NVIC*/
        NVIC_InitStruct.NVIC_IRQChannel = USART3_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
    }
    else if(ch == 3)/*UART4 初始化*/
    {
        /* 开启时钟 */
        RCC_AHB1PeriphClockCmd(UART4_RX_CLK|UART4_TX_CLK, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
        /* 配置RX */
        GPIO_InitStruct.GPIO_Pin = UART4_RX_PIN;
        GPIO_Init(UART4_RX_PORT, &GPIO_InitStruct);
        GPIO_PinAFConfig(UART4_RX_PORT, UART4_RX_PINSOURCE, GPIO_AF_UART4);
        /* 配置TX */
        GPIO_InitStruct.GPIO_Pin = UART4_TX_PIN;
        GPIO_Init(UART4_TX_PORT, &GPIO_InitStruct);    
        GPIO_PinAFConfig(UART4_TX_PORT, UART4_TX_PINSOURCE, GPIO_AF_UART4);  
        /* 配置UART的NVIC*/
        NVIC_InitStruct.NVIC_IRQChannel = UART4_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
    }
    else if(ch == 4) /*UART5*/
    {
        /* 开启时钟 */
        RCC_AHB1PeriphClockCmd(UART5_RX_CLK|UART5_TX_CLK, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);
        /* 配置RX */
        GPIO_InitStruct.GPIO_Pin = UART5_RX_PIN;
        GPIO_Init(UART5_RX_PORT, &GPIO_InitStruct);
        GPIO_PinAFConfig(UART5_RX_PORT, UART5_RX_PINSOURCE, GPIO_AF_UART5);
        /* 配置TX */
        GPIO_InitStruct.GPIO_Pin = UART5_TX_PIN;
        GPIO_Init(UART5_TX_PORT, &GPIO_InitStruct);    
        GPIO_PinAFConfig(UART5_TX_PORT, UART5_TX_PINSOURCE, GPIO_AF_UART5);  
        /* 配置UART的NVIC*/
        NVIC_InitStruct.NVIC_IRQChannel = UART5_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
    }
    else if(ch == 5)/*UART6 初始化*/
    {
        /* 开启时钟 */
        RCC_AHB1PeriphClockCmd(UART6_RX_CLK|UART6_TX_CLK, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);
        /* 配置RX */
        GPIO_InitStruct.GPIO_Pin = UART6_RX_PIN;
        GPIO_Init(UART6_RX_PORT, &GPIO_InitStruct);
        GPIO_PinAFConfig(UART6_RX_PORT, UART6_RX_PINSOURCE, GPIO_AF_USART6);
        /* 配置TX */
        GPIO_InitStruct.GPIO_Pin = UART6_TX_PIN;
        GPIO_Init(UART6_TX_PORT, &GPIO_InitStruct);    
        GPIO_PinAFConfig(UART6_TX_PORT, UART6_TX_PINSOURCE, GPIO_AF_USART6);  
        /* 配置UART的NVIC*/
        NVIC_InitStruct.NVIC_IRQChannel = USART6_IRQn;
        NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
        NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
        NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0; 
    }
    else
    {
        setErrno(NON_EXIST);
    }
    USART_Init(usartCH[ch], &USART_InitStruct[ch]);
    USART_ITConfig(usartCH[ch], USART_IT_RXNE, ENABLE);
    USART_ITConfig(usartCH[ch], USART_IT_TXE, DISABLE);

    NVIC_Init(&NVIC_InitStruct);
    USART_Cmd(usartCH[ch], ENABLE); 
}

void uartISR(uint8_t ch)
{
    char cha;

    /*接收*/
    if(USART_GetFlagStatus(usartCH[ch], USART_FLAG_RXNE) != RESET)
    {
        xrngPutForce(uartRxRngID[ch], (char)(usartCH[ch]->DR & 0xFF));
        USART_ClearFlag(usartCH[ch], USART_FLAG_RXNE);
    }
    
    /*发送*/
    if((usartCH[ch]->SR & USART_FLAG_TXE) != (uint16_t)RESET)
    {
        if((usartCH[ch]->CR1 & (1<<6)) == 0) /*判断是否未开启发送完成中断*/
        {
            if(xrngGet(uartTxRngID[ch], &cha))
            {
                usartCH[ch]->DR = cha & (uint16_t)0x01FF;
                usartCH[ch]->CR1 |= (1<<6); /*开启发送完成中断*/
            }
            else
            {
                if(uartSendafter[ch])
                {
                    uartSendafter[ch]();
                }
            }
        }
        usartCH[ch]->CR1 &= ~(1<<7); /*关闭发送缓冲区空中断*/
        usartCH[ch]->SR = (uint16_t)~USART_FLAG_TXE;
    }
    
    if((usartCH[ch]->SR & USART_FLAG_TC) != (uint16_t)RESET)
    {
        if(xrngGet(uartTxRngID[ch], &cha))
        {
            usartCH[ch]->DR = cha & (uint16_t)0x01FF;
        }
        else
        {
            usartCH[ch]->CR1 &= ~(1<<6); /*关闭发送完成中断*/
            if(uartSendafter[ch])
            {
                uartSendafter[ch]();
            }
        }
        usartCH[ch]->SR = (uint16_t)~USART_FLAG_TC;
    }
}

void USART1_IRQHandler(void)
{
    uartISR(0);
}

void USART2_IRQHandler(void)
{
    uartISR(1);
}

void USART3_IRQHandler(void)
{
    uartISR(2);
}

void UART4_IRQHandler(void)
{
    uartISR(3);
}

void UART5_IRQHandler(void)
{
    uartISR(4);
}

void USART6_IRQHandler(void)
{
    uartISR(5);
}

void uartEnableSendIT(uint8_t ch)
{
    if((usartCH[ch]->CR1 & (1<<7)) == 0)
    {
        usartCH[ch]->CR1 |= (1<<7); /*使能发送中断*/
    }
}

int uartWrChar(uint8_t ch, uint16_t data)
{
    if(IS_UART_CH(ch))
    {
        if(uartSendBefor[ch])
        {
            uartSendBefor[ch]();
        }
        while(xrngIsFull(uartTxRngID[ch]))
        {
        }
        if(xrngPut(uartTxRngID[ch],data) == TRUE)
        {
            uartEnableSendIT(ch);
            return 0;
        }
    }
    else
    {
        setErrno(NON_EXIST);
    }
    return -1;
}

int uartWrCharBlock(uint8_t ch, uint16_t data)
{
    uartWrChar(ch, data);
    while(xrngIsEmpty(uartTxRngID[ch]) == FALSE )
    {
    }
    return 1;
}

int uartWrNCharBlock(int ch, uint8_t* buffer, uint16_t bufferLen)
{
    int ret = uartWrNChar(ch, buffer, bufferLen);
    while(xrngIsEmpty(uartTxRngID[ch]) == FALSE )
    {
    }
    return ret;
}

int uartRdChar(uint8_t ch)
{
    uint8_t  temp;
    if(IS_UART_CH(ch))
    {
        if(xrngGet(uartRxRngID[ch], (char*)&temp))
        {
            return (uint8_t)temp;
        }
    }
    else
    {
        setErrno(NON_EXIST);
    }
    return -1;
}

int uartWrNChar(int ch, uint8_t* buffer, uint16_t bufferLen)
{
    uint16_t i=0;
    if(IS_UART_CH(ch))
    {
        if(uartSendBefor[ch])
        {
            uartSendBefor[ch]();
        }
        
        while(i<bufferLen)
        {
            if(xrngPut(uartTxRngID[ch], buffer[i]) != TRUE)
            {
                uartEnableSendIT(ch); /*使能发送中断*/
                setErrno(FIFO_OVERFLOW);
                while(xrngIsFull(uartTxRngID[ch]))
                {
                }
            }
            else
            {
                i++;
            }
        }
        uartEnableSendIT(ch); /*使能发送中断*/
        return i;
    }
    else
    {
        setErrno(NON_EXIST);
    }
    return -1;
}

int uartRdNChar(int ch, uint8_t* buffer, uint16_t bufferLen)
{
    /*设备数据读取操作*/
    if(IS_UART_CH(ch))
    {
        return xrngBufGet(uartRxRngID[ch], (char*)buffer, bufferLen);
    }
    else
    {
        setErrno(NON_EXIST);
    }
    return -1;  
}

int uartCtrl(int ch, uint8_t flag, uint32_t param)
{
    int ret = -1;
    switch(flag)
    {
        case UART_MODE:
        {
            ret = uartConfigMode(ch,param);
        }
        break;
        case UART_BAUDRATE:
        {
            ret = uartConfigBaudRate(ch, param);
        }
        break;
        case UART_REG_BEFOR_FUN:
        {
            ret = uartConfigCallBackFunSendBefor(ch, (T_uartCallBackFun)param);
        }
        break;
        case UART_REG_AFTER_FUN:
        {
            ret = uartConfigCallBackFunSendafter(ch, (T_uartCallBackFun)param);
        }
        break;
        default:
            setErrno(PARAM_ERROR);
            break;
    }
    return ret;
}

