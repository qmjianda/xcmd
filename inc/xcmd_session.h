#ifndef XCMD_SESSION_H
#define XCMD_SESSION_H

#include "xcmd_default_confg.h"
#include "xcmd_io.h"
#include "xcmd_term.h"
#include "xcmd_line_editor.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Types
 *============================================================================*/

typedef struct xcmd_session xcmd_session_t;
typedef struct xcmd_vfs xcmd_vfs_t;

struct xcmd_session {
    xcmd_term_t term;
    xcmd_line_editor_t editor;
    
    xcmd_io_t *stdin_io;
    xcmd_io_t *stdout_io;
    
#ifdef XCMD_USE_VFS
    xcmd_vfs_t *vfs;
#endif
    
    xcmd_io_t *redirect_orig_stdout;
    uint8_t redirect_active;
    
    uint8_t (*recv_hook_func)(char *);
    int (*complete_hook_func)(xcmd_line_editor_t *le,
                              const char *line,
                              uint16_t cursor,
                              char *matches[],
                              uint16_t max_matches);
    
    uint8_t initialized;
    uint8_t session_id;
    
    void *user_data;
};

/**
 * @brief RAII guard for IO redirection
 * 
 * Automatically restores stdout on scope exit, ensuring no resource leaks.
 * 
 * Usage:
 *   xcmd_io_redirect_guard_t guard = {0};
 *   xcmd_io_redirect_guard_enter(&guard, sess, new_stdout);
 *   // ... do work ...
 *   xcmd_io_redirect_guard_exit(&guard);  // or just let guard go out of scope
 */
typedef struct {
    xcmd_session_t *sess;
    xcmd_io_t *orig_stdout;
    uint8_t active;
} xcmd_io_redirect_guard_t;

/*============================================================================
 * Instance Management
 *============================================================================*/

int xcmd_session_init(xcmd_session_t *sess,
                      xcmd_io_t *io,
                      const xcmd_term_ops_t *ops);

int xcmd_session_init_full(xcmd_session_t *sess,
                           xcmd_io_t *stdin_io,
                           xcmd_io_t *stdout_io,
                           const xcmd_term_ops_t *ops);

#ifdef XCMD_USE_VFS
void xcmd_session_set_vfs(xcmd_session_t *sess, xcmd_vfs_t *vfs);
xcmd_vfs_t* xcmd_session_get_vfs(xcmd_session_t *sess);
#endif

uint8_t xcmd_session_get_id(xcmd_session_t *sess);
void* xcmd_session_get_user_data(xcmd_session_t *sess);
void xcmd_session_set_user_data(xcmd_session_t *sess, void *data);

/*============================================================================
 * IO Operations
 *============================================================================*/

xcmd_io_t* xcmd_session_get_stdin(xcmd_session_t *sess);
xcmd_io_t* xcmd_session_get_stdout(xcmd_session_t *sess);

int xcmd_session_redirect(xcmd_session_t *sess, xcmd_io_t *new_stdout);
int xcmd_session_redirect_restore(xcmd_session_t *sess);

void xcmd_io_redirect_guard_enter(xcmd_io_redirect_guard_t *guard,
                                  xcmd_session_t *sess,
                                  xcmd_io_t *new_stdout);

/**
 * @brief Exit redirect guard (restore original stdout, close redirected IO)
 */
void xcmd_io_redirect_guard_exit(xcmd_io_redirect_guard_t *guard);

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

void xcmd_session_set_complete_hook(xcmd_session_t *sess,
                                    int (*func)(xcmd_line_editor_t *le,
                                                const char *line,
                                                uint16_t cursor,
                                                char *matches[],
                                                uint16_t max_matches));

void xcmd_session_clear(xcmd_session_t *sess);
void xcmd_session_reset(xcmd_session_t *sess);

/*============================================================================
 * Display Operations
 *============================================================================*/

void xcmd_session_display_insert_char(xcmd_session_t *sess, char c);
void xcmd_session_display_delete_char(xcmd_session_t *sess);
void xcmd_session_display_cursor_set(xcmd_session_t *sess, uint16_t pos);
uint16_t xcmd_session_display_cursor_get(xcmd_session_t *sess);
void xcmd_session_display_clear(xcmd_session_t *sess);
char* xcmd_session_display_get(xcmd_session_t *sess);
void xcmd_session_display_print(xcmd_session_t *sess, const char *fmt, ...);

/*============================================================================
 * History Operations
 *============================================================================*/

#if XCMD_HISTORY_MAX_NUM > 0
void xcmd_session_history_insert_current(const char *str);
const char* xcmd_session_history_prev_current(void);
const char* xcmd_session_history_next_current(void);
uint16_t xcmd_session_history_len(xcmd_session_t *sess);
#endif

/*============================================================================
 * Current Instance (for command execution context)
 *============================================================================*/

xcmd_session_t* xcmd_session_get_current(void);

/*============================================================================
 * Convenience Functions (use current session)
 *============================================================================*/

void xcmd_session_print_current(const char *fmt, ...);

/*============================================================================
 * Legacy Compatibility (deprecated)
 *============================================================================*/

#ifndef XCMD_NO_DEPRECATED
int xcmd_session_init_legacy(xcmd_session_t *sess,
                              int (*get_c)(uint8_t *),
                              int (*put_c)(uint8_t),
                              const xcmd_term_ops_t *ops);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XCMD_SESSION_H */