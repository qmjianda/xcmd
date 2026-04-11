/**
 * @file xcmd_line_editor.h
 * @brief Independent line editor module for embedded CLI
 * 
 * Design goals:
 * - Zero dynamic memory allocation
 * - Configurable buffer sizes
 * - Terminal abstraction for compatibility
 * - Minimal code footprint
 * 
 * Memory usage (XCMD_LINE_MAX_LENGTH=64, XCMD_HISTORY_MAX_NUM=8):
 * - Editor sessance: ~160 bytes
 * - History pool: ~576 bytes (optional, can disable)
 */

#ifndef XCMD_LINE_EDITOR_H
#define XCMD_LINE_EDITOR_H

#include "xcmd_term.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Configuration
 *============================================================================*/

#ifndef XCMD_LE_LINE_MAX_LEN
#define XCMD_LE_LINE_MAX_LEN    64    /**< Max characters per line */
#endif

#ifndef XCMD_LE_HISTORY_MAX_NUM
#define XCMD_LE_HISTORY_MAX_NUM  8     /**< History entries (0=disabled) */
#endif

#ifndef XCMD_LE_ESCAPE_BUF_LEN
#define XCMD_LE_ESCAPE_BUF_LEN  8     /**< Escape sequence buffer */
#endif

/*============================================================================
 * Types
 *============================================================================*/

struct xcmd_line_editor;

typedef int (*xcmd_le_complete_cb)(struct xcmd_line_editor *le, 
                                    const char *line, 
                                    uint16_t cursor);

typedef int (*xcmd_le_key_cb)(struct xcmd_line_editor *le, 
                               const char *key_seq);

typedef void (*xcmd_le_submit_cb)(struct xcmd_line_editor *le,
                                   const char *line);

typedef struct {
    char *line;
    uint16_t len;
    uint16_t cursor;
} xcmd_le_state_t;

#if XCMD_LE_HISTORY_MAX_NUM > 0
typedef struct {
    char line[XCMD_LE_LINE_MAX_LEN + 1];
} xcmd_le_history_entry_t;

typedef struct {
    xcmd_le_history_entry_t pool[XCMD_LE_HISTORY_MAX_NUM];
    uint16_t count;
    xcmd_le_history_entry_t *current;
} xcmd_le_history_t;
#endif

typedef struct xcmd_line_editor {
    char buffer[XCMD_LE_LINE_MAX_LEN + 1];
    uint16_t length;
    uint16_t cursor;
    
    char escape_buf[XCMD_LE_ESCAPE_BUF_LEN];
    uint8_t escape_state;
    uint8_t escape_len;
    
    xcmd_term_t *term;
    
#if XCMD_LE_HISTORY_MAX_NUM > 0
    xcmd_le_history_t history;
#endif
    
    void *user_data;
    
    xcmd_le_submit_cb on_submit;
} xcmd_line_editor_t;

/*============================================================================
 * Lifecycle
 *============================================================================*/

int xcmd_le_init(xcmd_line_editor_t *le, 
                 xcmd_term_t *term);

void xcmd_le_set_on_submit(xcmd_line_editor_t *le, xcmd_le_submit_cb cb);

/*============================================================================
 * Input Processing
 *============================================================================*/

typedef enum {
    XCMD_LE_MORE,           /**< Need more input, line not complete */
    XCMD_LE_COMPLETE,       /**< Line submitted (Enter pressed) */
    XCMD_LE_CANCEL,         /**< Line cancelled (Ctrl+C) */
    XCMD_LE_EOF             /**< End of input */
} xcmd_le_result_t;

xcmd_le_result_t xcmd_le_feed(xcmd_line_editor_t *le, uint8_t byte);

const char* xcmd_le_get_line(xcmd_line_editor_t *le);
uint16_t xcmd_le_get_cursor(xcmd_line_editor_t *le);
uint16_t xcmd_le_get_length(xcmd_line_editor_t *le);

/*============================================================================
 * Editing Operations
 *============================================================================*/

void xcmd_le_insert_char(xcmd_line_editor_t *le, char c);
void xcmd_le_delete_char(xcmd_line_editor_t *le);
void xcmd_le_delete_char_forward(xcmd_line_editor_t *le);

void xcmd_le_cursor_left(xcmd_line_editor_t *le);
void xcmd_le_cursor_right(xcmd_line_editor_t *le);
void xcmd_le_cursor_home(xcmd_line_editor_t *le);
void xcmd_le_cursor_end(xcmd_line_editor_t *le);

void xcmd_le_word_left(xcmd_line_editor_t *le);
void xcmd_le_word_right(xcmd_line_editor_t *le);

void xcmd_le_delete_to_start(xcmd_line_editor_t *le);
void xcmd_le_delete_to_end(xcmd_line_editor_t *le);

void xcmd_le_clear_line(xcmd_line_editor_t *le);
void xcmd_le_set_line(xcmd_line_editor_t *le, const char *str);

/*============================================================================
 * History (if enabled)
 *============================================================================*/

#if XCMD_LE_HISTORY_MAX_NUM > 0
void xcmd_le_history_append(xcmd_line_editor_t *le, const char *line);
const char* xcmd_le_history_prev(xcmd_line_editor_t *le);
const char* xcmd_le_history_next(xcmd_line_editor_t *le);
const char* xcmd_le_history_current(xcmd_line_editor_t *le);
void xcmd_le_history_reset(xcmd_line_editor_t *le);
uint16_t xcmd_le_history_count(xcmd_line_editor_t *le);
#endif

/*============================================================================
 * Key Bindings
 *============================================================================*/

int xcmd_le_key_register(const char *key_seq, xcmd_le_key_cb callback);
int xcmd_le_key_unregister(const char *key_seq);

/*============================================================================
 * Built-in Key Handlers
 *============================================================================*/

int xcmd_le_key_enter(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_backspace(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_delete(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_left(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_right(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_home(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_end(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_ctrl_a(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_ctrl_e(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_ctrl_u(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_ctrl_k(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_ctrl_l(xcmd_line_editor_t *le, const char *key);

#if XCMD_LE_HISTORY_MAX_NUM > 0
int xcmd_le_key_up(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_down(xcmd_line_editor_t *le, const char *key);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XCMD_LINE_EDITOR_H */