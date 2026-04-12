#ifndef XCMD_VFS_UART_H
#define XCMD_VFS_UART_H

#include "xcmd_vfs.h"
#include "xcmd_vfs_dev.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * UART 操作接口
 *============================================================================*/

typedef struct {
    int (*init)(void *dev_data, uint32_t baudrate);
    int (*deinit)(void *dev_data);
    int (*read)(void *dev_data, void *buf, uint32_t len);
    int (*write)(void *dev_data, const void *buf, uint32_t len);
    int (*ioctl)(void *dev_data, uint32_t cmd, void *arg);
} xcmd_uart_ops_t;

/*============================================================================
 * UART 设备描述
 *============================================================================*/

typedef struct {
    const char *name;               /* 设备名称，如 "uart0" */
    const xcmd_uart_ops_t *ops;     /* UART 操作接口 */
    void *dev_data;                 /* 设备私有数据 */
    uint32_t baudrate;              /* 默认波特率 */
    uint8_t initialized;            /* 初始化标志 */
} xcmd_uart_t;

/*============================================================================
 * UART IOCTL 命令
 *============================================================================*/

#define XCMD_UART_IOCTL_SET_BAUDRATE    0x01
#define XCMD_UART_IOCTL_GET_BAUDRATE    0x02
#define XCMD_UART_IOCTL_SET_PARITY      0x03
#define XCMD_UART_IOCTL_GET_PARITY      0x04
#define XCMD_UART_IOCTL_SET_STOPBITS    0x05
#define XCMD_UART_IOCTL_GET_STOPBITS    0x06
#define XCMD_UART_IOCTL_FLUSH_RX        0x07
#define XCMD_UART_IOCTL_FLUSH_TX        0x08
#define XCMD_UART_IOCTL_GET_RX_COUNT    0x09
#define XCMD_UART_IOCTL_GET_TX_COUNT    0x0A

/* 校验位设置 */
#define XCMD_UART_PARITY_NONE           0
#define XCMD_UART_PARITY_EVEN           1
#define XCMD_UART_PARITY_ODD            2

/* 停止位设置 */
#define XCMD_UART_STOPBITS_1            0
#define XCMD_UART_STOPBITS_1_5          1
#define XCMD_UART_STOPBITS_2            2

/*============================================================================
 * API 函数
 *============================================================================*/

/**
 * @brief 注册 UART 设备到 VFS
 * 
 * @param uart UART 设备描述
 * @return int 0 成功，-1 失败
 */
int xcmd_vfs_uart_register(xcmd_uart_t *uart);

/**
 * @brief 注销 UART 设备
 * 
 * @param name 设备名称
 * @return int 0 成功，-1 失败
 */
int xcmd_vfs_uart_unregister(const char *name);

/**
 * @brief 初始化 UART 子系统
 * 
 * 自动注册内置 UART 设备（如 uart0）到 /dev
 * 
 * @return int 0 成功，-1 失败
 */
int xcmd_vfs_uart_init(void);

/*============================================================================
 * 内置 UART 设备
 *============================================================================*/

/* 内置 UART0 设备 - 可通过 xcmd_dev_uart0 访问 */
extern xcmd_uart_t xcmd_dev_uart0;

/* 获取 UART 设备的 xcmd_dev_ops_t 接口（用于直接注册到设备层） */
const xcmd_dev_ops_t* xcmd_vfs_uart_get_dev_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_VFS_UART_H */
