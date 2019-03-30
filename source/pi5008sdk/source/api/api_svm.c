/**
 * \file
 *
 * \brief	SVM APIs
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#include <string.h>

#include "debug.h"
#include "api_vin.h"
#include "osal.h"
#include "api_ipc.h"
#include "prj_config.h"
#include "sub_intr.h"

#include "api_svm.h"

PP_RESULT_E			PPAPI_SVM_CraeteView(PP_VIEWMODE_E IN enView);
PP_VOID				PPAPI_SVM_DeleteView(PP_VIEWMODE_E IN enView);
PP_RESULT_E			PPAPI_SVM_CraeteAllView(PP_VOID);
PP_VOID				PPAPI_SVM_DeleteAllView(PP_VOID);


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
typedef PP_BOOL (*PPAPI_SVM_CALLBACK_SWITCHING) (PP_U32 u32StartView, PP_U32 u32EndView, PP_U32 u32View, PP_U8 u8Ratio);

#define SVMAPI_SWITCHING_BUSY_CONTROL
#define SVMAPI_LOADING_ERROR_MAX				(100)

#define SVMAPI_DISPLAY_TIMER					(1)

#define SVMAPI_AUTOSECTION_SIZE					(8)

#if (LOG_BUILD_LEVEL != LOG_LVL_NONE)
#define _RET(x)						{PPAPI_SVM_Msg(x, __FUNCTION__, __LINE__); return x;}
#define _RETNULL(x)					{PPAPI_SVM_Msg(x, __FUNCTION__, __LINE__); return PP_NULL;}
#define _RET0(x)					{PPAPI_SVM_Msg(x, __FUNCTION__, __LINE__); return 0;}
#define _RETVOID(x)					{PPAPI_SVM_Msg(x, __FUNCTION__, __LINE__); return;}
#define _RETFALSE(x)				{PPAPI_SVM_Msg(x, __FUNCTION__, __LINE__); return PP_FALSE;}
#else
#define _RET(x)						{return x;}
#define _RETNULL(x)					{return PP_NULL;}
#define _RET0(x)					{return 0;}
#define _RETVOID(x)					{return;}
#define _RETFALSE(x)				{return PP_FALSE;}
#endif


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppSVMAPI_SWITCHING_STOP_STATE_E
{
	eSVMAPI_SWITCHING_STOP_STATE_NON_STOP = 0,
	eSVMAPI_SWITCHING_STOP_STATE_START_STOP,
	eSVMAPI_SWITCHING_STOP_STATE_END_STOP,
} SVMAPI_SWITCHING_STOP_STATE_E;

typedef enum ppSVMAPI_AUTOBC_STATE_E
{
	eSVMAPI_AUTOBC_STATE_ON = 0,
	eSVMAPI_AUTOBC_STATE_APPLY_ON,
	eSVMAPI_AUTOBC_STATE_OFF,
	eSVMAPI_AUTOBC_STATE_APPLY_OFF,
	eSVMAPI_AUTOBC_STATE_MAX,
} PP_SVMAPI_AUTOBC_STATE_E;


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppSVMAPI_CTRL_S
{
	PP_BOOL bInit;

	PP_SVMAPI_AUTOBC_STATE_E enAutoBC;

	PP_U8 u8ExcuteVsync;
	
	PP_U32 u32CurView;
	
	PP_SIZE_S stOutSize;

	PP_BOOL bAutoSectionRect;

	PP_U8 u8DynamicBlending[eSVMAPI_DB_MAX];

	PP_SVMDRV_MORPHING_NUM_E enMorphinNum;
	PPAPI_SVM_CALLBACK_SWITCHING cbViewHandler;

	PP_BOOL bReColorByFrameDelay;
	PP_SVMAPI_FRAMEDELAY_CTRL_E enFrameDelayCtrl;
	PPAPI_SVM_FRAMEDELAY_CALLBACK cbFrameDelayHandler;
} PP_SVMAPI_CTRL_S;

typedef struct ppSVMAPI_LUT_S
{
	PP_U32* pu32FBAddr[eFIELD_MAX];											// fb lut address 
	PP_U32* pu32LRAddr[eFIELD_MAX];											// lr lut address
	PP_U32* pu32BCAddr[eFIELD_MAX];											// bc lut address
	
	PP_BOOL bBCAddEnable[eSVMDRV_BC_ADD_LUT_MAX];
	PP_SVMDRV_BC_ADDITIONAL_LUT_S stBCAdd[eSVMDRV_BC_ADD_LUT_MAX];			// additional lut for bc
} PP_SVMAPI_LUT_S;

typedef struct ppSVMAPI_IMG_S
{
	PP_U32* pu32Addr[eFIELD_MAX];											// image address
	PP_RECT_S stRect;														// rectangle area for car output
	PP_U8 u8Alpha;
} PP_SVMAPI_IMG_S;

typedef struct ppSVMAPI_VIEW_S
{
	PP_SVMAPI_LUT_S stLut;													// fb, lr, bc, bc add lut
	PP_SVMAPI_IMG_S stImg[eSVMDRV_IMG_NUM_MAX];								// yuv422 image : max 2

	PP_RECT_S stSectionRect[eSVMDRV_SECTION_NUM_MAX];						// structure for section. Section is used to reduce MBW(Memory Band Width).
} PP_SVMAPI_VIEW_S;

typedef struct ppSVMAPI_SWITCHING_INFO_S
{
	PP_U32 u32StartView;
	PP_U32 u32EndView;
	PP_U32 u32View;
	PP_U8 u8Ratio;
} PP_SVMAPI_SWITCHING_INFO_S;

typedef struct ppSVMAPI_SWITCHING_S
{
	PP_U32 u32Cnt;
	PP_U32 u32CurIndex;
	
	PP_U32 u32RepeatCnt;
	PP_U32 u32RepeatIndex;

	PP_U32 u32LoadingErrCnt;

	TimerHandle_t xTimer;
	PP_U32 u32TimerView;
	PP_U32 u32DisplayView;

	PP_U32 u32SwingStartView;
	PP_U32 u32SwingEndView;
	PP_U32 u32SwingPauseView;
	PP_SVMAPI_SWING_DIRECTION_E enSwingDirection;

	SVMAPI_SWITCHING_STOP_STATE_E enStop;
	
	PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY cbDisplay;
	PPAPI_SVM_SWITCHING_CALLBACK_FINISH cbFinish;
	
	PP_SVMAPI_SWITCHING_INFO_S stInfo[eVIEWMODE_TOTAL_MAX];
} PP_SVMAPI_SWITCHING_S;


/***************************************************************************************************************************************************************
 * Global Variable
***************************************************************************************************************************************************************/
STATIC PP_U8 gu8SVM_vsynCntForCk = 0;
STATIC PP_BOOL bSVM_InIntr = PP_FALSE;
STATIC PP_SVMAPI_CTRL_S gstCtrl;
STATIC PP_SVMAPI_VIEW_S* gstView[eVIEWMODE_TOTAL_MAX] = {PP_NULL,};
STATIC PP_SVMAPI_SWITCHING_S gstSwitching;


/***************************************************************************************************************************************************************
 * Check error
***************************************************************************************************************************************************************/
#define _16BYTEALIGNCHECK(x)		{if ( ((PP_U32)x & 0xf) != 0x0 ) _RET(eERROR_INVALID_ALIGN);}

#define _MAXCHECK(x,y)				{if ( x >= y ) _RET(eERROR_INVALID_ARGUMENT);}
#define _MAXCHECKNULL(x,y)			{if ( x >= y ) _RETNULL(eERROR_INVALID_ARGUMENT);}
#define _MAXCHECK0(x,y)				{if ( x >= y ) _RET0(eERROR_INVALID_ARGUMENT);}
#define _MAXCHECKVOID(x,y)			{if ( x >= y ) _RETVOID(eERROR_INVALID_ARGUMENT);}
#define _MAXCHECKFALSE(x,y)			{if ( x >= y ) _RETFALSE(eERROR_INVALID_ARGUMENT);}

#define _INITCHECK					{if ( !gstCtrl.bInit ) _RET(eERROR_SVM_NOT_INITIALIZE);}
#define _INITCHECKNULL				{if ( !gstCtrl.bInit ) _RETNULL(eERROR_SVM_NOT_INITIALIZE);}
#define _INITCHECK0					{if ( !gstCtrl.bInit ) _RET0(eERROR_SVM_NOT_INITIALIZE);}
#define _INITCHECKVOID				{if ( !gstCtrl.bInit ) _RETVOID(eERROR_SVM_NOT_INITIALIZE);}

#define _MAXVIEWCHECK				{if ( enView >= eVIEWMODE_TOTAL_MAX ) _RET(eERROR_SVM_LIMIT_VIEWMODE);}
#define _MAXVIEWCHECKNULL			{if ( enView >= eVIEWMODE_TOTAL_MAX ) _RETNULL(eERROR_SVM_LIMIT_VIEWMODE);}
#define _MAXVIEWCHECK0				{if ( enView >= eVIEWMODE_TOTAL_MAX ) _RET0(eERROR_SVM_LIMIT_VIEWMODE);}
#define _MAXVIEWCHECKVOID			{if ( enView >= eVIEWMODE_TOTAL_MAX ) _RETVOID(eERROR_SVM_LIMIT_VIEWMODE);}

#define _HANDLECHECK				{if ( !gstView[enView] ) _RET(eERROR_SVM_NOT_CREATED_VIEWMODE);}
#define _HANDLECHECKNULL			{if ( !gstView[enView] ) _RETNULL(eERROR_SVM_NOT_CREATED_VIEWMODE);}
#define _HANDLECHECK0				{if ( !gstView[enView] ) _RET0(eERROR_SVM_NOT_CREATED_VIEWMODE);}
#define _HANDLECHECKVOID			{if ( !gstView[enView] ) _RETVOID(eERROR_SVM_NOT_CREATED_VIEWMODE);}

#if (BD_VIN_INTERLACE == 1)
#define _FIELDCHECK					{_MAXCHECK(enField, eFIELD_MAX)}
#define _FIELDCHECKNULL				{_MAXCHECKNULL(enField, eFIELD_MAX)}
#define _FIELDCHECK0				{_MAXCHECK0(enField, eFIELD_MAX)}
#define _FIELDCHECKVOID				{_MAXCHECKVOID(enField, eFIELD_MAX)}
#else
#define _FIELDCHECK					{if ( enField != eFIELD_NONE ) _RET(eERROR_INVALID_ARGUMENT);}
#define _FIELDCHECKNULL				{if ( enField != eFIELD_NONE ) _RETNULL(eERROR_INVALID_ARGUMENT);}
#define _FIELDCHECK0				{if ( enField != eFIELD_NONE ) _RET0(eERROR_INVALID_ARGUMENT);}
#define _FIELDCHECKVOID				{if ( enField != eFIELD_NONE ) _RETVOID(eERROR_INVALID_ARGUMENT);}
#endif

