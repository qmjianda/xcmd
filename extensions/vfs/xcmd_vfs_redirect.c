#include "xcmd_vfs_redirect.h"

#ifdef XCMD_USE_VFS

#include "xcmd_io.h"
#include "xcmd_vfs.h"
#include "xcmd_vfs_io.h"
#include "xcmd_ext.h"
#include "xcmd_session.h"

/* Static context for the extension handler */
static struct {
    xcmd_vfs_t *vfs;
} vfs_ctx;

static int vfs_ext_handler(xcmd_evt_t *evt) {
    if (!evt || evt->type != XCMD_EVT_PRE_EXEC) {
        return 0;
    }
    
    int argc = evt->data.pre_exec.argc;
    char **argv = evt->data.pre_exec.argv;
    xcmd_session_t *session = evt->session;
    
    /* Find ">" redirect symbol in argv */
    int redirect_idx = -1;
    for (int i = 0; i < argc; i++) {
        if (argv[i] && argv[i][0] == '>' && argv[i][1] == '\0') {
            redirect_idx = i;
            break;
        }
    }
    
    /* No redirect symbol found */
    if (redirect_idx < 0 || redirect_idx >= argc - 1) {
        return 0;
    }
    
    /* Get filename from argv[i+1] */
    const char *filename = argv[redirect_idx + 1];
    if (!filename) {
        evt->handled = 1;
        return -1;
    }
    
    /* Open file for write */
    xcmd_vfs_file_t *file = xcmd_vfs_open(vfs_ctx.vfs, filename,
                                          XCMD_VFS_MODE_WRITE | XCMD_VFS_MODE_CREATE);
    if (!file) {
        evt->handled = 1;
        return -1;
    }
    
    /* Convert VFS file to IO */
    xcmd_io_t *io = xcmd_io_file_create(file);
    if (!io) {
        xcmd_vfs_close(file);
        evt->handled = 1;
        return -1;
    }
    
    /* Save original stdout and set redirect */
    session->saved_stdout = session->stdout_io;
    session->stdout_io = io;
    
    /* Remove ">" and filename from argv by shifting */
    int new_argc = argc - 2;
    for (int i = redirect_idx; i < new_argc; i++) {
        argv[i] = argv[i + 2];
    }
    argv[new_argc] = NULL;
    
    /* Update argc */
    *evt->data.pre_exec.new_argc = new_argc;
    
    return 0;
}

void vfs_redirect_init(xcmd_session_t *sess, xcmd_vfs_t *vfs) {
    if (!sess || !vfs) {
        return;
    }
    vfs_ctx.vfs = vfs;
    xcmd_ext_register(sess, vfs_ext_handler, &vfs_ctx);
}

xcmd_io_t* xcmd_vfs_redirect_open(xcmd_vfs_t *vfs, const char *target) {
    if (!vfs || !target) return NULL;
    
    /* 路径解析 */
    if (target[0] != '/') {
        /* 相对路径，转换为绝对路径 */
        /* TODO: 使用 cwd */
    }
    
    /* 打开文件（VFS 会根据挂载点选择后端） */
    xcmd_vfs_file_t *file = xcmd_vfs_open(vfs, target, 
                                          XCMD_VFS_MODE_WRITE | XCMD_VFS_MODE_CREATE);
    if (!file) {
        return NULL;
    }
    
    /* 转换为 xcmd_io_t */
    return xcmd_io_file_create(file);
}

int xcmd_vfs_redirect_close(xcmd_io_t *io) {
    if (!io) return -1;
    return xcmd_io_close(io);
}

#endif /* XCMD_USE_VFS */
