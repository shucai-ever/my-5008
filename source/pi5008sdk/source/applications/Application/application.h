#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "type.h"
#include "error.h"
#include "viewmode_config.h"

#ifdef __cplusplus
EXTERN "C" {
#endif


#if defined(USE_PP_GUI)

//---------------------------------------------------------------------------
// Define

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	Enumeration

// scene list
typedef enum ppSCENE_E {
	eScene_Booting,
	eScene_LiveView,
	eScene_MainMenu,
	eScene_VehicleSetting,
	eScene_TriggerSetting,
	eScene_VideoSetting,
	eScene_CameraSetting,
	eScene_CameraPreview,
	eScene_CameraCapture,
	eScene_CalibSetting,
	eScene_ManualCalib,
	eScene_MovePoint,
	eScene_ViewGen,
	eScene_UpdateSetting,
	eScene_SystemInfo,
	eScene_Dialog,

	eScene_Max
} PP_SCENE_E;

// live view mode
typedef enum ppVIEW_MODE_E {
	eViewMode_TopCam,
	eViewMode_Top3D,

	eViewMode_Max
} PP_VIEW_MODE_E;

typedef enum ppVIEW_MODE_TOPCAM_E {
	eViewMode_TopCam_Front,
	eViewMode_TopCam_Left,
	eViewMode_TopCam_Right,
	eViewMode_TopCam_Rear,
	eViewMode_TopCam_WideFront,
	eViewMode_TopCam_WideRear,
	
	eViewMode_TopCam_Max
} PP_VIEW_MODE_TOPCAM_E;

typedef enum ppVIEW_MODE_TOP3D_8ANGLE_E{
	eViewMode_Top3D_Swing_8Angle_0,
	eViewMode_Top3D_Swing_8Angle_1,
	eViewMode_Top3D_Swing_8Angle_2,
	eViewMode_Top3D_Swing_8Angle_3,
	eViewMode_Top3D_Swing_8Angle_4,
	eViewMode_Top3D_Swing_8Angle_5,
	eViewMode_Top3D_Swing_8Angle_6,
	eViewMode_Top3D_Swing_8Angle_7,

	eViewMode_Top3D_8Angle_Max

}PP_VIEW_MODE_TOP3D_8ANGLE_E;


typedef enum ppVIEW_MODE_TOP3D_E {

	eViewMode_Top3D_Swing_0,
	eViewMode_Top3D_Swing_1,
	eViewMode_Top3D_Swing_2,
	eViewMode_Top3D_Swing_3,
	eViewMode_Top3D_Swing_4,
	eViewMode_Top3D_Swing_5,
	eViewMode_Top3D_Swing_6,
	eViewMode_Top3D_Swing_7,
	eViewMode_Top3D_Swing_8,
	eViewMode_Top3D_Swing_9,
	eViewMode_Top3D_Swing_10,
	eViewMode_Top3D_Swing_11,
	eViewMode_Top3D_Swing_12,
	eViewMode_Top3D_Swing_13,
	eViewMode_Top3D_Swing_14,
	eViewMode_Top3D_Swing_15,
	eViewMode_Top3D_Swing_16,
	eViewMode_Top3D_Swing_17,
	eViewMode_Top3D_Swing_18,
	eViewMode_Top3D_Swing_19,
	eViewMode_Top3D_Swing_20,
	eViewMode_Top3D_Swing_21,
	eViewMode_Top3D_Swing_22,
	eViewMode_Top3D_Swing_23,
	eViewMode_Top3D_Swing_24,
	eViewMode_Top3D_Swing_25,
	eViewMode_Top3D_Swing_26,
	eViewMode_Top3D_Swing_27,
	eViewMode_Top3D_Swing_28,
	eViewMode_Top3D_Swing_29,
	eViewMode_Top3D_Swing_30,
	eViewMode_Top3D_Swing_31,
	eViewMode_Top3D_Swing_32,
	eViewMode_Top3D_Swing_33,
	eViewMode_Top3D_Swing_34,
	eViewMode_Top3D_Swing_35,
	eViewMode_Top3D_Swing_36,
	eViewMode_Top3D_Swing_37,
	eViewMode_Top3D_Swing_38,
	eViewMode_Top3D_Swing_39,
	eViewMode_Top3D_Swing_40,
	eViewMode_Top3D_Swing_41,
	eViewMode_Top3D_Swing_42,
	eViewMode_Top3D_Swing_43,
	eViewMode_Top3D_Swing_44,
	eViewMode_Top3D_Swing_45,
	eViewMode_Top3D_Swing_46,
	eViewMode_Top3D_Swing_47,
	eViewMode_Top3D_Swing_48,
	eViewMode_Top3D_Swing_49,
	eViewMode_Top3D_Swing_50,
	eViewMode_Top3D_Swing_51,
	eViewMode_Top3D_Swing_52,
	eViewMode_Top3D_Swing_53,
	eViewMode_Top3D_Swing_54,
	eViewMode_Top3D_Swing_55,
	eViewMode_Top3D_Swing_56,
	eViewMode_Top3D_Swing_57,
	eViewMode_Top3D_Swing_58,
	eViewMode_Top3D_Swing_59,
	eViewMode_Top3D_Swing_60,
	eViewMode_Top3D_Swing_61,
	eViewMode_Top3D_Swing_62,
	eViewMode_Top3D_Swing_63,
	eViewMode_Top3D_Swing_64,
	eViewMode_Top3D_Swing_65,
	eViewMode_Top3D_Swing_66,
	eViewMode_Top3D_Swing_67,
	eViewMode_Top3D_Swing_68,
	eViewMode_Top3D_Swing_69,
	eViewMode_Top3D_Swing_70,
	eViewMode_Top3D_Swing_71,
	eViewMode_Top3D_Swing_72,
	eViewMode_Top3D_Swing_73,
	eViewMode_Top3D_Swing_74,
	eViewMode_Top3D_Swing_75,
	eViewMode_Top3D_Swing_76,
	eViewMode_Top3D_Swing_77,
	eViewMode_Top3D_Swing_78,
	eViewMode_Top3D_Swing_79,
	eViewMode_Top3D_Swing_80,
	eViewMode_Top3D_Swing_81,
	eViewMode_Top3D_Swing_82,
	eViewMode_Top3D_Swing_83,
	eViewMode_Top3D_Swing_84,
	eViewMode_Top3D_Swing_85,
	eViewMode_Top3D_Swing_86,
	eViewMode_Top3D_Swing_87,
	eViewMode_Top3D_Swing_88,
	eViewMode_Top3D_Swing_89,
	eViewMode_Top3D_Swing_90,
	eViewMode_Top3D_Swing_91,
	eViewMode_Top3D_Swing_92,
	eViewMode_Top3D_Swing_93,
	eViewMode_Top3D_Swing_94,
	eViewMode_Top3D_Swing_95,
	eViewMode_Top3D_Swing_96,
	eViewMode_Top3D_Swing_97,
	eViewMode_Top3D_Swing_98,
	eViewMode_Top3D_Swing_99,
	eViewMode_Top3D_Swing_100,
	eViewMode_Top3D_Swing_101,
	eViewMode_Top3D_Swing_102,
	eViewMode_Top3D_Swing_103,
	eViewMode_Top3D_Swing_104,
	eViewMode_Top3D_Swing_105,
	eViewMode_Top3D_Swing_106,
	eViewMode_Top3D_Swing_107,
	eViewMode_Top3D_Swing_108,
	eViewMode_Top3D_Swing_109,
	eViewMode_Top3D_Swing_110,
	eViewMode_Top3D_Swing_111,
	eViewMode_Top3D_Swing_112,
	eViewMode_Top3D_Swing_113,
	eViewMode_Top3D_Swing_114,
	eViewMode_Top3D_Swing_115,
	eViewMode_Top3D_Swing_116,
	eViewMode_Top3D_Swing_117,
	eViewMode_Top3D_Swing_118,
	eViewMode_Top3D_Swing_119,
	eViewMode_Top3D_Swing_120,
	eViewMode_Top3D_Swing_121,
	eViewMode_Top3D_Swing_122,
	eViewMode_Top3D_Swing_123,
	eViewMode_Top3D_Swing_124,
	eViewMode_Top3D_Swing_125,
	eViewMode_Top3D_Swing_126,
	eViewMode_Top3D_Swing_127,
	eViewMode_Top3D_Swing_128,
	eViewMode_Top3D_Swing_129,
	eViewMode_Top3D_Swing_130,
	eViewMode_Top3D_Swing_131,
	eViewMode_Top3D_Swing_132,
	eViewMode_Top3D_Swing_133,
	eViewMode_Top3D_Swing_134,
	eViewMode_Top3D_Swing_135,
	eViewMode_Top3D_Swing_136,
	eViewMode_Top3D_Swing_137,
	eViewMode_Top3D_Swing_138,
	eViewMode_Top3D_Swing_139,
	eViewMode_Top3D_Swing_140,
	eViewMode_Top3D_Swing_141,
	eViewMode_Top3D_Swing_142,
	eViewMode_Top3D_Swing_143,
	eViewMode_Top3D_Swing_144,
	eViewMode_Top3D_Swing_145,
	eViewMode_Top3D_Swing_146,
	eViewMode_Top3D_Swing_147,
	eViewMode_Top3D_Swing_148,
	eViewMode_Top3D_Swing_149,
	eViewMode_Top3D_Swing_150,
	eViewMode_Top3D_Swing_151,
	eViewMode_Top3D_Swing_152,
	eViewMode_Top3D_Swing_153,
	eViewMode_Top3D_Swing_154,
	eViewMode_Top3D_Swing_155,
	eViewMode_Top3D_Swing_156,
	eViewMode_Top3D_Swing_157,
	eViewMode_Top3D_Swing_158,
	eViewMode_Top3D_Swing_159,
	eViewMode_Top3D_Swing_160,
	eViewMode_Top3D_Swing_161,
	eViewMode_Top3D_Swing_162,
	eViewMode_Top3D_Swing_163,
	eViewMode_Top3D_Swing_164,
	eViewMode_Top3D_Swing_165,
	eViewMode_Top3D_Swing_166,
	eViewMode_Top3D_Swing_167,
	eViewMode_Top3D_Swing_168,
	eViewMode_Top3D_Swing_169,
	eViewMode_Top3D_Swing_170,
	eViewMode_Top3D_Swing_171,
	eViewMode_Top3D_Swing_172,
	eViewMode_Top3D_Swing_173,
	eViewMode_Top3D_Swing_174,
	eViewMode_Top3D_Swing_175,
	eViewMode_Top3D_Swing_176,
	eViewMode_Top3D_Swing_177,
	eViewMode_Top3D_Swing_178,
	eViewMode_Top3D_Swing_179,

//	eViewMode_Top3D_Swing_0,
//	eViewMode_Top3D_Swing_1,
//	eViewMode_Top3D_Swing_2,
//	eViewMode_Top3D_Swing_3,
//	eViewMode_Top3D_Swing_4,
//	eViewMode_Top3D_Swing_5,
//	eViewMode_Top3D_Swing_6,
//	eViewMode_Top3D_Swing_7,
	
	eViewMode_Top3D_Max
} PP_VIEW_MODE_TOP3D_E;

// main menu list
typedef enum ppMENU_E {
	eMenu_Vehicle,
	eMenu_Trigger,
	eMenu_Video,
	eMenu_Camera,
	eMenu_Calibration,
	eMenu_Update,
	eMenu_SystemInfo,
	
	eMenu_Max
} PP_MENU_E;

// menu list (1st depth) : vehicle
typedef enum ppVEHICLE_SETTING_E {
	eVehicle_Pgl,
	eVehicle_DnmBlending,
	eVehicle_Back,
	
	eVehicle_Max
} PP_VEHICLE_SETTING_E;

// sub menu list (2nd depth) : pgl
typedef enum ppPGL_E {
	ePgl_Off,
	ePgl_Rear,
	ePgl_Always,

	ePgl_Max
} PP_PGL_E;

typedef enum ppDYNAMIC_BLEDNING_E {
	eDnmBlending_Off,
	//eDnmBlending_On,	// 2018.11.23 Prevent function

	eDnmBlending_Max
} PP_DYNAMIC_BLENDING_E;

// menu list (1st depth) : trigger
typedef enum ppTRIGGER_SETTING_E {
	eTrigger_ReverseSignal,
	eTrigger_ReverseView,
	eTrigger_TurnSignal,
	eTrigger_TurnView,
	eTrigger_EmergencySignal,
	eTrigger_EmergencyView,
	eTrigger_Back,

	eTrigger_Max
} PP_TRIGGER_SETTING_E;

// sub menu list (2nd depth) : trigger signal
typedef enum ppTRIGGER_SIGNAL_E {
	eSignal_Off,
	eSignal_On,
	
	eSignal_Max
} PP_TRIGGER_SIGNAL_E;

// sub menu list (2nd depth) : reverse view
typedef enum ppREVERSE_VIEW_E {
	eReverse_View_Top3DRear,
	eReverse_View_TopRear,
	
	eReverse_View_Max
} PP_REVERSE_VIEW_E;

// sub menu list (2nd depth) : turn view
typedef enum ppTURN_VIEW_E {
	eTurn_View_Top3DLeftFront,
	eTurn_View_Top3DRightFront,
	eTurn_View_Top3DLeft,
	eTurn_view_Top3DRight,
	
	eTurn_View_Max
} PP_TURN_VIEW_E;

// sub menu list (2nd depth) : emergency view
typedef enum ppEMERGENCY_VIEW_E {
	eEmergency_View_Top3DFront,
	eEmergency_View_TopFront,
	eEmergency_View_Top3DLeftFront,
	eEmergency_View_Top3DRightFront,
	
	eEmergency_View_Max
} PP_EMERGENCY_VIEW_E;

// menu list (1st depth) : video
typedef enum ppVIDEO_SETTING_E {
	eVideo_ScreenX,
	eVideo_ScreenY,
	eVideo_Brightness,
	eVideo_Contras,
	eVideo_Saturation,
	eVideo_Back,
	
	eVideo_Max
} PP_VIDEO_SETTING_E;

// sub mmenu list (2nd depth) : step box
typedef enum ppSTEPBOX_E {
	eStepBox_0,
	eStepBox_1,
	eStepBox_2,
	eStepBox_3,
	eStepBox_4,
	eStepBox_5,
	eStepBox_6,
	eStepBox_7,
	eStepBox_8,
	eStepBox_9,
	eStepBox_10,

	eStepBox_Max
} PP_STEP_BOX_E;

// menu list (1st depth) : camera
typedef enum ppCAMERA_SETTING_E {
	eCamera_FrontPreview,
	eCamera_RightPreview,
	eCamera_LeftPreview,
	eCamera_RearPreview,
	eCamera_QuadPreview,
	eCamera_Capture,
	eCamera_Back,
	
	eCamera_Max
} PP_CAMERA_SETTING_E;

// menu list (1st depth) : calibration
typedef enum ppCALIB_SETTING_E {
	eCalib_Auto,
	eCalib_Manual,
	eCalib_Back,
	
	eCalib_Max
} PP_CALIB_SETTING_E;

// menu list (2nd depth) : manual calibration
typedef enum ppMANUAL_CALIB_SETTING_E {
	eManualCalib_Front,
	eManualCalib_Right,
	eManualCalib_Left,
	eManualCalib_Rear,
	eManualCalib_ViewGen,
	eManualCalib_Back,
	
	eManualCalib_Max
} PP_MANUAL_CALIB_SETTING_E;

// sub menu list (2nd depth) : mirror
typedef enum ppMANUAL_CALIB_VIEW_E {
	eManualCalib_View_Normal,
	eManualCalib_View_Mirror,
	
	eManualCalib_View_Max
} PP_MANUAL_CALIB_VIEW_E;

// menu list (1st depth) : update
typedef enum ppUPDATE_MENU_E {
	eUpdate_FactoryReset,
	eUpdate_FwUpdate,
	eUpdate_Back,

	eUpdate_Max
} PP_UPDATE_MENU_E;

// menu list (1st depth) : system info
typedef enum ppSYSTEM_INFO_E {
	eSystemInfo_FWVer,
	eSystemInfo_LibVer,
	eSystemInfo_SencorID,
	eSystemInfo_ISPVer,
	eSystemInfo_Back,

	eSystemInfo_Max
} PP_SYSTEM_INFO_E;

// pop-up list
typedef enum ppPOPUP_E {
	ePopUp_DoNotOff,		// "Do not turn off the power"
	ePopUp_SaveDone,		// "Save Done"
	ePopUp_NoSaveDone,		// "Save Failed"
	ePopUp_NoInput,			// "No Camera Input"
	ePopUp_NoSDCard,		// "No SD Card"
	ePopUp_ShutDown,		// "System Shutdown"
	ePopUp_Calibration,		// "Calibration in Progress..."
	ePopUp_ViewGen,			// "View Generation in Progress..."
	ePopUp_Done,			// "Done!"
	ePopUp_Failed,			// "Failed!"
	ePopUp_NoUpdate,		// "FW Update Filed!"
	ePopUp_TBD,				// To Be Continues...
	//ePopUp_DataError,

	ePopUp_Max
} PP_POPUP_E;

// dialog list
typedef enum ppDIALOG_BOX_E {
	eDialog_ManualCalib,	// "Calibrate manually?"
	eDialog_SaveView,		// "Save view?"
	eDialog_CalibFailRetry,	// "Calibration failed. Retry?"
	eDialog_RunMenu,		// "Run the selected menu?"
	eDialog_Update,			// "Update FW?"

	eDialog_Max
} PP_DIALOG_BOX_E;

typedef enum ppDIALOG_BTN_E {
	eDialog_Btn_Ok,
	eDialog_Btn_Cancel,

	eDialog_Btn_Max
} PP_DIALOG_BTN_E;

typedef enum ppCAR_DOOR_E {
	eCar_OpenDoor,
	eCar_OpenDoor_LF,
	eCar_OpenDoor_LF_RF,
	eCar_OpenDoor_LF_RF_LR,
	eCar_OpenDoor_LF_RF_RR,
	eCar_OpenDoor_LF_RF_LR_RR,
	eCar_OpenDoor_LF_LR,
	eCar_OpenDoor_LF_LR_RR,
	eCar_OpenDoor_LF_RR,
	eCar_OpenDoor_RF,
	eCar_OpenDoor_RF_LR,
	eCar_OpenDoor_RF_LR_RR,
	eCar_OpenDoor_RF_RR,
	eCar_OpenDoor_LR,
	eCar_OpenDoor_LR_RR,
	eCar_OpenDoor_RR,
	
	eCar_OpenDoor_Max
} PP_CAR_DOOR_E;

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Struct
#pragma pack(1)
typedef struct ppSCENE_SUB_ELEM_S {
	PP_U32 id;
	PP_U32 valueNum;
	PP_U32 *value;
} PP_SCENE_SUB_ELEM_S;
#pragma pack()

#pragma pack(1)
typedef struct ppSCENE_ELEM_S {
	PP_SCENE_E id;
	PP_VOID (*uiFunc) (void *, PP_U32 IN event);
	PP_U32 selIdx;
	PP_U32 subNum;
	PP_SCENE_SUB_ELEM_S subElem[10];
} PP_SCENE_ELEM_S;
#pragma pack()

#pragma pack(1)
typedef struct ppSCENE_S {
    PP_SCENE_ELEM_S		*elem;
	struct ppSCENE_S	*prev;
    struct ppSCENE_S	*next[10];
} PP_SCENE_S;
#pragma pack()
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	Function
PP_SCENE_S *PPAPP_UI_Initialize (PP_VOID);
PP_SCENE_S *PPAPP_UI_GetScene (PP_SCENE_E IN idx);
PP_VOID PPAPP_UI_LiveView (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_MainMenu (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_VehicleSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_TriggerSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_VideoSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_CameraSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_CameraCapture (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_CameraPreview (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_CalibSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_ManualCalibSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_MovePoint (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_ViewGeneration (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_UpdateSetting (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_SystemInfo (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_UI_Dialog (void* IN pArg, PP_U32 IN event);
PP_VOID PPAPP_View_SetSectionCar (PP_VIEWMODE_E IN view);

PP_RESULT_E PPAPP_Menu_SaveData (PP_VOID);
PP_RESULT_E PPAPP_Menu_LoadData (PP_VOID);
PP_VOID PPAPP_Menu_ResetData (PP_VOID);
PP_VOID PPAPP_Menu_SetData (PP_VOID);
PP_VOID PPAPP_View_ChangeImage (PP_VIEWMODE_E IN view);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	Temporaty	// Te be moved

PP_S16 gPglAngle;
PP_CAR_DOOR_E gCarOpenDoor;

//---------------------------------------------------------------------------

#endif // USE_PP_GUI

PP_RESULT_E PPAPP_UPGRADE_GetParam(PP_VOID IN *pParam, PP_U32 IN u32ElementMax, PP_U32 OUT *pu32ElementCnt);

#ifdef __cplusplus
}
#endif

#endif // __APPLICATION_H__
