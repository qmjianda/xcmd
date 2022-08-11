#include "fatfs_disk_mmc.h"
#include "fatfs_port.h"
#include <string.h>

static DiskIo_t disk;

#define RAM_DISK_SECTION_SIZE   (512)
#define RAM_DISK_SECTION_COUNT  (1024)
static char *ram_buf[RAM_DISK_SECTION_COUNT][RAM_DISK_SECTION_SIZE];
static FATFS g_fs;
static BYTE g_work[FF_MAX_SS];

static DSTATUS m_disk_status(void)
{
    return 0;
}

static DSTATUS m_disk_initialize(void)
{
    return 0;
}

static DRESULT m_disk_read(BYTE *buff, LBA_t sector, UINT count)
{
    memcpy(buff, &ram_buf[sector], count * RAM_DISK_SECTION_SIZE);
    return 0;
}

static DRESULT m_disk_write(const BYTE *buff, LBA_t sector, UINT count)
{
    memcpy(&ram_buf[sector], buff, count * RAM_DISK_SECTION_SIZE);
    return 0;
}

static DRESULT m_disk_ioctl(BYTE cmd, void *buff)
{
    switch(cmd)
    {
        case CTRL_SYNC:
        break;
        case GET_SECTOR_COUNT:
            *(WORD*)buff = RAM_DISK_SECTION_COUNT;
        break;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = RAM_DISK_SECTION_SIZE;
        break;
        case GET_BLOCK_SIZE:
            *(WORD*)buff = 1;
        break;
        case CTRL_TRIM:
        break;
    }
    return RES_OK;
}

int ram_disk_init(void)
{
    FRESULT res;
    disk.disk_name = "RAM Disk";
    disk.disk_initialize = m_disk_initialize;
    disk.disk_ioctl = m_disk_ioctl;
    disk.disk_read = m_disk_read;
    disk.disk_status = m_disk_status;
    disk.disk_write = m_disk_write;
    if(f_disk_regist(&disk, 0) == -1)
    {
        printf("RAM disk regist ERROR!\r\n");
        return FR_DISK_ERR;
    }

    res = f_mount(&g_fs, "0:", 1); //挂载文件系统 ， "1:"就是挂载的设备号为1的设备
    if (res == FR_NO_FILESYSTEM) //FR_NO_FILESYSTEM值为13，表示没有有效的设备
    {
        res = f_mkfs("0:", 0, g_work, sizeof(g_work));
        res = f_mount(NULL, "0:", 1);  //取消文件系统
        res = f_mount(&g_fs, "0:", 1); //挂载文件系统
    }
    
    if(res == FR_OK)
    {
        printf("RAM fs init OK!\r\n");
    }
    else
    {
        printf("RAM fs init ERROR!\r\n");
    }
    return res;
}