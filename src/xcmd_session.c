#include "xcmd_session.h"
#include "xcmd_cmd.h"
#include "xcmd_default_cmds.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static xcmd_session_t *g_current_session = NULL;

static int session_complete_hook(xcmd_line_editor_t *le, 
                                const char *word,
                                int word_id,
                                char *matches[],
                                int max) {
    if (word_id == 0) {
        return xcmd_complete_default(word, matches, max);
    }
    
    const char *cmd_word = xcmd_le_get_word(le, 0);
    xcmd_cmd_t *cmd = xcmd_cmd_find(cmd_word);
    
    if (cmd && cmd->complete) {
        return cmd->complete(le, word, word_id, matches, max);
    }
    
    xcmd_complete_cb path_cb = xcmd_get_path_complete();
    if (path_cb) {
        return path_cb(le, word, word_id, matches, max);
    }
    
    return 0;
}

uint8_t xcmd_session_get_id(xcmd_session_t *sess) {
    return sess ? sess->session_id : 0xFF;
}

void* xcmd_session_get_user_data(xcmd_session_t *sess) {
    return sess ? sess->user_data : NULL;
}

void xcmd_session_set_user_data(xcmd_session_t *sess, void *data) {
    if (sess) {
        sess->user_data = data;
    }
}

static char *session_strpbrk(char *s, size_t len, const char *delim) {
    uint8_t flag = 0;
    for (size_t i = 0; i < len && s[i] != '\0'; i++) {
        for (size_t j = 0; delim[j] != '\0'; j++) {
            if (flag == 0) {
                if (s[i] == '\"') {
                    /* Shift string left to remove quote, check boundary */
                    for (size_t k = i; k < len - 1 && s[k] != '\0'; k++) {
                        s[k] = s[k + 1];
                    }
                    /* Ensure null termination after shift */
                    s[len - 1] = '\0';
                    flag = 1;
                    i--; /* Re-check current position */
                    continue;
                }
            }
            if (flag) {
                if (s[i] == '\"') {
                    /* Shift string left to remove quote, check boundary */
                    for (size_t k = i; k < len - 1 && s[k] != '\0'; k++) {
                        s[k] = s[k + 1];
                    }
                    /* Ensure null termination after shift */
                    s[len - 1] = '\0';
                    flag = 0;
                    i--; /* Re-check current position */
                } else {
                    continue;
                }
            }
            if (s[i] == delim[j]) {
                return &s[i];
            }
        }
    }
    return NULL;
}

static char *session_strtok(char *s, const char *delim, char **save_ptr) {
    char *token;
    if (s == NULL) s = *save_ptr;
    s += strspn(s, delim);
    if (*s == '\0') return NULL;
    token = s;
    s = session_strpbrk(token, strlen(token) + 1, delim);
    if (s == NULL) {
        *save_ptr = strchr(token, '\0');
    } else {
        *s = '\0';
        *save_ptr = s + 1;
    }
    return token;
}

static int session_get_param(char *msg, char *delim, char *get[], int max_num) {
    int i;
    char *ptr = NULL;
    char *sp = NULL;
    ptr = session_strtok(msg, delim, &sp);
    for (i = 0; ptr != NULL && i < max_num; i++) {
        get[i] = ptr;
        ptr = session_strtok(NULL, delim, &sp);
    }
    return i;
}

static int session_cmd_match(xcmd_session_t *sess, int argc, char **argv) {
    uint8_t flag = 0;
    int ret = -1;
    xcmd_cmd_t *p = NULL;
    
    for (p = xcmd_cmd_first(); p; p = p->next) {
        if (strcmp(p->name, argv[0]) == 0) {
            flag = 1;
            if (argc > 1) {
                if ((strcmp(argv[1], "?") == 0) || (strcmp(argv[1], "-h") == 0)) {
                    xcmd_session_print(sess, "%s\r\n", p->help);
                    break;
                }
            }
            ret = p->func(argc, argv);
            break;
        }
    }
    
    if (flag) {
        xcmd_session_print(sess, "\r\n");
    } else {
        xcmd_session_print(sess, "cmd \"%s\" does not exist\r\n", argv[0]);
    }
    
    return ret;
}

