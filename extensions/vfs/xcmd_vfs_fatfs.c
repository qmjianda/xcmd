#include "xcmd_vfs_fatfs.h"
#include "diskio.h"
#include <string.h>
#include <stdio.h>

static void fatfs_make_path(char *dest, const char *path, const char *drive_path) {
    int drive_len = strlen(drive_path);
    strcpy(dest, drive_path);
    if (path[0] == '/') {
        strcpy(dest + drive_len, path);
    } else {
        dest[drive_len] = '/';
        strcpy(dest + drive_len + 1, path);
    }
}

static void *fatfs_alloc_handle(void *fs_data) {
    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    for (int i = 0; i < XCMD_VFS_MAX_OPEN_FILES; i++) {
        if (!vfs->handles[i].obj.fs) {
            return &vfs->handles[i];
        }
    }
    return NULL;
}

static void fatfs_free_handle(void *fs_data, void *handle) {
    (void)fs_data;
    (void)handle;
}

static void *fatfs_alloc_dir_handle(void *fs_data) {
    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    for (int i = 0; i < XCMD_VFS_MAX_OPEN_FILES; i++) {
        if (vfs->dir_handles[i].sect == 0) {
            return &vfs->dir_handles[i];
        }
    }
    return NULL;
}

static void fatfs_free_dir_handle(void *fs_data, void *handle) {
    (void)fs_data;
    (void)handle;
}

static int fatfs_mount(void *fs_data) {
    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    return f_mount(&vfs->fs, vfs->drive_path, 1) == FR_OK ? 0 : -1;
}

static int fatfs_unmount(void *fs_data) {
    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    return f_mount(NULL, vfs->drive_path, 0) == FR_OK ? 0 : -1;
}

static BYTE fatfs_mode_to_fatfs(uint8_t mode) {
    BYTE res = 0;
    if (mode & XCMD_VFS_MODE_READ) {
        if (mode & XCMD_VFS_MODE_WRITE) {
            res = FA_READ | FA_WRITE;
        } else {
            res = FA_READ;
        }
    } else if (mode & XCMD_VFS_MODE_WRITE) {
        res = FA_WRITE | FA_CREATE_ALWAYS;
    }
    if (mode & XCMD_VFS_MODE_APPEND) {
        res |= FA_OPEN_APPEND;
    }
    if (mode & XCMD_VFS_MODE_CREATE) {
        res |= FA_CREATE_NEW;
    }
    return res;
}

static int fatfs_open(xcmd_vfs_file_t *file, const char *path, uint8_t mode) {
    if (!file || !file->fs_data || !file->handle) return -1;

    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)file->fs_data;
    char fatfs_path[XCMD_VFS_PATH_MAX + 4];
    fatfs_make_path(fatfs_path, path, vfs->drive_path);

    FIL *fp = (FIL *)file->handle;

    FRESULT res = f_open(fp, fatfs_path, fatfs_mode_to_fatfs(mode));
    return (res == FR_OK) ? 0 : -1;
}

static int fatfs_close(xcmd_vfs_file_t *file) {
    if (!file || !file->handle) return -1;
    FIL *fp = (FIL *)file->handle;
    f_close(fp);
    return 0;
}

static int fatfs_read(xcmd_vfs_file_t *file, void *buf, uint32_t len) {
    if (!file || !buf || !file->handle) return -1;
    FIL *fp = (FIL *)file->handle;
    UINT br;
    FRESULT res = f_read(fp, buf, len, &br);
    return (res == FR_OK) ? (int)br : -1;
}

static int fatfs_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len) {
    if (!file || !buf || !file->handle) return -1;
    FIL *fp = (FIL *)file->handle;
    UINT bw;
    FRESULT res = f_write(fp, buf, len, &bw);
    return (res == FR_OK) ? (int)bw : -1;
}

static int fatfs_seek(xcmd_vfs_file_t *file, uint32_t offset) {
    if (!file || !file->handle) return -1;
    FIL *fp = (FIL *)file->handle;
    return f_lseek(fp, offset) == FR_OK ? 0 : -1;
}

static uint32_t fatfs_tell(xcmd_vfs_file_t *file) {
    if (!file || !file->handle) return 0;
    FIL *fp = (FIL *)file->handle;
    return f_tell(fp);
}

