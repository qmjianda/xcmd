#ifndef __BSP_RING_H_
#define __BSP_RING_H_

#include "include.h"

typedef struct
{
	char *pBuf;		 /*指向ring缓冲区的指针*/
	int  capaticy;   /*ring的大小*/
	int  fromBuf;    /*指向可取数据*/
	int  toBuf;	 	 /*指向可存数据*/
}T_Ring;

typedef int RING_ID;

RING_ID xrngInit(T_Ring *ring ,char *buffer, int bufLen);
BOOL xrngGet(RING_ID ring, char* rcv);
BOOL xrngPut(RING_ID ring, char val);
void xrngPutForce(RING_ID ring, char val);
int xrngBufGet(RING_ID ring, char*buf, int len);
int xrngBufPut(RING_ID ring, char*buf, int len);
BOOL xrngCpy(RING_ID ring, int where, char* rcv);
int xrngBufCpy(RING_ID ring , char* buf, int bufLen);
int xrngLen(RING_ID ring);
BOOL xrngIsFull(RING_ID ring);
BOOL xrngIsEmpty(RING_ID ring);
void xrngClear(RING_ID ring); 

#endif /*__XRNG_LIB_H_*/
