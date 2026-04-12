#include "xcmd_session.h"
#include "xcmd_cmd.h"
#include "xcmd_complete.h"
#include "xcmd_default_cmds.h"

#ifdef XCMD_USE_VFS
#include "xcmd_vfs.h"
#include "xcmd_vfs_redirect.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static xcmd_session_t *g_current_session = NULL;

static int session_complete_hook(xcmd_line_editor_t *le, 
                                const char *partial,
                                uint16_t partial_start,
                                char *matches[],
                                uint16_t max_matches) {
    xcmd_complete_result_t result;
    result.count = 0;
    
    const char *line = le->buffer;
    
    int word_count = 0;
    uint16_t word_starts[8] = {0};
    uint16_t word_ends[8] = {0};
    
    for (uint16_t i = 0; i <= partial_start && word_count < 8; i++) {
        if (i == 0 || (i > 0 && line[i-1] == ' ')) {
            word_starts[word_count] = i;
            while (i < le->length && line[i] != ' ') i++;
            word_ends[word_count] = i;
            word_count++;
        }
    }
    
    if (word_count == 0 || partial_start < word_starts[0]) {
        return xcmd_complete_cmd(partial, &result);
    }
    
    const char *cmd_name = line + word_starts[0];
    size_t cmd_len = word_ends[0] - word_starts[0];
    char cmd[64] = {0};
    if (cmd_len >= sizeof(cmd)) cmd_len = sizeof(cmd) - 1;
    memcpy(cmd, cmd_name, cmd_len);
    
    const xcmd_cmd_tree_t *tree = xcmd_cmd_tree_find(cmd);
    
    if (tree && tree->subcmds && tree->subcmd_count > 0) {
        if (word_count == 1 || (word_count == 2 && partial_start >= word_starts[1])) {
            const char *subcmd_partial = "";
            size_t subcmd_len = 0;
            
            if (word_count >= 2 && partial_start >= word_starts[1]) {
                subcmd_partial = line + word_starts[1];
                subcmd_len = word_ends[1] - word_starts[1];
                char tmp[64] = {0};
                if (subcmd_len >= sizeof(tmp)) subcmd_len = sizeof(tmp) - 1;
                memcpy(tmp, subcmd_partial, subcmd_len);
                subcmd_partial = tmp;
            }
            
            if (xcmd_complete_subcmd(subcmd_partial, tree->subcmds, tree->subcmd_count, &result) > 0) {
                int n = result.count < max_matches ? result.count : max_matches;
                for (int i = 0; i < n; i++) {
                    matches[i] = result.candidates[i];
                }
                return n;
            }
        }
        
        if (word_count >= 2) {
            char subcmd_name[64] = {0};
            size_t subcmd_len = word_ends[1] - word_starts[1];
            if (subcmd_len >= sizeof(subcmd_name)) subcmd_len = sizeof(subcmd_name) - 1;
            memcpy(subcmd_name, line + word_starts[1], subcmd_len);
            
            for (int i = 0; i < tree->subcmd_count; i++) {
                if (strcmp(tree->subcmds[i].name, subcmd_name) == 0) {
                    if (word_count == 2 || (word_count == 3 && partial_start >= word_starts[2])) {
                        const xcmd_subcmd_def_t *subcmd = &tree->subcmds[i];
                        
                        if (subcmd->params && subcmd->param_count > 0) {
                            int param_idx = (word_count >= 3) ? (word_count - 3) : 0;
                            
                            if (param_idx < subcmd->param_count) {
                                const char *param_partial = "";
                                if (word_count >= 3 && partial_start >= word_starts[2]) {
                                    param_partial = line + word_starts[2];
                                }
                                
                                if (xcmd_complete_param(param_partial, 
                                                       subcmd->params[param_idx].type,
                                                       subcmd->params[param_idx].enum_vals,
                                                       &result) > 0) {
                                    int n = result.count < max_matches ? result.count : max_matches;
                                    for (int j = 0; j < n; j++) {
                                        matches[j] = result.candidates[j];
                                    }
                                    return n;
                                }
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    
    if (word_count == 1) {
        return xcmd_complete_cmd(partial, &result);
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

#ifdef XCMD_USE_VFS
void xcmd_session_set_vfs(xcmd_session_t *sess, xcmd_vfs_t *vfs) {
    if (sess) {
        sess->vfs = vfs;
    }
}

xcmd_vfs_t* xcmd_session_get_vfs(xcmd_session_t *sess) {
    return sess ? sess->vfs : NULL;
}
#endif

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

#ifdef XCMD_USE_VFS
static xcmd_io_t* parse_redirect_target(xcmd_session_t *sess, const char *target) {
    if (!sess || !target) return NULL;
    if (!sess->vfs) return NULL;
    
    return xcmd_vfs_redirect_open(sess->vfs, target);
}
#else
static xcmd_io_t* parse_redirect_target(xcmd_session_t *sess, const char *target) {
    (void)sess;
    (void)target;
    return NULL;
}
#endif

static int session_redirected(xcmd_session_t *sess, int argc, char *argv[]) {
    for (int i = 1; i < (argc - 1); i++) {
        if (argv[i][0] == '>') {
            const char *target = argv[i + 1];
            xcmd_io_t *io = parse_redirect_target(sess, target);
            
            if (!io) {
                xcmd_session_print(sess, "Cannot open: %s\r\n", target);
                return argc;
            }
            
            sess->redirect_orig_stdout = sess->stdout_io;
            sess->stdout_io = io;
            sess->redirect_active = 1;
            
            return i;
        }
    }
    return argc;
}


void xcmd_io_redirect_guard_enter(xcmd_io_redirect_guard_t *guard,
                                  xcmd_session_t *sess,
                                  xcmd_io_t *new_stdout) {
    guard->sess = sess;
    guard->orig_stdout = sess->stdout_io;
    guard->active = 1;
    sess->stdout_io = new_stdout;
}

void xcmd_io_redirect_guard_exit(xcmd_io_redirect_guard_t *guard) {
    if (!guard->active || !guard->sess) return;
    
    if (guard->sess->stdout_io) {
        xcmd_io_close(guard->sess->stdout_io);
    }
    guard->sess->stdout_io = guard->orig_stdout;
    guard->active = 0;
}

static int session_cmd_match(xcmd_session_t *sess, int argc, char *argv[]) {
    uint8_t flag = 0;
    int ret = -1;
    xcmd_cmd_t *p = NULL;
    xcmd_io_redirect_guard_t guard = {0};
    
    for (p = xcmd_cmd_first(); p; p = p->next) {
        if (strcmp(p->name, argv[0]) == 0) {
            flag = 1;
            if (argc > 1) {
                if ((strcmp(argv[1], "?") == 0) || (strcmp(argv[1], "-h") == 0)) {
                    xcmd_session_print(sess, "%s\r\n", p->help);
                    break;
                }
            }
            
            int new_argc = session_redirected(sess, argc, argv);
            if (new_argc != argc && sess->redirect_active) {
                xcmd_io_redirect_guard_enter(&guard, sess, sess->stdout_io);
            }
            
            ret = p->func(new_argc, argv);
            xcmd_io_redirect_guard_exit(&guard);
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
    
    sess->redirect_orig_stdout = NULL;
    sess->redirect_active = 0;
    
    xcmd_term_init(&sess->term, ops);
    
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
    
    const char *line = xcmd_le_readline(&sess->editor, sess->stdin_io, sess->stdout_io, XCMD_DEFAULT_PROMPT);
    
    if (line) {
        if (sess->recv_hook_func) {
            if (!sess->recv_hook_func((char *)line)) {
                return 0;
            }
        }
        xcmd_session_exec(sess, (char *)line);
        xcmd_le_reset_line(&sess->editor);
    }
    
    return 0;
}

int xcmd_session_exec(xcmd_session_t *sess, char *str) {
    if (!sess || !str) {
        return -1;
    }
    
    int param_num = 0;
    char *cmd_param_buff[XCMD_PARAM_MAX_NUM];
    char temp[XCMD_LINE_MAX_LENGTH];
    
    XCMD_SESSION_LOCK(sess);
    g_current_session = sess;
    strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
    temp[XCMD_LINE_MAX_LENGTH - 1] = '\0';
    param_num = session_get_param(temp, " ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
    
    if (param_num > 0) {
        int ret = session_cmd_match(sess, param_num, cmd_param_buff);
        g_current_session = NULL;
        XCMD_SESSION_UNLOCK(sess);
        return ret;
    }
    
    g_current_session = NULL;
    XCMD_SESSION_UNLOCK(sess);
    return -1;
}

xcmd_io_t* xcmd_session_get_stdin(xcmd_session_t *sess) {
    return sess ? sess->stdin_io : NULL;
}

xcmd_io_t* xcmd_session_get_stdout(xcmd_session_t *sess) {
    return sess ? sess->stdout_io : NULL;
}

int xcmd_session_redirect(xcmd_session_t *sess, xcmd_io_t *new_stdout) {
    if (!sess || !new_stdout) return -1;
    
    sess->redirect_orig_stdout = sess->stdout_io;
    sess->stdout_io = new_stdout;
    sess->redirect_active = 1;
    
    return 0;
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

void xcmd_session_set_recv_hook(xcmd_session_t *sess, uint8_t (*func)(char *)) {
    if (sess) {
        sess->recv_hook_func = func;
    }
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
        sess->redirect_active = 0;
        sess->redirect_orig_stdout = NULL;
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