#define _VIEWCHECK					{_MAXVIEWCHECK; _HANDLECHECK}
#define _VIEWCHECKNULL				{_MAXVIEWCHECKNULL; _HANDLECHECKNULL}
#define _VIEWCHECK0					{_MAXVIEWCHECK0; _HANDLECHECK0}
#define _VIEWCHECKVOID				{_MAXVIEWCHECKVOID; _HANDLECHECKVOID}


/*======================================================================================================================
 = callback function
======================================================================================================================*/
STATIC PP_BOOL PPAPI_SVM_ViewSwitching(PP_VIEWMODE_E enStartView, PP_VIEWMODE_E enEndView, PP_VIEWMODE_E enView, PP_U8 u8Ratio)
{
	PP_S32 i = 0;
	PP_VIEWMODE_E enOutModeView = enView;

	if ( enView == VIEWMODE_NULL ) return PP_FALSE;

	if ( enStartView != VIEWMODE_NULL && enEndView != VIEWMODE_NULL )
	{
		if ( !PPAPI_SVMMEM_IsLoaded(enView) || !PPAPI_SVMMEM_IsLoaded(enStartView) || !PPAPI_SVMMEM_IsLoaded(enEndView) ) return PP_FALSE;
		
		PPDRV_SVM_CTRL_SetFBLUTAddress(gstView[enStartView]->stLut.pu32FBAddr[eFIELD_ODD], gstView[enStartView]->stLut.pu32FBAddr[eFIELD_EVEN], eSVMDRV_MORPING_NUM_0);
		PPDRV_SVM_CTRL_SetLRLUTAddress(gstView[enStartView]->stLut.pu32LRAddr[eFIELD_ODD], gstView[enStartView]->stLut.pu32LRAddr[eFIELD_EVEN], eSVMDRV_MORPING_NUM_0);
		PPDRV_SVM_CTRL_SetFBLUTAddress(gstView[enEndView]->stLut.pu32FBAddr[eFIELD_ODD], gstView[enEndView]->stLut.pu32FBAddr[eFIELD_EVEN], eSVMDRV_MORPING_NUM_1);
		PPDRV_SVM_CTRL_SetLRLUTAddress(gstView[enEndView]->stLut.pu32LRAddr[eFIELD_ODD], gstView[enEndView]->stLut.pu32LRAddr[eFIELD_EVEN], eSVMDRV_MORPING_NUM_1);
	}
	else
	{
		enStartView = VIEWMODE_NULL;
		enEndView = VIEWMODE_NULL;

		if ( !PPAPI_SVMMEM_IsLoaded(enView) ) return PP_FALSE;

		PPDRV_SVM_CTRL_SetFBLUTAddress(gstView[enView]->stLut.pu32FBAddr[eFIELD_ODD], gstView[enView]->stLut.pu32FBAddr[eFIELD_EVEN], eSVMDRV_MORPING_NUM_0);
		PPDRV_SVM_CTRL_SetLRLUTAddress(gstView[enView]->stLut.pu32LRAddr[eFIELD_ODD], gstView[enView]->stLut.pu32LRAddr[eFIELD_EVEN], eSVMDRV_MORPING_NUM_0);
	}

	gstCtrl.u32CurView = enView;

	switch ( enView )
	{
	case eVIEWMODE_BASIC_FRONT_BYPASS:
		PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_BYPASS_FRONT);
		break;
	case eVIEWMODE_BASIC_LEFT_BYPASS:
		PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_BYPASS_LEFT);
		break;
	case eVIEWMODE_BASIC_RIGHT_BYPASS:
		PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_BYPASS_RIGHT);
		break;
	case eVIEWMODE_BASIC_REAR_BYPASS:
		PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_BYPASS_REAR);
		break;
	case eVIEWMODE_BASIC_QUAD:
		PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_QUAD);
		break;
	default:
		if ( enStartView != VIEWMODE_NULL ) enOutModeView = enStartView;
		if ( gstView[enOutModeView]->stLut.pu32FBAddr[eFIELD_ODD] && !gstView[enOutModeView]->stLut.pu32LRAddr[eFIELD_ODD] )
			PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_LUT_FB);
		else if ( !gstView[enOutModeView]->stLut.pu32FBAddr[eFIELD_ODD] && gstView[enOutModeView]->stLut.pu32LRAddr[eFIELD_ODD] )
			PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_LUT_LR);
		else if ( gstView[enOutModeView]->stLut.pu32FBAddr[eFIELD_ODD] && gstView[enOutModeView]->stLut.pu32LRAddr[eFIELD_ODD] && !gstView[enOutModeView]->stLut.pu32BCAddr[eFIELD_ODD] )
			PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_LUT_FB_LR);
		else
			PPDRV_SVM_CTRL_SetOutputMode(eSVMDRV_OUTPUTMODE_LUT_FB_LR_BC);
		break;
	}

	PPDRV_SVM_CTRL_SetBCLUTAddress(gstView[enView]->stLut.pu32BCAddr[eFIELD_ODD], gstView[enView]->stLut.pu32BCAddr[eFIELD_EVEN]);

#if (BD_VIN_INTERLACE == 1)
	if ( gstView[enView]->stLut.pu32BCAddr[eFIELD_ODD] && gstView[enView]->stLut.pu32BCAddr[eFIELD_EVEN] )
#else
	if ( gstView[enView]->stLut.pu32BCAddr[eFIELD_NONE] )
#endif
	{
		for ( i = eSVMDRV_BC_ADD_LUT_ALPHA_0; i < eSVMDRV_BC_ADD_LUT_MAX; i++ )
		{
			if ( gstView[enView]->stLut.bBCAddEnable[i] )
				PPDRV_SVM_CTRL_SetBCAdditionalLUT(i, &gstView[enView]->stLut.stBCAdd[i]);
			else
				PPDRV_SVM_CTRL_SetBCAdditionalLUT(i, PP_NULL);
		}
	}

	// section
	for ( i = eSVMDRV_SECTION_NUM_0; i < eSVMDRV_SECTION_NUM_MAX; i++ )
	{
		if ( gstView[enView]->stSectionRect[i].u16Width && gstView[enView]->stSectionRect[i].u16Height )
		{
			PPDRV_SVM_OUT_SetSection(i, gstView[enView]->stSectionRect[i]);
			PPDRV_SVM_OUT_SetSectionEnable(i, PP_TRUE);
		}
		else
		{
			PPDRV_SVM_OUT_SetSectionEnable(i, PP_FALSE);
		}
	}

	// image
	for ( i = eSVMDRV_IMG_NUM_0; i < eSVMDRV_IMG_NUM_MAX; i++ )
	{
		if ( gstView[enView]->stImg[i].pu32Addr[eFIELD_ODD] && gstView[enView]->stImg[i].stRect.u16Width && gstView[enView]->stImg[i].stRect.u16Height )
		{
			PPDRV_SVM_CTRL_SetImage(i, gstView[enView]->stImg[i].stRect, gstView[enView]->stImg[i].u8Alpha);
			PPDRV_SVM_CTRL_SetImageAddr(i, gstView[enView]->stImg[i].pu32Addr[eFIELD_ODD], gstView[enView]->stImg[i].pu32Addr[eFIELD_EVEN]);
			PPDRV_SVM_CTRL_SetImageEnable(i, PP_TRUE);
		}
		else
		{
			PPDRV_SVM_CTRL_SetImageEnable(i, PP_FALSE);
		}
	}
	
	PPAPI_SVM_SetMorphingRatio(u8Ratio);

	PPDRV_SVM_CTRL_SetEnable(PP_TRUE);

	return PP_TRUE;
}

STATIC PP_VOID PPAPI_SVM_SetViewHandler(PPAPI_SVM_CALLBACK_SWITCHING IN callback)
{
	gstCtrl.cbViewHandler = callback;
}

STATIC PP_VOID PPAPI_SVM_DisplayTimerHandler(TimerHandle_t IN timer)
{ 
	gstSwitching.cbDisplay(gstSwitching.u32TimerView);
}

STATIC PP_BOOL PPAPI_SVM_SetDisplayTimer(PP_VOID)
{
	BaseType_t xHigherPriorityTaskWoken = PP_FALSE; 
	
	gstSwitching.u32TimerView = gstSwitching.u32DisplayView;
	gstSwitching.u32DisplayView = VIEWMODE_NULL;
	if ( xTimerStartFromISR(gstSwitching.xTimer, &xHigherPriorityTaskWoken) != PP_TRUE )
		return PP_FALSE;
	
	return PP_TRUE;
}

STATIC PP_VOID PPAPI_SVM_Switching_Display(PP_VOID)
{
	if ( gstCtrl.u8ExcuteVsync < 3 )
	{
		gstCtrl.u8ExcuteVsync++;
		if ( gstCtrl.u8ExcuteVsync == 2 )	// 2nd vsync after 1st view changing
			PPAPI_SVM_SetOutputReplaceColorOnOff(PP_FALSE);
	}
	
	if ( gstSwitching.u32DisplayView != VIEWMODE_NULL )
	{
		if ( gstSwitching.cbDisplay )
		{
			if ( PPAPI_SVM_SetDisplayTimer() == PP_FALSE )
			{
				//LOG_DEBUG("[SVMAPI] (%s_%d) : Queue of timer is full\n", __FUNCTION__, __LINE__);
				gstSwitching.cbDisplay(gstSwitching.u32TimerView);
			}
		}
	}
}

STATIC PP_VOID PPAPI_SVM_Switching_Finish(PP_VOID)
{
	PPAPI_SVM_SetViewHandler(PP_NULL);
	PPAPI_SVM_Switching_Display();
	if ( gstSwitching.cbFinish ) gstSwitching.cbFinish();
}

