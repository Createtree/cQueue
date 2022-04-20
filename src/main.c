#include <stdio.h>
#include "cQueue.h"
#include "mleak.h"

static int main_ret = 0;
int test_count = 0;
int test_pass = 0;

#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality)\
            test_pass++;\
        else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_cQueueAddFrame(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")



void Test_cQueue_AddFormatData(void)
{
	cQueue_t* Uart1Queue;
	Uart1Queue = cQueue_Create(10);
	char *testData = "AddFormatData";
	cQueue_Frame_t* cQF = NULL;
	for (size_t i = 0; i < 30; i++)
	{
		cQueue_AddFormatData(Uart1Queue,testData+i%14);
		cQF = cQueue_GetFrame(Uart1Queue);
		int c = 1;
		if(cQF && cQF->data)
		{
			c = memcmp(cQF->data,testData+i%14,cQF->size);
			// printf("%d:%s:%d\n",i,cQF->data,c);
		}
		EXPECT_EQ_cQueueAddFrame(0,c);
	}
	cQueue_Delete(Uart1Queue);
}

void Test_cQueue_AddFrame()
{
	cQueue_t* Uart1Queue;
	Uart1Queue = cQueue_Create(10);
	uint8_t *testData = "AddFrame ";

	for (size_t i = 31; i < 100; i++)
	{
		int c = 1;
		testData = (char*)malloc(10);
		memset(testData,i,9);testData[9]='\0';
		cQueue_AddFrame(Uart1Queue,&testData,10,1);
		cQueue_Frame_t* cQF = cQueue_GetFrame(Uart1Queue);
		if(cQF && cQF->data)
		{
			c = memcmp(cQF->data,testData,cQF->size);
			//printf("%d:%s:%d\n",i,cQF->data,c);	
		}
		EXPECT_EQ_cQueueAddFrame(0,c);	
	}
	cQueue_Delete(Uart1Queue);
}

void Test_cQueue_AddFrameCopy()
{
	cQueue_t* Uart1Queue;
	Uart1Queue = cQueue_Create(10);
	char *Text = "AddFrameCopy";
	char *testData = Text;
	for (size_t i = 0; i < 30; i++)
	{
		int c = 1;
		testData = Text + i%12;
		cQueue_AddFrameCopy(Uart1Queue,&(testData),12-i%12);
		cQueue_Frame_t* cQF = cQueue_GetFrame(Uart1Queue);
		if(cQF && cQF->data)
		{
			c = memcmp(cQF->data,testData,cQF->size);
			// printf("%d:%s:%d\n",i,cQF->data,c);
		}
		EXPECT_EQ_cQueueAddFrame(0,c);
	}
	cQueue_Delete(Uart1Queue);
 
}
void cQueue_Test(void)
{
	Test_cQueue_AddFormatData();
	Test_cQueue_AddFrame();
	Test_cQueue_AddFrameCopy();
	
}
int main()
{
	
	cQueue_Test();//µ¥Ôª²âÊÔ
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	MALLOC(4);
	PRINT_LEAK_INFO();//ÄÚ´æÐ¹Â©²âÊÔ
	return (0);
}