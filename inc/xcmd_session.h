/**
 * @file xcmd_session.h
 * @brief Multi-sessance support for xcmd
 * 
 * This module enables multiple independent xcmd sessances,
 * each with its own I/O, terminal, and editor state.
 * 
 * Memory usage (per sessance, XCMD_LINE_MAX_LENGTH=64, HISTORY=8):
 * - Base: ~80 bytes
 * - Line editor: ~160 bytes  
 * - History: ~600 bytes
 * - Total: ~840 bytes per sessance
 */

#ifndef XCMD_SESSION_H
#define XCMD_SESSION_H

#include "xcmd_default_confg.h"
#include "xcmd_term.h"
#include "xcmd_line_editor.h"
#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Types
 *============================================================================*/

typedef struct xcmd_session xcmd_session_t;

typedef int (*xcmd_session_output_t)(xcmd_session_t *sess, const char *str, size_t len);

struct xcmd_session {
    xcmd_term_t term;
    xcmd_line_editor_t editor;
    
    int (*get_c)(uint8_t *);
    int (*put_c)(uint8_t);
    
    ssize_t write_fd;
    ssize_t read_fd;
    
    uint8_t (*recv_hook_func)(char *);
    
    uint8_t initialized;
    uint8_t session_id;
    
    void *user_data;
};

typedef struct {
    char line[XCMD_LINE_MAX_LENGTH + 1];
    uint16_t length;
    uint16_t cursor;
} xcmd_line_state_t;

/*============================================================================
 * Instance Management
 *============================================================================*/

int xcmd_session_init(xcmd_session_t *sess,
                       int (*get_c)(uint8_t *),
                       int (*put_c)(uint8_t),
                       const xcmd_term_ops_t *ops);

uint8_t xcmd_session_get_id(xcmd_session_t *sess);
void* xcmd_session_get_user_data(xcmd_session_t *sess);
void xcmd_session_set_user_data(xcmd_session_t *sess, void *data);

/*============================================================================
 * Instance Operations
 *============================================================================*/

int xcmd_session_task(xcmd_session_t *sess);

int xcmd_session_exec(xcmd_session_t *sess, char *str);

void xcmd_session_print(xcmd_session_t *sess, const char *fmt, ...);
void xcmd_session_put_str(xcmd_session_t *sess, const char *str);

const char* xcmd_session_get_line(xcmd_session_t *sess);
void xcmd_session_set_line(xcmd_session_t *sess, const char *str);
void xcmd_session_set_prompt(xcmd_session_t *sess, const char *prompt);
const char* xcmd_session_get_prompt(xcmd_session_t *sess);

void xcmd_session_set_recv_hook(xcmd_session_t *sess, 
                                  uint8_t (*func)(char *));

void xcmd_session_clear(xcmd_session_t *sess);
void xcmd_session_reset(xcmd_session_t *sess);

/*============================================================================
 * Display Operations (per-sessance)
 *============================================================================*/

void xcmd_session_display_insert_char(xcmd_session_t *sess, char c);
void xcmd_session_display_delete_char(xcmd_session_t *sess);
void xcmd_session_display_cursor_set(xcmd_session_t *sess, uint16_t pos);
uint16_t xcmd_session_display_cursor_get(xcmd_session_t *sess);
void xcmd_session_display_clear(xcmd_session_t *sess);
char* xcmd_session_display_get(xcmd_session_t *sess);
void xcmd_session_display_print(xcmd_session_t *sess, const char *fmt, ...);

/*============================================================================
 * History Operations (per-sessance)
 *============================================================================*/

#if XCMD_HISTORY_MAX_NUM > 0
void xcmd_session_history_append(xcmd_session_t *sess, const char *str);
char* xcmd_session_history_prev(xcmd_session_t *sess);
char* xcmd_session_history_next(xcmd_session_t *sess);
char* xcmd_session_history_current(xcmd_session_t *sess);
void xcmd_session_history_reset(xcmd_session_t *sess);
uint16_t xcmd_session_history_len(xcmd_session_t *sess);
#endif

/*============================================================================
 * Current Instance (for command execution context)
 *============================================================================*/

xcmd_session_t* xcmd_session_get_current(void);

/*============================================================================
 * Convenience Functions (use current sessance)
 *============================================================================*/

void xcmd_session_print_current(const char *fmt, ...);
void xcmd_session_history_insert_current(const char *str);
const char* xcmd_session_history_prev_current(void);
const char* xcmd_session_history_next_current(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_SESSION_H */