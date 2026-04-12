# xcmd 架构改进工作计划

## TL;DR

> **快速总结**: 重构xcmd项目架构，删除冗余模块(fs_cmds, ex_keys)，修复安全隐患(缓冲区溢出)，添加线程安全保护机制。
>
> **交付物**:
> - 删除 `extensions/fs_cmds/` 目录
> - 删除 `extensions/ex_keys/` 目录
> - 修复 `session_strpbrk` 边界问题
> - 将 `sprintf` 改为 `snprintf`
> - 添加会话锁保护宏
> - 修复拼写错误
>
> **预计工作量**: 中等
> **并行执行**: 是 - 4个独立任务波次
> **关键路径**: 删除fs_cmds → 更新CMake → 验证构建

---

## Context

### 原始请求
用户要求分析MCU嵌入式命令行项目的架构质量，关注：冗余设计、过度封装、代码可读性、性能问题、内存问题。

### 分析发现
通过三个并行analyze任务发现：
1. **严重问题**: `fs_cmds`与`vfs_cmds`命令重复
2. **问题**: `ex_keys`目录为空
3. **安全隐患**: `session_strpbrk`边界问题，`sprintf`无边界检查
4. **线程安全**: 全局变量`g_current_session`竞态条件
5. **代码质量**: 多处拼写错误

### 用户决策
| 问题 | 决策 |
|------|------|
| fs_cmds vs VFS | 删除fs_cmds |
| 全局状态 | 加锁保护宏（暂为空） |
| 配置系统 | 保持不变 |
| session_strpbrk | 增加len参数 |
| sprintf | 改成snprintf |
| ex_keys | 删除 |
| parse_redirect_target | 下次讨论 |

---

## Work Objectives

### 核心目标
清理冗余代码，修复安全隐患，添加线程安全基础设施。

### 具体交付物
- [ ] 删除 `extensions/fs_cmds/` 目录及所有引用
- [ ] 删除 `extensions/ex_keys/` 目录
- [ ] 修复 `session_strpbrk` 函数边界问题
- [ ] 将所有 `sprintf` 调用改为 `snprintf`
- [ ] 添加 `XCMD_SESSION_LOCK/UNLOCK` 宏定义
- [ ] 修复拼写错误
- [ ] 修复 `strncpy` 终止符问题

### 验收标准
- 所有平台编译通过（Linux默认）
- 无新增编译警告
- 原有命令仍正常工作
- 锁宏可编译且默认为空实现

### Must Have
- fs_cmds 和 ex_keys 完全删除
- session_strpbrk 增加 len 参数
- sprintf → snprintf 替换
- 锁宏定义

### Must NOT Have (Guardrails)
- 不修改 VFS 核心实现
- 不修改配置系统
- 不实现 parse_redirect_target
- 不破坏现有API兼容性

---

## Verification Strategy

### Test Decision
- **Infrastructure exists**: 无专门测试框架
- **Automated tests**: 无（代码库无测试目录）
- **Agent-Executed QA**: 每个任务包含构建验证场景

### QA Policy
- **CLI/QEMU**: 使用编译命令验证构建成功
- **Evidence**: 保存构建日志到 `.sisyphus/evidence/`

---

## Execution Strategy

### Pre-Flight Checklist (执行前检查)

在开始Wave 1之前，先执行以下检查确认基准状态：

```bash
# 统计 fs_cmds 引用数量
grep -rn "fs_cmds" --include="*.c" --include="*.h" | wc -l
grep -rn "fs_cmds" --include="Makefile" | wc -l

# 统计待删除文件数量
find extensions/fs_cmds -type f | wc -l
find extensions/ex_keys -type f 2>/dev/null | wc -l

# 确认 session_strpbrk 调用点
grep -n "session_strpbrk" src/xcmd_session.c | wc -l

# 确认 sprintf 调用（项目代码，排除第三方）
grep -rn "sprintf\s*(" --include="*.c" src/ extensions/ | grep -v FreeRTOS | wc -l
```