STATIC PP_VOID PPAPI_SVM_Vsync_Handler(PP_VOID)
{
	bSVM_InIntr = PP_TRUE;
	gu8SVM_vsynCntForCk++;
	
	if ( gstCtrl.cbViewHandler )
	{
		if ( (gstSwitching.enStop != eSVMAPI_SWITCHING_STOP_STATE_END_STOP) &&
			 (gstSwitching.u32LoadingErrCnt < SVMAPI_LOADING_ERROR_MAX) &&
			 (gstSwitching.u32RepeatCnt == SVMAPI_REPEAT_LIMITLESS || gstSwitching.u32RepeatIndex < gstSwitching.u32RepeatCnt) )
		{
			PP_BOOL bSkip = PP_FALSE;
			PP_U32 u32CurDisplayView = gstSwitching.u32DisplayView;
			
			PPAPI_SVM_Switching_Display();

			if ( gstSwitching.enStop ==  eSVMAPI_SWITCHING_STOP_STATE_START_STOP )
			{
				PP_U32 u32View = u32CurDisplayView;
				if ( !(!PPAPI_SVMMEM_IsData(u32View, eSVMMEM_DATA_SUBTYPE_FB_ODD) && !PPAPI_SVMMEM_IsData(u32View, eSVMMEM_DATA_SUBTYPE_LR_ODD) && PPAPI_SVMMEM_IsData(u32View, eSVMMEM_DATA_SUBTYPE_BC_ODD)) )
				{
					gstSwitching.enStop = eSVMAPI_SWITCHING_STOP_STATE_END_STOP;
					gstSwitching.u32SwingPauseView = u32View;
					bSkip = PP_TRUE;
				}
			}
			
			if ( !bSkip )
			{
				if ( gstCtrl.cbViewHandler(gstSwitching.stInfo[gstSwitching.u32CurIndex].u32StartView, gstSwitching.stInfo[gstSwitching.u32CurIndex].u32EndView, gstSwitching.stInfo[gstSwitching.u32CurIndex].u32View, gstSwitching.stInfo[gstSwitching.u32CurIndex].u8Ratio) )
				{
					//printf("#### start : %d, end : %d, view : %d, ratio : %d\n", gstSwitching.stInfo[gstSwitching.u32CurIndex].u32StartView, gstSwitching.stInfo[gstSwitching.u32CurIndex].u32EndView, gstSwitching.stInfo[gstSwitching.u32CurIndex].u32View, gstSwitching.stInfo[gstSwitching.u32CurIndex].u8Ratio);
					
					gstSwitching.u32DisplayView = gstSwitching.stInfo[gstSwitching.u32CurIndex].u32View;
					gstSwitching.u32CurIndex++;
					if ( gstSwitching.u32CurIndex >= gstSwitching.u32Cnt )
					{
						gstSwitching.u32CurIndex = 0;
						gstSwitching.u32RepeatIndex++;
					}

					gstSwitching.u32LoadingErrCnt = 0;
				}
				else
				{
					gstSwitching.u32LoadingErrCnt++;
				}
			}
		}
		else
		{
			if ( gstSwitching.u32LoadingErrCnt >= SVMAPI_LOADING_ERROR_MAX )
				LOG_WARNING_ISR("[SVMAPI] (%s_%d) : Loading failed.\n", __FUNCTION__, __LINE__);
			
			PPAPI_SVM_Switching_Finish();

			if ( gstSwitching.enStop != eSVMAPI_SWITCHING_STOP_STATE_END_STOP )
				gstSwitching.u32SwingPauseView = VIEWMODE_NULL;
			
			gstSwitching.enStop = eSVMAPI_SWITCHING_STOP_STATE_NON_STOP;
		}
	}

	if ( gstCtrl.enAutoBC == eSVMAPI_AUTOBC_STATE_APPLY_ON )
	{
		PPAPI_IPC_SetBrightCtrl(PP_TRUE, 0);
		gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_ON;
	}
	else if ( gstCtrl.enAutoBC == eSVMAPI_AUTOBC_STATE_APPLY_OFF )
	{
		PPAPI_IPC_SetBrightCtrl(PP_FALSE, 0);
		gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_OFF;
	}		

	bSVM_InIntr = PP_FALSE;
}

STATIC PP_VOID PPAPI_SVM_Underflow_Handler(PP_BOOL bStatus)
{
	if ( bStatus && (gstCtrl.enFrameDelayCtrl == eSVMAPI_FRAMEDELAY_CTRL_ON || gstCtrl.enFrameDelayCtrl == eSVMAPI_FRAMEDELAY_CTRL_ONOFF) )
	{
		gstCtrl.bReColorByFrameDelay = PP_TRUE;
		PPDRV_SVM_OUT_SetReplaceColorEnable(PP_TRUE);
	}
	else if ( gstCtrl.enFrameDelayCtrl == eSVMAPI_FRAMEDELAY_CTRL_ONOFF )
	{
		if ( gstCtrl.bReColorByFrameDelay )
			PPDRV_SVM_OUT_SetReplaceColorEnable(PP_FALSE);
		gstCtrl.bReColorByFrameDelay = PP_FALSE;
	}
	
	if ( gstCtrl.cbFrameDelayHandler ) gstCtrl.cbFrameDelayHandler(bStatus);
}


/*======================================================================================================================
 = Local function
======================================================================================================================*/
#if (LOG_BUILD_LEVEL != LOG_LVL_NONE)
STATIC PP_VOID PPAPI_SVM_Msg(PP_S32 u32Ret, const PP_CHAR* cFunction, PP_U32 u32Line)
{
	PP_CHAR szError[64];

	switch (u32Ret)
	{
	case eERROR_NO_MEM:
		strcpy(szError, "eERROR_NO_MEM");
		break;
	case eERROR_INVALID_ARGUMENT:
		strcpy(szError, "eERROR_INVALID_ARGUMENT");
		break;
	case eERROR_SVM_NOT_INITIALIZE:
		strcpy(szError, "eERROR_SVM_NOT_INITIALIZE");
		break;
	case eERROR_SVM_VIEWMODE_NOT_SELECTED:
		strcpy(szError, "eERROR_SVM_VIEWMODE_NOT_SELECTED");
		break;
	case eERROR_SVM_LIMIT_VIEWMODE:
		strcpy(szError, "eERROR_SVM_LIMIT_VIEWMODE");
		break;
	case eERROR_SVM_NOT_CREATED_VIEWMODE:
		strcpy(szError, "eERROR_SVM_NOT_CREATED_VIEWMODE");
		break;
	case eERROR_SVM_UNUSE_LUT_VIEWMODE:
		strcpy(szError, "eERROR_SVM_UNUSE_LUT_VIEWMODE");
		break;
	case eERROR_SVM_UNUSE_IMG_VIEWMODE:
		strcpy(szError, "eERROR_SVM_UNUSE_IMG_VIEWMODE");
		break;
	case eERROR_SVM_RUNNING:
		strcpy(szError, "eERROR_SVM_RUNNING");
		break;
	case eERROR_SYS_DMA_ERROR:
		strcpy(szError, "eERROR_SYS_DMA_ERROR");
		break;
	case eERROR_SVM_MORPHING_RUNNING:
		strcpy(szError, "eERROR_SVM_MORPHING_RUNNING");
		break;
	case eERROR_SVM_INVALID_LUT:
		strcpy(szError, "eERROR_SVM_INVALID_LUT");
		break;
	case eERROR_SVM_RESOUTION_P2I:
		strcpy(szError, "eERROR_SVM_RESOUTION_P2I");
		break;
	case eERROR_INVALID_ALIGN:
		strcpy(szError, "eERROR_INVALID_ALIGN");
		break;
	case eERROR_BUSY:
		strcpy(szError, "eERROR_BUSY");
		break;
	case eERROR_SVM_NOT_LOADING:
		strcpy(szError, "eERROR_SVM_NOT_LOADING");
		break;
	default:
		break;
	}

	LOG_WARNING("[SVMAPI] (%s_%d) : %s\n", cFunction, u32Line, szError);
}
#endif

STATIC PP_RESULT_E PPAPI_SVM_CraeteDefaultView(PP_VOID)
{
	PP_U32 i;
	PP_RESULT_E enRet = eSUCCESS;

	for ( i = eVIEWMODE_BASIC_FRONT_BYPASS; i < eVIEWMODE_TOTAL_MAX; i++ )
	{
		enRet = PPAPI_SVM_CraeteView(i);
		if ( enRet != eSUCCESS )
		{
			PPAPI_SVM_DeleteAllView();
			break;
		}
	}

	return enRet;
}

STATIC PP_BOOL PPAPI_SVM_SetAddress(PP_VIEWMODE_E enView, PP_BOOL bLoading)
{
	PP_U32 i, j, k;
	PP_SVMMEM_DATA_SUBTYPE_E u8AddLutPos;
	PP_SVMMEM_LOAD_RET_E enLoadRet = eSVMMEM_LOAD_RET_FAILURE;
#if (BD_VIN_INTERLACE == 1)
	PP_U8 u8FieldMax = eFIELD_EVEN;
#else
	PP_U8 u8FieldMax = eFIELD_NONE;
#endif

	if ( enView < eVIEWMODE_LOAD_MAX )
	{
		enLoadRet = PPAPI_SVMMEM_LoadData(enView, bLoading);
		if ( enLoadRet == eSVMMEM_LOAD_RET_FAILURE )
		{
			printf("[SVMAPI] (%s_%d) : Loading failed. \n", __FUNCTION__, __LINE__);
			return PP_FALSE;
		}
		else if ( enLoadRet == eSVMMEM_LOAD_RET_SUCCESS_LOAD )
		{
			for ( i = eFIELD_ODD; i <= u8FieldMax; i++ )
			{
				if ( PPAPI_SVM_SetFBLUTAddress(enView, PPAPI_SVMMEM_GetLoadedFBLUTAddress(i), i) != eSUCCESS ) return PP_FALSE;
				if ( PPAPI_SVM_SetLRLUTAddress(enView, PPAPI_SVMMEM_GetLoadedLRLUTAddress(i), i) != eSUCCESS ) return PP_FALSE;
				if ( PPAPI_SVM_SetBCLUTAddress(enView, PPAPI_SVMMEM_GetLoadedBCLUTAddress(i), i) != eSUCCESS ) return PP_FALSE;

				if ( PPAPI_SVMMEM_GetViewDataSize(enView, eSVMMEM_VIEW_DATA_BC_ODD) )
				{
					for ( j = eSVMDRV_BC_ADD_LUT_ALPHA_0; j < eSVMDRV_BC_ADD_LUT_MAX; j++ )
					{
						for ( k = eSVMDRV_BC_ADD_LUT_SUBCORE_0; k < eSVMDRV_BC_ADD_LUT_SUBCORE_MAX; k++ )
						{
							u8AddLutPos = eSVMMEM_VIEW_DATA_A0S0_ODD + (j * 4) + (k * 2) + i;
							if ( PPAPI_SVM_SetBCAdditionalLUT(enView, j, k, PPAPI_SVMMEM_GetLoadedBCAddLUTAddress(j, k, i), PPAPI_SVMMEM_GetViewDataSize(enView, u8AddLutPos), i)  != eSUCCESS ) return PP_FALSE;
						}
					}
				}

				for ( j = eSVMDRV_IMG_NUM_0; j < eSVMDRV_IMG_NUM_MAX; j++ )
				{
					if ( PPAPI_SVM_SetImageAddress(enView, j, PPAPI_SVMMEM_GetLoadedImageAddress(j, i), i) != eSUCCESS ) return PP_FALSE;
					if ( PPAPI_SVM_SetImageRect(enView, j, PPAPI_SVMMEM_GetImageRect(enView, j)) != eSUCCESS ) return PP_FALSE;
				}
			}
		}
	}

	return PP_TRUE;
}

