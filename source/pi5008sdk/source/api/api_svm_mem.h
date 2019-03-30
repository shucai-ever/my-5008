/**
 * \file
 *
 * \brief	SVM memory control APIs
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __API_SVM_MEM_H__
#define __API_SVM_MEM_H__

#include "type.h"
#include "error.h"
#include "common.h"
#include "viewmode_config.h"
#include "svm_drv.h"

 
/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#ifdef SVM_TOGGLE_VIEW_USE
#define SVMMEM_VIEW_PAGE_FLIPPING_COUNT					(3)
#else
#define SVMMEM_VIEW_PAGE_FLIPPING_COUNT					(2)
#endif

#define SVMMEM_IN_BUFFER_COUNT							(3)
#if (BD_VOUT_INTERLACE == 1)
#define SVMMEM_OUT_BUFFER_COUNT							(3)
#else
#define SVMMEM_OUT_BUFFER_COUNT							(2)
#endif

#if (SVMMEM_VIEW_PAGE_FLIPPING_COUNT < 2)
#error "SVMMEM_VIEW_PAGE_FLIPPING_COUNT error"
#endif

#if (SVMMEM_IN_BUFFER_COUNT < 3)
#error "SVMMEM_IN_BUFFER_COUNT error"
#endif

#if (SVMMEM_OUT_BUFFER_COUNT < 2 || SVMMEM_OUT_BUFFER_COUNT > 4)
#error "SVMMEM_OUT_BUFFER_COUNT error"
#endif

#define SVMMEM_HEADER_COMMON_SECTION_COUNT_MAX		(4)			// only even number is possible
#define SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX		(4)			// only multiple of 4 is possible


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppSVMMEM_OUT_FRAMBUF_NUM_E
{
	eSVMMEM_OUT_FB_NUM_0 = 0,
	eSVMMEM_OUT_FB_NUM_1,
	eSVMMEM_OUT_FB_NUM_2,
	eSVMMEM_OUT_FB_NUM_3,
	eSVMMEM_OUT_FB_NUM_MAX = SVMMEM_OUT_BUFFER_COUNT,
} PP_SVMMEM_OUT_FRAMEBUF_NUM_E;

typedef enum ppSVMMEM_BC_ADDITIONAL_LUT_E
{
	eSVMMEM_BC_ADD_LUT_ALPHA_0 = 0,
	eSVMMEM_BC_ADD_LUT_ALPHA_1,
	eSVMMEM_BC_ADD_LUT_GRADIENT,
	eSVMMEM_BC_ADD_LUT_MAX,
} PP_SVMMEM_BC_ADDITIONAL_LUT_E;

typedef enum ppSVMMEM_BC_ADDITIONAL_LUT_SUBCORE_E
{
	eSVMMEM_BC_ADD_LUT_SUBCORE_0 = 0,
	eSVMMEM_BC_ADD_LUT_SUBCORE_1,
	eSVMMEM_BC_ADD_LUT_SUBCORE_MAX,
} PP_SVMMEM_BC_ADDITIONAL_LUT_SUBCORE_E;

typedef enum ppSVMMEM_SECTION_DATA_E
{
	eSVMMEM_SECTION_DATA_FB_ODD = 0,		// front & rear odd lut
	eSVMMEM_SECTION_DATA_FB_EVEN,			// front & rear even lut
	eSVMMEM_SECTION_DATA_LR_ODD,			// left & right odd lut
	eSVMMEM_SECTION_DATA_LR_EVEN,			// left & right even lut
	eSVMMEM_SECTION_DATA_BC_ODD,			// brightness control odd lut
	eSVMMEM_SECTION_DATA_BC_EVEN,			// brightness control even lut
	eSVMMEM_SECTION_DATA_IMG_ODD,			// odd shadow image
	eSVMMEM_SECTION_DATA_IMG_EVEN,			// even shadow image
	eSVMMEM_SECTION_DATA_MAX,
} PP_SVMMEM_SECTION_DATA_E;

typedef enum ppSVMMEM_VIEW_DATA_TYPE_E
{
	eSVMMEM_VIEW_DATA_BC_ODD = 0,			// bc odd lut
	eSVMMEM_VIEW_DATA_BC_EVEN,				// bc even lut
	eSVMMEM_VIEW_DATA_A0S0_ODD,				// alpha0 core0 odd lut
	eSVMMEM_VIEW_DATA_A0S0_EVEN,			// alpha0 core0 even lut
	eSVMMEM_VIEW_DATA_A0S1_ODD,				// alpha0 core1 odd lut
	eSVMMEM_VIEW_DATA_A0S1_EVEN,			// alpha0 core1 even lut
	eSVMMEM_VIEW_DATA_A1S0_ODD,				// alpha1 core0 odd lut
	eSVMMEM_VIEW_DATA_A1S0_EVEN,			// alpha1 core0 even lut
	eSVMMEM_VIEW_DATA_A1S1_ODD,				// alpha2 core1 odd lut
	eSVMMEM_VIEW_DATA_A1S1_EVEN,			// alpha2 core1 even lut
	eSVMMEM_VIEW_DATA_BGS0_ODD,				// gradient core0 odd lut of brightness control
	eSVMMEM_VIEW_DATA_BGS0_EVEN,			// gradient core0 even lut of brightness control
	eSVMMEM_VIEW_DATA_BGS1_ODD,				// gradient core1 odd lut of brightness control
	eSVMMEM_VIEW_DATA_BGS1_EVEN,			// gradient core1 even lut of brightness control
	eSVMMEM_VIEW_DATA_MAX,
} PP_SVMMEM_VIEW_DATA_E;

typedef enum ppSVMMEM_DATA_SUBTYPE_E
{
	eSVMMEM_DATA_SUBTYPE_FB_ODD = 0,		// front & rear odd lut
	eSVMMEM_DATA_SUBTYPE_FB_EVEN,			// front & rear even lut
	eSVMMEM_DATA_SUBTYPE_LR_ODD,			// left & right odd lut
	eSVMMEM_DATA_SUBTYPE_LR_EVEN,			// left & right even lut
	eSVMMEM_DATA_SUBTYPE_BC_ODD,			// brightness control odd lut
	eSVMMEM_DATA_SUBTYPE_BC_EVEN,			// brightness control even lut
	eSVMMEM_DATA_SUBTYPE_A0S0_ODD,			// alpha0 core0 odd lut
	eSVMMEM_DATA_SUBTYPE_A0S0_EVEN,			// alpha0 core0 even lut
	eSVMMEM_DATA_SUBTYPE_A0S1_ODD,			// alpha0 core1 odd lut
	eSVMMEM_DATA_SUBTYPE_A0S1_EVEN,			// alpha0 core1 even lut
	eSVMMEM_DATA_SUBTYPE_A1S0_ODD,			// alpha1 core0 odd lut
	eSVMMEM_DATA_SUBTYPE_A1S0_EVEN,			// alpha1 core0 even lut
	eSVMMEM_DATA_SUBTYPE_A1S1_ODD,			// alpha2 core1 odd lut
	eSVMMEM_DATA_SUBTYPE_A1S1_EVEN,			// alpha2 core1 even lut
	eSVMMEM_DATA_SUBTYPE_BGS0_ODD,			// gradient core0 odd lut of brightness control
	eSVMMEM_DATA_SUBTYPE_BGS0_EVEN,			// gradient core0 even lut of brightness control
	eSVMMEM_DATA_SUBTYPE_BGS1_ODD,			// gradient core1 odd lut of brightness control
	eSVMMEM_DATA_SUBTYPE_BGS1_EVEN,			// gradient core1 even lut of brightness control
	eSVMMEM_DATA_SUBTYPE_IMG0_ODD,			// shadow Img0 odd
	eSVMMEM_DATA_SUBTYPE_IMG0_EVEN,			// shadow Img0 even
	eSVMMEM_DATA_SUBTYPE_IMG1_ODD,			// shadow Img1 odd
	eSVMMEM_DATA_SUBTYPE_IMG1_EVEN,			// shadow Img1 even
	eSVMMEM_DATA_SUBTYPE_MAX,
} PP_SVMMEM_DATA_SUBTYPE_E;

typedef enum ppSVMMEM_LOAD_RET_E
{
	eSVMMEM_LOAD_RET_FAILURE = 0,
	eSVMMEM_LOAD_RET_SUCCESS_LOAD,		// The Data is current loaded.
	eSVMMEM_LOAD_RET_SUCCESS_STORE,		// The Data is already loaded..
} PP_SVMMEM_LOAD_RET_E;


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
#ifdef __cplusplus
EXTERN "C" {
#endif

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* address of in/out buffer 
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_U32*					PPAPI_SVMMEM_GetInputFrameBufferAddr(PP_CHANNEL_E IN enChannel);
PP_U32*					PPAPI_SVMMEM_GetOutputFrameBufferAddr(PP_SVMMEM_OUT_FRAMEBUF_NUM_E IN enOutFBNum);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* copy from flash to dram & get dram address
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_BOOL					PPAPI_SVMMEM_LoadCommonSectionData(PP_S32 u32SectionNum, PP_BOOL bOnlyViewLUT);
PP_SVMMEM_LOAD_RET_E	PPAPI_SVMMEM_LoadData(PP_VIEWMODE_E IN enView, PP_BOOL IN bLoading);
PP_U32*					PPAPI_SVMMEM_GetLoadedFBLUTAddress(PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetLoadedLRLUTAddress(PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetLoadedBCLUTAddress(PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetLoadedBCAddLUTAddress(PP_SVMMEM_BC_ADDITIONAL_LUT_E IN enType, PP_SVMMEM_BC_ADDITIONAL_LUT_SUBCORE_E IN enSubCore, PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetLoadedImageAddress(PP_SVMDRV_IMG_NUMBER_E enImgNum, PP_FIELD_E IN enField);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* stored flash information
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_U32					PPAPI_SVMMEM_GetSectionCount(PP_VOID);
PP_S32					PPAPI_SVMMEM_GetSectionNumber(PP_VIEWMODE_E IN enView, PP_RECT_S IN stSectionRect);
PP_U32					PPAPI_SVMMEM_GetSectionMaxDataSize(PP_VOID);
PP_U32					PPAPI_SVMMEM_GetSectionMergeMaxCount(PP_VOID);
PP_U32					PPAPI_SVMMEM_GetSectionDataSize(PP_U32 IN u32SectionIndex, PP_SVMMEM_SECTION_DATA_E IN enType);
PP_U32*					PPAPI_SVMMEM_GetSectionAddress(PP_U32 IN u32SectionIndex, PP_SVMMEM_SECTION_DATA_E IN enType);
PP_U32					PPAPI_SVMMEM_GetCommonSectionCount(PP_VOID);
PP_U16*					PPAPI_SVMMEM_GetCommonSectionList(PP_VOID);
PP_U32					PPAPI_SVMMEM_IsCommonSection(PP_U32 IN u32SectionIndex);
PP_U32					PPAPI_SVMMEM_GetViewCount(PP_VOID);
PP_U32					PPAPI_SVMMEM_GetViewTotalSize(PP_VIEWMODE_E IN enView);
PP_U32					PPAPI_SVMMEM_GetViewMaxSize(PP_VOID);
PP_U32					PPAPI_SVMMEM_GetViewDataSize(PP_VIEWMODE_E IN enView, PP_SVMMEM_VIEW_DATA_E IN enLutType);
PP_U32					PPAPI_SVMMEM_GetViewSectionCount(PP_VIEWMODE_E IN enView);
PP_S32					PPAPI_SVMMEM_GetViewSectionIndex(PP_VIEWMODE_E IN enView, PP_U32 IN u32viewSectionIndex);
PP_RECT_S*				PPAPI_SVMMEM_GetViewSectionRect(PP_VIEWMODE_E IN enView, PP_U32 IN u32ViewSectionIndex);
PP_BOOL					PPAPI_SVMMEM_CheckViewSectionRect(PP_VIEWMODE_E IN enView, PP_RECT_S* IN stSectionRect, PP_U32 u32SectionCnt);
PP_BOOL					PPAPI_SVMMEM_IsData(PP_VIEWMODE_E IN enView, PP_SVMMEM_DATA_SUBTYPE_E IN enLutType);
PP_RECT_S*				PPAPI_SVMMEM_GetImageRect(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E enImgNum);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* cache task control
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_U32					PPAPI_SVMMEM_GetCacheViewListCount(PP_VOID);
PP_VIEWMODE_E*			PPAPI_SVMMEM_GetCacheViewList(PP_VOID);
PP_BOOL					PPAPI_SVMMEM_CheckCacheView(PP_VIEWMODE_E enView);
PP_U32*					PPAPI_SVMMEM_GetCacheViewLoadedFBLUTAddress(PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetCacheViewLoadedLRLUTAddress(PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetCacheViewLoadedBCLUTAddress(PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetCacheViewLoadedBCAddLUTAddress(PP_SVMMEM_BC_ADDITIONAL_LUT_E IN enType, PP_SVMMEM_BC_ADDITIONAL_LUT_SUBCORE_E IN enSubCore, PP_FIELD_E IN enField);
PP_U32*					PPAPI_SVMMEM_GetCacheViewLoadedImageAddress(PP_SVMDRV_IMG_NUMBER_E enImgNum, PP_FIELD_E IN enField);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* check loaded view data
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_BOOL					PPAPI_SVMMEM_IsLoaded(PP_VIEWMODE_E IN enView);

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* initialize
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_BOOL					PPAPI_SVMMEM_IsInitialize(PP_VOID);
PP_RESULT_E				PPAPI_SVMMEM_Initialize(PP_VIEWMODE_E IN enCacheStartView, PP_VIEWMODE_E IN enCacheEndView);

#ifdef __cplusplus
}
#endif

#endif //__API_SVM_MEM_H__
