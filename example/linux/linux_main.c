#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "xcmd_platform.h"
#include "xcmd.h"
#include "xcmd_io.h"
#include "xcmd_session.h"
#include "xcmd_term.h"
#include "test.h"
#include "ex_cmds.h"
#include "ex_list.h"
#include "socket_cmds.h"
#include "xcmd_vfs.h"
#include "xcmd_vfs_cmds.h"
#include "xcmd_vfs_fatfs.h"
#include "fatfs_disk_ram.h"

static volatile int g_exit_requested = 0;

static int cmd_exit(int argc, char* argv[])
{
    (void)argc;
    (void)argv;
    xcmd_session_print_current("Goodbye!\r\n");
    g_exit_requested = 1;
    return 0;
}

static xcmd_t linux_cmds[] = {
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

int cmd_get_char(uint8_t *ch)
{
    *ch = getch();
    return 1;
}

int cmd_put_char(uint8_t ch)
{
    putchar(ch);
    fflush(stdout);
    return 1;
}

int main(void)
{
    if (ram_disk_init() != 0) {
        printf("Failed to initialize RAM disk\r\n");
        return -1;
    }

    if (mmc_disk_init() != 0) {
        printf("Failed to initialize MMC disk\r\n");
        return -1;
    }

    xcmd_vfs_cmds_init();
    
    static xcmd_vfs_fatfs_t fatfs_backend;
    xcmd_vfs_fatfs_init(&fatfs_backend, ram_disk_get_path());
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (xcmd_vfs_mount(vfs, "/", &xcmd_vfs_fatfs_ops, &fatfs_backend) != 0) {
        printf("Failed to mount FatFS to VFS\r\n");
        return -1;
    }
    
    xcmd_io_t *io = xcmd_io_term_create(cmd_get_char, cmd_put_char);
    if (!io) {
        printf("Failed to create terminal IO\r\n");
        return -1;
    }
    
    xcmd_term_ops_t *term_ops = xcmd_term_get_ansi_ops();
    if (!term_ops) {
        printf("Failed to get terminal operations\r\n");
        return -1;
    }

    static xcmd_session_t session;
    xcmd_session_t *sess = &session;
    
    if (xcmd_session_init(sess, io, term_ops) != 0) {
        printf("Failed to init xcmd session\r\n");
        return -1;
    }
    
    xcmd_session_set_vfs(sess, vfs);
    
    test_cmd_init();
    ex_cmds_init();
    socket_cmds_init();
    ex_list_init();
    xcmd_vfs_register_cmds();
    xcmd_cmd_register(linux_cmds, sizeof(linux_cmds)/sizeof(xcmd_t));
    
    printf("\r\nVFS and FatFS initialized. Try 'ls', 'mkdir test', 'touch file.txt' commands.\r\n\r\n");
    
    while (!g_exit_requested)
    {
        xcmd_session_task(sess);
    }
    return 0;
}