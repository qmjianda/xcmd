#ifndef __LIB_RNG_H_
#define __LIB_RNG_H_

#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x.h"

typedef struct
{
	char *pBuf;		/*指向ring缓冲区的指针*/
	int  capaticy;  /*ring的大小*/
	int  fromBuf;    /*指向可取数据*/
	int  toBuf;	 	/*指向可存数据*/
}T_Ring;

typedef int RING_ID;

RING_ID xrngInit(T_Ring *ring ,char *buffer, int bufLen);
bool xrngGet(RING_ID ring, char* rcv);
bool xrngPut(RING_ID ring, char val);
void xrngPutForce(RING_ID ring, char val);
int xrngBufGet(RING_ID ring, char*buf, int len);
int xrngBufPut(RING_ID ring, char*buf, int len);
bool xrngCpy(RING_ID ring, int where, char* rcv);
int xrngBufCpy(RING_ID ring , char* buf, int bufLen);
int xrngLen(RING_ID ring);
bool xrngIsFull(RING_ID ring);
bool xrngIsEmpty(RING_ID ring);
void xrngClear(RING_ID ring); 

#endif /*__LIB_RNG_H_*/
