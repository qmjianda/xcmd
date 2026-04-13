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

/*============================================================================
 * Key Handler Types and Tables (Table-Driven)
 *============================================================================*/

typedef void (*xcmd_le_void_handler_t)(xcmd_line_editor_t *le);

#define VOID_HANDLER(func) ((xcmd_le_void_handler_t)(func))

/* ESC sequence handlers - indexed by escape length and last byte for O(1) lookup */
typedef struct {
    const char             *seq;
    uint8_t               len;
    xcmd_le_void_handler_t handler;
} xcmd_escape_entry_t;

static const xcmd_escape_entry_t escape_table[] = {
#if XCMD_LE_HISTORY_MAX_NUM > 0
    { XCMD_KEY_UP,        3, VOID_HANDLER(xcmd_le_key_up) },
    { XCMD_KEY_DW,        3, VOID_HANDLER(xcmd_le_key_down) },
#endif
    { XCMD_KEY_RIGHT,     3, VOID_HANDLER(xcmd_le_cursor_right) },
    { XCMD_KEY_LEFT,      3, VOID_HANDLER(xcmd_le_cursor_left) },
    { XCMD_KEY_HOME,      3, VOID_HANDLER(xcmd_le_cursor_home) },
    { XCMD_KEY_HOME2,     4, VOID_HANDLER(xcmd_le_cursor_home) },
    { XCMD_KEY_END,       3, VOID_HANDLER(xcmd_le_cursor_end) },
    { XCMD_KEY_END2,      4, VOID_HANDLER(xcmd_le_cursor_end) },
    { XCMD_KEY_DELETE,    4, VOID_HANDLER(xcmd_le_delete_char_forward) },
    { XCMD_KEY_CTR_RIGHT, 6, VOID_HANDLER(xcmd_le_word_right) },
    { XCMD_KEY_CTR_LEFT,  6, VOID_HANDLER(xcmd_le_word_left) },
};
#define ESC_TABLE_SIZE (sizeof(escape_table) / sizeof(escape_table[0]))

/* Single-byte key handlers - indexed by byte value for O(1) lookup */
typedef struct {
    uint8_t               byte;
    xcmd_le_void_handler_t handler;
} xcmd_byte_entry_t;

static const xcmd_byte_entry_t byte_table[] = {
    { XCMD_KEY_BACKSPACE[0], VOID_HANDLER(xcmd_le_delete_char) },
    { XCMD_KEY_CTR_H[0],     VOID_HANDLER(xcmd_le_delete_char) },
    { XCMD_KEY_CTR_A[0],     VOID_HANDLER(xcmd_le_cursor_home) },
    { XCMD_KEY_CTR_E[0],     VOID_HANDLER(xcmd_le_cursor_end) },
    { XCMD_KEY_CTR_K[0],     VOID_HANDLER(xcmd_le_delete_to_end) },
    { XCMD_KEY_CTR_U[0],     VOID_HANDLER(xcmd_le_delete_to_start) },
    { XCMD_KEY_CTR_C[0],     VOID_HANDLER(xcmd_le_key_ctrl_c) },
};
#define BYTE_TABLE_SIZE (sizeof(byte_table) / sizeof(byte_table[0]))

static void le_putc(xcmd_line_editor_t *le, uint8_t c) {
    if (le->io) {
        xcmd_io_putc(le->io, c);
    }
}

static void le_write(xcmd_line_editor_t *le, const char *str) {
    if (le->io) {
        xcmd_io_puts(le->io, str);
    }
}

void xcmd_le_refresh(xcmd_line_editor_t *le) {
    if (!le || !le->term) return;
    
    if (le->term->ops && le->term->ops->clear_line) {
        le->term->ops->clear_line(le->term);
    } else {
        le_putc(le, '\r');
        for (int i = 0; i < 256; i++) {
            le_putc(le, ' ');
        }
        le_putc(le, '\r');
    }
    
    if (le->prompt) {
        le_write(le, le->prompt);
    }
    
    if (le->length > 0) {
        le_write(le, le->buffer);
    }
    
    if (le->cursor < le->length) {
        uint16_t diff = le->length - le->cursor;
        for (uint16_t i = 0; i < diff; i++) {
            le_putc(le, '\b');
        }
    }
}

