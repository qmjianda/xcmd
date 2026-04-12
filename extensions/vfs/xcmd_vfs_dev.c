#include "xcmd_vfs_dev.h"
#include <string.h>

#ifndef XCMD_VFS_DEV_MAX
#define XCMD_VFS_DEV_MAX  16
#endif

static xcmd_dev_t *dev_table[XCMD_VFS_DEV_MAX];
static int dev_count = 0;

#ifndef XCMD_VFS_DEV_MAX_HANDLES
#define XCMD_VFS_DEV_MAX_HANDLES  4
#endif

typedef struct {
    xcmd_dev_t *dev;
    uint8_t mode;
    uint8_t in_use;
} dev_file_handle_t;

static dev_file_handle_t dev_handles[XCMD_VFS_DEV_MAX_HANDLES];

static int dev_mount(void *fs_data) {
    (void)fs_data;
    return 0;
}

static int dev_unmount(void *fs_data) {
    (void)fs_data;
    return 0;
}

static void *dev_alloc_handle(void *fs_data) {
    (void)fs_data;
    for (int i = 0; i < XCMD_VFS_DEV_MAX_HANDLES; i++) {
        if (!dev_handles[i].in_use) {
            dev_handles[i].in_use = 1;
            dev_handles[i].dev = NULL;
            dev_handles[i].mode = 0;
            return &dev_handles[i];
        }
    }
    return NULL;
}

static void dev_free_handle(void *fs_data, void *handle) {
    (void)fs_data;
    if (handle) {
        dev_file_handle_t *h = (dev_file_handle_t *)handle;
        h->in_use = 0;
        h->dev = NULL;
        h->mode = 0;
    }
}

static xcmd_dev_t* dev_find(const char *name) {
    for (int i = 0; i < dev_count; i++) {
        if (dev_table[i] && strcmp(dev_table[i]->name, name) == 0) {
            return dev_table[i];
        }
    }
    return NULL;
}

static int dev_open(xcmd_vfs_file_t *file, const char *path, uint8_t mode) {
    if (!file || !file->handle) return -1;

    const char *dev_name = path;
    while (*dev_name == '/') dev_name++;

    if (*dev_name == '\0') {
        return 0;
    }

    xcmd_dev_t *dev = dev_find(dev_name);
    if (!dev || !dev->ops) return -1;

    dev_file_handle_t *h = (dev_file_handle_t *)file->handle;

    if (dev->ops->open) {
        int ret = dev->ops->open(dev->dev_data, mode);
        if (ret != 0) return -1;
    }

    h->dev = dev;
    h->mode = mode;
    return 0;
}

static int dev_close(xcmd_vfs_file_t *file) {
    if (!file || !file->handle) return -1;

    dev_file_handle_t *h = (dev_file_handle_t *)file->handle;
    if (h->dev && h->dev->ops && h->dev->ops->close) {
        h->dev->ops->close(h->dev->dev_data);
    }

    h->dev = NULL;
    h->mode = 0;
    return 0;
}

static int dev_read(xcmd_vfs_file_t *file, void *buf, uint32_t len) {
    if (!file || !buf || !file->handle) return -1;

    dev_file_handle_t *h = (dev_file_handle_t *)file->handle;
    if (!h->dev || !h->dev->ops || !h->dev->ops->read) return -1;

    return h->dev->ops->read(h->dev->dev_data, buf, len);
}

static int dev_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len) {
    if (!file || !buf || !file->handle) return -1;

    dev_file_handle_t *h = (dev_file_handle_t *)file->handle;
    if (!h->dev || !h->dev->ops || !h->dev->ops->write) return -1;

    return h->dev->ops->write(h->dev->dev_data, buf, len);
}

static int dev_stat(void *fs_data, const char *path, xcmd_vfs_stat_t *st) {
    (void)fs_data;
    if (!path || !st) return -1;

    const char *dev_name = path;
    while (*dev_name == '/') dev_name++;

    memset(st, 0, sizeof(xcmd_vfs_stat_t));

    if (*dev_name == '\0') {
        st->attr = XCMD_VFS_ATTR_DIRECTORY;
        strncpy(st->name, ".", XCMD_VFS_NAME_MAX - 1);
        return 0;
    }

    xcmd_dev_t *dev = dev_find(dev_name);
    if (!dev) return -1;

    strncpy(st->name, dev->name, XCMD_VFS_NAME_MAX - 1);
    st->size = 0;
    st->attr = 0;
    return 0;
}

typedef struct {
    int index;
} dev_dir_handle_t;

static dev_dir_handle_t dev_dir_handles[XCMD_VFS_DEV_MAX_HANDLES];

