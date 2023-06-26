#ifndef xcmd_confg_H
#define xcmd_confg_H

#ifdef __cplusplus
extern   "C" {
#endif

#ifndef XCMD_LINE_MAX_LENGTH
#define XCMD_LINE_MAX_LENGTH    (128) /* 命令行支持的最大字符数 */
#endif

#ifndef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (512) /* xcmd_print缓存 */
#endif  

#ifndef  XCMD_HISTORY_BUF_SZIE
#define XCMD_HISTORY_BUF_SZIE    (512)  /* 支持的历史记录个数, 这个参数对内存的影响很大，建议arduino设置为0，0为不支持 */
#endif

#ifndef XCMD_PARAM_MAX_NUM
#define XCMD_PARAM_MAX_NUM      (16)  /* 支持输入的参数个数 */
#endif

#if XCMD_PRINT_BUF_MAX_LENGTH <  (XCMD_LINE_MAX_LENGTH+32)
#undef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH   (XCMD_LINE_MAX_LENGTH+32) /* xcmd_print缓存 */
#endif 

#ifndef XCMD_DEFAULT_PROMPT
#define XCMD_DEFAULT_PROMPT     "$ " /*提示符*/
#endif

#ifndef XCMD_DEFAULT_PROMPT_CLOLR
#define XCMD_DEFAULT_PROMPT_CLOLR   ANSI_BLUE /*提示符颜色*/
#endif

#ifndef XCMD_VAR_NUM
#define XCMD_VAR_NUM      (32)
#endif

#ifndef XCMD_VAR_NAME_BUF_SZIE
#define XCMD_VAR_NAME_BUF_SZIE      (32)
#endif

#ifndef XCMD_VAR_VAR_SZIE
#define XCMD_VAR_VAR_SZIE      (64)
#endif

#ifndef ENABLE_XCMD_EXPORT
//#define ENABLE_XCMD_EXPORT /*使能XCMD_EXPORT_CMD和XCMD_EXPORT_KEY*/
#endif

#ifdef __cplusplus
        }
#endif

#endif /* xcmd_confg_H */
