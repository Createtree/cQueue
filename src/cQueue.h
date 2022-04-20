/*-----------------------------------------------------------------------
|                            FILE DESCRIPTION                           |
-----------------------------------------------------------------------*/
/*----------------------------------------------------------------------
  - File name     : cQueue.h
  - Author        : liuzhihua
  - Update date   : 2022.4.2                  
  -	File Function : Circular Queue
-----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
|                               UPDATE NOTE                             |
-----------------------------------------------------------------------*/
/**
  * Update note:
  * ------------   ---------------   ----------------------------------
  *     Date            Author                       Note
  * ------------   ---------------   ----------------------------------
  *   2022.4.2        liuzhihua                   Create file
***/

#ifndef __CQUEUE_H_
#define __CQUEUE_H_

#ifdef  __cplusplus
    extern "C" {
#endif
/*-----------------------------------------------------------------------
|                               INCLUDES                                |
-----------------------------------------------------------------------*/

#include "stdarg.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "mleak.h"
/*-----------------------------------------------------------------------
|                                DEFINES                                |
-----------------------------------------------------------------------*/
#define cQueue_Malloc(size) MALLOC(size)//malloc(size)
#define cQueue_Free(pv) FREE(pv)//free(pv)
#define cQueue_Memcpy(Dst,Src,size) memcpy(Dst,Src,size)
typedef struct	cQueue_Frame_TypeDef
{
	uint8_t* data;
	uint8_t size;
	uint8_t is_read:1;
	uint8_t canFree:1; 
}cQueue_Frame_t;
		
typedef struct cQueue_TypeDef{
	cQueue_Frame_t* FrameBuf;
	uint8_t size;
	uint8_t pWrite;
	uint8_t pRead;
	uint8_t clear; 
}cQueue_t;

		
		
		
/*-----------------------------------------------------------------------
|                             API FUNCTION                              |
-----------------------------------------------------------------------*/
cQueue_t* cQueue_Create(uint8_t FrameBuf_size);
uint8_t cQueue_AddFrame(cQueue_t* pcQ, uint8_t** data, uint8_t len, uint8_t CanFree);
uint8_t cQueue_AddFrameCopy(cQueue_t* pcQ, uint8_t** data, uint8_t len);
uint8_t cQueue_AddFormatData(cQueue_t* pcQ, const char* fmt,...);
void cQueue_Free_FrameData(cQueue_t* pcQ);
cQueue_Frame_t* cQueue_GetFrame(cQueue_t* pcQ);
void cQueue_Delete(cQueue_t* pcQ);



#ifdef __cplusplus
}
#endif

#endif

