# AGENTS.md

## Project Overview

xcmd is a lightweight embedded command-line tool for microcontrollers (MCUs). It provides a minimal-footprint shell with command history, autocomplete, and hotkey support.

## Key Architecture

### Core Files
- `src/xcmd.c` - Main interpreter (command parsing, history, display)
- `src/xcmd_default_cmds.c` - Built-in commands (help, etc.)
- `src/xcmd_default_keys.c` - Built-in key bindings
- `inc/xcmd.h` - **Primary public API header**
- `inc/xcmd_define.h` - Key codes and ANSI terminal sequences

### Extension Modules
- `extensions/ex_cmds/` - Extended commands
- `extensions/ex_keys/` - Extended key bindings
- `extensions/fs_cmds/` - FatFS file operations (ls, cd, rm, mkdir, cat)
- `extensions/net_cmds/` - Socket/network commands

## Installation Pattern

To port to a new platform:
1. Copy `xcmd_confg.h` from appropriate example (NOT xcmd_config.h)
2. Implement character I/O functions: `int get_char(uint8_t*)` and `int put_char(uint8_t)`
3. Call `xcmd_init(get_char, put_char)` in setup
4. Call `xcmd_task()` in main loop

Minimal example:
```c
#include "xcmd.h"
int cmd_get_char(uint8_t *ch) { /* platform-specific */ }
int cmd_put_char(uint8_t ch) { /* platform-specific */ }
void main(void) {
    xcmd_init(cmd_get_char, cmd_put_char);
    while(1) { xcmd_task(); }
}
```

## Command Registration

Two methods:

**Runtime Registration (simpler):**
```c
static xcmd_cmd_t cmds[] = {{"echo", cmd_echo, "description", NULL}};
xcmd_cmd_register(cmds, sizeof(cmds)/sizeof(xcmd_cmd_t));
```

**Static Export (requires linker script):**
```c
XCMD_EXPORT_CMD(echo, cmd_echo, "description", NULL)
```
Enable `ENABLE_XCMD_EXPORT` and add sections to linker script (see `example/stm32/qemu/demos/main.ld`).

## Build Commands

| Platform | Directory | Command |
|----------|-----------|---------|
| Linux | `example/linux/` | `make` |
| ESP-IDF | `example/esp_idf/` | `idf.py build` |
| STM32 QEMU | `example/stm32/qemu/` | `make` then `make main_QEMURUN_PTY` |
| Arduino | `example/arduino/` | `python3 init.py` then Arduino IDE |
| STM32 MDK | `example/stm32/MDK/` | Open in Keil IDE |

## Configuration

File: `xcmd_confg.h` (note: NOT `xcmd_config.h`)

Key macros:
- `XCMD_LINE_MAX_LENGTH` - Max command line chars (default 64)
- `XCMD_HISTORY_MAX_NUM` - History entries (0=disabled, default 8)
- `XCMD_PARAM_MAX_NUM` - Max parameters (default 8)
- `ENABLE_XCMD_EXPORT` - Enable static export (requires linker script mods)

Platform recommendations:
- Arduino: `XCMD_HISTORY_MAX_NUM=0`, `XCMD_LINE_MAX_LENGTH=32` (RAM limited)
- Linux: Full settings, history enabled
- STM32: Conservative settings based on available RAM

## Important Notes

- Config file is `xcmd_confg.h` (common misspelling: it's NOT "config" but "confg")
- No dedicated test directory - testing via QEMU or on-target
- Color codes in `xcmd_define.h`: `TX_GREEN`, `TX_RED`, etc. for ANSI terminals
- CI pipelines in `.workflow/` use Maven but appear to be templates, not project-specific

## Platform-Specific Details

### Arduino
Run `python3 init.py` in `example/arduino/` before opening in Arduino IDE. This sets up symlinks to core source files.

### STM32 QEMU
Requires qemu_stm32 and arm-none-eabi-gcc toolchain. See `example/stm32/qemu/README.md` for setup sessructions.

### ESP-IDF
Standard ESP-IDF project structure. Uses CMake build system.