/**
 * \file
 *
 * \brief	Cache task
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#include "task_cache.h"

#ifdef CACHE_VIEW_USE
#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"
#include "api_flash.h"
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
#include "api_FAT_FTL.h"
#include "sys_api.h"
#endif
#include "api_svm_mem.h"
#include "api_svm.h"
#include "api_display.h"


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppCACHETASK_STATUS_E
{
	eCACHETASK_STATUS_NULL = 0,
	eCACHETASK_STATUS_RUN,
	eCACHETASK_STATUS_SUSPENDING,
	eCACHETASK_STATUS_SUSPEND,
	eCACHETASK_STATUS_STOP,
	eCACHETASK_STATUS_DONE,
	eCACHETASK_STATUS_MAX,
} PP_CACHETASK_STATUS_E;

typedef enum ppCACHETASK_RUN_STATUS_E
{
	eCACHETASK_RUN_STATUS_SVMDATA = 0,
	eCACHETASK_RUN_STATUS_UIDATA,
	eCACHETASK_RUN_STATUS_BOTH,
	eCACHETASK_RUN_STATUS_MAX,
} PP_CACHETASK_RUN_STATUS_E;

typedef enum ppCACHETASK_STOP_STATUS_E
{
	eCACHETASK_STOP_STATUS_NOMAL = 0,
	eCACHETASK_STOP_STATUS_RELOAD,
	eCACHETASK_STOP_STATUS_MAX,
} PP_CACHETASK_STOP_STATUS_E;


/***************************************************************************************************************************************************************
 * Global Variable
***************************************************************************************************************************************************************/
STATIC PP_S32 gs32Cache_StartView = -1;
STATIC PP_S32 gs32Cache_EndView = -1;
STATIC PP_S32 gs32Cache_CurView = -1;
STATIC PP_CACHETASK_STATUS_E genCache_Status = eCACHETASK_STATUS_NULL;
STATIC PP_CACHETASK_RUN_STATUS_E genCache_RunStatus = eCACHETASK_RUN_STATUS_BOTH;
STATIC PP_CACHETASK_STOP_STATUS_E genCache_StopStatus = eCACHETASK_STOP_STATUS_NOMAL;


/***************************************************************************************************************************************************************
 * Local function
***************************************************************************************************************************************************************/
STATIC PP_VOID PPAPP_CACHE_LoadData_SVM(PP_S32 s32View)
{
	if ( PPAPI_SVMMEM_LoadData(s32View, PP_TRUE) == eSVMMEM_LOAD_RET_SUCCESS_LOAD )
		PPAPI_SVM_SetCacheViewAddress(s32View);
}

STATIC PP_VOID PPAPP_CACHE_LoadData_Display(PP_S32 s32View)
{
#if defined(USE_PP_GUI)
	PP_U32 u32FlashAddr = 0;
	PP_U32 u32DramAddr = 0;
	PP_U32 u32Size = 0;

	PPAPI_DISPLAY_LoadCacheAddr(s32View-INDEX_OFFSET, &u32FlashAddr, &u32DramAddr, &u32Size);
	if(u32DramAddr != 0)
	{
		PPAPI_FLASH_ReadQDMA((PP_U8 *)u32DramAddr, u32FlashAddr, u32Size);
	}

#if defined(USE_SEPERATE_SHADOW)
	PPAPI_DISPLAY_LoadCacheAddr_Shadow(s32View-SHADOW_INDEX_OFFSET, &u32FlashAddr, &u32DramAddr, &u32Size);
	if(u32DramAddr != 0)
	{
		PPAPI_FLASH_ReadQDMA((PP_U8 *)u32DramAddr, u32FlashAddr, u32Size);
	}
#endif
#endif
}

STATIC PP_BOOL PPAPP_CACHE_LoadView(PP_S32 s32StartView, PP_S32 s32EndView)
{
	PP_S32 i = s32StartView;
	
	for (;;)
	{
		if ( genCache_Status == eCACHETASK_STATUS_STOP )
		{
			break;
		}
		else if ( genCache_Status == eCACHETASK_STATUS_RUN )
		{
			if ( genCache_RunStatus == eCACHETASK_RUN_STATUS_BOTH || genCache_RunStatus == eCACHETASK_RUN_STATUS_UIDATA )
				PPAPP_CACHE_LoadData_Display(i);

			if ( genCache_RunStatus == eCACHETASK_RUN_STATUS_BOTH || genCache_RunStatus == eCACHETASK_RUN_STATUS_SVMDATA )
				PPAPP_CACHE_LoadData_SVM(i);
			
			gs32Cache_CurView = i;
			if ( ++i > s32EndView ) break;
			
			if ( genCache_Status == eCACHETASK_STATUS_SUSPENDING )
				genCache_Status = eCACHETASK_STATUS_SUSPEND;
		}
	}
	
	return PP_TRUE;
}

