#include "xcmd_vfs.h"
#include "xcmd_vfs_cmds.h"
#include "xcmd_cmd.h"
#include "xcmd_session.h"
#include "xcmd_define.h"
#include <string.h>
#include <stdio.h>

static xcmd_vfs_t g_vfs;
static int g_vfs_initialized = 0;

xcmd_vfs_t* xcmd_vfs_get_global(void) {
    return &g_vfs;
}

void xcmd_vfs_cmds_init(void) {
    if (!g_vfs_initialized) {
        xcmd_vfs_init(&g_vfs);
        g_vfs_initialized = 1;
    }
}

static int cmd_ls(int argc, char* argv[]) {
    if (argc < 1) return -1;
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    const char *path = (argc > 1) ? argv[1] : xcmd_vfs_getcwd(vfs);
    
    xcmd_vfs_dir_t dir;
    xcmd_vfs_stat_t st;
    
    if (xcmd_vfs_opendir(&dir, vfs, path) != 0) {
        xcmd_session_print_current("Cannot open directory: %s\r\n", path);
        return -1;
    }
    
    while (xcmd_vfs_readdir(&dir, &st) == 0) {
        if (st.attr & XCMD_VFS_ATTR_DIRECTORY) {
            xcmd_session_print_current("\x1B[34m%-20s\x1B[0m", st.name);
        } else {
            xcmd_session_print_current("%-20s", st.name);
        }
        xcmd_session_print_current("  %lu\r\n", (unsigned long)st.size);
    }
    
    xcmd_vfs_closedir(&dir);
    return 0;
}

static int cmd_cd(int argc, char* argv[]) {
    if (argc < 2) {
        xcmd_session_print_current("%s\r\n", xcmd_vfs_getcwd(xcmd_vfs_get_global()));
        return 0;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (xcmd_vfs_chdir(vfs, argv[1]) != 0) {
        xcmd_session_print_current("Cannot change directory: %s\r\n", argv[1]);
        return -1;
    }
    
    xcmd_session_print_current("%s\r\n", xcmd_vfs_getcwd(vfs));
    return 0;
}

static int cmd_pwd(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    xcmd_session_print_current("%s\r\n", xcmd_vfs_getcwd(xcmd_vfs_get_global()));
    return 0;
}

static int cmd_mkdir(int argc, char* argv[]) {
    if (argc < 2) {
        xcmd_session_print_current("Usage: mkdir <path>\r\n");
        return -1;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (xcmd_vfs_mkdir(vfs, argv[1]) != 0) {
        xcmd_session_print_current("Cannot create directory: %s\r\n", argv[1]);
        return -1;
    }
    return 0;
}

static int cmd_rm(int argc, char* argv[]) {
    if (argc < 2) {
        xcmd_session_print_current("Usage: rm [-r] <path>\r\n");
        return -1;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    
    if (strcmp(argv[1], "-r") == 0 && argc > 2) {
        return cmd_rm(argc - 1, argv + 1);
    }
    
    if (xcmd_vfs_rm(vfs, argv[1]) != 0) {
        xcmd_session_print_current("Cannot remove: %s\r\n", argv[1]);
        return -1;
    }
    return 0;
}

static int cmd_cat(int argc, char* argv[]) {
    if (argc < 2) {
        xcmd_session_print_current("Usage: cat <file>\r\n");
        return -1;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    xcmd_vfs_file_t *file = xcmd_vfs_open(vfs, argv[1], XCMD_VFS_MODE_READ);
    
    if (!file) {
        xcmd_session_print_current("Cannot open file: %s\r\n", argv[1]);
        return -1;
    }
    
    char buf[129];
    int len;
    while ((len = xcmd_vfs_read(file, buf, 128)) > 0) {
        buf[len] = '\0';
        xcmd_session_print_current("%s", buf);
    }
    
    xcmd_vfs_close(file);
    return 0;
}

static int cmd_cp(int argc, char* argv[]) {
    if (argc < 3) {
        xcmd_session_print_current("Usage: cp <src> <dst>\r\n");
        return -1;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    
    xcmd_vfs_file_t *src = xcmd_vfs_open(vfs, argv[1], XCMD_VFS_MODE_READ);
    if (!src) {
        xcmd_session_print_current("Cannot open source: %s\r\n", argv[1]);
        return -1;
    }
    
    xcmd_vfs_file_t *dst = xcmd_vfs_open(vfs, argv[2], XCMD_VFS_MODE_WRITE | XCMD_VFS_MODE_CREATE);
    if (!dst) {
        xcmd_session_print_current("Cannot create destination: %s\r\n", argv[2]);
        xcmd_vfs_close(src);
        return -1;
    }
    
    char buf[512];
    int len;
    while ((len = xcmd_vfs_read(src, buf, 512)) > 0) {
        if (xcmd_vfs_write(dst, buf, len) != len) {
            xcmd_session_print_current("Write error\r\n");
            xcmd_vfs_close(src);
            xcmd_vfs_close(dst);
            return -1;
        }
    }
    
    xcmd_vfs_close(src);
    xcmd_vfs_close(dst);
    return 0;
}

static int cmd_mv(int argc, char* argv[]) {
    if (argc < 3) {
        xcmd_session_print_current("Usage: mv <src> <dst>\r\n");
        return -1;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    
    if (xcmd_vfs_rename(vfs, argv[1], argv[2]) != 0) {
        xcmd_session_print_current("Cannot rename: %s -> %s\r\n", argv[1], argv[2]);
        return -1;
    }
    return 0;
}

static int cmd_df(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    uint32_t free_bytes = xcmd_vfs_getfree(vfs, "/");
    
    xcmd_session_print_current("Free space: %lu bytes\r\n", (unsigned long)free_bytes);
    return 0;
}

static int cmd_touch(int argc, char* argv[]) {
    if (argc < 2) {
        xcmd_session_print_current("Usage: touch <file>\r\n");
        return -1;
    }
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    xcmd_vfs_file_t *file = xcmd_vfs_open(vfs, argv[1], XCMD_VFS_MODE_WRITE | XCMD_VFS_MODE_CREATE);
    
    if (!file) {
        xcmd_session_print_current("Cannot create file: %s\r\n", argv[1]);
        return -1;
    }
    
    xcmd_vfs_close(file);
    return 0;
}

static xcmd_cmd_t vfs_cmds[] = {
    {"ls", cmd_ls, "list directory contents", NULL},
    {"cd", cmd_cd, "change directory", NULL},
    {"pwd", cmd_pwd, "print working directory", NULL},
    {"mkdir", cmd_mkdir, "make directory", NULL},
    {"rm", cmd_rm, "remove file or directory", NULL},
    {"cat", cmd_cat, "display file contents", NULL},
    {"cp", cmd_cp, "copy file", NULL},
    {"mv", cmd_mv, "move/rename file", NULL},
    {"df", cmd_df, "show disk free space", NULL},
    {"touch", cmd_touch, "create empty file", NULL},
};

void xcmd_vfs_register_cmds(void) {
    xcmd_vfs_cmds_init();
    xcmd_cmd_register(vfs_cmds, sizeof(vfs_cmds) / sizeof(xcmd_cmd_t));
}
