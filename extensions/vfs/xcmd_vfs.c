#include "xcmd_vfs.h"
#include <string.h>
#include <stdio.h>

void xcmd_vfs_init(xcmd_vfs_t *vfs) {
    if (!vfs) return;
    memset(vfs, 0, sizeof(xcmd_vfs_t));
    strcpy(vfs->cwd, "/");
}

int xcmd_vfs_mount(xcmd_vfs_t *vfs, const char *mount_point,
                   const xcmd_vfs_ops_t *ops, void *fs_data, uint8_t flags) {
    if (!vfs || !mount_point || !ops) {
        return -1;
    }

    xcmd_vfs_mount_t *existing = NULL;
    for (int i = 0; i < XCMD_VFS_MAX_MOUNTS; i++) {
        if (vfs->mounts[i].mounted &&
            strcmp(vfs->mounts[i].mount_point, mount_point) == 0) {
            existing = &vfs->mounts[i];
            break;
        }
    }

    if (existing) {
        if (flags & XCMD_MOUNT_FLAG_REMOUNT) {
            if (existing->ops->unmount) {
                existing->ops->unmount(existing->fs_data);
            }
        } else {
            return -1;
        }
    } else {
        if (strcmp(mount_point, "/") != 0) {
            xcmd_vfs_stat_t st;
            if (xcmd_vfs_stat(vfs, mount_point, &st) != 0) {
                if (xcmd_vfs_mkdir(vfs, mount_point) != 0) {
                    return -1;
                }
            }
        }
    }

    xcmd_vfs_mount_t *mnt = NULL;
    if (existing) {
        mnt = existing;
    } else {
        for (int i = 0; i < XCMD_VFS_MAX_MOUNTS; i++) {
            if (!vfs->mounts[i].mounted) {
                mnt = &vfs->mounts[i];
                break;
            }
        }
    }

    if (!mnt) {
        return -1;
    }

    strncpy(mnt->mount_point, mount_point, XCMD_VFS_NAME_MAX - 1);
    mnt->mount_point[XCMD_VFS_NAME_MAX - 1] = '\0';
    mnt->ops = ops;
    mnt->fs_data = fs_data;
    mnt->mounted = 1;
    mnt->flags = flags;

    if (ops->mount && ops->mount(fs_data) != 0) {
        mnt->mounted = 0;
        return -1;
    }

    if (!existing) {
        vfs->mount_count++;
    }
    return 0;
}

int xcmd_vfs_umount(xcmd_vfs_t *vfs, const char *mount_point, uint8_t flags) {
    (void)flags;
    if (!vfs || !mount_point) return -1;

    for (int i = 0; i < XCMD_VFS_MAX_MOUNTS; i++) {
        if (vfs->mounts[i].mounted &&
            strcmp(vfs->mounts[i].mount_point, mount_point) == 0) {
            if (vfs->mounts[i].ops->unmount) {
                vfs->mounts[i].ops->unmount(vfs->mounts[i].fs_data);
            }
            vfs->mounts[i].mounted = 0;
            vfs->mounts[i].flags = 0;
            vfs->mount_count--;
            return 0;
        }
    }
    return -1;
}

static void vfs_resolve_path(xcmd_vfs_t *vfs, char *dest, const char *path, size_t dest_size) {
    if (!vfs || !dest || !path || dest_size == 0) return;

    char temp[XCMD_VFS_PATH_MAX];

    if (path[0] == '/') {
        strncpy(temp, path, XCMD_VFS_PATH_MAX - 1);
        temp[XCMD_VFS_PATH_MAX - 1] = '\0';
    } else {
        const char *cwd = vfs->cwd;
        int cwd_len = strlen(cwd);
        int path_len = strlen(path);

        if (cwd_len > 1 && cwd[cwd_len - 1] == '/') {
            cwd_len--;
        }

        if (cwd_len + 1 + path_len < XCMD_VFS_PATH_MAX) {
            memcpy(temp, cwd, cwd_len);
            temp[cwd_len] = '/';
            memcpy(temp + cwd_len + 1, path, path_len);
            temp[cwd_len + 1 + path_len] = '\0';
        } else {
            strncpy(temp, path, XCMD_VFS_PATH_MAX - 1);
            temp[XCMD_VFS_PATH_MAX - 1] = '\0';
        }
    }

    char *components[32];
    int comp_count = 0;
    char *p = temp + 1;

    while (*p && comp_count < 32) {
        char *slash = strchr(p, '/');
        int len;
        if (slash) {
            len = slash - p;
            *slash = '\0';
        } else {
            len = strlen(p);
        }

        if (len > 0) {
            if (len == 1 && p[0] == '.') {
            } else if (len == 2 && p[0] == '.' && p[1] == '.') {
                if (comp_count > 0) {
                    comp_count--;
                }
            } else {
                components[comp_count++] = p;
            }
        }

        if (slash) {
            p = slash + 1;
        } else {
            break;
        }
    }

    dest[0] = '/';
    int pos = 1;
    for (int i = 0; i < comp_count && pos < (int)dest_size - 2; i++) {
        int len = strlen(components[i]);
        if (pos + len + 1 < (int)dest_size) {
            if (pos > 1) {
                dest[pos++] = '/';
            }
            memcpy(dest + pos, components[i], len);
            pos += len;
        }
    }
    dest[pos] = '\0';

    if (pos == 0) {
        dest[0] = '/';
        dest[1] = '\0';
    }
}

