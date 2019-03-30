/**
 * \file
 *
 * \brief	SVM drivers
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __SVM_DRV_H__
#define __SVM_DRV_H__

#include "type.h"
#include "error.h"
#include "common.h"
#include "vin.h"

 
/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
typedef PP_VOID (*PPDRV_SVM_CALLBACK_VSYNC) (PP_VOID);
typedef PP_VOID (*PPDRV_SVM_CALLBACK_ERROR) (PP_VOID);
typedef PP_VOID (*PPDRV_SVM_CALLBACK_UNDERFLOW) (PP_BOOL bStatus);


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppSVMDRV_OUTPUT_PART_E
{
	eSVMDRV_OUTPUT_PART_FB = 0,
	eSVMDRV_OUTPUT_PART_LR,
	eSVMDRV_OUTPUT_PART_MAX,
} PP_SVMDRV_OUTPUT_PART_E;

typedef enum ppSVMDRV_FB_PART_E
{
	eSVMDRV_FB_PART_F = 0,
	eSVMDRV_FB_PART_B,
	eSVMDRV_FB_PART_MAX,
} PP_SVMDRV_FB_PART_E;

typedef enum ppSVMDRV_LR_PART_E
{
	eSVMDRV_LR_PART_L,
	eSVMDRV_LR_PART_R,
	eSVMDRV_LR_PART_MAX,
} PP_SVMDRV_LR_PART_E;

typedef enum ppSVMDRV_SUBPART_E
{
	eSVMDRV_SUBPART_FL = 0,
	eSVMDRV_SUBPART_FR,
	eSVMDRV_SUBPART_BR,
	eSVMDRV_SUBPART_BL,
	eSVMDRV_SUBPART_MAX,
} PP_SVMDRV_SUBPART_E;

typedef enum ppSVMDRV_COLOR_E
{
	eSVMDRV_COLOR_R	= 0,
	eSVMDRV_COLOR_G,
	eSVMDRV_COLOR_B,
	eSVMDRV_COLOR_MAX,
} PP_SVMDRV_COLOR_E;

typedef enum ppSVMDRV_SECTION_NUMBER_E
{
	eSVMDRV_SECTION_NUM_0 = 0,
	eSVMDRV_SECTION_NUM_1,
	eSVMDRV_SECTION_NUM_2,
	eSVMDRV_SECTION_NUM_3,
	eSVMDRV_SECTION_NUM_MAX,
} PP_SVMDRV_SECTION_NUMBER_E;

typedef enum ppSVMDRV_OUTPUTMODE_E
{
	eSVMDRV_OUTPUTMODE_BYPASS_FRONT = 0,								// Front bypass
	eSVMDRV_OUTPUTMODE_BYPASS_LEFT,										// Left bypass
	eSVMDRV_OUTPUTMODE_BYPASS_RIGHT,									// Right bypass
	eSVMDRV_OUTPUTMODE_BYPASS_REAR,										// Rear bypass
	eSVMDRV_OUTPUTMODE_QUAD,											// Quad
	eSVMDRV_OUTPUTMODE_LUT_FB,											// FB LUT
	eSVMDRV_OUTPUTMODE_LUT_LR,											// LR LUT
	eSVMDRV_OUTPUTMODE_LUT_FB_LR,										// FB LUT + LR LUT
	eSVMDRV_OUTPUTMODE_LUT_FB_LR_BC,									// FB LUT + LR LUT + BC LUT
	eSVMDRV_OUTPUTMODE_MAX,
} PP_SVMDRV_OUTPUTMODE_E;

typedef enum ppSVMDRV_IMG_NUMBER_E
{
	eSVMDRV_IMG_NUM_0 = 0,
	eSVMDRV_IMG_NUM_1,
	eSVMDRV_IMG_NUM_MAX,
} PP_SVMDRV_IMG_NUMBER_E;

typedef enum ppSVMDRV_ANTI_ALIASING_STRENGTH_H_E
{
	eSVMDRV_AA_H_1 = 0,
	eSVMDRV_AA_H_2,
	eSVMDRV_AA_H_3,
	eSVMDRV_AA_H_4,
	eSVMDRV_AA_H_5,
	eSVMDRV_AA_H_6,
	eSVMDRV_AA_H_7,
	eSVMDRV_AA_H_MAX,
} PP_SVMDRV_ANTI_ALIASING_STRENGTH_H_E;

typedef enum ppSVMAPI_ANTI_ALIASING_STRENGTH_V_E
{
	eSVMDRV_AA_V_1 = 0,
	eSVMDRV_AA_V_2,
	eSVMDRV_AA_V_3,
	eSVMDRV_AA_V_4,
	eSVMDRV_AA_V_MAX,
} PP_SVMDRV_ANTI_ALIASING_STRENGTH_V_E;

typedef enum ppSVMDRV_BC_ADDITIONAL_LUT_E
{
	eSVMDRV_BC_ADD_LUT_ALPHA_0 = 0,
	eSVMDRV_BC_ADD_LUT_ALPHA_1,
	eSVMDRV_BC_ADD_LUT_GRADIENT,
	eSVMDRV_BC_ADD_LUT_MAX,
} PP_SVMDRV_BC_ADDITIONAL_LUT_E;

typedef enum ppSVMDRV_BC_ADDITIONAL_LUT_SUBCORE_E
{
	eSVMDRV_BC_ADD_LUT_SUBCORE_0 = 0,
	eSVMDRV_BC_ADD_LUT_SUBCORE_1,
	eSVMDRV_BC_ADD_LUT_SUBCORE_MAX,
} PP_SVMDRV_BC_ADDITIONAL_LUT_SUBCORE_E;

typedef enum ppSVMDRV_OUT_FRAMBUF_NUM_E
{
	eSVMDRV_OUT_FB_NUM_0 = 0,
	eSVMDRV_OUT_FB_NUM_1,
	eSVMDRV_OUT_FB_NUM_2,
	eSVMDRV_OUT_FB_NUM_3,
	eSVMDRV_OUT_FB_NUM_MAX,
} PP_SVMDRV_OUT_FRAMEBUF_NUM_E;

typedef enum ppSVMDRV_MORPHING_NUM_E
{
	eSVMDRV_MORPING_NUM_0 = 0,
	eSVMDRV_MORPING_NUM_1,
	eSVMDRV_MORPING_NUM_MAX,
} PP_SVMDRV_MORPHING_NUM_E;


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppSVMDRV_BCCOEF_S											// coefficient for brightness control
{
	PP_U8 u8Coef1[eSVMDRV_COLOR_MAX][eSVMDRV_SUBPART_MAX];					// 0 ~ 255
	PP_U8 u8Coef2[eSVMDRV_COLOR_MAX][eSVMDRV_FB_PART_MAX];					// 0 ~ 127
	PP_U8 u8Coef3[eSVMDRV_COLOR_MAX][eSVMDRV_SUBPART_MAX];					// 0 ~ 255
	PP_U8 u8Coef4[eSVMDRV_COLOR_MAX][eSVMDRV_LR_PART_MAX];					// 0 ~ 127
} PP_SVMDRV_BCCOEF_S;

typedef struct ppSVMDRV_RGB_STATISTICS_S									// structur for RGB statistics
{
	PP_U32 u32Value[eSVMDRV_OUTPUT_PART_MAX][eSVMDRV_SUBPART_MAX][eSVMDRV_COLOR_MAX];
} PP_SVMDRV_RGB_STATISTICS_S;

typedef struct ppSVMDRV_BC_ADDITIONAL_LUT_S
{
	PP_U16 u16TotalCnt[eFIELD_MAX][eSVMDRV_BC_ADD_LUT_SUBCORE_MAX];
	PP_U32* pu32Addr[eFIELD_MAX][eSVMDRV_BC_ADD_LUT_SUBCORE_MAX];
} PP_SVMDRV_BC_ADDITIONAL_LUT_S;


/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
#ifdef __cplusplus
EXTERN "C" {
#endif

/************************************************************************************
* input control
************************************************************************************/
PP_RESULT_E	PPDRV_SVM_IN_SetAddress(PP_CHANNEL_E IN enChannel, PP_U32* IN pu32Addr);
PP_U32*		PPDRV_SVM_IN_GetAddress(PP_CHANNEL_E IN enChannel);

