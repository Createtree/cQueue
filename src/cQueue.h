/*-----------------------------------------------------------------------
|                            FILE DESCRIPTION                           |
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  - File name     : cQueue.h
  - Author        : liuzhihua
  - Update date   : 2024.03.09
  -	File Function : Circular Queue
  - Version       : v1.1
  - Origin        ：https://github.com/Createtree/cQueue
-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
|                               UPDATE NOTE                             |
-----------------------------------------------------------------------*/
/**
  * Update note:
  * ------------   ---------------   ----------------------------------
  *     Date            Author                      Note
  * ------------   ---------------   ----------------------------------
  *  2022.04.02       liuzhihua                  Create file
  *  2023.04.17       liuzhihua                   Rewrite
  *  2024.03.09       liuzhihua                Add and modify
***/

#ifndef __CQUEUE_H_
#define __CQUEUE_H_

#ifdef  __cplusplus
    extern "C" {
#endif
/*-----------------------------------------------------------------------
|                               INCLUDES                                |
-----------------------------------------------------------------------*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include "mleak.h"
/*-----------------------------------------------------------------------
|                                DEFINES                                |
-----------------------------------------------------------------------*/
#define cQueue_Malloc(size) MALLOC(size)//malloc(size)
#define cQueue_Free(pv) FREE(pv)//free(pv)
#define cQueue_Memcpy(Dst,Src,size) memcpy(Dst,Src,size)
#define cQueueAssert(x) assert(x)
#define CQUEUE_USE_LOCK 0U

#ifdef CQUEUE_USE_LOCK
#define __CQUEUE_LOCK(_HANDLE_)            \
  do                                       \
  {                                        \
    if ((_HANDLE_)->lock == CQUEUE_LOCKED) \
    {                                      \
      return CQUEUE_BUSY;                  \
    }                                      \
    else                                   \
    {                                      \
      (_HANDLE_)->lock = CQUEUE_LOCKED;    \
    }                                      \
  } while (0)

#define __CQUEUE_UNLOCK(_HANDLE_) \
  do                              \
  {                               \
    (_HANDLE_)->lock = 0;         \
  } while (0)
#else
#define __CQUEUE_LOCK(_HANDLE_)
#define __CQUEUE_UNLOCK(_HANDLE_)
#endif

typedef struct cQueue_TypeDef
{
	void *data;
  uint8_t unitSize;
	uint16_t len;
	uint16_t pWrite;
	uint16_t pRead;
  uint8_t full:1;
  uint8_t canFree:1;
  uint8_t isInit:1;
  uint8_t lock:1;
}cQueue_t;

typedef enum
{
  CQUEUE_OK,
  CQUEUE_BUSY,
  CQUEUE_FULL,
  CQUEUE_NULL,
  CQUEUE_ERROR
}cQueueStatus;

enum{
  CQUEUE_UNLOCKED,
  CQUEUE_LOCKED,
};
/*-----------------------------------------------------------------------
|                             API FUNCTION                              |
-----------------------------------------------------------------------*/
cQueue_t *cQueue_Create(uint8_t UnitSize, uint16_t Length);
void cQueue_Create_Static(cQueue_t *pcQueue, void *pdata, uint8_t UnitSize, uint16_t Length);
cQueueStatus cQueue_Push(cQueue_t *pcQ, void *pdata);
cQueueStatus cQueue_Pushs(cQueue_t *pcQ, void *pdata, uint16_t size);
cQueueStatus cQueue_OverWrite(cQueue_t *pcQ, void *pdata, uint16_t size);
cQueueStatus cQueue_Pop(cQueue_t *pcQ, void *pReceive);
cQueueStatus cQueue_Pops(cQueue_t *pcQ, void *pReceive, uint16_t size);
cQueueStatus cQueue_Peek(cQueue_t *pcQ, void *pReceive);
cQueueStatus cQueue_Peeks(cQueue_t *pcQ, void *pReceive, uint16_t size);
cQueueStatus cQueue_Pushv(cQueue_t *pcQ, void *pdata, uint16_t size);
uint16_t cQueue_Popv(cQueue_t *pcQ, void *pdata, uint16_t size);
uint16_t cQueue_Empty(cQueue_t *pcQ);
uint16_t cQueue_Full(cQueue_t *pcQ);
uint16_t cQueue_Usage(cQueue_t *pcQ);
uint16_t cQueue_Spare(cQueue_t *pcQ);
uint16_t cQueue_GetReadPtrMargin(cQueue_t *pcQ);
uint16_t cQueue_GetWritePtrMargin(cQueue_t *pcQ);
uint16_t cQueue_GetLength(cQueue_t *pcQ);
int cQueue_Skip(cQueue_t *pcQ, uint16_t len);
void cQueue_Clear(cQueue_t *pcQ);
void *cQueue_GetReadAdr(cQueue_t *pcQ);
void *cQueue_GetWriteAdr(cQueue_t *pcQ);
void cQueue_MoveWrite(cQueue_t *pcQ, uint16_t len);
void cQueue_MoveRead(cQueue_t *pcQ, uint16_t len);
cQueueStatus cQueue_Destroy(cQueue_t *pcQ);
#ifdef __cplusplus
}
#endif

#endif

