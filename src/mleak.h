/*该文件代码来源：https://blog.csdn.net/qq_37120369/article/details/117487329*/
/*文件 mleak.h*/
#ifndef _MLEAK_H_
#define _MLEAK_H_
 
#include <stdio.h>
#include <malloc.h>
 
#define MALLOC(n) my_malloc(n,__FILE__,__LINE__)
#define FREE(p) my_free(p)
 
void * my_malloc(size_t n,const char* file,const int line);
void my_free(void *p);
void PRINT_LEAK_INFO();
 
#endif
 
 
