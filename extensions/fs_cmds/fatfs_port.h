#ifndef FATFS_PORT_H
#define FATFS_PORT_H

#include "ff.h"
#include "diskio.h"
#include <stdio.h>

#define MAX_VOL_NAME_LEN    (16)

typedef struct __DiskIo
{
    char disk_path[MAX_VOL_NAME_LEN];
    DSTATUS (*disk_status)(void);
    DSTATUS (*disk_initialize)(void);
    DRESULT (*disk_read)(BYTE *buff, LBA_t sector, UINT count);
    DRESULT (*disk_write)(const BYTE *buff, LBA_t sector, UINT count);
    DRESULT (*disk_ioctl) (BYTE cmd, void *buff);
}DiskIo_t;

int f_disk_regist(DiskIo_t * disk, const char* volume_name, int id);
DiskIo_t * f_disk_get(int id);
char * f_disk_path(int id);
#endif 