### Parallel Execution Waves

```
Wave 1 (立即开始 - 删除冗余模块):
├── Task 1: 删除 fs_cmds 目录 [quick]
├── Task 2: 删除 ex_keys 目录 [quick]
└── Task 3: 更新 Makefile 移除删除的模块 [quick]

Wave 2 (Wave 1后 - 安全修复):
├── Task 4: 修复 session_strpbrk 增加 len/delim_len 参数 [unspecified-high]
└── Task 5: sprintf → snprintf 替换 [quick]

Wave 3 (Wave 2后 - 基础设施与清理):
├── Task 6: 添加会话锁保护宏 [quick]
├── Task 7: 修复拼写错误 (8个文件) [quick]
└── Task 8: 修复 strncpy 终止符问题 [quick]

Wave FINAL (全部完成后):
├── Task F1: 构建验证 (oracle)
├── Task F2: 代码质量检查 (unspecified-high)
├── Task F3: 功能回归测试 (unspecified-high)
└── Task F4: 范围忠实检查 (deep)
```

### Critical Path
Task 1/2 → Task 3 → Task 4/5/6/7/8 → F1-F4 → 用户确认

### Dependency Matrix

| Task | Depends On | Blocks |
|------|-----------|--------|
| T1, T2 | - | T3 |
| T3 | T1, T2 | T4, T5, T6, T7, T8 |
| T4, T5 | T3 | F1 |
| T6, T7, T8 | T3 | F1 |
| F1-F4 | All above | User approval |

### Agent Dispatch Summary

| Wave | Tasks | Agents |
|------|-------|--------|
| 1 | T1, T2, T3 | quick, quick, quick |
| 2 | T4, T5 | unspecified-high, quick |
| 3 | T6, T7, T8 | quick, quick, quick |
| Final | F1, F2, F3, F4 | oracle, unspecified-high, unspecified-high, deep |

---

## TODOs

- [ ] 1. 删除 fs_cmds 目录

  **What to do**:
  - 删除 `extensions/fs_cmds/` 整个目录
  - 搜索并移除所有对 fs_cmds 的引用（头文件包含、初始化调用等）
  - 更新 AGENTS.md 移除 fs_cmds 相关描述

  **Must NOT do**:
  - 不要修改 VFS 模块
  - 不要修改其他扩展模块

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []
  - 删除文件是简单操作

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Task 2, 3)
  - **Blocks**: 无
  - **Blocked By**: 无

  **References**:
  - `extensions/fs_cmds/` - 要删除的目录
  - `example/linux/linux_main.c:487` - 可能引用 fs_cmds_init
  - `example/stm32/*/main.c` - 检查是否引用

  **Acceptance Criteria**:
  - [ ] fs_cmds 目录已删除
  - [ ] 无代码引用 fs_cmds
  - [ ] grep找不到 fs_cmds 相关引用

  **QA Scenarios**:
  ```
  Scenario: 验证fs_cmds已完全删除
    Tool: Bash
    Steps:
      1. find . -name "fs_cmds*" -type f 返回空
      2. grep -r "fs_cmds" --include="*.c" --include="*.h" 返回空
      3. grep -r "fs_cmds_init" --include="*.c" 返回空
    Expected Result: 所有搜索返回空
    Evidence: .sisyphus/evidence/task-1-fs-cmds-deleted.log
  ```

  **Commit**: YES
  - Message: `refactor: remove redundant fs_cmds module (use VFS instead)`
  - Files: `extensions/fs_cmds/`, 相关引用文件

---