PP_RESULT_E	PPDRV_SVM_IN_SetMirroring(PP_CHANNEL_E IN enChannel, PP_BOOL IN bHorizontal, PP_BOOL IN bVertical);

PP_RESULT_E	PPDRV_SVM_IN_SetAntiAliasing(PP_CHANNEL_E IN enChannel, PP_SVMDRV_ANTI_ALIASING_STRENGTH_H_E IN enHorizotal, PP_SVMDRV_ANTI_ALIASING_STRENGTH_V_E IN enVertical);

PP_VOID		PPDRV_SVM_INOUT_SetReplaceColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);
PP_VOID		PPDRV_SVM_IN_SetReplaceColorEnable(PP_CHANNEL_E IN enChannel, PP_BOOL IN bOn);
PP_BOOL		PPDRV_SVM_IN_GetReplaceColorEnable(PP_CHANNEL_E IN enChannel);

PP_RESULT_E	PPDRV_SVM_IN_SetEnable(PP_BOOL IN bFrontEnable, PP_BOOL IN bLeftEnable, PP_BOOL IN bRightEnable, PP_BOOL IN bRearEnable);

/************************************************************************************
* svm control
************************************************************************************/
PP_RESULT_E	PPDRV_SVM_CTRL_SetImage(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_RECT_S IN stRect, PP_U8 IN u8Alpha);
PP_VOID		PPDRV_SVM_CTRL_SetImageMaskColor(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);
PP_RESULT_E	PPDRV_SVM_CTRL_SetImageAddr(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even);
PP_U32*		PPDRV_SVM_CTRL_GetImageAddr(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_FIELD_E IN enField);
PP_RESULT_E	PPDRV_SVM_CTRL_SetImageEnable(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_BOOL IN bEnable);

