#ifndef XCMD_VFS_DEV_H
#define XCMD_VFS_DEV_H

#include "xcmd_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 设备操作接口 */
typedef struct {
    int (*open)(void *dev_data, uint8_t mode);
    int (*close)(void *dev_data);
    int (*read)(void *dev_data, void *buf, uint32_t len);
    int (*write)(void *dev_data, const void *buf, uint32_t len);
    int (*ioctl)(void *dev_data, uint32_t cmd, void *arg);
} xcmd_dev_ops_t;

/* 设备描述 */
typedef struct {
    const char *name;               /* 设备名称，如 "uart1" */
    const xcmd_dev_ops_t *ops;      /* 操作接口 */
    void *dev_data;                  /* 设备私有数据 */
} xcmd_dev_t;

/* 挂载设备虚拟文件系统 */
int xcmd_vfs_dev_mount(xcmd_vfs_t *vfs, const char *mount_point);

/* 注册设备 */
int xcmd_vfs_dev_register(xcmd_dev_t *dev);

/* 注销设备 */
int xcmd_vfs_dev_unregister(const char *name);

/* 获取 VFS 操作接口（用于挂载） */
const xcmd_vfs_ops_t* xcmd_vfs_dev_get_ops(void);

/* 内置设备 */
extern const xcmd_dev_t xcmd_dev_null;     /* /dev/null */
extern const xcmd_dev_t xcmd_dev_zero;     /* /dev/zero */

#ifdef __cplusplus
}
#endif

#endif /* XCMD_VFS_DEV_H */