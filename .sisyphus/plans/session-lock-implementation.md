# 会话锁实现计划

## TL;DR

> **目标**: 在 `g_current_session` 访问点添加 `XCMD_SESSION_LOCK/UNLOCK` 调用
>
> **修改范围**: `src/xcmd_session.c` 中的 `xcmd_session_exec()` 函数
>
> **工作量**: 极小 (单一函数修改)

---

## Context

### 问题
`g_current_session` 是全局静态指针，在多线程/多会话环境下不安全。已定义了锁宏 `XCMD_SESSION_LOCK/UNLOCK`，但实际代码中未使用。

### 需要保护的位置
1. `xcmd_session_exec()` - 设置和清除 `g_current_session`

### 不需要修改的位置
以下函数在命令执行期间被调用，假设锁已被持有：
- `xcmd_session_history_*_current()` - 读取 g_current_session
- `xcmd_session_get_current()` - 读取 g_current_session
- `xcmd_session_print_current()` - 读取 g_current_session

---

## TODOs

- [ ] 1. 在 xcmd_session_exec 中添加锁调用

  **What to do**:
  修改 `src/xcmd_session.c` 中的 `xcmd_session_exec()` 函数：
  - 在 `g_current_session = sess;` 之前调用 `XCMD_SESSION_LOCK(sess);`
  - 在 `g_current_session = NULL;` 之后调用 `XCMD_SESSION_UNLOCK(sess);`
  
  **Code change**:
  ```c
  int xcmd_session_exec(xcmd_session_t *sess, char *str) {
      if (!sess || !str) {
          return -1;
      }
      
      int param_num = 0;
      char *cmd_param_buff[XCMD_PARAM_MAX_NUM];
      char temp[XCMD_LINE_MAX_LENGTH];
      
      XCMD_SESSION_LOCK(sess);
      g_current_session = sess;
      strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
      temp[XCMD_LINE_MAX_LENGTH - 1] = '\0';
      param_num = session_get_param(temp, " ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
      
      if (param_num > 0) {
          int ret = session_cmd_match(sess, param_num, cmd_param_buff);
          g_current_session = NULL;
          XCMD_SESSION_UNLOCK(sess);
          return ret;
      }
      
      g_current_session = NULL;
      XCMD_SESSION_UNLOCK(sess);
      return -1;
  }
  ```

  **References**:
  - `inc/xcmd_default_confg.h:47-55` - 锁宏定义
  - `src/xcmd_session.c:280-302` - 当前实现

  **Acceptance Criteria**:
  - [ ] 编译通过无错误
  - [ ] `XCMD_SESSION_LOCK(sess);` 在设置 `g_current_session` 前调用
  - [ ] `XCMD_SESSION_UNLOCK(sess);` 在清除 `g_current_session` 后调用
  - [ ] 两个返回路径都有 UNLOCK

  **Commit**: YES
  - Message: `feat(session): add lock calls around g_current_session access`
  - Files: `src/xcmd_session.c`

---

## Verification

```bash
cd example/linux && make clean && make
# 编译成功

grep -n "XCMD_SESSION_LOCK" src/xcmd_session.c
# 应显示调用位置
```