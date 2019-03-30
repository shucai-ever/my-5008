#include "system.h"
#include "debug.h"
#include "osal.h"
#include "sys_api.h"
#include "pinmux.h"
#include "task_manager.h"
#include "task_ui.h"
#include "task_display.h"
#include "api_display.h"
#include "vin.h"
#include "api_vin.h"
#include "du_drv.h"
#include "board_config.h"
#include "dram.h"
#include "api_flash.h"
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
#include "api_FAT_FTL.h"
#endif

#include "api_display_mem.h"
#include "api_swing_car_mem.h"
#include "pcvSvmView.h"

#if defined(USE_PP_GUI)
#include "UI_rsclist.h"
#include "CAR_rsclist.h"
#include "PGL_rsclist.h"


//===========================================================================
#define	CAR_WHEEL_MAX			4
#define DYNAMIC_PGL_MAX			71

typedef enum ppRSC_CAR_TYPE_E {
	eRSC_CAR_TYPE_CAR,
	eRSC_CAR_TYPE_SHADOW,
	eRSC_CAR_TYPE_WHEEL,
	eRSC_CAR_TYPE_DOOR,

	eRSC_CAR_TYPE_MAX
} PP_RSC_CAR_TYPE_E;

typedef enum ppRSC_PGL_TYPE_E {
	eRSC_PGL_TYPE_STATIC,
	eRSC_PGL_TYPE_DYNAMIC,

	eRSC_PGL_TYPE_MAX
} PP_RSC_PGL_TYPE_E;

typedef enum ppRSC_PGL_DIR_E {
	eRSC_PGL_DIR_BACKWARD,
	eRSC_PGL_DIR_FORWARD,

	eRSC_PGL_DIR_MAX
} PP_RSC_PGL_DIR_E;

#pragma pack(1)
typedef struct ppRSC_INFO_S {
	PP_U32 id;
	PP_U8 format;
	PP_U8 field;
	PP_U32 size;
	PP_U16 x;
	PP_U16 y;
	PP_U16 width;
	PP_U16 height;
	PP_U32 offset;
	PP_U16 sectionID;
	PP_U16 viewType;
	PP_U8 type;	// car or pgl
	PP_U8 dir;	// pgl dir
	PP_U32 *buf;
} PP_RSC_INFO_S;
#pragma pack()

#pragma pack(1)
typedef struct ppRSC_HEADER_S {
	PP_U32			num;
	PP_U32			reserved[3];
	PP_RSC_INFO_S	*info;
} PP_RSC_HEADER_S;
#pragma pack()

#pragma pack(1)
typedef struct ppRSC_LUT_S {
	PP_RSC_MODE_E mode;
	PP_U32 lut_id;
} PP_RSC_LUT_S;
#pragma pack()

#pragma pack(1)
typedef struct ppRSC_UI_IMG_S {
	PP_DU_LAYER_E	layer;
	PP_DU_AREA_E	area;
	PP_U32 			id;
	PP_RSC_INFO_S	*info;
} PP_RSC_UI_IMG_S;
#pragma pack()

#pragma pack(1)
typedef struct ppRSC_IMG_S {
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;
} PP_RSC_IMG_S;
#pragma pack()

#pragma pack(1)
typedef struct ppCAR_SECTION_S {
	PP_RSC_INFO_S	*car;
	PP_RSC_INFO_S	*shadow;
	PP_U32			wheelNum;
	PP_RSC_INFO_S	*wheel[CAR_WHEEL_MAX];
} PP_CAR_SECTION_S;
#pragma pack()

#pragma pack(1)
typedef struct ppPGL_SECTION_S {
	PP_RSC_INFO_S	*bwStatic;
	PP_U32			bwDynamicNum;
	PP_RSC_INFO_S	*bwDynamic[DYNAMIC_PGL_MAX];
	
	PP_RSC_INFO_S	*fwStatic;
	PP_U32			fwDynamicNum;
	PP_RSC_INFO_S	*fwDynamic[DYNAMIC_PGL_MAX];
} PP_PGL_SECTION_S;
#pragma pack()

PP_RSC_HEADER_S gUiRsc = {0};
PP_RSC_HEADER_S gCarRsc = {0};
PP_RSC_HEADER_S gPglRsc = {0};

PP_RSC_LUT_S	*gRscLut = PP_NULL;

PP_RSC_UI_IMG_S *gUiImg = PP_NULL;


PP_CAR_SECTION_S *gCarSection = PP_NULL;
PP_PGL_SECTION_S *gPglSection = PP_NULL;

PP_U32 wheelAngle = 0;
PP_CAR_SECTION_S *gCarWheelBase = PP_NULL;
PP_RSC_INFO_S *gPgl2dBase = PP_NULL;
PP_RSC_INFO_S *gPglRearCamBase = PP_NULL;

PP_RSC_INFO_S *gCarDoor[eCar_OpenDoor_Max] = { PP_NULL };

PP_BOOL gExistFlashRsc = TRUE;


// CAR
PP_RSC_IMG_S car2d 				=	{	eLayer2,	eArea0	};
PP_RSC_IMG_S car3d 				=	{	eLayer2,	eArea1	};

#if defined(USE_SEPERATE_SHADOW)
// Shadow
PP_RSC_IMG_S car2dShadow 		=	{	eLayer3,	eArea0	};
PP_RSC_IMG_S car3dShadow 		=	{	eLayer3,	eArea1	};
#endif

// PGL
#if defined(USE_SEPERATE_PGL)
PP_RSC_IMG_S staticPgl2d		=	{	eLayer3,	eArea0	};
PP_RSC_IMG_S staticPglRearCam	=	{	eLayer3,	eArea1	};
#endif
PP_RSC_IMG_S dynamicPgl2d		=	{	eLayer3,	eArea0	};
PP_RSC_IMG_S dynamicPglRearCam	=	{	eLayer3,	eArea1	};

// UI
PP_RSC_LUT_S bootingImgLUT[] =	{
										 {	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_Booting_Layer1	}
										,{	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S bootingImg[] = 	{
									{	eLayer1, 	eArea0, 	eBooting_CI_RLE, 	PP_NULL		},
									{	eLayer4,	eArea0,		eBackground_RLE,	PP_NULL		},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 		PP_NULL		}
								};

PP_RSC_LUT_S liveViewImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_CAR,	ecarImage_LUT		}
										,{	eRSC_MODE_CAR,	ecarImage_LUT		}
										,{	eRSC_MODE_PGL,	e_PGL_LUT			}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}	//{	eRSC_MODE_PGL,	e_PGL_LUT	}
									};

PP_RSC_UI_IMG_S liveViewImg[] =	{
									{	eLayer0,	eArea0,		ePopUp_NoCameraInput_RLE,		PP_NULL	},
									{	eLayer4,	eArea0, 	eOutline_mode_2d_RLE,			PP_NULL	},
									{	eLayer4,	eArea0, 	eOutline_mode_3d_RLE,			PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing0_RLE,	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing1_RLE,	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing2_RLE,	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing3_RLE,	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing4_RLE, 	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing5_RLE,	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing6_RLE,	PP_NULL	},
//									{	eLayer4,	eArea0, 	eOutline_mode_3d_swing7_RLE,	PP_NULL	},
									{	eLayer4,	eArea0, 	eOutline_mode_init_RLE,			PP_NULL	},
									{	eLayer4,	eArea0, 	eOutline_mode_none_RLE,			PP_NULL	},
									{	eLayer4,	eArea0, 	eOutline_mode_wide_RLE,			PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_front_RLE,		PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_left_RLE,			PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_none_RLE,			PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_rear_RLE,			PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_right_RLE,		PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_wide_RLE,			PP_NULL	},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 					PP_NULL	}
								};

PP_RSC_LUT_S mainMenuImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_UI,	e_LUT_MainMenu_Layer1	}
										,{	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_MainMenu_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S mainMenuImg[] = 	{
									{	eLayer0,	eArea0,		ePopUp_TBD_RLE,			PP_NULL	},
									{	eLayer1,	eArea0,		eVehicle_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0,		eTrigger_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0,		eVideo_Sel_RLE,			PP_NULL	},
									{	eLayer1,	eArea0,		eCamera_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0,		eUpdate_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0,		eCalibration_Sel_RLE,	PP_NULL	},
									{	eLayer1,	eArea0,		eSystemInfo_Sel_RLE,	PP_NULL	},
									{	eLayer3,	eArea0,		eMainMenu_RLE,			PP_NULL	},
									{	eLayer4,	eArea0,		eBackground_RLE,		PP_NULL	},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 			PP_NULL	}
								};

