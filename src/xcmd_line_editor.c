#include "xcmd_line_editor.h"
#include "xcmd_term.h"
#include <string.h>
#include <ctype.h>

#define IS_ALPHA(c)  (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define IS_NUMBER(c)  ((c) >= '0' && (c) <= '9')
#define IS_PRINT(c)    ((c) >= 32 && (c) <= 126)

#define ESC_STATE_IDLE     0
#define ESC_STATE_ESC      1
#define ESC_STATE_BRACKET  2
#define ESC_STATE_END      3

static void term_putc(xcmd_line_editor_t *le, uint8_t c) {
    xcmd_term_putc(le->term, c);
}

static void term_write(xcmd_line_editor_t *le, const char *str) {
    xcmd_term_write(le->term, str);
}

static void refresh_line(xcmd_line_editor_t *le) {
    xcmd_term_refresh_line(le->term, le->buffer, le->cursor, le->length);
}

int xcmd_le_init(xcmd_line_editor_t *le, 
                 xcmd_term_t *term) {
    if (!le || !term) return -1;
    
    memset(le, 0, sizeof(xcmd_line_editor_t));
    le->term = term;
    le->length = 0;
    le->cursor = 0;
    le->buffer[0] = '\0';
    le->escape_state = ESC_STATE_IDLE;
    le->escape_len = 0;
    le->on_submit = NULL;
    
    return 0;
}

void xcmd_le_set_on_submit(xcmd_line_editor_t *le, xcmd_le_submit_cb cb) {
    if (le) {
        le->on_submit = cb;
    }
}

void xcmd_le_insert_char(xcmd_line_editor_t *le, char c) {
    if (!le || le->length >= XCMD_LE_LINE_MAX_LEN) return;
    
    if (le->cursor == le->length) {
        le->buffer[le->cursor] = c;
        le->buffer[le->cursor + 1] = '\0';
        le->length++;
        le->cursor++;
        
        term_putc(le, c);
    } else {
        memmove(&le->buffer[le->cursor + 1], 
                &le->buffer[le->cursor],
                le->length - le->cursor);
        le->buffer[le->cursor] = c;
        le->length++;
        le->buffer[le->length] = '\0';
        le->cursor++;
        
        if (le->term && le->term->ops && le->term->ops->insert_chars) {
            le->term->ops->insert_chars(le->term, 1);
            term_putc(le, c);
        } else {
            refresh_line(le);
        }
    }
}