int xcmd_le_init(xcmd_line_editor_t *le, xcmd_term_t *term) {
    if (!le || !term) return -1;
    
    memset(le, 0, sizeof(xcmd_line_editor_t));
    le->term = term;
    
    return 0;
}

void xcmd_le_set_complete_cb(xcmd_line_editor_t *le, xcmd_le_complete_cb cb) {
    if (le) {
        le->complete_cb = cb;
    }
}

static int le_get_word_at_cursor(xcmd_line_editor_t *le, uint16_t *word_start, uint16_t *word_end) {
    if (!le || !word_start || !word_end) return -1;
    
    uint16_t cursor = le->cursor;
    
    *word_end = cursor;
    while (*word_end < le->length && le->buffer[*word_end] != ' ') {
        (*word_end)++;
    }
    
    *word_start = cursor;
    if (*word_start > 0 && le->buffer[*word_start - 1] == ' ') {
        return -1;
    }
    
    while (*word_start > 0 && le->buffer[*word_start - 1] != ' ') {
        (*word_start)--;
    }
    
    if (*word_start == *word_end) {
        return -1;
    }
    
    return 0;
}

static void le_delete_word_at_cursor(xcmd_line_editor_t *le, uint16_t start, uint16_t end) {
    if (!le || start >= end) return;
    
    uint16_t len = le->length;
    uint16_t count = end - start;
    
    for (uint16_t i = end; i < len; i++) {
        le->buffer[start + (i - end)] = le->buffer[i];
    }
    le->buffer[start + (len - end)] = '\0';
    le->length = start + (len - end);
    
    if (le->cursor > end) {
        le->cursor -= count;
    } else if (le->cursor > start) {
        le->cursor = start;
    }
    
    for (uint16_t i = 0; i < count; i++) {
        le_putc(le, '\b');
        le_putc(le, ' ');
        le_putc(le, '\b');
    }
}

static void le_refresh_line(xcmd_line_editor_t *le) {
    if (!le) return;
    le_write(le, le->prompt);
    le_write(le, le->buffer);
    for (uint16_t i = le->cursor; i < le->length; i++) {
        le_putc(le, '\b');
    }
}

static void le_complete(xcmd_line_editor_t *le) {
    if (!le || !le->complete_cb) return;
    
    le->completion.count = 0;
    le->completion.current = 0;
    
    uint16_t word_start, word_end;
    int has_word = (le_get_word_at_cursor(le, &word_start, &word_end) == 0);
    uint16_t complete_from = has_word ? word_start : le->cursor;
    
    char word[64] = {0};
    if (has_word && word_end > word_start) {
        size_t len = word_end - word_start;
        if (len >= sizeof(word)) len = sizeof(word) - 1;
        memcpy(word, le->buffer + word_start, len);
    }
    
    // 计算 word_id（第几个单词）
    int word_id = 0;
    for (uint16_t i = 0; i < complete_from; i++) {
        if (le->buffer[i] != ' ' && (i == 0 || le->buffer[i-1] == ' ')) {
            word_id++;
        }
    }
    
    int n = le->complete_cb(le, word, word_id,
                            le->completion.candidates,
                            XCMD_LE_COMPLETION_MAX_NUM);
    if (n <= 0) return;
    
    le->completion.count = n;
    
    if (n == 1) {
        const char *match = le->completion.candidates[0];
        
        if (has_word) {
            le_delete_word_at_cursor(le, word_start, word_end);
        }
        
        for (int i = 0; match[i]; i++) {
            xcmd_le_insert_char(le, match[i]);
        }
    } else if (n > 1) {
        le_write(le, "\r\n");
        for (int i = 0; i < n; i++) {
            le_write(le, le->completion.candidates[i]);
            le_write(le, "  ");
        }
        le_write(le, "\r\n");
        le_refresh_line(le);
    }
}

