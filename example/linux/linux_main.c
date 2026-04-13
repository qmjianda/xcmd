#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "xcmd_platform.h"
#include "xcmd_cmd.h"
#include "xcmd_io.h"
#include "xcmd_session.h"
#include "xcmd_term.h"
#include "xcmd_define.h"
#include "test.h"
#include "ex_cmds.h"
#include "ex_list.h"
#include "socket_cmds.h"
#include "xcmd_vfs.h"
#include "xcmd_vfs_cmds.h"
#include "xcmd_vfs_fatfs.h"
#include "xcmd_vfs_proc.h"
#include "fatfs_disk_ram.h"
#include "xcmd_proc_confg.h"
#include "xcmd_vfs_dev.h"
#include "ex_redirect.h"

static volatile int g_exit_requested = 0;

static int cmd_exit(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    xcmd_session_print_current("Goodbye!\r\n");
    g_exit_requested = 1;
    return 0;
}

static xcmd_cmd_t linux_cmds[] = {
    {"exit", cmd_exit, "exit the program", NULL},
};

int getch(void)
{
    struct termios tm, tm_old;
    int fd = 0, ch;

    if (tcgetattr(fd, &tm) < 0)
    {
        return -1;
    }
    tm_old = tm;
    cfmakeraw(&tm);
    if (tcsetattr(fd, TCSANOW, &tm) < 0)
    {
        return -1;
    }

    ch = getchar();
    if (tcsetattr(fd, TCSANOW, &tm_old) < 0)
    {
        return -1;
    }

    return ch;
}

int linux_read(xcmd_io_t *io, void *buf, size_t len)
{
    char *char_buf = (char *)buf;
    char_buf[0] = getch();
    return 1;
}

int linux_write(xcmd_io_t *io, const void *buf, size_t len)
{
    const char *char_buf = (const char *)buf;
    while (len--) {
        putchar(*char_buf++);
    }
    fflush(stdout);
    return len; 
}

int linux_flush(xcmd_io_t *io)
{
    fflush(stdout);
    return 0;
}

xcmd_io_t linux_io = {
    .read = linux_read,
    .write = linux_write,
    .flush = linux_flush,
    .close = NULL,
    .readable = NULL,
    .user_data = NULL
};

xcmd_session_t session;

int main(void)
{
    if (ram_disk_init() != 0) {
        printf("Failed to initialize RAM disk\r\n");
        return -1;
    }

    // if (mmc_disk_init() != 0) {
    //     printf("Failed to initialize MMC disk\r\n");
    //     return -1;
    // }

    xcmd_vfs_cmds_init();
    
    static xcmd_vfs_fatfs_t fatfs_backend;
    xcmd_vfs_fatfs_init(&fatfs_backend, ram_disk_get_path());
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (xcmd_vfs_mount(vfs, "/", &xcmd_vfs_fatfs_ops, &fatfs_backend, 0) != 0) {
        printf("Failed to mount FatFS to VFS\r\n");
        return -1;
    }
    
    if (xcmd_vfs_proc_mount(vfs, "/proc") != 0) {
        printf("Failed to mount proc to VFS\r\n");
        return -1;
    }
    
    xcmd_proc_confg_init();
    
    /* Mount /dev for device access (null, zero, etc.) */
    if (xcmd_vfs_dev_mount(vfs, "/dev") != 0) {
        printf("Failed to mount dev to VFS\r\n");
        return -1;
    }
    
    /* Register built-in devices */
    xcmd_vfs_dev_register((xcmd_dev_t *)&xcmd_dev_null);
    xcmd_vfs_dev_register((xcmd_dev_t *)&xcmd_dev_zero);
    
    const xcmd_term_ops_t *term_ops = xcmd_term_get_ansi_ops();
    if (!term_ops) {
        printf("Failed to get terminal operations\r\n");
        return -1;
    }

    xcmd_io_t *io = &linux_io;
    xcmd_session_t *sess = &session;
    
    if (xcmd_session_init(sess, io, term_ops) != 0) {
        printf("Failed to init xcmd session\r\n");
        return -1;
    }
    
    test_cmd_init();
    ex_cmds_init();
    socket_cmds_init();
    ex_list_init();
    xcmd_vfs_register_cmds();
    ex_redirect_init();
    xcmd_cmd_register(linux_cmds, sizeof(linux_cmds)/sizeof(xcmd_cmd_t));
    
    printf("\r\nVFS initialized. Try 'ls', 'mkdir test', 'touch file.txt' commands.\r\n");
    printf("Redirect test: 'redirect_test > /test.txt' or 'redirect_test > /dev/null'\r\n\r\n");
    
    xcmd_session_exec(sess, "logo");
    while (!g_exit_requested)
    {
        xcmd_session_task(sess);
    }
    return 0;
}