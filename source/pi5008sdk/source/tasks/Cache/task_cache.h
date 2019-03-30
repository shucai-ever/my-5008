#ifndef __APP_CACHE_H__
#define __APP_CACHE_H__

/**
 * \file
 *
 * \brief	Cache Task : copy from flash to dram
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#include "type.h"

#ifdef CACHE_VIEW_USE

#include "error.h"
#include "viewmode_config.h"
#if defined(USE_PP_GUI)
#include "UI_rsclist.h"
#include "CAR_rsclist.h"
#include "PGL_rsclist.h"
#endif
/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#if defined(USE_PP_GUI)
#define INDEX_OFFSET		(eVIEWMODE_LOAD_TOP2D_SWING_0 - eView360_carImage_tire01_start)
#if defined(USE_SEPERATE_SHADOW)
#define SHADOW_INDEX_OFFSET	(eVIEWMODE_LOAD_TOP2D_SWING_0 - eView360_carImage_tire01_shadow_start)
#endif
#endif

/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
#ifdef __cplusplus
EXTERN "C" {
#endif

PP_BOOL PPAPP_CACHE_LoadData(PP_S32 s32StartView, PP_S32 s32EndView, PP_S32 s32BlockingEndView);
PP_BOOL PPAPP_CACHE_Reload(PP_BOOL bLoadSVMData, PP_BOOL bLoadUIData);

PP_VOID PPAPP_CACHE_Suspend(PP_VOID);
PP_VOID PPAPP_CACHE_Resume(PP_VOID);
PP_VOID PPAPP_CACHE_Stop(PP_VOID);
PP_BOOL PPAPP_CACHE_IsDone(PP_VOID);
PP_VOID vTaskCache(PP_VOID *pvData);

#ifdef __cplusplus
}
#endif

#else /* CACHE_VIEW_USE */

/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
#ifdef __cplusplus
EXTERN "C" {
#endif

PP_VOID PPAPP_CACHE_Suspend(PP_VOID);
PP_VOID PPAPP_CACHE_Resume(PP_VOID);
PP_VOID PPAPP_CACHE_Stop(PP_VOID);
PP_BOOL PPAPP_CACHE_IsDone(PP_VOID);

#ifdef __cplusplus
}
#endif

#endif /* CACHE_VIEW_USE */

#endif // __APP_CACHE_H__
