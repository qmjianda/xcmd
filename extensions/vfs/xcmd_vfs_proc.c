#include "xcmd_vfs_proc.h"
#include <string.h>

/* 伪文件注册表（静态） */
#ifndef XCMD_VFS_PROC_MAX
#define XCMD_VFS_PROC_MAX  32
#endif

#ifndef XCMD_VFS_PROC_HANDLE_MAX
#define XCMD_VFS_PROC_HANDLE_MAX  4
#endif

static xcmd_proc_entry_t *proc_table[XCMD_VFS_PROC_MAX];
static int proc_count = 0;

typedef struct {
    xcmd_proc_entry_t *entry;
    uint8_t used;
} proc_file_handle_t;

typedef struct {
    int index;
    uint8_t used;
} proc_dir_handle_t;

static proc_file_handle_t file_handle_pool[XCMD_VFS_PROC_HANDLE_MAX];
static proc_dir_handle_t dir_handle_pool[XCMD_VFS_PROC_HANDLE_MAX];

/* VFS 操作接口实现 */
static int proc_mount(void *fs_data) {
    (void)fs_data;
    return 0;
}

static int proc_unmount(void *fs_data) {
    (void)fs_data;
    return 0;
}

static void *proc_alloc_handle(void *fs_data) {
    (void)fs_data;
    for (int i = 0; i < XCMD_VFS_PROC_HANDLE_MAX; i++) {
        if (!file_handle_pool[i].used) {
            file_handle_pool[i].used = 1;
            file_handle_pool[i].entry = NULL;
            return &file_handle_pool[i];
        }
    }
    return NULL;
}

static void proc_free_handle(void *fs_data, void *handle) {
    (void)fs_data;
    proc_file_handle_t *h = (proc_file_handle_t *)handle;
    if (h != NULL) {
        h->used = 0;
        h->entry = NULL;
    }
}

static int proc_open(xcmd_vfs_file_t *file, const char *path, uint8_t mode) {
    (void)mode;
    const char *name = path;
    if (name[0] == '/') {
        name++;
    }

    for (int i = 0; i < proc_count; i++) {
        if (proc_table[i] != NULL && strcmp(proc_table[i]->name, name) == 0) {
            proc_file_handle_t *h = (proc_file_handle_t *)file->handle;
            if (h != NULL) {
                h->entry = proc_table[i];
            }
            return 0;
        }
    }
    return -1;
}

static int proc_close(xcmd_vfs_file_t *file) {
    (void)file;
    return 0;
}

static int proc_read(xcmd_vfs_file_t *file, void *buf, uint32_t len) {
    proc_file_handle_t *h = (proc_file_handle_t *)file->handle;
    if (h == NULL || h->entry == NULL || h->entry->ops == NULL || h->entry->ops->read == NULL) {
        return -1;
    }
    return h->entry->ops->read(h->entry->user_data, buf, len);
}

static int proc_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len) {
    proc_file_handle_t *h = (proc_file_handle_t *)file->handle;
    if (h == NULL || h->entry == NULL || h->entry->ops == NULL || h->entry->ops->write == NULL) {
        return -1;
    }
    return h->entry->ops->write(h->entry->user_data, buf, len);
}

static int proc_stat(void *fs_data, const char *path, xcmd_vfs_stat_t *st) {
    (void)fs_data;
    const char *name = path;
    if (name[0] == '/') {
        name++;
    }

    for (int i = 0; i < proc_count; i++) {
        if (proc_table[i] != NULL && strcmp(proc_table[i]->name, name) == 0) {
            strncpy(st->name, proc_table[i]->name, XCMD_VFS_NAME_MAX - 1);
            st->name[XCMD_VFS_NAME_MAX - 1] = '\0';
            if (proc_table[i]->ops != NULL && proc_table[i]->ops->size != NULL) {
                st->size = proc_table[i]->ops->size(proc_table[i]->user_data);
            } else {
                st->size = 0;
            }
            st->attr = proc_table[i]->attr;
            return 0;
        }
    }
    return -1;
}

static void *proc_alloc_dir_handle(void *fs_data) {
    (void)fs_data;
    for (int i = 0; i < XCMD_VFS_PROC_HANDLE_MAX; i++) {
        if (!dir_handle_pool[i].used) {
            dir_handle_pool[i].used = 1;
            dir_handle_pool[i].index = 0;
            return &dir_handle_pool[i];
        }
    }
    return NULL;
}

static void proc_free_dir_handle(void *fs_data, void *handle) {
    (void)fs_data;
    proc_dir_handle_t *h = (proc_dir_handle_t *)handle;
    if (h != NULL) {
        h->used = 0;
        h->index = 0;
    }
}

static int proc_opendir(xcmd_vfs_dir_t *dir, const char *path) {
    (void)path;
    proc_dir_handle_t *h = (proc_dir_handle_t *)dir->handle;
    if (h != NULL) {
        h->index = 0;
        return 0;
    }
    return -1;
}

static int proc_readdir(xcmd_vfs_dir_t *dir, xcmd_vfs_stat_t *st) {
    proc_dir_handle_t *h = (proc_dir_handle_t *)dir->handle;
    if (h == NULL) {
        return -1;
    }

    while (h->index < proc_count) {
        xcmd_proc_entry_t *entry = proc_table[h->index];
        h->index++;
        if (entry != NULL) {
            strncpy(st->name, entry->name, XCMD_VFS_NAME_MAX - 1);
            st->name[XCMD_VFS_NAME_MAX - 1] = '\0';
            if (entry->ops != NULL && entry->ops->size != NULL) {
                st->size = entry->ops->size(entry->user_data);
            } else {
                st->size = 0;
            }
            st->attr = entry->attr;
            return 0;
        }
    }
    return -1;
}

static int proc_closedir(xcmd_vfs_dir_t *dir) {
    (void)dir;
    return 0;
}

static const xcmd_vfs_ops_t proc_ops = {
    .mount = proc_mount,
    .unmount = proc_unmount,
    .alloc_handle = proc_alloc_handle,
    .free_handle = proc_free_handle,
    .alloc_dir_handle = proc_alloc_dir_handle,
    .free_dir_handle = proc_free_dir_handle,
    .open = proc_open,
    .close = proc_close,
    .read = proc_read,
    .write = proc_write,
    .stat = proc_stat,
    .opendir = proc_opendir,
    .readdir = proc_readdir,
    .closedir = proc_closedir,
};

/* 公开 API */
int xcmd_vfs_proc_mount(xcmd_vfs_t *vfs, const char *mount_point) {
    return xcmd_vfs_mount(vfs, mount_point, &proc_ops, NULL);
}

int xcmd_vfs_proc_register(xcmd_proc_entry_t *entry) {
    if (proc_count >= XCMD_VFS_PROC_MAX) return -1;
    proc_table[proc_count++] = entry;
    return 0;
}

int xcmd_vfs_proc_unregister(const char *name) {
    for (int i = 0; i < proc_count; i++) {
        if (proc_table[i] != NULL && strcmp(proc_table[i]->name, name) == 0) {
            for (int j = i; j < proc_count - 1; j++) {
                proc_table[j] = proc_table[j + 1];
            }
            proc_count--;
            proc_table[proc_count] = NULL;
            return 0;
        }
    }
    return -1;
}

const xcmd_vfs_ops_t* xcmd_vfs_proc_get_ops(void) {
    return &proc_ops;
}