STATIC PP_RESULT_E PPAPP_CACHE_LoadImage (PP_U32 startImage, PP_U32 endImage, PP_RSC_MODE_E mode)
{
	PP_U32 i;

	if(mode >= eRSC_MODE_MAX)
		return eERROR_INVALID_ARGUMENT;

	i = startImage;
	for (;;)
	{
		if ( genCache_Status == eCACHETASK_STATUS_STOP )
		{
			break;
		}
		else if ( genCache_Status == eCACHETASK_STATUS_RUN )
		{
			if (mode == eRSC_MODE_CAR)
				PPAPI_DISPLAY_LoadCarImage(i);
			else if(mode == eRSC_MODE_PGL)
				PPAPI_DISPLAY_LoadPglImage(i);
			else if(mode == eRSC_MODE_UI)
				PPAPI_DISPLAY_LoadUiImage(i);
			//LOG_DEBUG("[CACHE TASK] (mode %d) Load Image %d\n", mode, i);
			
			if ( ++i > endImage ) break;
			
			if ( genCache_Status == eCACHETASK_STATUS_SUSPENDING )
				genCache_Status = eCACHETASK_STATUS_SUSPEND;
		}
	}
	
	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * Export function
***************************************************************************************************************************************************************/
PP_BOOL PPAPP_CACHE_LoadData(PP_S32 s32StartView, PP_S32 s32EndView, PP_S32 s32BlockingEndView)
{
	if ( (s32StartView < 0 || s32EndView < 0) || (s32StartView > s32EndView) ||
		 (s32BlockingEndView >= 0 && (s32BlockingEndView < s32StartView || s32BlockingEndView > s32EndView)) )
	{
		LOG_DEBUG("[%s] Invalid argument\n", psTaskName[TASK_CACHE]);
		return PP_FALSE;
	}

	gs32Cache_StartView = s32StartView;
	gs32Cache_EndView = s32EndView;
	gs32Cache_CurView  = s32StartView - 1;

	if ( s32BlockingEndView >= 0 )
	{
		genCache_Status = eCACHETASK_STATUS_RUN;
		PPAPP_CACHE_LoadView(s32StartView, s32BlockingEndView);
	}
	
	return PP_TRUE;
}

PP_BOOL PPAPP_CACHE_Reload(PP_BOOL bLoadSVMData, PP_BOOL bLoadUIData)
{
	if ( (gs32Cache_StartView < 0 || gs32Cache_EndView < 0) || (gs32Cache_StartView > gs32Cache_EndView) )
	{
		LOG_DEBUG("[%s] There is no cache task data.\n", psTaskName[TASK_CACHE]);
		return PP_FALSE;
	}

	if ( !OSAL_get_start_os() )
	{
		LOG_DEBUG("[%s] OS did not start.\n", psTaskName[TASK_CACHE]);
	}

	genCache_StopStatus = eCACHETASK_STOP_STATUS_RELOAD;
	PPAPP_CACHE_Stop();

	if ( bLoadSVMData && bLoadUIData )	genCache_RunStatus = eCACHETASK_RUN_STATUS_BOTH;
	else if ( bLoadSVMData )			genCache_RunStatus = eCACHETASK_RUN_STATUS_SVMDATA;
	else if ( bLoadUIData )				genCache_RunStatus = eCACHETASK_RUN_STATUS_UIDATA;
	else 								return PP_FALSE;

	gs32Cache_CurView  = gs32Cache_StartView - 1;
	genCache_Status = eCACHETASK_STATUS_RUN;

	return PP_TRUE;
}

PP_VOID PPAPP_CACHE_Suspend(PP_VOID)
{
	if ( OSAL_get_start_os() && genCache_Status == eCACHETASK_STATUS_RUN )
	{
		genCache_Status = eCACHETASK_STATUS_SUSPENDING;
		do {
			OSAL_sleep(1);
		} while ( !(genCache_Status == eCACHETASK_STATUS_SUSPEND || genCache_Status == eCACHETASK_STATUS_DONE) );
	}
}

PP_VOID PPAPP_CACHE_Resume(PP_VOID)
{
	if ( genCache_Status == eCACHETASK_STATUS_SUSPEND )
		genCache_Status = eCACHETASK_STATUS_RUN;
}

PP_VOID PPAPP_CACHE_Stop(PP_VOID)
{
	PPAPP_CACHE_Suspend();
	
	if ( OSAL_get_start_os() && genCache_Status == eCACHETASK_STATUS_SUSPEND )
	{
		genCache_Status = eCACHETASK_STATUS_STOP;
		do {
			OSAL_sleep(1);
		} while ( genCache_Status != eCACHETASK_STATUS_DONE );
	}

	genCache_Status = eCACHETASK_STATUS_DONE;
}

PP_BOOL PPAPP_CACHE_IsDone(PP_VOID)
{
	if ( genCache_Status == eCACHETASK_STATUS_DONE )
		return PP_TRUE;

	return PP_FALSE;
}

/***************************************************************************************************************************************************************
 * Task function
***************************************************************************************************************************************************************/
PP_VOID vTaskCache(PP_VOID *pvData)
{
	LOG_DEBUG("Start Task(%s)\n", __FUNCTION__);
	
	genCache_Status = eCACHETASK_STATUS_RUN;

	for (;;)
	{
		if ( genCache_Status == eCACHETASK_STATUS_DONE )
		{
			OSAL_sleep(300);
		}
		else if ( genCache_Status == eCACHETASK_STATUS_STOP )
		{
#ifdef USE_PP_GUI
			if ( genCache_StopStatus == eCACHETASK_STOP_STATUS_NOMAL )
				AppTask_SendCmd(CMD_UI_CACHE_TASK_DONE, TASK_CACHE, TASK_UI, 0, PP_NULL, 0, 1000);
#endif
			genCache_StopStatus = eCACHETASK_STOP_STATUS_NOMAL;
			genCache_Status = eCACHETASK_STATUS_DONE;
		}
		else if ( gs32Cache_EndView >= 0 && gs32Cache_CurView != gs32Cache_EndView )
		{
			PPAPP_CACHE_LoadView(gs32Cache_CurView+1, gs32Cache_EndView);
		}
		else
		{
			if ( genCache_RunStatus == eCACHETASK_RUN_STATUS_BOTH || genCache_RunStatus == eCACHETASK_RUN_STATUS_UIDATA )
			{
				PPAPP_CACHE_LoadImage(0, eView360_carImage_tire01_start-1, eRSC_MODE_CAR);
				PPAPP_CACHE_LoadImage(0, ePGL_RSCLIST_MAX-1, eRSC_MODE_PGL);
				#if defined(USE_GUI_MENU)
				PPAPP_CACHE_LoadImage(0, eUI_RSCLIST_MAX-1, eRSC_MODE_UI);
				#else
				PPAPP_CACHE_LoadImage(0, 0, eRSC_MODE_UI);
				#endif
				#if defined (USE_CAR_WHEEL)
				PPAPP_CACHE_LoadImage(eView360_carImage_wheel_start, eView360_carImage_wheel_end, eRSC_MODE_CAR);
				#endif
				#if defined(USE_CAR_DOOR)
				PPAPP_CACHE_LoadImage(eTop2d_carImage_tire01_door_start, eTop2d_carImage_tire01_door_end, eRSC_MODE_CAR);
				#endif
			}

			genCache_Status = eCACHETASK_STATUS_STOP;
		}
	}

	gHandle[TASK_CACHE].phTask = PP_NULL;
	OSAL_destroy_thread(PP_NULL);
	
    return;
}
#else /* CACHE_VIEW_USE */

PP_VOID PPAPP_CACHE_Suspend(PP_VOID) {}
PP_VOID PPAPP_CACHE_Resume(PP_VOID) {}
PP_VOID PPAPP_CACHE_Stop(PP_VOID) {}
PP_BOOL PPAPP_CACHE_IsDone(PP_VOID) { return PP_TRUE; }

#endif /* CACHE_VIEW_USE */
