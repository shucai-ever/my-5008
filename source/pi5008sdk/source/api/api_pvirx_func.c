#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "api_pvirx_func.h"
#include "debug.h"

PP_RESULT_E PPAPI_PVIRX_CheckChipID(const PP_U8 IN chanAddr, PP_U16 OUT *pRetChipID, PP_U8 OUT *pRetRevID, PP_S32 OUT *pRetRWVerify)
{
    PP_RESULT_E result;
    result = PPDRV_PVIRX_CheckChipID(0, chanAddr, pRetChipID, pRetRevID, pRetRWVerify);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_SetAttrChip(const PP_U8 IN chanAddr, const _stAttrChip IN *pstPviRxAttrChip)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result =  PPDRV_PVIRX_SetAttrChip(0, chanAddr, pstPviRxAttrChip);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_SetTableStdResol(const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution, const enum _eVideoResolution IN videoResolution, const PP_S32 IN bWaitStableStatus)
{
	PP_RESULT_E ret = eERROR_FAILURE;

    ret =  PPDRV_PVIRX_SetTableStdResol(0, chanAddr, cameraStandard, cameraResolution, videoResolution, bWaitStableStatus);
    return(ret);
}

PP_RESULT_E PPAPI_PVIRX_SetNovidInitIRQ(const PP_U8 IN chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_SetNovidInitIRQ(0, chanAddr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_SetInit(const PP_U8 IN chanAddr)
{
	PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_SetInit(0, chanAddr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_ReadVidStatusReg(const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg OUT *pstVidStatusReg)
{
	PP_RESULT_E ret = eERROR_FAILURE;

    ret = PPDRV_PVIRX_ReadVidStatusReg(0, chanAddr, pstVidStatusReg);
    return(ret);
}

PP_RESULT_E PPAPI_PVIRX_MonitorCurVidStatusReg(const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg OUT *pstVidStatusReg)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result =  PPDRV_PVIRX_MonitorCurVidStatusReg(0, chanAddr, pstVidStatusReg);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_ReadStdResol(const PP_U8 IN chanAddr, const _stPVIRX_VidStatusReg IN *pstVidStatusReg, enum _eCameraStandard OUT *pCameraStandard, enum _eCameraResolution OUT *pCameraResolution, enum _eVideoResolution OUT *pVideoResolution)
{
    PP_RESULT_E ret;
    ret =  PPDRV_PVIRX_ReadStdResol(0, chanAddr, pstVidStatusReg, pCameraStandard, pCameraResolution, pVideoResolution);
    return(ret);
}

PP_RESULT_E PPAPI_PVIRX_GetStdResol(const PP_U8 IN chanAddr, _stPVIRX_VidStatusReg IN *pstVidStatusReg, enum _eCameraStandard OUT *pCameraStandard, enum _eCameraResolution OUT *pCameraResolution, enum _eVideoResolution OUT *pVideoResolution, int OUT *pReJudge)
{

    PP_RESULT_E ret;
    ret = PPDRV_PVIRX_GetStdResol(0, chanAddr, pstVidStatusReg, pCameraStandard, pCameraResolution, pVideoResolution, pReJudge);
    return(ret);
}

#ifdef SUPPORT_PVIRX_VID
PP_RESULT_E PPAPI_PVIRX_VID_SetChnAttr(const PP_U8 IN chanAddr, const _stChnAttr IN *pstChnAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result =  PPDRV_PVIRX_VID_SetChnAttr(0, chanAddr, pstChnAttr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetChnAttr(const PP_U8 IN chanAddr, _stChnAttr OUT *pstChnAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result =  PPDRV_PVIRX_VID_GetChnAttr(0, chanAddr, pstChnAttr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetCscAttr(const PP_U8 IN chanAddr, const _stCscAttr IN *pstCscAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;

    result =  PPDRV_PVIRX_VID_SetCscAttr(0, chanAddr, pstCscAttr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetCscAttr(const PP_U8 IN chanAddr, _stCscAttr OUT *pstCscAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_VID_GetCscAttr(0, chanAddr, pstCscAttr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetContrast(const PP_U8 IN chanAddr, const _stContrast IN *pstContrast)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result =  PPDRV_PVIRX_VID_SetContrast(0, chanAddr, pstContrast);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetContrast(const PP_U8 IN chanAddr, _stContrast OUT *pstContrast)
{

	PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_VID_GetContrast(0, chanAddr, pstContrast);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetBright(const PP_U8 IN chanAddr, const _stBright IN *pstBright)
{

    PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_VID_SetBright(0, chanAddr, pstBright);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetBright(const PP_U8 IN chanAddr, _stBright OUT *pstBright)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_VID_GetBright(0, chanAddr, pstBright);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetSaturation(const PP_U8 IN chanAddr, const _stSaturation IN *pstSaturation)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_VID_SetSaturation(0, chanAddr, pstSaturation);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetSaturation(const PP_U8 IN chanAddr, _stSaturation OUT *pstSaturation)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_VID_GetSaturation(0, chanAddr, pstSaturation);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetHue(const PP_U8 IN chanAddr, const _stHue IN *pstHue)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_VID_SetHue(0, chanAddr, pstHue);
	return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetHue(const PP_U8 IN chanAddr, _stHue OUT *pstHue)
{
	PP_RESULT_E result = eERROR_FAILURE;
result =  PPDRV_PVIRX_VID_GetHue(0, chanAddr, pstHue);
	return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetSharpness(const PP_U8 IN chanAddr, const _stSharpness IN *pstSharpness)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_VID_SetSharpness(0, chanAddr, pstSharpness);
	return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_GetSharpness(const PP_U8 IN chanAddr, _stSharpness OUT *pstSharpness)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_VID_GetSharpness(0, chanAddr, pstSharpness);
	return(result);
}

PP_RESULT_E PPAPI_PVIRX_VID_SetBlank(const PP_U8 IN chanAddr, const PP_S32 IN bEnable, const PP_S32 IN blankColor)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_VID_SetBlank(0, chanAddr, bEnable, blankColor);
	return(result);
}
#endif // SUPPORT_PVIRX_VID

PP_RESULT_E PPAPI_PVIRX_GetNovidStatus(const PP_U8 IN chanAddr, PP_U8 OUT *pStatus)
{
	PP_RESULT_E result = eERROR_FAILURE;
result = PPDRV_PVIRX_GetNovidStatus(0, chanAddr, pStatus);
	return(result);
}

#ifdef SUPPORT_PVIRX_UTC
PP_RESULT_E PPAPI_PVIRX_UTC_SetTable(const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution)
{
    PP_RESULT_E ret = eERROR_FAILURE;

    ret =  PPDRV_PVIRX_UTC_SetTable(0, chanAddr, cameraStandard, cameraResolution);
    return(ret);
}

PP_RESULT_E PPAPI_PVIRX_UTC_StartRX(const PP_U8 IN chanAddr, const PP_S32 IN bStart)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	if(bStart)
	{
		LOG_DEBUG("Start UTC_RX\n");
		stReg.reg = 0x0040;
		u8RegMask = 0xC0; u8RegData = 0xC0;
		if( (result = PPDRV_PVIRX_WriteMaskBit(0, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
		{
			LOG_CRITICAL("Write reg.\n");
		    return(result);
		}

		stReg.reg = 0x0054;
		u8RegMask = 0x97; u8RegData = 0x83;
		if( (result = PPDRV_PVIRX_WriteMaskBit(0, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
		{
			LOG_CRITICAL("Write reg.\n");
		    return(result);
		}
	}
	else
	{
		LOG_DEBUG("Stop UTC_RX\n");
		stReg.reg = 0x0040;
		u8RegMask = 0x40; u8RegData = 0x00;
		if( (result = PPDRV_PVIRX_WriteMaskBit(0, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
		{
			LOG_CRITICAL("Write reg.\n");
		    return(result);
		}
	}
	return(result);
}

PP_RESULT_E PPAPI_PVIRX_UTC_StartTX(const PP_U8 IN chanAddr, const PP_S32 IN bStart)
{
	PP_RESULT_E result = eERROR_FAILURE;
	_stPVIRX_Reg stReg;
	PP_U8 u8RegData;
	PP_U8 u8RegMask;

	if(bStart)
	{
		LOG_DEBUG("Start UTC_TX\n");
		u8RegMask = 0x40; u8RegData = 0x40;
	}
	else
	{
		LOG_DEBUG("Stop UTC_TX\n");
		u8RegMask = 0x40; u8RegData = 0x00;
	}
	stReg.reg = 0x0060;
	if( (result = PPDRV_PVIRX_WriteMaskBit(0, chanAddr, stReg.b.addr, u8RegMask, u8RegData)) != eSUCCESS)
	{
		LOG_CRITICAL("Write reg..\n");
		return(result);
	}

	return(result);
}

PP_RESULT_E PPAPI_PVIRX_UTC_SendData(const PP_U8 IN chanAddr, const enum _eCameraStandard IN cameraStandard, const enum _eCameraResolution IN cameraResolution, const PP_S32 IN dataSize, const PP_U8 IN *pData)
{
    PP_RESULT_E result = eERROR_FAILURE;

    result = PPDRV_PVIRX_UTC_SendData(0, chanAddr, cameraStandard, cameraResolution, dataSize, pData);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_UTC_GetRxAttr(const PP_U8 IN chanAddr, _stUTCRxAttr OUT *pstUTCRxAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_UTC_GetRxAttr(0, chanAddr, pstUTCRxAttr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_UTC_GetTxAttr(const PP_U8 IN chanAddr, _stUTCTxAttr OUT *pstUTCTxAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_UTC_GetTxAttr(0, chanAddr, pstUTCTxAttr);
    return(result);
}

PP_RESULT_E PPAPI_PVIRX_UTC_GetHVStartAttr(const PP_U8 IN chanAddr, _stUTCHVStartAttr OUT *pstUTCHVStartAttr)
{
    PP_RESULT_E result = eERROR_FAILURE;
    result = PPDRV_PVIRX_UTC_GetHVStartAttr(0, chanAddr, pstUTCHVStartAttr);
    return(result);
}


#endif //SUPPORT_PVIRX_UTC

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PPAPI_PVIRX_Initialize(void)
{

    PPDRV_PVIRX_Initialize();

	return;
}



