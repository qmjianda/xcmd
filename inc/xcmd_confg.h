/*
 * @Author: your name
 * @Date: 2021-09-17 23:03:06
 * @LastEditTime: 2021-09-22 21:59:50
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/inc/xcmd_confg.h
 */
#ifndef XCMD_CONFG_H
#define XCMD_CONFG_H

#ifdef __cplusplus
extern   "C" {
#endif

#define VERSION "1.0.0"

#ifndef XCMD_LINE_MAX_LENGTH
#define XCMD_LINE_MAX_LENGTH    (32) /* 命令行支持的最大字符数 */
#endif

#ifndef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (128) /* xcmd_print缓存 */
#endif  

#ifndef  XCMD_HISTORY_MAX_NUM
#define XCMD_HISTORY_MAX_NUM    (0)  /* 支持的历史记录个数，0为不支持 */
#endif

#ifndef XCMD_PARAM_MAX_NUM
#define XCMD_PARAM_MAX_NUM      (8)  /* 支持输入的参数个数 */
#endif

#if XCMD_PRINT_BUF_MAX_LENGTH <  (XCMD_LINE_MAX_LENGTH+32)
#undef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (XCMD_LINE_MAX_LENGTH+32) /* xcmd_print缓存 */
#endif 

#ifdef __cplusplus
        }
#endif

#endif /* XCMD_CONFG_H */
