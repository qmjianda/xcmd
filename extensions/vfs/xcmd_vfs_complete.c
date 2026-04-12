#include "xcmd_vfs_complete.h"
#include "xcmd_vfs.h"
#include "xcmd_vfs_cmds.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int xcmd_vfs_complete_path(const char *path, xcmd_complete_result_t *result) {
    if (!path || !result) return 0;
    
    result->count = 0;
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (!vfs) return 0;
    
    char dir_path[128] = {0};
    const char *filename;
    
    const char *last_slash = strrchr(path, '/');
    if (last_slash) {
        size_t dir_len = last_slash - path;
        if (dir_len >= sizeof(dir_path)) return 0;
        memcpy(dir_path, path, dir_len);
        dir_path[dir_len] = '\0';
        filename = last_slash + 1;
    } else {
        dir_path[0] = '/';
        dir_path[1] = '\0';
        filename = path;
    }
    
    size_t filename_len = strlen(filename);
    
    xcmd_vfs_dir_t dir;
    if (xcmd_vfs_opendir(&dir, vfs, dir_path) != 0) return 0;
    
    char entry_path[256];
    xcmd_vfs_stat_t stat;
    
    while (xcmd_vfs_readdir(&dir, &stat) == 0) {
        if (filename_len == 0 || strncmp(stat.name, filename, filename_len) == 0) {
            if (result->count >= 16) break;
            
            int len = snprintf(entry_path, sizeof(entry_path), "%s%s%s", 
                             dir_path, 
                             dir_path[strlen(dir_path)-1]=='/'?"":"/", 
                             stat.name);
            if (len > 0 && (size_t)len < sizeof(entry_path)) {
                if (stat.attr & XCMD_VFS_ATTR_DIRECTORY) {
                    strncat(entry_path, "/", sizeof(entry_path) - strlen(entry_path) - 1);
                }
                
                result->candidates[result->count] = strndup(entry_path, 255);
                if (result->candidates[result->count]) {
                    result->count++;
                }
            }
        }
    }
    
    xcmd_vfs_closedir(&dir);
    
    return result->count;
}