PP_VOID		PPDRV_SVM_CTRL_SetEdgeEnhancementFixedGain(PP_U8 IN u8Gain);
PP_RESULT_E	PPDRV_SVM_CTRL_SetEdgeEnhancementEnable(PP_BOOL IN bEnable);

PP_VOID		PPDRV_SVM_CTRL_SetDynamicblendingCoefficient(PP_U8 u8Coef1, PP_U8 u8Coef3, PP_U8 u8Coef6, PP_U8 u8Coef8);
PP_RESULT_E	PPDRV_SVM_CTRL_SetDynamicblendingEnable(PP_BOOL IN bEnable);

PP_RESULT_E	PPDRV_SVM_CTRL_GetRGBStatistics(PP_SVMDRV_RGB_STATISTICS_S* OUT pstStats);
PP_RESULT_E	PPDRV_SVM_CTRL_SetBCCoefficient(PP_SVMDRV_BCCOEF_S* IN pstCoefBC);
PP_BOOL		PPDRV_SVM_CTRL_GetBCState(PP_VOID);

PP_VOID		PPDRV_SVM_CTRL_SetMorphingRatio(PP_U8 IN u8FBLUTRatio, PP_U8 IN u8LRLUTRatio);

PP_RESULT_E	PPDRV_SVM_CTRL_SetFBLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum);
PP_RESULT_E	PPDRV_SVM_CTRL_GetFBLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum);
PP_RESULT_E	PPDRV_SVM_CTRL_SetLRLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum);
PP_RESULT_E	PPDRV_SVM_CTRL_GetLRLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum);
PP_RESULT_E	PPDRV_SVM_CTRL_SetBCLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even);
PP_RESULT_E	PPDRV_SVM_CTRL_GetBCLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even);
PP_RESULT_E	PPDRV_SVM_CTRL_SetBCAdditionalLUT(PP_SVMDRV_BC_ADDITIONAL_LUT_E enType, PP_SVMDRV_BC_ADDITIONAL_LUT_S* IN stBCAdd);

