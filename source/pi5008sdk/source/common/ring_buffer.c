/*
 * ring_buffer.c
 *
 *  Created on: 2018. 1. 17.
 *      Author: ihkong
 */
#include "system.h"
#include "debug.h"
#include "osal.h"
#include "ring_buffer.h"

#define USE_RING_BUFF_MUTEX		0

#if USE_RING_BUFF_MUTEX
#define MUTEX_LOCK(m) 		OSAL_wait_for_mutex(m);
#define MUTEX_UNLOCK(m) 	OSAL_release_mutex(m);
#else
#define MUTEX_LOCK(m)
#define MUTEX_UNLOCK(m)
#endif

#define CIRCLE_DISTANCE(cur, base, tsize)	( (cur>=base)? 			(cur-base) : (cur+tsize-base) )
#define CIRCLE_INC(cur, tsize)				( (cur == (tsize-1))? 	0 : (cur+1) )
#define CIRCLE_DEC(cur, tsize)				( (cur == 0)?			(tsize-1) : (cur-1) )
#define CIRCLE_ADD(cur, addval, tsize)		( (cur+addval >= tsize)? (cur+addval-tsize) : (cur+addval) )
#define CIRCLE_SUB(cur, subval, tsize)		( (cur >= subval)?		(cur-subval) : (cur+tsize-subval) )


typedef struct ppRINGBUFF_S
{
	PP_U8 *pu8Buf;
	PP_U32 u32ReadPos;
	PP_U32 u32WritePos;
	PP_U32 u32BufferSize;
	sys_os_mutex_t stMutex;

}RINGBUFF_S;



/* about circle(ring) buffer */

PP_U8 global_circ_buf_data_space[CIRC_BUF_DATA_SIZE];
circ_bbuf_t global_circ_buf = {					  \
	.buffer = global_circ_buf_data_space,		  \
	.head = 0,						  \
	.tail = 0,						  \
	.maxlen = CIRC_BUF_DATA_SIZE 					  \
};

PP_S32 PPUTIL_CIRCBUF_PUSH(circ_bbuf_t *c, PP_U8 data)
{
    PP_S32 next;

    next = c->head + 1;  // next is where head will point to after this write.
    if (next >= c->maxlen)
        next = 0;

    if (next == c->tail)  // if the head + 1 == tail, circular buffer is full
        return -1;

    c->buffer[c->head] = data;  // Load data and then move
    c->head = next;             // head to next data offset.
    return 0;  // return success to indicate successful push.
}

PP_S32 PPUTIL_CIRCBUF_POP(circ_bbuf_t *c, PP_U8 *data)
{
    PP_S32 next;

    if (c->head == c->tail)  // if the head == tail, we don't have any data
        return -1;

    next = c->tail + 1;  // next is where tail will point to after this read.
    if(next >= c->maxlen)
        next = 0;

    *data = c->buffer[c->tail];  // Read data and then move
    c->tail = next;              // tail to next offset.
    return 0;  // return success to indicate successful push.
}

PP_U32 PPUTIL_CIRCBUF_GETEMPTYSIZE(circ_bbuf_t *c)
{
	PP_U32 size;

	if(c->head>=c->tail)
	{
    	return ((c->maxlen)-(c->head - c->tail));
	}
	else
	{
		return (c->tail - c->head);
	}
}






HANDLE *PPUTIL_RINGBUFF_Initialize(PP_U32 u32BufferSize)
{
	RINGBUFF_S *pstRBuff;

	pstRBuff = (RINGBUFF_S *)OSAL_malloc(sizeof(RINGBUFF_S));
	if(pstRBuff == NULL)return NULL;

	memset(pstRBuff, 0, sizeof(RINGBUFF_S));

	pstRBuff->pu8Buf = (PP_U8 *)OSAL_malloc(u32BufferSize);
	if(pstRBuff == NULL){
		OSAL_free(pstRBuff);
		return NULL;
	}

	pstRBuff->u32BufferSize = u32BufferSize;
#if USE_RING_BUFF_MUTEX
	if(OSAL_create_mutex(&pstRBuff->stMutex) != eSUCCESS)return NULL;
#endif
	return (HANDLE *)pstRBuff;

}

