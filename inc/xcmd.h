#ifndef XCMD_H
#define XCMD_H

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "xcmd_default_confg.h"
#include "xcmd_define.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*cmd_func_t)(int, char**);
typedef int (*cmd_key_func_t)(void*);

typedef struct __cmd {
    const char* name;
    cmd_func_t func;
    const char* help;
#ifndef ENABLE_XCMD_EXPORT
    struct __cmd* next;
#endif
} xcmd_t;

typedef struct __key {
    const char* key;
    cmd_key_func_t func;
    const char* help;
#ifndef ENABLE_XCMD_EXPORT
    struct __key* next;
#endif
} xcmd_key_t;

/**
 * @description: 解释器初始化
 * @param {func*} get_c：获取一个字符的函数
 * @param {func*} put_c：发送一个字符的函数
 * @return {*}
 */
void xcmd_init(int (*get_c)(uint8_t*), int (*put_c)(uint8_t));

/**
 * @description: 解释器的主任务
 * @param {*}
 * @return {*}
 */
void xcmd_task(void);

/**
 * @description: 注册一组指令
 * @param {xcmd_t*} cmds：指令集
 * @param {uint16_t} number：指令个数
 * @return {int} 已经注册的指令的个数
 */
int xcmd_cmd_register(xcmd_t* cmds, uint16_t number);

/**
 * @description: 注册一组按键
 * @param {xcmd_key_t*} keys：快捷键集
 * @param {uint16_t} number：快捷键的个数
 * @return {int}：已经注册的快捷键的个数
 */
int xcmd_key_register(xcmd_key_t* keys, uint16_t number);

/**
 * @description: 删除已经注册的cmd
 * @param {char*} cmd：cmd集
 * @return {int}：0：success； !0：failed
 */
int xcmd_unregister_cmd(char* cmd);

/**
 * @description:删除已经注册的key
 * @param {char*} key：key集
 * @return {int}：0：success； !0：failed
 */
int xcmd_unregister_key(char* key);

#ifndef XCMD_SECTION
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define XCMD_SECTION(x) __attribute__((section(x)))
#elif defined(__IAR_SYSTEMS_ICC__)
#define XCMD_SECTION(x) @x
#elif defined(__GNUC__)
#define XCMD_SECTION(x) __attribute__((section(x)))
#else
#define XCMD_SECTION(x)
#endif
#endif

#ifndef XCMD_USED
#if defined(__CC_ARM) || defined(__CLANG_ARM)
#define XCMD_USED __attribute__((used))
#elif defined(__IAR_SYSTEMS_ICC__)
#define XCMD_USED __root
#elif defined(__GNUC__)
#define XCMD_USED __attribute__((used))
#else
#define XCMD_USED
#endif
#endif

#ifdef ENABLE_XCMD_EXPORT
#define XCMD_EXPORT_CMD(_name, _func, _help) XCMD_USED const xcmd_t XCMD_SECTION("_xcmd_cmd_list") \
                                                 xcmd_cmd_##_name = {                              \
                                                     .name = #_name,                               \
                                                     .func = _func,                                \
                                                     .help = _help};
#define XCMD_EXPORT_KEY(_key, _func, _help) XCMD_USED const xcmd_key_t XCMD_SECTION("_xcmd_key_list") \
                                                xcmd_key_##_key = {                                   \
                                                    .key = _key,                                      \
                                                    .func = _func,                                    \
                                                    .help = _help};
/* 导出段起止地址符号：
 * 链接器必须能提供两个导出段 "_xcmd_cmd_list"/"_xcmd_key_list" 的首尾地址。
 * 各工具链的取法不同，但都不需要手工修改 lds/sct 来定义起止符号：
 *  - Arm 编译环境 (ArmCC V5 / ArmClang AC6, armlink)：直接引用 armlink 为已放置的
 *    段自动合成的 "段名$$Base" / "段名$$Limit" 符号 (参考 letter-shell shellInit)。
 *    前提是这两个段被链接保留(通常由 sct 中的 * / .ANY 通配符覆盖)。
 *  - GCC (GNU ld)：引用 ld 为段名自动生成的 "__start_段名"/"__stop_段名" 符号
 *    (参考 letter-shell PR#175)。注意该分支必须放在 ArmClang 之后判断，
 *    因为 ArmClang 同时会定义 __GNUC__。 */
