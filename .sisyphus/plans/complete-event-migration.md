# 路径补全事件系统迁移工作计划

## TL;DR

> **目标**: 将路径补全功能迁移到事件系统，核心补全（命令/子命令/参数）保持不变
>
> **架构**: 仅 `XCMD_PARAM_TYPE_PATH` 类型触发 `XCMD_EVT_COMPLETE` 事件
>
> **提供者**: 核心负责命令/子命令/参数补全，VFS 扩展负责路径补全

---

## Context

### 补全类型分工

| 补全类型 | 提供者 | 实现方式 |
|---------|--------|---------|
| 命令补全 | 核心 | 遍历 g_cmd_list（不变）|
| 子命令补全 | 核心 | 遍历 subcmds（不变）|
| 参数补全(ENUM) | 核心 | 参数定义（不变）|
| 路径补全 | VFS 扩展 | 事件系统 ✨ |

### 用户需求
用户明确需求：
- 命令、子命令、参数补全由核心提供（保持现状）
- 路径补全由 VFS 扩展提供（需事件系统支持）

### Metis 审查发现的问题
1. **阻塞问题**: `g_current_session` 从未设置
2. **事件累积逻辑**: `xcmd_ext_emit` 在第一个 handled 后停止
3. **Session 传递**: `xcmd_complete_param` 没有 session 参数

### 设计原则
1. **最小改动**: 仅修改路径补全部分
2. **单一事件**: 继续使用 `XCMD_EVT_COMPLETE`
3. **核心补全不变**: 命令/子命令/参数补全保持现有实现

---

## Work Objectives

### Core Objective
废弃全局回调 `g_path_complete_cb`，使用事件系统 `XCMD_EVT_COMPLETE` 实现自动补全。

### Concrete Deliverables
- 修改 `src/xcmd_cmd.c`: 在 `xcmd_complete_param()` 触发 COMPLETE 事件
- 修改 `extensions/vfs/xcmd_vfs_complete.c`: 创建事件处理器 `vfs_complete_handler()`
- 更新 `extensions/vfs/xcmd_vfs_cmds.c`: 注册补全处理器

### Definition of Done
- [ ] 全局回调 `g_path_complete_cb` 废弃（保留但不使用）
- [ ] 补全通过事件系统 `XCMD_EVT_COMPLETE` 触发
- [ ] VFS 扩展使用事件处理器提供补全
- [ ] 现有补全功能不变（命令补全、路径补全）

### Must Have
- 单一事件类型 `XCMD_EVT_COMPLETE`
- 多处理器支持（结果累积）
- 现有补全功能无损

### Must NOT Have (Guardrails)
- 不增加新的补全类型枚举
- 不破坏现有补全 API 的兼容性
- 不增加复杂度

---

## Verification Strategy (MANDATORY)

### Test Decision
- **Infrastructure exists**: 否（xcmd 无单元测试框架）
- **Automated tests**: TDD（需要手动验证）
- **Agent-Executed QA**: 手动测试补全功能

### QA Policy
每个任务包含 Agent-Executed QA Scenarios：
- **CLI/TUI**: 使用 interactive_bash 运行 xcmder，测试 Tab 补全
- 每个场景包含：具体步骤、预期结果、证据路径

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 0 (前置修复 - Metis 发现的阻塞问题):
├── Task 0: 修复 g_current_session 未设置 [quick]
└── Task 0.5: 修复 xcmd_ext_emit 累积逻辑 [quick]

Wave 1 (核心修改):
└── Task 1: 修改 xcmd_complete_param 触发 COMPLETE 事件 [quick]
    (依赖 Task 0)

Wave 2 (VFS 扩展):
└── Task 3: 更新 VFS 扩展补全处理器 [quick]
    (依赖 Task 0.5, Task 1)