STATIC PP_BOOL PPAPI_SVM_SwitchingStop(PP_BOOL bStop)
{
	if ( gstCtrl.cbViewHandler )
	{
		if ( bStop )
		{
			gstSwitching.enStop = eSVMAPI_SWITCHING_STOP_STATE_START_STOP;
	
			do {
				asm volatile("nop");
			} while ( gstSwitching.enStop != eSVMAPI_SWITCHING_STOP_STATE_NON_STOP );
		}
		else
		{
			return PP_FALSE;
		}
	}

	return PP_TRUE;
}

STATIC void SVM_VsyncPolling(PP_VOID)
{
	if ( OSAL_get_start_os() )
	{
		if ( !bSVM_InIntr )
		{
			gu8SVM_vsynCntForCk = 0;
			while ( !gu8SVM_vsynCntForCk ) asm volatile("nop");
		}
	}
	else
	{
		uint32 u32Reg;

		u32Reg = gpSUBINTC[3]->pend;
		gpSUBINTC[3]->i_ispc = u32Reg;
		
		do
	    {
			u32Reg = gpSUBINTC[3]->pend;
		    if ( u32Reg & 0x40  )
		    {
				gpSUBINTC[3]->i_ispc = u32Reg;
				break;
		    }
	    } while ( 1 );
	}
}

STATIC PP_RESULT_E PPAPI_SVM_SetContinuousViewList(PP_VIEWMODE_E* IN penViewList, PP_U32 IN u32ViewListCount, PP_U32 IN u32RepeatCount, PP_BOOL IN bMorphingView, PP_BOOL IN bStop,
															PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback)
{
	PP_U32 i, k;
	PP_S64 j;
	PP_U32 u32MorphingCnt = 0, u32MorphingCalCnt = 0;
	PP_S16 s16RatioInterval = 0;
	PP_BOOL bCheckEndLUT = PP_TRUE;
	PP_S64 s64View = -1;
	PP_VIEWMODE_E enMorphingStartView = VIEWMODE_NULL;
	PP_VIEWMODE_E enMorphingEndView = VIEWMODE_NULL;
	PP_U32 u32ViewListTempCount = u32ViewListCount;
	PP_S8 s8Step = 1;
	
	if ( !gstCtrl.bInit )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_SVM_NOT_INITIALIZE);
	}
	
	if ( penViewList == PP_NULL || u32ViewListCount == 0 )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_INVALID_ARGUMENT);
	}

#if 0
	for ( i = 0; i < u32ViewListCount; i++ )
	{
		if ( gstView[i] == PP_NULL )
		{
			if ( cbFinishCallback ) cbFinishCallback();
			_RET(eERROR_INVALID_ARGUMENT);
		}
	}
#endif

	if ( !PPAPI_SVM_SwitchingStop(bStop) ) _RET(eERROR_BUSY);

	if ( bMorphingView && gstSwitching.enSwingDirection == eSVMAPI_SWING_DIRECTION_COUNTERCLOCKWISE )
		s8Step = -1;

	//Create switch view
	gstSwitching.u32Cnt = u32ViewListCount;
	gstSwitching.u32CurIndex = 0;
	gstSwitching.u32RepeatCnt = u32RepeatCount;
	gstSwitching.u32RepeatIndex = 0;
	gstSwitching.u32DisplayView = VIEWMODE_NULL;
	gstSwitching.u32LoadingErrCnt = 0;
	gstSwitching.cbDisplay = cbDisplayCallback;
	gstSwitching.cbFinish = cbFinishCallback;
	
	for ( i = 0; i < u32ViewListTempCount; i++ )
	{
		if ( u32ViewListTempCount > u32ViewListCount )
		{
			s64View = penViewList[u32ViewListCount-1] + ((u32ViewListTempCount - u32ViewListCount) * s8Step);
			if ( s64View < 0 || s64View < gstSwitching.u32SwingStartView || s64View > gstSwitching.u32SwingEndView ) break;
		}
		else
		{
			s64View = penViewList[i];
		}
		
		if ( s64View == VIEWMODE_NULL ) break;

		gstSwitching.stInfo[i].u32View = s64View;
		gstSwitching.stInfo[i].u32StartView = VIEWMODE_NULL;
		gstSwitching.stInfo[i].u32EndView = VIEWMODE_NULL;
		gstSwitching.stInfo[i].u8Ratio = 0x00;
		
		if ( PPAPI_SVMMEM_IsData(s64View, eSVMMEM_DATA_SUBTYPE_FB_ODD) && PPAPI_SVMMEM_IsData(s64View, eSVMMEM_DATA_SUBTYPE_LR_ODD) && PPAPI_SVMMEM_IsData(s64View, eSVMMEM_DATA_SUBTYPE_BC_ODD) )
		{
			if ( u32MorphingCalCnt > 0 )
			{
				enMorphingEndView = s64View;
				
				s16RatioInterval = -1 * 0xFF / (u32MorphingCalCnt + 1);

				for ( j = i - 1, k = 1; j >= 0 && j >= i - u32MorphingCnt; j--, k++ )
				{
					gstSwitching.stInfo[j].u32StartView = enMorphingStartView;
					gstSwitching.stInfo[j].u32EndView = enMorphingEndView;
					gstSwitching.stInfo[j].u8Ratio = s16RatioInterval * k;
				}
			}

			u32MorphingCnt = u32MorphingCalCnt = 0;
			enMorphingStartView = s64View;

			bCheckEndLUT = PP_TRUE;
		}
		else if ( !PPAPI_SVMMEM_IsData(s64View, eSVMMEM_DATA_SUBTYPE_FB_ODD) && !PPAPI_SVMMEM_IsData(s64View, eSVMMEM_DATA_SUBTYPE_LR_ODD) && PPAPI_SVMMEM_IsData(s64View, eSVMMEM_DATA_SUBTYPE_BC_ODD) )
		{
			u32MorphingCnt++;
			u32MorphingCalCnt++;
			bCheckEndLUT = PP_FALSE;
					
			if ( bMorphingView )
			{
				if ( i == 0 )
				{
					PP_BOOL bCheckAllLut = PP_FALSE;
					PP_VIEWMODE_E enAllLutView = s64View;
					
					while ( enAllLutView > gstSwitching.u32SwingStartView && enAllLutView < gstSwitching.u32SwingEndView )
					{
						enAllLutView -= s8Step;
						if ( PPAPI_SVMMEM_IsData(enAllLutView, eSVMMEM_DATA_SUBTYPE_FB_ODD) && PPAPI_SVMMEM_IsData(enAllLutView, eSVMMEM_DATA_SUBTYPE_LR_ODD) && PPAPI_SVMMEM_IsData(enAllLutView, eSVMMEM_DATA_SUBTYPE_BC_ODD) )
						{
							enMorphingStartView = enAllLutView;
							
							if ( u32ViewListCount == 1 )
							{
								if ( ++u32ViewListTempCount > gstSwitching.u32SwingEndView ) break;
							}

							bCheckAllLut = PP_TRUE;
							break;
						}
						else
						{
							u32MorphingCalCnt++;
						}

						if ( enAllLutView == 0 )
							break;
					}

					if ( !bCheckAllLut )
					{
						LOG_WARNING("[SVMAPI] The first morphing view must have FB&LR&BC LUT.\n", eERROR_SVM_MORPHING);
						if ( cbFinishCallback ) cbFinishCallback();
						_RET (eERROR_SVM_MORPHING);
					}
				}
				else
				{
					if ( i == u32ViewListTempCount - 1 )
					{
						if ( ++u32ViewListTempCount > gstSwitching.u32SwingEndView ) break;
					}
				}
			}
			else
			{
				if ( i == 0 )
				{
					LOG_WARNING("[SVMAPI] The first morphing view must have FB&LR&BC LUT.\n", eERROR_SVM_MORPHING);
					if ( cbFinishCallback ) cbFinishCallback();
					_RET (eERROR_SVM_MORPHING);
				}
			}
		}
	}

	if ( !bCheckEndLUT )
	{
		LOG_WARNING("[SVMAPI] The last morphing view must have FB&LR&BC LUT.\n", eERROR_SVM_MORPHING);
		if ( cbFinishCallback ) cbFinishCallback();
		_RET (eERROR_SVM_MORPHING);
	}

	gstSwitching.u32Cnt -= u32MorphingCnt;

	PPAPI_SVM_SetViewHandler(PPAPI_SVM_ViewSwitching);

	return eSUCCESS;
}


/*======================================================================================================================
 = Export function
======================================================================================================================*/
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Common Control
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_RESULT_E PPAPI_SVM_SetInFrameBufferAddress(PP_CHANNEL_E IN enChannel, PP_U32* IN pu32Addr)
{
	_INITCHECK;
	_MAXCHECK(enChannel, eCHANNEL_MAX);

	if ( pu32Addr ) _RET(eERROR_INVALID_ARGUMENT);

	return PPDRV_SVM_IN_SetAddress(enChannel, pu32Addr);
}

PP_U32*	PPAPI_SVM_GetInFrameBufferAddress(PP_CHANNEL_E IN enChannel)
{
	_INITCHECKNULL;
	_MAXCHECKNULL(enChannel, eCHANNEL_MAX);

	return PPDRV_SVM_IN_GetAddress(enChannel);
}

PP_RESULT_E	PPAPI_SVM_SetOutFrameBufferAddress(PP_SVMMEM_OUT_FRAMEBUF_NUM_E IN enOutFBNum, PP_U32* IN pu32Addr)
{
	_INITCHECK;
	_MAXCHECK(enOutFBNum, eSVMMEM_OUT_FB_NUM_MAX);
	
	if ( pu32Addr ) _RET(eERROR_INVALID_ARGUMENT);

	return PPDRV_SVM_OUT_SetAddress(enOutFBNum, pu32Addr);
}

