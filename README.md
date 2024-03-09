# cQueue

> 基于C语言的泛型队列

[项目地址 Createtree/cQueue: Generic Queue Based on C (github.com)](https://github.com/Createtree/cQueue)

## API

```C
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
```

## 使用方法

### 创建队列

动态创建

```C
// Global variable
cQueue_t *pcQ;
// Dynamic create (need malloc)
pcQ = cQueue_Create(sizeof(int), 10);
if(pcQ == NULL)
{
  // do something that...
}
```

静态创建

```C
// Global variable
cQueue_t cQStaticHandle;
int dataBuf[10];
// cQueue create static
cQueue_Create_Static(&cQStaticHandle, &dataBuf, sizeof(int), 10)
```

### 写入元素

写入一个

```C
int a = 10;
if (cQueue_Push(pcQ, &a) == CQUEUE_OK)
{
  // Successfully added a data
}
```

写入指定长度

```C
int arr[5] = {1,2,3,4,5};
if (cQueue_Pushs(pcQ, &arr, 5) == CQUEUE_OK)
{
  // Successfully added the array
}
```

### 读出元素

弹出一个

```C
int a;
if (cQueue_Pop(pcQ, &a) == CQUEUE_OK)
{
  // Successfully pop a data 
}
```

弹出指定长度

```C
int arr[5];
if (cQueue_Pops(pcQ, &arr, 5) == CQUEUE_OK)
{
  // Successfully popped 5 data to arr
}
```

偷看 (读出之后任然保留在缓存中)

```C
int a;
if (cQueue_Peek(pcQ, &a) == CQUEUE_OK)
{
  // Successfully peek a data 
}
```

### 销毁

```C
if (cQueue_Destroy(pcQ) != CQUEUE_OK)
{
  // May be in use during interrupts or other threads
}
```

## Update Log

### v1.1-2024.3.9

> - 新增可变长度数据的操作 `pushv/popv`
> - 新增队列使用量(Usage)、满状态(Full)、空状态(Empty)的获取
> - 新增获取队列读写指针到末端的空余量的获取功能
> - 开放了读指针写指针内存地址的获取接口，通过该接口可用方便DMA搬运数据或者外部操作数据

### v1.0-2023.4.17

> - Rewritten cQueue that supports generics
> - Unit testing completed
