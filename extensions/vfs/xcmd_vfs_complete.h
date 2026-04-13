#ifndef XCMD_VFS_COMPLETE_H
#define XCMD_VFS_COMPLETE_H

#include "xcmd_line_editor.h"

int xcmd_vfs_complete_path(xcmd_line_editor_t *le, const char *word, int word_id, char *matches[], int max);
void xcmd_vfs_complete_init(void);

#endif
