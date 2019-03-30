/*
 * i2s.h
 *
 *  Created on: 2017. 4. 6.
 *      Author: ihkong
 */

#ifndef _PI5008_I2S_API_H_
#define _PI5008_I2S_API_H_

#include "system.h"
#include "type.h"
#include "i2s.h"

typedef enum ppAUDIO_STATUS_E
{
	eSTATUS_STOP = 0,
	eSTATUS_PLAY,
	eSTATUS_PAUSE,
}PP_AUDIO_STATUS_E;

PP_RESULT_E PPAPI_AUDIO_Initialize(PP_I2S_INIT_S IN *pstInit);
PP_RESULT_E PPAPI_AUDIO_Play(PP_U8 IN *pu8Buf, PP_U32 IN u32Size);
PP_VOID PPAPI_AUDIO_Stop(PP_VOID);
PP_VOID PPAPI_AUDIO_Pause(PP_VOID);
PP_VOID PPAPI_AUDIO_Resume(PP_VOID);
PP_AUDIO_STATUS_E PPAPI_AUDIO_GetPlayStatus(PP_VOID);

#endif
