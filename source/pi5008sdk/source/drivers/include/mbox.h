/*
 * mbox.h
 *
 *  Created on: 2017. 10. 17.
 *      Author: ihkong
 */

#ifndef _PI5008_MBOX_H_
#define _PI5008_MBOX_H_

#include "system.h"
#include "type.h"
#include "error.h"

typedef PP_VOID (*MBOX_ISR_CALLBACK) (PP_U32 IN u32Msg);

PP_VOID PPDRV_MBOX_Initialize(PP_S32 IN s32CoreNum);
PP_VOID PPDRV_MBOX_SetISR(MBOX_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_MBOX_SetData(PP_S32 IN s32Idx, PP_U32 IN u32Data);
PP_U32 PPDRV_MBOX_GetData(PP_S32 IN s32Idx);
PP_VOID PPDRV_MBOX_SetIRQMsg(PP_S32 IN s32CoreNum, PP_U32 IN u32Msg);
PP_U32 PPDRV_MBOX_GetIRQMsg(PP_S32 IN s32CoreNum);

#endif /* _PI5008_MBOX_H_ */
