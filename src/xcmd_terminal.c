#include "xcmd_terminal.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

xcmd_terminal_t g_terminal = {0};
static int (*s_putc)(uint8_t) = NULL;

static void ansi_put_char(uint8_t c)
{
    if (s_putc) {
        s_putc(c);
    }
}

static void ansi_put_str(const char* str)
{
    while (*str) {
        ansi_put_char((uint8_t)*str++);
    }
}

static void ansi_cursor_move(uint16_t row, uint16_t col)
{
    char buf[32];
    sprintf(buf, "\x1B[%d;%dH", row + 1, col + 1);
    ansi_put_str(buf);
}

static void ansi_cursor_save(void)
{
    ansi_put_str("\x1B""7");
}

static void ansi_cursor_restore(void)
{
    ansi_put_str("\x1B""8");
}

static void ansi_cursor_show(void)
{
    ansi_put_str("\x1B[?25h");
}

static void ansi_cursor_hide(void)
{
    ansi_put_str("\x1B[?25l");
}

static void ansi_clear_screen(void)
{
    ansi_put_str("\x1B[2J\x1B[H");
}

static void ansi_clear_line(void)
{
    ansi_put_str("\x1B[2K");
}

static void ansi_clear_eol(void)
{
    ansi_put_str("\x1B[K");
}

static void ansi_clear_sol(void)
{
    ansi_put_str("\x1B[1K");
}

static void ansi_scroll_up(uint16_t n)
{
    (void)n;
}

static void ansi_scroll_down(uint16_t n)
{
    (void)n;
}

static void ansi_set_scroll_region(uint16_t top, uint16_t bottom)
{
    char buf[32];
    sprintf(buf, "\x1B[%d;%dr", top + 1, bottom + 1);
    ansi_put_str(buf);
}

static void ansi_reset_scroll_region(void)
{
    ansi_put_str("\x1B[r");
}

static void ansi_set_style(const xcmd_style_t* style)
{
    char buf[64];
    int pos = 0;
    
    buf[pos++] = '\x1B';
    buf[pos++] = '[';
    buf[pos++] = '0';
    
    if (style->attr) {
        if (style->attr & XCMD_ATTR_BOLD) {
            buf[pos++] = ';';
            buf[pos++] = '1';
        }
        if (style->attr & XCMD_ATTR_DIM) {
            buf[pos++] = ';';
            buf[pos++] = '2';
        }
        if (style->attr & XCMD_ATTR_ITALIC) {
            buf[pos++] = ';';
            buf[pos++] = '3';
        }
        if (style->attr & XCMD_ATTR_UNDERLINE) {
            buf[pos++] = ';';
            buf[pos++] = '4';
        }
        if (style->attr & XCMD_ATTR_BLINK) {
            buf[pos++] = ';';
            buf[pos++] = '5';
        }
        if (style->attr & XCMD_ATTR_REVERSE) {
            buf[pos++] = ';';
            buf[pos++] = '7';
        }
    }
    
    switch (style->mode) {
    case XCMD_COLOR_MODE_16:
        pos += sprintf(buf + pos, ";%d", 30 + style->fg.color8);
        pos += sprintf(buf + pos, ";%d", 40 + style->bg.color8);
        break;
    case XCMD_COLOR_MODE_256:
        pos += sprintf(buf + pos, ";38;5;%d", style->fg.color8);
        pos += sprintf(buf + pos, ";48;5;%d", style->bg.color8);
        break;
    case XCMD_COLOR_MODE_RGB:
        pos += sprintf(buf + pos, ";38;2;%d;%d;%d", 
            style->fg.rgb.r, style->fg.rgb.g, style->fg.rgb.b);
        pos += sprintf(buf + pos, ";48;2;%d;%d;%d", 
            style->bg.rgb.r, style->bg.rgb.g, style->bg.rgb.b);
        break;
    case XCMD_COLOR_MODE_NONE:
    default:
        break;
    }
    
    buf[pos++] = 'm';
    buf[pos] = '\0';
    ansi_put_str(buf);
}

static void ansi_reset_style(void)
{
    ansi_put_str("\x1B[0m");
}

static void ansi_alt_screen_enter(void)
{
    ansi_put_str("\x1B[?1049h");
}

static void ansi_alt_screen_exit(void)
{
    ansi_put_str("\x1B[?1049l");
}

static void ansi_set_title(const char* title)
{
    char buf[128];
    sprintf(buf, "\x1B]0;%s\x1B\\", title);
    ansi_put_str(buf);
}