void xcmd_le_delete_char(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;
    
    if (le->cursor <= le->length) {
        memmove(&le->buffer[le->cursor - 1],
                &le->buffer[le->cursor],
                le->length - le->cursor + 1);
    }
    le->cursor--;
    le->length--;
    
    if (le->term && le->term->ops && le->term->ops->delete_chars && le->term->ops->cursor_left) {
        le->term->ops->cursor_left(le->term, 1);
        le->term->ops->delete_chars(le->term, 1);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_delete_char_forward(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;
    
    memmove(&le->buffer[le->cursor],
            &le->buffer[le->cursor + 1],
            le->length - le->cursor);
    le->length--;
    le->buffer[le->length] = '\0';
    
    if (le->term && le->term->ops && le->term->ops->delete_chars) {
        le->term->ops->delete_chars(le->term, 1);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_cursor_left(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;

    le->cursor--;
    if (le->term && le->term->ops && le->term->ops->cursor_left) {
        le->term->ops->cursor_left(le->term, 1);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_cursor_right(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;
    
    le->cursor++;
    if (le->term && le->term->ops && le->term->ops->cursor_right) {
        le->term->ops->cursor_right(le->term, 1);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_cursor_home(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;

    le->cursor = 0;
    if (le->term && le->term->ops && le->term->ops->cursor_left) {
        le->term->ops->cursor_left(le->term, le->cursor);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_cursor_end(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;

    le->cursor = le->length;
    if (le->term && le->term->ops && le->term->ops->cursor_right) {
        le->term->ops->cursor_right(le->term, le->length - le->cursor);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_word_left(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;

    while (le->cursor > 0 && !IS_ALPHA(le->buffer[le->cursor - 1]) &&
           !IS_NUMBER(le->buffer[le->cursor - 1])) {
        le->cursor--;
    }
    while (le->cursor > 0 && (IS_ALPHA(le->buffer[le->cursor - 1]) ||
           IS_NUMBER(le->buffer[le->cursor - 1]))) {
        le->cursor--;
    }

    if (le->term && le->term->ops && le->term->ops->cursor_set) {
        le->term->ops->cursor_set(le->term, le->cursor);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_word_right(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;

    while (le->cursor < le->length && !IS_ALPHA(le->buffer[le->cursor]) &&
           !IS_NUMBER(le->buffer[le->cursor])) {
        le->cursor++;
    }
    while (le->cursor < le->length && (IS_ALPHA(le->buffer[le->cursor]) ||
           IS_NUMBER(le->buffer[le->cursor]))) {
        le->cursor++;
    }

    if (le->term && le->term->ops && le->term->ops->cursor_set) {
        le->term->ops->cursor_set(le->term, le->cursor);
    } else {
        refresh_line(le);
    }
}

void xcmd_le_delete_to_start(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;
    
    uint16_t n = le->cursor;
    memmove(le->buffer, &le->buffer[le->cursor], le->length - le->cursor + 1);
    le->length -= n;
    le->cursor = 0;
    
    refresh_line(le);
}

void xcmd_le_delete_to_end(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;
    
    le->length = le->cursor;
    le->buffer[le->length] = '\0';
    
    if (le->term && le->term->ops && le->term->ops->clear_to_eol) {
        le->term->ops->clear_to_eol(le->term);
    }
    else {
        refresh_line(le);
    }
}

void xcmd_le_clear_line(xcmd_line_editor_t *le) {
    if (!le) return;
    
    le->length = 0;
    le->cursor = 0;
    le->buffer[0] = '\0';
    
    if (le->term && le->term->ops && le->term->ops->clear_line) {
        le->term->ops->clear_line(le->term);
    }
    else {
        refresh_line(le);
    }
}

void xcmd_le_set_line(xcmd_line_editor_t *le, const char *str) {
    if (!le || !str) return;
    
    strncpy(le->buffer, str, XCMD_LE_LINE_MAX_LEN);
    le->buffer[XCMD_LE_LINE_MAX_LEN] = '\0';
    le->length = strlen(le->buffer);
    le->cursor = le->length;
    
    refresh_line(le);
}

const char* xcmd_le_get_line(xcmd_line_editor_t *le) {
    return le ? le->buffer : NULL;
}

uint16_t xcmd_le_get_cursor(xcmd_line_editor_t *le) {
    return le ? le->cursor : 0;
}

uint16_t xcmd_le_get_length(xcmd_line_editor_t *le) {
    return le ? le->length : 0;
}

static bool is_escape_end(uint8_t c) {
    return ((c >= 'A' && c <= 'D') || 
            (c >= 'F' && c <= 'H') ||
            (c >= 'P' && c <= 'S') || 
            (c == '~') ||
            (c >= 'a' && c <= 'z'));
}

xcmd_le_result_t xcmd_le_feed(xcmd_line_editor_t *le, uint8_t byte) {
    if (!le) return XCMD_LE_EOF;
    
    switch (le->escape_state) {
    case ESC_STATE_IDLE:
        if (byte == 0x1B) {
            le->escape_buf[0] = byte;
            le->escape_len = 1;
            le->escape_state = ESC_STATE_ESC;
            return XCMD_LE_MORE;
        }
        break;
        
    case ESC_STATE_ESC:
        le->escape_buf[le->escape_len++] = byte;
        if (byte == '[') {
            le->escape_state = ESC_STATE_BRACKET;
            return XCMD_LE_MORE;
        }
        le->escape_len = 0;
        le->escape_state = ESC_STATE_IDLE;
        return XCMD_LE_MORE;
        
    case ESC_STATE_BRACKET:
        le->escape_buf[le->escape_len++] = byte;
        if (is_escape_end(byte)) {
            le->escape_buf[le->escape_len] = '\0';
            le->escape_state = ESC_STATE_END;
            
            if (memcmp(le->escape_buf, "\x1B[A", 3) == 0 ||
                memcmp(le->escape_buf, "\x1B[B", 3) == 0) {
#if XCMD_LE_HISTORY_MAX_NUM > 0
                if (le->escape_buf[2] == 'A') {
                    xcmd_le_key_up(le, le->escape_buf);
                } else {
                    xcmd_le_key_down(le, le->escape_buf);
                }
#endif
            } else if (memcmp(le->escape_buf, "\x1B[C", 3) == 0) {
                xcmd_le_cursor_right(le);
            } else if (memcmp(le->escape_buf, "\x1B[D", 3) == 0) {
                xcmd_le_cursor_left(le);
            } else if (memcmp(le->escape_buf, "\x1B[H", 3) == 0 ||
                       memcmp(le->escape_buf, "\x1B[1~", 4) == 0) {
                xcmd_le_cursor_home(le);
            } else if (memcmp(le->escape_buf, "\x1B[F", 3) == 0 ||
                       memcmp(le->escape_buf, "\x1B[4~", 4) == 0) {
                xcmd_le_cursor_end(le);
            } else if (memcmp(le->escape_buf, "\x1B[3~", 4) == 0) {
                xcmd_le_delete_char_forward(le);
            } else if (memcmp(le->escape_buf, "\x1B[1;5C", 6) == 0 ||
                       memcmp(le->escape_buf, "\x1B[1;5D", 6) == 0) {
                if (le->escape_buf[5] == 'C') {
                    xcmd_le_word_right(le);
                } else {
                    xcmd_le_word_left(le);
                }
            }
            
            le->escape_state = ESC_STATE_IDLE;
            le->escape_len = 0;
            return XCMD_LE_MORE;
        }
        if (le->escape_len >= XCMD_LE_ESCAPE_BUF_LEN - 1) {
            le->escape_state = ESC_STATE_IDLE;
            le->escape_len = 0;
        }
        return XCMD_LE_MORE;
        
    default:
        le->escape_state = ESC_STATE_IDLE;
        le->escape_len = 0;
        break;
    }
    
    if (byte == '\r' || byte == '\n') {
        term_putc(le, '\r');
        term_putc(le, '\n');
        if (le->on_submit && le->length > 0) {
            le->on_submit(le, le->buffer);
        }
        return XCMD_LE_COMPLETE;
    }
    
    if (byte == 0x7F || byte == 0x08) {
        xcmd_le_delete_char(le);
        return XCMD_LE_MORE;
    }
    
    if (byte == 0x01) {
        xcmd_le_cursor_home(le);
        return XCMD_LE_MORE;
    }
    
    if (byte == 0x05) {
        xcmd_le_cursor_end(le);
        return XCMD_LE_MORE;
    }
    
    if (byte == 0x0B) {
        xcmd_le_delete_to_end(le);
        return XCMD_LE_MORE;
    }
    
    if (byte == 0x15) {
        xcmd_le_delete_to_start(le);
        return XCMD_LE_MORE;
    }
    
    if (byte == 0x03) {
        le->length = 0;
        le->cursor = 0;
        le->buffer[0] = '\0';
        term_write(le, "^C\r\n");
        return XCMD_LE_CANCEL;
    }
    
    if (IS_PRINT(byte)) {
        xcmd_le_insert_char(le, (char)byte);
    }
    
    return XCMD_LE_MORE;
}

#if XCMD_LE_HISTORY_MAX_NUM > 0
void xcmd_le_history_append(xcmd_line_editor_t *le, const char *line) {
    if (!le || !line || line[0] == '\0') return;
    
    xcmd_le_history_t *h = &le->history;
    
    /* 连续重复命令去重 */
    if (h->count > 0 &&
        strncmp(h->pool[h->count - 1].line, line, XCMD_LE_LINE_MAX_LEN) == 0) {
        h->current = NULL;
        return;
    }
    
    if (h->count < XCMD_LE_HISTORY_MAX_NUM) {
        strncpy(h->pool[h->count].line, line, XCMD_LE_LINE_MAX_LEN);
        h->pool[h->count].line[XCMD_LE_LINE_MAX_LEN] = '\0';
        h->count++;
    } else {
        memmove(h->pool[0].line, h->pool[1].line, 
                (XCMD_LE_HISTORY_MAX_NUM - 1) * sizeof(xcmd_le_history_entry_t));
        strncpy(h->pool[XCMD_LE_HISTORY_MAX_NUM - 1].line, line, XCMD_LE_LINE_MAX_LEN);
        h->pool[XCMD_LE_HISTORY_MAX_NUM - 1].line[XCMD_LE_LINE_MAX_LEN] = '\0';
    }
    
    h->current = NULL;
}

const char* xcmd_le_history_prev(xcmd_line_editor_t *le) {
    if (!le || le->history.count == 0) return NULL;
    
    xcmd_le_history_t *h = &le->history;
    
    if (h->current == NULL) {
        h->current = &h->pool[h->count - 1];
    } else if (h->current > h->pool) {
        h->current--;
    }
    
    return h->current ? h->current->line : NULL;
}

const char* xcmd_le_history_next(xcmd_line_editor_t *le) {
    if (!le || le->history.count == 0) return NULL;
    
    xcmd_le_history_t *h = &le->history;
    
    if (h->current == NULL) {
        return NULL;
    } else if (h->current < &h->pool[h->count - 1]) {
        h->current++;
        return h->current->line;
    } else {
        h->current = NULL;
        return NULL;
    }
}

const char* xcmd_le_history_current(xcmd_line_editor_t *le) {
    if (!le || le->history.count == 0) return NULL;
    
    xcmd_le_history_t *h = &le->history;
    
    return h->current ? h->current->line : NULL;
}

void xcmd_le_history_reset(xcmd_line_editor_t *le) {
    if (le) {
        le->history.current = NULL;
    }
}

uint16_t xcmd_le_history_count(xcmd_line_editor_t *le) {
    return le ? le->history.count : 0;
}

int xcmd_le_key_up(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    const char *line = xcmd_le_history_prev(le);
    if (line) {
        xcmd_le_set_line(le, line);
    }
    return 0;
}

int xcmd_le_key_down(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    const char *line = xcmd_le_history_next(le);
    if (line) {
        xcmd_le_set_line(le, line);
    } else {
        xcmd_le_clear_line(le);
    }
    return 0;
}
#endif

int xcmd_le_key_enter(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    return 0;
}

int xcmd_le_key_backspace(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_delete_char(le);
    return 0;
}

int xcmd_le_key_delete(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_delete_char_forward(le);
    return 0;
}

int xcmd_le_key_left(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_cursor_left(le);
    return 0;
}

int xcmd_le_key_right(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_cursor_right(le);
    return 0;
}

int xcmd_le_key_home(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_cursor_home(le);
    return 0;
}

int xcmd_le_key_end(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_cursor_end(le);
    return 0;
}

int xcmd_le_key_ctrl_a(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_cursor_home(le);
    return 0;
}

int xcmd_le_key_ctrl_e(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_cursor_end(le);
    return 0;
}

int xcmd_le_key_ctrl_u(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_delete_to_start(le);
    return 0;
}

int xcmd_le_key_ctrl_k(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_delete_to_end(le);
    return 0;
}

int xcmd_le_key_ctrl_l(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    xcmd_le_clear_line(le);
    return 0;
}