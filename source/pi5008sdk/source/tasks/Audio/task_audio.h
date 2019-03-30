#ifndef __APP_AUDIO_H__
#define __APP_AUDIO_H__

#include "type.h"
#include "error.h"


typedef struct ppAUDIO_RSC_HDR_S
{
	PP_U32 u32Id;
	PP_U32 u32DataBit;
	PP_U32 u32SampleRate;
	PP_U32 u32Mono;
	PP_U32 u32Offset;
}AUDIO_RSC_HDR_S;

typedef struct ppAUDIO_HDR_S
{
	PP_U32 u32Count;
	PP_U32 u32Rsv[3];
}AUDIO_HDR_S;

PP_VOID vTaskAudio(PP_VOID *pvData);
#endif // __APP_AUDIO_H__
