/**
 * @file xcmd_terminal.h
 * @brief Terminal abstraction layer for xcmd
 * @description Provides terminal capability detection and driver abstraction
 */

#ifndef XCMD_TERMINAL_H
#define XCMD_TERMINAL_H

#include "xcmd_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Terminal Type Definitions
 * ============================================================ */
typedef enum {
    XCMD_TERM_DUMB = 0,      /* Plain text, no escape sequences */
    XCMD_TERM_VT100,         /* DEC VT100 */
    XCMD_TERM_VT220,         /* DEC VT220 */
    XCMD_TERM_ANSI,          /* ANSI X3.64 */
    XCMD_TERM_XTERM,         /* xterm compatible */
    XCMD_TERM_CONEMU,        /* Windows ConEmu */
    XCMD_TERM_WINDOWS,       /* Windows Console */
    XCMD_TERM_TMUX,          /* tmux */
    XCMD_TERM_SCREEN,        /* GNU screen */
    XCMD_TERM_AUTO           /* Auto-detect */
} xcmd_terminal_type_t;

/* ============================================================
 * Terminal Capability Flags
 * ============================================================ */
typedef uint32_t xcmd_term_cap_t;

#define XCMD_CAP_COLOR        (1 << 0)   /* Basic color support */
#define XCMD_CAP_256COLOR     (1 << 1)   /* 256 color support */
#define XCMD_CAP_TRUECOLOR    (1 << 2)   /* 24-bit true color */
#define XCMD_CAP_CURSOR       (1 << 3)   /* Cursor movement */
#define XCMD_CAP_CURSOR_SAVE  (1 << 4)   /* Cursor save/restore */
#define XCMD_CAP_CLEAR        (1 << 5)   /* Clear screen */
#define XCMD_CAP_CLEAR_LINE   (1 << 6)   /* Clear line */
#define XCMD_CAP_SCROLL       (1 << 7)   /* Scrolling */
#define XCMD_CAP_SCROLL_REG   (1 << 8)   /* Scroll region */
#define XCMD_CAP_BRACKETED    (1 << 9)   /* Bracketed paste mode */
#define XCMD_CAP_ALT_SCREEN   (1 << 10)  /* Alternate screen */
#define XCMD_CAP_MOUSE        (1 << 11)  /* Mouse support */
#define XCMD_CAP_UTF8         (1 << 12)  /* UTF-8 support */
#define XCMD_CAP_KEYPAD       (1 << 13)  /* Numeric keypad */
#define XCMD_CAP_TITLE        (1 << 14)  /* Window title */

#define XCMD_CAP_BASIC        (XCMD_CAP_CURSOR | XCMD_CAP_CLEAR | XCMD_CAP_CLEAR_LINE)
#define XCMD_CAP_ENHANCED     (XCMD_CAP_BASIC | XCMD_CAP_256COLOR | XCMD_CAP_CURSOR_SAVE | XCMD_CAP_BRACKETED)

/* ============================================================
 * Standard ANSI Colors (16 colors)
 * ============================================================ */
typedef enum {
    XCMD_COLOR_BLACK   = 0,
    XCMD_COLOR_RED     = 1,
    XCMD_COLOR_GREEN   = 2,
    XCMD_COLOR_YELLOW  = 3,
    XCMD_COLOR_BLUE    = 4,
    XCMD_COLOR_MAGENTA = 5,
    XCMD_COLOR_CYAN    = 6,
    XCMD_COLOR_WHITE   = 7,
    XCMD_COLOR_DEFAULT = 9
} xcmd_color_t;

/* Color mode */
typedef enum {
    XCMD_COLOR_MODE_NONE = 0,
    XCMD_COLOR_MODE_16,      /* 16 color mode */
    XCMD_COLOR_MODE_256,    /* 256 color mode */
    XCMD_COLOR_MODE_RGB      /* 24-bit RGB mode */
} xcmd_color_mode_t;

/* Style attributes */
typedef uint8_t xcmd_attr_t;

#define XCMD_ATTR_BOLD         (1 << 0)
#define XCMD_ATTR_DIM          (1 << 1)
#define XCMD_ATTR_ITALIC       (1 << 2)
#define XCMD_ATTR_UNDERLINE    (1 << 3)
#define XCMD_ATTR_BLINK        (1 << 4)
#define XCMD_ATTR_REVERSE      (1 << 5)
#define XCMD_ATTR_HIDDEN       (1 << 6)
#define XCMD_ATTR_STRIKETHROUGH (1 << 7)

/* ============================================================
 * Style Structure
 * ============================================================ */
