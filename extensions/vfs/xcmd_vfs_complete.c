#include "xcmd_vfs_complete.h"
#include "xcmd_vfs.h"
#include "xcmd_vfs_cmds.h"
#include "xcmd_cmd.h"
#include <string.h>
#include <stdio.h>

static char g_vfs_complete_buf[16][128];

int xcmd_vfs_complete_path(xcmd_line_editor_t *le, const char *word, int word_id, char *matches[], int max) {
    (void)le;
    (void)word_id;
    if (!word || !matches) return 0;
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (!vfs) return 0;
    
    char dir_path[128] = {0};
    const char *filename;
    
    const char *last_slash = strrchr(word, '/');
    if (last_slash) {
        size_t dir_len = last_slash - word;
        if (dir_len >= sizeof(dir_path)) return 0;
        memcpy(dir_path, word, dir_len);
        dir_path[dir_len] = '\0';
        filename = last_slash + 1;
    } else {
        dir_path[0] = '/';
        dir_path[1] = '\0';
        filename = word;
    }
    
    size_t filename_len = strlen(filename);
    int count = 0;
    
    xcmd_vfs_dir_t dir;
    if (xcmd_vfs_opendir(&dir, vfs, dir_path) != 0) return 0;
    
    char entry_path[256];
    xcmd_vfs_stat_t stat;
    
    while (count < max && count < 16 && xcmd_vfs_readdir(&dir, &stat) == 0) {
        if (filename_len == 0 || strncmp(stat.name, filename, filename_len) == 0) {
            int len = snprintf(entry_path, sizeof(entry_path), "%s%s%s", 
                             dir_path, 
                             dir_path[strlen(dir_path)-1]=='/'?"":"/", 
                             stat.name);
            if (len > 0 && (size_t)len < sizeof(entry_path)) {
                if (stat.attr & XCMD_VFS_ATTR_DIRECTORY) {
                    strncat(entry_path, "/", sizeof(entry_path) - strlen(entry_path) - 1);
                }
                
                strncpy(g_vfs_complete_buf[count], entry_path, sizeof(g_vfs_complete_buf[count]) - 1);
                g_vfs_complete_buf[count][sizeof(g_vfs_complete_buf[count]) - 1] = '\0';
                matches[count] = g_vfs_complete_buf[count];
                count++;
            }
        }
    }
    
    xcmd_vfs_closedir(&dir);
    
    return count;
}

void xcmd_vfs_complete_init(void) {
    xcmd_register_path_complete(xcmd_vfs_complete_path);
}