static void ansi_mouse_enable(void)
{
    ansi_put_str("\x1B[?1000h");
}

static void ansi_mouse_disable(void)
{
    ansi_put_str("\x1B[?1000l");
}

static const xcmd_term_driver_t ansi_driver = {
    .put_char           = ansi_put_char,
    .put_str            = ansi_put_str,
    .flush              = NULL,
    .cursor_move        = ansi_cursor_move,
    .cursor_save        = ansi_cursor_save,
    .cursor_restore     = ansi_cursor_restore,
    .cursor_show        = ansi_cursor_show,
    .cursor_hide        = ansi_cursor_hide,
    .clear_screen       = ansi_clear_screen,
    .clear_line         = ansi_clear_line,
    .clear_eol          = ansi_clear_eol,
    .clear_sol          = ansi_clear_sol,
    .scroll_up          = ansi_scroll_up,
    .scroll_down        = ansi_scroll_down,
    .set_scroll_region  = ansi_set_scroll_region,
    .reset_scroll_region = ansi_reset_scroll_region,
    .set_style          = ansi_set_style,
    .reset_style        = ansi_reset_style,
    .alt_screen_enter   = ansi_alt_screen_enter,
    .alt_screen_exit    = ansi_alt_screen_exit,
    .set_title          = ansi_set_title,
    .mouse_enable       = ansi_mouse_enable,
    .mouse_disable      = ansi_mouse_disable,
};

xcmd_terminal_type_t xcmd_terminal_detect(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return XCMD_TERM_WINDOWS;
#elif defined(__linux__) || defined(__APPLE__)
    const char* term = getenv("TERM");
    if (term == NULL) {
        return XCMD_TERM_DUMB;
    }
    if (strstr(term, "xterm-256color") != NULL) {
        return XCMD_TERM_XTERM;
    }
    if (strstr(term, "xterm") != NULL) {
        return XCMD_TERM_XTERM;
    }
    if (strstr(term, "vt100") != NULL) {
        return XCMD_TERM_VT100;
    }
    if (strstr(term, "vt220") != NULL) {
        return XCMD_TERM_VT220;
    }
    if (strstr(term, "ansi") != NULL) {
        return XCMD_TERM_ANSI;
    }
    if (strstr(term, "screen") != NULL) {
        return XCMD_TERM_SCREEN;
    }
    if (strstr(term, "tmux") != NULL) {
        return XCMD_TERM_TMUX;
    }
    return XCMD_TERM_ANSI;
#else
    return XCMD_TERM_DUMB;
#endif
}

static xcmd_term_cap_t detect_caps(xcmd_terminal_type_t type)
{
    xcmd_term_cap_t caps = 0;
    
    switch (type) {
    case XCMD_TERM_DUMB:
        caps = 0;
        break;
    case XCMD_TERM_VT100:
        caps = XCMD_CAP_BASIC | XCMD_CAP_UTF8;
        break;
    case XCMD_TERM_VT220:
        caps = XCMD_CAP_BASIC | XCMD_CAP_CURSOR_SAVE | XCMD_CAP_UTF8;
        break;
    case XCMD_TERM_ANSI:
        caps = XCMD_CAP_BASIC | XCMD_CAP_CURSOR_SAVE | XCMD_CAP_UTF8;
        break;
    case XCMD_TERM_XTERM:
    case XCMD_TERM_TMUX:
    case XCMD_TERM_SCREEN:
        caps = XCMD_CAP_ENHANCED | XCMD_CAP_ALT_SCREEN | XCMD_CAP_TITLE;
        break;
    case XCMD_TERM_CONEMU:
    case XCMD_TERM_WINDOWS:
        caps = XCMD_CAP_ENHANCED;
        break;
    default:
        caps = XCMD_CAP_BASIC;
        break;
    }
    
#if defined(__linux__) || defined(__APPLE__)
    const char* term = getenv("TERM");
    if (term && strstr(term, "256color") != NULL) {
        caps |= XCMD_CAP_256COLOR;
    }
#endif
    return caps;
}

