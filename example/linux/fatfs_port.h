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

#define RAM_DISK_SECTION_SIZE   (512)
#define RAM_DISK_SECTION_COUNT  (1024)

#define MMC_DISK_SECTION_SIZE   (512)
#define MMC_DISK_SECTION_COUNT  (1024)

int RAM_disk_status(void);
int MMC_disk_status(void);

int RAM_disk_initialize(void);
int MMC_disk_initialize(void);

int RAM_disk_read(BYTE *buff, LBA_t sector, UINT count);
int MMC_disk_read(BYTE *buff, LBA_t sector, UINT count);

int RAM_disk_write(const BYTE *buff, LBA_t sector, UINT count);
int MMC_disk_write(const BYTE *buff, LBA_t sector, UINT count);

#endif 