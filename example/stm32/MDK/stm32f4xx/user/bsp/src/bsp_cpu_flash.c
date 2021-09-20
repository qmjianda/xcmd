/**********************************************************************************************************
*
*	模块名称 : cpu内部falsh操作模块
*	文件名称 : bsp_cpu_flash.c
*	版    本 : V1.0
*	说    明 : 提供读写CPU内部Flash的函数
**********************************************************************************************************/

#include "bsp_cpu_flash.h"

/**
  * @brief  根据输入的地址给出它所在的sector
  *					例如：
						uwStartSector = GetSector(FLASH_USER_START_ADDR);
						uwEndSector = GetSector(FLASH_USER_END_ADDR);	
  * @param  Address：地址
  * @retval 地址所在的sector
  */
static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector = 0;

    if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    {
        sector = FLASH_Sector_0;  
    }
    else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    {
        sector = FLASH_Sector_1;  
    }
    else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    {
        sector = FLASH_Sector_2;  
    }
    else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    {
        sector = FLASH_Sector_3;  
    }
    else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    {
        sector = FLASH_Sector_4;  
    }
    else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    {
        sector = FLASH_Sector_5;  
    }
    else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    {
        sector = FLASH_Sector_6;  
    }
    else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    {
        sector = FLASH_Sector_7;  
    }
    else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    {
        sector = FLASH_Sector_8;  
    }
    else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    {
        sector = FLASH_Sector_9;  
    }
    else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    {
        sector = FLASH_Sector_10;  
    }


    else if((Address < ADDR_FLASH_SECTOR_12) && (Address >= ADDR_FLASH_SECTOR_11))
    {
    sector = FLASH_Sector_11;  
    }

    else if((Address < ADDR_FLASH_SECTOR_13) && (Address >= ADDR_FLASH_SECTOR_12))
    {
    sector = FLASH_Sector_12;  
    }
    else if((Address < ADDR_FLASH_SECTOR_14) && (Address >= ADDR_FLASH_SECTOR_13))
    {
    sector = FLASH_Sector_13;  
    }
    else if((Address < ADDR_FLASH_SECTOR_15) && (Address >= ADDR_FLASH_SECTOR_14))
    {
    sector = FLASH_Sector_14;  
    }
    else if((Address < ADDR_FLASH_SECTOR_16) && (Address >= ADDR_FLASH_SECTOR_15))
    {
    sector = FLASH_Sector_15;  
    }
    else if((Address < ADDR_FLASH_SECTOR_17) && (Address >= ADDR_FLASH_SECTOR_16))
    {
    sector = FLASH_Sector_16;  
    }
    else if((Address < ADDR_FLASH_SECTOR_18) && (Address >= ADDR_FLASH_SECTOR_17))
    {
    sector = FLASH_Sector_17;  
    }
    else if((Address < ADDR_FLASH_SECTOR_19) && (Address >= ADDR_FLASH_SECTOR_18))
    {
    sector = FLASH_Sector_18;  
    }
    else if((Address < ADDR_FLASH_SECTOR_20) && (Address >= ADDR_FLASH_SECTOR_19))
    {
    sector = FLASH_Sector_19;  
    }
    else if((Address < ADDR_FLASH_SECTOR_21) && (Address >= ADDR_FLASH_SECTOR_20))
    {
    sector = FLASH_Sector_20;  
    } 
    else if((Address < ADDR_FLASH_SECTOR_22) && (Address >= ADDR_FLASH_SECTOR_21))
    {
    sector = FLASH_Sector_21;  
    }
    else if((Address < ADDR_FLASH_SECTOR_23) && (Address >= ADDR_FLASH_SECTOR_22))
    {
    sector = FLASH_Sector_22;  
    }
    else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_23))*/
    {
    sector = FLASH_Sector_23;  
    }
    return sector;
    }
