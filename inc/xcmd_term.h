#ifndef XCMD_TERM_H
#define XCMD_TERM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef XCMD_TERM_PRINTF_BUF_LEN
#define XCMD_TERM_PRINTF_BUF_LEN  128
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xcmd_term xcmd_term_t;
typedef struct xcmd_io xcmd_io_t;

typedef struct {
    const char* (*clear_line)(xcmd_term_t *term);
    const char* (*clear_to_eol)(xcmd_term_t *term);
    const char* (*cursor_left)(xcmd_term_t *term, uint16_t n);
    const char* (*cursor_right)(xcmd_term_t *term, uint16_t n);
    const char* (*cursor_set)(xcmd_term_t *term, uint16_t pos);
    const char* (*insert_chars)(xcmd_term_t *term, uint16_t n);
    const char* (*delete_chars)(xcmd_term_t *term, uint16_t n);
} xcmd_term_ops_t;

struct xcmd_term {
    const xcmd_term_ops_t *ops;
    char buff[32]; /* Buffer for printf formatting */
};

int xcmd_term_init(xcmd_term_t *term, const xcmd_term_ops_t *ops);
const xcmd_term_ops_t* xcmd_term_get_ansi_ops(void);
const xcmd_term_ops_t* xcmd_term_get_dumb_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_TERM_H */