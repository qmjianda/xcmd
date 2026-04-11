#include "xcmd_term.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int xcmd_term_init(xcmd_term_t *term, const xcmd_term_ops_t *ops) {
    if (!term || !ops) return -1;
    term->ops = ops;
    return 0;
}