- [ ] 2. 删除 ex_keys 目录

  **What to do**:
  - 删除 `extensions/ex_keys/` 整个目录
  - 确认该目录确实为空或仅含空文件
  - 搜索并移除相关引用

  **Must NOT do**:
  - 不影响其他扩展模块

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Task 1, 3)
  - **Blocks**: 无
  - **Blocked By**: 无

  **References**:
  - `extensions/ex_keys/` - 要删除的目录

  **Acceptance Criteria**:
  - [ ] ex_keys 目录已删除
  - [ ] 无代码引用 ex_keys

  **QA Scenarios**:
  ```
  Scenario: 验证ex_keys已删除
    Tool: Bash
    Steps:
      1. find . -name "ex_keys*" -type d 返回空
      2. grep -r "ex_keys" --include="*.c" --include="*.h" 返回空
    Expected Result: 所有搜索返回空
    Evidence: .sisyphus/evidence/task-2-ex-keys-deleted.log
  ```

  **Commit**: YES (与Task 1合并)

---

- [ ] 3. 更新构建配置

  **What to do**:
  - 更新 `example/linux/Makefile` 移除 fs_cmds 相关内容:
    - 行 22-25: FatFs 源文件引用
    - 行 34-35: 头文件包含路径
    - 行 52: 构建目录创建
    - 行 65: 编译标志中的 `-I$(EXT_DIR)/fs_cmds`
  - 检查其他 example 目录的构建文件
  - 验证编译通过

  **Must NOT do**:
  - 不修改 VFS 相关构建配置
  - 不修改第三方库（FreeRTOS等）

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO (依赖 Task 1, 2 完成)
  - **Parallel Group**: Wave 1 (需要等待 Task 1, 2)
  - **Blocks**: Wave 2
  - **Blocked By**: Task 1, Task 2

  **References**:
  - `example/linux/Makefile` - Linux 示例构建（关键！）
  - `CMakeLists.txt` - 主构建文件（如果存在）
  - `example/arduino/` - Arduino 构建配置
  - `example/stm32/` - STM32 构建配置

  **Acceptance Criteria**:
  - [ ] Makefile 中无 fs_cmds 引用
  - [ ] Linux 平台编译通过
  - [ ] 无构建错误或警告

  **QA Scenarios**:
  ```
  Scenario: Makefile清理验证
    Tool: Bash
    Steps:
      1. grep -n "fs_cmds" example/linux/Makefile 返回空
      2. grep -n "ex_keys" example/linux/Makefile 返回空
    Expected Result: 无残留引用
    Evidence: .sisyphus/evidence/task-3-makefile-clean.log

  Scenario: Linux构建验证
    Tool: Bash
    Steps:
      1. cd example/linux && make clean
      2. make
      3. 检查退出码为0
    Expected Result: 编译成功，退出码0
    Evidence: .sisyphus/evidence/task-3-build-success.log
  ```

  **Commit**: YES (与 Task 1, 2 合并)

---

