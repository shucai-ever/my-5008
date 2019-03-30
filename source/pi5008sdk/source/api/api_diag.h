#ifndef _PI5008_API_DIAG_H_
#define _PI5008_API_DIAG_H_

#include "diag.h"

PP_RESULT_E PPAPI_DIAG_Initialize(void);

PP_VOID PPAPI_DIAG_GetFrameRate(const PP_U8 u8WaitSec, PP_U32 *pu32FrameCnt);

PP_VOID PPAPI_DIAG_GetDisplayFreezeStatus(PP_U8 pRetStatus[]);
PP_VOID PPAPI_DIAG_GetCamInvalidStatus(PP_U8 pRetStatus[]);

#endif
