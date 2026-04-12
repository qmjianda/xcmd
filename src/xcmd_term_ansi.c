#include "xcmd_term.h"
#include "xcmd_term_ansi.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static const char* ansi_clear_line(xcmd_term_t *term) {
    snprintf(term->buff, sizeof(term->buff), EL_ALL);
    return term->buff;
}

static const char* ansi_clear_to_eol(xcmd_term_t *term) {
    snprintf(term->buff, sizeof(term->buff), EL_EOL);
    return term->buff;
}

static const char* ansi_cursor_left(xcmd_term_t *term, uint16_t n) {
    if (n == 0) {
        term->buff[0] = '\0';
        return term->buff;
    }
    snprintf(term->buff, sizeof(term->buff), CUB(n));
    return term->buff;
}

static const char* ansi_cursor_right(xcmd_term_t *term, uint16_t n) {
    if (n == 0) {
        term->buff[0] = '\0';
        return term->buff;
    }
    snprintf(term->buff, sizeof(term->buff), CUF(n));
    return term->buff;
}

static const char* ansi_cursor_set(xcmd_term_t *term, uint16_t pos) {
    snprintf(term->buff, sizeof(term->buff), CHA(pos + 1));
    return term->buff;
}

static const char* ansi_insert_chars(xcmd_term_t *term, uint16_t n) {
    if (n == 0) {
        term->buff[0] = '\0';
        return term->buff;
    }
    snprintf(term->buff, sizeof(term->buff), ICH(n));
    return term->buff;
}

static const char* ansi_delete_chars(xcmd_term_t *term, uint16_t n) {
    if (n == 0) {
        term->buff[0] = '\0';
        return term->buff;
    }
    snprintf(term->buff, sizeof(term->buff), DCH(n));
    return term->buff;
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
