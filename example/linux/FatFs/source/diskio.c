/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#include "fatfs_port.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM		0	/* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC		1	/* Example: Map MMC/SD card to physical drive 1 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		result = RAM_disk_status();
        if(result == 0)
        {
            stat = RES_OK;
        }
        else
        {
            stat = RES_NOTRDY;
        }
		return stat;

	case DEV_MMC :
		result = MMC_disk_status();
        if(result == 0)
        {
            stat = RES_OK;
        }
        else
        {
            stat = RES_NOTRDY;
        }
		return stat;
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
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		result = RAM_disk_initialize();
        result = result;
		stat = disk_status(pdrv);
		return stat;

	case DEV_MMC :
		result = MMC_disk_initialize();
		stat = disk_status(pdrv);
		return stat;
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
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		result = RAM_disk_read(buff, sector, count);
		if(result == 0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
		return res;

	case DEV_MMC :
		result = MMC_disk_read(buff, sector, count);
		if(result == 0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
		return res;
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
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :
		result = RAM_disk_write(buff, sector, count);
		if(result == 0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
        return res;

	case DEV_MMC :
		result = MMC_disk_write(buff, sector, count);
        if(result == 0)
        {
            res = RES_OK;
        }
        else
        {
            res = RES_ERROR;
        }
		return res;
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
	DRESULT res;

	switch (pdrv) {
	case DEV_RAM :
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
        res = RES_OK;
		return res;
	case DEV_MMC :
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
        res = RES_OK;
		return res;
    }
	return RES_PARERR;
}