- [ ] 4. 修复 session_strpbrk 边界问题

  **What to do**:
  - 修改 `src/xcmd_session.c` 中的 `session_strpbrk` 函数
  - 增加 `len` 参数用于边界检查
  - 增加 `delim_len` 参数（或确保 delim 是有效C字符串）
  - 检查内部字符串移动操作的边界
  - 更新所有调用处（约3-4处）传入正确的长度

  **Current Issue** (line 61-90):
  ```c
  static char *session_strpbrk(char *s, const char *delim) {
      for (uint16_t i = 0; s[i]; i++) {      // 无边界检查
          for (uint16_t j = 0; delim[j]; j++) {
              ...
              for (uint16_t k = i; s[k]; k++) {
                  s[k] = s[k + 1];           // 潜在越界
              }
  ```

  **Fix Pattern**:
  ```c
  static char *session_strpbrk(char *s, size_t len, const char *delim, size_t delim_len);
  // 内部循环需检查 i < len && k < len - 1
  ```

  **Must NOT do**:
  - 不改变函数的外部行为（保持语义一致）
  - 不增加动态内存分配

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
  - **Skills**: []
  - 需要仔细的边界检查逻辑

  **Parallelization**:
  - **Can Run In Parallel**: NO (依赖 Task 3 验证构建)
  - **Parallel Group**: Wave 2 (with Task 5)
  - **Blocks**: Task F1
  - **Blocked By**: Task 3

  **References**:
  - `src/xcmd_session.c:61-90` - session_strpbrk 函数定义
  - `src/xcmd_session.c:205,224,233` - 可能的调用处
  - 调用处需搜索 `session_strpbrk` 找到所有位置

  **Acceptance Criteria**:
  - [ ] 函数签名包含 `len` 参数（建议 size_t 或 uint16_t）
  - [ ] 内部循环有边界检查 `i < len`
  - [ ] 字符串移动操作有边界检查
  - [ ] 所有调用处更新
  - [ ] 编译无警告

  **QA Scenarios**:
  ```
  Scenario: 边界检查验证
    Tool: Bash
    Steps:
      1. grep -n "session_strpbrk" src/xcmd_session.c
      2. 确认函数签名已更新
      3. 编译项目，检查无警告
    Expected Result: 函数有len参数，编译通过
    Evidence: .sisyphus/evidence/task-4-boundary-fix.log
  ```

  **Commit**: YES
  - Message: `fix(security): add length parameters to session_strpbrk for boundary safety`
  - Files: `src/xcmd_session.c`

---

- [ ] 5. sprintf → snprintf 替换

  **What to do**:
  - 搜索所有项目代码中的 `sprintf` 调用（排除第三方库）
  - 替换为 `snprintf`，添加缓冲区大小参数
  - 确保 `snprintf` 的 size 参数正确（通常是 sizeof(buf) 或 缓冲区大小）

  **Scope Clarification**:
  | 文件 | 状态 | 处理 |
  |------|------|------|
  | `fs_cmds/fs_cmds.c` | 将删除 | 跳过 |
  | `FreeRTOS/tasks.c` | 第三方库 | 不修改 |
  | `example/*/myprintk.c` | 示例代码 | **需确认** |

  **Must NOT do**:
  - 不修改第三方库代码
  - 不改变输出格式
  - 不增加栈使用（使用现有缓冲区大小）

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Task 4)
  - **Blocks**: Task F1
  - **Blocked By**: Task 3

  **References**:
  - 搜索命令: `grep -rn "sprintf\s*(" --include="*.c" src/ extensions/ | grep -v FreeRTOS`
  - 注意：fs_cmds.c 已删除，无需处理

  **Acceptance Criteria**:
  - [ ] 项目源码中无 `sprintf` 调用（排除第三方库）
  - [ ] 所有 `snprintf` 调用有正确的 size 参数

  **QA Scenarios**:
  ```
  Scenario: 验证无危险sprintf调用
    Tool: Bash
    Steps:
      1. grep -rn "sprintf\s*(" --include="*.c" src/ extensions/
      2. 排除 FreeRTOS 目录后确认返回空
    Expected Result: 无危险的sprintf调用
    Evidence: .sisyphus/evidence/task-5-snprintf-conversion.log
  ```

  **Commit**: YES
  - Message: `fix(security): replace sprintf with snprintf for buffer safety`
  - Files: 所有包含 sprintf 的项目源文件

---

