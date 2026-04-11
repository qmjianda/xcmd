# xcmd_io 重构方案

## 1. 问题总结

| # | 问题 | 严重程度 | 影响 |
|---|------|----------|------|
| A | Terminal backend 使用静态存储，只能单例 | 严重 | 无法支持多session |
| B | Device registry 和 file pool 共用 XCMD_IO_MAX_DEVICES(8) | 严重 | 资源竞争 |
| C | 重定向只在成功路径关闭IO，异常路径泄漏 | 严重 | 资源泄漏 |
| D | term_readable() 始终返回1，无阻塞能力 | 中等 | 100% CPU轮询 |
| E | 返回值语义模糊（<=0既表示EOF又表示error） | 中等 | API难以正确使用 |

---

## 2. 重构目标

1. **消除静态存储** - 所有backend支持多实例
2. **分离资源池** - device registry和file pool独立计数
3. **安全重定向** - RAII风格，保证资源释放
4. **明确API契约** - 统一错误码语义
5. **可选阻塞** - 提供readable/poll机制
6. **移除legacy兼容** - 清理过时的xcmd_init/xcmd_task

---

## 3. 新架构设计

### 3.1 核心结构

```c
// xcmd_io.h 新增

/**
 * @brief IO结果码（统一错误语义）
 */
typedef enum {
    XCMD_IO_OK      =  0,   /* 成功 */
    XCMD_IO_EOF     =  0,   /* 读端关闭或无数据（可与OK合并）*/
    XCMD_IO_ERROR   = -1,   /* 错误 */
    XCMD_IO_AGAIN   = -2,   /* 暂时无数据（非阻塞模式）*/
    XCMD_IO_BUSY    = -3,   /* 设备忙 */
} xcmd_io_result_t;

/**
 * @brief Backend上下文基础结构（所有backend必须嵌入此结构）
 */
typedef struct xcmd_io_ctx {
    xcmd_io_type_t type;
    void (*destroy)(struct xcmd_io_ctx *ctx);  /* 析构回调 */
} xcmd_io_ctx_t;

/**
 * @brief 新的xcmd_io结构（保持二进制兼容）
 */
struct xcmd_io {
    const xcmd_io_ops_t *ops;
    void                 *handle;      /* 改为指向 xcmd_io_ctx_t */
    void                 *user_data;
    uint8_t              type;
    uint8_t              flags;
    uint8_t              reserved[2];
};
```

### 3.2 Terminal Backend 重构（解决单例问题）

**旧实现（有问题）：**
```c
// xcmd_io.c:57-58
static xcmd_io_t term_io_storage;
static xcmd_io_term_ctx_t term_ctx_storage;

xcmd_io_t* xcmd_io_term_create(...) {
    return &term_io_storage;  // 始终返回同一静态变量
}
```

**新实现（支持多实例）：**
```c
/**
 * @brief Terminal backend上下文
 * 必须用户传入buffer，由调用者管理生命周期
 */
typedef struct {
    xcmd_io_ctx_t base;           /* 基类，必须为第一个成员 */
    int (*get_char)(uint8_t *);   /* 读回调 */
    int (*put_char)(uint8_t);     /* 写回调 */
} xcmd_io_term_ctx_t;

/**
 * @brief 创建Terminal IO（用户管理内存）
 * 
 * @param ctx 用户提供的上下文buffer（必须足够大）
 * @param ctx_size buffer大小
 * @param get_char 读回调（可为NULL表示只写）
 * @param put_char 写回调（可为NULL表示只读）
 * @return xcmd_io_t* 成功返回，NULL失败
 */
xcmd_io_t* xcmd_io_term_init(void *ctx, size_t ctx_size,
                             int (*get_char)(uint8_t *),
                             int (*put_char)(uint8_t));

/**
 * @brief 创建Terminal IO（静态内联版本，零动态分配）
 * 
 * @param type 存储类型，必须是 xcmd_io_term_ctx_t 或更大
 * @param get_char 读回调
 * @param put_char 写回调
 */
#define XCMD_IO_TERM_STATIC(type, get_char, put_char) \
    xcmd_io_term_init((type), sizeof(type), get_char, put_char)
```

