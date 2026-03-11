# xcmd Display Module Redesign Specification

## Overview

This document describes the redesigned terminal abstraction layer for xcmd v2.0. The redesign provides a clean separation between terminal capabilities and display logic, enabling support for multiple terminal types with graceful degradation.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                      xcmd Application                        │
│                   (Command Parse & Execute)                 │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      xcmd_display.h                         │
│                   (High-level Display API)                  │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
┌─────────────────────────────────────────────────────────────┐
│                      xcmd_terminal.h                        │
│              (Terminal Abstraction Layer)                   │
└─────────────────────────────────────────────────────────────┘
                              │
        ┌─────────────────────┼─────────────────────┐
        ▼                     ▼                     ▼
┌──────────────┐      ┌──────────────┐      ┌──────────────┐
│  ANSI Driver │      │  Dumb Driver │      │  (Future)    │
│ (Linux/macOS)│      │(STM32/Serial)│      │              │
└──────────────┘      └──────────────┘      └──────────────┘
```

## Terminal Types

| Type | Description | Capabilities |
|------|-------------|--------------|
| `XCMD_TERM_DUMB` | Plain text terminal, no escape sequences | None |
| `XCMD_TERM_VT100` | DEC VT100 | Basic cursor, clear |
| `XCMD_TERM_VT220` | DEC VT220 | Basic + cursor save/restore |
| `XCMD_TERM_ANSI` | ANSI X3.64 standard | Basic + save/restore |
| `XCMD_TERM_XTERM` | xterm compatible | Enhanced + 256color + title |
| `XCMD_TERM_TMUX` | tmux session | Enhanced + alt screen |
| `XCMD_TERM_SCREEN` | GNU screen | Enhanced + alt screen |
| `XCMD_TERM_CONEMU` | Windows ConEmu | Enhanced |
| `XCMD_TERM_WINDOWS` | Windows Console | Enhanced |
| `XCMD_TERM_AUTO` | Auto-detect from TERM env | Depends on detected type |

## Capability Flags

```c
#define XCMD_CAP_COLOR        (1 << 0)   /* Basic color */
#define XCMD_CAP_256COLOR     (1 << 1)   /* 256 colors */
#define XCMD_CAP_TRUECOLOR    (1 << 2)   /* 24-bit RGB */
#define XCMD_CAP_CURSOR       (1 << 3)   /* Cursor movement */
#define XCMD_CAP_CURSOR_SAVE  (1 << 4)   /* Save/restore cursor */
#define XCMD_CAP_CLEAR        (1 << 5)   /* Clear screen */
#define XCMD_CAP_CLEAR_LINE   (1 << 6)   /* Clear line */
#define XCMD_CAP_SCROLL       (1 << 7)   /* Scroll */
#define XCMD_CAP_SCROLL_REG   (1 << 8)   /* Scroll region */
#define XCMD_CAP_BRACKETED    (1 << 9)   /* Bracketed paste */
#define XCMD_CAP_ALT_SCREEN   (1 << 10)  /* Alternate screen */
#define XCMD_CAP_MOUSE        (1 << 11)  /* Mouse support */
#define XCMD_CAP_UTF8         (1 << 12)  /* UTF-8 */
#define XCMD_CAP_TITLE        (1 << 14)  /* Window title */
```

## File Structure

```
inc/
├── xcmd_types.h           # Existing types
├── xcmd.h                 # Main header (updated)
├── xcmd_display.h         # Display API (new)
├── xcmd_display_style.h   # Style definitions (new)
├── xcmd_terminal.h        # Terminal abstraction (new)
└── xcmd_define.h          # Existing definitions

src/
├── xcmd.c                 # Core (updated)
├── xcmd_display.c         # Display implementation (new)
├── xcmd_terminal.c        # Terminal driver (new)
├── xcmd_term_dumb.c       # Dumb terminal driver (new)
└── xcmd_term_ansi.c       # ANSI terminal driver (new)
```

## API Usage

### Basic Usage

```c
// Platform initialization
void platform_init(void)
{
    // Auto-detect terminal type from TERM env
    xcmd_terminal_init(uart_put_char, XCMD_TERM_AUTO);
    
    // Or specify manually for embedded platforms
    // xcmd_terminal_init(uart_put_char, XCMD_TERM_ANSI);
    
    xcmd_init(get_char, uart_put_char);
}
```

### Color Output

```c
// Using convenience macros
xcmd_error("Error: %d\n", err_code);
xcmd_warn("Warning: %s\n", msg);
xcmd_info("Info: %d items\n", count);
xcmd_success("Success!\n");

// Direct API
xcmd_term_set_color(XCMD_COLOR_GREEN, XCMD_COLOR_BLACK);
xcmd_term_printf("Hello\n");
xcmd_term_reset_style();
```

### Progress Bar

```c
// 50% progress, 20 characters wide
xcmd_display_progress_bar(50, 20);
// Output: [##########          ] 50%
```

## Backward Compatibility

The existing API is preserved through compatibility macros:

```c
#define xcmd_print(...)      xcmd_term_printf(__VA_ARGS__)
#define xcmd_put_str(s)      xcmd_term_puts(s)
#define xcmd_display_get()   xcmd_display_get_buffer()
```

## Resource Usage

| Component | RAM | Flash |
|-----------|-----|-------|
| Terminal Layer | ~20B | ~1KB |
| ANSI Driver | 0 | ~800B |
| Dumb Driver | 0 | ~100B |
| Display Layer | ~150B | ~500B |
| **Total** | **~170B** | **~2.4KB** |

## Feature Flags

| Flag | Default | Description |
|------|---------|-------------|
| `XCMD_FEATURE_COLOR` | 1 | Enable color output |
| `XCMD_FEATURE_PROGRESS` | 1 | Enable progress bar |
| `XCMD_FEATURE_ALT_SCREEN` | 0 | Enable alternate screen |

## Platform Notes

### Linux/macOS
- Auto-detects from `TERM` environment variable
- Supports full ANSI features including 256-color

### STM32
- Use `XCMD_TERM_ANSI` or `XCMD_TERM_DUMB`
- Manually set terminal size via `xcmd_terminal_set_size()`

### Arduino
- Use `XCMD_TERM_DUMB` (limited serial capabilities)
- Disable history (`XCMD_HISTORY_MAX_NUM=0`) to save RAM

### ESP32
- Use `XCMD_TERM_ANSI`
- Supports WiFi-enabled terminal apps

## Implementation Status

- [x] Terminal type definitions
- [x] Capability flags
- [x] Driver interface
- [x] ANSI driver implementation
- [x] Dumb driver implementation
- [x] Terminal detection
- [x] Display API
- [x] Progress bar
- [x] Backward compatibility macros
- [ ] Vi mode (future)
- [ ] Syntax highlighting (future)
- [ ] Multiline input (future)

---

*Document Version: 1.0*
*Last Updated: 2026-03-11*