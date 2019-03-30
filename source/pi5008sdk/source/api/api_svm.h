/**
 * \file
 *
 * \brief	SVM APIs
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __API_SVM_H__
#define __API_SVM_H__

#include "type.h"
#include "error.h"
#include "common.h"
#include "viewmode_config.h"

#include "svm_drv.h"
#include "api_svm_mem.h"

 
/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
typedef PP_VOID (*PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY) (PP_VIEWMODE_E enView);
typedef PP_VOID (*PPAPI_SVM_SWITCHING_CALLBACK_FINISH) (PP_VOID);
typedef PP_VOID (*PPAPI_SVM_FRAMEDELAY_CALLBACK) (PP_BOOL bFrameDelayStatus);

#define SVMAPI_REPEAT_LIMITLESS			(0xFFFFFFFF)


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppSVMAPI_DYNAMIC_BLENDING_E
{
	eSVMAPI_DB_FRONTLEFT = 0,
	eSVMAPI_DB_FRONTRIGHT,
	eSVMAPI_DB_REARLEFT,
	eSVMAPI_DB_REARRIGHT,
	eSVMAPI_DB_MAX,
} PP_SVMAPI_DYNAMIC_BLENDING_E;

typedef enum ppSVMAPI_SWING_DIRECTION_E
{
	eSVMAPI_SWING_DIRECTION_AUTO = 0,
	eSVMAPI_SWING_DIRECTION_CLOCKWISE,
	eSVMAPI_SWING_DIRECTION_COUNTERCLOCKWISE,
	eSVMAPI_SWING_DIRECTION_MAX,
} PP_SVMAPI_SWING_DIRECTION_E;

typedef enum ppSVMAPI_FRAMEDELAY_CTRL_E
{
	eSVMAPI_FRAMEDELAY_CTRL_NULL = 0,
	eSVMAPI_FRAMEDELAY_CTRL_ON,				// If the frame delay occurs, turn on the replacement color of output.
	eSVMAPI_FRAMEDELAY_CTRL_ONOFF,			// If the frame delay occurs, turn on the replacement color of output. And if the frame delay does not occur, turn off the replacement color of output.
	eSVMAPI_FRAMEDELAY_CTRL_MAX,
} PP_SVMAPI_FRAMEDELAY_CTRL_E;


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
* Common Control
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_RESULT_E			PPAPI_SVM_SetInFrameBufferAddress(PP_CHANNEL_E IN enChannel, PP_U32* IN pu32Addr);
PP_U32*				PPAPI_SVM_GetInFrameBufferAddress(PP_CHANNEL_E IN enChannel);

PP_RESULT_E			PPAPI_SVM_SetOutFrameBufferAddress(PP_SVMMEM_OUT_FRAMEBUF_NUM_E IN enOutFBNum, PP_U32* IN pu32Addr);
PP_U32*				PPAPI_SVM_GetOutFrameBufferAddress(PP_SVMMEM_OUT_FRAMEBUF_NUM_E IN enOutFBNum);

PP_VOID				PPAPI_SVM_SetMirroring(PP_CHANNEL_E IN enChannel, PP_BOOL IN bHorizontal, PP_BOOL IN bVertical);

PP_RESULT_E			PPAPI_SVM_SetAntiAliasing(PP_CHANNEL_E IN enChannel, PP_SVMDRV_ANTI_ALIASING_STRENGTH_H_E IN enHorizotal, PP_SVMDRV_ANTI_ALIASING_STRENGTH_V_E IN enVertical);

PP_VOID				PPAPI_SVM_SetReplaceColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);
PP_VOID				PPAPI_SVM_SetInputReplaceColorOnOff(PP_CHANNEL_E IN enChannel, PP_BOOL IN bOn);
PP_BOOL				PPAPI_SVM_GetInputReplaceColorOnOff(PP_CHANNEL_E IN enChannel);
PP_VOID				PPAPI_SVM_SetOutputReplaceColorOnOff(PP_BOOL IN bOn);
PP_BOOL				PPAPI_SVM_GetOutputReplaceColorOnOff(PP_VOID);

PP_VOID				PPAPI_SVM_SetBackgroundColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);

PP_VOID				PPAPI_SVM_SetImageMaskColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);

PP_RESULT_E			PPAPI_SVM_SetEdgeEnhancement(PP_S16 IN s16Edge);

PP_VOID				PPAPI_SVM_SetDynamicBlending(PP_SVMAPI_DYNAMIC_BLENDING_E IN enPos, PP_U8 IN u8Ratio);
PP_VOID				PPAPI_SVM_SetDynamicBlendingOnOff(PP_BOOL IN bOn);

PP_VOID				PPAPI_SVM_SetWindowOffset(PP_S8 IN s8X, PP_S8 IN s8Y);

PP_VOID				PPAPI_SVM_SetOutputHold(PP_BOOL IN bEnable);
PP_U32*				PPAPI_SVM_GetHoldFrameBufferAddress(PP_FIELD_E IN enField);

PP_VOID				PPAPI_SVM_SetAutoBrightnessControlOnOff(PP_BOOL IN bOn);

PP_VOID				PPAPI_SVM_SetFrameDelayCtrl(PP_U32 IN u32DelayCnt, PP_SVMAPI_FRAMEDELAY_CTRL_E IN enOuputReplaceColor, PPDRV_SVM_CALLBACK_UNDERFLOW IN callback);
PP_U32				PPAPI_SVM_GetFrameDelayCtrlContinuousCount(PP_VOID);

PP_U32				PPAPI_SVM_GetVersion(PP_VOID);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* ViewMode Control
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_RESULT_E			PPAPI_SVM_SetFBLUTAddress(PP_VIEWMODE_E IN enView, PP_U32* IN pu32Addr, PP_FIELD_E IN enField);
PP_U32*				PPAPI_SVM_GetFBLUTAddress(PP_VIEWMODE_E IN enView, PP_FIELD_E IN enField);

PP_RESULT_E			PPAPI_SVM_SetLRLUTAddress(PP_VIEWMODE_E IN enView, PP_U32* IN pu32Addr, PP_FIELD_E IN enField);
PP_U32*				PPAPI_SVM_GetLRLUTAddress(PP_VIEWMODE_E IN enView, PP_FIELD_E IN enField);

PP_RESULT_E			PPAPI_SVM_SetBCLUTAddress(PP_VIEWMODE_E IN enView, PP_U32* IN pu32Addr, PP_FIELD_E IN enField);
PP_U32*				PPAPI_SVM_GetBCLUTAddress(PP_VIEWMODE_E IN enView, PP_FIELD_E IN enField);

PP_RESULT_E			PPAPI_SVM_SetBCAdditionalLUT(PP_VIEWMODE_E IN enView,
														  PP_SVMDRV_BC_ADDITIONAL_LUT_E IN enType, PP_SVMDRV_BC_ADDITIONAL_LUT_SUBCORE_E IN enSubCore,
														  PP_U32* IN pu32Addr, PP_U32 IN u32Size,
														  PP_FIELD_E IN enField);

PP_RESULT_E			PPAPI_SVM_SetImageRect(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_RECT_S* IN pstRect);
PP_RESULT_E			PPAPI_SVM_SetImageAlphaBlending(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U8 IN u8Alpha);
PP_RESULT_E			PPAPI_SVM_SetImageAddress(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U32* IN pu32Addr, PP_FIELD_E IN enField);
PP_U32*				PPAPI_SVM_GetImageAddress(PP_VIEWMODE_E IN enView,PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_FIELD_E IN enField);

PP_RESULT_E			PPAPI_SVM_SetSectionRect(PP_VIEWMODE_E IN enView, PP_SVMDRV_SECTION_NUMBER_E IN enNum, PP_RECT_S* IN pstRect);
PP_RESULT_E			PPAPI_SVM_SetAutoSectionRect(PP_BOOL IN bOn);

PP_VOID				PPAPI_SVM_SetMorphingRatio(PP_U8 IN u8Ratio);

PP_RESULT_E			PPAPI_SVM_SetView(PP_VIEWMODE_E IN enView, PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback);
#ifdef SVM_TOGGLE_VIEW_USE
PP_RESULT_E			PPAPI_SVM_SetToggleView(PP_VIEWMODE_E IN enView0, PP_VIEWMODE_E IN enView1, PP_U32 IN u32RepeatCount,
													PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback);
#endif /* SVM_TOGGLE_VIEW_USE */
PP_RESULT_E			PPAPI_SVM_SetSwingView(PP_VIEWMODE_E IN enDegreeStartView, PP_VIEWMODE_E IN enDegreeEndView, PP_SVMAPI_SWING_DIRECTION_E IN enDirection, PP_U32 IN u32RepeatCount,
													PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback);
PP_BOOL				PPAPI_SVM_SetCacheViewAddress(PP_VIEWMODE_E IN enView);

PP_U32				PPAPI_SVM_GetLoadedViewCount(PP_VOID);
PP_VIEWMODE_E		PPAPI_SVM_GetCurrentView(PP_VOID);


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Initiaize
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_RESULT_E			PPAPI_SVM_Initialize(PP_VOID);

#ifdef __cplusplus
}
#endif

#endif //__API_SVM_H__