**实现：**
```c
static void term_destroy(xcmd_io_ctx_t *ctx) {
    /* Terminal backend通常不需要动态资源清理 */
}

static const xcmd_io_ops_t term_ops = {
    .read = term_read,
    .write = term_write,
    .flush = term_flush,
    .close = term_close,
    .readable = term_readable,
};

xcmd_io_t* xcmd_io_term_init(void *ctx, size_t ctx_size,
                             int (*get_char)(uint8_t *),
                             int (*put_char)(uint8_t)) {
    if (!ctx || ctx_size < sizeof(xcmd_io_term_ctx_t)) {
        return NULL;
    }
    
    xcmd_io_term_ctx_t *term_ctx = (xcmd_io_term_ctx_t *)ctx;
    term_ctx->base.type = XCMD_IO_TERM;
    term_ctx->base.destroy = term_destroy;
    term_ctx->get_char = get_char;
    term_ctx->put_char = put_char;
    
    xcmd_io_t *io = (xcmd_io_t *)ctx;
    io->ops = &term_ops;
    io->handle = term_ctx;        /* handle指向上下文 */
    io->type = XCMD_IO_TERM;
    io->flags = 0;
    if (get_char) io->flags |= XCMD_IO_READ;
    if (put_char) io->flags |= XCMD_IO_WRITE;
    
    return io;
}
```

---

### 3.3 资源管理重构（分离资源池）

**旧实现（有问题）：**
```c
// xcmd_io.c
#define XCMD_IO_MAX_DEVICES     8   /* 同时用于file pool和device registry */

static xcmd_io_t file_io_pool[XCMD_IO_MAX_DEVICES];     /* 文件池 */
static xcmd_io_dev_t dev_registry[XCMD_IO_MAX_DEVICES];  /* 设备注册表 */
```

**新实现：**
```c
// xcmd_io.h

#ifndef XCMD_IO_MAX_FILE_HANDLES
#define XCMD_IO_MAX_FILE_HANDLES    8   /* 文件IO池大小 */
#endif

#ifndef XCMD_IO_MAX_DEVICES
#define XCMD_IO_MAX_DEVICES          16  /* 设备注册表大小（增大）*/
#endif

// xcmd_io.c

/* File IO池（预分配，适用于VFS file）*/
static xcmd_io_t s_file_io_pool[XCMD_IO_MAX_FILE_HANDLES];
static uint8_t s_file_io_used[XCMD_IO_MAX_FILE_HANDLES];

/* Device registry（独立计数）*/
typedef struct {
    char name[32];
    xcmd_io_t *io;
} xcmd_io_dev_t;

static xcmd_io_dev_t s_dev_registry[XCMD_IO_MAX_DEVICES];
static int s_dev_count = 0;  /* 独立计数器 */

/* 文件池获取/释放接口 */
static xcmd_io_t* file_io_alloc(void) {
    for (int i = 0; i < XCMD_IO_MAX_FILE_HANDLES; i++) {
        if (!s_file_io_used[i]) {
            s_file_io_used[i] = 1;
            return &s_file_io_pool[i];
        }
    }
    return NULL;
}

static void file_io_free(xcmd_io_t *io) {
    for (int i = 0; i < XCMD_IO_MAX_FILE_HANDLES; i++) {
        if (&s_file_io_pool[i] == io) {
            s_file_io_used[i] = 0;
            return;
        }
    }
}
```

---

### 3.4 RAII风格重定向机制

**旧实现（有问题）：**
```c
// xcmd_session.c
static int session_redirected(...) {
    // ...打开重定向
    return i;  /* 如果中间出错返回，redirect_restore不会被调用 */
}

static int session_cmd_match(...) {
    // ...
    argc = session_redirected(sess, argc, argv);  /* 可能返回但不执行redirect_restore */
    ret = p->func(argc, argv);
    session_redirect_restore(sess);  /* 只有成功路径才调用 */
}
```

**新实现（RAII风格）：**
```c
/**
 * @brief 自动重定向guard（RAII）
 * 构造函数设置重定向，析构函数自动恢复
 */
typedef struct {
    xcmd_session_t *sess;
    xcmd_io_t *orig_stdout;
    uint8_t active;
} xcmd_io_redirect_guard_t;

static void redirect_guard_enter(xcmd_io_redirect_guard_t *guard, 
                                  xcmd_session_t *sess, 
                                  xcmd_io_t *new_stdout) {
    guard->sess = sess;
    guard->orig_stdout = sess->stdout_io;
    guard->active = 1;
    sess->stdout_io = new_stdout;
}

static void redirect_guard_exit(xcmd_io_redirect_guard_t *guard) {
    if (!guard->active || !guard->sess) return;
    
    if (guard->sess->stdout_io && 
        guard->sess->stdout_io->type == XCMD_IO_FILE) {
        xcmd_io_close(guard->sess->stdout_io);
    }
    guard->sess->stdout_io = guard->orig_stdout;
    guard->active = 0;
}

/* C语言模拟RAII - 使用goto cleanup模式 */
static int session_cmd_match(...) {
    xcmd_io_redirect_guard_t guard = {0};
    
    for (...) {
        if (需要重定向) {
            xcmd_io_t *io = parse_redirect_target(sess, target);
            if (!io) {
                xcmd_session_print(sess, "Cannot open: %s\r\n", target);
                ret = -1;
                goto cleanup;  /* guard_exit会被调用 */
            }
            redirect_guard_enter(&guard, sess, io);
        }
        
        ret = p->func(argc, argv);
        goto cleanup;  /* 无论如何都会清理 */
    }
    
cleanup:
    redirect_guard_exit(&guard);
    return ret;
}
```