static int fatfs_stat(void *fs_data, const char *path, xcmd_vfs_stat_t *st) {
    if (!fs_data || !path || !st) return -1;

    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    char fatfs_path[XCMD_VFS_PATH_MAX + 4];
    fatfs_make_path(fatfs_path, path, vfs->drive_path);

    FILINFO fno;
    FRESULT res = f_stat(fatfs_path, &fno);
    if (res != FR_OK) return -1;

    strncpy(st->name, fno.fname, XCMD_VFS_NAME_MAX - 1);
    st->name[XCMD_VFS_NAME_MAX - 1] = '\0';
    st->size = fno.fsize;
    st->attr = 0;
    if (fno.fattrib & AM_DIR) st->attr |= XCMD_VFS_ATTR_DIRECTORY;
    if (fno.fattrib & AM_RDO) st->attr |= XCMD_VFS_ATTR_READONLY;

    return 0;
}

static int fatfs_mkdir(void *fs_data, const char *path) {
    if (!fs_data || !path) return -1;

    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    char fatfs_path[XCMD_VFS_PATH_MAX + 4];
    fatfs_make_path(fatfs_path, path, vfs->drive_path);

    return f_mkdir(fatfs_path) == FR_OK ? 0 : -1;
}

static int fatfs_rm(void *fs_data, const char *path) {
    if (!fs_data || !path) return -1;

    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    char fatfs_path[XCMD_VFS_PATH_MAX + 4];
    fatfs_make_path(fatfs_path, path, vfs->drive_path);

    return f_unlink(fatfs_path) == FR_OK ? 0 : -1;
}

static int fatfs_rename(void *fs_data, const char *old_path, const char *new_path) {
    if (!fs_data || !old_path || !new_path) return -1;

    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    char fatfs_old_path[XCMD_VFS_PATH_MAX + 4];
    char fatfs_new_path[XCMD_VFS_PATH_MAX + 4];
    fatfs_make_path(fatfs_old_path, old_path, vfs->drive_path);
    fatfs_make_path(fatfs_new_path, new_path, vfs->drive_path);

    return f_rename(fatfs_old_path, fatfs_new_path) == FR_OK ? 0 : -1;
}

static int fatfs_opendir(xcmd_vfs_dir_t *dir, const char *path) {
    if (!dir || !dir->fs_data || !dir->handle) return -1;

    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)dir->fs_data;
    char fatfs_path[XCMD_VFS_PATH_MAX + 4];
    fatfs_make_path(fatfs_path, path, vfs->drive_path);

    DIR *dp = (DIR *)dir->handle;
    return f_opendir(dp, fatfs_path) == FR_OK ? 0 : -1;
}

static int fatfs_readdir(xcmd_vfs_dir_t *dir, xcmd_vfs_stat_t *st) {
    if (!dir || !st || !dir->handle) return -1;

    DIR *dp = (DIR *)dir->handle;
    FILINFO fno;
    FRESULT res = f_readdir(dp, &fno);
    if (res != FR_OK || fno.fname[0] == 0) return -1;

    strncpy(st->name, fno.fname, XCMD_VFS_NAME_MAX - 1);
    st->name[XCMD_VFS_NAME_MAX - 1] = '\0';
    st->size = fno.fsize;
    st->attr = 0;
    if (fno.fattrib & AM_DIR) st->attr |= XCMD_VFS_ATTR_DIRECTORY;
    if (fno.fattrib & AM_RDO) st->attr |= XCMD_VFS_ATTR_READONLY;

    return 0;
}

static int fatfs_closedir(xcmd_vfs_dir_t *dir) {
    if (!dir || !dir->handle) return -1;
    DIR *dp = (DIR *)dir->handle;
    return f_closedir(dp) == FR_OK ? 0 : -1;
}

static uint32_t fatfs_getfree(void *fs_data) {
    if (!fs_data) return 0;
    xcmd_vfs_fatfs_t *vfs = (xcmd_vfs_fatfs_t *)fs_data;
    FATFS *fs;
    DWORD fre_clust;

    if (f_getfree(vfs->drive_path, &fre_clust, &fs) != FR_OK) return 0;
    return (uint32_t)(fre_clust * fs->csize * 512);
}

const xcmd_vfs_ops_t xcmd_vfs_fatfs_ops = {
    fatfs_mount,
    fatfs_unmount,
    fatfs_alloc_handle,
    fatfs_free_handle,
    fatfs_alloc_dir_handle,
    fatfs_free_dir_handle,
    fatfs_open,
    fatfs_close,
    fatfs_read,
    fatfs_write,
    fatfs_seek,
    fatfs_tell,
    fatfs_stat,
    fatfs_mkdir,
    fatfs_rm,
    fatfs_rename,
    fatfs_opendir,
    fatfs_readdir,
    fatfs_closedir,
    fatfs_getfree,
};

int xcmd_vfs_fatfs_init(xcmd_vfs_fatfs_t *vfs, const char *drive_path) {
    if (!vfs || !drive_path) return -1;
    memset(vfs, 0, sizeof(xcmd_vfs_fatfs_t));
    vfs->drive_path = drive_path;
    return 0;
}