PP_U32*	PPAPI_SVM_GetOutFrameBufferAddress(PP_SVMMEM_OUT_FRAMEBUF_NUM_E IN enOutFBNum)
{
	_INITCHECKNULL;
	_MAXCHECKNULL(enOutFBNum, eSVMMEM_OUT_FB_NUM_MAX);
	
	return PPDRV_SVM_OUT_GetAddress(enOutFBNum);
}

PP_VOID PPAPI_SVM_SetMirroring(PP_CHANNEL_E IN enChannel, PP_BOOL IN bHorizontal, PP_BOOL IN bVertical)
{
	_MAXCHECKVOID(enChannel, eCHANNEL_MAX);
	PPDRV_SVM_IN_SetMirroring(enChannel, bHorizontal, bVertical);
}

PP_RESULT_E PPAPI_SVM_SetAntiAliasing(PP_CHANNEL_E IN enChannel, PP_SVMDRV_ANTI_ALIASING_STRENGTH_H_E IN enHorizotal, PP_SVMDRV_ANTI_ALIASING_STRENGTH_V_E IN enVertical)
{
	_INITCHECK;
	_MAXCHECK(enHorizotal, eSVMDRV_AA_H_MAX);
	_MAXCHECK(enVertical, eSVMDRV_AA_V_MAX);

	return PPDRV_SVM_IN_SetAntiAliasing(enChannel, enHorizotal, enVertical);
}

PP_VOID PPAPI_SVM_SetReplaceColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr)
{
	PPDRV_SVM_INOUT_SetReplaceColor(u8Y, u8Cb, u8Cr);
}

PP_VOID PPAPI_SVM_SetInputReplaceColorOnOff(PP_CHANNEL_E IN enChannel, PP_BOOL IN bOn)
{
	_MAXCHECKVOID(enChannel, eCHANNEL_MAX);
	PPDRV_SVM_IN_SetReplaceColorEnable(enChannel, bOn);
}

PP_BOOL PPAPI_SVM_GetInputReplaceColorOnOff(PP_CHANNEL_E IN enChannel)
{
	_MAXCHECKFALSE(enChannel, eCHANNEL_MAX);
	return PPDRV_SVM_IN_GetReplaceColorEnable(enChannel);
}

PP_VOID PPAPI_SVM_SetOutputReplaceColorOnOff(PP_BOOL IN bOn)
{
	gstCtrl.bReColorByFrameDelay = PP_FALSE;
	PPDRV_SVM_OUT_SetReplaceColorEnable(bOn);
}

PP_BOOL PPAPI_SVM_GetOutputReplaceColorOnOff(PP_VOID)
{
	return PPDRV_SVM_OUT_GetReplaceColorEnable();
}

PP_VOID PPAPI_SVM_SetBackgroundColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr)
{
	PPDRV_SVM_CTRL_SetBackgroundColor(u8Y, u8Cb, u8Cr);
}

PP_VOID PPAPI_SVM_SetImageMaskColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr)
{
	PPDRV_SVM_CTRL_SetImageMaskColor(eSVMDRV_IMG_NUM_0, u8Y, u8Cb, u8Cr);
	PPDRV_SVM_CTRL_SetImageMaskColor(eSVMDRV_IMG_NUM_1, u8Y, u8Cb, u8Cr);
}

PP_RESULT_E PPAPI_SVM_SetEdgeEnhancement(PP_S16 IN s16Edge)
{
	_INITCHECK;

	if ( s16Edge < 0 )
	{
		PPDRV_SVM_CTRL_SetEdgeEnhancementEnable(PP_FALSE);
	}
	else
	{
		if ( s16Edge > 255 ) s16Edge = 255;
		
		PPDRV_SVM_CTRL_SetEdgeEnhancementFixedGain((PP_U8)s16Edge);
		PPDRV_SVM_CTRL_SetEdgeEnhancementEnable(PP_TRUE);
	}
		
	return eSUCCESS;
}

PP_VOID PPAPI_SVM_SetDynamicBlending(PP_SVMAPI_DYNAMIC_BLENDING_E enPos, PP_U8 u8Ratio)
{
	_INITCHECKVOID;
	
	gstCtrl.u8DynamicBlending[enPos] = u8Ratio;
	PPDRV_SVM_CTRL_SetDynamicblendingCoefficient(gstCtrl.u8DynamicBlending[eSVMAPI_DB_FRONTLEFT], gstCtrl.u8DynamicBlending[eSVMAPI_DB_FRONTRIGHT],
												 gstCtrl.u8DynamicBlending[eSVMAPI_DB_REARLEFT], gstCtrl.u8DynamicBlending[eSVMAPI_DB_REARRIGHT]);
}

PP_VOID PPAPI_SVM_SetDynamicBlendingOnOff(PP_BOOL bOn)
{
	_INITCHECKVOID;
	PPDRV_SVM_CTRL_SetDynamicblendingEnable(bOn);
}

PP_VOID PPAPI_SVM_SetWindowOffset(PP_S8 IN s8X, PP_S8 IN s8Y)
{
	_INITCHECKVOID;
	if ( s8X < -63 ) s8X = -63;
	if ( s8X > 63 ) s8X = 63;
	if ( s8Y < -63 ) s8Y = -63;
	if ( s8Y > 63 ) s8Y = 63;

	SVM_VsyncPolling();
	PPDRV_SVM_OUT_SetTilt(s8X, s8Y);
}

PP_VOID PPAPI_SVM_SetInputHold(PP_BOOL IN bEnable)
{
	PPDRV_SVM_IN_SetEnable(!bEnable, !bEnable, !bEnable, !bEnable);
}

PP_VOID PPAPI_SVM_SetOutputHold(PP_BOOL IN bEnable)
{
	_INITCHECKVOID;
	PPDRV_SVM_OUT_SetHold(bEnable);
	SVM_VsyncPolling();
}

PP_U32* PPAPI_SVM_GetHoldFrameBufferAddress(PP_FIELD_E IN enField)
{
	_INITCHECKNULL;
	_FIELDCHECKNULL;

	return PPDRV_SVM_OUT_GetHoldFrameBufferAddress(enField);
}

PP_VOID	PPAPI_SVM_SetAutoBrightnessControlOnOff(PP_BOOL bOn)
{
	
	if ( bOn && gstCtrl.enAutoBC >= eSVMAPI_AUTOBC_STATE_OFF )
	{
		if ( OSAL_get_start_os() )
		{
			PPAPI_IPC_SetBrightCtrl(PP_TRUE, 0);
			gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_ON;
		}
		else
		{
			gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_APPLY_ON;
		}
	}
	else if ( !bOn && gstCtrl.enAutoBC <= eSVMAPI_AUTOBC_STATE_APPLY_ON )
	{
		if ( OSAL_get_start_os() )
		{
			PPAPI_IPC_SetBrightCtrl(PP_FALSE, 0);
			gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_OFF;
		}
		else
		{
			gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_APPLY_OFF;
		}
	}
}

PP_VOID PPAPI_SVM_SetFrameDelayCtrl(PP_U32 IN u32DelayCnt, PP_SVMAPI_FRAMEDELAY_CTRL_E IN enOuputReplaceColor, PPDRV_SVM_CALLBACK_UNDERFLOW IN callback)
{
	PPDRV_SVM_SetUnderflowCtrlHandler(u32DelayCnt, PPAPI_SVM_Underflow_Handler);
	gstCtrl.cbFrameDelayHandler = callback;
	gstCtrl.enFrameDelayCtrl = enOuputReplaceColor;
}

PP_U32 PPAPI_SVM_GetFrameDelayCtrlContinuousCount(PP_VOID)
{
	return PPDRV_SVM_GetUnderflowCtrlContinuousCount();
}

