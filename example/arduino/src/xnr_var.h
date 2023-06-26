#ifndef XNR_ENV_H
#define XNR_ENV_H

#include "xnr_conf.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef XNR_VAR_NAME_LEN
#define XNR_VAR_NAME_LEN (32)
#endif
#ifndef XNR_VAR_VAR_LEN
#define XNR_VAR_VAR_LEN (64)
#endif

typedef struct xnr_var
{
    char name[XNR_VAR_NAME_LEN];
    char value[XNR_VAR_VAR_LEN];
} xnr_var_t;

typedef struct xnr_var_tab
{
    int capacity;
    xnr_var_t *var_buf;
} xnr_var_tab_t;

/**
 * @description: 初始化
 * @param tab：变量表，var_buf：变量存储空间，num：空间大小
 * @return -1：失败，0：成功
 */
int xnr_var_init(xnr_var_tab_t *tab, xnr_var_t *var_buf, int num);

/**
 * @description: 设置变量值
 * @param tab：变量表，name：变量名，value：变量值
 * @return -1：失败，其他：var_id
 */
int xnr_var_set(xnr_var_tab_t *tab, char *name, char *value);

/**
 * @description: 获取变量值
 * @param tab：变量表，name：变量名
 * @return "nil"：不存在此变量，其他：成功
 */
char *xnr_var_value(xnr_var_tab_t *tab, char *name);

/**
 * @description: 获取变量结构体
 * @param tab：变量表，id：变量id
 * @return NULL：不存在此变量，其他：成功
 */
xnr_var_t *xnr_var(xnr_var_tab_t *tab, int id);

/**
 * @description: 替换字符串中的变量，标识符可以是${name}，$name
 * @param tab：变量表，in：输入字符串，out：输出缓存，out_len：输出缓存长度
 * @return -1：失败，其他：成功
 */
int xnr_var_repalce(xnr_var_tab_t *tab, char *in, char *out, int out_len);

#ifdef __cplusplus
}
#endif

#endif