static void *dev_alloc_dir_handle(void *fs_data) {
    (void)fs_data;
    for (int i = 0; i < XCMD_VFS_DEV_MAX_HANDLES; i++) {
        if (dev_dir_handles[i].index < 0) {
            dev_dir_handles[i].index = 0;
            return &dev_dir_handles[i];
        }
    }
    return NULL;
}

static void dev_free_dir_handle(void *fs_data, void *handle) {
    (void)fs_data;
    if (handle) {
        dev_dir_handle_t *h = (dev_dir_handle_t *)handle;
        h->index = -1;
    }
}

static int dev_opendir(xcmd_vfs_dir_t *dir, const char *path) {
    (void)path;
    if (!dir || !dir->handle) return -1;

    dev_dir_handle_t *h = (dev_dir_handle_t *)dir->handle;
    h->index = 0;
    return 0;
}

static int dev_readdir(xcmd_vfs_dir_t *dir, xcmd_vfs_stat_t *st) {
    if (!dir || !st || !dir->handle) return -1;

    dev_dir_handle_t *h = (dev_dir_handle_t *)dir->handle;

    if (h->index >= dev_count) {
        return -1;
    }

    xcmd_dev_t *dev = dev_table[h->index];
    if (dev) {
        strncpy(st->name, dev->name, XCMD_VFS_NAME_MAX - 1);
        st->name[XCMD_VFS_NAME_MAX - 1] = '\0';
        st->size = 0;
        st->attr = 0;
    }

    h->index++;
    return 0;
}

static int dev_closedir(xcmd_vfs_dir_t *dir) {
    if (!dir || !dir->handle) return -1;

    dev_dir_handle_t *h = (dev_dir_handle_t *)dir->handle;
    h->index = -1;
    return 0;
}

static const xcmd_vfs_ops_t dev_ops = {
    .mount = dev_mount,
    .unmount = dev_unmount,
    .alloc_handle = dev_alloc_handle,
    .free_handle = dev_free_handle,
    .alloc_dir_handle = dev_alloc_dir_handle,
    .free_dir_handle = dev_free_dir_handle,
    .open = dev_open,
    .close = dev_close,
    .read = dev_read,
    .write = dev_write,
    .seek = NULL,
    .tell = NULL,
    .stat = dev_stat,
    .mkdir = NULL,
    .rm = NULL,
    .rename = NULL,
    .opendir = dev_opendir,
    .readdir = dev_readdir,
    .closedir = dev_closedir,
    .getfree = NULL,
};

int xcmd_vfs_dev_mount(xcmd_vfs_t *vfs, const char *mount_point) {
    return xcmd_vfs_mount(vfs, mount_point, &dev_ops, NULL);
}

int xcmd_vfs_dev_register(xcmd_dev_t *dev) {
    if (dev_count >= XCMD_VFS_DEV_MAX) return -1;
    dev_table[dev_count++] = dev;
    return 0;
}

int xcmd_vfs_dev_unregister(const char *name) {
    if (!name) return -1;

    for (int i = 0; i < dev_count; i++) {
        if (dev_table[i] && strcmp(dev_table[i]->name, name) == 0) {
            for (int j = i; j < dev_count - 1; j++) {
                dev_table[j] = dev_table[j + 1];
            }
            dev_count--;
            dev_table[dev_count] = NULL;
            return 0;
        }
    }
    return -1;
}

const xcmd_vfs_ops_t* xcmd_vfs_dev_get_ops(void) {
    return &dev_ops;
}

static int null_open(void *dev_data, uint8_t mode) {
    (void)dev_data;
    (void)mode;
    return 0;
}

static int null_close(void *dev_data) {
    (void)dev_data;
    return 0;
}

static int null_read(void *dev_data, void *buf, uint32_t len) {
    (void)dev_data;
    (void)buf;
    (void)len;
    return 0;
}

static int null_write(void *dev_data, const void *buf, uint32_t len) {
    (void)dev_data;
    (void)buf;
    return (int)len;
}

static const xcmd_dev_ops_t null_ops = {
    .open = null_open,
    .close = null_close,
    .read = null_read,
    .write = null_write,
    .ioctl = NULL,
};

const xcmd_dev_t xcmd_dev_null = {
    .name = "null",
    .ops = &null_ops,
    .dev_data = NULL,
};

static int zero_read(void *dev_data, void *buf, uint32_t len) {
    (void)dev_data;
    memset(buf, 0, len);
    return (int)len;
}

static const xcmd_dev_ops_t zero_ops = {
    .open = null_open,
    .close = null_close,
    .read = zero_read,
    .write = null_write,
    .ioctl = NULL,
};

const xcmd_dev_t xcmd_dev_zero = {
    .name = "zero",
    .ops = &zero_ops,
    .dev_data = NULL,
};
