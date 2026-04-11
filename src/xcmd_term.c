#include "xcmd_term.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

int xcmd_term_init(xcmd_term_t *term, int (*put_c)(uint8_t), const xcmd_term_ops_t *ops) {
    if (!term || !put_c) return -1;
    
    term->prompt = NULL;
    term->prompt_len = 0;
    term->ops = ops;
    term->put_c = put_c;
    term->user_data = NULL;
    
    return 0;
}

void xcmd_term_set_prompt(xcmd_term_t *term, const char *prompt) {
    if (term) {
        term->prompt = prompt;
        term->prompt_len = prompt ? strlen(prompt) : 0;
    }
}


const char* xcmd_term_get_prompt(xcmd_term_t *term) {
    return term ? term->prompt : NULL;
}

void xcmd_term_putc(xcmd_term_t *term, uint8_t c) {
    if (term && term->put_c) {
        term->put_c(c);
    }
}

void xcmd_term_write(xcmd_term_t *term, const char *str) {
    if (!term || !str) return;
    while (*str) {
        term->put_c(*str++);
    }
}

void xcmd_term_printf(xcmd_term_t *term, const char *fmt, ...) {
    if (!term || !fmt) return;
    
    char buf[XCMD_TERM_PRINTF_BUF_LEN];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    
    xcmd_term_write(term, buf);
}

void xcmd_term_refresh_line(xcmd_term_t *term,
                           const char *buffer, uint16_t cursor, uint16_t length) {
    if (!term || !term->ops) return;
    
    if (term->ops->clear_line) {
        term->ops->clear_line(term);
    } else {
        xcmd_term_putc(term, '\r');
        for (uint16_t i = 0; i < length + 32; i++) {
            xcmd_term_putc(term, ' ');
        }
        xcmd_term_putc(term, '\r');
    }
    
    if (term->prompt) {
        xcmd_term_write(term, term->prompt);
    }
    if (buffer) {
        xcmd_term_write(term, buffer);
    }
    
    if (cursor < length) {
        uint16_t n = length - cursor;
        for (uint16_t i = 0; i < n; i++) {
            xcmd_term_putc(term, '\b');
        }
    }
}