typedef struct {
    union {
        uint8_t  color8;     /* 16/256 color index */
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        } rgb;
    } fg;
    union {
        uint8_t  color8;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
        } rgb;
    } bg;
    xcmd_color_mode_t mode;
    xcmd_attr_t       attr;
} xcmd_style_t;

/* ============================================================
 * Terminal Size
 * ============================================================ */
typedef struct {
    uint16_t width;           /* Columns */
    uint16_t height;          /* Rows */
    uint16_t scroll_top;      /* Scroll region top */
    uint16_t scroll_bottom;   /* Scroll region bottom */
} xcmd_terminal_size_t;

/* ============================================================
 * Cursor Position
 * ============================================================ */
typedef struct {
    uint16_t row;             /* Row (0-based, top=0) */
    uint16_t col;             /* Column (0-based, left=0) */
} xcmd_cursor_pos_t;

/* ============================================================
 * Driver Interface
 * ============================================================ */
struct xcmd_term_driver {
    void (*put_char)(uint8_t c);
    void (*put_str)(const char* str);
    void (*flush)(void);
    
    void (*cursor_move)(uint16_t row, uint16_t col);
    void (*cursor_save)(void);
    void (*cursor_restore)(void);
    void (*cursor_show)(void);
    void (*cursor_hide)(void);
    
    void (*clear_screen)(void);
    void (*clear_line)(void);
    void (*clear_eol)(void);
    void (*clear_sol)(void);
    
    void (*scroll_up)(uint16_t lines);
    void (*scroll_down)(uint16_t lines);
    void (*set_scroll_region)(uint16_t top, uint16_t bottom);
    void (*reset_scroll_region)(void);
    
    void (*set_style)(const xcmd_style_t* style);
    void (*reset_style)(void);
    
    void (*alt_screen_enter)(void);
    void (*alt_screen_exit)(void);
    
    void (*set_title)(const char* title);
    
    void (*mouse_enable)(void);
    void (*mouse_disable)(void);
};

typedef struct xcmd_term_driver xcmd_term_driver_t;

/* ============================================================
 * Terminal Instance
 * ============================================================ */
typedef struct {
    xcmd_terminal_type_t   type;
    xcmd_term_cap_t        caps;
    const xcmd_term_driver_t* driver;
    xcmd_terminal_size_t   size;
    xcmd_style_t           current_style;
    uint8_t                initialized;
} xcmd_terminal_t;

/* Global terminal instance */
extern xcmd_terminal_t g_terminal;

/* ============================================================
 * Terminal Initialization
 * ============================================================ */

/**
 * @brief Initialize terminal with output function
 * @param put_c Output character function (platform provided)
 * @param type Terminal type, XCMD_TERM_AUTO for auto-detect
 */
void xcmd_terminal_init(int (*put_c)(uint8_t), xcmd_terminal_type_t type);

/**
 * @brief Auto-detect terminal type from TERM environment
 * @return Detected terminal type
 */
xcmd_terminal_type_t xcmd_terminal_detect(void);

/**
 * @brief Get terminal capabilities
 * @return Capability flags
 */
xcmd_term_cap_t xcmd_terminal_get_caps(void);

/**
 * @brief Get terminal type
 * @return Terminal type
 */
xcmd_terminal_type_t xcmd_terminal_get_type(void);

/**
 * @brief Get terminal size
 * @return Pointer to terminal size structure
 */
xcmd_terminal_size_t* xcmd_terminal_get_size(void);

/**
 * @brief Set terminal size manually (for serial terminals)
 * @param width Number of columns
 * @param height Number of rows
 */
void xcmd_terminal_set_size(uint16_t width, uint16_t height);

/**
 * @brief Check if terminal has a capability
 * @param cap Capability flag to check
 * @return 1 if supported, 0 if not
 */
#define xcmd_term_has(cap) ((g_terminal.caps & (cap)) != 0)

/* ============================================================
 * Output Functions
 * ============================================================ */

/**
 * @brief Output single character
 * @param c Character to output
 */
void xcmd_term_putc(char c);

/**
 * @brief Output string
 * @param str String to output
 */
void xcmd_term_puts(const char* str);

/**
 * @brief Output string with length
 * @param str String to output
 * @param len Number of characters
 */
void xcmd_term_puts_n(const char* str, uint16_t len);

/**
 * @brief Formatted output
 * @param fmt Format string
 * @param ... Arguments
 */
void xcmd_term_printf(const char* fmt, ...);

/**
 * @brief Flush output buffer
 */
void xcmd_term_flush(void);

/* ============================================================
 * Cursor Operations
 * ============================================================ */