- [ ] 6. 添加会话锁保护宏

  **What to do**:
  - 在 `inc/xcmd_default_confg.h` 中定义锁宏（配置文件位置更合适）
  - 默认实现为空（允许编译通过）
  - 添加注释说明用户可自定义实现

  **Definition Location**: 选择 `xcmd_default_confg.h` 而非 `xcmd_session.h`，因为：
  - 这是配置文件的惯例位置
  - 用户可能需要根据平台自定义实现
  - 与 `XCMD_NO_DEPRECATED` 等其他配置宏保持一致

  **Usage Locations** (g_current_session 访问点):
  - `src/xcmd_session.c:281,287,291` - in xcmd_session_exec()
  - `src/xcmd_session.c:433-444` - history functions
  - `src/xcmd_session.c:451-453` - get_current
  - `src/xcmd_session.c:455-465` - print_current

  **Implementation**:
  ```c
  // xcmd_default_confg.h
  #ifndef XCMD_SESSION_LOCK
  #define XCMD_SESSION_LOCK(sess)   ((void)0)
  #endif
  #ifndef XCMD_SESSION_UNLOCK
  #define XCMD_SESSION_UNLOCK(sess) ((void)0)
  #endif
  ```

  **Must NOT do**:
  - 不实现实际的锁逻辑（由用户自行实现）
  - 不破坏单线程使用场景
  - 不在本次修改中添加锁调用点（下次讨论）

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Task 7, 8)
  - **Blocks**: Task F1
  - **Blocked By**: Task 3

  **References**:
  - `inc/xcmd_default_confg.h` - 配置文件（定义位置）
  - `src/xcmd_session.c:14` - g_current_session 定义处

  **Acceptance Criteria**:
  - [ ] `XCMD_SESSION_LOCK(sess)` 和 `XCMD_SESSION_UNLOCK(sess)` 宏已定义
  - [ ] 默认为空实现（无操作）
  - [ ] 有注释说明用途
  - [ ] 编译通过无警告

  **QA Scenarios**:
  ```
  Scenario: 验证锁宏定义
    Tool: Bash
    Steps:
      1. grep -n "XCMD_SESSION_LOCK" inc/xcmd_default_confg.h
      2. grep -n "XCMD_SESSION_UNLOCK" inc/xcmd_default_confg.h
      3. 编译验证宏可展开
    Expected Result: 宏定义存在且编译通过
    Evidence: .sisyphus/evidence/task-6-lock-macro.log
  ```

  **Commit**: YES
  - Message: `feat(thread): add session lock macros for future thread safety`
  - Files: `inc/xcmd_default_confg.h`

---

- [ ] 7. 修复拼写错误

  **What to do**:
  - 修复 `PROMPT_CLOLR` → `PROMPT_COLOR`（8个文件）
  - 文件列表：
    1. `inc/xcmd_default_confg.h` (定义处)
    2. `example/linux/xcmd_confg.h`
    3. `example/arduino/xcmd_confg.h`
    4. `example/esp_idf/xcmd_confg.h`
    5. `example/stm32/MDK/xcmd_confg.h`
    6. `example/stm32/qemu/demos/xcmd_confg.h`
    7. `example/stm32/qemu/test/xcmd_confg.h`
    8. 其他可能存在的 xcmd_confg.h
  - 注：`xcmd_confg.h` 文件名保持不变（太多引用）
  - 注：`RESAULT_TO_STR` 在 fs_cmds.c 中，已删除，无需处理

  **Must NOT do**:
  - 不修改文件名（保持 `xcmd_confg.h`）
  - 不破坏任何现有代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Task 6, 8)
  - **Blocks**: 无
  - **Blocked By**: 无

  **References**:
  - `inc/xcmd_default_confg.h:47-48` - PROMPT_CLOLR 定义
  - 所有 `**/xcmd_confg.h` 文件

  **Acceptance Criteria**:
  - [ ] grep "PROMPT_CLOLR" 返回空
  - [ ] 所有配置文件使用 PROMPT_COLOR

  **QA Scenarios**:
  ```
  Scenario: 验证拼写修复
    Tool: Bash
    Steps:
      1. grep -rn "PROMPT_CLOLR" --include="*.h"
      2. grep -rn "PROMPT_COLOR" --include="*.h" 确认存在
    Expected Result: CLOLR 已全部修复为 COLOR
    Evidence: .sisyphus/evidence/task-7-typos-fixed.log
  ```

  **Commit**: YES
  - Message: `fix: correct spelling (PROMPT_CLOLR → PROMPT_COLOR)`
  - Files: `inc/xcmd_default_confg.h`, 所有 `xcmd_confg.h`