/*
*********************************************************************************************************
*	函 数 名: bsp_ReadCpuFlash
*	功能说明: 读取CPU Flash的内容
*	形    参：_ucpDst : 目标缓冲区
*			 _ulFlashAddr : 起始地址
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0=成功，1=失败
*********************************************************************************************************
*/
uint8_t bsp_ReadCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpDst, uint32_t _ulSize)
{
	uint32_t i;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_END_ADDR)
	{
		return 1;
	}

	/* 长度为0时不继续操作,否则起始地址为奇地址会出错 */
	if (_ulSize == 0)
	{
		return 1;
	}

	for (i = 0; i < _ulSize; i++)
	{
		*_ucpDst++ = *(uint8_t *)_ulFlashAddr++;
	}
	return 0;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_CmpCpuFlash
*	功能说明: 比较Flash指定地址的数据.
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpBuf : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值:
*			FLASH_IS_EQU		0   Flash内容和待写入的数据相等，不需要擦除和写操作
*			FLASH_REQ_WRITE		1	Flash不需要擦除，直接写
*			FLASH_REQ_ERASE		2	Flash需要先擦除,再写
*			FLASH_PARAM_ERR		3	函数参数错误
*********************************************************************************************************
*/
uint8_t bsp_CmpCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpBuf, uint32_t _ulSize)
{
	uint32_t i;
	uint8_t ucIsEqu;	/* 相等标志 */
	uint8_t ucByte;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_END_ADDR)
	{
		return FLASH_PARAM_ERR;		/*　函数参数错误　*/
	}

	/* 长度为0时返回正确 */
	if (_ulSize == 0)
	{
		return FLASH_IS_EQU;		/* Flash内容和待写入的数据相等 */
	}

	ucIsEqu = 1;			/* 先假设所有字节和待写入的数据相等，如果遇到任何一个不相等，则设置为 0 */
	for (i = 0; i < _ulSize; i++)
	{
		ucByte = *(uint8_t *)_ulFlashAddr;

		if (ucByte != *_ucpBuf)
		{
			ucIsEqu = 0;	/* 需要擦除后再写 */
		}

		_ulFlashAddr++;
		_ucpBuf++;
	}

	if (ucIsEqu == 1)
	{
		return FLASH_IS_EQU;	/* Flash内容和待写入的数据相等，不需要擦除和写操作 */
	}
	else
	{
		return FLASH_REQ_ERASE;  /* Flash需要先擦除,再写 */
	}

}

/**********************************************************************************************************
*	函 数 名: bsp_EraseCpuFlash
*	功能说明: 擦除flsh。
*	形    参: _ulFlashAddr : Flash地址
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0 成功，1 数据长度或地址溢出，2 写Flash出错(估计Flash寿命到)
**********************************************************************************************************/

uint8_t bsp_EraseCpuFlash(uint32_t _ulFlashAddr, uint32_t _ulSize)
{
  
	/*要擦除的起始扇区(包含)及结束扇区(包含)，如8-12，表示擦除8、9、10、11、12扇区*/
	uint32_t uwStartSector = 0;
	uint32_t uwEndSector = 0;
	uint32_t uwSectorCounter = 0;
//	FLASH_Status status = FLASH_COMPLETE;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_END_ADDR)
	{
		return 1;
	}

	/* 长度为0 时不继续操作  */
	if (_ulSize == 0)
	{
		return 0;
	}

	__set_PRIMASK(1);  		/* 关中断 */

	/* FLASH 解锁 */
	FLASH_Unlock();

    /* 清除各种FLASH的标志位 */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 	/* 需要擦除 */
	
    
    uwStartSector = GetSector(_ulFlashAddr);
    uwEndSector = GetSector(_ulFlashAddr+_ulSize);

    /* 开始擦除操作 */
    uwSectorCounter = uwStartSector;
    while (uwSectorCounter <= uwEndSector) 
    {
        /* VoltageRange_3 以“字”的大小进行操作 */ 
        if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
        { 
            /*擦除出错，返回，实际应用中可加入处理 */
            return 2;
        }
        /* 计数器指向下一个扇区 */
        if (uwSectorCounter == FLASH_Sector_23)
        {
            uwSectorCounter += 40;
        } 
        else 
        {
            uwSectorCounter += 8;
        }
    }
    __set_PRIMASK(0);  		/* 开中断 */
    return 0;
}