/**
 * @brief Move cursor to position
 * @param row Row number (0-based)
 * @param col Column number (0-based)
 */
void xcmd_term_cursor_to(uint16_t row, uint16_t col);

/**
 * @brief Move cursor to column in current row
 * @param col Column number (0-based)
 */
void xcmd_term_cursor_to_col(uint16_t col);

/**
 * @brief Move cursor up
 * @param n Number of rows
 */
void xcmd_term_cursor_up(uint16_t n);

/**
 * @brief Move cursor down
 * @param n Number of rows
 */
void xcmd_term_cursor_down(uint16_t n);

/**
 * @brief Move cursor forward (right)
 * @param n Number of columns
 */
void xcmd_term_cursor_forward(uint16_t n);

/**
 * @brief Move cursor backward (left)
 * @param n Number of columns
 */
void xcmd_term_cursor_back(uint16_t n);

/**
 * @brief Save cursor position
 */
void xcmd_term_cursor_save(void);

/**
 * @brief Restore cursor position
 */
void xcmd_term_cursor_restore(void);

/**
 * @brief Show cursor
 */
void xcmd_term_cursor_show(void);

/**
 * @brief Hide cursor
 */
void xcmd_term_cursor_hide(void);

/* ============================================================
 * Screen Operations
 * ============================================================ */

/**
 * @brief Clear entire screen
 */
void xcmd_term_clear_screen(void);

/**
 * @brief Clear entire current line
 */
void xcmd_term_clear_line(void);

/**
 * @brief Clear from cursor to end of line
 */
void xcmd_term_clear_eol(void);

/**
 * @brief Clear from start of line to cursor
 */
void xcmd_term_clear_sol(void);

/**
 * @brief Scroll content up
 * @param lines Number of lines to scroll
 */
void xcmd_term_scroll_up(uint16_t lines);

/**
 * @brief Scroll content down
 * @param lines Number of lines to scroll
 */
void xcmd_term_scroll_down(uint16_t lines);

/**
 * @brief Set scroll region
 * @param top Top line (0-based)
 * @param bottom Bottom line (0-based)
 */
void xcmd_term_set_scroll_region(uint16_t top, uint16_t bottom);

/**
 * @brief Reset scroll region to full screen
 */
void xcmd_term_reset_scroll_region(void);

/* ============================================================
 * Style and Color
 * ============================================================ */

/**
 * @brief Set style (color + attributes)
 * @param style Pointer to style structure
 */
void xcmd_term_set_style(const xcmd_style_t* style);

/**
 * @brief Reset style to default
 */
void xcmd_term_reset_style(void);

/**
 * @brief Set 16-color foreground/background
 * @param fg Foreground color
 * @param bg Background color
 */
void xcmd_term_set_color(xcmd_color_t fg, xcmd_color_t bg);

/**
 * @brief Set foreground color only
 * @param color Foreground color
 */
void xcmd_term_set_fg(xcmd_color_t color);

/**
 * @brief Set background color only
 * @param color Background color
 */
void xcmd_term_set_bg(xcmd_color_t color);

/**
 * @brief Set text attributes
 * @param attr Attribute flags
 */
void xcmd_term_set_attr(xcmd_attr_t attr);

/**
 * @brief Set 256-color mode
 * @param fg Foreground color (0-255)
 * @param bg Background color (0-255)
 */
void xcmd_term_set_color_256(uint8_t fg, uint8_t bg);

/**
 * @brief Set RGB true color
 * @param r Red (0-255)
 * @param g Green (0-255)
 * @param b Blue (0-255)
 */
void xcmd_term_set_fg_rgb(uint8_t r, uint8_t g, uint8_t b);
void xcmd_term_set_bg_rgb(uint8_t r, uint8_t g, uint8_t b);

/* Predefined styles */
void xcmd_term_style_default(void);
void xcmd_term_style_prompt(void);
void xcmd_term_style_error(void);
void xcmd_term_style_warning(void);
void xcmd_term_style_success(void);
void xcmd_term_style_info(void);

/* ============================================================
 * Advanced Features
 * ============================================================ */

/**
 * @brief Enter alternate screen buffer
 */
void xcmd_term_alt_screen_enter(void);

/**
 * @brief Exit alternate screen buffer
 */
void xcmd_term_alt_screen_exit(void);

/**
 * @brief Set window title
 * @param title Title string
 */
void xcmd_term_set_title(const char* title);

/**
 * @brief Enable bracketed paste mode
 */
void xcmd_term_bracketed_paste_on(void);

/**
 * @brief Disable bracketed paste mode
 */
void xcmd_term_bracketed_paste_off(void);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_TERMINAL_H */