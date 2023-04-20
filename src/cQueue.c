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
	if (pcQ->pWrite >= pcQ->len)//����ȡ�࣬����ٶ�
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
 * @brief ��������
 * @param UnitSize ���е�Ԫ�ߴ�(Byte)
 * @param Length ���е�Ԫ����
 * @retval cQueue_t* ���о��
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
 * @brief ��̬�������� 
 * @param pcQueue ���о��
 * @param pdata ��������ָ��
 * @param UnitSize ���е�Ԫ�ߴ�(byte)
 * @param Length ���е�Ԫ����
 * @retval cQueue_t* ���о��
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
  * @brief ������ݵ�Ԫ������
  * @param pcQ ���о��
  * @param pdata ����
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
  * @brief ��Ӷ�����ݵ�Ԫ������
  * @param pcQ ���о��
  * @param pdata ����
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
		if (spare < size)//û�������ռ�
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
		if (Limit < size)  //û�������ռ�
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
  * @brief ��Ӷ�����ݵ�Ԫ������(�ռ䲻���򸲸�δ������)
  * @param pcQ ���о��
  * @param pdata ����
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
	if (spare < size) //дָ�볬���˶�ָ��
	{
		pcQ->full = 1;
		pcQ->pRead = pcQ->pWrite;
	}
	__CQUEUE_UNLOCK(pcQ);
	return status;
}

 /**
  * @brief �ó����ݵ�Ԫ
  * @param pcQ ���о��
  * @param pReceive �������ݵĵ�ַ
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
  * @brief �ó�������ݵ�Ԫ
  * @param pcQ ���о��
  * @param pReceive �������ݵĵ�ַ
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
		if (Limit < size)  //û����������
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
		if (use < size) //û����������
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
  * @brief ͵�����ݵ�Ԫ
  * @param pcQ ���о��
  * @param pReceive �������ݵĵ�ַ
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
  * @brief ͵��������ݵ�Ԫ
  * @param pcQ ���о��
  * @param pReceive �������ݵĵ�ַ
  * @param size ͵���ĳ���
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
		if (Limit < size)  //û����������
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
		if (use < size) //û����������
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
 * @brief �õ�ʣ��ռ�
 * @param pcQ ���о��
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
 * @brief ��ն���
 * @param pcQ ���о��
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
 * @brief �������е�Ԫ
 * @param pcQ ���о��
 * @param len �����ĳ��ȣ�������ʹ�����൱�����δ������
 * @retval int ʵ�������ĳ���
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
  * @brief  ���ٶ���
  * @param  pcQ ���о�� 
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




