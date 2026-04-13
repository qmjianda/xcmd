/**
 * @file xcmd_vfs_io.c
 * @brief VFS file IO wrapper - wraps xcmd_vfs_file_t as xcmd_io_t
 * 
 * This module provides an adapter to use VFS files with the xcmd_io_t interface.
 * Zero dynamic memory allocation - uses the VFS file handle directly.
 */

#include "xcmd_io.h"
#include "xcmd_vfs.h"

/*============================================================================
 * Static callback implementations
 *============================================================================*/

/**
 * @brief Read from VFS file
 */
static int vfs_file_read(xcmd_io_t *io, void *buf, size_t len)
{
    if (!io || !io->user_data || !buf) {
        return XCMD_IO_ERROR;
    }
    
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    int ret = xcmd_vfs_read(file, buf, (uint32_t)len);
    
    if (ret < 0) {
        return XCMD_IO_ERROR;
    }
    return ret;
}

/**
 * @brief Write to VFS file
 */
static int vfs_file_write(xcmd_io_t *io, const void *buf, size_t len)
{
    if (!io || !io->user_data || !buf) {
        return XCMD_IO_ERROR;
    }
    
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    int ret = xcmd_vfs_write(file, buf, (uint32_t)len);
    
    if (ret < 0) {
        return XCMD_IO_ERROR;
    }
    return ret;
}

/**
 * @brief Flush VFS file buffers
 */
static int vfs_file_flush(xcmd_io_t *io)
{
    if (!io || !io->user_data) {
        return XCMD_IO_ERROR;
    }
    
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    
    /* VFS doesn't have explicit flush, but we can sync by closing/reopening
     * or rely on backend implementation. Return OK for now. */
    (void)file;
    return XCMD_IO_OK;
}

/**
 * @brief Close VFS file
 */
static int vfs_file_close(xcmd_io_t *io)
{
    if (!io || !io->user_data) {
        return XCMD_IO_ERROR;
    }
    
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    int ret = xcmd_vfs_close(file);
    
    /* Clear user_data to prevent double-close */
    io->user_data = NULL;
    
    return (ret == 0) ? XCMD_IO_OK : XCMD_IO_ERROR;
}

/**
 * @brief Check if VFS file is readable (always returns 1 for open files)
 */
static int vfs_file_readable(xcmd_io_t *io)
{
    if (!io || !io->user_data) {
        return XCMD_IO_ERROR;
    }
    
    xcmd_vfs_file_t *file = (xcmd_vfs_file_t *)io->user_data;
    
    /* File is readable if it's open and has read mode */
    if (file->is_open && (file->mode & XCMD_VFS_MODE_READ)) {
        return 1;
    }
    
    return 0;
}

/*============================================================================
 * Public API
 *============================================================================*/

/**
 * @brief Create an xcmd_io_t wrapper for a VFS file
 * 
 * @param file VFS file handle (must remain valid during IO operations)
 * @return xcmd_io_t* IO handle, or NULL on error
 * 
 * @note The VFS file remains owned by the caller. Closing the IO handle
 *       will close the underlying VFS file.
 */
xcmd_io_t* xcmd_io_file_create(xcmd_vfs_file_t *file)
{
    if (!file || !file->is_open) {
        return NULL;
    }
    
    static xcmd_io_t io;
    
    io.read = vfs_file_read;
    io.write = vfs_file_write;
    io.flush = vfs_file_flush;
    io.close = vfs_file_close;
    io.readable = vfs_file_readable;
    io.user_data = file;
    
    return &io;
}