**封装成宏（更易用）：**
```c
#define XCMD_IO_REDIRECT_GUARD(guard, sess, new_io) \
    for (int _guard_active = ({ \
        redirect_guard_enter(&(guard), (sess), (new_io)); \
        1; \
    }); _guard_active; _guard_active--, redirect_guard_exit(&(guard)))
```

---

### 3.5 统一错误码语义

**新的返回约定：**

| 返回值 | 含义 | 说明 |
|--------|------|------|
| `> 0` | 成功读取/写入的字节数 | 正常 |
| `0` | 无数据可读（非阻塞）或EOF | 需要结合readable()判断 |
| `-XCMD_IO_ERROR` | 系统错误 | 查看errno |
| `-XCMD_IO_AGAIN` | 暂时无数据（non-blocking） | 稍后重试 |
| `-XCMD_IO_BUSY` | 设备忙 | 稍后重试 |

**内联函数更新：**
```c
static inline int xcmd_io_read(xcmd_io_t *io, void *buf, size_t len) {
    if (!io || !io->ops || !io->ops->read) return -XCMD_IO_ERROR;
    return io->ops->read(io, buf, len);
}

/**
 * @brief 读取单个字符
 * @return 1成功, 0无数据, 负数错误
 */
static inline int xcmd_io_getc(xcmd_io_t *io, uint8_t *c) {
    if (!io || !io->ops || !io->ops->read) return -XCMD_IO_ERROR;
    int ret = io->ops->read(io, c, 1);
    return ret > 0 ? 1 : ret;  /* 转换语义 */
}

/**
 * @brief 检查是否可读（不阻塞）
 * @return 1可读, 0不可读, 负数错误
 */
static inline int xcmd_io_can_read(xcmd_io_t *io) {
    if (!io || !io->ops) return -XCMD_IO_ERROR;
    if (!io->ops->readable) return 1;  /* 默认认为可读 */
    return io->ops->readable(io);
}
```

---

### 3.6 修复term_readable()

**问题：** 旧的实现始终返回1，导致100% CPU轮询。

**新实现：** 使用用户提供的poll回调：

```c
typedef struct {
    xcmd_io_ctx_t base;
    int (*get_char)(uint8_t *);
    int (*put_char)(uint8_t);
    int (*poll)(void);  /* 可选：返回1表示有数据可读 */
} xcmd_io_term_ctx_t;

static int term_readable(xcmd_io_t *io) {
    xcmd_io_term_ctx_t *ctx = (xcmd_io_term_ctx_t *)io->handle;
    
    /* 如果用户提供了poll回调，使用它 */
    if (ctx->poll) {
        return ctx->poll();
    }
    
    /* 否则默认返回1，允许基于get_char的非阻塞返回 */
    return 1;
}

/**
 * @brief 创建Terminal IO（带poll支持）
 */
xcmd_io_t* xcmd_io_term_create_ex(int (*get_char)(uint8_t *),
                                  int (*put_char)(uint8_t),
                                  int (*poll)(void));

#define XCMD_IO_TERM_STATIC_EX(type, get_char, put_char, poll) \
    xcmd_io_term_init_ex((type), sizeof(type), get_char, put_char, poll)
```

---

## 4. 头文件变更摘要

### 4.1 xcmd_io.h 新增/修改

