#include "cQueue.h"
#define cQueue_WROFFSET(cQueue) (((cQueue)->pWrite > (cQueue)->pRead) ? \
							(cQueue)->pWrite - (cQueue)->pRead : \
							(cQueue)->pRead - (cQueue)->pWrite)
#define cQueue_IS_FULL(cQueue) (((cQueue)->pWrite == (cQueue)->pRead) && (cQueue->full) ? 1 : 0)
#define cQueue_IS_NULL(cQueue) (((cQueue)->pWrite == (cQueue)->pRead) && (!cQueue->full) ? 1 : 0)
#define cQueue_WriteAdr(cQueue) ((uint8_t*)(cQueue)->data + (cQueue)->pWrite * (cQueue)->unitSize)
#define cQueue_ReadAdr(cQueue)  ((uint8_t*)(cQueue)->data + (cQueue)->pRead * (cQueue)->unitSize)

static inline void cQueue_WriteMove(cQueue_t *pcQ, uint16_t len)
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

static inline void cQueue_ReadMove(cQueue_t *pcQ, uint16_t len)
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
 * @retval cQueue_t *队列句柄
 */
cQueue_t *cQueue_Create(uint8_t UnitSize, uint16_t Length)
{
	cQueueAssert(UnitSize && Length);

	cQueue_t *pcQ = cQueue_Malloc(sizeof(cQueue_t));
	if(pcQ != NULL)
	{
		pcQ->data = cQueue_Malloc(Length * UnitSize);
		if (pcQ->data == NULL)
		{
			cQueue_Free(pcQ);
			return NULL;
		}
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
 * @retval cQueue_t *队列句柄
 */
void cQueue_Create_Static(cQueue_t *pcQueue, void *pdata, uint8_t UnitSize, uint16_t Length)
{
	cQueueAssert(pcQueue && pdata);
	cQueueAssert(UnitSize && Length);

	cQueue_t *pcQ = pcQueue;
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
 * @brief 获取队列总单元数
 * @param pcQ 队列句柄
 * @retval uint16_t 总元素单元数
 */
uint16_t cQueue_GetLength(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	return pcQ->len;
}

/**
 * @brief 获取当前队列是否为空
 * @param pcQ 队列句柄
 * @retval uint16_t 1:当前队列为空/0:当前队列非空
 */
uint16_t cQueue_Empty(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	return !!(cQueue_IS_NULL(pcQ));
}

/**
 * @brief 获取当前队列是否已满
 * @param pcQ 队列句柄
 * @retval uint16_t 1:当前队列已满/0:当前队列未满
 */
uint16_t cQueue_Full(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	return !(cQueue_IS_NULL(pcQ));
}

/**
 * @brief 获取读指针地址
 * @param pcQ 队列句柄
 * @retval void *
 */
void *cQueue_GetReadAdr(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	return cQueue_ReadAdr(pcQ);
}

/**
 * @brief 获取写指针地址
 * @param pcQ 队列句柄
 * @retval void *
 */
void *cQueue_GetWriteAdr(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	return cQueue_WriteAdr(pcQ);
}

/**
 * @brief 获取读指针到末端的已用容量
 * @param pcQ 队列句柄
 * @retval uint16_t 已用容量[0, len]
 */
uint16_t cQueue_GetReadPtrMargin(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	uint16_t size = 0;
	if (pcQ->pWrite > pcQ->pRead)
	{
		size = pcQ->pWrite - pcQ->pRead;
	}
	else if (pcQ->pWrite < pcQ->pRead)
	{
		size = pcQ->len - pcQ->pRead;
	}
	else if (pcQ->full == 1)
	{
		size = pcQ->len - pcQ->pRead;
	}
	return size;
}

/**
 * @brief 获取写指针到末端的可用容量
 * @param pcQ 队列句柄
 * @retval uint16_t 可用容量[0, len]
 */
uint16_t cQueue_GetWritePtrMargin(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	uint16_t size = 0;
	if (pcQ->pWrite > pcQ->pRead)
	{
		size = pcQ->len - pcQ->pWrite;
	}
	else if (pcQ->pWrite < pcQ->pRead)
	{
		size = pcQ->pRead - pcQ->pWrite;
	}
	else if (pcQ->full != 1)
	{
		size = pcQ->len - pcQ->pWrite;
	}
	return size;
}

 /**
  * @brief 添加数据单元到队列
  * @param pcQ 队列句柄
  * @param pdata 数据
  * @retval cQueueState OK/FULL/BUSY
  */
cQueueStatus cQueue_Push(cQueue_t *pcQ, void *pdata)
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
	cQueue_Memcpy(cQueue_WriteAdr(pcQ), (uint8_t*)pdata, pcQ->unitSize);
	cQueue_WriteMove(pcQ, 1);

	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 添加多个数据单元到队列
  * @param pcQ 队列句柄
  * @param pdata 数据
  * @param size 数据长度
  * @retval cQueueStatus OK/FULL/BUSY
  */
cQueueStatus cQueue_Pushs(cQueue_t *pcQ, void *pdata, uint16_t size)
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
			cQueue_Memcpy(cQueue_WriteAdr(pcQ), pdata, pcQ->unitSize * size);
		}
		else
		{
			cQueue_Memcpy(cQueue_WriteAdr(pcQ), pdata, pcQ->unitSize * Limit);
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
		cQueue_Memcpy(cQueue_WriteAdr(pcQ), pdata, pcQ->unitSize * size);
	}
	cQueue_WriteMove(pcQ, size);
	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 添加多个数据单元到队列(空间不足则覆盖未读数据)
  * @param pcQ 队列句柄
  * @param pdata 数据
  * @param size 数据长度
  * @retval cQueueStatus OK/BUSY
  */
cQueueStatus cQueue_OverWrite(cQueue_t *pcQ, void *pdata, uint16_t size)
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
		cQueue_Memcpy(cQueue_WriteAdr(pcQ), pdata, pcQ->unitSize * size);
	}
	else
	{
		cQueue_Memcpy(cQueue_WriteAdr(pcQ), pdata, pcQ->unitSize * Limit);
		pdata = (uint8_t*)pdata + pcQ->unitSize * Limit;
		cQueue_Memcpy(pcQ->data, pdata, pcQ->unitSize * (size - Limit));
	}

	cQueue_WriteMove(pcQ, size);
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
cQueueStatus cQueue_Pop(cQueue_t *pcQ, void *pReceive)
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
	cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize);
	cQueue_ReadMove(pcQ, 1);

	exit:
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief 拿出多个数据单元
  * @param pcQ 队列句柄
  * @param pReceive 接收数据的地址
  * @param size 缓存的大小
  * @retval cQueueStatus OK/NULL/BUSY
  */
