#include "cQueue.h"
#define cQueue_WROFFSET(cQueue) (((cQueue)->pWrite > (cQueue)->pRead) ? \
							(cQueue)->pWrite - (cQueue)->pRead : \
							(cQueue)->pRead - (cQueue)->pWrite)
#define cQueue_IS_FULL(cQueue) (((cQueue)->pWrite == (cQueue)->pRead) && (cQueue->full) ? 1 : 0)
#define cQueue_IS_NULL(cQueue) (((cQueue)->pWrite == (cQueue)->pRead) && (!cQueue->full) ? 1 : 0)

#define _WriteAdr(cQueue) ((uint8_t*)(cQueue)->data + (cQueue)->pWrite * (cQueue)->unitSize)
#define _ReadAdr(cQueue)  ((uint8_t*)(cQueue)->data + (cQueue)->pRead * (cQueue)->unitSize)


static inline void _WriteMove(cQueue_t *pcQ, uint16_t len)
{
	pcQ->pWrite += len;
	if (pcQ->pWrite >= pcQ->len)//代替取余，提高速度
	{
		pcQ->pWrite -= pcQ->len;
	}
	if (pcQ->pWrite == pcQ->pRead)
	{
		pcQ->full = 1;
	}
}

static inline void _ReadMove(cQueue_t *pcQ, uint16_t len)
{
	pcQ->pRead += len;
	pcQ->full = 0;
	if (pcQ->pRead >= pcQ->len)
	{
		pcQ->pRead -= pcQ->len;
	}
}

/**
 * @brief 创建队列
 * @param UnitSize 队列单元尺寸(Byte)
 * @param Length 队列单元数量
 * @retval cQueue_t* 队列句柄
 */
cQueue_t* cQueue_Create(uint8_t UnitSize, uint16_t Length)
{
	cQueueAssert(UnitSize && Length);

	cQueue_t* pcQ = cQueue_Malloc(sizeof(cQueue_t));
	if(pcQ != NULL)
	{
		pcQ->data = cQueue_Malloc(Length * UnitSize);
		pcQ->unitSize = UnitSize;
		pcQ->len = Length;
		pcQ->pRead = 0;
		pcQ->pWrite = 0;
		pcQ->full = 0;
		pcQ->canFree = 1;
		pcQ->isInit = 1;
		pcQ->lock = 0;
	}
	return pcQ;
}

/**
 * @brief 静态创建队列 
 * @param pcQueue 队列句柄
 * @param pdata 队列数据指针
 * @param UnitSize 队列单元尺寸(byte)
 * @param Length 队列单元数量
 * @retval cQueue_t* 队列句柄
 */
void cQueue_Create_Static(cQueue_t* pcQueue, void* pdata, uint8_t UnitSize, uint16_t Length)
{
	cQueueAssert(pcQueue && pdata);
	cQueueAssert(UnitSize && Length);

	cQueue_t* pcQ = pcQueue;
	pcQ->unitSize = UnitSize;
	pcQ->data = pdata;
	pcQ->len = Length;
	pcQ->pRead = 0;
	pcQ->pWrite = 0;
	pcQ->full = 0;
	pcQ->canFree = 0;
	pcQ->isInit = 1;
	pcQ->lock = 0;
}

 /**
  * @brief 添加数据单元到队列
  * @param pcQ 队列句柄
  * @param pdata 数据
  * @retval cQueueState OK/FULL/BUSY
  */