int xcmd_session_init(xcmd_session_t *sess,
                      xcmd_io_t *io,
                      const xcmd_term_ops_t *ops) {
    return xcmd_session_init_full(sess, io, io, ops);
}

int xcmd_session_init_full(xcmd_session_t *sess,
                           xcmd_io_t *stdin_io,
                           xcmd_io_t *stdout_io,
                           const xcmd_term_ops_t *ops) {
    static uint8_t first_init = 1;
    
    if (!sess || !stdin_io || !stdout_io) {
        return -1;
    }
    
    memset(sess, 0, sizeof(xcmd_session_t));
    
    sess->stdin_io = stdin_io;
    sess->stdout_io = stdout_io;
    sess->saved_stdout = NULL;
    
    xcmd_term_init(&sess->term, ops);
    xcmd_term_set_io(&sess->term, stdout_io);
    
    xcmd_le_init(&sess->editor, &sess->term);
    sess->editor.user_data = sess;
    xcmd_le_set_complete_cb(&sess->editor, session_complete_hook);
    
    sess->initialized = 1;
    
    if (first_init) {
        default_cmds_init();
        first_init = 0;
    }
    
    return 0;
}

int xcmd_session_task(xcmd_session_t *sess) {
    if (!sess || !sess->initialized || !sess->stdin_io) {
        return -1;
    }
    
    g_current_session = sess;
    
    const char *line = xcmd_le_readline(&sess->editor, sess->stdin_io, sess->stdout_io, XCMD_DEFAULT_PROMPT);
    
    if (line) {
        char temp[XCMD_LINE_MAX_LENGTH];
        strncpy(temp, line, sizeof(temp));
        temp[XCMD_LINE_MAX_LENGTH - 1] = '\0';
        
        char *argv[XCMD_PARAM_MAX_NUM];
        int argc = session_get_param(temp, " ", argv, XCMD_PARAM_MAX_NUM);
        int new_argc = argc;
        
        xcmd_evt_t evt = {
            .type = XCMD_EVT_PRE_EXEC,
            .data.pre_exec = { argc, argv, &new_argc }
        };
        xcmd_ext_emit(sess, &evt);
        
        int ret = session_cmd_match(sess, new_argc, argv);
        
        if (sess->saved_stdout) {
            if (sess->stdout_io && sess->stdout_io->close) {
                sess->stdout_io->close(sess->stdout_io);
            }
            sess->stdout_io = sess->saved_stdout;
            sess->saved_stdout = NULL;
        }
        
        xcmd_le_reset_line(&sess->editor);
        g_current_session = NULL;
        return ret;
    }
    
    g_current_session = NULL;
    return 0;
}

int xcmd_session_exec(xcmd_session_t *sess, char *str) {
    if (!sess || !str) {
        return -1;
    }
    
    char temp[XCMD_LINE_MAX_LENGTH];
    strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
    temp[XCMD_LINE_MAX_LENGTH - 1] = '\0';
    
    char *argv[XCMD_PARAM_MAX_NUM];
    int argc = session_get_param(temp, " ", argv, XCMD_PARAM_MAX_NUM);
    
    if (argc <= 0) {
        return -1;
    }
    
    return session_cmd_match(sess, argc, argv);
}

xcmd_io_t* xcmd_session_get_stdin(xcmd_session_t *sess) {
    return sess ? sess->stdin_io : NULL;
}

xcmd_io_t* xcmd_session_get_stdout(xcmd_session_t *sess) {
    return sess ? sess->stdout_io : NULL;
}

void xcmd_session_print(xcmd_session_t *sess, const char *fmt, ...) {
    if (!sess) return;
    
    char buf[XCMD_PRINT_BUF_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buf, XCMD_PRINT_BUF_MAX_LENGTH, fmt, arg);
    va_end(arg);
    
    xcmd_session_put_str(sess, buf);
}