PP_U32 PPAPI_SVM_GetVersion(PP_VOID)
{
	return PPDRV_SVM_CTRL_GetVersion();
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* ViewMode Control
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_RESULT_E PPAPI_SVM_CraeteView(PP_VIEWMODE_E enView)
{
	_INITCHECK;
	_MAXVIEWCHECK;

	if ( gstView[enView] ) return eSUCCESS;

	gstView[enView] = (PP_SVMAPI_VIEW_S*)OSAL_malloc(sizeof(PP_SVMAPI_VIEW_S));
	if ( !gstView[enView] ) _RET(eERROR_NO_MEM);

	memset(gstView[enView], 0, sizeof(PP_SVMAPI_VIEW_S));
	gstView[enView]->stImg[eSVMDRV_IMG_NUM_0].u8Alpha = 63;
	gstView[enView]->stImg[eSVMDRV_IMG_NUM_1].u8Alpha = 63;
	
	return eSUCCESS;
}

PP_VOID PPAPI_SVM_DeleteView(PP_VIEWMODE_E enView)
{
	_INITCHECKVOID;
	_MAXVIEWCHECKVOID;
	
	OSAL_free(gstView[enView]);
	gstView[enView] = PP_NULL;
}

PP_RESULT_E PPAPI_SVM_CraeteAllView(PP_VOID)
{
	PP_U32 i;
	PP_RESULT_E enRet = eSUCCESS;

	_INITCHECK;

	for ( i = 0; i < eVIEWMODE_LOAD_MAX; i++ )
	{
		PPAPI_SVM_CraeteView(i);
		if ( enRet != eSUCCESS )
		{
			PPAPI_SVM_DeleteAllView();
			break;
		}
	}

	return enRet;
}

PP_VOID PPAPI_SVM_DeleteAllView(PP_VOID)
{
	PP_U32 i;

	_INITCHECKVOID;

	for ( i = 0; i < eVIEWMODE_LOAD_MAX; i++ )
		PPAPI_SVM_DeleteView(i);
}

PP_RESULT_E PPAPI_SVM_SetFBLUTAddress(PP_VIEWMODE_E IN enView, PP_U32* IN pu32Addr, PP_FIELD_E IN enField)
{	
	_INITCHECK;
	_VIEWCHECK;
	_FIELDCHECK;
	_16BYTEALIGNCHECK(pu32Addr);

	gstView[enView]->stLut.pu32FBAddr[enField] = pu32Addr;
	
	return eSUCCESS;
}

PP_U32* PPAPI_SVM_GetFBLUTAddress(PP_VIEWMODE_E IN enView, PP_FIELD_E IN enField)
{
	_INITCHECKNULL;
	_VIEWCHECKNULL;
	_FIELDCHECKNULL;

	return gstView[enView]->stLut.pu32FBAddr[enField];
}

PP_RESULT_E	PPAPI_SVM_SetLRLUTAddress(PP_VIEWMODE_E IN enView, PP_U32* IN pu32Addr, PP_FIELD_E IN enField)
{
	_INITCHECK;
	_VIEWCHECK;
	_FIELDCHECK;
	_16BYTEALIGNCHECK(pu32Addr);

	gstView[enView]->stLut.pu32LRAddr[enField] = pu32Addr;
	
	return eSUCCESS;
}

PP_U32* PPAPI_SVM_GetLRLUTAddress(PP_VIEWMODE_E IN enView, PP_FIELD_E IN enField)
{
	_INITCHECKNULL;
	_VIEWCHECKNULL;
	_FIELDCHECKNULL;
	
	return gstView[enView]->stLut.pu32LRAddr[enField];
}

PP_RESULT_E	PPAPI_SVM_SetBCLUTAddress(PP_VIEWMODE_E IN enView, PP_U32* IN pu32Addr, PP_FIELD_E IN enField)
{
	_INITCHECK;
	_VIEWCHECK;
	_FIELDCHECK;
	_16BYTEALIGNCHECK(pu32Addr);

	gstView[enView]->stLut.pu32BCAddr[enField] = pu32Addr;
	
	return eSUCCESS;
}

PP_U32* PPAPI_SVM_GetBCLUTAddress(PP_VIEWMODE_E IN enView, PP_FIELD_E IN enField)
{
	_INITCHECKNULL;
	_VIEWCHECKNULL;
	_FIELDCHECKNULL;

	return gstView[enView]->stLut.pu32BCAddr[enField];
}

PP_RESULT_E PPAPI_SVM_SetBCAdditionalLUT(PP_VIEWMODE_E IN enView,
												  PP_SVMDRV_BC_ADDITIONAL_LUT_E IN enType, PP_SVMDRV_BC_ADDITIONAL_LUT_SUBCORE_E IN enSubCore,
												  PP_U32* IN pu32Addr, PP_U32 IN u32Size,
												  PP_FIELD_E IN enField)
{
	_INITCHECK;
	_VIEWCHECK;
	_FIELDCHECK;
	_MAXCHECK(enType, eSVMDRV_BC_ADD_LUT_MAX);
	_MAXCHECK(enSubCore, eSVMDRV_BC_ADD_LUT_SUBCORE_MAX);
	_16BYTEALIGNCHECK(pu32Addr);

	if ( (u32Size != 0 && pu32Addr == PP_NULL) ||
		 (u32Size == 0 && pu32Addr != PP_NULL) )
	{
		_RET(eERROR_INVALID_ARGUMENT);
	}
	
	gstView[enView]->stLut.stBCAdd[enType].u16TotalCnt[enField][enSubCore] = u32Size / 64;
	gstView[enView]->stLut.stBCAdd[enType].pu32Addr[enField][enSubCore] = pu32Addr;

	gstView[enView]->stLut.bBCAddEnable[enType] = PP_TRUE;
	
#if (BD_VIN_INTERLACE == 1)
	{
		PP_U8 i;
		
		for ( i = eFIELD_ODD; i < eFIELD_MAX; i++ )
		{
			if ( (gstView[enView]->stLut.stBCAdd[enType].u16TotalCnt[i][eSVMDRV_BC_ADD_LUT_SUBCORE_0] == 0 || gstView[enView]->stLut.stBCAdd[enType].pu32Addr[i][eSVMDRV_BC_ADD_LUT_SUBCORE_0] == PP_NULL) &&
				 (gstView[enView]->stLut.stBCAdd[enType].u16TotalCnt[i][eSVMDRV_BC_ADD_LUT_SUBCORE_1] == 0 || gstView[enView]->stLut.stBCAdd[enType].pu32Addr[i][eSVMDRV_BC_ADD_LUT_SUBCORE_1] == PP_NULL) )
			{
				gstView[enView]->stLut.bBCAddEnable[enType] = PP_FALSE;
			}
		}
	}
#else
	if ( (gstView[enView]->stLut.stBCAdd[enType].u16TotalCnt[eFIELD_NONE][eSVMDRV_BC_ADD_LUT_SUBCORE_0] == 0 || gstView[enView]->stLut.stBCAdd[enType].pu32Addr[eFIELD_NONE][eSVMDRV_BC_ADD_LUT_SUBCORE_0] == PP_NULL) &&
		 (gstView[enView]->stLut.stBCAdd[enType].u16TotalCnt[eFIELD_NONE][eSVMDRV_BC_ADD_LUT_SUBCORE_1] == 0 || gstView[enView]->stLut.stBCAdd[enType].pu32Addr[eFIELD_NONE][eSVMDRV_BC_ADD_LUT_SUBCORE_1] == PP_NULL) )
	{
		gstView[enView]->stLut.bBCAddEnable[enType] = PP_FALSE;
	}
#endif
	
	return eSUCCESS;
}

PP_RESULT_E PPAPI_SVM_SetImageRect(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_RECT_S* IN pstRect)
{
	_INITCHECK;
	_VIEWCHECK;
	_MAXCHECK(enImgNum, eSVMDRV_IMG_NUM_MAX);

	if ( pstRect )
	{
		if ( (pstRect->u16Width == 0) || (pstRect->u16Height == 0 ) ||
			 (pstRect->u16X + pstRect->u16Width > gstCtrl.stOutSize.u16Width) || (pstRect->u16Y + pstRect->u16Height > gstCtrl.stOutSize.u16Height) )
		{
			gstView[enView]->stImg[enImgNum].stRect.u16Width = 0;
		}
		else
		{
			memcpy(&gstView[enView]->stImg[enImgNum].stRect, pstRect, sizeof(PP_RECT_S));
			
			if ( gstCtrl.bAutoSectionRect )
			{
				PP_U32 i;
				PP_U32 u32ViewSectionCnt;
				PP_U32 u32SectionIndex;
				PP_U8 u8ImageCnt = 0;
				PP_RECT_S *pstSectionRect;
				
				u32ViewSectionCnt = PPAPI_SVMMEM_GetViewSectionCount(enView);
				for ( i = 0; i < u32ViewSectionCnt; i++ )
				{
					u32SectionIndex = PPAPI_SVMMEM_GetViewSectionIndex(enView, i);
					if ( u32SectionIndex >= 0 && PPAPI_SVMMEM_GetSectionDataSize(u32SectionIndex, eSVMMEM_SECTION_DATA_IMG_ODD) )
					{
						if ( enImgNum == eSVMDRV_IMG_NUM_1 && !u8ImageCnt )
						{
							u8ImageCnt++;
							continue;
						}

						pstSectionRect = PPAPI_SVMMEM_GetViewSectionRect(enView, i);
						if ( pstSectionRect )
						{
							if ( pstSectionRect->u16X + pstSectionRect->u16Width != gstCtrl.stOutSize.u16Width &&
								 pstRect->u16X + pstRect->u16Width == pstSectionRect->u16X + pstSectionRect->u16Width )
							{
								gstView[enView]->stImg[enImgNum].stRect.u16X -= SVMAPI_AUTOSECTION_SIZE;
							}
							
							if ( pstSectionRect->u16Y + pstSectionRect->u16Height != gstCtrl.stOutSize.u16Height &&
								 pstRect->u16Y + pstRect->u16Height == pstSectionRect->u16Y + pstSectionRect->u16Height )
							{
								gstView[enView]->stImg[enImgNum].stRect.u16Y -= SVMAPI_AUTOSECTION_SIZE;
							}
						}
						break;
					}
				}
			}
		}
	}
	else
	{
		gstView[enView]->stImg[enImgNum].stRect.u16Width = 0;
	}

	return eSUCCESS;
}

PP_RESULT_E PPAPI_SVM_SetImageAlphaBlending(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U8 IN u8Alpha)
{
	_INITCHECK;
	_VIEWCHECK;
	_MAXCHECK(enImgNum, eSVMDRV_IMG_NUM_MAX);

	gstView[enView]->stImg[enImgNum].u8Alpha = u8Alpha;
		
	return eSUCCESS;
}

PP_RESULT_E	PPAPI_SVM_SetImageAddress(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U32* IN pu32Addr, PP_FIELD_E IN enField)
{
	_INITCHECK;
	_VIEWCHECK;
	_FIELDCHECK;
	_16BYTEALIGNCHECK(pu32Addr);

	gstView[enView]->stImg[enImgNum].pu32Addr[enField] = pu32Addr;
	
	return eSUCCESS;
}

PP_U32* PPAPI_SVM_GetImageAddress(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_FIELD_E IN enField)
{
	_INITCHECKNULL;
	_VIEWCHECKNULL;
	_FIELDCHECKNULL;

	return gstView[enView]->stImg[enImgNum].pu32Addr[enField];
}

PP_RESULT_E PPAPI_SVM_SetSectionRect(PP_VIEWMODE_E IN enView, PP_SVMDRV_SECTION_NUMBER_E IN enNum, PP_RECT_S* IN pstRect)
{
	_INITCHECK;
	_VIEWCHECK;
	_MAXCHECK(enNum, eSVMDRV_SECTION_NUM_MAX);

	if ( pstRect )
	{
		if ( (pstRect->u16Width == 0) || (pstRect->u16Height == 0 ) ||
			 (pstRect->u16X + pstRect->u16Width > gstCtrl.stOutSize.u16Width) || (pstRect->u16Y + pstRect->u16Height > gstCtrl.stOutSize.u16Height) )
		{
			_RET(eERROR_INVALID_ARGUMENT);
		}
		
		memcpy(&gstView[enView]->stSectionRect[enNum], pstRect, sizeof(PP_RECT_S));
	}
	else
	{
		gstView[enView]->stSectionRect[enNum].u16Width = 0;
	}
	
	return eSUCCESS;
}

PP_RESULT_E	PPAPI_SVM_SetAutoSectionRect(PP_BOOL bOn)
{
	PP_U32 i, j;
	PP_U32 u32ViewSectionCnt;
	PP_RECT_S *pstSectionRect;
	
	_INITCHECK;

	for ( i = 0; i < eVIEWMODE_LOAD_MAX; i++ )
	{
		if ( bOn )
		{
			u32ViewSectionCnt = PPAPI_SVMMEM_GetViewSectionCount(i);
			if ( u32ViewSectionCnt > eSVMDRV_SECTION_NUM_MAX )
				u32ViewSectionCnt = eSVMDRV_SECTION_NUM_MAX;
			for ( j = eSVMDRV_SECTION_NUM_0; j < eSVMDRV_SECTION_NUM_MAX; j++ )
			{
				if ( j < u32ViewSectionCnt )
				{
					pstSectionRect = PPAPI_SVMMEM_GetViewSectionRect(i, j);
					if ( pstSectionRect )
					{
						if ( pstSectionRect->u16X + pstSectionRect->u16Width != gstCtrl.stOutSize.u16Width )
							pstSectionRect->u16Width -= SVMAPI_AUTOSECTION_SIZE;
						if ( pstSectionRect->u16Y + pstSectionRect->u16Height != gstCtrl.stOutSize.u16Height )
							pstSectionRect->u16Height -= SVMAPI_AUTOSECTION_SIZE;
					}

					PPAPI_SVM_SetSectionRect(i, j, pstSectionRect);
				}
				else
				{
					gstView[i]->stSectionRect[j].u16Width = 0;
				}
			}
		}
		else
		{
			for ( j = 0; j < eSVMDRV_SECTION_NUM_MAX; j++ )
				gstView[i]->stSectionRect[j].u16Width = 0;
		}
	}

	gstCtrl.bAutoSectionRect = bOn;

	return eSUCCESS;
}

PP_VOID PPAPI_SVM_SetMorphingRatio(PP_U8 IN u8Ratio)
{
	PPDRV_SVM_CTRL_SetMorphingRatio(u8Ratio, u8Ratio);
}

PP_RESULT_E PPAPI_SVM_SetView(PP_VIEWMODE_E IN enView, PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback)
{
	STATIC PP_BOOL bFirst = PP_TRUE;
	
	if ( eVIEWMODE_LOAD_TOP2D_SWING_START != VIEWMODE_NULL && eVIEWMODE_LOAD_TOP2D_SWING_MAX != VIEWMODE_NULL &&
		 eVIEWMODE_LOAD_TOP2D_SWING_START <= enView && enView <= eVIEWMODE_LOAD_TOP2D_SWING_MAX )
	{
		return PPAPI_SVM_SetSwingView(enView, VIEWMODE_NULL, eSVMAPI_SWING_DIRECTION_AUTO, 1, cbDisplayCallback, cbFinishCallback);
	}
	else
	{
		if ( PPAPI_SVM_SetAddress(enView, PP_TRUE) )
		{
			gstSwitching.u32SwingStartView = VIEWMODE_NULL;
			gstSwitching.u32SwingEndView = VIEWMODE_NULL;
		
			if ( OSAL_get_start_os() )
			{
				return PPAPI_SVM_SetContinuousViewList(&enView, 1, 1, PP_FALSE, PP_TRUE, cbDisplayCallback, cbFinishCallback);
			}
			else
			{
				gstSwitching.u32Cnt = 1;
				PPAPI_SVM_ViewSwitching(VIEWMODE_NULL, VIEWMODE_NULL, enView, 0x00);
				//SVM_VsyncPolling();
				if ( cbDisplayCallback ) cbDisplayCallback(enView);
				if ( cbFinishCallback ) cbFinishCallback();

				if ( bFirst )
				{
					PPDRV_SVM_OUT_SetReplaceColorEnable(PP_FALSE);
					bFirst = PP_FALSE;
				}
				
				return eSUCCESS;
			}
		}
	}

	if ( cbFinishCallback ) cbFinishCallback();
	
	_RET(eERROR_SVM_NOT_LOADING);
}

#ifdef SVM_TOGGLE_VIEW_USE
PP_RESULT_E PPAPI_SVM_SetToggleView(PP_VIEWMODE_E IN enView0, PP_VIEWMODE_E IN enView1, PP_U32 IN u32RepeatCount,
											PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback)
{
	PP_VIEWMODE_E enViewModeList[2] = {enView0, enView1};
	PP_U32 u32ViewModeListCnt = 2;
	
	if ( !gstCtrl.bInit || !OSAL_get_start_os() )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_SVM_NOT_INITIALIZE);
	}
	
	if ( enView0 == VIEWMODE_NULL || enView1 == VIEWMODE_NULL || gstView[enView0] == PP_NULL || gstView[enView1] == PP_NULL )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_INVALID_ARGUMENT);
	}

	if ( !PPAPI_SVMMEM_CheckCacheView(enView0) )
	{
		if ( !PPAPI_SVM_SetAddress(enView0, PP_TRUE) )
		{
			if ( cbFinishCallback ) cbFinishCallback();
			_RET(eERROR_SVM_NOT_LOADING);
		}
	}

	if ( !PPAPI_SVMMEM_CheckCacheView(enView1) )
	{
		if ( !PPAPI_SVM_SetAddress(enView1, PP_TRUE) )
		{
			if ( cbFinishCallback ) cbFinishCallback();
			_RET(eERROR_SVM_NOT_LOADING);
		}
	}

	gstSwitching.u32SwingStartView = VIEWMODE_NULL;
	gstSwitching.u32SwingEndView = VIEWMODE_NULL;

	return PPAPI_SVM_SetContinuousViewList(enViewModeList, u32ViewModeListCnt, u32RepeatCount, PP_FALSE, PP_TRUE, cbDisplayCallback, cbFinishCallback);
}
#endif