cQueueStatus cQueue_Push(cQueue_t* pcQ, void* pdata)
{
	cQueueAssert(pcQ);
	cQueueAssert(pdata);
	cQueueAssert(pcQ->isInit);

	cQueueStatus status = CQUEUE_OK;
	__CQUEUE_LOCK(pcQ);

	if (cQueue_IS_FULL(pcQ))
	{
		status = CQUEUE_FULL;
		goto exit;
	}
	cQueue_Memcpy(_WriteAdr(pcQ), (uint8_t*)pdata, pcQ->unitSize);
	_WriteMove(pcQ, 1);

	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 添加多个数据单元到队列
  * @param pcQ 队列句柄
  * @param pdata 数据
  * @retval cQueueStatus OK/FULL/BUSY
  */
cQueueStatus cQueue_Pushs(cQueue_t* pcQ, void* pdata, uint16_t size)
{
	cQueueAssert(pcQ);
	cQueueAssert(pcQ->isInit);
	cQueueAssert(pcQ->len >= size);

	cQueueStatus status = CQUEUE_OK;
	uint16_t Limit;
	uint16_t spare;

	__CQUEUE_LOCK(pcQ);
	if (pcQ->pWrite >= pcQ->pRead)
	{
		spare = pcQ->len - pcQ->pWrite + pcQ->pRead;
		if (pcQ->full && pcQ->pWrite == pcQ->pRead) spare = 0;
		if (spare < size)//没有足量空间
		{
			status = CQUEUE_FULL; 
			goto exit;
		}
		Limit = pcQ->len - pcQ->pWrite;
		if (Limit >= size)
		{
			cQueue_Memcpy(_WriteAdr(pcQ), pdata, pcQ->unitSize * size);
		}
		else
		{
			cQueue_Memcpy(_WriteAdr(pcQ), pdata, pcQ->unitSize * Limit);
			pdata = (uint8_t*)pdata + pcQ->unitSize * Limit;
			cQueue_Memcpy(pcQ->data, pdata, pcQ->unitSize * (size - Limit));
		}
	}
	else //(pcQ->pWrite < pcQ->pRead)
	{
		Limit = pcQ->pRead - pcQ->pWrite;
		if (Limit < size)  //没有足量空间
		{
			status =  CQUEUE_FULL;
			goto exit;
		}
		cQueue_Memcpy(_WriteAdr(pcQ), pdata, pcQ->unitSize * size);
	}
	_WriteMove(pcQ, size);
	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 添加多个数据单元到队列(空间不足则覆盖未读数据)
  * @param pcQ 队列句柄
  * @param pdata 数据
  * @retval cQueueStatus OK/BUSY
  */
cQueueStatus cQueue_OverWrite(cQueue_t* pcQ, void* pdata, uint16_t size)
{
	cQueueAssert(pcQ);
	cQueueAssert(pcQ->isInit);
	cQueueAssert(pcQ->len >= size);

	cQueueStatus status = CQUEUE_OK;
	uint16_t Limit;
	uint16_t spare;

	__CQUEUE_LOCK(pcQ);
	spare = cQueue_Spare(pcQ);
	Limit = pcQ->len - pcQ->pWrite;
	if (Limit >= size)
	{
		cQueue_Memcpy(_WriteAdr(pcQ), pdata, pcQ->unitSize * size);
	}
	else
	{
		cQueue_Memcpy(_WriteAdr(pcQ), pdata, pcQ->unitSize * Limit);
		pdata = (uint8_t*)pdata + pcQ->unitSize * Limit;
		cQueue_Memcpy(pcQ->data, pdata, pcQ->unitSize * (size - Limit));
	}

	_WriteMove(pcQ, size);
	if (spare < size) //写指针超过了读指针
	{
		pcQ->full = 1;
		pcQ->pRead = pcQ->pWrite;
	}
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 拿出数据单元
  * @param pcQ 队列句柄
  * @param pReceive 接收数据的地址
  * @retval cQueueStatus OK/NULL/BUSY
  */
cQueueStatus cQueue_Pop(cQueue_t *pcQ, void* pReceive)
{
	cQueueAssert(pcQ);
	cQueueAssert(pReceive);
	cQueueStatus status = CQUEUE_OK;
	__CQUEUE_LOCK(pcQ);
	if (cQueue_IS_NULL(pcQ))
	{
		status = CQUEUE_NULL;
		goto exit;
	}
	cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize);
	_ReadMove(pcQ, 1);

	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 拿出多个数据单元
  * @param pcQ 队列句柄
  * @param pReceive 接收数据的地址
  * @retval cQueueStatus OK/NULL/BUSY
  */
cQueueStatus cQueue_Pops(cQueue_t *pcQ, void* pReceive, uint16_t size)
{
	cQueueAssert(pcQ);
	cQueueAssert(pReceive);
	cQueueAssert(pcQ->len >= size);

	cQueueStatus status = CQUEUE_OK;
	uint16_t Limit;
	uint16_t use;

	__CQUEUE_LOCK(pcQ);
	if (pcQ->pWrite > pcQ->pRead)
	{
		Limit = pcQ->pWrite - pcQ->pRead;
		if (Limit < size)  //没有足量数据
		{
			status = CQUEUE_NULL;
			goto exit;
		}
		cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize * size);
	}
	else //(pcQ->pWrite <= pcQ->pRead)
	{
		use = pcQ->len - pcQ->pRead + pcQ->pWrite;
		if (!pcQ->full && pcQ->pWrite == pcQ->pRead) use = 0;
		if (use < size) //没有足量数据
		{
			status = CQUEUE_NULL; 
			goto exit;
		}
		Limit = pcQ->len - pcQ->pRead;
		if (Limit >= size)
		{
			cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize * size);
		}
		else
		{
			cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize * Limit);
			pReceive = (uint8_t*)pReceive + pcQ->unitSize * Limit;
			cQueue_Memcpy(pReceive, pcQ->data, pcQ->unitSize * (size - Limit));
		}
	}
	_ReadMove(pcQ, size);
	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 偷看数据单元
  * @param pcQ 队列句柄
  * @param pReceive 接收数据的地址
  * @retval cQueueStatus OK/NULL
  */
cQueueStatus cQueue_Peek(cQueue_t *pcQ, void* pReceive)
{
	cQueueAssert(pcQ);

	if (cQueue_IS_NULL(pcQ))
		return CQUEUE_NULL;
	cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize);
	return CQUEUE_OK;
}

 /**
  * @brief 偷看多个数据单元
  * @param pcQ 队列句柄
  * @param pReceive 接收数据的地址
  * @param size 偷看的长度
  * @retval cQueueStatus OK/NULL
  */
