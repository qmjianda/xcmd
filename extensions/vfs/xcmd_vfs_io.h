#ifndef XCMD_VFS_IO_H
#define XCMD_VFS_IO_H

#include "xcmd_io.h"
#include "xcmd_vfs.h"

#ifdef __cplusplus
extern "C" {
#endif

xcmd_io_t* xcmd_io_file_create(xcmd_vfs_file_t *file);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_VFS_IO_H */
