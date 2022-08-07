/*
 * @Author: your name
 * @Date: 2021-09-17 23:03:06
 * @LastEditTime: 2021-10-11 21:40:01
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/inc/xcmd_default_confg.h
 */
#ifndef XCMD_DEFAULT_CONFG_H
#define XCMD_DEFAULT_CONFG_H

#ifdef __cplusplus
extern   "C" {
#endif

#include "xcmd_confg.h"

#define VERSION "1.1.0"

#ifndef XCMD_LINE_MAX_LENGTH
#define XCMD_LINE_MAX_LENGTH    (64) /* 命令行支持的最大字符数 */
#endif

#ifndef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (128) /* xcmd_print缓存 */
#endif  

#ifndef  XCMD_HISTORY_MAX_NUM
#define XCMD_HISTORY_MAX_NUM    (8)  /* 支持的历史记录个数, 这个参数对内存的影响很大，建议arduino设置为0，0为不支持 */
#endif

#ifndef XCMD_PARAM_MAX_NUM
#define XCMD_PARAM_MAX_NUM      (8)  /* 支持输入的参数个数 */
#endif

#if XCMD_PRINT_BUF_MAX_LENGTH <  (XCMD_LINE_MAX_LENGTH+32)
#undef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (XCMD_LINE_MAX_LENGTH+32) /* xcmd_print缓存 */
#endif 

#ifndef XCMD_DEFAULT_PROMPT
#define XCMD_DEFAULT_PROMPT     "->" /*提示符*/
#endif

#ifndef XCMD_DEFAULT_PROMPT_CLOLR
#define XCMD_DEFAULT_PROMPT_CLOLR   TX_GREEN /*提示符颜色*/
#endif

#ifndef ENABLE_XCMD_EXPORT
//#define ENABLE_XCMD_EXPORT /*使能XCMD_EXPORT_CMD和XCMD_EXPORT_KEY*/
#endif

#ifdef __cplusplus
        }
#endif

#endif /* XCMD_DEFAULT_CONFG_H */