void xcmd_terminal_init(int (*put_c)(uint8_t), xcmd_terminal_type_t type)
{
    s_putc = put_c;
    
    if (type == XCMD_TERM_AUTO) {
        type = xcmd_terminal_detect();
    }
    
    g_terminal.type = type;
    g_terminal.caps = detect_caps(type);
    g_terminal.driver = &ansi_driver;
    g_terminal.size.width = 80;
    g_terminal.size.height = 24;
    g_terminal.size.scroll_top = 0;
    g_terminal.size.scroll_bottom = 23;
    g_terminal.initialized = 1;
    
    g_terminal.current_style.mode = XCMD_COLOR_MODE_NONE;
    g_terminal.current_style.attr = 0;
    g_terminal.current_style.fg.color8 = XCMD_COLOR_WHITE;
    g_terminal.current_style.bg.color8 = XCMD_COLOR_BLACK;
}

xcmd_term_cap_t xcmd_terminal_get_caps(void)
{
    return g_terminal.caps;
}

xcmd_terminal_type_t xcmd_terminal_get_type(void)
{
    return g_terminal.type;
}

xcmd_terminal_size_t* xcmd_terminal_get_size(void)
{
    return &g_terminal.size;
}

void xcmd_terminal_set_size(uint16_t width, uint16_t height)
{
    g_terminal.size.width = width;
    g_terminal.size.height = height;
    g_terminal.size.scroll_bottom = height - 1;
}

void xcmd_term_cursor_to(uint16_t row, uint16_t col)
{
    if (g_terminal.driver && g_terminal.driver->cursor_move) {
        g_terminal.driver->cursor_move(row, col);
    }
}

void xcmd_term_cursor_to_col(uint16_t col)
{
    xcmd_term_cursor_to(g_terminal.size.scroll_bottom, col);
}

void xcmd_term_cursor_up(uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        ansi_put_str("\x1B[A");
    }
}

void xcmd_term_cursor_down(uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        ansi_put_str("\x1B[B");
    }
}

void xcmd_term_cursor_forward(uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        ansi_put_str("\x1B[C");
    }
}

void xcmd_term_cursor_back(uint16_t n)
{
    for (uint16_t i = 0; i < n; i++) {
        ansi_put_str("\x1B[D");
    }
}

void xcmd_term_cursor_save(void)
{
    if (g_terminal.driver && g_terminal.driver->cursor_save) {
        g_terminal.driver->cursor_save();
    }
}

void xcmd_term_cursor_restore(void)
{
    if (g_terminal.driver && g_terminal.driver->cursor_restore) {
        g_terminal.driver->cursor_restore();
    }
}

void xcmd_term_cursor_show(void)
{
    if (g_terminal.driver && g_terminal.driver->cursor_show) {
        g_terminal.driver->cursor_show();
    }
}

void xcmd_term_cursor_hide(void)
{
    if (g_terminal.driver && g_terminal.driver->cursor_hide) {
        g_terminal.driver->cursor_hide();
    }
}

void xcmd_term_clear_screen(void)
{
    if (g_terminal.driver && g_terminal.driver->clear_screen) {
        g_terminal.driver->clear_screen();
    }
}

void xcmd_term_clear_line(void)
{
    if (g_terminal.driver && g_terminal.driver->clear_line) {
        g_terminal.driver->clear_line();
    }
}

void xcmd_term_clear_eol(void)
{
    if (g_terminal.driver && g_terminal.driver->clear_eol) {
        g_terminal.driver->clear_eol();
    }
}

void xcmd_term_clear_sol(void)
{
    if (g_terminal.driver && g_terminal.driver->clear_sol) {
        g_terminal.driver->clear_sol();
    }
}

void xcmd_term_scroll_up(uint16_t lines)
{
    if (g_terminal.driver && g_terminal.driver->scroll_up) {
        g_terminal.driver->scroll_up(lines);
    }
}

void xcmd_term_scroll_down(uint16_t lines)
{
    if (g_terminal.driver && g_terminal.driver->scroll_down) {
        g_terminal.driver->scroll_down(lines);
    }
}

void xcmd_term_set_scroll_region(uint16_t top, uint16_t bottom)
{
    if (g_terminal.driver && g_terminal.driver->set_scroll_region) {
        g_terminal.driver->set_scroll_region(top, bottom);
    }
}

void xcmd_term_reset_scroll_region(void)
{
    if (g_terminal.driver && g_terminal.driver->reset_scroll_region) {
        g_terminal.driver->reset_scroll_region();
    }
}

void xcmd_term_set_style(const xcmd_style_t* style)
{
    if (g_terminal.driver && g_terminal.driver->set_style) {
        g_terminal.driver->set_style(style);
    }
    memcpy(&g_terminal.current_style, style, sizeof(xcmd_style_t));
}