static xcmd_vfs_mount_t* vfs_find_mount(xcmd_vfs_t *vfs, const char *path) {
    if (!vfs || !path) return NULL;

    xcmd_vfs_mount_t *best = NULL;
    int best_len = 0;

    for (int i = 0; i < XCMD_VFS_MAX_MOUNTS; i++) {
        if (vfs->mounts[i].mounted) {
            int mp_len = strlen(vfs->mounts[i].mount_point);
            if (mp_len > best_len && strncmp(path, vfs->mounts[i].mount_point, mp_len) == 0) {
                best = &vfs->mounts[i];
                best_len = mp_len;
            }
        }
    }
    return best;
}

static void vfs_make_relative_path(char *dest, const char *mount_point, const char *full_path) {
    int mp_len = strlen(mount_point);

    if (mp_len == 1 && mount_point[0] == '/') {
        strcpy(dest, full_path);
        return;
    }

    if (full_path[mp_len] == '/') {
        strcpy(dest, full_path + mp_len);
    } else {
        strcpy(dest, full_path + mp_len - 1);
    }

    if (dest[0] == '\0') {
        strcpy(dest, "/");
    }
}

xcmd_vfs_file_t* xcmd_vfs_open(xcmd_vfs_t *vfs, const char *path, uint8_t mode) {
    if (!vfs || !path) return NULL;

    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt || !mnt->ops->open) return NULL;

    for (int i = 0; i < XCMD_VFS_MAX_OPEN_FILES; i++) {
        if (!vfs->files[i].is_open) {
            xcmd_vfs_file_t *file = &vfs->files[i];
            char rel_path[XCMD_VFS_PATH_MAX];
            vfs_make_relative_path(rel_path, mnt->mount_point, abs_path);

            file->fs_data = mnt->fs_data;
            file->handle = mnt->ops->alloc_handle(mnt->fs_data);
            if (!file->handle) {
                return NULL;
            }

            if (mnt->ops->open(file, rel_path, mode) != 0) {
                mnt->ops->free_handle(mnt->fs_data, file->handle);
                file->fs_data = NULL;
                file->handle = NULL;
                return NULL;
            }

            file->ops = mnt->ops;
            file->mode = mode;
            file->is_open = 1;
            strncpy(file->path, abs_path, XCMD_VFS_PATH_MAX - 1);
            return file;
        }
    }
    return NULL;
}

int xcmd_vfs_close(xcmd_vfs_file_t *file) {
    if (!file || !file->is_open) return -1;
    int ret = file->ops->close(file);
    file->ops->free_handle(file->fs_data, file->handle);
    file->is_open = 0;
    file->handle = NULL;
    return ret;
}

int xcmd_vfs_read(xcmd_vfs_file_t *file, void *buf, uint32_t len) {
    if (!file || !file->is_open || !buf) return -1;
    return file->ops->read(file, buf, len);
}

int xcmd_vfs_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len) {
    if (!file || !file->is_open || !buf) return -1;
    return file->ops->write(file, buf, len);
}

int xcmd_vfs_seek(xcmd_vfs_file_t *file, uint32_t offset) {
    if (!file || !file->is_open) return -1;
    return file->ops->seek(file, offset);
}

uint32_t xcmd_vfs_tell(xcmd_vfs_file_t *file) {
    if (!file || !file->is_open) return 0;
    return file->ops->tell(file);
}

int xcmd_vfs_stat(xcmd_vfs_t *vfs, const char *path, xcmd_vfs_stat_t *st) {
    if (!vfs || !path || !st) return -1;

    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt || !mnt->ops->stat) return -1;

    char rel_path[XCMD_VFS_PATH_MAX];
    vfs_make_relative_path(rel_path, mnt->mount_point, abs_path);

    return mnt->ops->stat(mnt->fs_data, rel_path, st);
}

int xcmd_vfs_mkdir(xcmd_vfs_t *vfs, const char *path) {
    if (!vfs || !path) return -1;

    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt || !mnt->ops->mkdir) return -1;

    char rel_path[XCMD_VFS_PATH_MAX];
    vfs_make_relative_path(rel_path, mnt->mount_point, abs_path);

    return mnt->ops->mkdir(mnt->fs_data, rel_path);
}

int xcmd_vfs_rm(xcmd_vfs_t *vfs, const char *path) {
    if (!vfs || !path) return -1;

    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt || !mnt->ops->rm) return -1;

    char rel_path[XCMD_VFS_PATH_MAX];
    vfs_make_relative_path(rel_path, mnt->mount_point, abs_path);

    return mnt->ops->rm(mnt->fs_data, rel_path);
}

