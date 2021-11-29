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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

char *ram_buf[RAM_DISK_SECTION_COUNT][RAM_DISK_SECTION_SIZE];
int mmc_fd = -1;

int RAM_disk_status(void)
{
    return 0;
}

int MMC_disk_status(void)
{
    return 0;
}

int RAM_disk_initialize(void)
{
    return 0;
}

int MMC_disk_initialize(void)
{
    if(access("./fatfs.img", F_OK) != 0)
    {
        mmc_fd = open("./fatfs.img", O_CREAT|O_RDWR);
        if(mmc_fd >= 0)
        {
            char buf[MMC_DISK_SECTION_SIZE] = {0};
            for(DWORD i=0; i<MMC_DISK_SECTION_COUNT; i++)
            {
                write(mmc_fd, buf, MMC_DISK_SECTION_SIZE);
            }
        }
        else
        {
            return -1;
        }
        close(mmc_fd);
    }
    return 0;
}

int RAM_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    memcpy(buff, &ram_buf[sector], count * RAM_DISK_SECTION_SIZE);
    return 0;
}

int MMC_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    int ret = -1;
    mmc_fd = open("./fatfs.img", O_RDWR);
    if(mmc_fd >= 0)
    {
        lseek(mmc_fd, sector*MMC_DISK_SECTION_SIZE, L_SET);
        int read_len = read(mmc_fd, buff, count*MMC_DISK_SECTION_SIZE);
        if(read_len == count*MMC_DISK_SECTION_SIZE)
        {
            ret = 0;
        }
    }
    close(mmc_fd);
    return ret;
}

int RAM_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    memcpy(&ram_buf[sector], buff, count * RAM_DISK_SECTION_SIZE);
    return 0;
}

int MMC_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    int ret = -1;
    mmc_fd = open("./fatfs.img", O_RDWR);
    if(mmc_fd >= 0)
    {
        lseek(mmc_fd, sector*MMC_DISK_SECTION_SIZE, L_SET);
        int write_len = write(mmc_fd, buff, count*MMC_DISK_SECTION_SIZE);
        if(write_len == count*MMC_DISK_SECTION_SIZE)
        {
            ret = 0;
        }
    }
    close(mmc_fd);
    return ret;
}