#if defined(__CC_ARM) || (defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6000000))
extern const xcmd_t _xcmd_cmd_list$$Base;
extern const xcmd_t _xcmd_cmd_list$$Limit;
extern const xcmd_key_t _xcmd_key_list$$Base;
extern const xcmd_key_t _xcmd_key_list$$Limit;
#define XCMD_CMD_FOR_EACH(pos) for ((pos) = (xcmd_t*)&_xcmd_cmd_list$$Base; \
                                    (pos) < (xcmd_t*)&_xcmd_cmd_list$$Limit; ++(pos))
#define XCMD_KEY_FOR_EACH(pos) for ((pos) = (xcmd_key_t*)&_xcmd_key_list$$Base; \
                                    (pos) < (xcmd_key_t*)&_xcmd_key_list$$Limit; ++(pos))
#elif defined(__GNUC__)
extern const xcmd_t __start__xcmd_cmd_list;
extern const xcmd_t __stop__xcmd_cmd_list;
extern const xcmd_key_t __start__xcmd_key_list;
extern const xcmd_key_t __stop__xcmd_key_list;
#define XCMD_CMD_FOR_EACH(pos) for ((pos) = (xcmd_t*)&__start__xcmd_cmd_list; \
                                    (pos) < (xcmd_t*)&__stop__xcmd_cmd_list; ++(pos))
#define XCMD_KEY_FOR_EACH(pos) for ((pos) = (xcmd_key_t*)&__start__xcmd_key_list; \
                                    (pos) < (xcmd_key_t*)&__stop__xcmd_key_list; ++(pos))
#else
#error "ENABLE_XCMD_EXPORT: unsupported compiler, please define the section start/end symbols or use command table mode"
#endif
#else
/**
 * @description: 获取命令列表，可以通过next指针可以遍历所有指令
 * @param {*}
 * @param {}
 * @return {xcmd_t *}：指令链表表头
 */
xcmd_t* xcmd_cmdlist_get(void);
/**
 * @description: 获取按键列表，可以通过next指针可以遍历所有按键
 * @param {*}
 * @param {}
 * @return {xcmd_key_t *}：快捷键链表表头
 */
xcmd_key_t* xcmd_keylist_get(void);
#define XCMD_EXPORT_CMD(name, func, help)
#define XCMD_EXPORT_KEY(key, func, help)
#define XCMD_CMD_FOR_EACH(pos) for ((pos) = xcmd_cmdlist_get(); (pos); (pos) = (pos)->next)
#define XCMD_KEY_FOR_EACH(pos) for ((pos) = xcmd_keylist_get(); (pos); (pos) = (pos)->next)
#endif

/**
 * @description: 手动执行命令
 * @param {char* } str：命令
 * @return {uint8_t}  返回执行结果
 */
int xcmd_exec(char* str);

/**
 * @description: 打印字符串
 */
void xcmd_print(const char* fmt, ...);
void xcmd_put_str(const char* str);

#ifdef XCMD_END_LINE_MODE
/**
 * @description: 尾行模式打印 (参考 letter-shell shellWriteEndLine):
 *              将内容"插入"到当前命令行上方, 命令行始终保持在终端最后一行;
 *              用于日志输出等与命令行共占用同一终端的场景
 * @param {char*} fmt 格式化字符串
 * @return 无
 * @note  输出内容必须以 \r\n 结尾 (elog 的 ELOG_NEWLINE_SIGN 已保证)
 * @note  仅 xcmd_ready() 返回 1 时调用才生效, 否则直接 return
 */
void xcmd_print_end_line(const char* fmt, ...);

/**
 * @description: 尾行模式写字符串 (非格式化版本)
 * @param {char*} str 字符串
 * @return 无
 */
void xcmd_put_str_end_line(const char* str);

/**
 * @description: 尾行模式按精确长度写 (供 elog 等带 size 的日志使用)
 * @param {char*} str 数据缓冲 (不要求 NUL 结尾)
 * @param {uint16_t} len 数据长度
 * @return 无
 * @note  日志缓冲区是复用的, 短日志后可能残留上一条长日志的尾部,
 *        必须用 len 精确输出, 不能依赖 strlen/NUL 结尾
 */
void xcmd_put_str_end_line_len(const char* str, uint16_t len);

/**
 * @description: 查询 xcmd 是否已初始化完毕 (get_c/put_c 桥接已就绪)
 * @note  供日志等外部模块在调用尾行接口前判断; 未就绪时请走普通直写输出
 * @return {uint8_t} 1=已就绪; 0=未初始化
 */