cQueueStatus cQueue_Pops(cQueue_t *pcQ, void *pReceive, uint16_t size)
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
		cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize * size);
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
			cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize * size);
		}
		else
		{
			cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize * Limit);
			pReceive = (uint8_t*)pReceive + pcQ->unitSize * Limit;
			cQueue_Memcpy(pReceive, pcQ->data, pcQ->unitSize * (size - Limit));
		}
	}
	cQueue_ReadMove(pcQ, size);
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
cQueueStatus cQueue_Peek(cQueue_t *pcQ, void *pReceive)
{
	cQueueAssert(pcQ);

	if (cQueue_IS_NULL(pcQ))
		return CQUEUE_NULL;
	cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize);
	return CQUEUE_OK;
}

 /**
  * @brief 偷看多个数据单元
  * @param pcQ 队列句柄
  * @param pReceive 接收数据的地址
  * @param size 偷看的长度
  * @retval cQueueStatus OK/NULL
  */
cQueueStatus cQueue_Peeks(cQueue_t *pcQ, void *pReceive, uint16_t size)
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
		cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize * size);
		//cQueue_ReadMove(pcQ, size);
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
			cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize * size);
		}
		else
		{
			cQueue_Memcpy(pReceive, cQueue_ReadAdr(pcQ), pcQ->unitSize * Limit);
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
 * @retval uint16_t [0, len]
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
 * @brief 得到已用空间
 * @param pcQ 队列句柄
 * @retval uint16_t 已使用的单元数量[0, len]
 */
uint16_t cQueue_Usage(cQueue_t *pcQ)
{
	cQueueAssert(pcQ);
	uint16_t usage = 0;
	if (pcQ->pRead > pcQ->pWrite)
	{
		usage = pcQ->len - pcQ->pRead + pcQ->pWrite;
	}
	else if (pcQ->pRead < pcQ->pWrite)
	{
		usage = pcQ->pWrite - pcQ->pRead;
	}
	else if(pcQ->full != 0) // pWrite == pRead
	{
		usage = pcQ->len;
	}
	return usage;
}

/**
 * @brief 写入变长度数据包 (size+data)
 * @param pcQ cQueue ptr
 * @param pdata data ptr
 * @param size data size
 * @retval cQueueStatus OK/NULL/BUSY
 * @note unitSize 必须设定为 1
 */
cQueueStatus cQueue_Pushv(cQueue_t *pcQ, void *pdata, uint16_t size)
{
	cQueueAssert(pcQ);
	cQueueAssert(pdata);
	cQueueAssert(pcQ->unitSize == 1);

    cQueueStatus status;
    status = cQueue_Pushs(pcQ, &size, 2);
    if (status != CQUEUE_OK)
    {
        return status;
    }
    status = cQueue_Pushs(pcQ, pdata, size);
    if (status != CQUEUE_OK)
    {
        return status;
    }
    return CQUEUE_OK;
}

/**
 * @brief 弹出变长数据包 (size+data)
 * @param pcQ cQueue ptr
 * @param pdata data buffer
 * @param size data buffer size
 * @retval data size
 * @note unitSize 必须设定为 1
 */
uint16_t cQueue_Popv(cQueue_t *pcQ, void *pdata, uint16_t size)
{
	cQueueAssert(pcQ);
	cQueueAssert(pdata);
	cQueueAssert(pcQ->unitSize == 1);

    cQueueStatus status;
    uint16_t dataSize;
    status = cQueue_Pops(pcQ, &dataSize, 2);
    if (status != CQUEUE_OK)
    {
        return 0;
    }
    if (dataSize > size)
    {
        /* 数据包太大只能存一部分到缓存区，其他的丢弃！ */
        status = cQueue_Pops(pcQ, pdata, size);
        if (status != CQUEUE_OK)
        {
            return 0;
        }
        cQueue_Skip(pcQ, dataSize - size);
        return size;
    }
    else
    {
        status = cQueue_Pops(pcQ, pdata, dataSize);
        if (status != CQUEUE_OK)
        {
            return status;
        }
        return dataSize;
    }
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

	int use = cQueue_Usage(pcQ);
	if (len > use)
		len = use;
	cQueue_ReadMove(pcQ, len);
	return len;
}

/**
 * @brief 移动写指针位置
 * @param pcQ 队列句柄
 * @param len 移动的长度
 */
void cQueue_MoveWrite(cQueue_t *pcQ, uint16_t len)
{
	cQueueAssert(pcQ);
	if (len == 0 && len > pcQ->len) return;
	cQueue_WriteMove(pcQ, len);
}

/**
 * @brief 移动读指针位置
 * @param pcQ 队列句柄
 * @param len 移动的长度
 */
void cQueue_MoveRead(cQueue_t *pcQ, uint16_t len)
{
	cQueueAssert(pcQ);
	if (len == 0 && len > pcQ->len) return;
	cQueue_ReadMove(pcQ, len);
}

/**
  * @brief  销毁队列
  * @param  pcQ 队列句柄 
  */
cQueueStatus cQueue_Destroy(cQueue_t *pcQ)
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

