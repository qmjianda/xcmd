#ifndef XCMD_PROC_CONFG_H
#define XCMD_PROC_CONFG_H

#include <stdint.h>
#include "xcmd_vfs_proc.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 配置项路径常量 */
#define XCMD_PROC_PATH_VERSION      "version"
#define XCMD_PROC_PATH_UPTIME       "uptime"
#define XCMD_PROC_PATH_CMDLINE      "cmdline"

/**
 * @brief 初始化proc配置项
 * 
 * 注册所有示例proc条目到VFS proc文件系统
 * 应在VFS初始化完成后调用
 */
void xcmd_proc_confg_init(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_PROC_CONFG_H */
