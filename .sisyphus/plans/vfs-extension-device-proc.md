# VFS 扩展计划：设备与伪文件系统支持

## TL;DR

> **目标**: 扩展现有 VFS 支持设备和伪文件系统，重定向功能作为 VFS 扩展
>
> **原则**: xcmd 核心保持简单，所有复杂性在 VFS 扩展层
>
> **依赖**: `XCMD_USE_VFS` 宏控制，无 VFS 时无重定向功能

---

## 当前架构分析

### 已有 VFS 设计（良好）

```c
// 挂载点机制
xcmd_vfs_mount(xcmd_vfs_t *vfs, const char *mount_point, 
               const xcmd_vfs_ops_t *ops, void *backend_data);

// 后端操作接口
struct xcmd_vfs_ops {
    int (*open)(xcmd_vfs_file_t *file, const char *path, uint8_t mode);
    int (*close)(xcmd_vfs_file_t *file);
    int (*read)(xcmd_vfs_file_t *file, void *buf, uint32_t len);
    int (*write)(xcmd_vfs_file_t *file, const void *buf, uint32_t len);
    // ... 其他操作
};
```

**优点**：
- 已有挂载点机制
- 后端操作抽象完整
- 零动态内存分配

### 扩展点

当前仅支持 FatFS 后端。需要添加：
1. **设备后端** (`/dev/*`) - UART、SPI、网络等
2. **伪文件系统后端** (`/proc/*`) - 配置读写

---

## 设计方案

### 1. 设备后端 (`/dev`)

```c
// extensions/vfs/xcmd_vfs_dev.h

// 设备操作（比文件更简单）
typedef struct {
    int (*open)(void *dev, uint8_t mode);
    int (*close)(void *dev);
    int (*read)(void *dev, void *buf, uint32_t len);
    int (*write)(void *dev, const void *buf, uint32_t len);
    int (*ioctl)(void *dev, uint32_t cmd, void *arg);
} xcmd_dev_ops_t;

// 设备注册
typedef struct {
    const char *name;           // "uart1", "null", etc.
    const xcmd_dev_ops_t *ops;
    void *dev_data;             // 设备私有数据
} xcmd_dev_t;

// 设备后端挂载
int xcmd_vfs_dev_mount(xcmd_vfs_t *vfs, const char *mount_point);
int xcmd_vfs_dev_register(xcmd_vfs_t *vfs, const xcmd_dev_t *dev);
```

### 2. 伪文件系统后端 (`/proc`)

```c
// extensions/vfs/xcmd_vfs_proc.h

// 伪文件操作
typedef struct {
    int (*read)(void *buf, uint32_t len);   // 读取配置
    int (*write)(const void *buf, uint32_t len); // 写入配置
    uint32_t size;                           // 数据大小
} xcmd_proc_ops_t;

// 伪文件注册
typedef struct {
    const char *name;           // "cpu_freq", "network/ip", etc.
    const xcmd_proc_ops_t *ops;
    void *user_data;            // 上下文数据
} xcmd_proc_entry_t;

// 伪文件系统挂载
int xcmd_vfs_proc_mount(xcmd_vfs_t *vfs, const char *mount_point);
int xcmd_vfs_proc_register(xcmd_vfs_t *vfs, const xcmd_proc_entry_t *entry);
```

### 3. 重定向功能（VFS 扩展）

```c
// extensions/vfs/xcmd_vfs_redirect.h

// 仅当 XCMD_USE_VFS 定义时编译
#ifdef XCMD_USE_VFS

// 替换 xcmd_session.c 中的空实现
xcmd_io_t* xcmd_vfs_redirect_open(xcmd_vfs_t *vfs, const char *target);

// 使用示例
// session_redirected() 调用:
// xcmd_io_t *io = xcmd_vfs_redirect_open(sess->vfs, target);

#endif
```

---

## 实现路径

### Phase 1: 设备后端（优先级高）

**文件**:
- `extensions/vfs/xcmd_vfs_dev.c`
- `extensions/vfs/xcmd_vfs_dev.h`

**TODO**:
- [ ] 实现 `xcmd_vfs_dev_ops` (适配 VFS 接口)
- [ ] 实现 `xcmd_vfs_dev_mount()` 
- [ ] 实现 `xcmd_vfs_dev_register()`
- [ ] 内置设备: `/dev/null`, `/dev/zero`
- [ ] 示例设备: `/dev/uart1`

### Phase 2: 伪文件系统后端

**文件**:
- `extensions/vfs/xcmd_vfs_proc.c`
- `extensions/vfs/xcmd_vfs_proc.h`

**TODO**:
- [ ] 实现 `xcmd_vfs_proc_ops` (适配 VFS 接口)
- [ ] 实现 `xcmd_vfs_proc_mount()`
- [ ] 实现 `xcmd_vfs_proc_register()`
- [ ] 注册/取消注册 API

### Phase 3: 重定向集成

**文件**:
- `extensions/vfs/xcmd_vfs_redirect.c`
- `extensions/vfs/xcmd_vfs_redirect.h`
- `src/xcmd_session.c` (修改 parse_redirect_target)

