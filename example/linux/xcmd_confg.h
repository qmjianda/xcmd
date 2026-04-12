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

#include "xcmd_define.h"

#define XCMD_LINE_MAX_LENGTH            (128)           /* 命令行支持的最大字符数 */
#define XCMD_HISTORY_MAX_NUM            (16)            /* 支持的历史记录个数, 这个参数对内存的影响很大，建议arduino设置为0，0为不支持 */
#define XCMD_PARAM_MAX_NUM              (64)             /* 支持输入的参数个数 */
// #define XCMD_DEFAULT_PROMPT           "->"            /* 提示符 */
// #define XCMD_DEFAULT_PROMPT_COLOR     TX_GREEN        /* 提示符颜色 */

#ifdef __cplusplus
        }
#endif

#endif /* XCMD_CONFG_H */
