#include "xcmd_vfs_uart.h"
#include <string.h>
#include <stdio.h>

#ifndef XCMD_VFS_UART_MAX
#define XCMD_VFS_UART_MAX  4
#endif

#ifndef XCMD_UART_RX_BUF_SIZE
#define XCMD_UART_RX_BUF_SIZE  256
#endif

#ifndef XCMD_UART_TX_BUF_SIZE
#define XCMD_UART_TX_BUF_SIZE  256
#endif

typedef struct {
    uint8_t buf[XCMD_UART_RX_BUF_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} uart_ringbuf_t;

typedef struct {
    uart_ringbuf_t rx_buf;
    uart_ringbuf_t tx_buf;
    uint32_t baudrate;
    uint8_t parity;
    uint8_t stopbits;
    uint8_t initialized;
} uart0_private_t;

static uart0_private_t uart0_priv;
static xcmd_uart_t *uart_table[XCMD_VFS_UART_MAX];
static int uart_count = 0;

static int ringbuf_put(uart_ringbuf_t *rb, uint8_t data)
{
    uint16_t next_head = (rb->head + 1) % XCMD_UART_RX_BUF_SIZE;
    if (next_head == rb->tail) {
        return -1;
    }
    rb->buf[rb->head] = data;
    rb->head = next_head;
    return 0;
}

static int ringbuf_get(uart_ringbuf_t *rb, uint8_t *data)
{
    if (rb->head == rb->tail) {
        return -1;
    }
    *data = rb->buf[rb->tail];
    rb->tail = (rb->tail + 1) % XCMD_UART_RX_BUF_SIZE;
    return 0;
}

static uint16_t ringbuf_count(uart_ringbuf_t *rb)
{
    return (rb->head - rb->tail + XCMD_UART_RX_BUF_SIZE) % XCMD_UART_RX_BUF_SIZE;
}

static void ringbuf_flush(uart_ringbuf_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

static int uart0_init(void *dev_data, uint32_t baudrate)
{
    uart0_private_t *priv = (uart0_private_t *)dev_data;
    
    (void)baudrate;
    
    ringbuf_flush(&priv->rx_buf);
    ringbuf_flush(&priv->tx_buf);
    priv->baudrate = baudrate ? baudrate : 115200;
    priv->parity = XCMD_UART_PARITY_NONE;
    priv->stopbits = XCMD_UART_STOPBITS_1;
    priv->initialized = 1;
    
    return 0;
}

static int uart0_deinit(void *dev_data)
{
    uart0_private_t *priv = (uart0_private_t *)dev_data;
    priv->initialized = 0;
    return 0;
}

static int uart0_read(void *dev_data, void *buf, uint32_t len)
{
    uart0_private_t *priv = (uart0_private_t *)dev_data;
    uint8_t *pbuf = (uint8_t *)buf;
    uint32_t i;
    
    if (!priv->initialized) {
        return -1;
    }
    
    for (i = 0; i < len; i++) {
        if (ringbuf_get(&priv->rx_buf, &pbuf[i]) != 0) {
            break;
        }
    }
    
    return (int)i;
}

static int uart0_write(void *dev_data, const void *buf, uint32_t len)
{
    uart0_private_t *priv = (uart0_private_t *)dev_data;
    const uint8_t *pbuf = (const uint8_t *)buf;
    uint32_t i;
    
    if (!priv->initialized) {
        return -1;
    }
    
    for (i = 0; i < len; i++) {
        if (ringbuf_put(&priv->tx_buf, pbuf[i]) != 0) {
            break;
        }
    }
    
    return (int)i;
}

static int uart0_ioctl(void *dev_data, uint32_t cmd, void *arg)
{
    uart0_private_t *priv = (uart0_private_t *)dev_data;
    
    if (!priv->initialized) {
        return -1;
    }
    
    switch (cmd) {
        case XCMD_UART_IOCTL_SET_BAUDRATE:
            if (arg) {
                priv->baudrate = *(uint32_t *)arg;
            }
            break;
            
        case XCMD_UART_IOCTL_GET_BAUDRATE:
            if (arg) {
                *(uint32_t *)arg = priv->baudrate;
            }
            break;
            
        case XCMD_UART_IOCTL_SET_PARITY:
            if (arg) {
                priv->parity = *(uint8_t *)arg;
            }
            break;
            
        case XCMD_UART_IOCTL_GET_PARITY:
            if (arg) {
                *(uint8_t *)arg = priv->parity;
            }
            break;
            
        case XCMD_UART_IOCTL_SET_STOPBITS:
            if (arg) {
                priv->stopbits = *(uint8_t *)arg;
            }
            break;
            
        case XCMD_UART_IOCTL_GET_STOPBITS:
            if (arg) {
                *(uint8_t *)arg = priv->stopbits;
            }
            break;
            
        case XCMD_UART_IOCTL_FLUSH_RX:
            ringbuf_flush(&priv->rx_buf);
            break;
            
        case XCMD_UART_IOCTL_FLUSH_TX:
            ringbuf_flush(&priv->tx_buf);
            break;
            
        case XCMD_UART_IOCTL_GET_RX_COUNT:
            if (arg) {
                *(uint16_t *)arg = ringbuf_count(&priv->rx_buf);
            }
            break;
            
        case XCMD_UART_IOCTL_GET_TX_COUNT:
            if (arg) {
                *(uint16_t *)arg = ringbuf_count(&priv->tx_buf);
            }
            break;
            
        default:
            return -1;
    }
    
    return 0;
}

static const xcmd_uart_ops_t uart0_ops = {
    .init = uart0_init,
    .deinit = uart0_deinit,
    .read = uart0_read,
    .write = uart0_write,
    .ioctl = uart0_ioctl,
};

xcmd_uart_t xcmd_dev_uart0 = {
    .name = "uart0",
    .ops = &uart0_ops,
    .dev_data = &uart0_priv,
    .baudrate = 115200,
    .initialized = 0,
};

static int uart_dev_open(void *dev_data, uint8_t mode)
{
    xcmd_uart_t *uart = (xcmd_uart_t *)dev_data;
    
    (void)mode;
    
    if (!uart || !uart->ops) {
        return -1;
    }
    
    if (!uart->initialized && uart->ops->init) {
        if (uart->ops->init(uart->dev_data, uart->baudrate) != 0) {
            return -1;
        }
        uart->initialized = 1;
    }
    
    return 0;
}

static int uart_dev_close(void *dev_data)
{
    xcmd_uart_t *uart = (xcmd_uart_t *)dev_data;
    
    if (!uart || !uart->ops) {
        return -1;
    }
    
    if (uart->initialized && uart->ops->deinit) {
        uart->ops->deinit(uart->dev_data);
        uart->initialized = 0;
    }
    
    return 0;
}

static int uart_dev_read(void *dev_data, void *buf, uint32_t len)
{
    xcmd_uart_t *uart = (xcmd_uart_t *)dev_data;
    
    if (!uart || !uart->ops || !uart->ops->read) {
        return -1;
    }
    
    return uart->ops->read(uart->dev_data, buf, len);
}

static int uart_dev_write(void *dev_data, const void *buf, uint32_t len)
{
    xcmd_uart_t *uart = (xcmd_uart_t *)dev_data;
    
    if (!uart || !uart->ops || !uart->ops->write) {
        return -1;
    }
    
    return uart->ops->write(uart->dev_data, buf, len);
}

static int uart_dev_ioctl(void *dev_data, uint32_t cmd, void *arg)
{
    xcmd_uart_t *uart = (xcmd_uart_t *)dev_data;
    
    if (!uart || !uart->ops || !uart->ops->ioctl) {
        return -1;
    }
    
    return uart->ops->ioctl(uart->dev_data, cmd, arg);
}

static const xcmd_dev_ops_t uart_dev_ops = {
    .open = uart_dev_open,
    .close = uart_dev_close,
    .read = uart_dev_read,
    .write = uart_dev_write,
    .ioctl = uart_dev_ioctl,
};

const xcmd_dev_ops_t* xcmd_vfs_uart_get_dev_ops(void)
{
    return &uart_dev_ops;
}

int xcmd_vfs_uart_register(xcmd_uart_t *uart)
{
    xcmd_dev_t dev;
    
    if (!uart || !uart->name || uart_count >= XCMD_VFS_UART_MAX) {
        return -1;
    }
    
    uart_table[uart_count++] = uart;
    
    dev.name = uart->name;
    dev.ops = &uart_dev_ops;
    dev.dev_data = uart;
    
    return xcmd_vfs_dev_register(&dev);
}

int xcmd_vfs_uart_unregister(const char *name)
{
    int i;
    
    if (!name) {
        return -1;
    }
    
    for (i = 0; i < uart_count; i++) {
        if (uart_table[i] && strcmp(uart_table[i]->name, name) == 0) {
            for (int j = i; j < uart_count - 1; j++) {
                uart_table[j] = uart_table[j + 1];
            }
            uart_count--;
            uart_table[uart_count] = NULL;
            return xcmd_vfs_dev_unregister(name);
        }
    }
    
    return -1;
}

int xcmd_vfs_uart_init(void)
{
    static uint8_t initialized = 0;
    
    if (initialized) {
        return 0;
    }
    
    xcmd_vfs_uart_register(&xcmd_dev_uart0);
    
    initialized = 1;
    return 0;
}