---

- [ ] 8. 修复 strncpy 终止符问题

  **What to do**:
  - 检查 `src/xcmd_session.c:282` 的 strncpy 使用
  - 如果 `str` 长度恰好为 `XCMD_LINE_MAX_LENGTH`，`temp` 不会以 `\0` 结尾
  - 添加显式终止符 `temp[XCMD_LINE_MAX_LENGTH - 1] = '\0';`

  **Current Issue**:
  ```c
  // src/xcmd_session.c:282
  strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
  // 缺少: temp[XCMD_LINE_MAX_LENGTH - 1] = '\0';
  ```

  **Must NOT do**:
  - 不改变函数行为（仅添加安全终止符）
  - 不增加缓冲区大小

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Task 6, 7)
  - **Blocks**: Task F1
  - **Blocked By**: Task 3

  **References**:
  - `src/xcmd_session.c:282` - strncpy 调用位置
  - 搜索其他可能的 strncpy 问题

  **Acceptance Criteria**:
  - [ ] strncpy 后有显式终止符
  - [ ] 编译无警告

  **QA Scenarios**:
  ```
  Scenario: 验证strncpy安全
    Tool: Bash
    Steps:
      1. grep -n "strncpy" src/xcmd_session.c
      2. 检查后续行是否有终止符赋值
    Expected Result: 所有strncpy后有终止符
    Evidence: .sisyphus/evidence/task-8-strncpy-fix.log
  ```

  **Commit**: YES
  - Message: `fix(security): ensure null termination after strncpy`
  - Files: `src/xcmd_session.c`

---

## Final Verification Wave

> **注意**: 所有F任务完成后，汇总结果并请求用户确认。只有用户明确说"OK"后才能标记计划完成。

- [ ] F1. **构建验证** — `oracle`
  
  **What to do**:
  - 验证 Linux 平台编译通过
  - 检查无新增编译错误/警告
  - 保存完整构建日志
  
  **Evidence Requirements**:
  ```bash
  cd example/linux && make clean && make 2>&1 | tee .sisyphus/evidence/final-build.log
  # 检查退出码为0，无错误输出
  ```
  
  **Pass Criteria**:
  - [ ] 构建退出码为 0
  - [ ] 无编译错误
  - [ ] 无新增警告（比之前不增加）
  - [ ] `.sisyphus/evidence/final-build.log` 已保存

- [ ] F2. **代码质量检查** — `unspecified-high`
  
  **What to do**:
  - 检查无危险函数调用（sprintf, gets等）
  - 检查全局变量保护声明
  - 检查边界安全
  
  **Evidence Requirements**:
  ```bash
  # 危险函数检查
  grep -rn "sprintf\s*(" --include="*.c" src/ extensions/ | grep -v FreeRTOS > .sisyphus/evidence/final-danger-funcs.log
  grep -rn "gets\s*(" --include="*.c" src/ extensions/ >> .sisyphus/evidence/final-danger-funcs.log
  
  # 安全修复验证
  grep -rn "session_strpbrk" src/xcmd_session.c > .sisyphus/evidence/final-strpbrk.log
  ```
  
  **Pass Criteria**:
  - [ ] 无 sprintf 调用（排除第三方库）
  - [ ] session_strpbrk 有 len/delim_len 参数
  - [ ] 所有 strncpy 后有终止符

- [ ] F3. **功能回归测试** — `unspecified-high`
  
  **What to do**:
  - 构建并运行 Linux 示例
  - 验证基本命令功能
  
  **Evidence Requirements**:
  ```bash
  # 功能测试（如果有测试环境）
  cd example/linux && ./xcmd
  # 交互测试: help, ls, exit 等命令
  # 如果无法自动测试，记录"需要手动验证"
  ```
  
  **Pass Criteria**:
  - [ ] `help` 命令输出正常
  - [ ] 文件命令（ls等）通过 VFS 正常工作
  - [ ] 无明显运行时错误/崩溃
  - [ ] 输出保存到 `.sisyphus/evidence/final-func-test.log`