cQueueStatus cQueue_Peeks(cQueue_t *pcQ, void* pReceive, uint16_t size)
{
	cQueueAssert(pcQ);
	cQueueAssert(pReceive);
	cQueueAssert(pcQ->len >= size);

	cQueueStatus status = CQUEUE_OK;
	uint16_t Limit;
	uint16_t use;

	if (pcQ->pWrite > pcQ->pRead)
	{
		Limit = pcQ->pWrite - pcQ->pRead;
		if (Limit < size)  //没有足量数据
		{
			status = CQUEUE_NULL;
			goto exit;
		}
		cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize * size);
		//_ReadMove(pcQ, size);
	}
	else //(pcQ->pWrite <= pcQ->pRead)
	{
		use = pcQ->len - pcQ->pRead + pcQ->pWrite;
		if (pcQ->full && pcQ->pWrite == pcQ->pRead) use = pcQ->len;
		if (use < size) //没有足量数据
		{
			status = CQUEUE_NULL; 
			goto exit;
		}
		Limit = pcQ->len - pcQ->pRead;
		if (Limit >= size)
		{
			cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize * size);
		}
		else
		{
			cQueue_Memcpy(pReceive, _ReadAdr(pcQ), pcQ->unitSize * Limit);
			pReceive = (uint8_t*)pReceive + pcQ->unitSize * Limit;
			cQueue_Memcpy(pReceive, pcQ->data, pcQ->unitSize * (size - Limit));
		}
	}
	exit:
	return status;
}

/**
 * @brief 得到剩余空间
 * @param pcQ 队列句柄
 * @retval uint16_t 
 */
uint16_t cQueue_Spare(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	uint16_t spare = 0;
	if (pcQ->pRead > pcQ->pWrite)
	{
		spare = pcQ->pRead - pcQ->pWrite;
	}
	else if (pcQ->pRead < pcQ->pWrite)
	{
		spare = pcQ->len - pcQ->pWrite + pcQ->pRead;
	}
	else if(pcQ->full == 0) // pWrite == pRead
	{
		spare = pcQ->len;
	}
	return spare;
}

/**
 * @brief 清空队列
 * @param pcQ 队列句柄
 * @retval uint16_t 
 */
void cQueue_Clear(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	pcQ->pWrite = 0;
	pcQ->pRead = 0;
	pcQ->full = 0;
}

/**
 * @brief 跳过队列单元
 * @param pcQ 队列句柄
 * @param len 跳过的长度，超过已使用量相当于清空未读数据
 * @retval int 实际跳过的长度
 */
int cQueue_Skip(cQueue_t *pcQ, uint16_t len)
{
	cQueueAssert(pcQ);
	cQueueAssert(len > 0);

	int use = pcQ->len - cQueue_Spare(pcQ);
	if (len > use)
		len = use;
	_ReadMove(pcQ, len);
	return len;
}

/**
  * @brief  销毁队列
  * @param  pcQ 队列句柄 
  */
cQueueStatus cQueue_Destroy(cQueue_t* pcQ)
{
	cQueueAssert(pcQ);
	cQueueAssert(pcQ->canFree);

	if (pcQ->lock == CQUEUE_LOCKED)
	{
		return CQUEUE_BUSY;
	}
	cQueue_Free(pcQ->data);
	cQueue_Free(pcQ);
	return CQUEUE_OK;
}




