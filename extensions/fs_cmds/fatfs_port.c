#include "ff.h"			/* Obtains integer types */
#include "fatfs_port.h"
#include <stdio.h>

static DiskIo_t * g_Disks[FF_VOLUMES] = {0};

#if FF_STR_VOLUME_ID >= 1
#ifndef FF_VOLUME_STRS
const char* VolumeStr[FF_VOLUMES] = {0};
#else
static char * VolumeStr[FF_VOLUMES] = {FF_VOLUME_STRS};
#endif
#endif

int f_disk_regist(DiskIo_t * disk, const char* volume_name, int id)
{
    if(!disk || !volume_name)
    {
        return -1;
    }
    if(id<0)
    {
        for(int i=0; i<FF_VOLUMES; i++)
        {
            if(!g_Disks[i])
            {
                g_Disks[i] = disk;
                id = i;
                goto ok;
            }
        }
    }
    else
    {
        if((id < FF_VOLUMES) && !g_Disks[id])
        {
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

DiskIo_t * f_disk_get(int id)
{
    if(id < FF_VOLUMES)
    {
        return g_Disks[id];
    }
    return NULL;
}

char * f_disk_path(int id)
{
    if(id < FF_VOLUMES)
    {
        return g_Disks[id]->disk_path;
    }
    return NULL;
}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
    DiskIo_t * disk = NULL;
	disk = f_disk_get(pdrv);
    if(disk)
    {
       return disk->disk_status();
    }
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DiskIo_t * disk = NULL;
	disk = f_disk_get(pdrv);
    if(disk)
    {
       return disk->disk_initialize();
    }
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	DiskIo_t * disk = NULL;
	disk = f_disk_get(pdrv);
    if(disk)
    {
       return disk->disk_read(buff, sector, count);
    }

	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    DiskIo_t * disk = NULL;
	disk = f_disk_get(pdrv);
    if(disk)
    {
       return disk->disk_write(buff, sector, count);
    }
	return RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DiskIo_t * disk = NULL;
	disk = f_disk_get(pdrv);
    if(disk)
    {
       return disk->disk_ioctl(cmd, buff);
    }
	return RES_PARERR;
}
