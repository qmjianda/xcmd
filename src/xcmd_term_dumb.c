#include "xcmd_terminal.h"

static int (*s_dumb_putc)(uint8_t) = NULL;

static void dumb_put_char(uint8_t c)
{
    if (s_dumb_putc) {
        s_dumb_putc(c);
    }
}

static void dumb_put_str(const char* str)
{
    while (*str) {
        dumb_put_char((uint8_t)*str++);
    }
}

static void dumb_cursor_move(uint16_t row, uint16_t col)
{
    (void)row;
    (void)col;
}

static void dumb_cursor_save(void)
{
}

static void dumb_cursor_restore(void)
{
}

static void dumb_cursor_show(void)
{
}

static void dumb_cursor_hide(void)
{
}

static void dumb_clear_screen(void)
{
    dumb_put_str("\r\n");
}

static void dumb_clear_line(void)
{
    dumb_put_str("\r\n");
}

static void dumb_clear_eol(void)
{
}

static void dumb_clear_sol(void)
{
}

static void dumb_scroll_up(uint16_t n)
{
    (void)n;
}

static void dumb_scroll_down(uint16_t n)
{
    (void)n;
}

static void dumb_set_scroll_region(uint16_t top, uint16_t bottom)
{
    (void)top;
    (void)bottom;
}

static void dumb_reset_scroll_region(void)
{
}

static void dumb_set_style(const xcmd_style_t* style)
{
    (void)style;
}

static void dumb_reset_style(void)
{
}

static void dumb_alt_screen_enter(void)
{
}

static void dumb_alt_screen_exit(void)
{
}

static void dumb_set_title(const char* title)
{
    (void)title;
}

static void dumb_mouse_enable(void)
{
}

static void dumb_mouse_disable(void)
{
}

const xcmd_term_driver_t dumb_driver = {
    .put_char           = dumb_put_char,
    .put_str            = dumb_put_str,
    .flush              = NULL,
    .cursor_move        = dumb_cursor_move,
    .cursor_save        = dumb_cursor_save,
    .cursor_restore     = dumb_cursor_restore,
    .cursor_show        = dumb_cursor_show,
    .cursor_hide        = dumb_cursor_hide,
    .clear_screen       = dumb_clear_screen,
    .clear_line         = dumb_clear_line,
    .clear_eol          = dumb_clear_eol,
    .clear_sol          = dumb_clear_sol,
    .scroll_up          = dumb_scroll_up,
    .scroll_down        = dumb_scroll_down,
    .set_scroll_region  = dumb_set_scroll_region,
    .reset_scroll_region = dumb_reset_scroll_region,
    .set_style          = dumb_set_style,
    .reset_style        = dumb_reset_style,
    .alt_screen_enter   = dumb_alt_screen_enter,
    .alt_screen_exit    = dumb_alt_screen_exit,
    .set_title          = dumb_set_title,
    .mouse_enable       = dumb_mouse_enable,
    .mouse_disable      = dumb_mouse_disable,
};

void xcmd_terminal_init_dumb(int (*put_c)(uint8_t))
{
    s_dumb_putc = put_c;
    
    g_terminal.type = XCMD_TERM_DUMB;
    g_terminal.caps = 0;
    g_terminal.driver = &dumb_driver;
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