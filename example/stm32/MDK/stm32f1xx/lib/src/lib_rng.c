#include "lib_rng.h"
#include "stdbool.h"
#include <stdio.h>

/*
 *实际使用中使用xrngBufPut和xrngPut只能存储buflen-1个数据
 *使用xrngPutForce可以存储bufLen个数据 
 */
RING_ID xrngInit(T_Ring *ring ,char *buffer, int bufLen)
{
	ring->pBuf  = buffer;
	ring->capaticy = bufLen;
	ring->fromBuf   = 0;
	ring->toBuf	 = 0;
	return (int)ring;
}

/*
 * 从ring中读取数据
 * 返回 1：读取成功
 * 返回 0：读取失败，ring已经空了
 */
bool xrngGet(RING_ID ring, char* rcv)
{
	bool ret = false;
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return ret;
    }
    
	if(_ring->fromBuf != _ring->toBuf) /*判断是否已经空了*/
	{
		*rcv = _ring->pBuf[_ring->fromBuf++];
		/*fromBuf = fromBuf%capaticy*/
		if(_ring->fromBuf>=_ring->capaticy)
		{
			_ring->fromBuf = 0;
		}
		ret = true;
	}
	return ret;
}

/*
 * 从ring中拷贝数据，拷贝数据并不会是ring中的数据减少
 * 返回 1：读取成功
 * 返回 0：读取失败，ring已经空了
 */
bool xrngCpy(RING_ID ring, int where, char* rcv)
{
    int fromBuf , i=0;
    bool ret = false;
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return ret;
    }
    fromBuf = _ring->fromBuf;
    while(fromBuf  != _ring->toBuf)
    {
        if( i == where)
        {
            *rcv = _ring->pBuf[_ring->fromBuf];
            ret = true;
            break;
        }
        i++;
        fromBuf++;
        if(fromBuf >= _ring->capaticy)
        {
            fromBuf = 0;
        }
    }
    return ret;
}

/*
 * 从ring中拷贝len数据存储在buf中，实际读取的长度可能小于len，拷贝函数并不会清空ring中的数据
 * 返回：实际读取的数据长度
 */
int xrngBufCpy(RING_ID ring , char* buf, int bufLen)
{
    int fromBuf = 0, i=0;
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return 0;
    }
    
    fromBuf = _ring->fromBuf;
    while(fromBuf != _ring->toBuf)
    {
        if(i<bufLen)
            buf[i++] = _ring->pBuf[_ring->fromBuf++];
        else
            break;
        if(fromBuf >= _ring->capaticy)
        {
            fromBuf = 0;
        }
    }
    return i;
}

/*
 * 将数据写入到ring中，在写入过程中最多只能写入 capaticy-1个数据 
 * 返回 1：写入成功
 * 返回 0：写入失败，ring已经满了
 */
bool xrngPut(RING_ID ring, char val)
{
	bool ret = false;
    int temp = 0;
    
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return ret;
    }
    
	temp = _ring->toBuf; /*用于计算toBuf+1是否与fromBuf相等*/
	/*temp = temp%capaticy*/
	if(++temp>= _ring->capaticy)
	{
		temp = 0;
	}
	if(temp != _ring->fromBuf) /*判断ring是否满了*/
	{
		_ring->pBuf[_ring->toBuf++] = val;
		if(_ring->toBuf >= _ring->capaticy)
		{
			_ring->toBuf = 0;
		}
		ret = true;
	}
	return ret;
}

/*
 *强制将数据写入到ring中，如果已经写满则覆盖掉之前的数据
 */
void xrngPutForce(RING_ID ring, char val)
{
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return;
    }
    
	_ring->pBuf[_ring->toBuf++] = val; /*先写入数据*/
	if(_ring->toBuf >= _ring->capaticy)
	{
		_ring->toBuf = 0;
	}
	if(_ring->toBuf == _ring->fromBuf) /*判断是否写满了*/
	{
		
        _ring->fromBuf++; /*移动fromBuf给toBuf让一个空间*/
		/*fromBuf = fromBuf%capaticy*/
		if(_ring->fromBuf>=_ring->capaticy)
		{
			_ring->fromBuf = 0;
		}
	}
}

/*
 * 从ring中读取len数据存储在buf中，实际读取的长度可能小于len
 * 返回：实际读取的数据长度
 */
int xrngBufGet(RING_ID ring, char*buf, int len)
{
	int i=0;
	int min;
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return 0;
    }
    
	/*计算实际可以读取的数量*/
	min = xrngLen(ring);
	if(min > len) 
	{
		min = len;
	}
	
    for(i=0; i< min; i++)
    {
        buf[i] = _ring->pBuf[_ring->fromBuf++];
        if( _ring->fromBuf>=_ring->capaticy )
        {
            _ring->fromBuf = 0;
        }
    }
	return i;
}

/*
 *向ring中写入一组数据，写入的长度可能小于len，最多只能写入 capaticy-1个数据  
 *返回：实际写入的数据的长度
 */
int xrngBufPut(RING_ID ring, char*buf, int len)
{
	int i=0;
    int min;
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return 0;
    }
    
	/*计算实际可以写入的数据量*/
	min = (_ring->capaticy - xrngLen(ring))-1;
	if(min > len)
	{
		min = len;
	}

    for(i=0; i<min; i++)
    {
        _ring->pBuf[_ring->toBuf++] = buf[i];
        if(_ring->toBuf >= _ring->capaticy)
        {
            _ring->toBuf = 0;
        }
    }
	return i;
}

/*
 *获取ring中已经存储的数据的个数 
 */
int xrngLen(RING_ID ring)
{
	int ret = 0;
    
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return ret;
    }
    
	if(_ring->toBuf >= _ring->fromBuf)
	{
		ret = _ring->toBuf - _ring->fromBuf;
	}
	else
	{
		ret = _ring->capaticy - _ring->fromBuf;
		ret += _ring->toBuf;
	}
	return ret;
}

bool xrngIsFull(RING_ID ring)
{
	int toBuf;
    
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return false;
    }
    
    toBuf = _ring->toBuf;
    
	if(++toBuf >= _ring->capaticy)
	{
		toBuf = 0;
	}
	return (bool)(toBuf == _ring->fromBuf);
}

bool xrngIsEmpty(RING_ID ring)
{
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return false;
    }
	return (bool)(_ring->toBuf == _ring->fromBuf);
}

void xrngClear(RING_ID ring)
{
    T_Ring* _ring = (T_Ring*)ring;
    if(_ring == NULL)
    {
        return;
    }
	_ring->toBuf = _ring->fromBuf = 0;
}
