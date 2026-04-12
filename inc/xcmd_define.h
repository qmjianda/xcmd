/*
 * @Author: your name
 * @Date: 2021-09-20 19:59:05
 * @LastEditTime: 2022-02-17 22:41:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/inc/xcmd_define.h
 */

#ifndef XCMD_DEFINE_H
#define XCMD_DEFINE_H

#ifdef __cplusplus
extern   "C" {
#endif


/* 打印字体颜色设置 */
#define TX_DEF          "\x1b[0m"
#define TX_BLACK        "\x1b[30m"
#define TX_RED          "\x1b[31m"
#define TX_GREEN        "\x1b[32m"
#define TX_YELLOW       "\x1b[33m"
#define TX_BLUE         "\x1b[34m"
#define TX_WHITE        "\x1b[37m"

/* 打印背景颜色设置 */
#define BK_DEF          "\x1b[0m"
#define BK_BLACK        "\x1b[40m"
#define BK_RED          "\x1b[41m"
#define BK_GREEN        "\x1b[42m"
#define BK_YELLOW       "\x1b[43m"
#define BK_BLUE         "\x1b[44m"
#define BK_WHITE        "\x1b[47m"

#ifdef __cplusplus
        }
#endif
#endif /*XCMD_DEFINE_H*/