void xcmd_le_insert_char(xcmd_line_editor_t *le, char c) {
    if (!le || le->length >= XCMD_LE_LINE_MAX_LEN) return;
    
    if (le->cursor == le->length) {
        le->buffer[le->cursor] = c;
        le->buffer[le->cursor + 1] = '\0';
        le->length++;
        le->cursor++;
        
        le_putc(le, c);
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
            le_putc(le, c);
        } else {
            xcmd_le_refresh(le);
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
        xcmd_le_refresh(le);
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
        xcmd_le_refresh(le);
    }
}

void xcmd_le_cursor_left(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;

    le->cursor--;
    if (le->term && le->term->ops && le->term->ops->cursor_left) {
        le->term->ops->cursor_left(le->term, 1);
    } else {
        xcmd_le_refresh(le);
    }
}

void xcmd_le_cursor_right(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;
    
    le->cursor++;
    if (le->term && le->term->ops && le->term->ops->cursor_right) {
        le->term->ops->cursor_right(le->term, 1);
    } else {
        xcmd_le_refresh(le);
    }
}

void xcmd_le_cursor_home(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;

    le->cursor = 0;
    if (le->term && le->term->ops && le->term->ops->cursor_left) {
        le->term->ops->cursor_left(le->term, le->length);
    } else {
        xcmd_le_refresh(le);
    }
}

void xcmd_le_cursor_end(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;
    uint16_t n = le->length - le->cursor;
    le->cursor = le->length;
    if (le->term && le->term->ops && le->term->ops->cursor_right) {
        le->term->ops->cursor_right(le->term, n);
    } else {
        xcmd_le_refresh(le);
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
        le->term->ops->cursor_set(le->term, le->cursor + le->prompt_len);
    } else {
        xcmd_le_refresh(le);
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
        le->term->ops->cursor_set(le->term, le->cursor + le->prompt_len);
    } else {
        xcmd_le_refresh(le);
    }
}

void xcmd_le_delete_to_start(xcmd_line_editor_t *le) {
    if (!le || le->cursor == 0) return;
    
    uint16_t n = le->cursor;
    memmove(le->buffer, &le->buffer[le->cursor], le->length - le->cursor + 1);
    le->length -= n;
    le->cursor = 0;
    
    xcmd_le_refresh(le);
}

void xcmd_le_delete_to_end(xcmd_line_editor_t *le) {
    if (!le || le->cursor >= le->length) return;
    
    le->length = le->cursor;
    le->buffer[le->length] = '\0';
    
    if (le->term && le->term->ops && le->term->ops->clear_to_eol) {
        le->term->ops->clear_to_eol(le->term);
    }
    else {
        xcmd_le_refresh(le);
    }
}

void xcmd_le_clear_line(xcmd_line_editor_t *le) {
    if (!le) return;
    
    le->length = 0;
    le->cursor = 0;
    le->buffer[0] = '\0';
    xcmd_le_refresh(le);
}

/**
 * @brief Reset line state and output prompt
 * @param le Line editor
 * 
 * Clears: buffer, cursor, length
 * Then outputs the prompt
 */
void xcmd_le_reset_line(xcmd_line_editor_t *le) {
    if (!le) return;
    
    le->history.current = NULL;
    xcmd_le_clear_line(le);
    
}

void xcmd_le_set_line(xcmd_line_editor_t *le, const char *str) {
    if (!le || !str) return;
    
    strncpy(le->buffer, str, XCMD_LE_LINE_MAX_LEN);
    le->buffer[XCMD_LE_LINE_MAX_LEN] = '\0';
    le->length = strlen(le->buffer);
    le->cursor = le->length;
    
    xcmd_le_refresh(le);
}

const char* xcmd_le_get_submitted_line(xcmd_line_editor_t *le) {
    if (!le || le->length == 0) return NULL;
    return le->buffer;
}

static bool is_escape_end(uint8_t c) {
    return ((c >= 'A' && c <= 'D') || 
            (c >= 'F' && c <= 'H') ||
            (c >= 'P' && c <= 'S') || 
            (c == '~') ||
            (c >= 'a' && c <= 'z'));
}

