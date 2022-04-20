#include "cQueue.h"

#define _cQueue_IS_FULL(cQueue) ((cQueue->pWrite - cQueue->pWrite) >= cQueue->size ? 1 : 0)
#define _cQueue_IS_NULL(cQueue) ((cQueue->pWrite == cQueue->pRead) ? 1 : 0)
#define cQueue_Spare (cQueue->size - (cQueue->pWrite - cQueue->pRead))
/**
  * @brief  创建队列 
  * @param  队列的最大帧容纳量 
  * @notes  不使用记得回收 
  * @retval 队列句柄 
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
  * @brief  添加队列帧
  * @param  队列句柄
  * @param  帧数据的首地址
  * @param	帧数据的长度
  * @param	可以释放 
  * @notes  帧的传递为引用的传递
  * @retval 成功:1,失败:0 
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
		if(pcQ->pWrite > pcQ->size && pcQ->pRead > pcQ->size)//空闲时减少冗余数
		{
			pcQ->pRead -= pcQ->size;
			pcQ->pWrite -= pcQ->size;
		}
		return 1;
	}
	return 0;
}

/**
  * @brief  添加队列帧的拷贝
  * @param  队列句柄
  * @param  帧数据的首地址
  * @param	帧数据的长度
  * @notes  帧的传递为拷贝的传递
  * @retval 成功:1,失败:0 
  */
uint8_t cQueue_AddFrameCopy(cQueue_t* pcQ, uint8_t** data, uint8_t len)
{
	uint8_t* dataCopy = cQueue_Memcpy(cQueue_Malloc(len),*data,len);
	return cQueue_AddFrame(pcQ,&dataCopy,len,1);
}

/**
  * @brief  添加格式化帧类型数据 
  * @param  队列句柄 
  * @notes  拷贝的方式，注意格式化缓存大小 
  * @retval 格式化后数据的长度 
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
  * @brief  释放帧数据的内存
  * @param  队列句柄 
  * @notes  回收帧数据的内存 
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
  * @brief  从队列得到消息 
  * @param  队列句柄 
  * @notes  获取发送数据的引用 
  * @retval 数据帧格式 
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
  * @brief  删除队列
  * @param  队列句柄 
  * @notes  请勿在使用时删除 
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




