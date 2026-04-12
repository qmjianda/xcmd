/**
 * @file xcmd_vfs.h
 * @brief Virtual File System - unified interface for file operations
 * 
 * Design goals:
 * - Zero dynamic memory allocation
 * - Support multiple filesystem backends (FatFS, RAM, etc.)
 * - Standard POSIX-like API abstraction
 */
#ifndef XCMD_VFS_H
#define XCMD_VFS_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================
 * Configuration
 *============================================================================*/

#ifndef XCMD_VFS_MAX_MOUNTS
#define XCMD_VFS_MAX_MOUNTS      4
#endif

#ifndef XCMD_VFS_MAX_OPEN_FILES
#define XCMD_VFS_MAX_OPEN_FILES  8
#endif

#ifndef XCMD_VFS_PATH_MAX
#define XCMD_VFS_PATH_MAX        128
#endif

#ifndef XCMD_VFS_NAME_MAX
#define XCMD_VFS_NAME_MAX        64
#endif

/*============================================================================
 * Types
 *============================================================================*/

typedef struct xcmd_vfs xcmd_vfs_t;
typedef struct xcmd_vfs_file xcmd_vfs_file_t;
typedef struct xcmd_vfs_ops xcmd_vfs_ops_t;

/* File modes */
#define XCMD_VFS_MODE_READ        (1 << 0)
#define XCMD_VFS_MODE_WRITE       (1 << 1)
#define XCMD_VFS_MODE_APPEND      (1 << 2)
#define XCMD_VFS_MODE_CREATE      (1 << 3)

/* File attributes */
#define XCMD_VFS_ATTR_DIRECTORY   (1 << 0)
#define XCMD_VFS_ATTR_READONLY   (1 << 1)
#define XCMD_VFS_ATTR_HIDDEN     (1 << 2)
#define XCMD_VFS_ATTR_SYSTEM     (1 << 3)

typedef struct {
    char name[XCMD_VFS_NAME_MAX];
    uint32_t size;
    uint16_t attr;
    uint16_t flags;
    void *backend_data;
} xcmd_vfs_stat_t;

typedef struct {
    char path[XCMD_VFS_PATH_MAX];
    void *fs_data;
    void *handle;
    struct xcmd_vfs *vfs;
    uint8_t at_root;
    uint8_t root_mnt_index;
} xcmd_vfs_dir_t;

/* Backend operations */
struct xcmd_vfs_ops {
    int (*mount)(void *fs_data);
    int (*unmount)(void *fs_data);

    void *(*alloc_handle)(void *fs_data);
    void (*free_handle)(void *fs_data, void *handle);

    void *(*alloc_dir_handle)(void *fs_data);
    void (*free_dir_handle)(void *fs_data, void *handle);

    int (*open)(xcmd_vfs_file_t *file, const char *path, uint8_t mode);
    int (*close)(xcmd_vfs_file_t *file);
    int (*read)(xcmd_vfs_file_t *file, void *buf, uint32_t len);
    int (*write)(xcmd_vfs_file_t *file, const void *buf, uint32_t len);
    int (*seek)(xcmd_vfs_file_t *file, uint32_t offset);
    uint32_t (*tell)(xcmd_vfs_file_t *file);

    int (*stat)(void *fs_data, const char *path, xcmd_vfs_stat_t *st);
    int (*mkdir)(void *fs_data, const char *path);
    int (*rm)(void *fs_data, const char *path);
    int (*rename)(void *fs_data, const char *old_path, const char *new_path);

    int (*opendir)(xcmd_vfs_dir_t *dir, const char *path);
    int (*readdir)(xcmd_vfs_dir_t *dir, xcmd_vfs_stat_t *st);
    int (*closedir)(xcmd_vfs_dir_t *dir);

    uint32_t (*getfree)(void *fs_data);
};

/* File handle */
struct xcmd_vfs_file {
    const xcmd_vfs_ops_t *ops;
    void *fs_data;
    void *handle;
    char path[XCMD_VFS_PATH_MAX];
    uint8_t mode;
    uint8_t is_open;
};

/* Mount point */
typedef struct {
    char mount_point[XCMD_VFS_NAME_MAX];
    const xcmd_vfs_ops_t *ops;
    void *fs_data;
    uint8_t mounted;
} xcmd_vfs_mount_t;

/* VFS core */
struct xcmd_vfs {
    xcmd_vfs_mount_t mounts[XCMD_VFS_MAX_MOUNTS];
    uint8_t mount_count;
    
    xcmd_vfs_file_t files[XCMD_VFS_MAX_OPEN_FILES];
    
    char cwd[XCMD_VFS_PATH_MAX];
};

/*============================================================================
 * Core API
 *============================================================================*/

void xcmd_vfs_init(xcmd_vfs_t *vfs);
int xcmd_vfs_mount(xcmd_vfs_t *vfs, const char *mount_point, 
                   const xcmd_vfs_ops_t *ops, void *backend_data);
int xcmd_vfs_umount(xcmd_vfs_t *vfs, const char *mount_point);

/* File operations */
xcmd_vfs_file_t* xcmd_vfs_open(xcmd_vfs_t *vfs, const char *path, uint8_t mode);
int xcmd_vfs_close(xcmd_vfs_file_t *file);
int xcmd_vfs_read(xcmd_vfs_file_t *file, void *buf, uint32_t len);
int xcmd_vfs_write(xcmd_vfs_file_t *file, const void *buf, uint32_t len);
int xcmd_vfs_seek(xcmd_vfs_file_t *file, uint32_t offset);
uint32_t xcmd_vfs_tell(xcmd_vfs_file_t *file);

/* Directory operations */
int xcmd_vfs_stat(xcmd_vfs_t *vfs, const char *path, xcmd_vfs_stat_t *st);
int xcmd_vfs_mkdir(xcmd_vfs_t *vfs, const char *path);
int xcmd_vfs_rm(xcmd_vfs_t *vfs, const char *path);
int xcmd_vfs_rename(xcmd_vfs_t *vfs, const char *old_path, const char *new_path);

/* Directory iteration */
int xcmd_vfs_opendir(xcmd_vfs_dir_t *dir, xcmd_vfs_t *vfs, const char *path);
int xcmd_vfs_readdir(xcmd_vfs_dir_t *dir, xcmd_vfs_stat_t *st);
int xcmd_vfs_closedir(xcmd_vfs_dir_t *dir);

/* Utility */
uint32_t xcmd_vfs_getfree(xcmd_vfs_t *vfs, const char *path);
int xcmd_vfs_chdir(xcmd_vfs_t *vfs, const char *path);
const char* xcmd_vfs_getcwd(xcmd_vfs_t *vfs);

/*============================================================================
 * Path Utilities
 *============================================================================*/

int xcmd_vfs_parse_path(const char *path, char *mount_point, char *relative_path);
int xcmd_vfs_join_path(char *dest, const char *base, const char *追加);

#ifdef __cplusplus
}
#endif

#endif /* XCMD_VFS_H */
