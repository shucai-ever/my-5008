/**
 * \file
 *
 * \brief	Canny Edge Detector driver
 *
 * Copyright (c) 2017 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __CANNYEDGE_H__
#define __CANNYEDGE_H__

#include "type.h"
#include "error.h"


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
typedef PP_VOID (*PPDRV_CANNYEDGE_CALLBACK_DONE)(PP_VOID);

/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppCANNYEDGE_INPUT_E
{
	eCANNYEDGE_INPUT_CH0 = 0,	// Front : default
	eCANNYEDGE_INPUT_CH1,		// Left
	eCANNYEDGE_INPUT_CH2,		// Right
	eCANNYEDGE_INPUT_CH3,		// Rear
	eCANNYEDGE_INPUT_QUAD,
	eCANNYEDGE_INPUT_SVM,
	eCANNYEDGE_INPUT_MAX,
} PP_CANNYEDGE_INPUT_E;

typedef enum ppCANNYEDGE_THRESHOLD_TYPE_E
{
	eCANNYEDGE_TH_TYPE_RATIO = 0,
	eCANNYEDGE_TH_TYPE_FIXED,
	eCANNYEDGE_TH_TYPE_MAX,
} PP_CANNYEDGE_THRESHOLD_TYPE_E;


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
#ifdef __cplusplus
EXTERN "C" {
#endif

PP_RESULT_E	PPDRV_CANNYEDGE_SetInput(PP_CANNYEDGE_INPUT_E IN enCh);

PP_RESULT_E	PPDRV_CANNYEDGE_SetDownScale(PP_BOOL IN bHScale, PP_BOOL IN bVScale);
PP_RESULT_E	PPDRV_CANNYEDGE_SetROI(PP_U16 IN u16StartLine, PP_U16 IN u16EndLine);

PP_RESULT_E	PPDRV_CANNYEDGE_SetThresholdType(PP_CANNYEDGE_THRESHOLD_TYPE_E IN enType);
PP_RESULT_E	PPDRV_CANNYEDGE_SetThresholdRatio(PP_F32 IN f32HighTh, PP_F32 IN f32LowTh);
PP_RESULT_E	PPDRV_CANNYEDGE_SetFixedThreshold(PP_U8 IN u8HighTh, PP_U8 IN u8LowTh);

PP_U32*		PPDRV_CANNYEDGE_GetBufferAddress(PP_U16* OUT pu16OutWidth, PP_U16* OUT pu16OutHeight);

PP_RESULT_E	PPDRV_CANNYEDGE_SetDoneHandler(PPDRV_CANNYEDGE_CALLBACK_DONE IN callback);

PP_RESULT_E	PPDRV_CANNYEDGE_SetEnable(PP_BOOL IN bEnable);

PP_RESULT_E	PPDRV_CANNYEDGE_Initialize(PP_U16 IN u16InWidth, PP_U16 IN u16InHeight,
										   PP_U16 IN u16OutWidth, PP_U16 IN u16OutHeight,
										   PP_U8 IN u8FrameBufferCount);
PP_RESULT_E	PPDRV_CANNYEDGE_InitializeByConfig(PP_U8 IN u8FrameBufferCount);
PP_VOID		PPDRV_CANNYEDGE_Deinitialize(PP_VOID);

#ifdef __cplusplus
}
#endif

#endif //__CANNYEDGE_H__