void xcmd_term_reset_style(void)
{
    if (g_terminal.driver && g_terminal.driver->reset_style) {
        g_terminal.driver->reset_style();
    }
    g_terminal.current_style.mode = XCMD_COLOR_MODE_NONE;
    g_terminal.current_style.attr = 0;
}

void xcmd_term_set_color(xcmd_color_t fg, xcmd_color_t bg)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_16;
    style.attr = 0;
    style.fg.color8 = fg;
    style.bg.color8 = bg;
    xcmd_term_set_style(&style);
}

void xcmd_term_set_fg(xcmd_color_t color)
{
    g_terminal.current_style.fg.color8 = color;
    g_terminal.current_style.mode = XCMD_COLOR_MODE_16;
    xcmd_term_set_style(&g_terminal.current_style);
}

void xcmd_term_set_bg(xcmd_color_t color)
{
    g_terminal.current_style.bg.color8 = color;
    g_terminal.current_style.mode = XCMD_COLOR_MODE_16;
    xcmd_term_set_style(&g_terminal.current_style);
}

void xcmd_term_set_attr(xcmd_attr_t attr)
{
    g_terminal.current_style.attr = attr;
    xcmd_term_set_style(&g_terminal.current_style);
}

void xcmd_term_set_color_256(uint8_t fg, uint8_t bg)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_256;
    style.attr = 0;
    style.fg.color8 = fg;
    style.bg.color8 = bg;
    xcmd_term_set_style(&style);
}

void xcmd_term_set_fg_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    xcmd_style_t style = g_terminal.current_style;
    style.mode = XCMD_COLOR_MODE_RGB;
    style.fg.rgb.r = r;
    style.fg.rgb.g = g;
    style.fg.rgb.b = b;
    xcmd_term_set_style(&style);
}

void xcmd_term_set_bg_rgb(uint8_t r, uint8_t g, uint8_t b)
{
    xcmd_style_t style = g_terminal.current_style;
    style.mode = XCMD_COLOR_MODE_RGB;
    style.bg.rgb.r = r;
    style.bg.rgb.g = g;
    style.bg.rgb.b = b;
    xcmd_term_set_style(&style);
}

void xcmd_term_style_default(void)
{
    xcmd_term_set_color(XCMD_COLOR_WHITE, XCMD_COLOR_BLACK);
}

void xcmd_term_style_prompt(void)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_16;
    style.attr = XCMD_ATTR_BOLD;
    style.fg.color8 = XCMD_COLOR_GREEN;
    style.bg.color8 = XCMD_COLOR_BLACK;
    xcmd_term_set_style(&style);
}

void xcmd_term_style_error(void)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_16;
    style.attr = 0;
    style.fg.color8 = XCMD_COLOR_RED;
    style.bg.color8 = XCMD_COLOR_BLACK;
    xcmd_term_set_style(&style);
}

void xcmd_term_style_warning(void)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_16;
    style.attr = 0;
    style.fg.color8 = XCMD_COLOR_YELLOW;
    style.bg.color8 = XCMD_COLOR_BLACK;
    xcmd_term_set_style(&style);
}

void xcmd_term_style_success(void)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_16;
    style.attr = 0;
    style.fg.color8 = XCMD_COLOR_GREEN;
    style.bg.color8 = XCMD_COLOR_BLACK;
    xcmd_term_set_style(&style);
}

void xcmd_term_style_info(void)
{
    xcmd_style_t style;
    style.mode = XCMD_COLOR_MODE_16;
    style.attr = 0;
    style.fg.color8 = XCMD_COLOR_CYAN;
    style.bg.color8 = XCMD_COLOR_BLACK;
    xcmd_term_set_style(&style);
}

void xcmd_term_alt_screen_enter(void)
{
    if (g_terminal.driver && g_terminal.driver->alt_screen_enter) {
        g_terminal.driver->alt_screen_enter();
    }
}

void xcmd_term_alt_screen_exit(void)
{
    if (g_terminal.driver && g_terminal.driver->alt_screen_exit) {
        g_terminal.driver->alt_screen_exit();
    }
}

void xcmd_term_set_title(const char* title)
{
    if (g_terminal.driver && g_terminal.driver->set_title) {
        g_terminal.driver->set_title(title);
    }
}

void xcmd_term_bracketed_paste_on(void)
{
    ansi_put_str("\x1B[200~");
}

void xcmd_term_bracketed_paste_off(void)
{
    ansi_put_str("\x1B[201~");
}