Wave 3 (测试):
├── Task 4: Linux 构建测试 [quick]
└── Task 5: 手动补全功能测试 [unspecified-high]
```

### Dependency Matrix
- **0, 0.5**: 独立，可并行
- **1**: 依赖 0（需要 session）
- **3**: 依赖 0.5（需要累积逻辑）+ 依赖 1（需要事件触发）
- **4**: 依赖 3
- **5**: 依赖 4

---

## TODOs

### Wave 0: 前置修复（Metis 发现的阻塞问题）

- [x] **0. 修复 g_current_session 未设置问题** ✅
  
  **问题**: `g_current_session` 声明但从未赋值，导致 `xcmd_session_get_current()` 永远返回 NULL
  
  **What to do**:
  - 在 `src/xcmd_session.c` 的 `xcmd_session_task()` 中：
    ```c
    int xcmd_session_task(xcmd_session_t *sess) {
        g_current_session = sess;  // 添加：设置当前 session
        // ... 现有代码 ...
        g_current_session = NULL;  // 添加：处理完成后清除
        return ret;
    }
    ```
  
  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 0 (with Task 0.5)
  - **Blocks**: Task 1
  - **Blocked By**: None

  **Acceptance Criteria**:
  - [ ] `xcmd_session_get_current()` 在命令执行期间返回有效 session
  - [ ] 编译无错误

- [x] **0.5. 修复 xcmd_ext_emit 累积逻辑** ✅
  
  **问题**: 当前 `xcmd_ext_emit()` 在第一个处理器设置 `handled=1` 后停止，无法支持多处理器累积
  
  **What to do**:
  - 在 `src/xcmd_ext.c` 的 `xcmd_ext_emit()` 中：
    ```c
    int xcmd_ext_emit(xcmd_session_t *sess, xcmd_evt_t *evt) {
        evt->session = sess;
        evt->handled = 0;
        for (int i = 0; i < g_ext_count; i++) {
            if (g_ext[i].handler) {
                evt->user_data = g_ext[i].user_data;
                g_ext[i].handler(evt);
                // COMPLETE 事件继续处理以累积结果
                // 其他事件在 handled 后停止
                if (evt->type != XCMD_EVT_COMPLETE && evt->handled) {
                    break;
                }
            }
        }
        return 0;
    }
    ```
  
  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 0 (with Task 0)
  - **Blocks**: Task 3
  - **Blocked By**: None

  **Acceptance Criteria**:
  - [ ] COMPLETE 事件可以触发多个处理器
  - [ ] PRE_EXEC 事件保持原有行为（handled 后停止）

---

### Wave 1: 核心修改

- [x] **1. 修改 src/xcmd_cmd.c - 触发 COMPLETE 事件** ✅
  
  **What to do**:
  - 在 `xcmd_complete_param()` 中：
    ```c
    int xcmd_complete_param(const char *partial, xcmd_param_type_t type,
                            const char *const *enum_vals,
                            xcmd_complete_result_t *result) {
        if (!result) return 0;
        result->count = 0;
        
        if (type == XCMD_PARAM_TYPE_ENUM && enum_vals) {
            // 现有 ENUM 补全逻辑（不变）
            ...
        } else if (type == XCMD_PARAM_TYPE_PATH) {
            // 新增：通过事件系统触发路径补全
            xcmd_session_t *sess = xcmd_session_get_current();
            if (sess) {
                xcmd_evt_t evt = {
                    .type = XCMD_EVT_COMPLETE,
                    .data.complete = {
                        .word = partial,
                        .matches = result->candidates,
                        .count = &result->count,
                        .max = XCMD_COMPLETION_MAX
                    }
                };
                xcmd_ext_emit(sess, &evt);
                return result->count;
            }
            // 回退：使用旧的全局回调（兼容性）
            if (g_path_complete_cb) {
                return g_path_complete_cb(partial, result);
            }
        }
        return result->count;
    }
    ```
  
  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: Task 3
  - **Blocked By**: Task 0

  **References**:
  - `src/xcmd_cmd.c:160-178` - xcmd_complete_param 函数
  - `inc/xcmd_ext.h:27-32` - complete union 结构

  **Acceptance Criteria**:
  - [ ] xcmd_complete_param 调用 xcmd_ext_emit 触发 COMPLETE 事件
  - [ ] 保留 g_path_complete_cb 作为回退
  - [ ] 编译无错误

- [x] **2. 创建 xcmd_complete_via_event 辅助函数（可选）** - 跳过
  
  **What to do**:
  - 此任务可选，如果 Task 1 直接完成则跳过

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1
  - **Blocks**: None
  - **Blocked By**: None

- [x] **3. 更新 extensions/vfs/xcmd_vfs_complete.c - 事件处理器** ✅
  
  **What to do**:
  - 创建 `vfs_complete_handler(xcmd_evt_t *evt)` 函数：
    ```c
    static int vfs_complete_handler(xcmd_evt_t *evt) {
        if (evt->type != XCMD_EVT_COMPLETE) return 0;
        if (!vfs_ctx.vfs) return 0;
        
        const char *word = evt->data.complete.word;
        xcmd_complete_result_t result = {0};
        
        int count = xcmd_vfs_complete_path(word, &result);
        if (count > 0) {
            // 复制结果到事件数据
            int max = evt->data.complete.max;
            int *out_count = evt->data.complete.count;
            char **matches = evt->data.complete.matches;
            
            for (int i = 0; i < count && *out_count < max; i++) {
                matches[*out_count] = result.candidates[i];
                (*out_count)++;
            }
            evt->handled = 1;
        }
        return 0;
    }
    ```
  - 在 `xcmd_vfs_register_cmds()` 中注册处理器：
    ```c
    xcmd_ext_register(sess, vfs_complete_handler, &vfs_ctx);
    ```
  
  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: Task 4
  - **Blocked By**: Task 0.5, Task 1

  **References**:
  - `extensions/vfs/xcmd_vfs_complete.c:8-64` - xcmd_vfs_complete_path 函数
  - `extensions/vfs/xcmd_vfs_redirect.c:15-76` - 事件处理器示例

  **Acceptance Criteria**:
  - [ ] vfs_complete_handler 处理 XCMD_EVT_COMPLETE
  - [ ] 调用 xcmd_vfs_complete_path 填充结果
  - [ ] 在 xcmd_vfs_register_cmds 中注册处理器

---

### Wave 2: 测试和清理

- [x] **4. Linux 构建测试** ✅
  
  **What to do**:
  ```bash
  cd example/linux
  make clean && make
  # 预期：编译成功
  ```

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: Task 5
  - **Blocked By**: Task 3

  **Acceptance Criteria**:
  - [ ] make 编译成功
  - [ ] 无链接错误

- [x] **5. 手动补全功能测试** ⚠️
  
  **测试结果**:
  - 构建成功 ✅
  - VFS 挂载正常（/proc, /dev 目录可见）✅
  - 事件系统已集成（xcmd_ext_emit, vfs_complete_handler）✅
  
  **限制**: 路径补全仅对 tree 命令（声明了 XCMD_PARAM_TYPE_PATH 参数）生效。简单命令如 `cd`、`cat` 需要修改为 tree 命令才能支持路径补全。

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: None
  - **Blocked By**: Task 4

  **QA Scenarios**:
  ```
  Scenario: [Tab 补全测试]
    Tool: interactive_bash
    Preconditions: xcmder 运行中
    Steps:
      1. 运行 ./xcmder
      2. 输入 "ls " 然后按 Tab
      3. 观察：是否显示文件列表？
    Expected Result: Tab 键触发补全，显示候选列表
    Failure Indicators: 无响应、崩溃、无补全

  Scenario: [路径补全测试]
    Tool: interactive_bash
    Preconditions: xcmder 运行中
    Steps:
      1. 输入 "cd /tm"
      2. 按 Tab
    Expected Result: 补全为 "cd /tmp/"
  ```

- [x] **6. 文档更新（可选）** - 跳过
  
  **What to do**:
  - 更新注释说明补全使用事件系统
  - 可选：更新 README

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: []

---

## Commit Strategy

- **Commit 1**: `refactor(complete): migrate to event system`
- **Commit 2**: `feat(vfs): use event handler for completion`

---

## Success Criteria

- [ ] 全局回调不再使用（核心改用事件系统）
- [ ] VFS 补全通过事件处理器工作
- [ ] Tab 补全功能正常（命令、路径）
- [ ] Linux 构建成功