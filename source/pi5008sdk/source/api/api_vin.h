/*
 * api_vin.h
 *
 *  Created on: 2017. 4. 18.
 *      Author: ihkong
 */

#ifndef _PI5008_API_VIN_H_
#define _PI5008_API_VIN_H_

#include "system.h"
#include "type.h"
#include "error.h"
#include "vin.h"
#include "svm_drv.h"

typedef enum ppQUAD_CH_E {
	QUAD_0CH = 0,
	QUAD_1CH = 1,
	QUAD_2CH = 2,
	QUAD_3CH = 3,
}PP_QUAD_CH_E;

PP_VOID PPAPI_VIN_Initialize(PP_VOID);
PP_VOID PPAPI_VIN_SetGenlockParam(PP_VOID);
PP_RESULT_E PPAPI_VIN_GetResol(const PP_S32 defVideoFmt, PP_S32 *pRetWidth, PP_S32 *pRetHeight, _VID_RESOL *peRetResol);

PP_RESULT_E PPAPI_VIN_SetQuadViewMode(const PP_S32 defVideoFmt, const PP_U8 bQuadView, const PP_S32 chSel, const PP_S32 pathSel);
PP_RESULT_E PPAPI_VIN_SetCaptureMode(const PP_S32 defVideoFmt, const PP_S32 chSel, const PP_S32 pathSel);
PP_RESULT_E PPAPI_VIN_GetCaptureImage(const PP_S32 defVideoFmt, const PP_S32 bYOnly, PP_U32 u32BufPAddr, PP_U32 *pRetBufSize, PP_QUAD_CH_E chan, PP_BOOL bFull);
PP_RESULT_E PPAPI_VIN_SetCaptureUserMode(const PP_S32 defVideoFmt, const PP_U32 sclWidth, const PP_U32 sclHeight, const PP_U8 chSelBit);
PP_RESULT_E PPAPI_VIN_GetCaptureUserImage(const PP_S32 defVideoFmt, const PP_U32 sclWidth, const PP_U32 sclHeight, const PP_U8 chSelBit, PP_U32 *pu32BufPAddr, PP_U32 *pRetBufSize);
PP_VOID PPAPI_VIN_EnableQuad(const PP_BOOL bEnable);

PP_RESULT_E PPAPI_VIN_GetVIDPortFromUserDefined(const PP_S8 s8VinPort[], PP_S8 s8RetVinPort[]);
PP_RESULT_E PPAPI_VIN_SetVIDPort(const PP_S8 s8VinPort0, const PP_S8 s8VinPort1,  const PP_S8 s8VinPort2, const PP_S8 s8VinPort3, const PP_S8 s8VinPort4);
PP_RESULT_E PPAPI_VIN_SetSVMChannel(const PP_S8 s8SvmChannel, PP_S8 s8SvmPath, PP_S8 s8SvmPort);
PP_RESULT_E PPAPI_VIN_SetROChannel(const PP_U8 u8ROPath, const PP_U8 b8Bit, const PP_S8 s8OutCh0, const PP_S8 s8OutCh1,  const PP_S8 s8OutCh2, const PP_S8 s8OutCh3);

PP_RESULT_E PPAPI_VIN_GetInputInfo(const PP_U8 u8Channel, PP_U32 *pRetInputInfo);

PP_RESULT_E PPAPI_VIN_DiagCameraInput(const PP_U8 u8Channel);
PP_RESULT_E PPAPI_VIN_GetCameraPlugStatus(PP_S8 pRetCamStatus[]);

#endif /* _PI5008_API_VIN_H_ */
