#ifndef XCMD_DISPLAY_H
#define XCMD_DISPLAY_H

#include "xcmd_terminal.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XCMD_FEATURE_PROGRESS
#define XCMD_FEATURE_PROGRESS 1
#endif

typedef struct {
    char        buffer[XCMD_LINE_MAX_LENGTH];
    uint16_t    len;
    uint16_t    cursor;
    uint16_t    prompt_len;
} xcmd_line_editor_t;

void xcmd_display_init(void);

void xcmd_display_insert_char(char c);
void xcmd_display_delete_char(void);
void xcmd_display_backspace(void);

void xcmd_display_cursor_left(uint16_t n);
void xcmd_display_cursor_right(uint16_t n);
void xcmd_display_cursor_home(void);
void xcmd_display_cursor_end(void);
void xcmd_display_cursor_set(uint16_t pos);
uint16_t xcmd_display_cursor_get(void);

void xcmd_display_clear_line(void);
void xcmd_display_kill_line(void);
void xcmd_display_kill_word(void);
void xcmd_display_clear_input(void);

void xcmd_display_refresh(void);
void xcmd_display_redraw(void);

char* xcmd_display_get_buffer(void);
uint16_t xcmd_display_get_len(void);
void xcmd_display_set_buffer(const char* buf, uint16_t len);

void xcmd_display_set_prompt(const char* prompt);
const char* xcmd_display_get_prompt(void);
void xcmd_display_draw_prompt(void);

#if XCMD_FEATURE_PROGRESS
void xcmd_display_progress_bar(uint8_t percent, uint16_t width);
#endif

#define xcmd_print(...) xcmd_term_printf(__VA_ARGS__)
#define xcmd_put_str(s) xcmd_term_puts(s)

#ifdef __cplusplus
}
#endif

#endif