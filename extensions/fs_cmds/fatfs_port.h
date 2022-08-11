/*
 * @Author: your name
 * @Date: 2021-10-09 13:55:20
 * @LastEditTime: 2021-10-09 15:51:46
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: /xcmd/example/linux/fatfs_port.h
 */

#ifndef FATFS_PORT_H
#define FATFS_PORT_H

#include "ff.h"
#include "diskio.h"
#include <stdio.h>

typedef struct __DiskIo
{
    char * disk_name;
    DSTATUS (*disk_status)(void);
    DSTATUS (*disk_initialize)(void);
    DRESULT (*disk_read)(BYTE *buff, LBA_t sector, UINT count);
    DRESULT (*disk_write)(const BYTE *buff, LBA_t sector, UINT count);
    DRESULT (*disk_ioctl) (BYTE cmd, void *buff);
}DiskIo_t;

int f_disk_regist(DiskIo_t * disk, int id);
DiskIo_t * f_disk_get(int id);

#endif 