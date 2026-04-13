#include "xcmd_term.h"
#include "xcmd_io.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>


/*光标操作符，其中0x1B是ESC，只适用于xcmd_print函数*/
#define CUU(n)      "\x1B[%dA",n		/* 光标向上	光标向上 <n> 行 */
#define CUD(n)      "\x1B[%dB",n		/* 光标向下	光标向下 <n> 行 */
#define CUF(n)      "\x1B[%dC",n		/* 光标向前	光标向前（右）<n> 行 */
#define CUB(n)      "\x1B[%dD",n		/* 光标向后	光标向后（左）<n> 行 */
#define CNL(n)      "\x1B[%dE",n		/* 光标下一行	光标从当前位置向下 <n> 行 */
#define CPL(n)      "\x1B[%dF",n		/* 光标当前行	光标从当前位置向上 <n> 行 */
#define CHA(n)      "\x1B[%dG",n		/* 绝对光标水平	光标在当前行中水平移动到第 <n> 个位置 */
#define VPA(n)      "\x1B[%dd",n		/* 绝对垂直行位置	光标在当前列中垂直移动到第 <n> 个位置 */
#define CUP(y,x)    "\x1B[%d;%dH",y,x	/* 光标位置	*光标移动到视区中的 <x>; <y> 坐标，其中 <x> 是 <y> 行的列 */
#define HVP(y,x)    "\x1B[%d;%df",y,x	/* 水平垂直位置	*光标移动到视区中的 <x>; <y> 坐标，其中 <x> 是 <y> 行的列 */

/*光标可见性*/
#define CU_START_BL "\x1B[?12h"	/* ATT160	文本光标启用闪烁	开始光标闪烁 */
#define CU_STOP_BL  "\x1B[?12l"	/* ATT160	文本光标禁用闪烁	停止闪烁光标 */
#define CU_SHOW     "\x1B[?25h"	/* DECTCEM	文本光标启用模式显示	显示光标 */
#define CU_HIDE     "\x1B[?25l"	/* DECTCEM	文本光标启用模式隐藏	隐藏光标 */

/* 字符操作 */
#define ICH(n)      "\x1B[%d@",n	/* 插入字符	在当前光标位置插入 <n> 个空格，这会将所有现有文本移到右侧。 向右溢出屏幕的文本会被删除。*/
#define DCH(n)      "\x1B[%dP",n	/* 删除字符	删除当前光标位置的 <n> 个字符，这会从屏幕右边缘以空格字符移动。*/
#define ECH(n)      "\x1B[%dX",n	/* 擦除字符	擦除当前光标位置的 <n> 个字符，方法是使用空格字符覆盖它们。*/
#define IL(n)       "\x1B[%dL",n	/* 插入行	将 <n> 行插入光标位置的缓冲区。 光标所在的行及其下方的行将向下移动。*/
#define DL(n)       "\x1B[%dM",n	/* 删除行	从缓冲区中删除 <n> 行，从光标所在的行开始。*/

/* 清除行 */
#define EL_EOL      "\x1B[K"     /* 清除从光标到行尾 */
#define EL_ALL      "\x1B[2K"    /* 清除整行 */


static void ansi_clear_line(xcmd_term_t *term) {
    if (term->io) {
        xcmd_io_puts(term->io, EL_ALL);
        xcmd_io_putc(term->io, '\r');
    }
}

static void ansi_clear_to_eol(xcmd_term_t *term) {
    if (term->io) {
        xcmd_io_puts(term->io, EL_EOL);
    }
}

static void ansi_cursor_left(xcmd_term_t *term, uint16_t n) {
    if (term->io && n > 0) {
        snprintf(term->buff, sizeof(term->buff), CUB(n));
        xcmd_io_puts(term->io, term->buff);
    }
}

static void ansi_cursor_right(xcmd_term_t *term, uint16_t n) {
    if (term->io && n > 0) {
        snprintf(term->buff, sizeof(term->buff), CUF(n));
        xcmd_io_puts(term->io, term->buff);
    }
}

static void ansi_cursor_set(xcmd_term_t *term, uint16_t pos) {
    if (term->io) {
        snprintf(term->buff, sizeof(term->buff), CHA(pos + 1));
        xcmd_io_puts(term->io, term->buff);
    }
}

static void ansi_insert_chars(xcmd_term_t *term, uint16_t n) {
    if (term->io && n > 0) {
        snprintf(term->buff, sizeof(term->buff), ICH(n));
        xcmd_io_puts(term->io, term->buff);
    }
}

static void ansi_delete_chars(xcmd_term_t *term, uint16_t n) {
    if (term->io && n > 0) {
        snprintf(term->buff, sizeof(term->buff), DCH(n));
        xcmd_io_puts(term->io, term->buff);
    }
}

static const xcmd_term_ops_t ansi_ops = {
    .clear_line = ansi_clear_line,
    .clear_to_eol = ansi_clear_to_eol,
    .cursor_left = ansi_cursor_left,
    .cursor_right = ansi_cursor_right,
    .cursor_set = ansi_cursor_set,
    .insert_chars = ansi_insert_chars,
    .delete_chars = ansi_delete_chars,
};

const xcmd_term_ops_t* xcmd_term_get_ansi_ops(void) {
    return &ansi_ops;
}