PP_RSC_LUT_S vehicleImgLUT[]	=	{
										 {	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_Vehicle_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Vehicle_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Vehicle_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S vehicleImg[] = {
								{	eLayer1,	eArea0, 	ePgl_Sel_RLE,				PP_NULL	},
								{	eLayer1,	eArea0, 	eDnmBlending_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eVehicleBack_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea1,		ePgl_Off_RLE,				PP_NULL	},
								{	eLayer1,	eArea1,		ePgl_Off_Sel_RLE,			PP_NULL	},
								{	eLayer1,	eArea1, 	ePgl_Rear_RLE,				PP_NULL	},
								{	eLayer1,	eArea1, 	ePgl_Rear_Sel_RLE,			PP_NULL	},
								{	eLayer1,	eArea1, 	ePgl_Always_RLE,			PP_NULL	},
								{	eLayer1,	eArea1, 	ePgl_Always_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eDnmBlending_Off_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eDnmBlending_Off_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2, 	eDnmBlending_On_RLE,		PP_NULL	},
								{	eLayer1,	eArea2, 	eDnmBlending_On_Sel_RLE,	PP_NULL	},
								{	eLayer3,	eArea0,		eVehicle_Menu_RLE,			PP_NULL	},
								{	eLayer4,	eArea0, 	eBackground_RLE, 			PP_NULL	},
								{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 				PP_NULL	}
							};


PP_RSC_LUT_S triggerImgLUT[]	=	{
										 {	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_Trigger_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Trigger_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Trigger_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S triggerImg[] =	{
								{	eLayer1,	eArea0, 	eTriggerBack_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eReverseSignal_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eReverseViewMode_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eTurnSignal_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eTurnViewMode_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eEmergencySignal_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eEmergencyViewMode_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea1,		eTriggerSignal_On_RLE,		PP_NULL	},
								{	eLayer1,	eArea1,		eTriggerSignal_On_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea1,		eTriggerSignal_Off_RLE,		PP_NULL	},
								{	eLayer1,	eArea1,		eTriggerSignal_Off_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3db_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3db_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_tb_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_tb_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3dlf_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3dlf_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3drf_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3drf_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3dl_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3dl_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3dr_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3dr_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3df_RLE,		PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_t3df_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea2,		eTriggerView_tf_RLE,		PP_NULL	},
								{	eLayer2,	eArea2,		eTriggerView_tf_Sel_RLE,	PP_NULL	},
								{	eLayer3,	eArea0,		eTrigger_Menu_RLE,			PP_NULL	},
								{	eLayer4,	eArea0, 	eBackground_RLE, 			PP_NULL	},
								{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 				PP_NULL	}
							};


PP_RSC_LUT_S videoImgLUT[]	=	{
										 {	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_Video_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Video_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Video_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S videoImg[] =	{
								{	eLayer1,	eArea0, 	eVideoBack_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eScreenX_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eScreenY_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eBrightness_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eContrast_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eSaturation_Sel_RLE,	PP_NULL	},
								{	eLayer2,	eArea1,		eStep_0_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_1_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_2_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_3_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_4_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_5_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_6_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_7_RLE,			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_8_RLE,			PP_NULL	},
								{	eLayer2,	eArea1, 	eStep_9_RLE, 			PP_NULL	},
								{	eLayer2,	eArea1, 	eStep_10_RLE, 			PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_0_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_1_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_2_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_3_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_4_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_5_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_6_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_7_RLE,		PP_NULL	},
								{	eLayer2,	eArea1,		eStep_Sel_8_RLE,		PP_NULL	},
								{	eLayer2,	eArea1, 	eStep_Sel_9_RLE, 		PP_NULL	},
								{	eLayer2,	eArea1, 	eStep_Sel_10_RLE, 		PP_NULL	},
								{	eLayer3,	eArea0,		eVideo_Menu_RLE,		PP_NULL	},
								{	eLayer4,	eArea0, 	eBackground_RLE, 		PP_NULL	},
								{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 			PP_NULL	}
							};

PP_RSC_LUT_S cameraImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_UI,	e_LUT_Camera_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Camera_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Camera_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S cameraImg[] =	{
								{	eLayer0,	eArea0,		ePopUp_Saved_RLE,		PP_NULL	},
								{	eLayer0,	eArea0,		ePopUp_NoSaved_RLE,		PP_NULL	},
								{	eLayer0,	eArea0,		ePopUp_NoSDCard_RLE,	PP_NULL	},
								{	eLayer0,	eArea1,		eProgress_0_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_10_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_20_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_30_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_40_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_50_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_60_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_70_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_80_RLE,		PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_90_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress_100_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_10_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_20_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_30_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_40_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_50_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_60_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_70_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_80_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_90_RLE,		PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_100_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraBack_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraFront_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraRear_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraLeft_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraRight_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraQuad_Sel_RLE,	PP_NULL	},
								{	eLayer1,	eArea0, 	eCameraCapture_Sel_RLE,	PP_NULL	},
								{	eLayer3,	eArea0,		eCamera_Menu_RLE,		PP_NULL	},
								{	eLayer4,	eArea0,		eBackground_RLE,		PP_NULL	},
								{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 			PP_NULL	}
							};

PP_RSC_LUT_S calibImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer1	}
										,{	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S calibImg[] = 	{
								{	eLayer0,	eArea0,		ePopUp_Calibration_RLE,		PP_NULL	},
								{	eLayer0,	eArea0,		ePopUp_Done_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress_0_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_10_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_20_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_30_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_40_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_50_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_60_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_70_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_80_RLE,			PP_NULL	},
								{	eLayer0,	eArea1, 	eProgress_90_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress_100_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_10_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_20_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_30_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_40_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_50_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_60_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_70_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_80_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_90_RLE,			PP_NULL	},
								{	eLayer0,	eArea1,		eProgress1_100_RLE,			PP_NULL	},
								{	eLayer0,	eArea2, 	eBtn_OK_Sel_RLE, 			PP_NULL	},
								{	eLayer0,	eArea2, 	eBtn_Cancel_Sel_RLE,	 	PP_NULL	},
								{	eLayer1,	eArea0, 	eCalibAuto_Sel_RLE,			PP_NULL	},
								{	eLayer1,	eArea0, 	eCalibManual_Sel_RLE,		PP_NULL	},
								{	eLayer1,	eArea0, 	eCalibBack_Sel_RLE,			PP_NULL	},
								{	eLayer1,	eArea1,		eDialog_RunMenu_RLE,		PP_NULL	},
								{	eLayer1,	eArea1,		eDialog_CalibFailRetry_RLE,	PP_NULL	},
								{	eLayer3,	eArea0,		eCalibration_Menu_RLE,		PP_NULL	},
								{	eLayer4,	eArea0,		eBackground_RLE,			PP_NULL	},
								{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 				PP_NULL	}	
							};


PP_RSC_LUT_S manualCalibImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer1	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer2n4	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer2n4	}
									};
PP_RSC_UI_IMG_S manualCalibImg[] =	{
									{	eLayer0,	eArea0, 	ePopUp_Calibration_RLE, 	PP_NULL	},
									{	eLayer0,	eArea0, 	ePopUp_Done_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_0_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_10_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_20_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_30_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_40_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_50_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_60_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_70_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_80_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_90_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_100_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_10_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_20_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_30_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_40_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_50_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_60_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_70_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_80_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress1_90_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_100_RLE,			PP_NULL	},
									{	eLayer0,	eArea2, 	eBtn_OK_Sel_RLE,			PP_NULL	},
									{	eLayer0,	eArea2, 	eBtn_Cancel_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0, 	eManualCalibFront_Sel_RLE,	PP_NULL	},
									{	eLayer1,	eArea0, 	eManualCalibRear_Sel_RLE,	PP_NULL	},
									{	eLayer1,	eArea0, 	eManualCalibLeft_Sel_RLE,	PP_NULL	},
									{	eLayer1,	eArea0, 	eManualCalibRight_Sel_RLE,	PP_NULL	},
									{	eLayer1,	eArea0, 	eManualCalibViewGen_Sel_RLE,PP_NULL	},
									{	eLayer1,	eArea0, 	eManualCalibBack_Sel_RLE,	PP_NULL	},
									{	eLayer1,	eArea1,		eDialog_CalibManual_RLE,	PP_NULL	},
									{	eLayer1,	eArea1,		eDialog_RunMenu_RLE,		PP_NULL	},
									{	eLayer1,	eArea1,		eDialog_CalibFailRetry_RLE,	PP_NULL	},
									{	eLayer2,	eArea0,		ePoint_RLE,					PP_NULL	},
									{	eLayer2,	eArea0,		ePoint_Sel_RLE,				PP_NULL	},
									{	eLayer4,	eArea0,		ePoint_Move_RLE,			PP_NULL	},
									{	eLayer3,	eArea0, 	eManualCalibration_Menu_RLE,PP_NULL	},
									{	eLayer3,	eArea1,		eMirror_RLE,				PP_NULL	},
									{	eLayer3,	eArea1,		eNormal_RLE,				PP_NULL	},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 				PP_NULL	}
								};

PP_RSC_LUT_S viewGenImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_UI,	e_LUT_Calibration_Layer1	}
										,{	0xFFFFFFFF,		0xFFFFFFFF	}
										#if defined(USE_16BIT_CAR)
										,{	0xFFFFFFFF,		0xFFFFFFFF	}
										#else
										,{	eRSC_MODE_CAR,	ecarImage_LUT	}
										#endif
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S viewGenImg[] = 	{
									{	eLayer0,	eArea0,		ePopUp_ViewGen_RLE,		PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_Done_RLE,		PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_DoNotTurnOff_RLE,PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_Shutdown_RLE,	PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_Saved_RLE,		PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_NoSaved_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress_0_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_10_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_20_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_30_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_40_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_50_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_60_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_70_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_80_RLE,		PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_90_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress_100_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_10_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_20_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_30_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_40_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_50_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_60_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_70_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_80_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_90_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_100_RLE,		PP_NULL	},
									{	eLayer0,	eArea2, 	eBtn_OK_Sel_RLE, 		PP_NULL	},
									{	eLayer0,	eArea2, 	eBtn_Cancel_Sel_RLE,	PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_tf_RLE,		PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_tb_RLE,		PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_t3df_RLE,		PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_t3db_RLE,		PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_t3dl_RLE,		PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_t3dlf_RLE, 	PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_t3dr_RLE,		PP_NULL	},
									{	eLayer0,	eArea3, 	eViewMode_t3drf_RLE,	PP_NULL	},
									{	eLayer1,	eArea1,		eDialog_SaveView_RLE,	PP_NULL	},
									{	eLayer4,	eArea0, 	eOutline_mode_none_RLE,	PP_NULL	},
									{	eLayer4,	eArea1, 	eOutline_view_none_RLE,	PP_NULL	},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 			PP_NULL	}
								};

PP_RSC_LUT_S updateImgLUT[]	=	{
										 {	eRSC_MODE_UI,	e_LUT_Common_Layer0	}
										,{	eRSC_MODE_UI,	e_LUT_Update_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Update_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_Update_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};
PP_RSC_UI_IMG_S updateImg[] = 	{
									{	eLayer0,	eArea0,		ePopUp_Done_RLE,			PP_NULL	},
									{	eLayer0,	eArea0,		ePopup_Failed_RLE,			PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_DoNotTurnOff_RLE,	PP_NULL	},
									{	eLayer0,	eArea0,		ePopUp_Shutdown_RLE,		PP_NULL	},
									{	eLayer0,	eArea0,		ePopup_NoUpdate_RLE,		PP_NULL	},
									{	eLayer0,	eArea1,		eProgress_0_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_10_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_20_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_30_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_40_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_50_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_60_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_70_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_80_RLE,			PP_NULL	},
									{	eLayer0,	eArea1, 	eProgress_90_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress_100_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_10_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_20_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_30_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_40_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_50_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_60_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_70_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_80_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_90_RLE,			PP_NULL	},
									{	eLayer0,	eArea1,		eProgress1_100_RLE,			PP_NULL	},
									{	eLayer0,	eArea2, 	eBtn_OK_Sel_RLE, 			PP_NULL	},
									{	eLayer0,	eArea2, 	eBtn_Cancel_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0, 	eFactoryReset_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea0, 	eFwUpdate_Sel_RLE,			PP_NULL	},
									{	eLayer1,	eArea0, 	eUpdateBack_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea1, 	eDialog_Update_RLE, 		PP_NULL	},
									{	eLayer3,	eArea0,		eUpdate_Menu_RLE,			PP_NULL	},
									{	eLayer4,	eArea0,		eBackground_RLE,			PP_NULL	},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 				PP_NULL	}
								};


PP_RSC_LUT_S systemInfoImgLUT[]	=	{
										 {	0xFFFFFFFF,		0xFFFFFFFF	}
										,{	eRSC_MODE_UI,	e_LUT_SystemInfo_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_SystemInfo_Layer1n2	}
										,{	eRSC_MODE_UI,	e_LUT_SystemInfo_Layer3	}
										,{	eRSC_MODE_UI,	e_LUT_Common_Layer4	}
									};

PP_RSC_UI_IMG_S systemInfoImg[] =	{
									{	eLayer1,	eArea0, 	eInfoBack_Sel_RLE,		PP_NULL	},
									{	eLayer1,	eArea1,		eFwVer_RLE,				PP_NULL	},
									{	eLayer1,	eArea2,		eLibVer_RLE,			PP_NULL	},
									{	eLayer1,	eArea3,		eIspVer_RLE,			PP_NULL	},
									{	eLayer2,	eArea0,		eSensorId_RLE,			PP_NULL	},
									{	eLayer3,	eArea0,		eSystemInfo_Menu_RLE,	PP_NULL	},
									{	eLayer4,	eArea0,		eBackground_RLE,		PP_NULL	},
									{	eLayer_MAX,	eArea_MAX,	0xFFFFFFFF, 			PP_NULL	}
								};

EXTERN PP_BOOL gUiLoadingDone;

//#define USE_IMAGE_LOAD_LOG

#define CarRscAddr(id)			(gCarRsc.info[id].offset + gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr)
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
#define UiRscAddr(id)			(gUiRsc.info[id].offset + gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr)
#else
#define UiRscAddr(id)			(gUiRsc.info[id].offset + gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr)
#endif
#define PglRscAddr(id)			(gPglRsc.info[id].offset + gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr)

#define RSC_PRE_HEADER_SIZE		(16)
#define RSC_HEADER_INFO_SIZE	(32)


// Notice!!! : Image Load Order Important by dram map!
PP_RESULT_E PPAPI_DISPLAY_LoadHeader (PP_VOID)
{
	PP_U32 i = 0, j = 0, num = 0;
	PP_RSC_UI_IMG_S *img = PP_NULL;
	PP_U32 ViewCnt;

	// version check
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	if(FLASH_VER_ERROR_CHECK(gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32Ver))
#else
	if(FLASH_VER_ERROR_CHECK(gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32Ver))
#endif
	{
		gExistFlashRsc = FALSE;
		printf("Not exist GUI resource in flash!\n");
		return eERROR_FAILURE;
	}

	if(FLASH_VER_ERROR_CHECK(gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32Ver))
	{
		gExistFlashRsc = FALSE;
		printf("Not exist PGL resource in flash!\n");
		return eERROR_FAILURE;
	}
	
	if(FLASH_VER_ERROR_CHECK(gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32Ver))
	{
		gExistFlashRsc = FALSE;
		printf("Not exist CAR resource in flash!\n");
		return eERROR_FAILURE;
	}

	// Allocation View Manager
	ViewCnt = PPAPI_SVMMEM_GetSectionCount();

	if(ViewCnt != 0)
	{
		gCarSection = (PP_CAR_SECTION_S *)OSAL_malloc(sizeof(PP_CAR_SECTION_S) * ViewCnt);
		if(gCarSection == PP_NULL)
		{
			printf("OSAL allocation fail!\n");
			return eERROR_FAILURE;
		}
		
		gPglSection = (PP_PGL_SECTION_S *)OSAL_malloc(sizeof(PP_PGL_SECTION_S) * ViewCnt);
		if(gPglSection == PP_NULL)
		{
			printf("OSAL allocation fail!\n");
			return eERROR_FAILURE;
		}
	}

	// Load GUI Resource Header
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PPAPI_FLASH_Read((PP_U8 *)&gUiRsc, gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr, RSC_PRE_HEADER_SIZE);
#else
	PPAPI_FTL_Read((PP_U8 *)&gUiRsc, gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr, RSC_PRE_HEADER_SIZE);
	PPAPI_SYS_CACHE_Writeback((PP_U32 *)&gUiRsc, RSC_PRE_HEADER_SIZE);
#endif
	if(gUiRsc.num != 0)
	{
		gUiRsc.info = (PP_RSC_INFO_S *)OSAL_malloc(sizeof(PP_RSC_INFO_S) * gUiRsc.num);
		if(gUiRsc.info == PP_NULL)
		{
			printf("OSAL allocation fail!\n");
			return eERROR_FAILURE;
		}
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		PPAPI_FLASH_Read((PP_U8 *)gUiRsc.info, gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr + RSC_PRE_HEADER_SIZE, (sizeof(PP_RSC_INFO_S) * gUiRsc.num));
#else
		PPAPI_FTL_Read((PP_U8 *)gUiRsc.info, gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr + RSC_PRE_HEADER_SIZE, (sizeof(PP_RSC_INFO_S) * gUiRsc.num));
		PPAPI_SYS_CACHE_Writeback((PP_U32 *)gUiRsc.info, (sizeof(PP_RSC_INFO_S) * gUiRsc.num));
#endif
	}
	
	// Load PGL Resource Header
	PPAPI_FLASH_Read((PP_U8 *)&gPglRsc, gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr, RSC_PRE_HEADER_SIZE);
	if(gPglRsc.num != 0)
	{
		gPglRsc.info = (PP_RSC_INFO_S *)OSAL_malloc(sizeof(PP_RSC_INFO_S) * gPglRsc.num);
		if(gPglRsc.info == PP_NULL)
		{
			printf("OSAL allocation fail!\n");
			return eERROR_FAILURE;
		}
		PPAPI_FLASH_Read((PP_U8 *)gPglRsc.info, gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr + RSC_PRE_HEADER_SIZE, (sizeof(PP_RSC_INFO_S) * gPglRsc.num));
	}

	// Load Car Image Resource Header
	PPAPI_FLASH_Read((PP_U8 *)&gCarRsc, gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr, RSC_PRE_HEADER_SIZE);
	if(gCarRsc.num != 0)
	{
		gCarRsc.info = (PP_RSC_INFO_S *)OSAL_malloc(sizeof(PP_RSC_INFO_S) * gCarRsc.num);
		if(gCarRsc.info == PP_NULL)
		{
			printf("OSAL allocation fail!\n");
			return eERROR_FAILURE;
		}
		PPAPI_FLASH_Read((PP_U8 *)gCarRsc.info, gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr + RSC_PRE_HEADER_SIZE, (sizeof(PP_RSC_INFO_S) * gCarRsc.num));
	}
	
	// Init Resource Image Info
#if defined(USE_GUI_MENU)
	for(i=0 ; i<eScene_Max ; i++)
#else
	for(i=0 ; i<eScene_MainMenu ; i++)
#endif
	{
		switch(i)
		{
			#if defined(USE_BOOTING_IMG)
			case eScene_Booting:
				img = (PP_RSC_UI_IMG_S *)&bootingImg;
				num = (sizeof(bootingImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			#endif
			case eScene_LiveView:
				img = (PP_RSC_UI_IMG_S *)&liveViewImg;
				num = (sizeof(liveViewImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_MainMenu:
				img = (PP_RSC_UI_IMG_S *)&mainMenuImg;
				num = (sizeof(mainMenuImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_VehicleSetting:
				img = (PP_RSC_UI_IMG_S *)&vehicleImg;
				num = (sizeof(vehicleImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_TriggerSetting:
				img = (PP_RSC_UI_IMG_S *)&triggerImg;
				num = (sizeof(triggerImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_VideoSetting:
				img = (PP_RSC_UI_IMG_S *)&videoImg;
				num = (sizeof(videoImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_CameraSetting:	//(+eScene_CameraPreview)(+eScene_CameraCapture)
				img = (PP_RSC_UI_IMG_S *)&cameraImg;
				num = (sizeof(cameraImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_CalibSetting:
				img = (PP_RSC_UI_IMG_S *)&calibImg;
				num = (sizeof(calibImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_ManualCalib:	// eScene_MovePoint
				img = (PP_RSC_UI_IMG_S *)&manualCalibImg;
				num = (sizeof(manualCalibImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_ViewGen:
				img = (PP_RSC_UI_IMG_S *)&viewGenImg;
				num = (sizeof(viewGenImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_SystemInfo:
				img = (PP_RSC_UI_IMG_S *)&systemInfoImg;
				num = (sizeof(systemInfoImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
			case eScene_UpdateSetting:
				img = (PP_RSC_UI_IMG_S *)&updateImg;
				num = (sizeof(updateImg) / sizeof(PP_RSC_UI_IMG_S)) - 1;
				break;
            default:
		    	break;
		}
		for(j=0 ; j<num ; j++)
		{
			img[j].info = (PP_RSC_INFO_S *)&gUiRsc.info[img[j].id];
		}
	}

	{
		PP_U32 id;

		// load ui image for booting
		for(j=0 ; j<6 ; j++)
		{
			switch(j)
			{
				#if defined(USE_BOOTING_IMG)
				case 0:	id = eBooting_CI_RLE;		break;
				case 1:	id = eBackground_RLE;		break;
				case 2:	id = e_LUT_Booting_Layer1;	break;
				#else
				case 0:
				case 1:
				case 2:
					continue;
				#endif
				case 3:	id = e_LUT_Common_Layer4;		break;
				case 4: id = eOutline_mode_init_RLE;	break;
				case 5: id = eOutline_view_rear_RLE;	break;
			}
			
			if(&gUiRsc.info[id] == PP_NULL)
				continue;
			
			if(xMemMangDisplayGetMinimumEverFreeSize() < gUiRsc.info[id].size)
				break;
			gUiRsc.info[id].buf = (PP_U32 *)pvMemMangDisplayMalloc (gUiRsc.info[id].size);
			if(gUiRsc.info[id].buf == PP_NULL)
			{
				printf("Display allocation fail!\n");
				return eERROR_FAILURE;
			}
			#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
			PPAPI_FLASH_ReadQDMA((PP_U8 *)gUiRsc.info[id].buf, UiRscAddr(gUiRsc.info[id].id), gUiRsc.info[id].size);
			#else
			PPAPI_FTL_Read((PP_U8 *)gUiRsc.info[id].buf, UiRscAddr(gUiRsc.info[id].id), gUiRsc.info[id].size);
			PPAPI_SYS_CACHE_Writeback(gUiRsc.info[id].buf, gUiRsc.info[id].size);
			#endif
		}

		// load LUT for Car Image
		for(j=0 ; j<3 ; j++)
		{
			switch(j)
			{
				case 0:
					id = eTop2d_carImage_tire01;
					break;
				case 1:
					#if defined(USE_16BIT_CAR)
					continue;
					#else
					id = ecarImage_LUT;
					#endif
					break;
				case 2:
					#if defined(USE_SEPERATE_SHADOW)
					id = eTop2d_carImage_tire01_shadow;
					#else
					continue;
					#endif
					break;
			}

			PPAPI_DISPLAY_LoadCarImage(id);
		}
	}

	return eSUCCESS;
}

PP_RESULT_E PPAPI_DISPLAY_LoadUiImage (PP_U32 IN idx)
{
#if defined(USE_GUI_MENU)
	if(&gUiRsc.info[idx] == PP_NULL)
		return eERROR_UNAVAILABLE;
	if(gUiRsc.info[idx].buf != PP_NULL)
		return eERROR_UNAVAILABLE;

	if(xMemMangDisplayGetMinimumEverFreeSize() < gUiRsc.info[idx].size)
		return eERROR_NO_MEM;
	gUiRsc.info[idx].buf = (PP_U32 *)pvMemMangDisplayMalloc (gUiRsc.info[idx].size);
	if(gUiRsc.info[idx].buf == PP_NULL)
	{
		printf("Display allocation fail!\n");
		return eERROR_FAILURE;
	}
	#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PPAPI_FLASH_ReadQDMA((PP_U8 *)gUiRsc.info[idx].buf, UiRscAddr(gUiRsc.info[idx].id), gUiRsc.info[idx].size);
	#else
	PPAPI_FTL_Read((PP_U8 *)gUiRsc.info[idx].buf, UiRscAddr(gUiRsc.info[idx].id), gUiRsc.info[idx].size);
	PPAPI_SYS_CACHE_Writeback(gUiRsc.info[idx].buf, gUiRsc.info[idx].size);
	#endif
#else
	PP_U32 i, j;

	for(i=eLayer0 ; i<eLayer_MAX ; i++)
	{
		if(liveViewImgLUT[i].mode != eRSC_MODE_UI)
			continue;

		j = liveViewImgLUT[i].lut_id;
		
		if(&gUiRsc.info[j] == PP_NULL)
			continue;
		if(gUiRsc.info[j].buf != PP_NULL)
			continue;
		
		if(xMemMangDisplayGetMinimumEverFreeSize() < gUiRsc.info[j].size)
			break;
		gUiRsc.info[j].buf = (PP_U32 *)pvMemMangDisplayMalloc (gUiRsc.info[j].size);
		if(gUiRsc.info[j].buf == PP_NULL)
		{
			printf("Display allocation fail!\n");
			return eERROR_FAILURE;
		}
		#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		PPAPI_FLASH_ReadQDMA((PP_U8 *)gUiRsc.info[j].buf, UiRscAddr(gUiRsc.info[j].id), gUiRsc.info[j].size);
		#else
		PPAPI_FTL_Read((PP_U8 *)gUiRsc.info[j].buf, UiRscAddr(gUiRsc.info[j].id), gUiRsc.info[j].size);
		PPAPI_SYS_CACHE_Writeback(gUiRsc.info[j].buf, gUiRsc.info[j].size);
		#endif
	}
	
	i = 0;
	do {
		j = liveViewImg[i].id;
		
		if(&gUiRsc.info[j] == PP_NULL)
			continue;
		if(gUiRsc.info[j].buf != PP_NULL)
			continue;
		
		if(xMemMangDisplayGetMinimumEverFreeSize() < gUiRsc.info[j].size)
			break;
		gUiRsc.info[j].buf = (PP_U32 *)pvMemMangDisplayMalloc (gUiRsc.info[j].size);
		if(gUiRsc.info[j].buf == PP_NULL)
		{
			printf("Display allocation fail!\n");
			return eERROR_FAILURE;
		}
		#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		PPAPI_FLASH_ReadQDMA((PP_U8 *)gUiRsc.info[j].buf, UiRscAddr(gUiRsc.info[j].id), gUiRsc.info[j].size);
		#else
		PPAPI_FTL_Read((PP_U8 *)gUiRsc.info[j].buf, UiRscAddr(gUiRsc.info[j].id), gUiRsc.info[j].size);
		PPAPI_SYS_CACHE_Writeback(gUiRsc.info[j].buf, gUiRsc.info[j].size);
		#endif
		
	} while(liveViewImg[++i].id != 0xFFFFFFFF);
#endif

	return eSUCCESS;
}

PP_RESULT_E PPAPI_DISPLAY_LoadCarImage (PP_U32 IN idx)
{
	if(&gCarRsc.info[idx] == PP_NULL)
		return eERROR_UNAVAILABLE;
	if(gCarRsc.info[idx].buf != PP_NULL)
		return eERROR_UNAVAILABLE;

	if(xMemMangDisplayGetMinimumEverFreeSize() < gCarRsc.info[idx].size)
		return eERROR_NO_MEM;
	
	gCarRsc.info[idx].buf = (PP_U32 *)pvMemMangDisplayMalloc (gCarRsc.info[idx].size);
	if(gCarRsc.info[idx].buf == PP_NULL)
	{
		printf("Display allocation fail!\n");
		return eERROR_FAILURE;
	}
	PPAPI_FLASH_ReadQDMA((PP_U8 *)gCarRsc.info[idx].buf, CarRscAddr(gCarRsc.info[idx].id), gCarRsc.info[idx].size);

	if( (gCarRsc.info[idx].format != 255) && gCarSection )
	{
		if(gCarRsc.info[idx].type == eRSC_CAR_TYPE_CAR)
		{
			gCarSection[gCarRsc.info[idx].sectionID].car = &gCarRsc.info[idx];
			#if defined(USE_SEPERATE_SHADOW)
			gCarSection[gCarRsc.info[idx].sectionID].wheel[gCarSection[gCarRsc.info[idx].sectionID].wheelNum] = &gCarRsc.info[idx];
			gCarSection[gCarRsc.info[idx].sectionID].wheelNum++;
			#endif
			#if defined(USE_CAR_DOOR)
			if (idx == eTop2d_carImage_tire01)
				gCarDoor[eCar_OpenDoor] = &gCarRsc.info[idx];
			#endif
		}
		else if(gCarRsc.info[idx].type == eRSC_CAR_TYPE_SHADOW)
		{
			#if defined(USE_SEPERATE_SHADOW)
			gCarSection[gCarRsc.info[idx].sectionID].shadow = &gCarRsc.info[idx];
			#endif
		}
		else if(gCarRsc.info[idx].type == eRSC_CAR_TYPE_WHEEL)
		{
			#if defined(USE_CAR_WHEEL)
			gCarSection[gCarRsc.info[idx].sectionID].wheel[gCarSection[gCarRsc.info[idx].sectionID].wheelNum] = &gCarRsc.info[idx];
			gCarSection[gCarRsc.info[idx].sectionID].wheelNum++;
			#endif
		}
		else if(gCarRsc.info[idx].type == eRSC_CAR_TYPE_DOOR)
		{
			#if defined(USE_CAR_DOOR)
			PP_RSC_INFO_S **door = PP_NULL;

			if (idx == eTop2d_carImage_tire01_lf)				door = &gCarDoor[eCar_OpenDoor_LF];
			else if (idx == eTop2d_carImage_tire01_lf_rf)		door = &gCarDoor[eCar_OpenDoor_LF_RF];
			else if (idx == eTop2d_carImage_tire01_lf_rf_lr)	door = &gCarDoor[eCar_OpenDoor_LF_RF_LR];
			else if (idx == eTop2d_carImage_tire01_lf_rf_rr)	door = &gCarDoor[eCar_OpenDoor_LF_RF_RR];
			else if (idx == eTop2d_carImage_tire01_lf_rf_lr_rr)	door = &gCarDoor[eCar_OpenDoor_LF_RF_LR_RR];
			else if (idx == eTop2d_carImage_tire01_lf_lr)		door = &gCarDoor[eCar_OpenDoor_LF_LR];
			else if (idx == eTop2d_carImage_tire01_lf_lr_rr)	door = &gCarDoor[eCar_OpenDoor_LF_LR_RR];
			else if (idx == eTop2d_carImage_tire01_lf_rr)		door = &gCarDoor[eCar_OpenDoor_LF_RR];
			else if (idx == eTop2d_carImage_tire01_rf)			door = &gCarDoor[eCar_OpenDoor_RF];
			else if (idx == eTop2d_carImage_tire01_rf_lr)		door = &gCarDoor[eCar_OpenDoor_RF_LR];
			else if (idx == eTop2d_carImage_tire01_rf_lr_rr)	door = &gCarDoor[eCar_OpenDoor_RF_LR_RR];
			else if (idx == eTop2d_carImage_tire01_rf_rr)		door = &gCarDoor[eCar_OpenDoor_RF_RR];
			else if (idx == eTop2d_carImage_tire01_lr)			door = &gCarDoor[eCar_OpenDoor_LR];
			else if (idx == eTop2d_carImage_tire01_lr_rr)		door = &gCarDoor[eCar_OpenDoor_LR_RR];
			else if (idx == eTop2d_carImage_tire01_rr)			door = &gCarDoor[eCar_OpenDoor_RR];
			else												door = PP_NULL;

			if(door)											*door = &gCarRsc.info[idx];
			#endif
		}
	}

	return eSUCCESS;
}

PP_RESULT_E PPAPI_DISPLAY_LoadPglImage (PP_U32 IN idx)
{
	if(&gPglRsc.info[idx] == PP_NULL)
		return eERROR_UNAVAILABLE;
	if(gPglRsc.info[idx].buf != PP_NULL)
		return eERROR_UNAVAILABLE;

	if(xMemMangDisplayGetMinimumEverFreeSize() < gPglRsc.info[idx].size)
		return eERROR_NO_MEM;
	gPglRsc.info[idx].buf = (PP_U32 *)pvMemMangDisplayMalloc (gPglRsc.info[idx].size);
	if(gPglRsc.info[idx].buf == PP_NULL)
	{
		printf("Display allocation fail!\n");
		return eERROR_FAILURE;
	}
	PPAPI_FLASH_ReadQDMA((PP_U8 *)gPglRsc.info[idx].buf, PglRscAddr(gPglRsc.info[idx].id), gPglRsc.info[idx].size);

	if( (gPglRsc.info[idx].format != 255) && gPglSection )
	{
		if(gPglRsc.info[idx].type == eRSC_PGL_TYPE_STATIC)	// static
		{
			if(gPglRsc.info[idx].dir == eRSC_PGL_DIR_BACKWARD)	// bw
			{
				gPglSection[gPglRsc.info[idx].sectionID].bwStatic = &gPglRsc.info[idx];
			}
			else	// fw
			{
				gPglSection[gPglRsc.info[idx].sectionID].fwStatic = &gPglRsc.info[idx];
			}
		}
		else	// dynamic
		{
			if(gPglRsc.info[idx].dir == eRSC_PGL_DIR_BACKWARD)	// bw
			{
				gPglSection[gPglRsc.info[idx].sectionID].bwDynamic[gPglSection[gPglRsc.info[idx].sectionID].bwDynamicNum] = &gPglRsc.info[idx];
				gPglSection[gPglRsc.info[idx].sectionID].bwDynamicNum++;
			}
			else	// fw
			{
				gPglSection[gPglRsc.info[idx].sectionID].fwDynamic[gPglSection[gPglRsc.info[idx].sectionID].fwDynamicNum] = &gPglRsc.info[idx];
				gPglSection[gPglRsc.info[idx].sectionID].fwDynamicNum++;
			}
		}
	}

	return eSUCCESS;
}

PP_VOID PPAPI_DISPLAY_PrintLoadImage (PP_VOID)
{
	PP_U32 i;

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	LOG_DEBUG("ui : ver(%d), addr(0x%08X), size(%d)\n", gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32Ver, gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr, gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32Size);
#else
	LOG_DEBUG("ui : ver(%d), addr(0x%08X), size(%d)\n", gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32Ver, gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32Size);
#endif
	LOG_DEBUG("  rsc num : %d\n", gUiRsc.num);
	for(i=0 ; i<gUiRsc.num ; i++)
	{
		LOG_DEBUG("  %02d. format(%d), field(%d), size(%d), (%d, %d, %d, %d), addr(0x%08X), buf(0x%08X)\n"
					,	gUiRsc.info[i].id
					,	gUiRsc.info[i].format
					,	gUiRsc.info[i].field
					,	gUiRsc.info[i].size
					,	gUiRsc.info[i].x
					,	gUiRsc.info[i].y
					,	gUiRsc.info[i].width
					,	gUiRsc.info[i].height
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
					,	gUiRsc.info[i].offset+gstFlashHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr
#else
					,	gUiRsc.info[i].offset+gstFlashFTLHeader.stSect[eFLASH_SECT_UI_IMG].u32FlashAddr
#endif
					,	gUiRsc.info[i].buf
					);
		}
	
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		LOG_DEBUG("pgl : ver(%d), addr(0x%08X), size(%d)\n", gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32Ver, gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr, gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32Size);
#else
		LOG_DEBUG("pgl : ver(%d), addr(0x%08X), size(%d)\n", gstFlashFTLHeader.stSect[eFLASH_SECT_PGL_IMG].u32Ver, gstFlashFTLHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_PGL_IMG].u32Size);
#endif
		LOG_DEBUG("  rsc num : %d\n", gPglRsc.num);
		for(i=0 ; i<gPglRsc.num ; i++)
		{
			LOG_DEBUG("  %02d. format(%d), field(%d), size(%d), (%d, %d, %d, %d), addr(0x%08X), sectionID(%d), viewType(%d), pglType(%d), pglDir(%d), buf(0x%08X)\n"
						,	gPglRsc.info[i].id
						,	gPglRsc.info[i].format
						,	gPglRsc.info[i].field
						,	gPglRsc.info[i].size
						,	gPglRsc.info[i].x
						,	gPglRsc.info[i].y
						,	gPglRsc.info[i].width
						,	gPglRsc.info[i].height
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
						,	gUiRsc.info[i].offset+gstFlashHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr
#else
						,	gUiRsc.info[i].offset+gstFlashFTLHeader.stSect[eFLASH_SECT_PGL_IMG].u32FlashAddr
#endif
						,	gPglRsc.info[i].sectionID
						,	gPglRsc.info[i].viewType
						,	gPglRsc.info[i].type
						,	gPglRsc.info[i].dir
						,	gPglRsc.info[i].buf
					);
		}
	
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		LOG_DEBUG("car : ver(%d), addr(0x%08X), size(%d)\n", gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32Ver, gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr, gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32Size);
#else
		LOG_DEBUG("car : ver(%d), addr(0x%08X), size(%d)\n", gstFlashFTLHeader.stSect[eFLASH_SECT_CAR_IMG].u32Ver, gstFlashFTLHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr, gstFlashFTLHeader.stSect[eFLASH_SECT_CAR_IMG].u32Size);
#endif
		LOG_DEBUG("  rsc num : %d\n", gCarRsc.num);
		for(i=0 ; i<gCarRsc.num ; i++)
		{
			LOG_DEBUG("  %02d. format(%d), field(%d), size(%d), (%d, %d, %d, %d), addr(0x%08X), sectionID(%d), viewType(%d), carType(%d),buf(0x%08X)\n"
						,	gCarRsc.info[i].id
						,	gCarRsc.info[i].format
						,	gCarRsc.info[i].field
						,	gCarRsc.info[i].size
						,	gCarRsc.info[i].x
						,	gCarRsc.info[i].y
						,	gCarRsc.info[i].width
						,	gCarRsc.info[i].height
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
						,	gUiRsc.info[i].offset+gstFlashHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr
#else
						,	gUiRsc.info[i].offset+gstFlashFTLHeader.stSect[eFLASH_SECT_CAR_IMG].u32FlashAddr
#endif
						,	gCarRsc.info[i].sectionID
						,	gCarRsc.info[i].viewType
						,	gCarRsc.info[i].type
						,	gCarRsc.info[i].buf
						);
		}
	
		LOG_DEBUG("*	[usable reserved size : %d Byte]\n", (PP_S32)xMemMangDisplayGetFreeSize());
		LOG_DEBUG("*	[used reserved size : %d Byte\n", (PP_S32)DISPLAY_RESERVED_BUFFSIZE - xMemMangDisplayGetFreeSize());
}

PP_RESULT_E PPAPI_DISPLAY_ParsingImage (PP_SCENE_E IN scene)
{
	if(!gExistFlashRsc)
	{
		printf("Not exist resource in flash!\n");
		return eERROR_FAILURE;
	}

	switch(scene)
	{
		case eScene_Booting:
			gRscLut = (PP_RSC_LUT_S *)&bootingImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&bootingImg;
			break;
		case eScene_LiveView:
			gRscLut = (PP_RSC_LUT_S *)&liveViewImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&liveViewImg;
			break;
		case eScene_MainMenu:
			gRscLut = (PP_RSC_LUT_S *)&mainMenuImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&mainMenuImg;
			break;
		case eScene_VehicleSetting:
			gRscLut = (PP_RSC_LUT_S *)&vehicleImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&vehicleImg;
			break;
		case eScene_TriggerSetting:
			gRscLut = (PP_RSC_LUT_S *)&triggerImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&triggerImg;
			break;
		case eScene_VideoSetting:
			gRscLut = (PP_RSC_LUT_S *)&videoImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&videoImg;
			break;
		case eScene_CameraSetting:
			gRscLut = (PP_RSC_LUT_S *)&cameraImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&cameraImg;
			break;
		case eScene_CalibSetting:
			gRscLut = (PP_RSC_LUT_S *)&calibImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&calibImg;
			break;
		case eScene_ManualCalib:	// + eScene_MovePoint
			gRscLut = (PP_RSC_LUT_S *)&manualCalibImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&manualCalibImg;
			break;
		case eScene_ViewGen:
			gRscLut = (PP_RSC_LUT_S *)&viewGenImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&viewGenImg;
			break;
		case eScene_SystemInfo:
			gRscLut = (PP_RSC_LUT_S *)&systemInfoImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&systemInfoImg;
			break;
		case eScene_UpdateSetting:
			gRscLut = (PP_RSC_LUT_S *)&updateImgLUT;
			gUiImg = (PP_RSC_UI_IMG_S *)&updateImg;
			break;
        default:
			break;
	}

	return eSUCCESS;
}

STATIC PP_RSC_UI_IMG_S *PPAPI_DISPLAY_GetUiImg (PP_U32 IN id)
{
	PP_U32 i = 0;

	if(!gExistFlashRsc)
	{
		printf("Not exist resource in flash!\n");
		return PP_NULL;
	}

	do {
		if(gUiImg[i].id == id)
			return &gUiImg[i];
	} while(gUiImg[++i].id != 0xFFFFFFFF);
	
	printf("Not exist image!\n");
	return PP_NULL;
}

#if defined(CACHE_VIEW_USE)
PP_RESULT_E PPAPI_DISPLAY_LoadCacheAddr (PP_U32 IN carDeg, PP_U32* OUT pu32FlashAddr, PP_U32* OUT pu32DramAddr, PP_U32* OUT u32Size)
{
	if(carDeg > gCarRsc.num)
		return eERROR_FAILURE;
	if(&gCarRsc.info[carDeg] == PP_NULL)
		return eERROR_FAILURE;

	// exception code for 0degree == 360degree
	if( carDeg == (eView360_carImage_tire01_end + 1) )
	{
		if(&gCarRsc.info[eView360_carImage_tire01_start] != PP_NULL)
		{
			gCarSection[gCarRsc.info[eView360_carImage_tire01_end].sectionID + 1].car = &gCarRsc.info[eView360_carImage_tire01_start];
			return eSUCCESS;
		}
		else
		{
			return eERROR_FAILURE;
		}
	}

	if(xMemMangSwingCarGetMinimumEverFreeSize() < gCarRsc.info[carDeg].size)
		return eERROR_FAILURE;

	gCarRsc.info[carDeg].buf = (PP_U32 *)pvMemMangSwingCarMalloc (gCarRsc.info[carDeg].size);
	if(gCarRsc.info[carDeg].buf == PP_NULL)
	{
		printf("Swing Car allocation fail!\n");
		return eERROR_FAILURE;
	}

	*pu32DramAddr = (PP_U32)gCarRsc.info[carDeg].buf;
	*pu32FlashAddr = CarRscAddr(gCarRsc.info[carDeg].id);
	*u32Size = gCarRsc.info[carDeg].size;

	if(gCarSection)
	{
		gCarSection[gCarRsc.info[carDeg].sectionID].car = &gCarRsc.info[carDeg];
	}
	
	return eSUCCESS;
}

PP_RESULT_E PPAPI_DISPLAY_LoadCacheAddr_Shadow (PP_U32 IN shadowDeg, PP_U32* OUT pu32FlashAddr, PP_U32* OUT pu32DramAddr, PP_U32* OUT u32Size)
{
#if defined(USE_SEPERATE_SHADOW)
	if(shadowDeg > gCarRsc.num)
		return eERROR_FAILURE;
	if(&gCarRsc.info[shadowDeg] == PP_NULL)
		return eERROR_FAILURE;
	
	// exception code for 0degree == 360degree
	if( shadowDeg == (eView360_carImage_tire01_shadow_end + 1) )
	{
		if(&gCarRsc.info[eView360_carImage_tire01_shadow_start] != PP_NULL)
		{
			gCarSection[gCarRsc.info[eView360_carImage_tire01_shadow_end].sectionID + 1].shadow = &gCarRsc.info[eView360_carImage_tire01_shadow_start];
			return eSUCCESS;
		}
		else
		{
			return eERROR_FAILURE;
		}
	}

	if(xMemMangSwingCarGetMinimumEverFreeSize() < gCarRsc.info[shadowDeg].size)
		return eERROR_FAILURE;
		
	gCarRsc.info[shadowDeg].buf = (PP_U32 *)pvMemMangSwingCarMalloc (gCarRsc.info[shadowDeg].size);
	if(gCarRsc.info[shadowDeg].buf == PP_NULL)
	{
		printf("Swing Car Shadow allocation fail!\n");
		return eERROR_FAILURE;
	}

	*pu32DramAddr = (PP_U32)gCarRsc.info[shadowDeg].buf;
	*pu32FlashAddr = CarRscAddr(gCarRsc.info[shadowDeg].id);
	*u32Size = gCarRsc.info[shadowDeg].size;

	if(gCarSection)
	{
		gCarSection[gCarRsc.info[shadowDeg].sectionID].shadow = &gCarRsc.info[shadowDeg];
	}
	
	return eSUCCESS;
#else
	return eERROR_NOT_SUPPORT;
#endif
}

PP_VOID PPAPI_DISPLAY_SetSwingCar (PP_VIEWMODE_E IN viewDeg)
{
	PP_RSC_INFO_S *car;
	#if defined(USE_SEPERATE_SHADOW)
	PP_RSC_INFO_S *shadow;
	#endif

	if(gCarSection == PP_NULL)
		return;
	
	car = gCarSection[viewDeg].car;
	if(car)
	{
		PPDRV_DU_OSD_SetArea(car3d.layer, car3d.area, car->buf, PP_NULL, car->size, (PP_U32)PP_NULL, car->format);
	}
	
	#if defined(USE_SEPERATE_SHADOW)
	shadow = gCarSection[viewDeg].shadow;
	if(shadow)
	{
		PPDRV_DU_OSD_SetArea(car3dShadow.layer, car3dShadow.area, shadow->buf, PP_NULL, shadow->size, (PP_U32)PP_NULL, shadow->format);
	}
	#endif
}
#endif

PP_RESULT_E PPAPI_DISPlAY_UpdateLUT (PP_VOID)
{
	PP_U32 i = 0;
	PP_U32 lut[256] __attribute__((aligned(16))) = {0};
	
	if(!gExistFlashRsc)
	{
		printf("Not exist resource in flash!\n");
		return eERROR_FAILURE;
	}

	PPDRV_DU_WaitBlank();

	for(i=eLayer0 ; i<eLayer_MAX ; i++)
	{
		if(gRscLut[i].lut_id == 0xFFFFFFFF)
			continue;

		if(gRscLut[i].mode == eRSC_MODE_CAR)
			memcpy(&lut, gCarRsc.info[gRscLut[i].lut_id].buf, 1024);
		else if(gRscLut[i].mode == eRSC_MODE_UI)
			memcpy(&lut, gUiRsc.info[gRscLut[i].lut_id].buf, 1024);
		else if(gRscLut[i].mode == eRSC_MODE_PGL)
			memcpy(&lut, gPglRsc.info[gRscLut[i].lut_id].buf, 1024);

		PPDRV_DU_OSD_SetColorLut(i, (PP_U32 *)&lut);
	}
	
	return eSUCCESS;
}


//--------------------------------------------------------
PP_RESULT_E PPAPI_DISPLAY_DisableAll (PP_VOID)
{
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;
	PP_RESULT_E result;

	result = PPDRV_DU_OSD_EnableArea(eLayer0, eArea2, PP_FALSE);
	if(result != eSUCCESS)	return result;
	result = PPDRV_DU_OSD_EnableArea(eLayer0, eArea3, PP_FALSE);
	if(result != eSUCCESS)	return result;
	
	for(layer=eLayer1 ; layer<eLayer4 ; layer++)
	{
		for(area=eArea0 ; area<eArea_MAX ; area++)
		{
			result = PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);
			if(result != eSUCCESS)
				return result;
		}
	}

	return result;
}

//===========================================================================

PP_RESULT_E PPAPI_DISPLAY_Background_On (PP_VOID)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;

	PPDRV_DU_OSD_SetMixerPath(ePATH_IN_43210_OUT);
	#if defined(USE_16BIT_CAR)
	PPDRV_DU_OSD_SetLayerFormat(eLayer3, eFORMAT_RLE);
	#endif

	img = PPAPI_DISPLAY_GetUiImg(eBackground_RLE);
	if(!img)
		return eERROR_FAILURE;
	
	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_Background_Off (PP_VOID)
{
	PP_RESULT_E result;
	
	PPDRV_DU_OSD_SetMixerPath(ePATH_IN_43210_OUT);
	#if defined(USE_16BIT_CAR)
	PPDRV_DU_OSD_SetLayerFormat(eLayer3, eFORMAT_RLE);
	#endif
	
	result = PPDRV_DU_OSD_EnableArea(eLayer4, eArea0, PP_FALSE);
	result = PPDRV_DU_OSD_EnableArea(eLayer4, eArea1, PP_FALSE);
	result = PPDRV_DU_OSD_EnableArea(eLayer4, eArea2, PP_FALSE);
	result = PPDRV_DU_OSD_EnableArea(eLayer4, eArea3, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_BOOTING_CI (PP_VOID)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;

	img = PPAPI_DISPLAY_GetUiImg(eBooting_CI_RLE);
	if(!img)
		return eERROR_FAILURE;
	
	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_VIEW_Car_On (PP_U32 IN sectionID)
{
	PP_RSC_INFO_S *car = PP_NULL;
		PP_DU_LAYER_E carLayer;
	PP_DU_AREA_E carArea;
	#if defined(USE_SEPERATE_SHADOW)
	PP_RSC_INFO_S *shadow = PP_NULL;
	PP_DU_LAYER_E shadowLayer;
	PP_DU_AREA_E shadowArea;
	#endif
	PP_RECT_S rect;
	PP_RESULT_E result;

	if(gCarSection == PP_NULL)
		return eERROR_FAILURE;

	car = gCarSection[sectionID].car;
	#if defined(USE_SEPERATE_SHADOW)
	shadow = gCarSection[sectionID].shadow;
	#endif
	
	if(car)
	{
		if(car->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_2D)
		{
			carLayer = car2d.layer;
			carArea = car2d.area;
			#if defined(USE_SEPERATE_SHADOW)
			shadowLayer = car2dShadow.layer;
			shadowArea = car2dShadow.area;
			#endif

			#if defined(USE_CAR_DOOR)
			car = gCarDoor[gCarOpenDoor];
			#endif
		}
		else if(car->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_3D)
		{
			carLayer = car3d.layer;
			carArea = car3d.area;
			#if defined(USE_SEPERATE_SHADOW)
			shadowLayer = car3dShadow.layer;
			shadowArea = car3dShadow.area;
			#endif
			
			gCarWheelBase = &gCarSection[sectionID];
			wheelAngle = 0;
		}
		else if(car->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_DEG360)
		{
			carLayer = car3d.layer;
			carArea = car3d.area;
			#if defined(USE_SEPERATE_SHADOW)
			shadowLayer = car3dShadow.layer;
			shadowArea = car3dShadow.area;
			#endif

			gCarWheelBase = &gCarSection[sectionID];
			wheelAngle = 0;
		}
	}
	else
	{
		carLayer = car3d.layer;
		carArea = car3d.area;
		#if defined(USE_SEPERATE_SHADOW)
		shadowLayer = car3dShadow.layer;
		shadowArea = car3dShadow.area;
		#endif
	}
	
#if defined(USE_SEPERATE_SHADOW)
	if(shadow)
	{
		rect.u16X = shadow->x;
		rect.u16Y = shadow->y;
		rect.u16Width = shadow->width;
		rect.u16Height = shadow->height;
		result = PPDRV_DU_OSD_SetAreaConfig(shadowLayer, shadowArea, shadow->buf, PP_NULL, shadow->size, rect, PP_NULL, shadow->format);
		if(result == eSUCCESS)
			PPDRV_DU_OSD_EnableArea(shadowLayer, shadowArea, PP_TRUE);
		else
			PPDRV_DU_OSD_EnableArea(shadowLayer, shadowArea, PP_FALSE);
	}
	else
	{
		PPDRV_DU_OSD_EnableArea(shadowLayer, shadowArea, PP_FALSE);
	}
#endif

	if(car)
	{
		rect.u16X = car->x;
		rect.u16Y = car->y;
		rect.u16Width = car->width;
		rect.u16Height = car->height;

		result = PPDRV_DU_OSD_SetAreaConfig(carLayer, carArea, car->buf, PP_NULL, car->size, rect, PP_NULL, car->format);
		if(result == eSUCCESS)
			PPDRV_DU_OSD_EnableArea(carLayer, carArea, PP_TRUE);
		else
			PPDRV_DU_OSD_EnableArea(carLayer, carArea, PP_FALSE);
	}
	else
	{
		PPDRV_DU_OSD_EnableArea(carLayer, carArea, PP_FALSE);
	}
	return result;
}

PP_VOID PPAPI_DISPLAY_VIEW_Car_Off (PP_VOID)
{
	PPDRV_DU_OSD_EnableArea(car2d.layer, car2d.area, PP_FALSE);
	PPDRV_DU_OSD_EnableArea(car3d.layer, car3d.area, PP_FALSE);
#if defined(USE_SEPERATE_SHADOW)
	PPDRV_DU_OSD_EnableArea(car2dShadow.layer, car2dShadow.area, PP_FALSE);
	PPDRV_DU_OSD_EnableArea(car3dShadow.layer, car3dShadow.area, PP_FALSE);
#endif
}

PP_VOID PPAPI_DISPLAY_VIEW_SetCarDoor (PP_CAR_DOOR_E IN open)
{
	#if defined(USE_CAR_DOOR)
	if(gCarDoor[open])
	{
		PPDRV_DU_OSD_SetArea(car2d.layer, car2d.area, gCarDoor[open]->buf, PP_NULL, gCarDoor[open]->size, (PP_U32)PP_NULL, gCarDoor[open]->format);
	}
	#endif
}

PP_VOID PPAPI_DISPLAY_VIEW_UpdateWheel (PP_VOID)
{
	PP_RSC_INFO_S *carWheel;
	
	if(gCarWheelBase == PP_NULL)
		return;
	if(gCarWheelBase->wheelNum <= 1)
		return;

	wheelAngle = (wheelAngle + 1) % gCarWheelBase->wheelNum;
	carWheel = gCarWheelBase->wheel[wheelAngle];
	if(carWheel)
	{
		PPDRV_DU_OSD_SetArea(car3d.layer, car3d.area, carWheel->buf, PP_NULL, carWheel->size, (PP_U32)PP_NULL, carWheel->format);
	}
}

PP_RESULT_E PPAPI_DISPLAY_VIEW_PGL_On (PP_U32 IN sectionID, PP_BOOL IN isBw)
{
#if defined(USE_SEPERATE_PGL)
	PP_RECT_S staticRect;
	PP_RSC_INFO_S *pStaticPgl = PP_NULL;
	PP_DU_LAYER_E staticLayer = eLayer_MAX;
	PP_DU_AREA_E staticArea = eArea_MAX;
#endif
	PP_RECT_S dynamicRect;
	PP_RSC_INFO_S *pDynamicPgl = PP_NULL;;
	PP_RSC_INFO_S *pDynamicBasePgl = PP_NULL;
	PP_DU_LAYER_E dynamicLayer = eLayer_MAX;
	PP_DU_AREA_E dynamicArea = eArea_MAX;
	PP_RESULT_E result;

	if (gPglSection == PP_NULL)
		return eERROR_FAILURE;

	// Top2d static PGL
#if defined(USE_SEPERATE_PGL)
	if (isBw)
		pStaticPgl = gPglSection[sectionID].bwStatic;
	else
		pStaticPgl = gPglSection[sectionID].fwStatic;

	if (pStaticPgl)
	{
		if (pStaticPgl->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_2D)
		{
			staticLayer = staticPgl2d.layer;
			staticArea = staticPgl2d.area;
		}
		else if ( (pStaticPgl->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT) || (pStaticPgl->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_SD) )
		{
			staticLayer = staticPglRearCam.layer;
			staticArea = staticPglRearCam.area;
		}

		if ( (staticLayer != eLayer_MAX) && (staticArea != eArea_MAX) )
		{
			staticRect.u16X = pStaticPgl->x;
			staticRect.u16Y = pStaticPgl->y;
			staticRect.u16Width = pStaticPgl->width;
			staticRect.u16Height = pStaticPgl->height;
			
			result = PPDRV_DU_OSD_SetAreaConfig(staticLayer, staticArea, pStaticPgl->buf, PP_NULL, pStaticPgl->size, staticRect, PP_NULL, pStaticPgl->format);
			if (result == eSUCCESS)
				PPDRV_DU_OSD_EnableArea(staticLayer, staticArea, PP_TRUE);
			else
				PPDRV_DU_OSD_EnableArea(staticLayer, staticArea, PP_FALSE);
		}
	}
#endif

	// Top2d dynamic PGL
	if(isBw)
	{
		pDynamicBasePgl = gPglSection[sectionID].bwDynamicNum ? gPglSection[sectionID].bwDynamic[gPglSection[sectionID].bwDynamicNum / 2] : PP_NULL;
	}
	else
	{
		pDynamicBasePgl = gPglSection[sectionID].fwDynamicNum ? gPglSection[sectionID].fwDynamic[gPglSection[sectionID].fwDynamicNum / 2] : PP_NULL;
	}
	pDynamicPgl = pDynamicBasePgl ? pDynamicBasePgl + gPglAngle : PP_NULL;

	if (pDynamicPgl)
	{
		if (pDynamicPgl->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_2D)
		{
			gPgl2dBase = pDynamicBasePgl;
			dynamicLayer = dynamicPgl2d.layer;
			dynamicArea = dynamicPgl2d.area;
		}
		else if ( (pDynamicPgl->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT) || (pDynamicPgl->viewType == PCV_SVM_VIEW_SUB_VIEW_TYPE_SD) )
		{
			gPglRearCamBase = pDynamicBasePgl;
			dynamicLayer = dynamicPglRearCam.layer;
			dynamicArea = dynamicPglRearCam.area;
		}

			
		if ( (dynamicLayer != eLayer_MAX) && (dynamicArea != eArea_MAX) )
		{
			dynamicRect.u16X = pDynamicPgl->x;
			dynamicRect.u16Y = pDynamicPgl->y;
			dynamicRect.u16Width = pDynamicPgl->width;
			dynamicRect.u16Height = pDynamicPgl->height;

			result = PPDRV_DU_OSD_SetAreaConfig(dynamicLayer, dynamicArea, pDynamicPgl->buf, PP_NULL, pDynamicPgl->size, dynamicRect, PP_NULL, pDynamicPgl->format);
			if(result == eSUCCESS)
				PPDRV_DU_OSD_EnableArea(dynamicLayer, dynamicArea, PP_TRUE);
			else
				PPDRV_DU_OSD_EnableArea(dynamicLayer, dynamicArea, PP_FALSE);
		}
	}
	
	return result;
}

PP_VOID PPAPI_DISPLAY_VIEW_PGL_SetAngle (PP_S16 IN angle)	// angle range : -35 ~ 35
{
	PP_RSC_INFO_S *top2dAngle;
	PP_RSC_INFO_S *rearAngle;

	// Update Top2d PGL
	top2dAngle = gPgl2dBase ? gPgl2dBase + angle : PP_NULL;
	if(top2dAngle && gPgl2dBase)
	{
		PPDRV_DU_OSD_SetArea(dynamicPgl2d.layer, dynamicPgl2d.area, top2dAngle->buf, PP_NULL, top2dAngle->size, (PP_U32)PP_NULL, top2dAngle->format);
	}

	// Update Rear PGL
	rearAngle = gPglRearCamBase ? gPglRearCamBase + angle : PP_NULL;
	if(rearAngle && gPglRearCamBase)
	{
		PPDRV_DU_OSD_SetArea(dynamicPglRearCam.layer, dynamicPglRearCam.area, rearAngle->buf, PP_NULL, rearAngle->size, (PP_U32)PP_NULL, rearAngle->format);
	}
}

PP_RESULT_E PPAPI_DISPLAY_VIEW_PGL_Off (PP_VOID)
{
	PP_RESULT_E result;

	// Disable Top2d PGL
#if defined(USE_SEPERATE_PGL)
	result = PPDRV_DU_OSD_EnableArea(staticPgl2d.layer, staticPgl2d.area, PP_FALSE);
#endif
	result = PPDRV_DU_OSD_EnableArea(dynamicPgl2d.layer, dynamicPgl2d.area, PP_FALSE);

	// Disable Rear PGL
#if defined(USE_SEPERATE_PGL)
	result = PPDRV_DU_OSD_EnableArea(staticPglRearCam.layer, staticPglRearCam.area, PP_FALSE);
#endif
	result = PPDRV_DU_OSD_EnableArea(dynamicPglRearCam.layer, dynamicPglRearCam.area, PP_FALSE);

	return result;
}


PP_RESULT_E PPAPI_DISPLAY_VIEW_Outline_On (PP_U32 IN mode, PP_U32 IN view)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E viewId, modeId;
	PP_U32 i;
	
	PPDRV_DU_OSD_SetMixerPath(ePATH_IN_32140_OUT);
	#if defined(USE_16BIT_CAR)
	PPDRV_DU_OSD_SetLayerFormat(eLayer2, eFORMAT_RGBA4444);
	#endif
	
	if(mode == eViewMode_TopCam)
	{
		switch(view)
		{
			case eViewMode_TopCam_Front:
				modeId = eOutline_mode_2d_RLE;
				viewId = eOutline_view_front_RLE;
				break;
			case eViewMode_TopCam_Left:
				modeId = eOutline_mode_2d_RLE;
				viewId = eOutline_view_left_RLE;
				break;
			case eViewMode_TopCam_Right:
				modeId = eOutline_mode_2d_RLE;
				viewId = eOutline_view_right_RLE;
				break;
			case eViewMode_TopCam_Rear:
				modeId = eOutline_mode_2d_RLE;
				viewId = eOutline_view_rear_RLE;
				break;
			case eViewMode_TopCam_WideFront:
			case eViewMode_TopCam_WideRear:
				modeId = eOutline_mode_wide_RLE;
				viewId = eOutline_view_wide_RLE;
				break;
			default:
				modeId = eOutline_mode_none_RLE;
				viewId = eOutline_view_none_RLE;
		}
		
		if(!gUiLoadingDone)
			modeId = eOutline_mode_init_RLE;
	}
	else if(mode == eViewMode_Top3D)
	{
		switch(view)
		{
//			case eViewMode_Top3D_Swing_0:
//				modeId = eOutline_mode_3d_swing0_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_1:
//				modeId = eOutline_mode_3d_swing1_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_2:
//				modeId = eOutline_mode_3d_swing2_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_3:
//				modeId = eOutline_mode_3d_swing3_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_4:
//				modeId = eOutline_mode_3d_swing4_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_5:
//				modeId = eOutline_mode_3d_swing5_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_6:
//				modeId = eOutline_mode_3d_swing6_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
//			case eViewMode_Top3D_Swing_7:
//				modeId = eOutline_mode_3d_swing7_RLE;
//				viewId = eOutline_view_none_RLE;
//				break;
			default:
				modeId = eOutline_mode_3d_RLE;
				viewId = eOutline_view_none_RLE;
				break;
		}
	}
	else
	{
		modeId = eOutline_mode_none_RLE;
		viewId = eOutline_view_none_RLE;
	}

	for(i=0 ; i<2 ; i++)
	{
		if(i == 0)		img = PPAPI_DISPLAY_GetUiImg(modeId);
		else if(i == 1)	img = PPAPI_DISPLAY_GetUiImg(viewId);
		else			img = PP_NULL;
		
		if(!img)
			return eERROR_FAILURE;
		
		rect.u16X = img->info->x;
		rect.u16Y = img->info->y;
		rect.u16Width = img->info->width;
		rect.u16Height = img->info->height;
		stride = img->info->width;
		result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
		if(result == eSUCCESS)
			PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
		else
			PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);
	}
	
	return result;
}

PP_RESULT_E PPAPI_DISPLAY_VIEW_Outline_Off (PP_VOID)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;

	PPDRV_DU_OSD_SetMixerPath(ePATH_IN_32140_OUT);
	#if defined(USE_16BIT_CAR)
	PPDRV_DU_OSD_SetLayerFormat(eLayer2, eFORMAT_RGBA4444);
	#endif
	
	img = PPAPI_DISPLAY_GetUiImg(eOutline_mode_none_RLE);
	if(!img)
		return eERROR_FAILURE;
	
	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	img = PPAPI_DISPLAY_GetUiImg(eOutline_view_none_RLE);
	if(!img)
		return eERROR_FAILURE;
	
	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);
	
	return result;
}

PP_RESULT_E PPAPI_DISPLAY_CALIB_DelPoint (PP_VOID)
{
	PP_U32  i;
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;
	PP_RESULT_E result;
	
	for(i=0 ; i<8 ; i++)
	{
		if(i < 4)	layer = eLayer2;
		else		layer = eLayer4;
		area = i % 4;
		
		result = PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);
	}

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_CALIB_Point (PP_POS_S* IN pnt, PP_U32 IN num)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_U32 i;
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;

	if(num > 8)	// maximum num : 8
		return eERROR_FAILURE;

	img = PPAPI_DISPLAY_GetUiImg(ePoint_RLE);
	if(!img)
		return eERROR_FAILURE;
	
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;

	for(i=0 ; i<num ; i++)
	{
		if(i < 4)	layer = eLayer2;
		else		layer = eLayer4;
		area = i % 4;
		rect.u16X = pnt[i].u16X - (img->info->width / 2);
		rect.u16Y = pnt[i].u16Y - (img->info->height / 2);
		result = PPDRV_DU_OSD_SetAreaConfig(layer, area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
		if(result == eSUCCESS)
			PPDRV_DU_OSD_EnableArea(layer, area, PP_TRUE);
		else
			PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);
	}
	
	return result;
}

PP_RESULT_E PPAPI_DISPLAY_CALIB_SelPoint (PP_POS_S* IN pnt, PP_U32 IN num, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;
	
	if((num > 8) || (idx >= num))	// maximum num : 8
		return eERROR_FAILURE;

	img = PPAPI_DISPLAY_GetUiImg(ePoint_Sel_RLE);
	if(!img)
		return eERROR_FAILURE;

	if(idx < 4)			layer = eLayer2;
	else if(idx < 8)	layer = eLayer4;
	else				return eERROR_INVALID_ARGUMENT;
	area = idx % 4;

	rect.u16X = pnt[idx].u16X - (img->info->width / 2);
	rect.u16Y = pnt[idx].u16Y - (img->info->height / 2);
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(layer, area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(layer, area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);
	
	return result;
}

PP_RESULT_E PPAPI_DISPLAY_CALIB_MovePoint (PP_POS_S* IN pnt, PP_U32 IN num, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;

	if((num > 8) || (idx >= num))	// maximum num : 8
		return eERROR_FAILURE;

	img = PPAPI_DISPLAY_GetUiImg(ePoint_Move_RLE);
	if(!img)
		return eERROR_FAILURE;

	if(idx < 4)			layer = eLayer2;
	else if(idx < 8)	layer = eLayer4;
	else				return eERROR_INVALID_ARGUMENT;
	area = idx % 4;
	
	rect.u16X = pnt[idx].u16X - (img->info->width / 2);
	rect.u16Y = pnt[idx].u16Y - (img->info->height / 2);
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(layer, area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(layer, area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);
	
	return result;
}

PP_RESULT_E PPAPI_DISPLAY_MenuList (PP_SCENE_E IN scene)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	if(scene == eScene_MainMenu)
		id = eMainMenu_RLE;
	else if(scene == eScene_VehicleSetting)
		id = eVehicle_Menu_RLE;
	else if(scene == eScene_TriggerSetting)
		id = eTrigger_Menu_RLE;
	else if(scene == eScene_VideoSetting)
		id = eVideo_Menu_RLE;
	else if(scene == eScene_CameraSetting)
		id = eCamera_Menu_RLE;
	else if(scene == eScene_CalibSetting)
		id = eCalibration_Menu_RLE;
	else if(scene == eScene_ManualCalib)
		id = eManualCalibration_Menu_RLE;
	else if(scene == eScene_UpdateSetting)
		id = eUpdate_Menu_RLE;
	else if(scene == eScene_SystemInfo)
		id = eSystemInfo_Menu_RLE;
	else
		return eERROR_INVALID_ARGUMENT;
		
	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_MenuItem (PP_SCENE_E IN scene, PP_U32 IN idx, PP_BOOL IN isSel)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	switch(scene)
	{
		case eScene_MainMenu:
			if(idx == eMenu_Vehicle)
			{
				id = eVehicle_Sel_RLE;
			}
			else if(idx == eMenu_Trigger)
			{
				id = eTrigger_Sel_RLE;
			}
			else if(idx == eMenu_Video)
			{
				id = eVideo_Sel_RLE;
			}
			else if(idx == eMenu_Camera)
			{
				id = eCamera_Sel_RLE;
			}
			else if(idx == eMenu_Update)
			{
				id = eUpdate_Sel_RLE;
			}
			else if(idx == eMenu_Calibration)
			{
				id = eCalibration_Sel_RLE;
			}
			else if(idx == eMenu_SystemInfo)
			{
				id = eSystemInfo_Sel_RLE;
			}
			else
			{
				id = eSystemInfo_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;

		case eScene_VehicleSetting:
			if(idx == eVehicle_Pgl)
			{
				id = ePgl_Sel_RLE;
			}
			else if(idx == eVehicle_DnmBlending)
			{
				id = eDnmBlending_Sel_RLE;
			}
			else if(idx == eVehicle_Back)
			{
				id = eVehicleBack_Sel_RLE;
			}
			else
			{
				id = eVehicleBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;
			
		case eScene_TriggerSetting:
			if(idx == eTrigger_ReverseSignal)
			{
				id = eReverseSignal_Sel_RLE;
			}
			else if(idx == eTrigger_ReverseView)
			{
				id = eReverseViewMode_Sel_RLE;
			}
			else if(idx == eTrigger_TurnSignal)
			{
				id = eTurnSignal_Sel_RLE;
			}
			else if(idx == eTrigger_TurnView)
			{
				id = eTurnViewMode_Sel_RLE;
			}
			else if(idx == eTrigger_EmergencySignal)
			{
				id = eEmergencySignal_Sel_RLE;
			}
			else if(idx == eTrigger_EmergencyView)
			{
				id = eEmergencyViewMode_Sel_RLE;
			}
			else if(idx == eTrigger_Back)
			{
				id = eTriggerBack_Sel_RLE;
			}
			else
			{
				id = eTriggerBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;
			
		case eScene_VideoSetting:
			if(idx == eVideo_ScreenX)
			{
				id = eScreenX_Sel_RLE;
			}
			else if(idx == eVideo_ScreenY)
			{
				id = eScreenY_Sel_RLE;
			}
			else if(idx == eVideo_Brightness)
			{
				id = eBrightness_Sel_RLE;
			}
			else if(idx == eVideo_Contras)
			{
				id = eContrast_Sel_RLE;
			}
			else if(idx == eVideo_Saturation)
			{
				id = eSaturation_Sel_RLE;
			}
			else if(idx == eVideo_Back)
			{
				id = eVideoBack_Sel_RLE;
			}
			else
			{
				id = eVideoBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;

		case eScene_CameraSetting:
			if(idx == eCamera_FrontPreview)
			{
				id = eCameraFront_Sel_RLE;
			}
			else if(idx == eCamera_RightPreview)
			{
				id = eCameraRight_Sel_RLE;
			}
			else if(idx == eCamera_LeftPreview)
			{
				id = eCameraLeft_Sel_RLE;
			}
			else if(idx == eCamera_RearPreview)
			{
				id = eCameraRear_Sel_RLE;
			}
			else if(idx == eCamera_QuadPreview)
			{
				id = eCameraQuad_Sel_RLE;
			}
			else if(idx == eCamera_Capture)
			{
				id = eCameraCapture_Sel_RLE;
			}
			else if(idx == eCamera_Back)
			{
				id = eCameraBack_Sel_RLE;
			}
			else
			{
				id = eCameraBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;

		case eScene_CalibSetting:
			if(idx == eCalib_Auto)
			{
				id = eCalibAuto_Sel_RLE;
			}
			else if(idx == eCalib_Manual)
			{
				id = eCalibManual_Sel_RLE;
			}
			else if(idx == eCalib_Back)
			{
				id = eCalibBack_Sel_RLE;
			}
			else
			{
				id = eCalibBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;

		case eScene_ManualCalib:
			if(idx == eManualCalib_Front)
			{
				id = eManualCalibFront_Sel_RLE;
			}
			else if(idx == eManualCalib_Right)
			{
				id = eManualCalibRight_Sel_RLE;
			}
			else if(idx == eManualCalib_Left)
			{
				id = eManualCalibLeft_Sel_RLE;
			}
			else if(idx == eManualCalib_Rear)
			{
				id = eManualCalibRear_Sel_RLE;
			}
			else if(idx == eManualCalib_ViewGen)
			{
				id = eManualCalibViewGen_Sel_RLE;
			}
			else if(idx == eManualCalib_Back)
			{
				id = eManualCalibBack_Sel_RLE;
			}
			else
			{
				id = eManualCalibBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;
		
		case eScene_UpdateSetting:
			if(idx == eUpdate_FactoryReset)
			{
				id = eFactoryReset_Sel_RLE;
			}
			else if(idx == eUpdate_FwUpdate)
			{
				id = eFwUpdate_Sel_RLE;
			}
			else if(idx == eUpdate_Back)
			{
				id = eUpdateBack_Sel_RLE;
			}
			else
			{
				id = eUpdateBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;

		case eScene_SystemInfo:
			if(idx == eSystemInfo_Back)
			{
				id = eInfoBack_Sel_RLE;
			}
			else
			{
				id = eInfoBack_Sel_RLE;
				isSel = PP_FALSE;
			}
			break;

		default:
			return eERROR_INVALID_ARGUMENT;
	}
	
	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	if(isSel)
	{
		rect.u16X = img->info->x;
		rect.u16Y = img->info->y;
		rect.u16Width = img->info->width;
		rect.u16Height = img->info->height;
		stride = img->info->width;
		result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
		if(result == eSUCCESS)
			PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
		else
			PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);
	}
	else
	{
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);
	}

	return result;
}

STATIC PP_RESULT_E PPAPI_DISPLAY_SubMenuList_Vehicle (PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	if(elem->id == eVehicle_Back)
		return eSUCCESS;
	if(elem->valueNum == 0xFFFFFFFF)
		return eSUCCESS;
	
	
	switch(elem->id)
	{
		case eVehicle_Pgl:
			if(*elem->value == ePgl_Off)
			{
				if(elem->id == idx)
					id = ePgl_Off_Sel_RLE;
				else
					id = ePgl_Off_RLE;
			}
			else if(*elem->value == ePgl_Rear)
			{
				if(elem->id == idx)
					id = ePgl_Rear_Sel_RLE;
				else
					id = ePgl_Rear_RLE;
			}
			else if(*elem->value == ePgl_Always)
			{
				if(elem->id == idx)
					id = ePgl_Always_Sel_RLE;
				else
					id = ePgl_Always_RLE;
			}
			break;

		case eVehicle_DnmBlending:
			#if 0	// 2018.11.23 Prevent function
			if(*elem->value == eDnmBlending_Off)
			{
				if(elem->id == idx)
					id = eDnmBlending_Off_Sel_RLE;
				else
					id = eDnmBlending_Off_RLE;
			}
			else if(*elem->value == eDnmBlending_On)
			{
				if(elem->id == idx)
					id = eDnmBlending_On_Sel_RLE;
				else
					id = eDnmBlending_On_RLE;
			}
			#else
			id = eDnmBlending_Off_RLE;
			#endif
			break;
	}

	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	elem++;
	return PPAPI_DISPLAY_SubMenuList_Vehicle(elem, idx);
}


STATIC PP_RESULT_E PPAPI_DISPLAY_SubMenuList_ManualCalib (PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;
	
	if(*elem[idx].value == eManualCalib_View_Mirror)
		id = eMirror_RLE;
	else
		id = eNormal_RLE;

	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	if(elem[idx].valueNum == 0xFFFFFFFF)
	{
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);
		return eSUCCESS;
	}
	
	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

STATIC PP_RESULT_E PPAPI_DISPLAY_SubMenuList_SystemInfo (PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	if(elem->id == eSystemInfo_Back)
		return eSUCCESS;
	if(elem->valueNum == 0xFFFFFFFF)
		return eSUCCESS;

	switch(elem->id)
	{
		case eSystemInfo_FWVer:		id = eFwVer_RLE; 	break;
		case eSystemInfo_LibVer:	id = eLibVer_RLE;	break;
		case eSystemInfo_ISPVer:	id = eIspVer_RLE;	break;
		case eSystemInfo_SencorID:	id = eSensorId_RLE;	break;
	}

	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	elem++;
	return PPAPI_DISPLAY_SubMenuList_SystemInfo(elem, idx);
}

STATIC PP_RESULT_E PPAPI_DISPLAY_SubMenuList_Trigger (PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;

	if(elem->id == eTrigger_Back)
		return eSUCCESS;
	if(elem->valueNum == 0xFFFFFFFF)
		return eSUCCESS;

	
	switch(elem->id)
	{
		case eTrigger_ReverseSignal:
		case eTrigger_TurnSignal:
		case eTrigger_EmergencySignal:
			if(*elem->value == eSignal_On)
			{
				if(elem->id == idx)
					id = eTriggerSignal_On_Sel_RLE;
				else
					id = eTriggerSignal_On_RLE;
			}
			else	// eSignal_Off
			{
				if(elem->id == idx)
					id = eTriggerSignal_Off_Sel_RLE;
				else
					id = eTriggerSignal_Off_RLE;
			}
			break;
		case eTrigger_ReverseView:
			if(*elem->value == eReverse_View_Top3DRear)
			{
				if(elem->id == idx)
					id = eTriggerView_t3db_Sel_RLE;
				else
					id = eTriggerView_t3db_RLE;
			}
			else	// eReverse_View_TopRear
			{
				if(elem->id == idx)
					id = eTriggerView_tb_Sel_RLE;
				else
					id = eTriggerView_tb_RLE;
			}
			break;
		case eTrigger_TurnView:
			if(*elem->value == eTurn_View_Top3DLeftFront)
			{
				if(elem->id == idx)
					id = eTriggerView_t3dlf_Sel_RLE;
				else
					id = eTriggerView_t3dlf_RLE;
			}
			else if(*elem->value == eTurn_View_Top3DRightFront)
			{
				if(elem->id == idx)
					id = eTriggerView_t3drf_Sel_RLE;
				else
					id = eTriggerView_t3drf_RLE;
			}
			else if(*elem->value == eTurn_View_Top3DLeft)
			{
				if(elem->id == idx)
					id = eTriggerView_t3dl_Sel_RLE;
				else
					id = eTriggerView_t3dl_RLE;
			}
			else	// eTurn_view_Top3DRight
			{
				if(elem->id == idx)
					id = eTriggerView_t3dr_Sel_RLE;
				else
					id = eTriggerView_t3dr_RLE;
			}
			break;
		case eTrigger_EmergencyView:
			if(*elem->value == eEmergency_View_Top3DFront)
			{
				if(elem->id == idx)
					id = eTriggerView_t3df_Sel_RLE;
				else
					id = eTriggerView_t3df_RLE;
			}
			else if(*elem->value == eEmergency_View_TopFront)
			{
				if(elem->id == idx)
					id = eTriggerView_tf_Sel_RLE;
				else
					id = eTriggerView_tf_RLE;
			}
			else if(*elem->value == eEmergency_View_Top3DLeftFront)
			{
				if(elem->id == idx)
					id = eTriggerView_t3dlf_Sel_RLE;
				else
					id = eTriggerView_t3dlf_RLE;
			}
			else	// eEmergency_View_Top3DRightFront
			{
				if(elem->id == idx)
					id = eTriggerView_t3drf_Sel_RLE;
				else
					id = eTriggerView_t3drf_RLE;
			}
			break;
	}

	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	if(elem->id < 3)	layer = eLayer1;
	else				layer = eLayer2;
	area = (elem->id + 1) % 4;
	
	rect.u16X = img->info->x;
	rect.u16Y = img->info->y + (elem->id * 71);;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(layer, area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(layer, area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);

	elem++;
	return PPAPI_DISPLAY_SubMenuList_Trigger(elem, idx);
}

STATIC PP_RESULT_E PPAPI_DISPLAY_SubMenuList_Video (PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;
	PP_DU_LAYER_E layer;
	PP_DU_AREA_E area;

	if(elem->id == eVideo_Back)
		return eSUCCESS;
	if(elem->valueNum == 0xFFFFFFFF)
		return eSUCCESS;

	
	switch(elem->id)
	{
		case eVideo_ScreenX:
		case eVideo_ScreenY:
		case eVideo_Brightness:
		case eVideo_Contras:
		case eVideo_Saturation:
			if(*elem->value >= eStepBox_Max)
			{
				if(elem->id == idx)
					id = eStep_Sel_10_RLE;
				else
					id = eStep_10_RLE;
			}
			else
			{
				if(elem->id == idx)
					id = eStep_Sel_0_RLE + *elem->value;
				else
					id = eStep_0_RLE + *elem->value;
			}
			break;
	}

	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	if(elem->id < 3)	layer = eLayer1;
	else				layer = eLayer2;
	area = (elem->id + 1) % 4;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y + (elem->id * 71);
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(layer, area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(layer, area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(layer, area, PP_FALSE);

	elem++;
	return PPAPI_DISPLAY_SubMenuList_Video(elem, idx);
}


PP_RESULT_E PPAPI_DISPLAY_SubMenuList (PP_SCENE_E IN scene, PP_SCENE_SUB_ELEM_S* IN elem, PP_U32 IN idx)
{
	PP_RESULT_E result;

	switch(scene)
	{
		case eScene_VehicleSetting:
			result = PPAPI_DISPLAY_SubMenuList_Vehicle(elem, idx);
			break;
		case eScene_TriggerSetting:
			result = PPAPI_DISPLAY_SubMenuList_Trigger(elem, idx);
			break;
		case eScene_VideoSetting:
			result = PPAPI_DISPLAY_SubMenuList_Video(elem, idx);
			break;
		case eScene_ManualCalib:
			result = PPAPI_DISPLAY_SubMenuList_ManualCalib(elem, idx);
			break;
		case eScene_SystemInfo:
			result = PPAPI_DISPLAY_SubMenuList_SystemInfo(elem, idx);
			break;
		default:
			return eERROR_INVALID_ARGUMENT;
	}

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_DIALOG_On (PP_DIALOG_BOX_E IN box, PP_DIALOG_BTN_E IN btn)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	// Display Dialog Box
	switch(box)
	{
		case eDialog_ManualCalib:
			id = eDialog_CalibManual_RLE;
			break;
		case eDialog_SaveView:
			id = eDialog_SaveView_RLE;
			break;
		case eDialog_RunMenu:
			id = eDialog_RunMenu_RLE;
			break;
		case eDialog_CalibFailRetry:
			id = eDialog_CalibFailRetry_RLE;
			break;
		case eDialog_Update:
			id = eDialog_Update_RLE;
			break;
		default:
			return eERROR_INVALID_ARGUMENT;
	}
	
	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);


	// Display Dialog Box Button
	if(btn == eDialog_Btn_Ok)
		id = eBtn_OK_Sel_RLE;
	else if(btn == eDialog_Btn_Cancel)
		id = eBtn_Cancel_Sel_RLE;
	else
		return eERROR_INVALID_ARGUMENT;
	
	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X += (btn == eDialog_Btn_Ok) ? 9 : 152;
	rect.u16Y += 94;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_DIALOG_Off (PP_VOID)
{
	PP_RESULT_E result;
	
	result = PPDRV_DU_OSD_EnableArea(eLayer0, eArea2, PP_FALSE);
	if(result != eSUCCESS)	return eERROR_FAILURE;
	result = PPDRV_DU_OSD_EnableArea(eLayer1, eArea1, PP_FALSE);
	if(result != eSUCCESS)	return eERROR_FAILURE;

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_POPUP_On (PP_POPUP_E IN msg)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	switch(msg)
	{
		case ePopUp_DoNotOff:
			id = ePopUp_DoNotTurnOff_RLE;
			break;
		case ePopUp_SaveDone:
			id = ePopUp_Saved_RLE;
			break;
		case ePopUp_NoSaveDone:
			id = ePopUp_NoSaved_RLE;
			break;
		case ePopUp_NoInput:
			id = ePopUp_NoCameraInput_RLE;
			break;
		case ePopUp_NoSDCard:
			id = ePopUp_NoSDCard_RLE;
			break;
		case ePopUp_ShutDown:
			id = ePopUp_Shutdown_RLE;
			break;
		case ePopUp_Calibration:
			id = ePopUp_Calibration_RLE;
			break;
		case ePopUp_ViewGen:
			id = ePopUp_ViewGen_RLE;
			break;
		case ePopUp_Done:
			id = ePopUp_Done_RLE;
			break;
		case ePopUp_Failed:
			id = ePopup_Failed_RLE;
			break;
		case ePopUp_NoUpdate:
			id = ePopup_NoUpdate_RLE;
			break;
		case ePopUp_TBD:
			id = ePopUp_TBD_RLE;
			break;
		default:
			return eERROR_INVALID_ARGUMENT;
	}

	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_POPUP_Off (PP_VOID)
{
	PP_RESULT_E result;
	result = PPDRV_DU_OSD_EnableArea(eLayer0, eArea0, PP_FALSE);
	return result;
}

PP_RESULT_E PPAPI_DISPLAY_PROGRESSBAR_On (PP_U32 IN level, PP_U32 IN flicker)
{
	PP_RSC_UI_IMG_S *img;
	PP_RESULT_E result;
	PP_RECT_S rect;
	PP_U32 stride;
	PP_UI_RSCLIST_E id;

	if( (level > 10) || (flicker > 1) )
		return eERROR_INVALID_ARGUMENT;

	if(level == 0)
		id = eProgress_0_RLE;
	else
		id = (flicker==0) ? eProgress_10_RLE+(level-1) : eProgress1_10_RLE+(level-1);
	
	img = PPAPI_DISPLAY_GetUiImg(id);
	if(!img)
		return eERROR_FAILURE;

	rect.u16X = img->info->x;
	rect.u16Y = img->info->y;
	rect.u16Width = img->info->width;
	rect.u16Height = img->info->height;
	stride = img->info->width;
	result = PPDRV_DU_OSD_SetAreaConfig(img->layer, img->area, img->info->buf, PP_NULL, img->info->size, rect, stride, img->info->format);
	if(result == eSUCCESS)
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_TRUE);
	else
		PPDRV_DU_OSD_EnableArea(img->layer, img->area, PP_FALSE);

	return result;
}

PP_RESULT_E PPAPI_DISPLAY_PROGRESSBAR_Off (PP_VOID)
{
	PP_RESULT_E result;
	result = PPDRV_DU_OSD_EnableArea(eLayer0, eArea1, PP_FALSE);
	return result;
}
#endif	// USE_PP_GUI

PP_VOID PPAPI_DISPLAY_VPU_Init (PP_RECT_S IN rect, PP_U32* IN buf)
{
	static uint32 clut[256] = {0x00000000, 0xFFFF00FF,0xFFFF0000, 0xFF00FF00, 0xFF0000FF, 0xFFFFFF00, 0xFFFF00FF, 0xFF00FFFF, 0xFFFFFFFF };
	PP_U32 stride = ((rect.u16Width/ 8) + 15) &0xFFFFFFF0; // 16byte align
	PP_U32 byte = (rect.u16Width * rect.u16Height) / 8;
	
	PPDRV_DU_OSD_SetLayerColor(eLayer4, 0x00000000);
	PPDRV_DU_OSD_SetLayerFormat(eLayer4, eFORMAT_1BIT);
	PPDRV_DU_OSD_SetColorLut(eLayer4, (PP_U32*)&clut);
	PPDRV_DU_OSD_SetAreaConfig(eLayer4, eArea0, buf, PP_NULL, byte, rect, stride, eFORMAT_1BIT);
}

PP_VOID PPAPI_DISPLAY_VPU_Screen (PP_BOOL IN enable)
{
	PPDRV_DU_OSD_EnableArea(eLayer4, eArea0, enable);
}

PP_VOID PPAPI_DISPLAY_VPU_Deinit (PP_VOID)
{
	PPDRV_DU_OSD_EnableArea(eLayer4, eArea0, PP_FALSE);
	PPDRV_DU_OSD_SetLayerColor(eLayer4, 0x00FF00FF);
	PPDRV_DU_OSD_SetLayerFormat(eLayer4, eFORMAT_RLE);
}

PP_RESULT_E PPAPI_DISPlAY_Initialize (PP_VOID)
{
	typedef struct ppLayerCfg_S {
		PP_U32 format;
		PP_U32 color;
	} PP_LayerCfg_S;
	PP_LayerCfg_S config[eLayer_MAX] =	{
										{	eFORMAT_RLE,	0x00FF00FF	},
										{	eFORMAT_RLE,	0x00FF00FF	},
										{	eFORMAT_RLE,	0x00FF00FF	},
										{	eFORMAT_RLE, 	0x00FF00FF	},
										{	eFORMAT_RLE,	0x00FF00FF	}
									};
	PP_DU_RESOLUTION_E in_resolution;
	PP_DU_RESOLUTION_E out_resolution;
	PP_U32 idx;
	PP_RECT_S rect;

	PP_S32 vinWidth, vinHeight;
	_VID_RESOL eVinResol;
	PP_RESULT_E result = eSUCCESS;

    if( PPAPI_VIN_GetResol(BD_DU_IN_FMT, &vinWidth, &vinHeight, &eVinResol) != eSUCCESS )
    {
        LOG_WARNING("[(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }
	PPDRV_DU_SetVideoPath(ePATH_SVMOUT_MIXER_OUT);
	PPDRV_DU_OSD_SetMixerPath(ePATH_IN_43210_OUT);
	rect.u16X = 0;
	rect.u16Y = 0;
	rect.u16Width = vinWidth;
	rect.u16Height = vinHeight;

	for(idx=0; idx<eLayer_MAX ; idx++)
	{
		PPDRV_DU_OSD_SetLayerSize(idx, rect);
		PPDRV_DU_OSD_SetLayerFormat(idx, config[idx].format);
		PPDRV_DU_OSD_SetLayerColor(idx, config[idx].color);
		PPDRV_DU_OSD_SetLayerGlobalAlpha(idx, 0xFF);
		PPDRV_DU_OSD_EnableLayerGlobalAlpha(idx, FALSE);
		
		if( ((BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H) )
		{
			PPDRV_DU_OSD_EnableLayer2dDMA(idx, TRUE);
			PPDRV_DU_OSD_EnableLayerInterlace(idx, TRUE);
		}
	}

	PPDRV_DU_BTO_SetFreeze(rect);

	PPDRV_DU_OSD_RunMixer();
//	PPDRV_DU_SetInterrupt(du_isr_handler);	// for debug

#if defined(USE_PP_GUI) && defined(USE_BOOTING_IMG)
	PPAPI_DISPLAY_ParsingImage(eScene_Booting);
	PPAPI_DISPlAY_UpdateLUT();
	PPAPI_DISPLAY_Background_On();
	PPAPI_DISPLAY_BOOTING_CI();
#endif

	in_resolution = eRESOLUTION_MAX;
	if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H )
	{
		if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30 )
		{
			in_resolution = eRESOLUTION_720Hi_NTSC;
		}
		else if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25 )
		{
			in_resolution = eRESOLUTION_720Hi_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H )
	{
		if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30 )
		{
			in_resolution = eRESOLUTION_960Hi_NTSC;
		}
		else if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25 )
		{
			in_resolution = eRESOLUTION_960Hi_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH720 )
	{
		if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60 )
		{
			in_resolution = eRESOLUTION_720Hp_NTSC;
		}
		else if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50 )
		{
			in_resolution = eRESOLUTION_720Hp_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH960 )
	{
		if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60 )
		{
			in_resolution = eRESOLUTION_960Hp_NTSC;
		}
		else if( (BD_DU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50 )
		{
			in_resolution = eRESOLUTION_960Hp_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD800_480P )
	{
		in_resolution = eRESOLUTION_WVGA;
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1024_600P )
	{
		in_resolution = eRESOLUTION_WSVGA;
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P )
	{
		in_resolution = eRESOLUTION_720P;
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P )
	{
		in_resolution = eRESOLUTION_960P;
	}
	else if( (BD_DU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P )
	{
		in_resolution = eRESOLUTION_1080P;
	}
	else
	{
		LOG_DEBUG("resolution is invalid!\n");
		result = eERROR_FAILURE;
	}

	out_resolution = eRESOLUTION_MAX;
	if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H )
	{
		if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30 )
		{
			out_resolution = eRESOLUTION_720Hi_NTSC;
		}
		else if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25 )
		{
			out_resolution = eRESOLUTION_720Hi_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H )
	{
		if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30 )
		{
			out_resolution = eRESOLUTION_960Hi_NTSC;
		}
		else if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25 )
		{
			out_resolution = eRESOLUTION_960Hi_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH720 )
	{
		if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60 )
		{
			out_resolution = eRESOLUTION_720Hp_NTSC;
		}
		else if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50 )
		{
			out_resolution = eRESOLUTION_720Hp_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH960 )
	{
		if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_60 )
		{
			out_resolution = eRESOLUTION_960Hp_NTSC;
		}
		else if( (BD_DU_OUT_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_50 )
		{
			out_resolution = eRESOLUTION_960Hp_PAL;
		}
		else
		{
			LOG_DEBUG("resolution is invalid!\n");
			result = eERROR_FAILURE;
		}
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD800_480P )
	{
		out_resolution = eRESOLUTION_WVGA;
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1024_600P )
	{
		out_resolution = eRESOLUTION_WSVGA;
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P )
	{
		out_resolution = eRESOLUTION_720P;
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P )
	{
		out_resolution = eRESOLUTION_960P;
	}
	else if( (BD_DU_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P )
	{
		out_resolution = eRESOLUTION_1080P;
	}
	else
	{
		LOG_DEBUG("resolution is invalid!\n");
		result = eERROR_FAILURE;
	}

	if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB )
	{
		PPDRV_DU_BTO_SetYC8(in_resolution, out_resolution, eSYNC_EMBEDDED);
	}
	else if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT )
	{
		PPDRV_DU_BTO_SetYC8(in_resolution, out_resolution, eSYNC_EXTERNAL);
	}
	else if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC16_EMB )
	{
		PPDRV_DU_BTO_SetYC16(out_resolution, eSYNC_EMBEDDED);
	}
	else if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC16_EXT )
	{
		PPDRV_DU_BTO_SetYC16(out_resolution, eSYNC_EXTERNAL);
	}
	else if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_RGB24 )
	{
		PPDRV_DU_BTO_SetRGB24(out_resolution);
	}
	else if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_BAYER_8BIT )
	{
		PPDRV_DU_BTO_SetBayer(out_resolution, eBAYER_8BIT);
	}
	else if( (BD_DU_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_BAYER_10BIT )
	{
		PPDRV_DU_BTO_SetBayer(out_resolution, eBAYER_10BIT);
	}
	else
	{
		LOG_DEBUG("OUTPUT FORMAT IS INVALID!\n");
		result = eERROR_FAILURE;
	}

    if( (BD_PVITX_OUT_FMT & PVITX_SRC_MASKBIT) != PVITX_SRC_NONE )
    {
        if( (BD_PVITX_OUT_FMT & PVITX_SRC_MASKBIT) == PVITX_SRC_DU )
        {
            PPDRV_DU_PVITX_SelSrc(ePVITX_SRC_DU);
        }
        else if( (BD_PVITX_OUT_FMT & PVITX_SRC_MASKBIT) == PVITX_SRC_QUAD )
        {
            PPDRV_DU_PVITX_SelSrc(ePVITX_SRC_QUAD);
        }
        else
        {
            LOG_DEBUG("PVITX SOURCE IS INVALID!\n");
            result = eERROR_FAILURE;
        }
    }

    PPDRV_SYSTEM_SetVOPadIO(1); //0:input, 1:output

	return result;
}