/**********************************************************************************************************
*	函 数 名: bsp_WriteCpuFlash
*	功能说明: 写数据到CPU 内部Flash。
*	形    参: _ulFlashAddr : Flash地址
*			 _ucpSrc : 数据缓冲区
*			 _ulSize : 数据大小（单位是字节）
*	返 回 值: 0 成功，1 数据长度或地址溢出，2 写Flash出错(估计Flash寿命到)
**********************************************************************************************************/
uint8_t bsp_WriteCpuFlash(uint32_t _ulFlashAddr, uint8_t *_ucpSrc, uint32_t _ulSize)
{
	uint8_t ucRet;
    
	/*要擦除的起始扇区(包含)及结束扇区(包含)，如8-12，表示擦除8、9、10、11、12扇区*/
	uint32_t uwStartSector = 0;
	uint32_t uwEndSector = 0;
	uint32_t uwSectorCounter = 0;
    uint32_t uwAddress = 0;
    uint32_t uwEndAddr = 0;
    uint32_t* p_uwData32 = (uint32_t*)_ucpSrc;
	FLASH_Status status = FLASH_COMPLETE;

	/* 如果偏移地址超过芯片容量，则不改写输出缓冲区 */
	if (_ulFlashAddr + _ulSize > FLASH_END_ADDR)
	{
		return 1;
	}

	/* 长度为0 时不继续操作  */
	if (_ulSize == 0)
	{
		return 0;
	}

	ucRet = bsp_CmpCpuFlash(_ulFlashAddr, _ucpSrc, _ulSize);

	if (ucRet == FLASH_IS_EQU)
	{
		return 0;
	}

	__set_PRIMASK(1);  		/* 关中断 */

	/* FLASH 解锁 */
	FLASH_Unlock();

    /* 清除各种FLASH的标志位 */  
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 	/* 需要擦除 */
	
    
    uwStartSector = GetSector(_ulFlashAddr);
    uwEndSector = GetSector(_ulFlashAddr+_ulSize);

    /* 开始擦除操作 */
    uwSectorCounter = uwStartSector;
    while (uwSectorCounter <= uwEndSector) 
    {
        /* VoltageRange_3 以“字”的大小进行操作 */ 
        if (FLASH_EraseSector(uwSectorCounter, VoltageRange_3) != FLASH_COMPLETE)
        { 
            /*擦除出错，返回，实际应用中可加入处理 */
            return 2;
        }
        /* 计数器指向下一个扇区 */
        if (uwSectorCounter == FLASH_Sector_23)
        {
            uwSectorCounter += 40;
        } 
        else 
        {
            uwSectorCounter += 8;
        }
    }
    
    /* 以“字”的大小为单位写入数据 ********************************/
    uwAddress = _ulFlashAddr;
    uwEndAddr = (uwAddress+_ulSize);
    
    /*4字节对齐*/
    if(uwEndAddr%4)
    {
        uwEndAddr = uwEndAddr+(4-uwEndAddr%4);
    }
    
    while (uwAddress <= uwEndAddr)
    {
        if (FLASH_ProgramWord(uwAddress, *p_uwData32) == FLASH_COMPLETE)
        {
            uwAddress = uwAddress + 4;
            p_uwData32++;
        }
        else
        { 
            /*写入出错，返回，实际应用中可加入处理 */
            return 2;
        }
    }

  	/* Flash 加锁，禁止写Flash控制寄存器 */
  	FLASH_Lock();

  	__set_PRIMASK(0);  		/* 开中断 */

	if (status == FLASH_COMPLETE)
	{
		return 0;
	}
	return 2;
}

/************************************** DIHEELEC (END OF FILE) *******************************************/

