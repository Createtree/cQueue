#include "cQueue.h"

#define _cQueue_IS_FULL(cQueue) ((cQueue->pWrite - cQueue->pWrite) >= cQueue->size ? 1 : 0)
#define _cQueue_IS_NULL(cQueue) ((cQueue->pWrite == cQueue->pRead) ? 1 : 0)
#define cQueue_Spare (cQueue->size - (cQueue->pWrite - cQueue->pRead))
/**
  * @brief  �������� 
  * @param  ���е����֡������ 
  * @notes  ��ʹ�üǵû��� 
  * @retval ���о�� 
  */
cQueue_t* cQueue_Create(uint8_t FrameBuf_size)
{
	cQueue_t* pcQ = cQueue_Malloc(sizeof(cQueue_t));
	if(pcQ != NULL)
	{
		pcQ->FrameBuf = cQueue_Malloc(FrameBuf_size*sizeof(cQueue_Frame_t));
		pcQ->size = FrameBuf_size;
		pcQ->clear = 0;
		pcQ->pRead = 0;
		pcQ->pWrite = 0;
	}


	return pcQ;
}


/**
  * @brief  ��Ӷ���֡
  * @param  ���о��
  * @param  ֡���ݵ��׵�ַ
  * @param	֡���ݵĳ���
  * @param	�����ͷ� 
  * @notes  ֡�Ĵ���Ϊ���õĴ���
  * @retval �ɹ�:1,ʧ��:0 
  */
uint8_t cQueue_AddFrame(cQueue_t* pcQ, uint8_t** data, uint8_t len, uint8_t CanFree)
{
	int pos = pcQ->pWrite%pcQ->size;
	if(!_cQueue_IS_FULL(pcQ))
	{
		pcQ->FrameBuf[pos].size = len;
		pcQ->FrameBuf[pos].data = *data;
		pcQ->FrameBuf[pos].is_read = 0;
		pcQ->FrameBuf[pos].canFree = CanFree; 
		pcQ->pWrite++;
		if(pcQ->pWrite > pcQ->size && pcQ->pRead > pcQ->size)//����ʱ����������
		{
			pcQ->pRead -= pcQ->size;
			pcQ->pWrite -= pcQ->size;
		}
		return 1;
	}
	return 0;
}

/**
  * @brief  ��Ӷ���֡�Ŀ���
  * @param  ���о��
  * @param  ֡���ݵ��׵�ַ
  * @param	֡���ݵĳ���
  * @notes  ֡�Ĵ���Ϊ�����Ĵ���
  * @retval �ɹ�:1,ʧ��:0 
  */
uint8_t cQueue_AddFrameCopy(cQueue_t* pcQ, uint8_t** data, uint8_t len)
{
	uint8_t* dataCopy = cQueue_Memcpy(cQueue_Malloc(len),*data,len);
	return cQueue_AddFrame(pcQ,&dataCopy,len,1);
}

/**
  * @brief  ��Ӹ�ʽ��֡�������� 
  * @param  ���о�� 
  * @notes  �����ķ�ʽ��ע���ʽ�������С 
  * @retval ��ʽ�������ݵĳ��� 
  */
unsigned char* cQueueBuf[128];
uint8_t cQueue_AddFormatData(cQueue_t* pcQ, const char* fmt,...)
{
	uint8_t len = 0;
	int pos = (pcQ->pWrite)%pcQ->size;
	if(pcQ->FrameBuf[pos].data != NULL && !_cQueue_IS_FULL(pcQ))
	{
		va_list ap;
		va_start(ap,fmt);
		len = vsprintf((char*)cQueueBuf,fmt,ap);
		va_end(ap);
		pcQ->FrameBuf[pos].is_read = 0;
		pcQ->FrameBuf[pos].size = len;
		pcQ->FrameBuf[pos].data = (uint8_t*)memcpy(cQueue_Malloc(len),cQueueBuf,len);
		pcQ->FrameBuf[pos].canFree = 1;
		pcQ->pWrite++;
		return len;
	}
	return 0;

}


/**
  * @brief  �ͷ�֡���ݵ��ڴ�
  * @param  ���о�� 
  * @notes  ����֡���ݵ��ڴ� 
  * @retval None
  */
void cQueue_Free_FrameData(cQueue_t* pcQ)
{
	int i = pcQ->size;
	cQueue_Frame_t* cQF = NULL;
	while(i--)
	{
		cQF = &pcQ->FrameBuf[i];
		if(cQF->is_read == 1 && cQF->canFree == 1)
		{
			cQueue_Free(cQF->data);
			cQF->size = 0;
			cQF->canFree = 0;
			if(--pcQ->clear == 0)break;
		}
	}
}


/**
  * @brief  �Ӷ��еõ���Ϣ 
  * @param  ���о�� 
  * @notes  ��ȡ�������ݵ����� 
  * @retval ����֡��ʽ 
  */
cQueue_Frame_t* cQueue_GetFrame(cQueue_t* pcQ)
{
	if(pcQ->clear > 0)cQueue_Free_FrameData(pcQ);
	if(_cQueue_IS_NULL(pcQ))
	{
		return NULL;
	}
	pcQ->FrameBuf[pcQ->pRead%pcQ->size].is_read = 1;
	pcQ->clear++;
	return &pcQ->FrameBuf[pcQ->pRead++%pcQ->size];
	
}

/**
  * @brief  ɾ������
  * @param  ���о�� 
  * @notes  ������ʹ��ʱɾ�� 
  * @retval None 
  */
void cQueue_Delete(cQueue_t* pcQ)
{
//	assert(pcQ != NULL);
	for (size_t i = 0; i < pcQ->size; i++)
	{
		cQueue_GetFrame(pcQ);
	}
	cQueue_Free(pcQ->FrameBuf);
	cQueue_Free(pcQ);
}