PP_RESULT_E	PPDRV_SVM_CTRL_SetOutputMode(PP_SVMDRV_OUTPUTMODE_E enOutputMode); 

PP_VOID		PPDRV_SVM_CTRL_GetLuminanceAverage(PP_U16* OUT pu16Front, PP_U16* OUT pu16Left, PP_U16* OUT pu16Right, PP_U16* OUT pu16Rear);

PP_RESULT_E	PPDRV_SVM_CTRL_SetEnable(PP_BOOL IN bEnable);
PP_BOOL		PPDRV_SVM_CTRL_GetEnable(PP_VOID);

/************************************************************************************
* output control
************************************************************************************/
PP_RESULT_E	PPDRV_SVM_OUT_SetAddress(PP_SVMDRV_OUT_FRAMEBUF_NUM_E IN enOutFBNum, PP_U32* IN pu32Addr);
PP_U32*		PPDRV_SVM_OUT_GetAddress(PP_SVMDRV_OUT_FRAMEBUF_NUM_E IN enOutFBNum);

//PP_VOID	PPDRV_SVM_INOUT_SetReplaceColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);
PP_VOID		PPDRV_SVM_OUT_SetReplaceColorEnable(PP_BOOL IN bOn);
PP_BOOL		PPDRV_SVM_OUT_GetReplaceColorEnable(PP_VOID);

PP_VOID		PPDRV_SVM_CTRL_SetBackgroundColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr);

PP_RESULT_E	PPDRV_SVM_OUT_SetSection(PP_SVMDRV_SECTION_NUMBER_E IN enSectionNumber, PP_RECT_S IN stSectionRect);
PP_RESULT_E	PPDRV_SVM_OUT_SetSectionEnable(PP_SVMDRV_SECTION_NUMBER_E IN enSectionNumber, PP_BOOL IN bEnable);

PP_RESULT_E	PPDRV_SVM_OUT_SetTilt(PP_S8 IN s8X, PP_S8 IN s8Y);

PP_RESULT_E	PPDRV_SVM_OUT_SetHold(PP_BOOL IN bEnable);
PP_U32*		PPDRV_SVM_OUT_GetHoldFrameBufferAddress(PP_FIELD_E IN enField);

PP_RESULT_E	PPDRV_SVM_OUT_SetEnable(PP_BOOL IN bFBEnable, PP_BOOL IN bLREnable);

/************************************************************************************
* callback function (ISR)
************************************************************************************/
PP_VOID		PPDRV_SVM_ISR_SetVsyncHandler(PPDRV_SVM_CALLBACK_VSYNC IN callback);
PP_VOID		PPDRV_SVM_ISR_SetErrorHandler(PPDRV_SVM_CALLBACK_ERROR IN callback);

/************************************************************************************
* Check underflow error (frame delay)
************************************************************************************/
PP_VOID		PPDRV_SVM_SetUnderflowCtrlHandler(PP_U32 IN u32CheckCnt, PPDRV_SVM_CALLBACK_UNDERFLOW IN callback);
PP_U32		PPDRV_SVM_GetUnderflowCtrlContinuousCount(PP_VOID);

/************************************************************************************
* Initalize
************************************************************************************/
PP_U32		PPDRV_SVM_CTRL_GetVersion(PP_VOID);
PP_RESULT_E PPDRV_SVM_Initialize(_VID_RESOL enInput, _VID_RESOL enOutput, PP_U8 u8InFrameBufCnt, PP_U8 u8OutFrameBufCnt);

#ifdef __cplusplus
}
#endif

#endif //__SVM_DRV_H__