const char* xcmd_le_readline(xcmd_line_editor_t *le, xcmd_io_t *in, xcmd_io_t *out, const char *prompt) {
    if (!le) return NULL;
    
    le->io = out;
    le->prompt = prompt;
    le->prompt_len = prompt ? strlen(prompt) : 0;
    xcmd_term_set_io(le->term, out);
    
    uint8_t byte;
    if (xcmd_io_read(in, &byte, 1) <= 0) {
        return NULL;
    }
    
    switch (le->escape_state) {
    case ESC_STATE_IDLE:
        if (byte == XCMD_KEY_ESC[0]) {
            le->escape_buf[0] = byte;
            le->escape_len = 1;
            le->escape_state = ESC_STATE_ESC;
            return NULL;
        }
        break;
        
    case ESC_STATE_ESC:
        le->escape_buf[le->escape_len++] = byte;
        if (byte == '[') {
            le->escape_state = ESC_STATE_BRACKET;
            return NULL;
        }
        le->escape_len = 0;
        le->escape_state = ESC_STATE_IDLE;
        return NULL;
        
    case ESC_STATE_BRACKET:
        le->escape_buf[le->escape_len++] = byte;
        if (is_escape_end(byte)) {
            le->escape_buf[le->escape_len] = '\0';
            le->escape_state = ESC_STATE_END;
            
            for (int i = 0; i < ESC_TABLE_SIZE; i++) {
                if (memcmp(le->escape_buf, escape_table[i].seq, escape_table[i].len) == 0 &&
                    le->escape_buf[escape_table[i].len] == '\0') {
                    escape_table[i].handler(le);
                    break;
                }
            }
            
            le->escape_state = ESC_STATE_IDLE;
            le->escape_len = 0;
            return NULL;
        }
        if (le->escape_len >= XCMD_LE_ESCAPE_BUF_LEN - 1) {
            le->escape_state = ESC_STATE_IDLE;
            le->escape_len = 0;
        }
        return NULL;
        
    default:
        le->escape_state = ESC_STATE_IDLE;
        le->escape_len = 0;
        break;
    }
    
    if (byte == '\r' || byte == '\n') {
        le_putc(le, '\r');
        le_putc(le, '\n');
        if (le->length > 0) {
#if XCMD_LE_HISTORY_MAX_NUM > 0
            xcmd_le_history_append(le, le->buffer);
#endif
            const char *line = le->buffer;
            return line;
        }
        xcmd_le_refresh(le);
        return NULL;
    }
    
    for (int i = 0; i < BYTE_TABLE_SIZE; i++) {
        if (byte == byte_table[i].byte) {
            byte_table[i].handler(le);
            return NULL;
        }
    }
    
    if (byte == '\t') {
        le_complete(le);
        return NULL;
    }
    
    if (IS_PRINT(byte)) {
        xcmd_le_insert_char(le, (char)byte);
    }
    
    return NULL;
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

int xcmd_le_key_ctrl_c(xcmd_line_editor_t *le, const char *key) {
    (void)key;
    le->length = 0;
    le->cursor = 0;
    le->buffer[0] = '\0';
    le_write(le, "^C\r\n");
    xcmd_le_refresh(le);
    return 0;
}

const char* xcmd_le_get_word(xcmd_line_editor_t *le, int word_id) {
    static char result[64] = {0};
    result[0] = '\0';
    
    if (!le || word_id < 0) return result;
    
    int current_id = 0;
    uint16_t i = 0;
    
    while (i < le->length && current_id < word_id) {
        while (i < le->length && le->buffer[i] == ' ') i++;
        if (i >= le->length) break;
        current_id++;
        if (current_id == word_id) break;
        while (i < le->length && le->buffer[i] != ' ') i++;
    }
    
    if (current_id != word_id) return result;
    
    uint16_t start = i;
    while (i < le->length && le->buffer[i] != ' ') i++;
    uint16_t end = i;
    
    size_t len = end - start;
    if (len >= sizeof(result)) len = sizeof(result) - 1;
    memcpy(result, le->buffer + start, len);
    result[len] = '\0';
    
    return result;
}