**TODO**:
- [ ] 实现 `xcmd_vfs_redirect_open()`
- [ ] 修改 `parse_redirect_target()` 调用 VFS
- [ ] 条件编译 (`#ifdef XCMD_USE_VFS`)

---

## 使用示例

### 设备使用

```c
// 初始化时注册设备
xcmd_dev_t uart_dev = {
    .name = "uart1",
    .ops = &uart_ops,
    .dev_data = &uart1_handle
};
xcmd_vfs_dev_register(vfs, &uart_dev);

// 命令行使用
> echo hello > /dev/uart1
> cat /dev/uart1
hello
```

### 伪文件系统使用

```c
// 注册配置项
static int get_cpu_freq(void *buf, uint32_t len) {
    return snprintf(buf, len, "%d", SystemCoreClock);
}
static int set_cpu_freq(const void *buf, uint32_t len) {
    // 解析并设置
    return 0;
}

xcmd_proc_entry_t cpu_freq = {
    .name = "cpu_freq",
    .ops = &(xcmd_proc_ops_t){ .read = get_cpu_freq, .write = set_cpu_freq }
};
xcmd_vfs_proc_register(vfs, &cpu_freq);

// 命令行使用
> cat /proc/cpu_freq
16000000
> echo 8000000 > /proc/cpu_freq
```

### 重定向使用

```c
// 不需要手动操作，VFS 命令自动支持
> ls /dev
uart1
null
> ls /proc
cpu_freq
log_level

> log_save > /sdcard/log.txt     // 文件
> log_save > /dev/uart1           // 设备
> config_dump > /proc/config      // 伪文件
```

---

## 架构图

```
┌─────────────────────────────────────────────────┐
│                  xcmd core                       │
│  (简单，无 VFS 依赖，parse_redirect_target=空)   │
└──────────────────────┬──────────────────────────┘
                       │
                       │ xcmd_session_set_vfs()
                       ▼
┌─────────────────────────────────────────────────┐
│                VFS Layer (扩展)                  │
├─────────────────────────────────────────────────┤
│                                                 │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐ │
│  │/sdcard   │  │/dev      │  │/proc         │ │
│  │(FatFS)   │  │(设备)    │  │(伪文件)      │ │
│  └────┬─────┘  └────┬─────┘  └──────┬───────┘ │
│       │             │               │         │
│       ▼             ▼               ▼         │
│  ┌──────────┐  ┌──────────┐  ┌──────────────┐ │
│  │xcmd_vfs  │  │xcmd_vfs  │  │xcmd_vfs      │ │
│  │_fatfs    │  │_dev      │  │_proc         │ │
│  └──────────┘  └──────────┘  └──────────────┘ │
│                                                 │
│  ┌────────────────────────────────────────────┐ │
│  │xcmd_vfs_redirect (重定向)                  │ │
│  │  xcmd_io_file_open() / dev_open / proc_open│ │
│  └────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────┘
```

---

## 文件结构

```
extensions/vfs/
├── xcmd_vfs.c              # 核心 VFS（已有）
├── xcmd_vfs.h              # 核心 VFS 头（已有）
├── xcmd_vfs_fatfs.c        # FatFS 后端（已有）
├── xcmd_vfs_fatfs.h        # FatFS 后端头（已有）
├── xcmd_vfs_cmds.c         # VFS 命令（已有）
├── xcmd_vfs_cmds.h         # VFS 命令头（已有）
├── xcmd_vfs_dev.c          # 设备后端（新增）
├── xcmd_vfs_dev.h          # 设备后端头（新增）
├── xcmd_vfs_proc.c         # 伪文件系统后端（新增）
├── xcmd_vfs_proc.h         # 伪文件系统后端头（新增）
├── xcmd_vfs_redirect.c     # 重定向功能（新增）
└── xcmd_vfs_redirect.h     # 重定向头（新增）
```

---

## 条件编译

```c
// xcmd_session.c

#ifdef XCMD_USE_VFS
#include "xcmd_vfs_redirect.h"

static xcmd_io_t* parse_redirect_target(xcmd_session_t *sess, const char *target) {
    return xcmd_vfs_redirect_open(sess->vfs, target);
}
#else
static xcmd_io_t* parse_redirect_target(xcmd_session_t *sess, const char *target) {
    (void)sess;
    (void)target;
    return NULL;  // 无VFS时无重定向功能
}
#endif
```

---

## 与 xcmd 核心的关系

| 组件 | 依赖 | 说明 |
|------|------|------|
| xcmd 核心命令 | 无 VFS | 基础功能独立 |
| `parse_redirect_target` | 有 VFS | 条件编译 |
| VFS 扩展 | 独立模块 | 不影响核心 |

**设计原则**: 核心模块不知道 `/dev` 或 `/proc` 的存在，只看到 `xcmd_io_t*`。

---

## 用户确认点

1. **是否同意此架构？** 
   - VFS 扩展层实现设备和伪文件
   - 重定向仅当 VFS 存在时可用

2. **优先级？**
   - A) 设备后端优先
   - B) 伪文件系统优先
   - C) 两者并行

3. **是否需要更多细节？**
   - 具体设备清单？
   - 伪文件配置项示例？