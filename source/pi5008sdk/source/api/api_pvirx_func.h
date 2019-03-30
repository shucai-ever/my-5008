#ifndef __API_PVIRX_FUNC_H__
#define __API_PVIRX_FUNC_H__

#include "error.h"
#include "pvirx_func.h"
#include "pvirx_drvcommon.h"

PP_RESULT_E PPAPI_PVIRX_CheckChipID(const PP_U8 IN chanAddr, PP_U16 OUT *pRetChipID, PP_U8 OUT *pRetRevID, PP_S32 OUT *pRetRWVerify);
PP_RESULT_E PPAPI_PVIRX_SetAttrChip(const PP_U8 IN chanAddr, const _stAttrChip IN *pstPviRxAttrChip);
PP_RESULT_E PPAPI_PVIRX_SetTableStdResol(const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution, const enum _eVideoResolution IN videoResolution, const PP_S32 IN bWaitStableStatus);
PP_RESULT_E PPAPI_PVIRX_SetNovidInitIRQ(const PP_U8 IN chanAddr);
PP_RESULT_E PPAPI_PVIRX_SetInit(const PP_U8 IN chanAddr);

PP_RESULT_E PPAPI_PVIRX_ReadVidStatusReg(const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg OUT *pstVidStatusReg);
PP_RESULT_E PPAPI_PVIRX_MonitorCurVidStatusReg(const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg OUT *pstVidStatusReg);
PP_RESULT_E PPAPI_PVIRX_ReadStdResol(const PP_U8 IN chanAddr, const _stPVIRX_VidStatusReg IN *pstVidStatusReg, enum _eCameraStandard OUT *pCameraStandard, enum _eCameraResolution OUT *pCameraResolution, enum _eVideoResolution OUT *pVideoResolution);
PP_RESULT_E PPAPI_PVIRX_GetStdResol(const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, enum _eCameraStandard OUT *pCameraStandard, enum _eCameraResolution OUT *pCameraResolution, enum _eVideoResolution OUT *pVideoResolution, int OUT *pReJudge);
#ifdef SUPPORT_PVIRX_VID
PP_RESULT_E PPAPI_PVIRX_VID_SetChnAttr(const PP_U8 IN chanAddr, const _stChnAttr IN *pstChnAttr);
PP_RESULT_E PPAPI_PVIRX_VID_GetChnAttr(const PP_U8 IN chanAddr, _stChnAttr OUT *pstChnAttr);
PP_RESULT_E PPAPI_PVIRX_VID_SetCscAttr(const PP_U8 IN chanAddr, const _stCscAttr IN *pstCscAttr);
PP_RESULT_E PPAPI_PVIRX_VID_GetCscAttr(const PP_U8 IN chanAddr, _stCscAttr OUT *pstCscAttr);
PP_RESULT_E PPAPI_PVIRX_VID_SetContrast(const PP_U8 IN chanAddr, const _stContrast IN *pstContrast);
PP_RESULT_E PPAPI_PVIRX_VID_GetContrast(const PP_U8 IN chanAddr, _stContrast OUT *pstContrast);
PP_RESULT_E PPAPI_PVIRX_VID_SetBright(const PP_U8 IN chanAddr, const _stBright IN *pstBright);
PP_RESULT_E PPAPI_PVIRX_VID_GetBright(const PP_U8 IN chanAddr, _stBright OUT *pstBright);
PP_RESULT_E PPAPI_PVIRX_VID_SetSaturation(const PP_U8 IN chanAddr, const _stSaturation IN *pstSaturation);
PP_RESULT_E PPAPI_PVIRX_VID_GetSaturation(const PP_U8 IN chanAddr, _stSaturation OUT *pstSaturation);
PP_RESULT_E PPAPI_PVIRX_VID_SetHue(const PP_U8 IN chanAddr, const _stHue IN *pstHue);
PP_RESULT_E PPAPI_PVIRX_VID_GetHue(const PP_U8 IN chanAddr, _stHue OUT *pstHue);
PP_RESULT_E PPAPI_PVIRX_VID_SetSharpness(const PP_U8 IN chanAddr, const _stSharpness IN *pstSharpness);
PP_RESULT_E PPAPI_PVIRX_VID_GetSharpness(const PP_U8 IN chanAddr, _stSharpness OUT *pstSharpness);
PP_RESULT_E PPAPI_PVIRX_VID_SetBlank(const PP_U8 IN chanAddr, const PP_S32 IN bEnable, const PP_S32 IN blankColor);
#endif // SUPPORT_PVIRX_VID
PP_RESULT_E PPAPI_PVIRX_GetNovidStatus(const PP_U8 IN chanAddr, PP_U8 OUT *pStatus);
#ifdef SUPPORT_PVIRX_UTC
PP_RESULT_E PPAPI_PVIRX_UTC_SetTable(const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution);
PP_RESULT_E PPAPI_PVIRX_UTC_StartRX(const PP_U8 IN chanAddr, const PP_S32 IN bStart);
PP_RESULT_E PPAPI_PVIRX_UTC_StartTX(const PP_U8 IN chanAddr, const PP_S32 IN bStart);
PP_RESULT_E PPAPI_PVIRX_UTC_SendData(const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution, const PP_S32 IN dataSize, const PP_U8 IN *pData);
PP_RESULT_E PPAPI_PVIRX_UTC_GetRxAttr(const PP_U8 IN chanAddr, _stUTCRxAttr OUT *pstUTCRxAttr);
PP_RESULT_E PPAPI_PVIRX_UTC_GetTxAttr(const PP_U8 IN chanAddr, _stUTCTxAttr OUT *pstUTCTxAttr);
PP_RESULT_E PPAPI_PVIRX_UTC_GetHVStartAttr(const PP_U8 IN chanAddr, _stUTCHVStartAttr OUT *pstUTCHVStartAttr);
#endif //SUPPORT_PVIRX_UTC

void PPAPI_PVIRX_Initialize(void);
#endif // __PVIRX_FUNC_H__
