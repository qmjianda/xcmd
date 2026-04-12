# VFS 扩展框架实施计划

## TL;DR

> **策略**: 先搭建完整框架骨架，再并行填充各后端实现
>
> **优点**: 结构清晰、接口先行、可并行开发
>
> **预计**: 框架骨架约 200 行代码，各后端独立实现

---

## 骨架结构

### 文件布局

```
extensions/vfs/
├── xcmd_vfs.c              # 核心 VFS（已有）
├── xcmd_vfs.h              # 核心 VFS 头（已有）
├── xcmd_vfs_fatfs.c        # FatFS 后端（已有）
├── xcmd_vfs_fatfs.h        # FatFS 后端头（已有）
├── xcmd_vfs_cmds.c         # VFS 命令（已有）
├── xcmd_vfs_cmds.h         # VFS 命令头（已有）
│
├── xcmd_vfs_dev.c          # 设备后端（新增 - 骨架）
├── xcmd_vfs_dev.h          # 设备后端头（新增 - 完整）
│
├── xcmd_vfs_proc.c         # 伪文件系统后端（新增 - 骨架）
├── xcmd_vfs_proc.h         # 伪文件系统后端头（新增 - 完整）
│
├── xcmd_vfs_redirect.c     # 重定向功能（新增 - 骨架）
└── xcmd_vfs_redirect.h     # 重定向头（新增 - 完整）
```

---

## Phase 1: 头文件定义（完整接口）

### xcmd_vfs_dev.h

```c
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
```

### xcmd_vfs_proc.h

```c
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
```

### xcmd_vfs_redirect.h

```c
#ifndef XCMD_VFS_REDIRECT_H
#define XCMD_VFS_REDIRECT_H

#include "xcmd_io.h"
#include "xcmd_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

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
```

---

## Phase 2: 骨架实现（仅接口，功能返回空）

### xcmd_vfs_dev.c 骨架

```c
#include "xcmd_vfs_dev.h"
#include <string.h>

/* 设备注册表（静态） */
#ifndef XCMD_VFS_DEV_MAX
#define XCMD_VFS_DEV_MAX  16
#endif

static xcmd_dev_t *dev_table[XCMD_VFS_DEV_MAX];
static int dev_count = 0;

/* VFS 操作接口实现 */
static int dev_open(xcmd_vfs_file_t *file, const char *path, uint8_t mode) {
    /* TODO: 查找设备表，调用 ops->open */
    return -1;
}

static int dev_close(xcmd_vfs_file_t *file) {
    /* TODO: 调用 ops->close */
    return -1;
}

static int dev_read(xcmd_vfs_file_t *file, void *buf, uint32_t len) {
    /* TODO: 调用 ops->read */
    return -1;
}

static int dev_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len) {
    /* TODO: 调用 ops->write */
    return -1;
}

/* ... 其他操作骨架 ... */

static const xcmd_vfs_ops_t dev_ops = {
    .open = dev_open,
    .close = dev_close,
    .read = dev_read,
    .write = dev_write,
    /* ... */
};

/* 公开 API */
int xcmd_vfs_dev_mount(xcmd_vfs_t *vfs, const char *mount_point) {
    return xcmd_vfs_mount(vfs, mount_point, &dev_ops, NULL);
}

int xcmd_vfs_dev_register(xcmd_dev_t *dev) {
    if (dev_count >= XCMD_VFS_DEV_MAX) return -1;
    dev_table[dev_count++] = dev;
    return 0;
}

const xcmd_vfs_ops_t* xcmd_vfs_dev_get_ops(void) {
    return &dev_ops;
}

/* 内置设备骨架 */
static int null_open(void *dev_data, uint8_t mode) { return 0; }
static int null_close(void *dev_data) { return 0; }
static int null_read(void *dev_data, void *buf, uint32_t len) { return 0; }
static int null_write(void *dev_data, const void *buf, uint32_t len) { 
    return len; /* 吞掉所有数据 */
}

static const xcmd_dev_ops_t null_ops = {
    .open = null_open,
    .close = null_close,
    .read = null_read,
    .write = null_write,
    .ioctl = NULL,
};

const xcmd_dev_t xcmd_dev_null = {
    .name = "null",
    .ops = &null_ops,
    .dev_data = NULL,
};
```

### xcmd_vfs_proc.c 骨架

