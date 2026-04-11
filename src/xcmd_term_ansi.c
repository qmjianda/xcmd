#include "xcmd_term.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static void ansi_clear_line(xcmd_term_t *term) {
    const char *s = "\x1B[2K\r";
    while (*s) term->put_c(*s++);
}

static void ansi_clear_to_eol(xcmd_term_t *term) {
    const char *s = "\x1B[K";
    while (*s) term->put_c(*s++);
}

static void ansi_cursor_left(xcmd_term_t *term, uint16_t n) {
    if (n == 0) return;
    if (n == 1) {
        term->put_c('\b');
        return;
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1B[%dD", n);
    const char *s = buf;
    while (*s) term->put_c(*s++);
}

static void ansi_cursor_right(xcmd_term_t *term, uint16_t n) {
    if (n == 0) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1B[%dC", n);
    const char *s = buf;
    while (*s) term->put_c(*s++);
}

static void ansi_cursor_set(xcmd_term_t *term, uint16_t pos) {
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1B[%dG", term->prompt_len + pos + 1);
    const char *s = buf;
    while (*s) term->put_c(*s++);
}

static void ansi_insert_chars(xcmd_term_t *term, uint16_t n) {
    if (n == 0) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1B[%d@", n);
    const char *s = buf;
    while (*s) term->put_c(*s++);
}

static void ansi_delete_chars(xcmd_term_t *term, uint16_t n) {
    if (n == 0) return;
    char buf[16];
    snprintf(buf, sizeof(buf), "\x1B[%dP", n);
    const char *s = buf;
    while (*s) term->put_c(*s++);
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