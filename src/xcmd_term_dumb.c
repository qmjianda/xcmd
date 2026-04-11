#include "xcmd_term.h"


static const xcmd_term_ops_t dumb_ops = {
    .clear_line = NULL,
    .clear_to_eol = NULL,
    .cursor_left = NULL,
    .cursor_right = NULL,
    .cursor_set = NULL,
    .insert_chars = NULL,
    .delete_chars = NULL,
};

const xcmd_term_ops_t* xcmd_term_get_dumb_ops(void) {
    return &dumb_ops;
}
