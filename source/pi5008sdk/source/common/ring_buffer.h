/*
 * ring_buffer.h
 *
 *  Created on: 2018. 1. 17.
 *      Author: ihkong
 */

#ifndef _PI5008_RING_BUFFER_H_
#define _PI5008_RING_BUFFER_H_

#include "type.h"

/* about circle buffer */
typedef struct {
	PP_U32 u32Cmd;	//32bit command ID.
	PP_U16 u16Sender;	//Sender Queue ID. one of eQUEUE_CMDOWNER.
	PP_U16 u16RecvTask;	//
	PP_U16 u16Attr; //eQUEUE_CMDATTR bit combination.
	PP_U32 u32Length; //length of pData. If 0 is NULL.
}stCIRCBufItemForCmdSend;

typedef struct {
    PP_U8 * const buffer;
    PP_S32 head;
    PP_S32 tail;
    const PP_S32 maxlen;
} circ_bbuf_t;

#if 0
#define CIRC_BBUF_DEF(x,y)                \
    uint8_t x##_data_space[y];            \
    circ_bbuf_t x = {                     \
        .buffer = x##_data_space,         \
        .head = 0,                        \
        .tail = 0,                        \
        .maxlen = y                       \
    }    
#endif
   
#define CIRC_BUF_DATA_SIZE				1024		

extern circ_bbuf_t global_circ_buf;
extern PP_U8 global_circ_buf_data_space[CIRC_BUF_DATA_SIZE];

PP_S32 PPUTIL_CIRCBUF_PUSH(circ_bbuf_t *c, PP_U8 data);
PP_S32 PPUTIL_CIRCBUF_POP(circ_bbuf_t *c, PP_U8 *data);
PP_U32 PPUTIL_CIRCBUF_GETEMPTYSIZE(circ_bbuf_t *c);


HANDLE *PPUTIL_RINGBUFF_Initialize(PP_U32 u32BufferSize);
PP_RESULT_E PPUTIL_RINGBUFF_WriteData(HANDLE *phHandle, PP_U8 *pu8Data, PP_U32 u32Size);
PP_RESULT_E PPUTIL_RINGBUFF_ReadData(HANDLE *phHandle, PP_U8 *pu8Data, PP_U32 u32Size);
PP_U32 PPUTIL_RINGBUFF_GetBufferedSize(HANDLE *phHandle);
PP_U32 PPUTIL_RINGBUFF_GetEmptySize(HANDLE *phHandle);
PP_VOID PPUTIL_RINGBUFF_Reset(HANDLE *phHandle);



#endif /* _PI5008_RING_BUFFER_H_ */
