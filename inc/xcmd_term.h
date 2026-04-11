/**
 * @file xcmd_term.h
 * @brief Terminal abstraction layer for line editor
 * 
 * This module abstracts terminal capabilities to support different
 * terminal types (ANSI, dumb, etc.) with fallback strategies.
 * 
 * Memory footprint: ~4 bytes for function pointer table
 */

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

/**
 * @brief Terminal operations structure
 * 
 * All operations receive the terminal context for state tracking.
 */
typedef struct xcmd_term xcmd_term_t;

typedef struct {
    void (*clear_line)(xcmd_term_t *term);
    void (*clear_to_eol)(xcmd_term_t *term);
    void (*cursor_left)(xcmd_term_t *term, uint16_t n);
    void (*cursor_right)(xcmd_term_t *term, uint16_t n);
    void (*cursor_set)(xcmd_term_t *term, uint16_t pos);
    void (*insert_chars)(xcmd_term_t *term, uint16_t n);
    void (*delete_chars)(xcmd_term_t *term, uint16_t n);
} xcmd_term_ops_t;

/**
 * @brief Terminal sessance structure
 * 
 * Size: ~40 bytes (depends on architecture)
 */
struct xcmd_term {
    const char *prompt;
    uint16_t prompt_len;
    const xcmd_term_ops_t *ops;
    int (*put_c)(uint8_t);
    void *user_data;
};

/**
 * @brief Initialize terminal
 * 
 * @param term Terminal sessance to initialize
 * @param put_c Character output function
 * @param ops Terminal operations table (e.g., xcmd_term_get_ansi_ops())
 * @return 0 on success, -1 on error
 */
int xcmd_term_init(xcmd_term_t *term, int (*put_c)(uint8_t), const xcmd_term_ops_t *ops);


/**
 * @brief Set terminal prompt
 * 
 * @param term Terminal sessance
 * @param prompt Prompt string (should be null-terminated)
 */
void xcmd_term_set_prompt(xcmd_term_t *term, const char *prompt);

/**
 * @brief Get terminal prompt
 * 
 * @param term Terminal sessance
 * @return Current prompt string
 */
const char* xcmd_term_get_prompt(xcmd_term_t *term);

/**
 * @brief Write a single character to terminal
 * 
 * @param term Terminal sessance
 * @param c Character to write
 */
void xcmd_term_putc(xcmd_term_t *term, uint8_t c);

/**
 * @brief Write string to terminal
 * 
 * @param term Terminal sessance
 * @param str String to write
 */
void xcmd_term_write(xcmd_term_t *term, const char *str);

/**
 * @brief Write formatted output to terminal
 * 
 * @param term Terminal sessance
 * @param fmt Format string
 * @param ... Format arguments
 */
void xcmd_term_printf(xcmd_term_t *term, const char *fmt, ...);

/**
 * @brief Refresh the current line (clear and redraw)
 * 
 * @param term Terminal sessance
 * @param prompt Prompt string to display
 * @param buffer Current line buffer
 * @param cursor Current cursor position
 * @param length Length of buffer content
 */
void xcmd_term_refresh_line(xcmd_term_t *term,
                           const char *buffer, uint16_t cursor, uint16_t length);

/*============================================================================
 * Built-in Terminal Implementations
 *============================================================================*/

/**
 * @brief Get ANSI terminal operations
 * 
 * Full-featured terminal with ICH/DCH support.
 * Memory: ~200 bytes code
 */
const xcmd_term_ops_t* xcmd_term_get_ansi_ops(void);

/**
 * @brief Get dumb terminal operations
 * 
 * Minimal terminal using only \r, \b for editing.
 * Memory: ~100 bytes code
 */
const xcmd_term_ops_t* xcmd_term_get_dumb_ops(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_TERM_H */