```c
#include "xcmd_vfs_proc.h"
#include <string.h>

/* 伪文件注册表（静态） */
#ifndef XCMD_VFS_PROC_MAX
#define XCMD_VFS_PROC_MAX  32
#endif

static xcmd_proc_entry_t *proc_table[XCMD_VFS_PROC_MAX];
static int proc_count = 0;

/* VFS 操作接口实现 */
static int proc_open(xcmd_vfs_file_t *file, const char *path, uint8_t mode) {
    /* TODO: 查找伪文件表，返回条目 */
    return -1;
}

static int proc_close(xcmd_vfs_file_t *file) {
    return 0;
}

static int proc_read(xcmd_vfs_file_t *file, void *buf, uint32_t len) {
    /* TODO: 调用 entry->ops->read */
    return -1;
}

static int proc_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len) {
    /* TODO: 调用 entry->ops->write */
    return -1;
}

/* ... 其他操作骨架 ... */

static const xcmd_vfs_ops_t proc_ops = {
    .open = proc_open,
    .close = proc_close,
    .read = proc_read,
    .write = proc_write,
    /* ... */
};

/* 公开 API */
int xcmd_vfs_proc_mount(xcmd_vfs_t *vfs, const char *mount_point) {
    return xcmd_vfs_mount(vfs, mount_point, &proc_ops, NULL);
}

int xcmd_vfs_proc_register(xcmd_proc_entry_t *entry) {
    if (proc_count >= XCMD_VFS_PROC_MAX) return -1;
    proc_table[proc_count++] = entry;
    return 0;
}

const xcmd_vfs_ops_t* xcmd_vfs_proc_get_ops(void) {
    return &proc_ops;
}
```

### xcmd_vfs_redirect.c 骨架

```c
#include "xcmd_vfs_redirect.h"

#ifdef XCMD_USE_VFS

#include "xcmd_io.h"
#include "xcmd_vfs.h"

xcmd_io_t* xcmd_vfs_redirect_open(xcmd_vfs_t *vfs, const char *target) {
    if (!vfs || !target) return NULL;
    
    /* 路径解析 */
    if (target[0] != '/') {
        /* 相对路径，转换为绝对路径 */
        /* TODO: 使用 cwd */
    }
    
    /* 打开文件（VFS 会根据挂载点选择后端） */
    xcmd_vfs_file_t *file = xcmd_vfs_open(vfs, target, 
                                          XCMD_VFS_MODE_WRITE | XCMD_VFS_MODE_CREATE);
    if (!file) {
        return NULL;
    }
    
    /* 转换为 xcmd_io_t */
    return xcmd_io_file_create(file);
}

int xcmd_vfs_redirect_close(xcmd_io_t *io) {
    if (!io) return -1;
    return xcmd_io_close(io);
}

#endif /* XCMD_USE_VFS */
```

---

## Phase 3: 集成到 xcmd_session

### 修改 parse_redirect_target

```c
// src/xcmd_session.c

#ifdef XCMD_USE_VFS
#include "xcmd_vfs_redirect.h"

static xcmd_io_t* parse_redirect_target(xcmd_session_t *sess, const char *target) {
    if (!sess || !target) return NULL;
    if (!sess->vfs) return NULL;
    
    return xcmd_vfs_redirect_open(sess->vfs, target);
}
#else
static xcmd_io_t* parse_redirect_target(xcmd_session_t *sess, const char *target) {
    (void)sess;
    (void)target;
    return NULL;
}
#endif
```

---

## TODOs

- [ ] 1. 创建 xcmd_vfs_dev.h（完整接口定义）
- [ ] 2. 创建 xcmd_vfs_proc.h（完整接口定义）
- [ ] 3. 创建 xcmd_vfs_redirect.h（完整接口定义）
- [ ] 4. 创建 xcmd_vfs_dev.c（骨架实现）
- [ ] 5. 创建 xcmd_vfs_proc.c（骨架实现）
- [ ] 6. 创建 xcmd_vfs_redirect.c（骨架实现）
- [ ] 7. 修改 xcmd_session.c（条件编译集成）
- [ ] 8. 更新 Makefile（添加新文件）
- [ ] 9. 验证编译（无 VFS 和有 VFS 两种配置）

---

## 验证命令

```bash
# 无 VFS 编译（应通过，重定向返回 NULL）
make clean && make

# 有 VFS 编译（应通过）
make clean && make CFLAGS+="-DXCMD_USE_VFS"
```

---

## 后续填充（骨架完成后）

| 阶段 | 内容 | 可并行 |
|------|------|--------|
| Fill 1 | xcmd_vfs_dev.c 完整实现 | 是 |
| Fill 2 | xcmd_vfs_proc.c 完整实现 | 是 |
| Fill 3 | 内置设备 (null, zero) | 是 |
| Fill 4 | 用户设备注册示例 | 否 |

---

## 是否开始实施？