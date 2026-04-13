# VFS 解耦工作计划

## TL;DR

> **目标**: 将所有 VFS 相关代码从核心移除，通过事件系统扩展重定向功能
>
> **架构**: 核心完成 argc/argv 解析，事件系统支持 PRE_EXEC 和 COMPLETE 两种事件
>
> **改动**: 移除~120行核心代码，新增~50行事件系统，修改扩展实现

---

## Context

### 当前问题

核心代码中存在 VFS 条件编译耦合：
- `#ifdef XCMD_USE_VFS` 散布在核心文件
- `sess->vfs` 成员直接依赖 VFS 类型
- `parse_redirect_target()` 函数硬编码 VFS 逻辑

### 设计原则

1. **argc/argv 解析由核心完成** - 不需要事件/回调
2. **重定向和自动补全使用事件系统** - 扩展介入
3. **零动态分配** - 静态数组，编译时确定
4. **精简易懂** - 最小化代码量

---

## Work Objectives

### 核心目标

1. **移除所有 VFS 条件编译** - 核心零依赖
2. **添加事件系统** - 支持 PRE_EXEC 和 COMPLETE
3. **VFS 扩展独立实现** - 重定向功能移到扩展

### Definition of Done

- [ ] 核心代码零 `#ifdef XCMD_USE_VFS`
- [ ] 核心代码零 `xcmd_vfs_t` 引用
- [ ] 事件系统正常工作
- [ ] Linux 示例 + VFS 配置编译通过
- [ ] 无 VFS 配置时核心编译通过
- [ ] 重定向功能正常（`echo hello > file.txt`）

---

## TODOs

### Wave 1: 核心清理

- [x] **1. 创建 inc/xcmd_ext.h - 事件定义** ✅
  
  已创建 `inc/xcmd_ext.h`，包含：
  - 事件类型枚举 (PRE_EXEC, COMPLETE)
  - 事件结构体 (带 union)
  - 扩展处理函数类型定义
  - 注册和触发 API

- [x] **2. 创建 src/xcmd_ext.c - 事件实现** ✅
  
  已创建 `src/xcmd_ext.c`，包含：
  - 静态扩展注册表 (g_ext)
  - xcmd_ext_register() 实现
  - xcmd_ext_emit() 实现 (初始化 handled=0)

- [x] **3. 修改 inc/xcmd_session.h - 移除 VFS 依赖** ✅
  
  已修改 `inc/xcmd_session.h`:
  - 移除 VFS 类型声明
  - 移除 vfs 成员和条件编译
  - 移除 redirect_orig_stdout, redirect_active → 替换为 saved_stdout
  - 移除 xcmd_io_redirect_guard_t 结构体
  - 移除 set_vfs/get_vfs 函数
  - 移除 recv_hook_func
  - 移除 redirect 相关函数
  - 添加 xcmd_ext.h 包含

- [x] **4. 修改 src/xcmd_session.c - 添加事件触发** ✅
  
  已修改 `src/xcmd_session.c`:
  - 移除 VFS 相关代码 (set_vfs, get_vfs, parse_redirect, session_redirected, guard_enter/exit)
  - session_cmd_match 改为接受 (sess, argc, argv)
  - xcmd_session_task 添加 PRE_EXEC 事件触发
  - xcmd_session_exec 解析后调用 session_cmd_match
  - 使用 saved_stdout 替代 redirect_orig_stdout

- [x] **5. 修改 inc/xcmd_io.h - 移除 VFS API** ✅
   
   已修改 `inc/xcmd_io.h`:
   - 移除 VFS 相关的 `xcmd_io_file_create`, `xcmd_io_file_open` 声明
   - 核心代码不再包含任何 VFS 相关的公开 API

- [x] **6. 修改 src/xcmd_io.c - 移除 VFS 实现** ✅
   
   已修改 `src/xcmd_io.c`:
   - 移除 VFS File IO Backend 实现 (`file_read`, `file_write`, `file_close`, `xcmd_io_file_create`, `xcmd_io_file_open`)
   - 仅保留 `xcmd_io_copy()` 函数