PP_RESULT_E PPAPI_SVM_SetContinuousView(PP_VIEWMODE_E IN enStartView, PP_VIEWMODE_E IN enEndView, PP_U32 IN u32RepeatCount,
												 PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback)
{
	PP_RESULT_E enRet = eSUCCESS;
	PP_U32 i, m;
	PP_S8 s8Step = 1;
	PP_VIEWMODE_E *enViewModeList = PP_NULL;
	PP_U32 u32ViewModeListCnt = 0;
	PP_U32 u32TempStartView;

	if ( !gstCtrl.bInit || !OSAL_get_start_os() )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_SVM_NOT_INITIALIZE);
	}
	
	if ( enStartView == VIEWMODE_NULL && enEndView == VIEWMODE_NULL )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_INVALID_ARGUMENT);
	}

	if ( enStartView == enEndView )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		return eSUCCESS;
	}

	if ( enStartView == VIEWMODE_NULL || enEndView == VIEWMODE_NULL )
	{
		u32ViewModeListCnt = 1;
		if ( enStartView == VIEWMODE_NULL )
			u32TempStartView = enEndView;
		else
			u32TempStartView = enStartView;
	}
	else if ( enStartView > enEndView )
	{
		s8Step = -1;
		u32ViewModeListCnt = enStartView - enEndView + 1;
		u32TempStartView = enEndView;
	}
	else
	{
		u32ViewModeListCnt = enEndView - enStartView + 1;
		u32TempStartView = enStartView;
	}

	enViewModeList = (PP_VIEWMODE_E*)OSAL_malloc(sizeof(PP_VIEWMODE_E) * u32ViewModeListCnt);
	if ( !enViewModeList )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_NO_MEM);
	}

	for ( i = 0, m = u32TempStartView; i < u32ViewModeListCnt; i++, m+=s8Step )
	{
		enViewModeList[i] = m;
	}

	gstSwitching.u32SwingStartView = VIEWMODE_NULL;
	gstSwitching.u32SwingEndView = VIEWMODE_NULL;

	enRet = PPAPI_SVM_SetContinuousViewList(enViewModeList, u32ViewModeListCnt, u32RepeatCount, PP_FALSE, PP_TRUE, cbDisplayCallback, cbFinishCallback);
	OSAL_free(enViewModeList);
	
	return enRet;
}




