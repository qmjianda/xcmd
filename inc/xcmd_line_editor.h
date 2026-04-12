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
#include "xcmd_io.h"
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

#ifndef XCMD_LE_COMPLETION_MAX_NUM
#define XCMD_LE_COMPLETION_MAX_NUM  16  /**< Max completion candidates */
#endif

/*============================================================================
 * Types
 *============================================================================*/

struct xcmd_line_editor;

typedef int (*xcmd_le_complete_cb)(struct xcmd_line_editor *le, 
                                    const char *line, 
                                    uint16_t cursor,
                                    char *matches[],
                                    uint16_t max_matches);

typedef struct {
    char *candidates[XCMD_LE_COMPLETION_MAX_NUM];
    uint16_t count;
    uint16_t current;
} xcmd_le_completion_t;

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
    
    xcmd_io_t *io;
    xcmd_term_t *term;
    const char *prompt;
    uint16_t prompt_len;
    
#if XCMD_LE_HISTORY_MAX_NUM > 0
    xcmd_le_history_t history;
#endif
    
    xcmd_le_complete_cb complete_cb;
    xcmd_le_completion_t completion;
    
    void *user_data;
    
    char submitted[XCMD_LE_LINE_MAX_LEN + 1];
} xcmd_line_editor_t;

/*============================================================================
 * Lifecycle
 *============================================================================*/

int xcmd_le_init(xcmd_line_editor_t *le, xcmd_term_t *term);

void xcmd_le_set_complete_cb(xcmd_line_editor_t *le, xcmd_le_complete_cb cb);

/*============================================================================
 * Input Processing (non-blocking, readline style)
 *============================================================================*/

/**
 * @brief Read a line from IO (non-blocking, readline style)
 * @param le Line editor
 * @param in IO for reading input
 * @param out IO for writing output
 * @param prompt Prompt string
 * @return Submitted line, or NULL if no complete line available
 */
const char* xcmd_le_readline(xcmd_line_editor_t *le, xcmd_io_t *in, xcmd_io_t *out, const char *prompt);

void xcmd_le_refresh(xcmd_line_editor_t *le);

const char* xcmd_le_get_line(xcmd_line_editor_t *le);
const char* xcmd_le_get_submitted_line(xcmd_line_editor_t *le);
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
int xcmd_le_key_ctrl_c(xcmd_line_editor_t *le, const char *key);

#if XCMD_LE_HISTORY_MAX_NUM > 0
int xcmd_le_key_up(xcmd_line_editor_t *le, const char *key);
int xcmd_le_key_down(xcmd_line_editor_t *le, const char *key);
#endif

/*============================================================================
 * Key Definitions
 *============================================================================*/

#define XCMD_KEY_CTR_A       "\x01"
#define XCMD_KEY_CTR_B       "\x02"
#define XCMD_KEY_CTR_C       "\x03"
#define XCMD_KEY_CTR_D       "\x04"
#define XCMD_KEY_CTR_E       "\x05"
#define XCMD_KEY_CTR_F       "\x06"
#define XCMD_KEY_CTR_G       "\x07"
#define XCMD_KEY_CTR_H       "\x08"
#define XCMD_KEY_CTR_I       "\x09"
#define XCMD_KEY_TAB         "\x09"
#define XCMD_KEY_CTR_J       "\x0A"
#define XCMD_KEY_CTR_K       "\x0B"
#define XCMD_KEY_CTR_L       "\x0C"
#define XCMD_KEY_CTR_M       "\x0D"
#define XCMD_KEY_CTR_N       "\x0E"
#define XCMD_KEY_CTR_O       "\x0F"
#define XCMD_KEY_CTR_P       "\x10"
#define XCMD_KEY_CTR_Q       "\x11"
#define XCMD_KEY_CTR_R       "\x12"
#define XCMD_KEY_CTR_S       "\x13"
#define XCMD_KEY_CTR_T       "\x14"
#define XCMD_KEY_CTR_U       "\x15"
#define XCMD_KEY_CTR_V       "\x16"
#define XCMD_KEY_CTR_W       "\x17"
#define XCMD_KEY_CTR_X       "\x18"
#define XCMD_KEY_CTR_Y       "\x19"
#define XCMD_KEY_CTR_Z       "\x1A"
#define XCMD_KEY_PAUSE       "\x1A"
#define XCMD_KEY_ESC         "\x1B"
#define XCMD_KEY_BACKSPACE   "\x7F"
#define XCMD_KEY_UP          "\x1B[A"
#define XCMD_KEY_DW          "\x1B[B"
#define XCMD_KEY_RIGHT       "\x1B[C"
#define XCMD_KEY_LEFT        "\x1B[D"
#define XCMD_KEY_HOME        "\x1B[H"
#define XCMD_KEY_HOME2       "\x1B[1~"
#define XCMD_KEY_END         "\x1B[F"
#define XCMD_KEY_END2        "\x1B[4~"
#define XCMD_KEY_CTR_UP      "\x1B[1;5A"
#define XCMD_KEY_CTR_DW      "\x1B[1;5B"
#define XCMD_KEY_CTR_RIGHT   "\x1B[1;5C"
#define XCMD_KEY_CTR_LEFT    "\x1B[1;5D"
#define XCMD_KEY_INSERT      "\x1B[2~"
#define XCMD_KEY_DELETE      "\x1B[3~"
#define XCMD_KEY_PAGE_UP     "\x1B[5~"
#define XCMD_KEY_PAGE_DOWN   "\x1B[6~"
#define XCMD_KEY_F1          "\x1BOP"
#define XCMD_KEY_F2          "\x1BOQ"
#define XCMD_KEY_F3          "\x1BOR"
#define XCMD_KEY_F4          "\x1BOS"
#define XCMD_KEY_F5          "\x1B[15~"
#define XCMD_KEY_F6          "\x1B[17~"
#define XCMD_KEY_F7          "\x1B[18~"
#define XCMD_KEY_F8          "\x1B[19~"
#define XCMD_KEY_F9          "\x1B[20~"
#define XCMD_KEY_F10         "\x1B[21~"
#define XCMD_KEY_F11         "\x1B[23~"
#define XCMD_KEY_F12         "\x1B[24~"

#ifdef __cplusplus
}
#endif

#endif /* XCMD_LINE_EDITOR_H */