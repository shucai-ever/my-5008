
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "osal.h"
#include "pcvOdAlloc.h"

#ifdef DEBUG
static uint32_t u32AllocSized = 0;
static uint32_t u32AllocCnt = 0;
static uint32_t u32FreeCnt = 0;
#endif
void *pcvOdMalloc(int size)
{
	void *ptr = NULL;

    if(size > 0)
    {
	    if( (ptr = (void *)OSAL_malloc(size)) == NULL)
        {
            return(NULL);
        }
    }
	
#ifdef DEBUG
	u32AllocSized += size;
	u32AllocCnt++;
	printf("ptr:%s, size:%d, total:%d, cnt:%d\n", (ptr==NULL)?"NULL":"Done", size, u32AllocSized, u32AllocCnt);
#endif

	return ptr;
}

void pcvOdFree(void *ptr)
{

#ifdef DEBUG
    u32FreeCnt++;
    printf("ptr:%s, cnt:%d\n", (ptr==NULL)?"NULL":"Done", u32FreeCnt);
#endif

	if(ptr)
    {
		OSAL_free(ptr);
    }
}
