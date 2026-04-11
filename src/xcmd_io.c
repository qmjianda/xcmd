#include "xcmd_io.h"
#include <stdlib.h>

/*============================================================================
 * VFS File IO Backend
 *============================================================================*/

#ifdef XCMD_USE_VFS

#include "xcmd_vfs.h"

static int file_read(xcmd_io_t *io, void *buf, size_t len) {
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    return xcmd_vfs_read(file, buf, (uint32_t)len);
}

static int file_write(xcmd_io_t *io, const void *buf, size_t len) {
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    return xcmd_vfs_write(file, buf, (uint32_t)len);
}

static int file_close(xcmd_io_t *io) {
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    return xcmd_vfs_close(file);
}

xcmd_io_t* xcmd_io_file_create(xcmd_vfs_file_t *file) {
    static xcmd_io_t io;
    io.read = file_read;
    io.write = file_write;
    io.flush = NULL;
    io.close = file_close;
    io.readable = NULL;
    io.user_data = file;
    return &io;
}

xcmd_io_t* xcmd_io_file_open(xcmd_vfs_t *vfs, const char *path, uint8_t mode) {
    xcmd_vfs_file_t *file = xcmd_vfs_open(vfs, path, mode);
    if (!file) return NULL;
    return xcmd_io_file_create(file);
}

#endif /* XCMD_USE_VFS */

/*============================================================================
 * Utility Functions
 *============================================================================*/

int xcmd_io_copy(xcmd_io_t *src, xcmd_io_t *dst, void *buf, size_t buf_size) {
    if (!src || !dst || !buf || buf_size == 0) return XCMD_IO_ERROR;
    
    int total = 0;
    int n;
    
    while ((n = xcmd_io_read(src, buf, buf_size)) > 0) {
        int written = xcmd_io_write(dst, buf, n);
        if (written < 0) return XCMD_IO_ERROR;
        total += written;
        if (written < n) break;
    }
    
    return (n < 0) ? XCMD_IO_ERROR : total;
}