- [ ] F4. **范围忠实检查** — `deep`
  
  **What to do**:
  - 使用 git diff 确认仅修改计划范围内文件
  - 检查无意外改动
  
  **Evidence Requirements**:
  ```bash
  git diff --stat > .sisyphus/evidence/final-diff-stat.log
  git diff --name-only > .sisyphus/evidence/final-diff-files.log
  ```
  
  **Pass Criteria**:
  - [ ] 仅修改以下类型的文件:
    - `extensions/` (仅删除)
    - `src/xcmd_session.c`
    - `inc/xcmd_default_confg.h`
    - `example/*/xcmd_confg.h`
    - `example/linux/Makefile`
  - [ ] 无计划外的文件修改
  - [ ] fs_cmds 和 ex_keys 已完全移除

- [ ] F5. **用户确认** (必须步骤)
  
  **What to do**:
  - 汇总 F1-F4 结果
  - 展示关键证据（构建日志摘要、diff统计）
  - 请求用户确认
  
  **Pass Criteria**:
  - [ ] 用户确认 "OK" 或 "通过"
  - [ ] 所有问题已解决

---

## Commit Strategy

采用合并提交策略，减少提交数量：

**Commit 1**: Wave 1 完成
```
refactor: remove redundant fs_cmds and ex_keys modules, update Makefile
```
- 删除 extensions/fs_cmds/
- 删除 extensions/ex_keys/
- 更新 example/linux/Makefile

**Commit 2**: Wave 2 完成
```
fix(security): add length parameters to session_strpbrk for boundary safety
```
- 修改 src/xcmd_session.c 中 session_strpbrk函数

**Commit 3**: Wave 2-3 完成
```
fix(security): replace sprintf with snprintf, ensure strncpy null termination
```
- sprintf → snprintf 替换
- 添加 strncpy 终止符

**Commit 4**: Wave 3 完成
```
feat(thread): add session lock macros, fix spelling errors
```
- 添加 XCMD_SESSION_LOCK/UNLOCK 宏
- 修复 PROMPT_CLOLR → PROMPT_COLOR

## Success Criteria

### 验证命令
```bash
# 构建验证
cd example/linux && make clean && make

# 代码检查 - 无残留引用
grep -rn "fs_cmds" --include="*.c" --include="*.h"      # 应返回空
grep -rn "ex_keys" --include="*.c" --include="*.h"      # 应返回空

# 安全检查 - 无危险函数调用
grep -rn "sprintf\s*(" --include="*.c" src/ extensions/ | grep -v FreeRTOS  # 返回空
grep -rn "PROMPT_CLOLR" --include="*.h"                  # 应返回空

# Makefile 清理
grep -n "fs_cmds" example/linux/Makefile                 # 应返回空

# 宏定义验证
grep -n "XCMD_SESSION_LOCK" inc/xcmd_default_confg.h    # 应找到定义
```

### 最终检查清单
- [ ] fs_cmds 目录已删除
- [ ] ex_keys 目录已删除
- [ ] Makefile 中无 fs_cmds/ex_keys 引用
- [ ] session_strpbrk 有 len 和 delim_len 参数
- [ ] 无 sprintf 调用（项目源码）
- [ ] strncpy 后有终止符
- [ ] XCMD_SESSION_LOCK/UNLOCK 宏已定义
- [ ] PROMPT_CLOLR 已修复为 PROMPT_COLOR（所有文件）
- [ ] Linux 平台编译通过
- [ ] 基本命令功能正常

---

## Out of Scope (Future Work)

以下内容不在本次改进范围：
- 配置系统重构
- `parse_redirect_target` 功能实现
- VFS stub 函数完整实现
- `ex_list` 使用 malloc 问题
- 多会话完整支持