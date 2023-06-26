#ifndef XNR_CONF_H
#define XNR_CONF_H

#include "xcmd_confg.h"

//行缓存长度
#ifndef  XNR_LINE_MAX_LENGTH
#define XNR_LINE_MAX_LENGTH (XCMD_LINE_MAX_LENGTH)
#endif

//历史记录缓存长度
#ifndef XNR_HISTORY_BUF_SZIE
#define XNR_HISTORY_BUF_SZIE (XCMD_HISTORY_BUF_SZIE)
#endif

//变量名长度
#ifndef XNR_VAR_NAME_LEN
#define XNR_VAR_NAME_LEN    (XCMD_VAR_NAME_BUF_SZIE)
#endif

//变量值长度
#ifndef XNR_VAR_VAR_LEN
#define XNR_VAR_VAR_LEN    (XCMD_VAR_VAR_SZIE)
#endif

//IO print缓存大小
#ifndef XNR_PRINT_BUF_MAX_LENGTH
#define XNR_PRINT_BUF_MAX_LENGTH (XCMD_PRINT_BUF_MAX_LENGTH)
#endif

#endif