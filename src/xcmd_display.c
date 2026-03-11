#include "xcmd_display.h"
#include <string.h>

static xcmd_line_editor_t g_editor = {0};
static const char* g_prompt = "-> ";

void xcmd_display_init(void)
{
    memset(&g_editor, 0, sizeof(g_editor));
    g_editor.prompt_len = 3;
}

void xcmd_display_cursor_left(uint16_t n)
{
    if (g_editor.cursor > 0) {
        uint16_t move = (g_editor.cursor < n) ? g_editor.cursor : n;
        g_editor.cursor -= move;
        xcmd_term_cursor_back(move);
    }
}

void xcmd_display_cursor_right(uint16_t n)
{
    if (g_editor.cursor < g_editor.len) {
        uint16_t move = (g_editor.len - g_editor.cursor < n) ? (g_editor.len - g_editor.cursor) : n;
        g_editor.cursor += move;
        xcmd_term_cursor_forward(move);
    }
}

void xcmd_display_cursor_home(void)
{
    xcmd_display_cursor_left(g_editor.cursor);
}

void xcmd_display_cursor_end(void)
{
    xcmd_display_cursor_right(g_editor.len - g_editor.cursor);
}

void xcmd_display_cursor_set(uint16_t pos)
{
    if (pos > g_editor.len) {
        pos = g_editor.len;
    }
    if (pos < g_editor.cursor) {
        xcmd_display_cursor_left(g_editor.cursor - pos);
    } else {
        xcmd_display_cursor_right(pos - g_editor.cursor);
    }
}

uint16_t xcmd_display_cursor_get(void)
{
    return g_editor.cursor;
}

void xcmd_display_clear_line(void)
{
    xcmd_term_cursor_to_col(0);
    xcmd_term_clear_eol();
    g_editor.len = 0;
    g_editor.cursor = 0;
    g_editor.buffer[0] = '\0';
}

void xcmd_display_kill_line(void)
{
    if (g_editor.cursor < g_editor.len) {
        uint16_t to_delete = g_editor.len - g_editor.cursor;
        for (uint16_t i = 0; i < to_delete; i++) {
            xcmd_term_puts("\x1B[P");
        }
        g_editor.len = g_editor.cursor;
        g_editor.buffer[g_editor.len] = '\0';
    }
}

void xcmd_display_kill_word(void)
{
    uint16_t word_start = g_editor.cursor;
    while (word_start > 0 && g_editor.buffer[word_start - 1] == ' ') {
        word_start--;
    }
    while (word_start > 0 && g_editor.buffer[word_start - 1] != ' ') {
        word_start--;
    }
    
    uint16_t to_delete = g_editor.cursor - word_start;
    if (to_delete > 0) {
        xcmd_display_cursor_left(to_delete);
        for (uint16_t i = g_editor.cursor; i < g_editor.len; i++) {
            g_editor.buffer[i - to_delete] = g_editor.buffer[i];
        }
        g_editor.len -= to_delete;
        g_editor.buffer[g_editor.len] = '\0';
        
        for (uint16_t i = 0; i < to_delete; i++) {
            xcmd_term_puts("\x1B[P");
        }
    }
}

void xcmd_display_clear_input(void)
{
    xcmd_display_cursor_home();
    xcmd_display_kill_line();
}

void xcmd_display_refresh(void)
{
    xcmd_term_cursor_to_col(0);
    xcmd_term_clear_eol();
    xcmd_display_draw_prompt();
    xcmd_put_str(g_editor.buffer);
    
    if (g_editor.cursor < g_editor.len) {
        uint16_t to_move = g_editor.len - g_editor.cursor;
        xcmd_term_cursor_back(to_move);
    }
}

void xcmd_display_redraw(void)
{
    xcmd_display_draw_prompt();
    xcmd_put_str(g_editor.buffer);
}

char* xcmd_display_get_buffer(void)
{
    return g_editor.buffer;
}

uint16_t xcmd_display_get_len(void)
{
    return g_editor.len;
}

void xcmd_display_set_buffer(const char* buf, uint16_t len)
{
    if (len < XCMD_LINE_MAX_LENGTH) {
        memcpy(g_editor.buffer, buf, len);
        g_editor.buffer[len] = '\0';
        g_editor.len = len;
        g_editor.cursor = len;
    }
}

void xcmd_display_set_prompt(const char* prompt)
{
    g_prompt = prompt;
    g_editor.prompt_len = (uint16_t)strlen(prompt);
}

const char* xcmd_display_get_prompt(void)
{
    return g_prompt;
}

void xcmd_display_draw_prompt(void)
{
    xcmd_term_style_prompt();
    xcmd_put_str(g_prompt);
    xcmd_term_reset_style();
}

#if XCMD_FEATURE_PROGRESS
void xcmd_display_progress_bar(uint8_t percent, uint16_t width)
{
    if (percent > 100) percent = 100;
    
    uint16_t filled = (uint16_t)((uint32_t)percent * width / 100);
    uint16_t empty = width - filled;
    
    xcmd_term_putc('[');
    for (uint16_t i = 0; i < filled; i++) {
        xcmd_term_putc('=');
    }
    for (uint16_t i = 0; i < empty; i++) {
        xcmd_term_putc('-');
    }
    xcmd_term_puts("] ");
    xcmd_print("%d%%\r", percent);
    
    if (percent == 100) {
        xcmd_term_puts("\n");
    }
}
#endif