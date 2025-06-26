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

#define EXPECT_EQ_INT(expect, actual) EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")
#define EXPECT_EQ_TESTARRAY(testData_, actData_, len)                         \
	do                                                                       \
	{                                                                        \
		test_count++;                                                        \
		if (compareTestDatas(testData_, actData_, len) == -1)                \
			test_pass++;                                                     \
		else                                                                 \
		{                                                                    \
			fprintf(stderr, "[%d]%s:%d:\n", test_count, __FILE__, __LINE__); \
			printTestDatas(testData_, len);                                  \
			printTestDatas(actData_, len);                                   \
			main_ret = 1;                                                    \
		}                                                                    \
	} while (0)
#define EXPECT_EQ_TESTTYPE(testData_, actData_, len) EXPECT_EQ_TESTARRAY(testData_, actData_, len*sizeof(testType))

typedef short testType;
#define TESTSIZE 20
#define TESTUNIT sizeof(testType)
testType DataBuf[TESTSIZE] = {0};
cQueue_t cQStatic;
cQueue_t* pcQ;
testType TestData[TESTSIZE] = {0,1,2,3,4,5,6,7,8,9};
testType TestData2[TESTSIZE+2] = {0,1,2,3,4,5,6,7,8,9,10,11};
int compareTestDatas(const void *td1, const void *td2, int size)
{
	int i;

	for (i = 0; i < size; i++)
	{
		if(((uint8_t*)td1)[i] != ((uint8_t*)td2)[i])
			return i;
	}
	return -1;
}

