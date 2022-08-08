/*
 * @Author: your name
 * @Date: 2021-09-17 23:03:06
 * @LastEditTime: 2021-10-11 21:40:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/inc/xcmd_confg.h
 */
#ifndef XCMD_CONFG_H
#define XCMD_CONFG_H

#ifdef __cplusplus
extern   "C" {
#endif


#define XCMD_LINE_MAX_LENGTH            (128)           /* 命令行支持的最大字符数 */
#define XCMD_HISTORY_MAX_NUM            (8)             /* 支持的历史记录个数, 这个参数对内存的影响很大，建议arduino设置为0，0为不支持 */
#define XCMD_PARAM_MAX_NUM              (8)             /* 支持输入的参数个数 */
// #define XCMD_DEFAULT_PROMPT           "->"            /* 提示符 */
// #define XCMD_DEFAULT_PROMPT_CLOLR     TX_GREEN        /* 提示符颜色 */
#define ENABLE_XCMD_EXPORT                            /* 需要修改lds，启用后支持XCMD_EXPORT_CMD和XCMD_EXPORT_KEY方法导出命令 */

#ifdef __cplusplus
        }
#endif

#endif /* XCMD_CONFG_H */
