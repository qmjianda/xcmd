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

#ifdef XCMD_CONFG_H
#include "xcmd_confg.h"
#endif

#define VERSION "1.2.0"

#ifndef XCMD_LINE_MAX_LENGTH
#define XCMD_LINE_MAX_LENGTH    (64)
#endif

#ifndef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (128)
#endif  

#ifndef  XCMD_HISTORY_MAX_NUM
#define XCMD_HISTORY_MAX_NUM    (8)
#endif

#ifndef XCMD_PARAM_MAX_NUM
#define XCMD_PARAM_MAX_NUM      (8)
#endif

#if XCMD_PRINT_BUF_MAX_LENGTH <  (XCMD_LINE_MAX_LENGTH+32)
#undef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (XCMD_LINE_MAX_LENGTH+32)
#endif 

#ifndef XCMD_DEFAULT_PROMPT
#define XCMD_DEFAULT_PROMPT     "-> "
#endif

#ifndef XCMD_DEFAULT_PROMPT_CLOLR
#define XCMD_DEFAULT_PROMPT_CLOLR   TX_GREEN
#endif

#ifndef ENABLE_XCMD_EXPORT
#endif

#ifndef XCMD_IO_MAX_DEVICES
#define XCMD_IO_MAX_DEVICES     (8)
#endif

#ifndef XCMD_NO_DEPRECATED
#endif

#ifdef __cplusplus
        }
#endif

#endif /* XCMD_DEFAULT_CONFG_H */