```c
// === 新增类型 ===
typedef enum {
    XCMD_IO_OK      =  0,
    XCMD_IO_ERROR   = -1,
    XCMD_IO_AGAIN   = -2,
    XCMD_IO_BUSY    = -3,
} xcmd_io_result_t;

typedef struct xcmd_io_ctx xcmd_io_ctx_t;

// === 修改宏 ===
#define XCMD_IO_MAX_FILE_HANDLES    8   /* 文件IO池大小 */
#define XCMD_IO_MAX_DEVICES         16  /* 设备注册表大小（增大）*/

// === 新增API ===
xcmd_io_t* xcmd_io_term_init(void *ctx, size_t ctx_size,
                             int (*get_char)(uint8_t *),
                             int (*put_char)(uint8_t));

xcmd_io_t* xcmd_io_term_init_ex(void *ctx, size_t ctx_size,
                                int (*get_char)(uint8_t *),
                                int (*put_char)(uint8_t),
                                int (*poll)(void));

#define XCMD_IO_TERM_STATIC(type, get_char, put_char) \
    xcmd_io_term_init((type), sizeof(type), get_char, put_char)

#define XCMD_IO_TERM_STATIC_EX(type, get_char, put_char, poll) \
    xcmd_io_term_init_ex((type), sizeof(type), get_char, put_char, poll)

/* 可选：保留旧API但标记为deprecated */
__attribute__((deprecated))
xcmd_io_t* xcmd_io_term_create(int (*get_char)(uint8_t *), 
                                int (*put_char)(uint8_t));

// === 新增guard API ===
void xcmd_io_redirect_guard_enter(xcmd_io_redirect_guard_t *guard,
                                   xcmd_session_t *sess,
                                   xcmd_io_t *new_stdout);
void xcmd_io_redirect_guard_exit(xcmd_io_redirect_guard_t *guard);
```

### 4.2 xcmd_session.h 新增

```c
/* RAII重定向guard */
typedef struct {
    xcmd_session_t *sess;
    xcmd_io_t *orig_stdout;
    uint8_t active;
} xcmd_io_redirect_guard_t;
```

---

## 5. 文件变更清单

| 文件 | 变更类型 | 描述 |
|------|----------|------|
| `inc/xcmd_io.h` | 大改 | 新增类型、新增API、分离宏定义 |
| `src/xcmd_io.c` | 大改 | 重写backend实现、分离资源池 |
| `inc/xcmd_session.h` | 中改 | 新增RAII guard类型 |
| `src/xcmd_session.c` | 中改 | 使用RAII guard |
| `example/linux/linux_main.c` | 小改 | 使用新API |
| `example/stm32/qemu/demos/main.c` | 小改 | 更新为新API |
| `example/esp_idf/main/main.c` | 小改 | 更新为新API |

---

## 6. 二进制兼容性

**保持兼容的字段：**
```c
struct xcmd_io {
    const xcmd_io_ops_t *ops;   /* 不变 */
    void                 *handle;    /* 语义变更：指向xcmd_io_ctx_t */
    void                 *user_data; /* 不变 */
    uint8_t              type;       /* 不变 */
    uint8_t              flags;      /* 不变 */
    uint8_t              reserved[2];/* 不变 */
};
```

结构体大小和偏移不变，现有代码无需重新编译。

---

## 7. 使用示例

### 7.1 新API（推荐）

```c
/* 多session支持 */
xcmd_io_term_ctx_t term1_ctx, term2_ctx;
xcmd_session_t sess1, sess2;

xcmd_io_t *io1 = XCMD_IO_TERM_STATIC(&term1_ctx, get_char1, put_char1);
xcmd_io_t *io2 = XCMD_IO_TERM_STATIC(&term2_ctx, get_char2, put_char2);

xcmd_session_init(&sess1, io1, xcmd_term_get_ansi_ops());
xcmd_session_init(&sess2, io2, xcmd_term_get_ansi_ops());

/* 带poll的非阻塞终端 */
xcmd_io_t *io3 = XCMD_IO_TERM_STATIC_EX(&term3_ctx, 
                                         get_char3, put_char3, uart_poll3);
```

### 7.2 Legacy API（向后兼容）

```c
/* 旧代码无需修改 */
void main(void) {
    xcmd_init(my_get_char, my_put_char);  /* 正常工作 */
    while (1) {
        xcmd_task();
    }
}
```

---

## 8. 迁移指南

1. **立即迁移**：将 `xcmd_io_term_create()` 替换为 `XCMD_IO_TERM_STATIC()`
2. **可选迁移**：实现 `poll()` 回调以支持非阻塞模式
3. **更新示例**：STM32/ESP32示例改用 `xcmd_session_init()` + `xcmd_session_task()`

---

## 9. 测试计划

| 测试项 | 方法 |
|--------|------|
| 多session支持 | 创建2个session，独立输入输出 |
| 重定向资源泄漏 | 命令执行中途ctrl+C，验证资源释放 |
| 非阻塞poll | mock poll返回0，验证不占用CPU |
| 新API编译运行 | Linux/STM32示例使用新API编译运行 |
| 错误码语义 | mock read返回-1/0/正数，验证行为正确 |
