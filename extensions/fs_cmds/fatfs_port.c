/*
 * @Author: your name
 * @Date: 2021-10-09 13:55:09
 * @LastEditTime: 2021-10-09 17:04:56
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/linux/fatfs_port.c
 */
#include "fatfs_port.h"

#include <stdio.h>

static DiskIo_t* g_Disks[FF_VOLUMES] = {0};

#if FF_STR_VOLUME_ID >= 1
#ifndef FF_VOLUME_STRS
const char* VolumeStr[FF_VOLUMES] = {0};
#else
static char* VolumeStr[FF_VOLUMES] = {FF_VOLUME_STRS};
#endif
#endif

int f_disk_regist(DiskIo_t* disk, const char* volume_name, int id) {
    if (!disk || !volume_name) {
        return -1;
    }
    if (id < 0) {
        for (int i = 0; i < FF_VOLUMES; i++) {
            if (!g_Disks[i]) {
                g_Disks[i] = disk;
                id = i;
                goto ok;
            }
        }
    } else {
        if ((id < FF_VOLUMES) && !g_Disks[id]) {
            g_Disks[id] = disk;
            goto ok;
        }
    }
    return -1;

ok:
#if FF_STR_VOLUME_ID >= 1

#ifndef FF_VOLUME_STRS

#if FF_STR_VOLUME_ID == 1
    snprintf(disk->disk_path, MAX_VOL_NAME_LEN, "%s:", volume_name);
#endif
#if FF_STR_VOLUME_ID == 2
    snprintf(disk->disk_path, MAX_VOL_NAME_LEN, "/%s/", volume_name);
#endif
    VolumeStr[id] = volume_name;

#else

#if FF_STR_VOLUME_ID == 1
    snprintf(disk->disk_path, MAX_VOL_NAME_LEN, "%s:", VolumeStr[id]);
#endif
#if FF_STR_VOLUME_ID == 2
    snprintf(disk->disk_path, MAX_VOL_NAME_LEN, "/%s/", VolumeStr[id]);
#endif

#endif

#else
    snprintf(disk->disk_path, MAX_VOL_NAME_LEN, "%d:", id);
#endif
    return id;
}

DiskIo_t* f_disk_get(int id) {
    if (id < FF_VOLUMES) {
        return g_Disks[id];
    }
    return NULL;
}

char* f_disk_path(int id) {
    if (id < FF_VOLUMES) {
        return g_Disks[id]->disk_path;
    }
    return NULL;
}