PP_RESULT_E PPAPI_SVM_SetSwingView(PP_VIEWMODE_E IN enDegreeStartView, PP_VIEWMODE_E IN enDegreeEndView, PP_SVMAPI_SWING_DIRECTION_E IN enDirection, PP_U32 IN u32RepeatCount,
											PPAPI_SVM_SWITCHING_CALLBACK_DISPLAY IN cbDisplayCallback, PPAPI_SVM_SWITCHING_CALLBACK_FINISH IN cbFinishCallback)
{
	PP_RESULT_E enRet = eSUCCESS;
	PP_U32 i, m;
	PP_VIEWMODE_E enBigDegreeView, enSmallDegreeView;
	PP_VIEWMODE_E u32TempStartView, u32TempEndView;
	PP_VIEWMODE_E *enViewModeList = PP_NULL;
	PP_U32 u32ViewModeListCnt = 0;
	PP_SVMAPI_SWING_DIRECTION_E enSwingDirection = enDirection;
	PP_S8 s8Step = 1;
	PP_BOOL bStop = PP_FALSE;

	PP_VIEWMODE_E enSwingMintView = eVIEWMODE_LOAD_TOP2D_SWING_START;
	PP_VIEWMODE_E enSwingMaxView = eVIEWMODE_LOAD_TOP2D_SWING_MAX;

	if ( !gstCtrl.bInit || !OSAL_get_start_os() )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_SVM_NOT_INITIALIZE);
	}

	if ( enSwingMintView == VIEWMODE_NULL || enSwingMaxView == VIEWMODE_NULL || enSwingMintView >= enSwingMaxView )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_NOT_SUPPORT);
	}

	if ( enDegreeStartView == VIEWMODE_NULL && enDegreeEndView == VIEWMODE_NULL )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_INVALID_ARGUMENT);
	}
	else if ( enDegreeStartView == enDegreeEndView )
	{
		enDegreeEndView = VIEWMODE_NULL;
	}

	if ( (enDegreeStartView != VIEWMODE_NULL && (enDegreeStartView > enSwingMaxView || enDegreeStartView < enSwingMintView)) || (enDegreeEndView != VIEWMODE_NULL && (enDegreeEndView > enSwingMaxView || enDegreeEndView < enSwingMintView)) )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_INVALID_ARGUMENT);
	}

	if ( gstSwitching.u32SwingPauseView != VIEWMODE_NULL &&
		 gstSwitching.u32SwingStartView == enSwingMintView &&
		 gstSwitching.u32SwingEndView == enSwingMaxView )
	{
		enDegreeStartView = gstSwitching.u32SwingPauseView;
	}

	if ( gstSwitching.u32SwingStartView == VIEWMODE_NULL && gstSwitching.u32SwingEndView == VIEWMODE_NULL ) bStop = PP_TRUE;

	gstSwitching.u32SwingStartView = enSwingMintView;
	gstSwitching.u32SwingEndView = enSwingMaxView;

	u32TempStartView = enDegreeStartView;
	u32TempEndView = enDegreeEndView;

	if ( enDegreeStartView > enDegreeEndView )
	{
		enBigDegreeView = enDegreeStartView;
		enSmallDegreeView = enDegreeEndView;
	}
	else
	{
		enBigDegreeView = enDegreeEndView;
		enSmallDegreeView = enDegreeStartView;
	}

	if ( enDegreeStartView != VIEWMODE_NULL && enDegreeEndView != VIEWMODE_NULL )
	{
		if ( enSwingDirection == eSVMAPI_SWING_DIRECTION_AUTO )
		{
			if ( (enSwingMaxView - enBigDegreeView + enSmallDegreeView) < (enBigDegreeView - enSmallDegreeView) )
			{
				if ( enDegreeStartView > enDegreeEndView )
					enSwingDirection = eSVMAPI_SWING_DIRECTION_CLOCKWISE;
				else
					enSwingDirection = eSVMAPI_SWING_DIRECTION_COUNTERCLOCKWISE;
			}
			else
			{
				if ( enDegreeStartView > enDegreeEndView )
					enSwingDirection = eSVMAPI_SWING_DIRECTION_COUNTERCLOCKWISE;
				else
					enSwingDirection = eSVMAPI_SWING_DIRECTION_CLOCKWISE;
			}
		}

		if ( enSwingDirection == eSVMAPI_SWING_DIRECTION_CLOCKWISE )
		{
			if ( enDegreeStartView > enDegreeEndView )
			{
				u32TempEndView = enDegreeEndView + (enSwingMaxView - enSwingMintView);
				enBigDegreeView = u32TempEndView;
				enSmallDegreeView = u32TempStartView;
			}
		}
		else
		{
			if ( enDegreeStartView < enDegreeEndView )
			{
				u32TempStartView = enDegreeStartView + (enSwingMaxView - enSwingMintView);
				enBigDegreeView = u32TempStartView;
				enSmallDegreeView = u32TempEndView;
			}
		}

		if ( enSwingDirection == eSVMAPI_SWING_DIRECTION_COUNTERCLOCKWISE ) s8Step = -1;
		gstSwitching.enSwingDirection = enSwingDirection;
		//u32ViewModeListCnt = enBigDegreeView - enSmallDegreeView + 1;
		u32ViewModeListCnt = 1;
	}
	else
	{
		u32ViewModeListCnt = 1;
		if ( enDegreeStartView != VIEWMODE_NULL )
			u32TempStartView = enDegreeStartView;
		else
			u32TempStartView = enDegreeEndView;
	}

	enViewModeList = (PP_VIEWMODE_E*)OSAL_malloc(sizeof(PP_VIEWMODE_E) * u32ViewModeListCnt);
	if ( !enViewModeList )
	{
		if ( cbFinishCallback ) cbFinishCallback();
		_RET(eERROR_NO_MEM);
	}

    if(u32TempStartView != enDegreeEndView )
    	m = enDegreeStartView ;
    else
    	m = enDegreeEndView;

	for ( i = 0; i < u32ViewModeListCnt; i++ )
	{
		if ( m < enSwingMintView )
			enViewModeList[i] = enSwingMaxView - (enSwingMintView - m);
		else if ( m > enSwingMaxView )
			enViewModeList[i] = enSwingMintView + (m - enSwingMaxView);
		else
			enViewModeList[i] = m;
	}

	enRet = PPAPI_SVM_SetContinuousViewList(enViewModeList, u32ViewModeListCnt, u32RepeatCount, PP_TRUE, bStop, cbDisplayCallback, cbFinishCallback);
	OSAL_free(enViewModeList);

	return enRet;
}

PP_BOOL PPAPI_SVM_SetCacheViewAddress(PP_VIEWMODE_E enView)
{
	PP_U32 i, j, k;
	PP_SVMMEM_DATA_SUBTYPE_E u8AddLutPos;
#if (BD_VIN_INTERLACE == 1)
	PP_U8 u8FieldMax = eFIELD_EVEN;
#else
	PP_U8 u8FieldMax = eFIELD_NONE;
#endif

	if ( PPAPI_SVMMEM_CheckCacheView(enView) )
	{	
		for ( i = eFIELD_ODD; i <= u8FieldMax; i++ )
		{
			if ( PPAPI_SVM_SetFBLUTAddress(enView, PPAPI_SVMMEM_GetCacheViewLoadedFBLUTAddress(i), i) != eSUCCESS ) return PP_FALSE;
			if ( PPAPI_SVM_SetLRLUTAddress(enView, PPAPI_SVMMEM_GetCacheViewLoadedLRLUTAddress(i), i) != eSUCCESS ) return PP_FALSE;
			if ( PPAPI_SVM_SetBCLUTAddress(enView, PPAPI_SVMMEM_GetCacheViewLoadedBCLUTAddress(i), i) != eSUCCESS ) return PP_FALSE;

			if ( PPAPI_SVMMEM_GetViewDataSize(enView, eSVMMEM_VIEW_DATA_BC_ODD) )
			{
				for ( j = eSVMDRV_BC_ADD_LUT_ALPHA_0; j < eSVMDRV_BC_ADD_LUT_MAX; j++ )
				{
					for ( k = eSVMDRV_BC_ADD_LUT_SUBCORE_0; k < eSVMDRV_BC_ADD_LUT_SUBCORE_MAX; k++ )
					{
						u8AddLutPos = eSVMMEM_VIEW_DATA_A0S0_ODD + (j * 4) + (k * 2) + i;
						if ( PPAPI_SVM_SetBCAdditionalLUT(enView, j, k, PPAPI_SVMMEM_GetCacheViewLoadedBCAddLUTAddress(j, k, i), PPAPI_SVMMEM_GetViewDataSize(enView, u8AddLutPos), i)  != eSUCCESS ) return PP_FALSE;
					}
				}
			}

			for ( j = eSVMDRV_IMG_NUM_0; j < eSVMDRV_IMG_NUM_MAX; j++ )
			{
				if ( PPAPI_SVM_SetImageAddress(enView, j, PPAPI_SVMMEM_GetCacheViewLoadedImageAddress(j, i), i) != eSUCCESS ) return PP_FALSE;
				if ( PPAPI_SVM_SetImageRect(enView, j, PPAPI_SVMMEM_GetImageRect(enView, j)) != eSUCCESS ) return PP_FALSE;
			}
		}

		return PP_TRUE;
	} 

	return PP_FALSE;
} 

PP_U32 PPAPI_SVM_GetLoadedViewCount(PP_VOID)
{
	_INITCHECK0;
	return PPAPI_SVMMEM_GetViewCount();
}

PP_VIEWMODE_E PPAPI_SVM_GetCurrentView(PP_VOID)
{
	return gstCtrl.u32CurView;
}


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
* Initiaize
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
PP_RESULT_E PPAPI_SVM_Initialize(PP_VOID)
{
	PP_U32 i;
	PP_S32 s32VinWidth, s32VinHeight, s32VoutWidth, s32VoutHeight;
	_VID_RESOL enVinResol, enVoutResol;

	if ( gstCtrl.bInit ) return eSUCCESS;

	PPAPI_VIN_GetResol(BD_SVM_IN_FMT, &s32VinWidth, &s32VinHeight, &enVinResol);
	PPAPI_VIN_GetResol(BD_SVM_OUT_FMT, &s32VoutWidth, &s32VoutHeight, &enVoutResol);

	if ( PPDRV_SVM_Initialize(enVinResol, enVoutResol, SVMMEM_IN_BUFFER_COUNT, SVMMEM_OUT_BUFFER_COUNT) != eSUCCESS ) _RET(eERROR_SVM_NOT_INITIALIZE);

	for ( i = eCHANNEL_FRONT; i < eCHANNEL_MAX; i++ )
		PPDRV_SVM_IN_SetAddress(i, PPAPI_SVMMEM_GetInputFrameBufferAddr(i));

	for ( i = eSVMMEM_OUT_FB_NUM_0; i < eSVMMEM_OUT_FB_NUM_MAX; i++ )
		PPDRV_SVM_OUT_SetAddress(i, PPAPI_SVMMEM_GetOutputFrameBufferAddr(i));
	
	PPAPI_SVM_SetImageMaskColor(0x69, 0xCA, 0xFF);
	PPAPI_SVM_SetBackgroundColor(0x0, 0x80, 0x80);
	PPDRV_SVM_ISR_SetVsyncHandler(PPAPI_SVM_Vsync_Handler);

	memset(&gstCtrl, 0, sizeof(PP_SVMAPI_CTRL_S));
	
	gstCtrl.u32CurView = VIEWMODE_NULL;
	
	gstCtrl.stOutSize.u16Width = (PP_U16)s32VoutWidth;
	gstCtrl.stOutSize.u16Height = (PP_U16)s32VoutHeight;
	
	for ( i = eSVMAPI_DB_FRONTLEFT; i < eSVMAPI_DB_MAX; i++ )
		gstCtrl.u8DynamicBlending[i] = 0xFF;

	gstSwitching.xTimer = xTimerCreate("Display Timer", OSAL_ms2ticks(SVMAPI_DISPLAY_TIMER), PP_FALSE, (void*)NULL, PPAPI_SVM_DisplayTimerHandler);
	configASSERT(gstSwitching.xTimer);

	gstCtrl.enAutoBC = eSVMAPI_AUTOBC_STATE_OFF;
	PPAPI_SVM_SetAutoBrightnessControlOnOff(PP_TRUE);
	gstCtrl.u8ExcuteVsync = 0;

	gstCtrl.bInit = PP_TRUE;

	if ( PPAPI_SVM_GetLoadedViewCount() )
		PPDRV_SVM_OUT_SetReplaceColorEnable(PP_TRUE);

	if ( PPAPI_SVM_CraeteDefaultView() != eSUCCESS )
	{
		LOG_CRITICAL("[SVMAPI] View mode Creation failed\n");
		gstCtrl.bInit = PP_FALSE;
		_RET(eERROR_SVM_NOT_INITIALIZE);
	}

	if ( PPAPI_SVM_CraeteAllView() != eSUCCESS )
	{
		LOG_CRITICAL("[SVMAPI] View mode Creation failed\n");
		gstCtrl.bInit = PP_FALSE;
		_RET(eERROR_SVM_NOT_INITIALIZE);
	}

	gstSwitching.u32Cnt = 1;
	PPAPI_SVM_ViewSwitching(VIEWMODE_NULL, VIEWMODE_NULL, eVIEWMODE_BASIC_FRONT_BYPASS, 0x00);

	if ( !PPAPI_SVMMEM_LoadCommonSectionData(-1, PP_FALSE) ) _RET(eERROR_SVM_NOT_INITIALIZE);
	PPAPI_SVM_SetAutoSectionRect(PP_TRUE);
	
	return eSUCCESS;
}