uint8_t xcmd_ready(void);
#endif

/**
 * @description: 向显示器插入一个字符
 * @param {char} c
 * @return 无
 */
void xcmd_display_insert_char(char c);

/**
 * @description: 删除显示器的一个字符
 * @param {*}
 * @return 无
 */
void xcmd_display_delete_char(void);

/**
 * @description: 返回光标当前的字符
 * @param {char*}cha存储返回的字符
 * @return {uint8_t}0光标位置无字符，1有字符
 */
uint8_t xcmd_display_current_char(char* cha);

/**
 * @description: 清除显示器
 * @param {*}
 * @return 无
 */
void xcmd_display_clear(void);

/**
 * @description: 打印提示符 (含颜色), 供需要自行重绘命令行的场合使用
 * @param {*}
 * @return 无
 */
void xcmd_display_prompt_print(void);

/**
 * @description: 重绘命令行: 擦除当前行并打印提示符。
 * 供 shell 就绪前使用——用户 main 里若在 xcmd_init 之后还有其它打印,
 * 应在所有打印结束后调用本函数, 保证提示符是屏幕上最后的内容。
 * @param {*}
 * @return 无
 */
void xcmd_display_redraw(void);

/**
 * @description: 标记命令行已由命令自行重绘 (如 clear 清屏后), 框架收尾时跳过补行+提示符。
 * 注意: 调用方需保证此时提示符已在屏幕上, 跳过收尾意味着提示符完全由命令负责。
 * @param {*}
 * @return 无
 */
void xcmd_display_redraw_set(void);

/**
 * @description: 取走重绘标志: 返回1表示命令已自行重绘, 并清零标志
 * @param {*}
 * @return {uint8_t} 1=有待处理的重绘标志; 0=无
 */
uint8_t xcmd_display_redraw_take(void);

/**
 * @description: 获取显示器的内容
 * @param {*}
 * @return {char*} *显示器的内容的指针
 */
char* xcmd_display_get(void);

/**
 * @description: 设置显示器的内容
 * @param {char*} 要现实的内容
 * @return 无
 */
void xcmd_display_print(const char* fmt, ...);
void xcmd_display_write(const char* buf, uint16_t len);

/**
 * @description: 光标操作函数
 * @param {*}
 * @return {*}
 */
void xcmd_display_cursor_set(uint16_t pos);
uint16_t xcmd_display_cursor_get(void);

/**
 * @description: 设置命令行提示字符串，此函数并不拷贝字符串，只是记住了传入的指针
 * @param {char*} prompt
 * @return {*}
 */
void xcmd_set_prompt(const char* prompt);
const char* xcmd_get_prompt(void);

/**
 * @description: 注册解释器接收函数的钩子函数
 * @param {func_p} 钩子函数，返回0则接收到的数据会返回给解释器，返回1则不会
 * @return {*} 无
 */
void xcmd_register_rcv_hook_func(uint8_t (*func_p)(char*));

/**
 * @description: 获取历史记录的个数
 * @param {*}
 * @return {uint16_t} 已经记录的历史个数
 */
uint16_t xcmd_history_len(void);

/**
 * @description: 插入一条历史记录
 * @param {char*} str
 * @return 无
 */
void xcmd_history_insert(char* str);

/**
 * @description: 获取下一条(更新的)历史记录, 未在浏览状态时返回 NULL
 * @param {*}
 * @return 历史命令
 */
char* xcmd_history_next(void);

/**
 * @description: 获取上一条(更旧的)历史记录, 未在浏览状态时从最新一条开始
 * @param {*}
 * @return 历史命令
 */
char* xcmd_history_prev(void);

/**
 * @description: 获取当前历史记录
 * @param {*}
 * @return 历史命令
 */
char* xcmd_history_current(void);

/**
 * @description: 将浏览指针移到最旧一条历史记录并返回, 用于从头遍历历史
 * @param {*}
 * @return 最旧的历史命令
 */
char* xcmd_history_slider_head(void);

/**
 * @description: 复位历史浏览指针(结束浏览状态)
 * @param {*}
 * @return 无
 */
void xcmd_history_slider_reset(void);

/**
 * @description: 结束输入
 * @param {*}
 * @return {*}
 */
char* xcmd_end_of_input(void);

#ifdef __cplusplus
}
#endif

#endif /*XCMD_H*/