PP_RESULT_E PPUTIL_RINGBUFF_WriteData(HANDLE *phHandle, PP_U8 *pu8Data, PP_U32 u32Size)
{
	RINGBUFF_S *pstRBuff = (RINGBUFF_S *)phHandle;
	PP_RESULT_E ret = eSUCCESS;

	MUTEX_LOCK(&pstRBuff->stMutex);

	if(CIRCLE_DISTANCE(pstRBuff->u32WritePos, pstRBuff->u32ReadPos, pstRBuff->u32BufferSize) + u32Size >= pstRBuff->u32BufferSize){
		ret = eERROR_FAILURE;
		goto END_FT;
	}


	if(pstRBuff->u32WritePos + u32Size > pstRBuff->u32BufferSize){
		memcpy(&pstRBuff->pu8Buf[pstRBuff->u32WritePos], pu8Data, pstRBuff->u32BufferSize - pstRBuff->u32WritePos);
		memcpy(pstRBuff->pu8Buf, &pu8Data[pstRBuff->u32BufferSize - pstRBuff->u32WritePos], pstRBuff->u32WritePos + u32Size - pstRBuff->u32BufferSize);
	}else{
		memcpy(&pstRBuff->pu8Buf[pstRBuff->u32WritePos], pu8Data, u32Size);
	}
	pstRBuff->u32WritePos = CIRCLE_ADD(pstRBuff->u32WritePos, u32Size, pstRBuff->u32BufferSize);
END_FT:
	MUTEX_UNLOCK(&pstRBuff->stMutex);

	return ret;
}


PP_RESULT_E PPUTIL_RINGBUFF_ReadData(HANDLE *phHandle, PP_U8 *pu8Data, PP_U32 u32Size)
{
	RINGBUFF_S *pstRBuff = (RINGBUFF_S *)phHandle;
	PP_RESULT_E ret = eSUCCESS;

	MUTEX_LOCK(&pstRBuff->stMutex);
	if(CIRCLE_DISTANCE(pstRBuff->u32WritePos, pstRBuff->u32ReadPos, pstRBuff->u32BufferSize) < u32Size){
		ret = eERROR_FAILURE;
		goto END_FT;
	}

	if(pstRBuff->u32ReadPos + u32Size > pstRBuff->u32BufferSize){
		memcpy(pu8Data, &pstRBuff->pu8Buf[pstRBuff->u32ReadPos], pstRBuff->u32BufferSize - pstRBuff->u32ReadPos);
		memcpy(&pu8Data[pstRBuff->u32BufferSize - pstRBuff->u32ReadPos], pstRBuff->pu8Buf, pstRBuff->u32ReadPos + u32Size - pstRBuff->u32BufferSize);

	}else{
		memcpy(pu8Data, &pstRBuff->pu8Buf[pstRBuff->u32ReadPos], u32Size);
	}
	pstRBuff->u32ReadPos = CIRCLE_ADD(pstRBuff->u32ReadPos, u32Size, pstRBuff->u32BufferSize);

END_FT:
	MUTEX_UNLOCK(&pstRBuff->stMutex);

	return ret;

}

PP_U32 PPUTIL_RINGBUFF_GetBufferedSize(HANDLE *phHandle)
{
	RINGBUFF_S *pstRBuff = (RINGBUFF_S *)phHandle;
	PP_U32 u32Ret;

	MUTEX_LOCK(&pstRBuff->stMutex);
	u32Ret = CIRCLE_DISTANCE(pstRBuff->u32WritePos, pstRBuff->u32ReadPos, pstRBuff->u32BufferSize);
	MUTEX_UNLOCK(&pstRBuff->stMutex);

	return u32Ret;
}

PP_U32 PPUTIL_RINGBUFF_GetEmptySize(HANDLE *phHandle)
{
	RINGBUFF_S *pstRBuff = (RINGBUFF_S *)phHandle;
	PP_U32 u32Ret;

	MUTEX_LOCK(&pstRBuff->stMutex);
	u32Ret = pstRBuff->u32BufferSize - CIRCLE_DISTANCE(pstRBuff->u32WritePos, pstRBuff->u32ReadPos, pstRBuff->u32BufferSize);
	MUTEX_UNLOCK(&pstRBuff->stMutex);

	return u32Ret;

}

PP_VOID PPUTIL_RINGBUFF_Reset(HANDLE *phHandle)
{
	RINGBUFF_S *pstRBuff = (RINGBUFF_S *)phHandle;

	MUTEX_LOCK(&pstRBuff->stMutex);
	pstRBuff->u32ReadPos = 0;
	pstRBuff->u32WritePos = 0;
	pstRBuff->u32BufferSize = 0;
	MUTEX_UNLOCK(&pstRBuff->stMutex);
}
