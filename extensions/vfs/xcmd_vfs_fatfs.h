#ifndef XCMD_VFS_FATFS_H
#define XCMD_VFS_FATFS_H

#include "xcmd_vfs.h"
#include "ff.h"

typedef struct {
    FATFS fs;
    const char *drive_path;
    FIL handles[XCMD_VFS_MAX_OPEN_FILES];
    DIR dir_handles[XCMD_VFS_MAX_OPEN_FILES];
} xcmd_vfs_fatfs_t;

int xcmd_vfs_fatfs_init(xcmd_vfs_fatfs_t *vfs, const char *drive_path);

extern const xcmd_vfs_ops_t xcmd_vfs_fatfs_ops;

#endif
