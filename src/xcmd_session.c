#include "xcmd_session.h"
#include "xcmd.h"
#include "xcmd_default_cmds.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static xcmd_session_t *g_current_session = NULL;

static xcmd_t g_cmd_list_head = {NULL, NULL, NULL, NULL};

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

static char *session_strpbrk(char *s, const char *delim) {
    uint8_t flag = 0;
    for (uint16_t i = 0; s[i]; i++) {
        for (uint16_t j = 0; delim[j]; j++) {
            if (flag == 0) {
                if (s[i] == '\"') {
                    for (uint16_t k = i; s[k]; k++) {
                        s[k] = s[k + 1];
                    }
                    flag = 1;
                    continue;
                }
            }
            if (flag) {
                if (s[i] == '\"') {
                    for (uint16_t k = i; s[k]; k++) {
                        s[k] = s[k + 1];
                    }
                    flag = 0;
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
    s = session_strpbrk(token, delim);
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

static int session_redirected(xcmd_session_t *sess, int argc, char *argv[]) {
    for (int i = 1; i < (argc - 1); i++) {
        if (argv[i][0] == '>') {
            argc = i;
            extern ssize_t file_open(char *name, int is_write, int is_append);
            extern void file_close(ssize_t fd);
            if (argv[i][1] == '>') {
                sess->write_fd = file_open(argv[i + 1], 1, 1);
            } else {
                sess->write_fd = file_open(argv[i + 1], 1, 0);
            }
            break;
        }
    }
    return argc;
}

static int session_cmd_match(xcmd_session_t *sess, int argc, char *argv[]) {
    uint8_t flag = 0;
    int ret = -1;
    xcmd_t *p = NULL;
    
    for (p = g_cmd_list_head.next; p; p = p->next) {
        if (strcmp(p->name, argv[0]) == 0) {
            flag = 1;
            if (argc > 1) {
                if ((strcmp(argv[1], "?") == 0) || (strcmp(argv[1], "-h") == 0)) {
                    xcmd_session_print(sess, "%s\r\n", p->help);
                    break;
                }
            }
            argc = session_redirected(sess, argc, argv);
            ret = p->func(argc, argv);
            extern void file_close(ssize_t fd);
            file_close(sess->write_fd);
            sess->write_fd = -1;
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

static void on_line_submit(xcmd_line_editor_t *le, const char *line) {
    xcmd_session_t *sess = (xcmd_session_t *)le->user_data;
    
    if (!sess) return;
    
    if (sess->recv_hook_func) {
        if (!sess->recv_hook_func((char *)line)) {
            return;
        }
    }
    
    if (line[0] != '\0') {
#if XCMD_HISTORY_MAX_NUM > 0
        xcmd_session_history_append(sess, line);
#endif
        xcmd_session_exec(sess, (char *)line);
    }
}

int xcmd_session_init(xcmd_session_t *sess,
                       int (*get_c)(uint8_t *),
                       int (*put_c)(uint8_t),
                       const xcmd_term_ops_t *ops) {
    static uint8_t first_init = 1;
    
    if (!sess || !get_c || !put_c) {
        return -1;
    }
    
    memset(sess, 0, sizeof(xcmd_session_t));
    
    sess->get_c = get_c;
    sess->put_c = put_c;
    sess->write_fd = -1;
    sess->read_fd = -1;
    
    xcmd_term_init(&sess->term, put_c, ops);
    xcmd_term_set_prompt(&sess->term, XCMD_DEFAULT_PROMPT);
    
    xcmd_le_init(&sess->editor, &sess->term);
    sess->editor.user_data = sess;
    xcmd_le_set_on_submit(&sess->editor, on_line_submit);
    
    sess->initialized = 1;
    
    if (first_init) {
        default_cmds_init();
        first_init = 0;
    }
    
    return 0;
}

int xcmd_session_task(xcmd_session_t *sess) {
    if (!sess || !sess->initialized) {
        return -1;
    }
    
    uint8_t c;
    if (sess->get_c(&c)) {
        xcmd_le_result_t result = xcmd_le_feed(&sess->editor, c);
        
        if (result == XCMD_LE_COMPLETE || result == XCMD_LE_CANCEL) {
            sess->editor.length = 0;
            sess->editor.cursor = 0;
            sess->editor.buffer[0] = '\0';
#if XCMD_HISTORY_MAX_NUM > 0
            xcmd_session_history_reset(sess);
#endif
            xcmd_term_write(&sess->term, sess->term.prompt);
        }
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
    
    g_current_session = sess;
    strncpy(temp, str, XCMD_LINE_MAX_LENGTH);
    param_num = session_get_param(temp, " ", cmd_param_buff, XCMD_PARAM_MAX_NUM);
    
    if (param_num > 0) {
        int ret = session_cmd_match(sess, param_num, cmd_param_buff);
        g_current_session = NULL;
        return ret;
    }
    
    g_current_session = NULL;
    return -1;
}

extern ssize_t file_open(char *name, int is_write, int is_append) __attribute__((weak));
extern void file_close(ssize_t fd) __attribute__((weak));
extern int file_write(ssize_t fd, const char *str) __attribute__((weak));
extern int file_read(ssize_t fd, char *buf, int buflen) __attribute__((weak));

void file_close(ssize_t fd) {}
ssize_t file_open(char *name, int is_write, int is_append) { return -1; }
int file_write(ssize_t fd, const char *str) { return -1; }
int file_read(ssize_t fd, char *buf, int buflen) { return -1; }

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
    if (!sess || !str || !sess->put_c) return;
    
    if (sess->write_fd != -1) {
        extern int file_write(ssize_t fd, const char *str);
        if (file_write(sess->write_fd, str) != -1) {
            return;
        }
    }
    
    for (uint16_t i = 0; str[i]; i++) {
        sess->put_c(str[i]);
    }
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
    if (sess && prompt) {
        xcmd_term_set_prompt(&sess->term, prompt);
    }
}

const char* xcmd_session_get_prompt(xcmd_session_t *sess) {
    return sess ? xcmd_term_get_prompt(&sess->term) : NULL;
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
        sess->write_fd = -1;
        sess->read_fd = -1;
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

typedef struct {
    char line[XCMD_LINE_MAX_LENGTH + 1];
    struct session_history *next;
    struct session_history *prev;
} session_history_t;

static void session_history_init(xcmd_session_t *sess) {
    memset(&sess->editor.history, 0, sizeof(sess->editor.history));
}

void xcmd_session_history_append(xcmd_session_t *sess, const char *str) {
    if (!sess || !str || str[0] == '\0') return;
    xcmd_le_history_append(&sess->editor, str);
}

char* xcmd_session_history_prev(xcmd_session_t *sess) {
    return sess ? (char *)xcmd_le_history_prev(&sess->editor) : NULL;
}

char* xcmd_session_history_next(xcmd_session_t *sess) {
    return sess ? (char *)xcmd_le_history_next(&sess->editor) : NULL;
}

char* xcmd_session_history_current(xcmd_session_t *sess) {
    return sess ? (char *)xcmd_le_history_current(&sess->editor) : NULL;
}

void xcmd_session_history_reset(xcmd_session_t *sess) {
    if (sess) {
        xcmd_le_history_reset(&sess->editor);
    }
}

uint16_t xcmd_session_history_len(xcmd_session_t *sess) {
    return sess ? xcmd_le_history_count(&sess->editor) : 0;
}

#endif

int xcmd_cmd_register(xcmd_t *cmds, uint16_t number) {
#ifndef ENABLE_XCMD_EXPORT
    uint16_t i = 0;
    xcmd_t *temp;
    while (i < number) {
        temp = g_cmd_list_head.next;
        g_cmd_list_head.next = &cmds[i];
        cmds[i].next = temp;
        ++i;
    }
#endif
    return 0;
}

xcmd_t* xcmd_cmdlist_get(void) {
#ifndef ENABLE_XCMD_EXPORT
    return g_cmd_list_head.next;
#else
    return NULL;
#endif
}

int xcmd_unregister_cmd(char *cmd) {
#ifndef ENABLE_XCMD_EXPORT
    xcmd_t *bk = &g_cmd_list_head;
    xcmd_t *p = NULL;
    for (p = g_cmd_list_head.next; p; p = p->next) {
        if (strcmp(cmd, p->name) == 0) {
            bk->next = p->next;
            return 0;
        }
        bk = p;
    }
#endif
    return -1;
}

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

void xcmd_session_history_insert_current(const char *str) {
    if (g_current_session) {
        xcmd_session_history_append(g_current_session, str);
    }
}

const char* xcmd_session_history_prev_current(void) {
    return g_current_session ? xcmd_session_history_prev(g_current_session) : NULL;
}

const char* xcmd_session_history_next_current(void) {
    return g_current_session ? xcmd_session_history_next(g_current_session) : NULL;
}