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

static DiskIo_t * g_Disks[FF_VOLUMES] = {0};

int f_disk_regist(DiskIo_t * disk, int id)
{
    if(!disk)
    {
        return -1;
    }

    if(id<0)
    {
        for(int i=0; i<FF_VOLUMES; i++)
        {
            if(!g_Disks[id])
            {
                g_Disks[i] = disk;
                return i;
            }
        }
    }
    else
    {
        if((id < FF_VOLUMES) && !g_Disks[id])
        {
            g_Disks[id] = disk;
            return id;
        }
    }
    return -1;
}

DiskIo_t * f_disk_get(int id)
{
    if(id < FF_VOLUMES)
    {
        return g_Disks[id];
    }
    return NULL;
}
