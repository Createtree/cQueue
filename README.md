# cQueue
>基于C语言的循环队列，采用引用或拷贝方式来传递帧格式数据，并实现了自动管理内存
## API
```C
cQueue_t* cQueue_Create(uint8_t FrameBuf_size);
uint8_t cQueue_AddFrame(cQueue_t* pcQ, uint8_t** data, uint8_t len, uint8_t CanFree);
uint8_t cQueue_AddFrameCopy(cQueue_t* pcQ, uint8_t** data, uint8_t len);
uint8_t cQueue_AddFormatData(cQueue_t* pcQ, const char* fmt,...);
void cQueue_Free_FrameData(cQueue_t* pcQ);
cQueue_Frame_t* cQueue_GetFrame(cQueue_t* pcQ);
void cQueue_Delete(cQueue_t* pcQ);
```
## 使用方法
```C
/**
  * @brief  创建队列 
  * @param  队列的最大帧容纳量 
  * @notes  不使用记得回收 
  * @retval 队列句柄 
  */
cQueue_t* cQueue_Create(uint8_t FrameBuf_size)



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


/**
  * @brief  添加队列帧的拷贝
  * @param  队列句柄
  * @param  帧数据的首地址
  * @param	帧数据的长度
  * @notes  帧的传递为拷贝的传递
  * @retval 成功:1,失败:0 
  */
uint8_t cQueue_AddFrameCopy(cQueue_t* pcQ, uint8_t** data, uint8_t len)


/**
  * @brief  添加格式化帧类型数据 
  * @param  队列句柄 
  * @notes  拷贝的方式，注意格式化缓存大小 
  * @retval 格式化后数据的长度 
  */
unsigned char* cQueueBuf[128];
uint8_t cQueue_AddFormatData(cQueue_t* pcQ, const char* fmt,...)



/**
  * @brief  释放帧数据的内存
  * @param  队列句柄 
  * @notes  回收帧数据的内存 
  * @retval None
  */
void cQueue_Free_FrameData(cQueue_t* pcQ)



/**
  * @brief  从队列得到消息 
  * @param  队列句柄 
  * @notes  获取发送数据的引用 
  * @retval 数据帧格式 
  */
cQueue_Frame_t* cQueue_GetFrame(cQueue_t* pcQ)


/**
  * @brief  删除队列
  * @param  队列句柄 
  * @notes  请勿在使用时删除 
  * @retval None 
  */
void cQueue_Delete(cQueue_t* pcQ)
```

Example:
```C
cQueue_t* Uart1Queue = cQueue_Create(10); //申请一个最大缓存10帧数据的队列对象
char* RData;
int len;
char *testData = "AddFormatData";//需要发送的数据
cQueue_AddFrame(Uart1Queue,&testData,10,0);//发送该数据(该数据内存不需要回收)
cQueue_Frame_t* cQF = cQueue_GetFrame(Uart1Queue);//接收该帧
if(cQF && cQF->data)//不为空
{
    RData = cQF->data;//得到数据
    len = cQF->len;//数据的长度        
}
cQueue_Delete(Uart1Queue);//删除该队列

```