#define printTestDatas(td, size)               \
	do                                         \
	{                                          \
		int ii = size;                         \
		printf(#td ":\n    ");                 \
		while (ii--)                           \
			printf("%3d ", td[size - 1 - ii]); \
		printf("\n");                          \
	} while (0)

void Test_cQueue_Create(void)
{
	cQueue_Create_Static(&cQStatic, &DataBuf, TESTUNIT, TESTSIZE);
	pcQ = cQueue_Create(TESTUNIT, TESTSIZE);
	if(!pcQ) return;
	EXPECT_EQ_INT(TESTSIZE, cQueue_Spare(pcQ));
	EXPECT_EQ_INT(TESTSIZE, cQueue_Spare(&cQStatic));
}

void Test_cQueue_WRData(cQueue_t *p, testType *testData, int len)
{
    int i;
    testType actData[TESTSIZE] = {0};
    for (i = 0; i < len; i++)
    {
        cQueue_Push(p, &testData[i]);
    }
    while (i--)
	{
		cQueue_Pop(p, &actData[len - 1 - i]);
	}

	EXPECT_EQ_TESTTYPE(testData, actData, len);
}

void Test_cQueue_WRDatas(cQueue_t *p, testType *testData, int len)
{
	int i;
	testType actData[TESTSIZE] = {0};
	cQueue_Pushs(p, testData, len);
	cQueue_Pops(p, actData, len);
	EXPECT_EQ_TESTTYPE(testData, actData, len);
	memset(actData, 0, len);
	for (i = 0; i < len; i++)
    {
        cQueue_Pushs(p, &testData[i], 1);
    }
    while (i--)
	{
		cQueue_Pops(p, &actData[len - 1 - i], 1);
	}
	EXPECT_EQ_TESTTYPE(testData, actData, len);
}

void Test_cQueue_OverWrite(cQueue_t *p, testType *testData, int len)
{
	int i = 0;
	testType actData[TESTSIZE] = {0};
	testType test[TESTSIZE] = {0};
	// memset(actData, 11, len);
	cQueue_OverWrite(p, testData, len);
	cQueue_Peeks(p, actData, len);
	EXPECT_EQ_TESTTYPE(testData, actData, len);

	cQueue_OverWrite(p, test, TESTSIZE); //Ð´Âú0
	cQueue_Peeks(p, actData, TESTSIZE);
	EXPECT_EQ_TESTTYPE(test, actData, TESTSIZE);
	
	for (i = 0; i < len; i++)
	{
		cQueue_OverWrite(p, &testData[i], 1);
		test[TESTSIZE - len + i] = testData[i];
	}
	cQueue_Pops(p, actData, TESTSIZE);
	EXPECT_EQ_TESTTYPE(test, actData, len);

}

void Test_cQueue_FULL_NULL(cQueue_t *p, testType *testData, int testlen, int actlen)
{
	testType *actData = MALLOC(TESTUNIT * testlen);
	int i;
	int full = testlen - actlen;
	cQueue_Clear(p);
	if(actData == NULL || full <= 0)
	{
		assert(0);
		return;
	}
	for (i = 0; i < testlen; i++)
	{
		if (cQueue_Push(p, &testData[i]) == CQUEUE_FULL)
			full--;
	}
	EXPECT_EQ_INT(0, full);
	while (i--)
	{
		if (cQueue_Pop(p, &actData[testlen - 1 - i]) == CQUEUE_NULL)
			full++;
	}
	EXPECT_EQ_INT(testlen - actlen, full);

	full = 0;
	if (cQueue_Peek(p, &actData[0]) == CQUEUE_NULL)
		full++;
	EXPECT_EQ_INT(1, full);

	full = testlen - actlen;
	for (i = 0; i < testlen; i++)
	{
		if (cQueue_Pushs(p, &testData[i], 1) == CQUEUE_FULL)
			full--;
	}
	EXPECT_EQ_INT(0, full);
	while (i--)
	{
		if (cQueue_Pops(p, &actData[testlen - 1 - i], 1) == CQUEUE_NULL)
			full++;
	}
	EXPECT_EQ_INT(testlen - actlen, full);

	full = 0;
	cQueue_Push(p, &testData[0]);
	if (cQueue_Peeks(p, &actData[testlen - 1 - i], 2) == CQUEUE_NULL)
			full++;
	EXPECT_EQ_INT(1, full);

	FREE(actData);
}

void Test_cQueue_Peek(cQueue_t *p , testType *testData, int len)
{
	testType actData[TESTSIZE] = {0};
	int i = len;
	cQueue_Pushs(p, testData, len);
	while(i--)
	{
		cQueue_Peek(p, &actData[len - 1 - i]);
		cQueue_Skip(p, 1);
	}
	EXPECT_EQ_TESTTYPE(testData, actData, len);
}

void Test_cQueue_Peeks(cQueue_t *p, testType *testData, int len)
{
	testType actData[TESTSIZE] = {0};
	cQueue_Pushs(p, testData, len);
	cQueue_Peeks(p, actData, len);
	EXPECT_EQ_TESTTYPE(testData, actData, len);
	cQueue_Clear(p);
}

void Test_cQueue_WR_VariableLength(cQueue_t *p, uint8_t *testData, int len)
{
	int retlen;
	uint8_t *actData = MALLOC(len * sizeof(uint8_t));
	uint8_t unitSize = cQueue_Get_UnitSize(p);
	cQueue_Set_UnitSize(p, 1);
	cQueue_Pushv(p, testData, len);
	cQueue_Pushv(p, testData, len>>1);
	retlen = cQueue_Popv(p, actData, len);
	EXPECT_EQ_INT(len, retlen);
	EXPECT_EQ_TESTARRAY(testData, actData, len);
	retlen = cQueue_Popv(p, actData, len);
	EXPECT_EQ_INT(len>>1, retlen);
	EXPECT_EQ_TESTARRAY(testData, actData, (len>>1));
	FREE(actData);
	cQueue_Set_UnitSize(p, unitSize);
}

void Test_cQueue_WR_Margin(cQueue_t *p, testType *testData, int len)
{
	uint16_t retlen;
	cQueue_Clear(p);
	retlen = cQueue_GetReadPtrMargin(p);
	EXPECT_EQ_INT(0, retlen);
	retlen = cQueue_GetWritePtrMargin(p);
	EXPECT_EQ_INT(p->len, retlen);
	
	cQueue_Pushs(p, testData, len>>1);
	retlen = cQueue_GetWritePtrMargin(p);
	EXPECT_EQ_INT(p->len - (len>>1), retlen);
	retlen = cQueue_GetReadPtrMargin(p);
	EXPECT_EQ_INT(len>>1, retlen);
}

void Test_cQueue_Clear(cQueue_t *p)
{
	testType t;
	cQueue_Push(p, &t);
	cQueue_Clear(p);
	EXPECT_EQ_INT(1, cQueue_Empty(p));
	EXPECT_EQ_INT(p->len, cQueue_Spare(p));
}

void Test_cQueue_Full_Usage(cQueue_t *p)
{
	testType t;
	int cnt = 0;
	cQueue_Clear(p);
	for (size_t i = 0; i < p->len; i++)
	{
		cQueue_Push(p, &t);
		if (i+1 == cQueue_Usage(p))
		{
			cnt++;
		}
	}
	EXPECT_EQ_INT(p->len, cnt);
	EXPECT_EQ_INT(1, cQueue_Full(p));
	cQueue_Clear(p);
}

void Test_cQueue_Destroy()
{
	cQueue_Destroy(pcQ);
	pcQ = NULL;
}

void cQueue_Test(void)
{

	Test_cQueue_Create();

	Test_cQueue_WRData(pcQ, TestData, TESTSIZE/2);
	Test_cQueue_WRData(&cQStatic, TestData, TESTSIZE/2);
	Test_cQueue_WRData(pcQ, TestData, TESTSIZE);
	Test_cQueue_WRData(&cQStatic, TestData, TESTSIZE);

	Test_cQueue_WRDatas(pcQ, TestData, TESTSIZE/2-1);
	Test_cQueue_WRDatas(&cQStatic, TestData, TESTSIZE/2-1);
	Test_cQueue_WRDatas(pcQ, TestData, TESTSIZE/2+1);
	Test_cQueue_WRDatas(&cQStatic, TestData, TESTSIZE/2+1);
	Test_cQueue_WRDatas(pcQ, TestData, TESTSIZE/2);
	Test_cQueue_WRDatas(&cQStatic, TestData, TESTSIZE/2);
	Test_cQueue_WRDatas(pcQ, TestData, TESTSIZE);
	Test_cQueue_WRDatas(&cQStatic, TestData, TESTSIZE);

	Test_cQueue_OverWrite(pcQ, TestData, TESTSIZE/2-1);
	Test_cQueue_OverWrite(&cQStatic, TestData, TESTSIZE/2-1);
	Test_cQueue_OverWrite(pcQ, TestData, TESTSIZE/2+1);
	Test_cQueue_OverWrite(&cQStatic, TestData, TESTSIZE/2+1);
	Test_cQueue_OverWrite(pcQ, TestData, TESTSIZE/2);
	Test_cQueue_OverWrite(&cQStatic, TestData, TESTSIZE/2);
	Test_cQueue_OverWrite(pcQ, TestData, TESTSIZE);
	Test_cQueue_OverWrite(&cQStatic, TestData, TESTSIZE);

	Test_cQueue_Clear(pcQ);
	Test_cQueue_Clear(&cQStatic);
	Test_cQueue_Full_Usage(pcQ);
	Test_cQueue_Full_Usage(&cQStatic);

	Test_cQueue_Peek(pcQ, TestData, TESTSIZE);
	Test_cQueue_Peek(&cQStatic, TestData, TESTSIZE);
	Test_cQueue_Peeks(pcQ, TestData, TESTSIZE/2);
	Test_cQueue_Peeks(&cQStatic, TestData, TESTSIZE/2);
	Test_cQueue_Peeks(pcQ, TestData, TESTSIZE);
	Test_cQueue_Peeks(&cQStatic, TestData, TESTSIZE);

	Test_cQueue_WR_VariableLength(pcQ, (uint8_t*)TestData, TESTSIZE/2);
	Test_cQueue_WR_VariableLength(&cQStatic, (uint8_t*)TestData, TESTSIZE/2);
	Test_cQueue_WR_Margin(pcQ, TestData, TESTSIZE);
	Test_cQueue_WR_Margin(&cQStatic, TestData, TESTSIZE/2);
	//Exception Testing
	Test_cQueue_FULL_NULL(pcQ, TestData2, TESTSIZE + 2, TESTSIZE);
	Test_cQueue_FULL_NULL(&cQStatic, TestData2, TESTSIZE + 2, TESTSIZE);

	Test_cQueue_Destroy();
	
}

int main()
{
	
	cQueue_Test();//µ¥Ôª²âÊÔ
	printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
	//MALLOC(4);
	PRINT_LEAK_INFO();//ÄÚ´æÐ¹Â©²âÊÔ
	return (0);
}