---

### Wave 2: VFS 扩展重构

- [x] **7. 创建 extensions/vfs/xcmd_vfs_io.c** ✅
   
   已创建 `extensions/vfs/xcmd_vfs_io.c`:
   - VFS 文件 IO 包装器，将 `xcmd_vfs_file_t*` 包装为 `xcmd_io_t*`
   - 静态回调实现: `vfs_file_read`, `vfs_file_write`, `vfs_file_flush`, `vfs_file_close`, `vfs_file_readable`
   - 公共 API: `xcmd_io_file_create(xcmd_vfs_file_t *file)`
   - 零动态分配 (使用静态 `xcmd_io_t io`)

- [x] **8. 修改 extensions/vfs/xcmd_vfs_redirect.c - 重定向扩展** ✅
   
   已修改 `extensions/vfs/xcmd_vfs_redirect.c`:
   - 添加 `vfs_ext_handler()` - PRE_EXEC 事件处理器
   - 查找 argv 中的 `">"`, 打开 VFS 文件, 设置 stdout 重定向
   - 添加 `vfs_redirect_init()` - 注册事件处理器
   - 保留原有的 `xcmd_vfs_redirect_open/close()` 函数
   - 更新 `xcmd_vfs_redirect.h` 声明 `vfs_redirect_init()`

- [x] **9. 更新 extensions/vfs/xcmd_vfs_cmds.c** ✅
   
   已修改 `extensions/vfs/xcmd_vfs_cmds.c`:
   - 添加 `#include "xcmd_vfs_redirect.h"`
   - 修改 `xcmd_vfs_register_cmds()` 调用 `vfs_redirect_init(sess, &g_vfs)`
   - 使用 `xcmd_session_get_current()` 获取 session

---

### Wave 3: 测试和清理

- [x] **10. 更新 example/linux/Makefile** ✅
   
   已修改 `example/linux/Makefile`:
   - 添加 `$(SRC_DIR)/xcmd_ext.c` 到 SRC 列表
   - 添加 `$(EXT_DIR)/vfs/xcmd_vfs_io.c` 到 EXT_SRC 列表

- [x] **11. 测试 VFS 功能** ✅
   
   已验证:
   - `make clean && make` 编译成功
   - 生成 `xcmder` 二进制文件

- [x] **12. 测试无 VFS 配置** ✅
   
   已验证:
   - `make CFLAGS="-g -O0 -Wall -I./ -I../../inc"` 编译成功
   - 核心代码可在无 VFS 配置下编译

---

## Verification Strategy

### 编译验证

```bash
# 核心（无 VFS）
cd example/linux
make CFLAGS=""
nm src/xcmd_session.o | grep vfs
# 预期：无输出

# 核心 + VFS
make CFLAGS="-DXCMD_USE_VFS"
nm extensions/vfs/*.o | grep session
# 预期：只有 xcmd_ext_register 等公开 API
```

### 功能验证

```bash
# 测试重定向
echo hello > /tmp/test.txt
cat /tmp/test.txt
# 预期：hello

# 测试命令
ls
cd /
pwd
# 预期：正常工作
```

---

## Commit Strategy

- **Commit 1**: `feat(core): add event system for extensions`
- **Commit 2**: `refactor(core): remove VFS dependencies from session`
- **Commit 3**: `refactor(io): remove VFS file IO from core`
- **Commit 4**: `feat(vfs): implement redirect via event system`
- **Commit 5**: `chore: update examples and makefiles`

---

## Success Criteria

- [ ] 核心无 `#ifdef XCMD_USE_VFS`
- [ ] 核心无 `xcmd_vfs_t` 引用
- [ ] 事件系统编译通过
- [ ] VFS 扩展通过事件实现重定向
- [ ] 功能测试通过
- [ ] 编译通过（核心单独 + 核心+VFS）