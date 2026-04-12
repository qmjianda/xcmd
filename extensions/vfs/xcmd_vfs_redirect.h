#ifndef XCMD_VFS_REDIRECT_H
#define XCMD_VFS_REDIRECT_H

#include "xcmd_io.h"
#include "xcmd_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XCMD_REDIRECT_NONE      0
#define XCMD_REDIRECT_FILE      1
#define XCMD_REDIRECT_UART      2
#define XCMD_REDIRECT_SOCKET    3

/* 仅当 VFS 可用时编译 */
#ifdef XCMD_USE_VFS

/**
 * @brief 打开重定向目标
 * 
 * 支持的目标类型：
 * - 普通文件：/sdcard/log.txt（通过 FatFS）
 * - 设备文件：/dev/uart1（通过设备后端）
 * - 伪文件：/proc/config（通过伪文件后端）
 * 
 * @param vfs VFS 实例
 * @param target 目标路径
 * @return xcmd_io_t* IO 实例，失败返回 NULL
 */
xcmd_io_t* xcmd_vfs_redirect_open(xcmd_vfs_t *vfs, const char *target);

/**
 * @brief 关闭重定向 IO
 */
int xcmd_vfs_redirect_close(xcmd_io_t *io);

#else

/* 无 VFS 时，空实现 */
#define xcmd_vfs_redirect_open(vfs, target)  ((void)(vfs), (void)(target), NULL)
#define xcmd_vfs_redirect_close(io)           ((void)(io), 0)

#endif /* XCMD_USE_VFS */

#ifdef __cplusplus
}
#endif

#endif /* XCMD_VFS_REDIRECT_H */
