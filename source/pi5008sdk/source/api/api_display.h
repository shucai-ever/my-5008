#ifndef __API_DISPLAY_H__
#define __API_DISPLAY_H__

#ifdef __cplusplus
EXTERN "C" {
#endif

#include "type.h"
#include "error.h"
#include "du_drv.h"
#include "viewmode_config.h"

#if defined(USE_PP_GUI)
#include "application.h"

typedef enum ppRSC_MODE_E {
	eRSC_MODE_CAR,
	eRSC_MODE_UI,
	eRSC_MODE_PGL,

	eRSC_MODE_MAX
} PP_RSC_MODE_E;

PP_RESULT_E PPAPI_DISPLAY_LoadHeader (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_LoadUiImage (PP_U32 IN idx);
PP_RESULT_E PPAPI_DISPLAY_LoadCarImage (PP_U32 IN idx);
PP_RESULT_E PPAPI_DISPLAY_LoadPglImage (PP_U32 IN idx);
PP_VOID PPAPI_DISPLAY_PrintLoadImage (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_ParsingImage (PP_SCENE_E IN scene);
PP_RESULT_E PPAPI_DISPLAY_DisableAll (PP_VOID);
PP_RESULT_E PPAPI_DISPlAY_UpdateLUT (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_Background_On (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_Background_Off (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_BOOTING_CI (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_VIEW_Outline_On (PP_U32 IN mode, PP_U32 IN view);
PP_RESULT_E PPAPI_DISPLAY_VIEW_Outline_Off (PP_VOID);
PP_VOID PPAPI_DISPLAY_VIEW_UpdateWheel (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_MenuList (PP_SCENE_E IN scene);
PP_RESULT_E PPAPI_DISPLAY_MenuItem (PP_SCENE_E IN scene, PP_U32 IN idx, PP_BOOL IN isSel);
PP_RESULT_E PPAPI_DISPLAY_SubMenuList (PP_SCENE_E IN scene, PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx);
PP_RESULT_E PPAPI_DISPLAY_CALIB_Point (PP_POS_S* IN pnt, PP_U32 IN num);
PP_RESULT_E PPAPI_DISPLAY_CALIB_SelPoint (PP_POS_S* IN pnt, PP_U32 IN num, PP_U32 IN idx);
PP_RESULT_E PPAPI_DISPLAY_CALIB_MovePoint (PP_POS_S* IN pnt, PP_U32 IN num, PP_U32 IN idx);

PP_RESULT_E PPAPI_DISPLAY_POPUP_On (PP_POPUP_E IN msg);
PP_RESULT_E PPAPI_DISPLAY_POPUP_Off (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_PROGRESSBAR_On (PP_U32 IN level, PP_U32 IN flicker);
PP_RESULT_E PPAPI_DISPLAY_PROGRESSBAR_Off (PP_VOID);
PP_RESULT_E PPAPI_DISPLAY_DIALOG_On (PP_DIALOG_BOX_E IN box, PP_DIALOG_BTN_E IN btn);
PP_RESULT_E PPAPI_DISPLAY_DIALOG_Off (PP_VOID);

PP_RESULT_E PPAPI_DISPLAY_VIEW_Car_On (PP_U32 IN sectionID);
PP_VOID PPAPI_DISPLAY_VIEW_Car_Off(PP_VOID);
PP_VOID PPAPI_DISPLAY_VIEW_SetCarDoor (PP_CAR_DOOR_E IN open);
PP_RESULT_E PPAPI_DISPLAY_VIEW_PGL_On (PP_U32 IN sectionID, PP_BOOL IN isBw);
PP_VOID PPAPI_DISPLAY_VIEW_PGL_SetAngle (PP_S16 IN angle);
PP_RESULT_E PPAPI_DISPLAY_VIEW_PGL_Off (PP_VOID);


#if defined(CACHE_VIEW_USE)
PP_RESULT_E PPAPI_DISPLAY_LoadCacheAddr (PP_U32 IN carDeg, PP_U32* OUT pu32FlashAddr, PP_U32* OUT pu32DramAddr, PP_U32* OUT u32Size);
PP_RESULT_E PPAPI_DISPLAY_LoadCacheAddr_Shadow (PP_U32 IN shadowDeg, PP_U32* OUT pu32FlashAddr, PP_U32* OUT pu32DramAddr, PP_U32* OUT u32Size);
PP_VOID PPAPI_DISPLAY_SetSwingCar (PP_U32 IN carDeg);
#endif

#endif // USE_PP_GUI

// for VPU test
PP_VOID PPAPI_DISPLAY_VPU_Init (PP_RECT_S IN rect, PP_U32* IN buf);
PP_VOID PPAPI_DISPLAY_VPU_Screen (PP_BOOL IN enable);
PP_VOID PPAPI_DISPLAY_VPU_Deinit (PP_VOID);

PP_RESULT_E PPAPI_DISPlAY_Initialize (PP_VOID);

#ifdef __cplusplus
}
#endif

#endif // __APP_DISPLAY_H__
