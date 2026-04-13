# Draft: 自动补全事件系统迁移

## 用户需求
将自动补全功能完全迁移到事件系统，废弃全局回调机制。
用户选择了**方案 B：完全迁移**。

## 已确认
1. **多处理器支持**: 是 - 事件按顺序触发，每个处理器可添加补全项
2. **补全类型**: 统一类型 - 不要太复杂，使用 XCMD_EVT_COMPLETE 一个事件

## 当前架构
1. **全局回调机制**：
   - `g_path_complete_cb` - 全局路径补全回调指针
   - `xcmd_register_path_complete_cb(cb)` - 注册回调函数
   - `xcmd_complete_param()` 中判断 `type == XCMD_PARAM_TYPE_PATH` 则调用回调

2. **VFS 补全实现**：
   - `xcmd_vfs_complete_path()` - VFS 路径补全函数
   - 在 `xcmd_vfs_register_cmds()` 中注册

3. **事件系统已定义**：
   - `XCMD_EVT_COMPLETE` 事件类型
   - `complete` union 成员：word, matches, count, max

## 迁移方案

### 设计原则
1. **简单优先** - 单一 COMPLETE 事件类型
2. **多处理器累积** - 所有处理器结果累积到 matches 数组
3. **不终止** - 处理器不设置 handled=1 来终止

### 核心改动
1. `xcmd_cmd.c`: 在 `xcmd_complete_param()` 中触发 COMPLETE 事件
2. 保留 `xcmd_register_path_complete_cb()` 作为简化 API（内部转事件）

### 扩展改动
1. VFS 扩展: `vfs_complete_handler()` 处理 COMPLETE 事件
2. 合并到现有的 `vfs_redirect_init()` 注册