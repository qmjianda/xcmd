#include "xcmd_term.h"
#include "xcmd_io.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int xcmd_term_init(xcmd_term_t *term, const xcmd_term_ops_t *ops) {
    if (!term || !ops) return -1;
    term->ops = ops;
    term->io = NULL;
    return 0;
}

void xcmd_term_set_io(xcmd_term_t *term, xcmd_io_t *io) {
    if (term) {
        term->io = io;
    }
}

void xcmd_term_put_str(xcmd_term_t *term, const char *str) {
    if (term && term->io && str) {
        xcmd_io_puts(term->io, str);
    }
}