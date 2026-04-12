#ifndef XCMD_VFS_PROC_H
#define XCMD_VFS_PROC_H

#include "xcmd_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 伪文件操作接口 */
typedef struct {
    int (*read)(void *user_data, void *buf, uint32_t len);
    int (*write)(void *user_data, const void *buf, uint32_t len);
    uint32_t (*size)(void *user_data);
} xcmd_proc_ops_t;

/* 伪文件条目 */
typedef struct {
    const char *name;               /* 名称，如 "cpu_freq" */
    const xcmd_proc_ops_t *ops;     /* 操作接口 */
    void *user_data;                 /* 用户数据 */
    uint8_t attr;                   /* 属性：只读/读写 */
} xcmd_proc_entry_t;

/* 挂载伪文件系统 */
int xcmd_vfs_proc_mount(xcmd_vfs_t *vfs, const char *mount_point);

/* 注册伪文件条目 */
int xcmd_vfs_proc_register(xcmd_proc_entry_t *entry);

/* 注销伪文件条目 */
int xcmd_vfs_proc_unregister(const char *name);

/* 获取 VFS 操作接口（用于挂载） */
const xcmd_vfs_ops_t* xcmd_vfs_proc_get_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_VFS_PROC_H */
