#ifndef xcmd_confg_H
#define xcmd_confg_H

#ifdef __cplusplus
extern   "C" {
#endif

#define XCMD_LINE_MAX_LENGTH            (256) /* 命令行支持的最大字符数 */

#define XCMD_PRINT_BUF_MAX_LENGTH       (4096) /* xcmd_print缓存 */

#define XCMD_HISTORY_BUF_SZIE           (4096)  /* 支持的历史记录缓存大小，实际历史记录个数取决于记录的命令长度，建议内存较小的单片机设置为0 */

#define XCMD_PARAM_MAX_NUM              (32)  /* 支持输入的参数个数 */

#if XCMD_PRINT_BUF_MAX_LENGTH <  (XCMD_LINE_MAX_LENGTH+32)
#undef XCMD_PRINT_BUF_MAX_LENGTH
#define XCMD_PRINT_BUF_MAX_LENGTH       (XCMD_LINE_MAX_LENGTH+32) /* xcmd_print缓存 */
#endif 

#define XCMD_DEFAULT_PROMPT             "$ " /*提示符*/
#define XCMD_DEFAULT_PROMPT_CLOLR       ANSI_BLUE /*提示符颜色*/


#define XCMD_VAR_NUM                    (512) /* 支持的变量的个数，可以为0 */
#define XCMD_VAR_NAME_BUF_SZIE          (64)  /* 变量名最大长度，可以为0 */
#define XCMD_VAR_VAR_SZIE               (128) /* 变量最大长度，可以为0 */

//#define ENABLE_XCMD_EXPORT /*使能XCMD_EXPORT_CMD和XCMD_EXPORT_KEY*/

#ifdef __cplusplus
        }
#endif

#endif /* xcmd_confg_H */
