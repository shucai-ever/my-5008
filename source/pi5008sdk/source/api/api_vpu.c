#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "pi5008.h"
#include "osal.h"
#include "error.h"
#include "vpu_register.h"
#include "interrupt.h"
#include "utils.h"
#include "proc.h"
#include "dram_config.h"
#include "dram.h"

#include "vin.h"
#include "api_vin.h"

#include "api_vpu.h"
#include "vpu.h"
#include "debug.h"

PP_U32 PPAPI_VPU_GetVer(void)
{
    return(PPDRV_VPU_GetVer());
}

void PPAPI_VPU_DumpReg(void)
{
    PPDRV_VPU_DumpReg();
    return;
}

PP_RESULT_E PPAPI_VPU_FAST_Initialize(void)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_FAST_Initialize();
    return(result);
}

PP_RESULT_E PPAPI_VPU_FAST_SetScale(VPU_SIZE_T inputSize, VPU_SIZE_T outputSize)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_FAST_SetScale(inputSize, outputSize);
    return(result);
}

PP_RESULT_E PPAPI_VPU_FAST_SetInputSize(VPU_SIZE_T inputSize)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_FAST_SetInputSize(inputSize);
    return(result);
}

PP_RESULT_E PPAPI_VPU_FAST_SetROI(VPU_RECT_T roi)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_FAST_SetROI(roi);
    return(result);
}

PP_RESULT_E PPAPI_VPU_FAST_SetConfig(const USER_VPU_FB_CONFIG_U *pVPUFBConfig)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_FAST_SetConfig(pVPUFBConfig);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_FAST_GetConfig(USER_VPU_FB_CONFIG_U *pVPUFBConfig)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_FAST_GetConfig(pVPUFBConfig);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_FAST_SetZone(PP_U8 zoneNumber, VPU_ZONINFO_T *pZoneInfo)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_FAST_SetZone(zoneNumber, pZoneInfo);
    return(result);
}

PP_RESULT_E PPAPI_VPU_BRIEF_SetLut(VPU_BRIEF_LUT_VALUE_T *pBriefLut)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_BRIEF_SetLut(pBriefLut);
    return(result);
}

void PPAPI_VPU_BRIEF_SetProgramFilter(const PP_U8 *pCoef0Array, const PP_U8 *pCoef1Array)
{
    PPDRV_VPU_BRIEF_SetProgramFilter(pCoef0Array, pCoef1Array);
    return;
}

PP_RESULT_E PPAPI_VPU_FAST_Start(const PP_S32 runCount, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_FAST_Start(runCount, u32TimeOut);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_FAST_RunNext(const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_FAST_RunNext(u32TimeOut);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_FAST_Stop(void)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_FAST_Stop();
    return(ret);
}

PP_RESULT_E PPAPI_VPU_DMA_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_DMA_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, u16RdWidth, u16RdHeight, u32WrPAddr, u32WrStride, u32TimeOut);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_DMA_1Dto2D_Start(const PP_U32 u32RdPAddr, const PP_U16 u16RdSize, const PP_U16 u16WrWidth, const PP_U16 u16WrHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    _eDMAMODE eDMAMode = eDMA_2D;
    _eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_COPY;
    _eRDMATYPE eRDMAType = eRDMA_1D;
    _eWDMATYPE eWDMAType = eWDMA_2D;
    _eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
    ret = PPDRV_VPU_DMA_1Dto2D_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u16RdSize, u16WrWidth, u16WrHeight, u32WrPAddr, u32WrStride, u32TimeOut);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_DMA_2Dto1D_Start(const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U32 u32WrPAddr, const PP_U16 u16WrSize, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    _eDMAMODE eDMAMode = eDMA_2D;
    _eDMA_MINORMODE eDMAMinorMode = eDMA_MINOR_COPY;
    _eRDMATYPE eRDMAType = eRDMA_2D;
    _eWDMATYPE eWDMAType = eWDMA_1D;
    _eOTF2DMATYPE eOTF2DMAType = eOTF2DMA_DISABLE;
    ret = PPDRV_VPU_DMA_2Dto1D_Start(eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u16RdWidth, u16RdHeight, u32RdPAddr, u32RdStride, u32WrPAddr, u16WrSize, u32TimeOut);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_HAMMINGD_Start(_VPUStatus *pVPUStatus, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_HAMMINGD_Start(pVPUStatus, u32TimeOut);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_WaitFrmUpdate(PP_S32 timeOutMsec)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_WaitFrmUpdate(timeOutMsec);
    return(result);
}

PP_RESULT_E PPAPI_VPU_SetConfig(const PP_S32 eVpuChannel, const VPU_SIZE_T imageInSize)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_SetConfig(eVpuChannel, imageInSize);
    return(result);
}

void PPAPI_VPU_UnsetConfig(void)
{
    PPDRV_VPU_UnsetConfig();
}

PP_RESULT_E PPAPI_VPU_GetStatus(_VPUStatus *pVPUStatus)
{
    PP_RESULT_E ret = eSUCCESS;
    ret = PPDRV_VPU_GetStatus(pVPUStatus);
    return(ret);
}

PP_RESULT_E PPAPI_VPU_Initialize(void)
{
    PP_RESULT_E result;
    result = PPDRV_VPU_Initialize();
    return(result);
}

