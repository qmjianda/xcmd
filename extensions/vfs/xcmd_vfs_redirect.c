#include "xcmd_vfs_redirect.h"

#ifdef XCMD_USE_VFS

#include "xcmd_io.h"
#include "xcmd_vfs.h"

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
