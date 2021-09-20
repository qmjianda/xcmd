/*
 * @Author: your name
 * @Date: 2021-09-20 19:59:05
 * @LastEditTime: 2021-09-20 20:07:50
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/inc/xcmd_define.h
 */

#ifndef XCMD_DEFINE_H
#define XCMD_DEFINE_H

#ifdef __cplusplus
extern   "C" {
#endif

typedef enum
{
    CTR_A = 1,
    CTR_B = 2,
    CTR_C = 3,
    CTR_D = 4,
    CTR_E = 5,
    CTR_F = 6,
    CTR_G = 7,
    CTR_H = 8,
    BACKSPACE = 8,
    CTR_I = 9,
    LF = 9,
    TAB = 9,
    CTR_J = 10,
    CTR_K = 11,
    CTR_L = 12,
    CTR_M = 13,
    CR = 13,
    CTR_N = 14,
    CTR_O = 15,
    CTR_P = 16,
    CTR_Q = 17,
    CTR_R = 18,
    CTR_S = 19,
    CTR_T = 20,
    CTR_U = 21,
    CTR_V = 22,
    CTR_W = 23,
    CTR_X = 24,
    CTR_Y = 25,
    CTR_Z = 26,
    ESC  = 27,
    FS  = 28,
    GS  = 29,
    RS  = 30,
    US  = 31,
    L_DELETE = 127,
    UP = 0x00415B1B,
    DW = 0x00425B1B,
    RIGHT = 0x00435B1B,
    LEFT = 0x00445B1B,
    END = 0x00465B1B,
    HOME = 0x00485B1B,
    INSERT = 0x7E325B1B,
    R_DELETE = 0x7E335B1B,
    PGUP = 0x7E355B1B,
    PGDW = 0x7E365B1B,
}XCMD_KEY_T;

/*光标操作符，其中0x1B是ESC，只适用于xcmd_print函数*/
#define CUU(n)         "\x1B[%dA",n		/* 光标向上	光标向上 <n> 行 */
#define CUD(n)         "\x1B[%dB",n		/* 光标向下	光标向下 <n> 行 */
#define CUF(n)         "\x1B[%dC",n		/* 光标向前	光标向前（右）<n> 行 */
#define CUB(n)         "\x1B[%dD",n		/* 光标向后	光标向后（左）<n> 行 */
#define CNL(n)         "\x1B[%dE",n		/* 光标下一行	光标从当前位置向下 <n> 行 */
#define CPL(n)         "\x1B[%dF",n		/* 光标当前行	光标从当前位置向上 <n> 行 */
#define CHA(n)         "\x1B[%dG",n		/* 绝对光标水平	光标在当前行中水平移动到第 <n> 个位置 */
#define VPA(n)         "\x1B[%dd",n		/* 绝对垂直行位置	光标在当前列中垂直移动到第 <n> 个位置 */
#define CUP(y,x)       "\x1B[%d;%dH",y,x	/* 光标位置	*光标移动到视区中的 <x>; <y> 坐标，其中 <x> 是 <y> 行的列 */
#define HVP(y,x)       "\x1B[%d;%df",y,x	/* 水平垂直位置	*光标移动到视区中的 <x>; <y> 坐标，其中 <x> 是 <y> 行的列 */

/*光标可见性*/
#define CU_START_BL   "\x1B[?12h"	/* ATT160	文本光标启用闪烁	开始光标闪烁 */
#define CU_STOP_BL    "\x1B[?12l"	/* ATT160	文本光标禁用闪烁	停止闪烁光标 */
#define CU_SHOW       "\x1B[?25h"	/* DECTCEM	文本光标启用模式显示	显示光标 */
#define CU_HIDE       "\x1B[?25l"	/* DECTCEM	文本光标启用模式隐藏	隐藏光标 */

/* 字符操作 */
#define ICH(n)	"\x1B[%d@",n	/* 插入字符	在当前光标位置插入 <n> 个空格，这会将所有现有文本移到右侧。 向右溢出屏幕的文本会被删除。*/
#define DCH(n)	"\x1B[%dP",n	/* 删除字符	删除当前光标位置的 <n> 个字符，这会从屏幕右边缘以空格字符移动。*/
#define ECH(n)	"\x1B[%dX",n	/* 擦除字符	擦除当前光标位置的 <n> 个字符，方法是使用空格字符覆盖它们。*/
#define IL(n)	"\x1B[%dL",n	/* 插入行	将 <n> 行插入光标位置的缓冲区。 光标所在的行及其下方的行将向下移动。*/
#define DL(n)	"\x1B[%dM",n	/* 删除行	从缓冲区中删除 <n> 行，从光标所在的行开始。*/

#ifdef __cplusplus
        }
#endif
#endif /*XCMD_DEFINE_H*/