void xcmd_session_put_str(xcmd_session_t *sess, const char *str) {
    if (!sess || !str || !sess->stdout_io) return;
    
    xcmd_io_puts(sess->stdout_io, str);
}

const char* xcmd_session_get_line(xcmd_session_t *sess) {
    return sess ? sess->editor.buffer : NULL;
}

void xcmd_session_set_line(xcmd_session_t *sess, const char *str) {
    if (sess && str) {
        xcmd_le_set_line(&sess->editor, str);
    }
}

void xcmd_session_set_prompt(xcmd_session_t *sess, const char *prompt) {
    (void)sess;
    (void)prompt;
}

const char* xcmd_session_get_prompt(xcmd_session_t *sess) {
    return sess ? sess->editor.prompt : NULL;
}

void xcmd_session_clear(xcmd_session_t *sess) {
    if (sess) {
        xcmd_le_clear_line(&sess->editor);
    }
}

void xcmd_session_reset(xcmd_session_t *sess) {
    if (sess) {
        sess->editor.length = 0;
        sess->editor.cursor = 0;
        sess->editor.buffer[0] = '\0';
        sess->saved_stdout = NULL;
    }
}

void xcmd_session_display_insert_char(xcmd_session_t *sess, char c) {
    if (sess) {
        xcmd_le_insert_char(&sess->editor, c);
    }
}

void xcmd_session_display_delete_char(xcmd_session_t *sess) {
    if (sess) {
        xcmd_le_delete_char(&sess->editor);
    }
}

void xcmd_session_display_cursor_set(xcmd_session_t *sess, uint16_t pos) {
    if (sess) {
        xcmd_le_cursor_home(&sess->editor);
        for (uint16_t i = 0; i < pos && i < sess->editor.length; i++) {
            xcmd_le_cursor_right(&sess->editor);
        }
    }
}

uint16_t xcmd_session_display_cursor_get(xcmd_session_t *sess) {
    return sess ? sess->editor.cursor : 0;
}

void xcmd_session_display_clear(xcmd_session_t *sess) {
    if (sess) {
        xcmd_le_clear_line(&sess->editor);
    }
}

char* xcmd_session_display_get(xcmd_session_t *sess) {
    return sess ? sess->editor.buffer : NULL;
}

void xcmd_session_display_print(xcmd_session_t *sess, const char *fmt, ...) {
    if (!sess) return;
    
    char buf[XCMD_PRINT_BUF_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buf, XCMD_PRINT_BUF_MAX_LENGTH, fmt, arg);
    va_end(arg);
    
    xcmd_le_set_line(&sess->editor, buf);
}

#if XCMD_HISTORY_MAX_NUM > 0
void xcmd_session_history_insert_current(const char *str) {
    if (g_current_session) {
        xcmd_le_history_append(&g_current_session->editor, str);
    }
}

const char* xcmd_session_history_prev_current(void) {
    return g_current_session ? xcmd_le_history_prev(&g_current_session->editor) : NULL;
}

const char* xcmd_session_history_next_current(void) {
    return g_current_session ? xcmd_le_history_next(&g_current_session->editor) : NULL;
}

uint16_t xcmd_session_history_len(xcmd_session_t *sess) {
    return sess ? xcmd_le_history_count(&sess->editor) : 0;
}
#endif

xcmd_session_t* xcmd_session_get_current(void) {
    return g_current_session;
}

void xcmd_session_print_current(const char *fmt, ...) {
    if (!g_current_session) return;
    
    char buf[XCMD_PRINT_BUF_MAX_LENGTH] = {0};
    va_list arg;
    va_start(arg, fmt);
    vsnprintf(buf, XCMD_PRINT_BUF_MAX_LENGTH, fmt, arg);
    va_end(arg);
    
    xcmd_session_put_str(g_current_session, buf);
}
