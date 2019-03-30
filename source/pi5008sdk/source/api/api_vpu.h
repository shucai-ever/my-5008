#ifndef __API_VPU_H__
#define __API_VPU_H__

#include "vpu.h"

PP_U32 PPAPI_VPU_GetVer(void);
void PPAPI_VPU_DumpReg(void);
PP_RESULT_E PPAPI_VPU_FAST_SetConfig(const USER_VPU_FB_CONFIG_U *pVPUFBConfig);
PP_RESULT_E PPAPI_VPU_FAST_GetConfig(USER_VPU_FB_CONFIG_U *pVPUFBConfig);
void PPAPI_VPU_FAST_InitFrmInx(void);
PP_RESULT_E PPAPI_VPU_FAST_Start(const PP_S32 runCount, const PP_U32 u32TimeOut);
PP_RESULT_E PPAPI_VPU_FAST_RunNext(const PP_U32 u32TimeOut);
PP_RESULT_E PPAPI_VPU_FAST_Stop(void);

PP_RESULT_E PPAPI_VPU_DMA_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut);
PP_RESULT_E PPAPI_VPU_DMA_1Dto2D_Start(const PP_U32 u32RdPAddr, const PP_U16 u16RdSize, const PP_U16 u16WrWidth, const PP_U16 u16WrHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut);
PP_RESULT_E PPAPI_VPU_DMA_2Dto1D_Start(const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U32 u32WrPAddr, const PP_U16 u16WrSize, const PP_U32 u32TimeOut);
PP_RESULT_E PPAPI_VPU_HAMMINGD_Start(_VPUStatus *pVPUStatus, const PP_U32 u32TimeOut);
PP_RESULT_E PPAPI_VPU_WaitFrmUpdate(PP_S32 timeOutMsec);
PP_RESULT_E PPAPI_VPU_SetConfig(const PP_S32 eVpuChannel, const VPU_SIZE_T imageInSize);
void PPAPI_VPU_UnsetConfig(void);
PP_RESULT_E PPAPI_VPU_GetStatus(_VPUStatus *pVPUStatus);
PP_RESULT_E PPAPI_VPU_Initialize(void);

#endif // __API_VPU_H__


