#include "xcmd_cmd.h"
#include <string.h>
#include <stdlib.h>

static xcmd_cmd_t *g_cmd_list = NULL;

int xcmd_cmd_register(xcmd_cmd_t *cmds, int count) {
    if (!cmds || count <= 0) return -1;
    
    for (int i = 0; i < count; i++) {
        cmds[i].next = g_cmd_list;
        g_cmd_list = &cmds[i];
    }
    return 0;
}

xcmd_cmd_t* xcmd_cmd_find(const char *name) {
    if (!name) return NULL;
    
    for (xcmd_cmd_t *p = g_cmd_list; p; p = p->next) {
        if (strcmp(p->name, name) == 0) {
            return p;
        }
    }
    return NULL;
}

xcmd_cmd_t* xcmd_cmd_first(void) {
    return g_cmd_list;
}

int xcmd_cmd_unregister(const char *name) {
    if (!name) return -1;
    
    xcmd_cmd_t **prev = &g_cmd_list;
    xcmd_cmd_t *p = NULL;
    for (p = g_cmd_list; p; p = p->next) {
        if (strcmp(name, p->name) == 0) {
            *prev = p->next;
            return 0;
        }
        prev = &p->next;
    }
    return -1;
}

xcmd_cmd_t* xcmd_cmdlist_get(void) {
    return g_cmd_list;
}

int xcmd_unregister_cmd(const char *cmd) {
    return xcmd_cmd_unregister(cmd);
}

const char* xcmd_cmd_get_name(xcmd_cmd_t *cmd) {
    return cmd ? cmd->name : NULL;
}

int xcmd_cmd_complete_name(const char *partial, xcmd_complete_result_t *result) {
    if (!partial || !result) return 0;
    
    result->count = 0;
    size_t partial_len = strlen(partial);
    
    for (xcmd_cmd_t *p = g_cmd_list; p && result->count < XCMD_CMD_COMPLETION_MAX; p = p->next) {
        if (partial_len == 0 || strncmp(p->name, partial, partial_len) == 0) {
            result->candidates[result->count++] = p->name;
        }
    }
    
    return result->count;
}

int xcmd_cmd_complete_prefix(const char *input, 
                            const char *candidates[],
                            xcmd_complete_result_t *result) {
    if (!input || !candidates || !result) return 0;
    
    result->count = 0;
    size_t input_len = strlen(input);
    
    for (int i = 0; candidates[i] && result->count < XCMD_CMD_COMPLETION_MAX; i++) {
        if (input_len == 0 || strncmp(candidates[i], input, input_len) == 0) {
            result->candidates[result->count++] = (char*)candidates[i];
        }
    }
    
    return result->count;
}

int xcmd_cmd_complete_enum(const char *input,
                          const char *enum_vals[],
                          xcmd_complete_result_t *result) {
    return xcmd_cmd_complete_prefix(input, enum_vals, result);
}

#ifdef XCMD_USE_VFS
#include "xcmd_vfs.h"
#include "xcmd_vfs_cmds.h"
#include <stdio.h>

int xcmd_cmd_complete_path(const char *path, xcmd_complete_result_t *result) {
    if (!path || !result) return 0;
    
    result->count = 0;
    
    xcmd_vfs_t *vfs = xcmd_vfs_get_global();
    if (!vfs) return 0;
    
    char dir_path[256] = {0};
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
            if (result->count >= XCMD_CMD_COMPLETION_MAX) break;
            
            if (dir_path[strlen(dir_path) - 1] == '/') {
                snprintf(entry_path, sizeof(entry_path), "%s%s", dir_path, stat.name);
            } else {
                snprintf(entry_path, sizeof(entry_path), "%s/%s", dir_path, stat.name);
            }
            
            if (stat.attr & XCMD_VFS_ATTR_DIRECTORY) {
                strncat(entry_path, "/", sizeof(entry_path) - strlen(entry_path) - 1);
            }
            
            result->candidates[result->count] = strndup(entry_path, 255);
            if (result->candidates[result->count]) {
                result->count++;
            }
        }
    }
    
    xcmd_vfs_closedir(&dir);
    
    for (int i = 0; i < result->count; i++) {
        if (result->candidates[i] == NULL) {
            result->count = i;
            break;
        }
    }
    
    return result->count;
}
#else
int xcmd_cmd_complete_path(const char *path, xcmd_complete_result_t *result) {
    (void)path;
    if (result) result->count = 0;
    return 0;
}
#endif