int xcmd_vfs_rename(xcmd_vfs_t *vfs, const char *old_path, const char *new_path) {
    if (!vfs || !old_path || !new_path) return -1;

    char abs_old[XCMD_VFS_PATH_MAX];
    char abs_new[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_old, old_path, XCMD_VFS_PATH_MAX);
    vfs_resolve_path(vfs, abs_new, new_path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt_old = vfs_find_mount(vfs, abs_old);
    xcmd_vfs_mount_t *mnt_new = vfs_find_mount(vfs, abs_new);

    if (!mnt_old || !mnt_new || mnt_old != mnt_new || !mnt_old->ops->rename) {
        return -1;
    }

    char rel_old[XCMD_VFS_PATH_MAX];
    char rel_new[XCMD_VFS_PATH_MAX];
    vfs_make_relative_path(rel_old, mnt_old->mount_point, abs_old);
    vfs_make_relative_path(rel_new, mnt_new->mount_point, abs_new);

    return mnt_old->ops->rename(mnt_old->fs_data, rel_old, rel_new);
}

int xcmd_vfs_opendir(xcmd_vfs_dir_t *dir, xcmd_vfs_t *vfs, const char *path) {
    if (!dir || !vfs || !path) return -1;

    memset(dir, 0, sizeof(xcmd_vfs_dir_t));
    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt || !mnt->ops->opendir) return -1;

    char rel_path[XCMD_VFS_PATH_MAX];
    vfs_make_relative_path(rel_path, mnt->mount_point, abs_path);

    dir->fs_data = mnt->fs_data;
    dir->handle = mnt->ops->alloc_dir_handle(mnt->fs_data);
    if (!dir->handle) {
        return -1;
    }
    dir->vfs = vfs;
    strncpy(dir->path, abs_path, XCMD_VFS_PATH_MAX - 1);
    dir->path[XCMD_VFS_PATH_MAX - 1] = '\0';

    dir->at_root = (strcmp(abs_path, "/") == 0);
    dir->root_mnt_index = 0;

    if (mnt->ops->opendir(dir, rel_path) != 0) {
        mnt->ops->free_dir_handle(mnt->fs_data, dir->handle);
        dir->handle = NULL;
        return -1;
    }

    return 0;
}

int xcmd_vfs_readdir(xcmd_vfs_dir_t *dir, xcmd_vfs_stat_t *st) {
    if (!dir || !st) return -1;
    
    xcmd_vfs_mount_t *mnt = vfs_find_mount(dir->vfs, dir->path);
    if (!mnt || !mnt->ops->readdir) return -1;
    
    if (dir->at_root) {
        int ret = mnt->ops->readdir(dir, st);
        if (ret == 0) {
            return 0;
        }
        
        xcmd_vfs_t *vfs = dir->vfs;
        for (int i = dir->root_mnt_index + 1; i < vfs->mount_count; i++) {
            xcmd_vfs_mount_t *m = &vfs->mounts[i];
            if (m->mounted && strcmp(m->mount_point, "/") != 0) {
                const char *name = m->mount_point;
                while (*name == '/') name++;
                strncpy(st->name, name, XCMD_VFS_NAME_MAX - 1);
                st->name[XCMD_VFS_NAME_MAX - 1] = '\0';
                st->size = 0;
                st->attr = XCMD_VFS_ATTR_DIRECTORY;
                dir->root_mnt_index = i;
                return 0;
            }
        }
        return -1;
    }
    
    return mnt->ops->readdir(dir, st);
}

int xcmd_vfs_closedir(xcmd_vfs_dir_t *dir) {
    if (!dir) return -1;

    xcmd_vfs_mount_t *mnt = vfs_find_mount(dir->vfs, dir->path);
    if (!mnt || !mnt->ops->closedir) return -1;

    int ret = mnt->ops->closedir(dir);
    mnt->ops->free_dir_handle(mnt->fs_data, dir->handle);
    dir->handle = NULL;

    return ret;
}

uint32_t xcmd_vfs_getfree(xcmd_vfs_t *vfs, const char *path) {
    if (!vfs || !path) return 0;

    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt || !mnt->ops->getfree) return 0;

    return mnt->ops->getfree(mnt->fs_data);
}

int xcmd_vfs_chdir(xcmd_vfs_t *vfs, const char *path) {
    if (!vfs || !path) return -1;

    char abs_path[XCMD_VFS_PATH_MAX];
    vfs_resolve_path(vfs, abs_path, path, XCMD_VFS_PATH_MAX);

    xcmd_vfs_mount_t *mnt = vfs_find_mount(vfs, abs_path);
    if (!mnt) return -1;

    strncpy(vfs->cwd, abs_path, XCMD_VFS_PATH_MAX - 1);
    vfs->cwd[XCMD_VFS_PATH_MAX - 1] = '\0';
    return 0;
}

const char* xcmd_vfs_getcwd(xcmd_vfs_t *vfs) {
    return vfs ? vfs->cwd : "/";
}

int xcmd_vfs_parse_path(const char *path, char *mount_point, char *relative_path) {
    return 0;
}
