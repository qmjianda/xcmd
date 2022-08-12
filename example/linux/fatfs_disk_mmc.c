#include "fatfs_disk_mmc.h"
#include "fatfs_port.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MMC_DISK_SECTION_SIZE   (512)
#define MMC_DISK_SECTION_COUNT  (1024)

static int mmc_fd = -1;
static DiskIo_t disk;
static FATFS g_fs;
static BYTE g_work[FF_MAX_SS];

static DSTATUS m_disk_status(void)
{
    return RES_OK;
}

static DSTATUS m_disk_initialize(void)
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
            return RES_NOTRDY;
        }
        close(mmc_fd);
    }
    return RES_OK;
}

static DRESULT m_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    DRESULT ret = RES_ERROR;
    mmc_fd = open("./fatfs.img", O_RDWR);
    if(mmc_fd >= 0)
    {
        lseek(mmc_fd, sector*MMC_DISK_SECTION_SIZE, L_SET);
        int read_len = read(mmc_fd, buff, count*MMC_DISK_SECTION_SIZE);
        if(read_len == count*MMC_DISK_SECTION_SIZE)
        {
            ret = RES_OK;
        }
    }
    close(mmc_fd);
    return ret;
}

static DRESULT m_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    DRESULT ret = RES_ERROR;
    mmc_fd = open("./fatfs.img", O_RDWR);
    if(mmc_fd >= 0)
    {
        lseek(mmc_fd, sector*MMC_DISK_SECTION_SIZE, L_SET);
        int write_len = write(mmc_fd, buff, count*MMC_DISK_SECTION_SIZE);
        if(write_len == count*MMC_DISK_SECTION_SIZE)
        {
            ret = RES_OK;
        }
    }
    close(mmc_fd);
    return ret;
}

static DRESULT m_disk_ioctl(BYTE cmd, void *buff)
{
    switch(cmd)
    {
        case CTRL_SYNC:
        break;
        case GET_SECTOR_COUNT:
            *(WORD*)buff = MMC_DISK_SECTION_COUNT;
        break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = MMC_DISK_SECTION_SIZE;
        break;
        case GET_BLOCK_SIZE:
            *(WORD*)buff = 1;
        break;
        case CTRL_TRIM:
        break;
    }
    return RES_OK;
}

int mmc_disk_init(void)
{
    FRESULT res;
    disk.disk_initialize = m_disk_initialize;
    disk.disk_ioctl = m_disk_ioctl;
    disk.disk_read = m_disk_read;
    disk.disk_status = m_disk_status;
    disk.disk_write = m_disk_write;

    if(f_disk_regist(&disk, "MMC", 1) == -1)
    {
        printf("MMC disk regist ERROR!\r\n");
        return FR_DISK_ERR;
    }

    res = f_mount(&g_fs, disk.disk_path, 1); //挂载文件系统 ， "1:"就是挂载的设备号为1的设备
    if (res == FR_NO_FILESYSTEM) //FR_NO_FILESYSTEM值为13，表示没有有效的设备
    {
        res = f_mkfs(disk.disk_path, 0, g_work, sizeof(g_work));
        res = f_mount(NULL, disk.disk_path, 1);  //取消文件系统
        res = f_mount(&g_fs, disk.disk_path, 1); //挂载文件系统
    }
    
    if(res == FR_OK)
    {
        printf("MMC fs init OK!\r\n");
    }
    else
    {
        printf("MMC fs init ERROR!\r\n");
    }
    return res;
}