#ifndef XCMD_VFS_CMDS_H
#define XCMD_VFS_CMDS_H

#include "xcmd_vfs.h"

void xcmd_vfs_cmds_init(void);
xcmd_vfs_t* xcmd_vfs_get_global(void);
void xcmd_vfs_register_cmds(void);

#endif
