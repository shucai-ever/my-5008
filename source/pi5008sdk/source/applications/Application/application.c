#include "system.h"
#include "debug.h"
#include "osal.h"
#include "sys_api.h"
#include "task_manager.h"
#include "board_config.h"

#include "task_fwdn.h"
#include "task_ui.h"
#include "api_svm.h"
#include "application.h"
#include "api_display.h"
#include "task_display.h"
#include "ver.h"
#include "api_calibration.h"
#include "app_calibration.h"
#include "api_FAT_FTL.h"
#include "task_cache.h"
#include "menu_data.h"
#include "api_calibration.h"

//for capture
#include "api_vpu.h"
#include "api_vin.h"
EXTERN _VPUConfig *gpVPUConfig;

#if defined(USE_PP_GUI)

//---------------------------------------------------------------------------
// Variable

EXTERN PP_BOOL gUiLoadingDone;

PP_U32 apps_pos_point_num = 0;
PP_POS_S *apps_pos_point = NULL_PTR;

PP_RECT_S viewRect[] = {	{0, 0, 528, 720}
						,	{528, 0, 752, 656}
						,	{0, 0, 1280, 720}	};

PP_SCENE_ELEM_S booting =	{
								eScene_Booting,
								NULL_PTR,
								0xFFFFFFFF,
								0xFFFFFFFF,
								{
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
								}
							};

PP_U32 gTopCamViewMode = eViewMode_TopCam_Rear;
PP_U32 gTop3DViewMode = eViewMode_Top3D_Swing_0;

//int angle_value=0;


//switch(angle_value)
//{
//	case 0:
//		gTop3DViewMode = eViewMode_Top3D_Swing_6;
//		break;
//	case 1:
//		gTop3DViewMode = eViewMode_Top3D_Swing_7;
//		break;
//	case 2:
//		gTop3DViewMode = eViewMode_Top3D_Swing_0;
//		break;
//	case 3:
//		gTop3DViewMode = eViewMode_Top3D_Swing_1;
//		break;
//	case 4:
//		gTop3DViewMode = eViewMode_Top3D_Swing_2;
//		break;
//	case 5:
//		gTop3DViewMode = eViewMode_Top3D_Swing_3;
//		break;
//	case 6:
//		gTop3DViewMode = eViewMode_Top3D_Swing_4;
//		break;
//	case 7:
//		gTop3DViewMode = eViewMode_Top3D_Swing_5;
//		break;
//	default:
//		break;
//
//}


PP_SCENE_ELEM_S liveView =	{
								eScene_LiveView,
								PPAPP_UI_LiveView,
								eViewMode_TopCam,
								eViewMode_Max,
								{
									{ eViewMode_TopCam, eViewMode_TopCam_Max, &gTopCamViewMode },
									{ eViewMode_Top3D, eViewMode_Top3D_Max, &gTop3DViewMode },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
								}
							};

PP_SCENE_ELEM_S mainMenu =	{
								eScene_MainMenu,
								PPAPP_UI_MainMenu,
								eMenu_Vehicle,
								eMenu_Max,
								{
									{ eMenu_Vehicle, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Trigger, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Video, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Camera, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Calibration, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Update, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_SystemInfo, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Max, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Max, 0xFFFFFFFF, NULL_PTR },
									{ eMenu_Max, 0xFFFFFFFF, NULL_PTR }
								}
							};

PP_SCENE_ELEM_S vehicleSetting =	{
										eScene_VehicleSetting,
										PPAPP_UI_VehicleSetting,
										eVehicle_Back,
										eVehicle_Max,
										{
											{ eVehicle_Pgl, ePgl_Max, &gMenuData.u32PGL },
											{ eVehicle_DnmBlending, eDnmBlending_Max, &gMenuData.u32DnmBlending },
											{ eVehicle_Back, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR },
											{ eVehicle_Max, 0xFFFFFFFF, NULL_PTR }
										}
									};

PP_SCENE_ELEM_S triggerSetting =	{
										eScene_TriggerSetting,
										PPAPP_UI_TriggerSetting,
										eTrigger_Back,
										eTrigger_Max,
										{
											{ eTrigger_ReverseSignal, eSignal_Max, &gMenuData.u32ReverseSignal },
											{ eTrigger_ReverseView, eReverse_View_Max, &gMenuData.u32ReserveViewMode },
											{ eTrigger_TurnSignal, eSignal_Max, &gMenuData.u32TurnSignal },
											{ eTrigger_TurnView, eTurn_View_Max, &gMenuData.u32TurnViewMode },
											{ eTrigger_EmergencySignal, eSignal_Max, &gMenuData.u32EmergencySignal },
											{ eTrigger_EmergencyView, eEmergency_View_Max, &gMenuData.u32EmergencyViewMode },
											{ eTrigger_Back, 0xFFFFFFFF, NULL_PTR },
											{ eTrigger_Max, 0xFFFFFFFF, NULL_PTR },
											{ eTrigger_Max, 0xFFFFFFFF, NULL_PTR },
											{ eTrigger_Max, 0xFFFFFFFF, NULL_PTR }
										}
									};

PP_SCENE_ELEM_S videoSetting =	{
									eScene_VideoSetting,
									PPAPP_UI_VideoSetting,
									eVideo_Back,
									eVideo_Max,
									{
										{ eVideo_ScreenX, eStepBox_Max, &gMenuData.u32ScreenX },
										{ eVideo_ScreenY, eStepBox_Max, &gMenuData.u32ScreenY },
										{ eVideo_Brightness, eStepBox_Max, &gMenuData.u32Brightness },
										{ eVideo_Contras, eStepBox_Max, &gMenuData.u32Contrast },
										{ eVideo_Saturation, eStepBox_Max, &gMenuData.u32Saturation },
										{ eVideo_Back, 0xFFFFFFFF, NULL_PTR },
										{ eVideo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eVideo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eVideo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eVideo_Max, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S cameraSetting =	{
									eScene_CameraSetting,
									PPAPP_UI_CameraSetting,
									eCamera_Back,
									eCamera_Max,
									{
										{ eCamera_FrontPreview, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_RightPreview, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_LeftPreview, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_RearPreview, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_QuadPreview, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_Capture, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_Back, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCamera_Max, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S cameraPreview =	{
									eScene_CameraPreview,
									PPAPP_UI_CameraPreview,
									0xFFFFFFFF,
									0xFFFFFFFF,
									{
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S cameraCapture =	{
									eScene_CameraCapture,
									PPAPP_UI_CameraCapture,
									0xFFFFFFFF,
									0xFFFFFFFF,
									{
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S calibSetting =	{
									eScene_CalibSetting,
									PPAPP_UI_CalibSetting,
									eCalib_Back,
									eCalib_Max,
									{
										{ eCalib_Auto, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Manual, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Back, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eCalib_Max, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S manualCalib =	{
									eScene_ManualCalib,
									PPAPP_UI_ManualCalibSetting,
									eManualCalib_Back,
									eManualCalib_Max,
									{
										{ eManualCalib_Front, eManualCalib_View_Max, &gMenuData.u32FrontMirror },
										{ eManualCalib_Right, eManualCalib_View_Max, &gMenuData.u32RightMirror },
										{ eManualCalib_Left, eManualCalib_View_Max, &gMenuData.u32LeftMirror },
										{ eManualCalib_Rear, eManualCalib_View_Max, &gMenuData.u32RearMirror },
										{ eManualCalib_ViewGen, 0xFFFFFFFF, NULL_PTR },
										{ eManualCalib_Back, 0xFFFFFFFF, NULL_PTR },
										{ eManualCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eManualCalib_Max, 0xFFFFFFFF, NULL_PTR },
										{ eManualCalib_Max, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S movePoint =	{
								eScene_MovePoint,
								PPAPP_UI_MovePoint,
								0xFFFFFFFF,
								0xFFFFFFFF,
								{
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
								}
							};

PP_SCENE_ELEM_S viewGen =	{
								eScene_ViewGen,
								PPAPP_UI_ViewGeneration,
								0xFFFFFFFF,
								0xFFFFFFFF,
								{
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
									{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
								}
							};

PP_SCENE_ELEM_S updateSetting =	{
									eScene_UpdateSetting,
									PPAPP_UI_UpdateSetting,
									eUpdate_Back,
									eUpdate_Max,
									{
										{ eUpdate_FactoryReset, 0, NULL_PTR },
										{ eUpdate_FwUpdate, 0, NULL_PTR },
										{ eUpdate_Back, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR },
										{ 0xFFFFFFFF, 0xFFFFFFFF, NULL_PTR }
									}
								};

PP_SCENE_ELEM_S systemInfo =	{
									eScene_SystemInfo,
									PPAPP_UI_SystemInfo,
									eSystemInfo_Back,
									eSystemInfo_Max,
									{
										{ eSystemInfo_FWVer, 0, NULL_PTR },
										{ eSystemInfo_LibVer, 0, NULL_PTR },
										{ eSystemInfo_SencorID, 0, NULL_PTR },
										{ eSystemInfo_ISPVer, 0, NULL_PTR },
										{ eSystemInfo_Back, 0xFFFFFFFF, NULL_PTR },
										{ eSystemInfo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eSystemInfo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eSystemInfo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eSystemInfo_Max, 0xFFFFFFFF, NULL_PTR },
										{ eSystemInfo_Max, 0xFFFFFFFF, NULL_PTR },
									}
								};

PP_SCENE_ELEM_S dialog	=	{
								eScene_Dialog,
								PPAPP_UI_Dialog,
								0,
								eDialog_Max,
								{
									{ eDialog_ManualCalib, eDialog_Btn_Max, &gMenuData.u32DialogBtn },
									{ eDialog_SaveView, eDialog_Btn_Max, &gMenuData.u32DialogBtn },
									{ eDialog_RunMenu, eDialog_Btn_Max, &gMenuData.u32DialogBtn },
									{ eDialog_CalibFailRetry , eDialog_Btn_Max, &gMenuData.u32DialogBtn },
									{ eDialog_Update, eDialog_Btn_Max, &gMenuData.u32DialogBtn },
									{ eDialog_Max, 0xFFFFFFFF, NULL_PTR },
									{ eDialog_Max, 0xFFFFFFFF, NULL_PTR },
									{ eDialog_Max, 0xFFFFFFFF, NULL_PTR },
									{ eDialog_Max, 0xFFFFFFFF, NULL_PTR },
									{ eDialog_Max, 0xFFFFFFFF, NULL_PTR },
								}
							};


PP_SCENE_S sceneList[eScene_Max] =	{
										{
											&booting,
											NULL_PTR,
											{ &sceneList[eScene_LiveView], NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }	
										},
										{	&liveView,
											NULL_PTR,
											{ &sceneList[eScene_MainMenu], NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }	
										},
										{
											&mainMenu,
											&sceneList[eScene_LiveView],
											{ &sceneList[eScene_VehicleSetting], &sceneList[eScene_TriggerSetting], &sceneList[eScene_VideoSetting], &sceneList[eScene_CameraSetting], &sceneList[eScene_CalibSetting], &sceneList[eScene_UpdateSetting], &sceneList[eScene_SystemInfo], NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&vehicleSetting,
											&sceneList[eScene_MainMenu],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&triggerSetting,
											&sceneList[eScene_MainMenu],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&videoSetting,
											&sceneList[eScene_MainMenu],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&cameraSetting,
											&sceneList[eScene_MainMenu],
											{ &sceneList[eScene_CameraPreview], &sceneList[eScene_CameraPreview], &sceneList[eScene_CameraPreview], &sceneList[eScene_CameraPreview], &sceneList[eScene_CameraPreview], &sceneList[eScene_CameraCapture], NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&cameraPreview,
											&sceneList[eScene_CameraSetting],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&cameraCapture,
											&sceneList[eScene_CameraSetting],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&calibSetting,
											&sceneList[eScene_MainMenu],
											{ NULL_PTR, &sceneList[eScene_ManualCalib], NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&manualCalib,
											&sceneList[eScene_CalibSetting],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&movePoint,
											&sceneList[eScene_ManualCalib],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&viewGen,
											&sceneList[eScene_ManualCalib],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&updateSetting,
											&sceneList[eScene_MainMenu],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&systemInfo,
											&sceneList[eScene_MainMenu],
											{ NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										},
										{
											&dialog,
											NULL_PTR,
											{ &sceneList[eScene_ManualCalib], &sceneList[eScene_MovePoint], &sceneList[eScene_ViewGen], NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR, NULL_PTR }
										}
									};

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Function
#ifdef CALIB_LIB_USE
STATIC PP_VOID PPAPP_Calib_SearchPoint (PP_OFFCALIB_CAMERA_CAPTURE_CH_E camera_ch)
{

	PPAPI_Offcalib_Save_Cam_Ch(camera_ch);
	AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_UI, TASK_CALIBRATION, eOFFCALIB_START, NULL, 0, 1000);
}
#endif
STATIC PP_VOID PPAPP_Video_SetScreenX (PP_U32 stepIdx)
{
	PP_S8 winx, winy;
	
	switch(stepIdx)
	{
		case eStepBox_0:	winx = -25;	break;
		case eStepBox_1:	winx = -20;	break;
		case eStepBox_2:	winx = -15;	break;
		case eStepBox_3:	winx = -10;	break;
		case eStepBox_4:	winx = -5;	break;
		case eStepBox_5:	winx = 0;	break;
		case eStepBox_6:	winx = 5;	break;
		case eStepBox_7:	winx = 10;	break;
		case eStepBox_8:	winx = 15;	break;
		case eStepBox_9:	winx = 20;	break;
		case eStepBox_10:	winx = 25;	break;
	}

	switch(gMenuData.u32ScreenY)
	{
		case eStepBox_0:	winy = -25;	break;
		case eStepBox_1:	winy = -20;	break;
		case eStepBox_2:	winy = -15;	break;
		case eStepBox_3:	winy = -10;	break;
		case eStepBox_4:	winy = -5;	break;
		case eStepBox_5:	winy = 0;	break;
		case eStepBox_6:	winy = 5;	break;
		case eStepBox_7:	winy = 10;	break;
		case eStepBox_8:	winy = 15;	break;
		case eStepBox_9:	winy = 20;	break;
		case eStepBox_10:	winy = 25;	break;
	}

	PPAPI_SVM_SetWindowOffset(winx, winy);
}

STATIC PP_VOID PPAPP_Video_SetScreenY (PP_U32 stepIdx)
{
	PP_S8 winx, winy;

	switch(gMenuData.u32ScreenX)
	{
		case eStepBox_0:	winx = -25;	break;
		case eStepBox_1:	winx = -20;	break;
		case eStepBox_2:	winx = -15;	break;
		case eStepBox_3:	winx = -10;	break;
		case eStepBox_4:	winx = -5;	break;
		case eStepBox_5:	winx = 0;	break;
		case eStepBox_6:	winx = 5;	break;
		case eStepBox_7:	winx = 10;	break;
		case eStepBox_8:	winx = 15;	break;
		case eStepBox_9:	winx = 20;	break;
		case eStepBox_10:	winx = 25;	break;
	}
	
	switch(stepIdx)
	{
		case eStepBox_0:	winy = -25;	break;
		case eStepBox_1:	winy = -20;	break;
		case eStepBox_2:	winy = -15;	break;
		case eStepBox_3:	winy = -10;	break;
		case eStepBox_4:	winy = -5;	break;
		case eStepBox_5:	winy = 0;	break;
		case eStepBox_6:	winy = 5;	break;
		case eStepBox_7:	winy = 10;	break;
		case eStepBox_8:	winy = 15;	break;
		case eStepBox_9:	winy = 20;	break;
		case eStepBox_10:	winy = 25;	break;
	}

	 PPAPI_SVM_SetWindowOffset(winx, winy);
}

STATIC PP_VOID PPAPP_Video_SetBrightness (PP_U32 stepIdx)
{
	PP_U8 brightness;

	if(stepIdx == eStepBox_5)	// default
	{
		PPDRV_DU_BTO_EnableBrightness(PP_FALSE);
	}
	else
	{
		switch(stepIdx)
		{
			case eStepBox_0:	brightness = 78;	break;
			case eStepBox_1:	brightness = 88;	break;
			case eStepBox_2:	brightness = 98;	break;
			case eStepBox_3:	brightness = 108;	break;
			case eStepBox_4:	brightness = 118;	break;
			case eStepBox_5:	brightness = 128;	break;
			case eStepBox_6:	brightness = 138;	break;
			case eStepBox_7:	brightness = 148;	break;
			case eStepBox_8:	brightness = 158;	break;
			case eStepBox_9:	brightness = 168;	break;
			case eStepBox_10:	brightness = 178;	break;
		}
		PPDRV_DU_BTO_SetBrightness(brightness);
		PPDRV_DU_BTO_EnableBrightness(PP_TRUE);
	}
}

STATIC PP_VOID PPAPP_Video_SetContrast (PP_U32 stepIdx)
{
	PP_U8 contrast;

	if(stepIdx == eStepBox_5)	// default
	{
		PPDRV_DU_BTO_EnableContrast(PP_FALSE);
	}
	else
	{
		switch(stepIdx)
		{
			case eStepBox_0:	contrast = 78;	break;
			case eStepBox_1:	contrast = 88;	break;
			case eStepBox_2:	contrast = 98;	break;
			case eStepBox_3:	contrast = 108;	break;
			case eStepBox_4:	contrast = 118;	break;
			case eStepBox_5:	contrast = 128;	break;
			case eStepBox_6:	contrast = 138;	break;
			case eStepBox_7:	contrast = 148;	break;
			case eStepBox_8:	contrast = 158;	break;
			case eStepBox_9:	contrast = 168;	break;
			case eStepBox_10:	contrast = 178;	break;
		}
		PPDRV_DU_BTO_SetContrast(contrast);
		PPDRV_DU_BTO_EnableContrast(PP_TRUE);
	}
}

STATIC PP_VOID PPAPP_Video_SetSaturation (PP_U32 stepIdx)
{
	PP_U8 saturation;

	if(stepIdx == eStepBox_5)	// default
	{
		PPDRV_DU_BTO_EnableSaturation(PP_FALSE);
	}
	else
	{
		switch(stepIdx)
		{
			case eStepBox_0:	saturation = 78;	break;
			case eStepBox_1:	saturation = 88;	break;
			case eStepBox_2:	saturation = 98;	break;
			case eStepBox_3:	saturation = 108;	break;
			case eStepBox_4:	saturation = 118;	break;
			case eStepBox_5:	saturation = 128;	break;
			case eStepBox_6:	saturation = 138;	break;
			case eStepBox_7:	saturation = 148;	break;
			case eStepBox_8:	saturation = 158;	break;
			case eStepBox_9:	saturation = 168;	break;
			case eStepBox_10:	saturation = 178;	break;
		}
		PPDRV_DU_BTO_SetSaturation(saturation);
		PPDRV_DU_BTO_EnableSaturation(PP_TRUE);
	}
}

STATIC PP_U32 PPAPP_Menu_GetCheckSum (PP_VOID)
{
	PP_U32 i = 0;
	PP_U32 sum = 0;
	
	for(i=0 ; i<(MENU_DATA_NUM-1) ; i++)
	{
		sum += ((PP_U32*)&gMenuData)[i];
	}

	return sum;
}

PP_RESULT_E PPAPP_Menu_SaveData (PP_VOID)
{
	PP_U8 buf[MENU_DATA_SIZE] = {0};
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_FLASH_HDR_S *pstHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
#else
	// Menu data exists in FTL
	PP_FLASH_HDR_S *pstHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;
#endif
	
	if(FLASH_VER_ERROR_CHECK(pstHdr->stSect[eFLASH_SECT_MENU].u32Ver))
		return eERROR_NOT_FOUND;
	
	gMenuData.u32CheckSum = PPAPP_Menu_GetCheckSum();
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PPAPI_FLASH_Write((PP_U8 *)&gMenuData, pstHdr->stSect[eFLASH_SECT_MENU].u32FlashAddr, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);	// erase & write
	PPAPI_FLASH_Read((PP_U8 *)&buf, pstHdr->stSect[eFLASH_SECT_MENU].u32FlashAddr, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);
#else
	PPAPI_FTL_Write((PP_U8 *)&gMenuData, pstHdr->stSect[eFLASH_SECT_MENU].u32FlashAddr, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);	// erase & write
	PPAPI_FTL_Read((PP_U8 *)&buf, pstHdr->stSect[eFLASH_SECT_MENU].u32FlashAddr, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);
	PPAPI_SYS_CACHE_Writeback((PP_U32 *)&buf, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);
#endif
	if( (memcmp(&gMenuData, &buf, pstHdr->stSect[eFLASH_SECT_MENU].u32Size)) != 0 )
	{
		return eERROR_FAILURE;
	}
	
	return eSUCCESS;
}

PP_RESULT_E PPAPP_Menu_LoadData (PP_VOID)
{
	PP_RESULT_E result = eSUCCESS;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_FLASH_HDR_S *pstHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
#else
	// Menu data exists in FTL
	PP_FLASH_HDR_S *pstHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;
#endif
	
	if(FLASH_VER_ERROR_CHECK(pstHdr->stSect[eFLASH_SECT_MENU].u32Ver))
	{
		result = eERROR_NOT_FOUND;
		goto LOAD_FAIL;
	}
	
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PPAPI_FLASH_Read((PP_U8 *)&gMenuData, pstHdr->stSect[eFLASH_SECT_MENU].u32FlashAddr, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);
#else
	PPAPI_FTL_Read((PP_U8 *)&gMenuData, pstHdr->stSect[eFLASH_SECT_MENU].u32FlashAddr, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);
	PPAPI_SYS_CACHE_Writeback((PP_U32 *)&gMenuData, pstHdr->stSect[eFLASH_SECT_MENU].u32Size);
#endif
	if(gMenuData.u32MagicNumber != MENU_DATA_MAGIC_NUMBER)
	{
		result = eERROR_FAILURE;
		goto LOAD_FAIL;
	}
	
	if(gMenuData.u32CheckSum != PPAPP_Menu_GetCheckSum())
	{
		result = eERROR_FAILURE;
		goto LOAD_FAIL;
	}

	return result;

LOAD_FAIL:
	memcpy(&gMenuData, &gMenuDataBackup, MENU_DATA_SIZE);
	return result;
}

PP_VOID PPAPP_Menu_ResetData (PP_VOID)
{
	memcpy(&gMenuData, &gMenuDataBackup, MENU_DATA_SIZE);
}

PP_VOID PPAPP_Menu_SetData (PP_VOID)
{
	PPAPP_Video_SetScreenX(gMenuData.u32ScreenX);
	PPAPP_Video_SetScreenY(gMenuData.u32ScreenY);
	PPAPP_Video_SetBrightness(gMenuData.u32Brightness);
	PPAPP_Video_SetContrast(gMenuData.u32Contrast);
	PPAPP_Video_SetSaturation(gMenuData.u32Saturation);
}

PP_VOID PPAPP_View_SetSectionCar (PP_VIEWMODE_E IN view)
{
	PP_U32 sectionCnt;
	PP_S32 sectionID;
	PP_U32 i;
	
	sectionCnt = PPAPI_SVMMEM_GetViewSectionCount(view);
	if(sectionCnt <= 1)
	{
		PPAPI_DISPLAY_VIEW_Car_Off();
	}
	else
	{
		for(i=0 ; i<sectionCnt ; i++)
		{
			sectionID = PPAPI_SVMMEM_GetViewSectionIndex(view, i);
			PPAPI_DISPLAY_VIEW_Car_On(sectionID);
		}
	}
}

STATIC PP_VOID PPAPP_View_SetSectionPgl (PP_VIEWMODE_E IN view)
{
	PP_U32 sectionCnt;
	PP_S32 sectionID;
	PP_U32 i;
	PP_BOOL isBw = PP_FALSE;
	
	sectionCnt = PPAPI_SVMMEM_GetViewSectionCount(view);
	for(i=0 ; i<sectionCnt ; i++)
	{
		sectionID = PPAPI_SVMMEM_GetViewSectionIndex(view, i);
		if(view == eVIEWMODE_LOAD_TOP2D_REARSD)
			isBw = PP_TRUE;
		else
			isBw = PP_FALSE;
		PPAPI_DISPLAY_VIEW_PGL_On(sectionID, isBw);
	}
}

STATIC PP_BOOL PPAPP_UI_CheckLayout (PP_VIEWMODE_E IN view)
{
	PP_U32 sectionCnt;
	PP_S32 sectionIdx;
	PP_RECT_S *sectionRect;
	PP_U32 i, j;
	
	sectionCnt = PPAPI_SVMMEM_GetViewSectionCount(view);
	for(i=0 ; i<sectionCnt ; i++)
	{
		sectionIdx = PPAPI_SVMMEM_GetViewSectionIndex(view, i);
		sectionRect = PPAPI_SVMMEM_GetViewSectionRect(view, sectionIdx);

		for(j=0 ; j<2 ; j++)
		{
			if(memcmp(sectionRect, &viewRect[j], sizeof(PP_RECT_S)) == 0)
			{
				return PP_TRUE;
			}
		}
	}
	return PP_FALSE;
}

//  UI ---------------------------------------------------------------------------
PP_SCENE_S *PPAPP_UI_Initialize (PP_VOID)
{
	PPAPI_VIN_EnableQuad(PP_TRUE);

	PPAPP_Menu_LoadData();
	PPAPP_Menu_SetData();

	return &sceneList[eScene_LiveView];
}

PP_SCENE_S *PPAPP_UI_GetScene (PP_SCENE_E IN idx)
{
	return &sceneList[idx];
}

PP_VOID PPAPP_View_ChangeImage (PP_VIEWMODE_E IN view)
{
	PP_SCENE_S *scene = PPAPP_UI_GetScene(eScene_LiveView);

	if(PPAPP_UI_CheckLayout(view))
		PPAPI_DISPLAY_VIEW_Outline_On(scene->elem->selIdx, *scene->elem->subElem[scene->elem->selIdx].value);
	else
		PPAPI_DISPLAY_VIEW_Outline_Off();

	PPAPP_View_SetSectionCar(view);

	PPAPI_DISPLAY_VIEW_PGL_Off();
	if(gUiLoadingDone)
	{
		if(gMenuData.u32PGL == ePgl_Rear)
		{
			if(view == eVIEWMODE_LOAD_TOP2D_REARSD)
				PPAPP_View_SetSectionPgl(view);
		}
		else if(gMenuData.u32PGL == ePgl_Always)
		{
			PPAPP_View_SetSectionPgl(view);
		}
	}
}

STATIC PP_VOID PPAPP_UI_InitLiveView (PP_VIEWMODE_E IN view)
{
	PPAPI_DISPLAY_DisableAll();
	PPAPI_DISPLAY_VIEW_PGL_Off();

	PPAPI_DISPlAY_UpdateLUT();
	PPAPP_View_ChangeImage(view);
}

PP_VOID PPAPP_UI_LiveView (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	STATIC PP_VIEW_MODE_E revertView = eViewMode_TopCam;
	STATIC PP_VIEWMODE_E svmCurView;
	PP_VIEWMODE_E svmChangeView;
	PP_RESULT_E result = eSUCCESS;
	
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);

	if( (event != CMD_UI_INIT_SCENE) && !gUiLoadingDone )
		return;

	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			
			if(scene->elem->selIdx == eViewMode_TopCam)
			{
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_TopCam_Front:	svmChangeView = eVIEWMODE_LOAD_TOP2D_FRONTSD;		break;
					case eViewMode_TopCam_Left:		svmChangeView = eVIEWMODE_LOAD_TOP2D_LEFTSD;		break;
					case eViewMode_TopCam_Right:	svmChangeView = eVIEWMODE_LOAD_TOP2D_RIGHTSD;		break;
					case eViewMode_TopCam_Rear:		svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
					case eViewMode_TopCam_WideFront:svmChangeView = eVIEWMODE_LOAD_FRONTWIDECAM;		break;
					case eViewMode_TopCam_WideRear:	svmChangeView = eVIEWMODE_LOAD_REARWIDECAM;			break;
					default:						svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
				}
			}
			else
			{
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_Top3D_Swing_0:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
					case eViewMode_Top3D_Swing_1:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;		break;
					case eViewMode_Top3D_Swing_2:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;		break;
					case eViewMode_Top3D_Swing_3:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_134;		break;
					case eViewMode_Top3D_Swing_4:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;		break;
					case eViewMode_Top3D_Swing_5:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_224;		break;
					case eViewMode_Top3D_Swing_6:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;		break;
					case eViewMode_Top3D_Swing_7:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;		break;
					default:						svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
				}
			}

			PPAPP_CACHE_Suspend();
			result = PPAPI_SVM_SetView(svmChangeView, PPAPP_UI_InitLiveView, PP_NULL);
			PPAPP_CACHE_Resume();

			if(result == eSUCCESS)
				svmCurView = svmChangeView;
			break;
			
		case CMD_UI_KEY_CENTER:
			if(!gUiLoadingDone)
				break;
			
			if(scene->elem->selIdx == eViewMode_Top3D)
			{
				scene->elem->selIdx = eViewMode_TopCam;
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_TopCam_Front:	svmChangeView = eVIEWMODE_LOAD_TOP2D_FRONTSD;		break;
					case eViewMode_TopCam_Left:		svmChangeView = eVIEWMODE_LOAD_TOP2D_LEFTSD;		break;
					case eViewMode_TopCam_Right:	svmChangeView = eVIEWMODE_LOAD_TOP2D_RIGHTSD;		break;
					case eViewMode_TopCam_Rear:		svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
					case eViewMode_TopCam_WideFront:svmChangeView = eVIEWMODE_LOAD_FRONTWIDECAM;		break;
					case eViewMode_TopCam_WideRear:	svmChangeView = eVIEWMODE_LOAD_REARWIDECAM;			break;
					default:						svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
				}
			}
			else
			{
				scene->elem->selIdx = eViewMode_Top3D;
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_Top3D_Swing_0:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
					case eViewMode_Top3D_Swing_1:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;		break;
					case eViewMode_Top3D_Swing_2:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;		break;
					case eViewMode_Top3D_Swing_3:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_134;		break;
					case eViewMode_Top3D_Swing_4:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;		break;
					case eViewMode_Top3D_Swing_5:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_224;		break;
					case eViewMode_Top3D_Swing_6:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;		break;
					case eViewMode_Top3D_Swing_7:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;		break;
					default:						svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
				}
			}

			PPAPP_CACHE_Suspend(); 
			result = PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
			PPAPP_CACHE_Resume();

			if(result != eSUCCESS)
			{
				if(scene->elem->selIdx == eViewMode_TopCam)
					scene->elem->selIdx = eViewMode_Top3D;
				else
					scene->elem->selIdx = eViewMode_TopCam;
			}
			else
			{
				svmCurView = svmChangeView;
			}
			break;

		case CMD_UI_KEY_UP:
			if(scene->elem->selIdx == eViewMode_TopCam)
			{
				if(*scene->elem->subElem[scene->elem->selIdx].value == eViewMode_TopCam_Front)
				{
					svmChangeView = eVIEWMODE_LOAD_FRONTWIDECAM;
					*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_TopCam_WideFront;
				}
				else if(*scene->elem->subElem[scene->elem->selIdx].value != eViewMode_TopCam_WideFront)
				{
				svmChangeView = eVIEWMODE_LOAD_TOP2D_FRONTSD;
					*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_TopCam_Front;
			}
			else
			{
					break;
			}
				
			PPAPP_CACHE_Suspend();
			result = PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
			PPAPP_CACHE_Resume();

			if(result != eSUCCESS)
					*scene->elem->subElem[scene->elem->selIdx].value = svmCurView;
				else
					svmCurView = svmChangeView;
			}
			break;

		case CMD_UI_KEY_DOWN:
			if(scene->elem->selIdx == eViewMode_TopCam)
			{
				if(*scene->elem->subElem[scene->elem->selIdx].value == eViewMode_TopCam_Rear)
				{
					svmChangeView = eVIEWMODE_LOAD_REARWIDECAM;
					*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_TopCam_WideRear;
				}
				else if(*scene->elem->subElem[scene->elem->selIdx].value != eViewMode_TopCam_WideRear)
				{
				svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;
					*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_TopCam_Rear;
			}
			else
			{
					break;
			}
				
			PPAPP_CACHE_Suspend();
			result = PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
			PPAPP_CACHE_Resume();
			
			if(result != eSUCCESS)
					*scene->elem->subElem[scene->elem->selIdx].value = svmCurView;
				else
					svmCurView = svmChangeView;
			}
			break;

		case CMD_UI_KEY_LEFT:
			if(scene->elem->selIdx == eViewMode_TopCam)
			{
				*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_TopCam_Left;
				svmChangeView = eVIEWMODE_LOAD_TOP2D_LEFTSD;
				PPAPP_CACHE_Suspend();
				result = PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume();

				if(result != eSUCCESS)
					*scene->elem->subElem[scene->elem->selIdx].value = svmCurView;
			}
			else
			{
				if(*scene->elem->subElem[scene->elem->selIdx].value > eViewMode_Top3D_Swing_0)
				{
					(*scene->elem->subElem[scene->elem->selIdx].value)--;
				}
				else
				{
					*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_Top3D_Swing_7;
				}

				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_Top3D_Swing_0:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;	break;
					case eViewMode_Top3D_Swing_1:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;	break;
					case eViewMode_Top3D_Swing_2:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;	break;
					case eViewMode_Top3D_Swing_3:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_134;	break;
					case eViewMode_Top3D_Swing_4:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;	break;
					case eViewMode_Top3D_Swing_5:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_224;	break;
					case eViewMode_Top3D_Swing_6:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;	break;
					case eViewMode_Top3D_Swing_7:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;	break;
				} 
				PPAPP_CACHE_Suspend();
				result = PPAPI_SVM_SetSwingView(svmCurView, svmChangeView, eSVMAPI_SWING_DIRECTION_AUTO, 1, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume(); 

				if(result != eSUCCESS)
				{
					if(*scene->elem->subElem[scene->elem->selIdx].value == eViewMode_Top3D_Swing_7)
						*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_Top3D_Swing_0;
					else
						(*scene->elem->subElem[scene->elem->selIdx].value)++;
				}
			}

			if(result == eSUCCESS)
				svmCurView = svmChangeView;
			break;
		case CMD_UI_KEY_RIGHT:
			if(scene->elem->selIdx == eViewMode_TopCam)
			{
				*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_TopCam_Right;
				svmChangeView = eVIEWMODE_LOAD_TOP2D_RIGHTSD;
				PPAPP_CACHE_Suspend();
				result = PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume();
				
				if(result != eSUCCESS)
					*scene->elem->subElem[scene->elem->selIdx].value = svmCurView;
			}
			else
			{
				if(*scene->elem->subElem[scene->elem->selIdx].value < eViewMode_Top3D_Swing_7)
				{
					(*scene->elem->subElem[scene->elem->selIdx].value)++;
				}
				else
				{
					*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_Top3D_Swing_0;
				}
				
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_Top3D_Swing_0:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;	break;
					case eViewMode_Top3D_Swing_1:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;	break;
					case eViewMode_Top3D_Swing_2:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;	break;
					case eViewMode_Top3D_Swing_3:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;	break;
					case eViewMode_Top3D_Swing_4:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;	break;
					case eViewMode_Top3D_Swing_5:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_134;	break;
					case eViewMode_Top3D_Swing_6:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;	break;
					case eViewMode_Top3D_Swing_7:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_224;	break;

				} 
				PPAPP_CACHE_Suspend();
				result = PPAPI_SVM_SetSwingView(svmCurView, svmChangeView, eSVMAPI_SWING_DIRECTION_CLOCKWISE, 1, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume(); 
				
				if(result != eSUCCESS)
				{
					if(*scene->elem->subElem[scene->elem->selIdx].value == eViewMode_Top3D_Swing_0)
						*scene->elem->subElem[scene->elem->selIdx].value = eViewMode_Top3D_Swing_7;
					else
						(*scene->elem->subElem[scene->elem->selIdx].value)--;
				}
			}
			
			if(result == eSUCCESS)
				svmCurView = svmChangeView;
			break;


		case CMD_UI_KEY_3D_ANGLE:
			if(scene->elem->selIdx == eViewMode_Top3D)
			{

				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_Top3D_Swing_8Angle_0:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;	break;
					case eViewMode_Top3D_Swing_8Angle_1:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314; break;
					case eViewMode_Top3D_Swing_8Angle_2:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;	break;
					case eViewMode_Top3D_Swing_8Angle_3:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;	break;
					case eViewMode_Top3D_Swing_8Angle_4:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;	break;
					case eViewMode_Top3D_Swing_8Angle_5:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_134;	break;
					case eViewMode_Top3D_Swing_8Angle_6:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;	break;
					case eViewMode_Top3D_Swing_8Angle_7:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_224;	break;
				}

				PPAPP_CACHE_Suspend();
				result = PPAPI_SVM_SetSwingView(svmCurView, svmChangeView, eSVMAPI_SWING_DIRECTION_AUTO, 1, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume();

				if(result != eSUCCESS)
				{
					(*scene->elem->subElem[scene->elem->selIdx].value) = eViewMode_Top3D_Swing_0;
				}

			}
			if(result == eSUCCESS)
				svmCurView = svmChangeView;
			break;


		case CMD_UI_KEY_MENU:
			#if defined(USE_GUI_MENU)
			if(!gUiLoadingDone)
				break;
			
			AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			#endif
			break;

		case CMD_UI_TRIGGER_REVERSE:
			if(gMenuData.u32ReverseSignal == eSignal_On)
			{
				revertView = scene->elem->selIdx;
				
				switch(gMenuData.u32ReserveViewMode)
				{
					case eReverse_View_Top3DRear:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;		break;
					case eReverse_View_TopRear:		svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
						break;
				}
				
				PPAPP_CACHE_Suspend();
				PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume();
			}
			break;
			
		case CMD_UI_TRIGGER_TURN:
			if(gMenuData.u32TurnSignal == eSignal_On)
			{
				revertView = scene->elem->selIdx;
				
				switch(gMenuData.u32TurnViewMode)
				{
					case eTurn_View_Top3DLeftFront:		svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;	break;
					case eTurn_View_Top3DRightFront:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;	break;
					case eTurn_View_Top3DLeft:			svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;	break;
					case eTurn_view_Top3DRight:			svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;	break;
				}
				PPAPP_CACHE_Suspend();
				PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume();
			}
			break;
			
		case CMD_UI_TRIGGER_EMERGENCY:
			if(gMenuData.u32EmergencySignal == eSignal_On)
			{
				revertView = scene->elem->selIdx;
				
				switch(gMenuData.u32EmergencyViewMode)
				{
					case eEmergency_View_Top3DFront:		svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
					case eEmergency_View_TopFront:			svmChangeView = eVIEWMODE_LOAD_TOP2D_FRONTSD;		break;
					case eEmergency_View_Top3DLeftFront:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;		break;
					case eEmergency_View_Top3DRightFront:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;		break;
				}
				PPAPP_CACHE_Suspend();
				PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
				PPAPP_CACHE_Resume();
			}
			break;

		case CMD_UI_TRIGGER_DRIVE:
			if(revertView == eViewMode_TopCam)
			{
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_TopCam_Front:	svmChangeView = eVIEWMODE_LOAD_TOP2D_FRONTSD;		break;
					case eViewMode_TopCam_Left:		svmChangeView = eVIEWMODE_LOAD_TOP2D_LEFTSD;		break;
					case eViewMode_TopCam_Right:	svmChangeView = eVIEWMODE_LOAD_TOP2D_RIGHTSD;		break;
					case eViewMode_TopCam_Rear:		svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
					case eViewMode_TopCam_WideFront:svmChangeView = eVIEWMODE_LOAD_FRONTWIDECAM;		break;
					case eViewMode_TopCam_WideRear:	svmChangeView = eVIEWMODE_LOAD_REARWIDECAM;			break;
					default:						svmChangeView = eVIEWMODE_LOAD_TOP2D_REARSD;		break;
				}
			}
			else
			{
				switch(*scene->elem->subElem[scene->elem->selIdx].value)
				{
					case eViewMode_Top3D_Swing_0:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
					case eViewMode_Top3D_Swing_1:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_44;		break;
					case eViewMode_Top3D_Swing_2:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_90;		break;
					case eViewMode_Top3D_Swing_3:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_134;		break;
					case eViewMode_Top3D_Swing_4:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_180;		break;
					case eViewMode_Top3D_Swing_5:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_224;		break;
					case eViewMode_Top3D_Swing_6:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_270;		break;
					case eViewMode_Top3D_Swing_7:	svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_314;		break;
					default:						svmChangeView = eVIEWMODE_LOAD_TOP2D_SWING_0;		break;
				}
			}

			PPAPP_CACHE_Suspend(); 
			result = PPAPI_SVM_SetView(svmChangeView, PPAPP_View_ChangeImage, PP_NULL);
			PPAPP_CACHE_Resume();

			if(result == eSUCCESS)
				svmCurView = svmChangeView;
			break;
	}
}

PP_VOID PPAPP_UI_MainMenu (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);

			PPAPI_SVM_SetView(0, PP_NULL, PP_NULL);
			break;
			
		case CMD_UI_KEY_CENTER:
		#ifdef CALIB_LIB_USE
			
			AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, scene->elem->selIdx, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
		#else //CALIB_LIB_USE
			if(scene->elem->selIdx == eMenu_Calibration)
			{
				AppTask_SendCmd(CMD_DISPLAY_POPUP_TIMER, TASK_UI, TASK_DISPLAY, ePopUp_TBD, NULL_PTR, 0, 1000);
			}
			else
			
			{
				AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, scene->elem->selIdx, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
		#endif //CALIB_LIB_USE
			break;

		case CMD_UI_KEY_UP:
			break;

		case CMD_UI_KEY_DOWN:
			break;
			
		case CMD_UI_KEY_LEFT:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;

		case CMD_UI_KEY_RIGHT:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;

			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;

		case CMD_UI_KEY_MENU:
			AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			break;
	}
}

PP_VOID PPAPP_UI_VehicleSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eVehicle_Pgl;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eVehicle_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				
				PPAPP_Menu_SaveData();
#ifdef DB_LIB_USE
				AppTask_SendCmd(CMD_DYNBLEND_OPER, TASK_UI, TASK_DYNBLEND, 0, &gMenuData.u32DnmBlending, sizeof(gMenuData.u32DnmBlending), 1000);
#endif // DB_LIB_USE
			}
			break;

		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;

			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;
			
		case CMD_UI_KEY_LEFT:
			if(*scene->elem->subElem[scene->elem->selIdx].value > 0)
				(*scene->elem->subElem[scene->elem->selIdx].value)--;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = scene->elem->subElem[scene->elem->selIdx].valueNum - 1;
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_RIGHT:
			if(*scene->elem->subElem[scene->elem->selIdx].value < (scene->elem->subElem[scene->elem->selIdx].valueNum - 1))
				(*scene->elem->subElem[scene->elem->selIdx].value)++;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = 0;
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

PP_VOID PPAPP_UI_TriggerSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eTrigger_ReverseSignal;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eTrigger_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);

				PPAPP_Menu_SaveData();
			}
			break;
			
		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;

			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;
			
		case CMD_UI_KEY_LEFT:
			if(*scene->elem->subElem[scene->elem->selIdx].value > 0)
				(*scene->elem->subElem[scene->elem->selIdx].value)--;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = scene->elem->subElem[scene->elem->selIdx].valueNum - 1;
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_RIGHT:
			if(*scene->elem->subElem[scene->elem->selIdx].value < (scene->elem->subElem[scene->elem->selIdx].valueNum - 1))
				(*scene->elem->subElem[scene->elem->selIdx].value)++;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = 0;
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

PP_VOID PPAPP_UI_VideoSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eVideo_ScreenX;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eVideo_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);

				PPAPP_Menu_SaveData();
			}
			break;

		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;
			
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_LEFT:
			if(*scene->elem->subElem[scene->elem->selIdx].value > 0)
				(*scene->elem->subElem[scene->elem->selIdx].value)--;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = 0;
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			
			if(scene->elem->selIdx == eVideo_ScreenX)
			{
				PPAPP_Video_SetScreenX(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_ScreenY)
			{
				PPAPP_Video_SetScreenY(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_Brightness)
			{
				PPAPP_Video_SetBrightness(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_Contras)
			{
				PPAPP_Video_SetContrast(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_Saturation)
			{
				PPAPP_Video_SetSaturation(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			break;

		case CMD_UI_KEY_RIGHT:
			if(*scene->elem->subElem[scene->elem->selIdx].value < (scene->elem->subElem[scene->elem->selIdx].valueNum - 1))
				(*scene->elem->subElem[scene->elem->selIdx].value)++;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = scene->elem->subElem[scene->elem->selIdx].valueNum - 1;
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);

			if(scene->elem->selIdx == eVideo_ScreenX)
			{
				PPAPP_Video_SetScreenX(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_ScreenY)
			{
				PPAPP_Video_SetScreenY(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_Brightness)
			{
				PPAPP_Video_SetBrightness(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_Contras)
			{
				PPAPP_Video_SetContrast(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			else if(scene->elem->selIdx == eVideo_Saturation)
			{
				PPAPP_Video_SetSaturation(*scene->elem->subElem[scene->elem->selIdx].value);
			}
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

STATIC PP_VOID PPAPP_UI_InitCameraSetting (PP_VIEWMODE_E IN view)
{
	PP_SCENE_S *scene = PPAPP_UI_GetScene(eScene_CameraSetting);
	PPAPI_DISPLAY_Background_Off();
	PPAPI_DISPLAY_DisableAll();
	PPAPI_DISPlAY_UpdateLUT();
	PPAPI_DISPLAY_MenuList(scene->elem->id);
	PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
}

PP_VOID PPAPP_UI_CameraSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
#ifdef CALIB_LIB_USE 
	PP_OFFCALIB_CAP_YUV_INFO_S pYuvBufInfoS;
#endif
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);

	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eCamera_FrontPreview;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			if(scene->elem->selIdx == eCamera_FrontPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PPAPP_UI_InitCameraSetting, PP_NULL);	// front preview
			else if(scene->elem->selIdx == eCamera_LeftPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_LEFT_BYPASS, PPAPP_UI_InitCameraSetting, PP_NULL);	// left preview
			else if(scene->elem->selIdx == eCamera_RightPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_RIGHT_BYPASS, PPAPP_UI_InitCameraSetting, PP_NULL);	// right preview
			else if(scene->elem->selIdx == eCamera_RearPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_REAR_BYPASS, PPAPP_UI_InitCameraSetting, PP_NULL);	// rear preview
			else if(scene->elem->selIdx == eCamera_QuadPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PPAPP_UI_InitCameraSetting, PP_NULL);	// quad preview
			else
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PPAPP_UI_InitCameraSetting, PP_NULL);	// front preview
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eCamera_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			
			
#ifdef CALIB_LIB_USE
		
			else if(scene->elem->selIdx == eCamera_FrontPreview)
			{
				PPAPI_DISPLAY_DisableAll();
				PPAPI_DISPLAY_Background_Off();
				PPAPI_Offcalib_Capture_YUV_Image(&pYuvBufInfoS,0,CAPTURE_UYVY);
				PPAPI_Offcalib_BMP_Image_Save_SD(&pYuvBufInfoS,0);
				AppTask_SendCmd(CMD_DISPLAY_PROGRESSBAR_TIMER_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
				PPAPP_UI_InitCameraSetting(VIEWMODE_NULL);
			}
			else if(scene->elem->selIdx == eCamera_RightPreview)
			{
				PPAPI_DISPLAY_DisableAll();
				PPAPI_DISPLAY_Background_Off();
				PPAPI_Offcalib_Capture_YUV_Image(&pYuvBufInfoS,2,CAPTURE_UYVY);
				PPAPI_Offcalib_BMP_Image_Save_SD(&pYuvBufInfoS,2);
				AppTask_SendCmd(CMD_DISPLAY_PROGRESSBAR_TIMER_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
				PPAPP_UI_InitCameraSetting(VIEWMODE_NULL);
			}
			else if(scene->elem->selIdx == eCamera_LeftPreview)
			{
				PPAPI_DISPLAY_DisableAll();
				PPAPI_DISPLAY_Background_Off();
				PPAPI_Offcalib_Capture_YUV_Image(&pYuvBufInfoS,1,CAPTURE_UYVY);
				PPAPI_Offcalib_BMP_Image_Save_SD(&pYuvBufInfoS,1);
				AppTask_SendCmd(CMD_DISPLAY_PROGRESSBAR_TIMER_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
				PPAPP_UI_InitCameraSetting(VIEWMODE_NULL);
			}
			else if(scene->elem->selIdx == eCamera_RearPreview)
			{
				PPAPI_DISPLAY_DisableAll();
				PPAPI_DISPLAY_Background_Off();
				PPAPI_Offcalib_Capture_YUV_Image(&pYuvBufInfoS,3,CAPTURE_UYVY);
				PPAPI_Offcalib_BMP_Image_Save_SD(&pYuvBufInfoS,3);
				AppTask_SendCmd(CMD_DISPLAY_PROGRESSBAR_TIMER_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
				PPAPP_UI_InitCameraSetting(VIEWMODE_NULL);
			}
#endif
			else
			{
				AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, scene->elem->selIdx, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			break;

		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			
			if(scene->elem->selIdx == eCamera_FrontPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL);	// front preview
			else if(scene->elem->selIdx == eCamera_LeftPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_LEFT_BYPASS, PP_NULL, PP_NULL);	// left preview
			else if(scene->elem->selIdx == eCamera_RightPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_RIGHT_BYPASS, PP_NULL, PP_NULL);	// right preview
			else if(scene->elem->selIdx == eCamera_RearPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_REAR_BYPASS, PP_NULL, PP_NULL);	// rear preview
			else if(scene->elem->selIdx == eCamera_QuadPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PP_NULL, PP_NULL);	// quad preview
			else
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL);	// front preview
				
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;

			if(scene->elem->selIdx == eCamera_FrontPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL);	// front preview
			else if(scene->elem->selIdx == eCamera_LeftPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_LEFT_BYPASS, PP_NULL, PP_NULL);	// left preview
			else if(scene->elem->selIdx == eCamera_RightPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_RIGHT_BYPASS, PP_NULL, PP_NULL);	// right preview
			else if(scene->elem->selIdx == eCamera_RearPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_REAR_BYPASS, PP_NULL, PP_NULL);	// rear preview
			else if(scene->elem->selIdx == eCamera_QuadPreview)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PP_NULL, PP_NULL);	// quad preview
			else
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL);	// front preview

			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			break;

		case CMD_UI_KEY_LEFT:
			break;

		case CMD_UI_KEY_RIGHT:
			break;

		case CMD_UI_KEY_MENU:
		
#ifdef CALIB_LIB_USE
			AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
#endif
			break;
	}
}

PP_VOID PPAPP_UI_CameraPreview (void* IN pArg, PP_U32 IN event)
{
//    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPLAY_Background_Off();
			break;
			
		case CMD_UI_KEY_CENTER:
			AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			break;
	}
}

PP_VOID PPAPP_UI_CameraCapture (void* IN pArg, PP_U32 IN event)
{
//    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
		{
			PP_U32 ch = 0;
			PP_U32 path = 0;	//0:input, 1:TP
			PP_U32 u32BufPAddr = 0x0;
			PP_U32 u32BufSize = 0;
			PP_RESULT_E status = 0;
			PP_S32 size;
			PP_VOID *pfHandle_YUVimage = NULL;
			PP_S32 bYonly = 1; //0:YUV, 1:Y only

			PP_U32 prgLvUnit = 10 / ((eVIEWMODE_BASIC_REAR_BYPASS - eVIEWMODE_BASIC_FRONT_BYPASS) + 1);
			PP_U32 prgLv = 0;
			
			PPAPI_DISPLAY_DisableAll();

			SetProgressBarLevel(0);
			AppTask_SendCmd(CMD_DISPLAY_PROGRESSBAR_TIMER_ON, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
			SetProgressBarLevel(1);
			
			for(ch=eVIEWMODE_BASIC_FRONT_BYPASS ; ch<=eVIEWMODE_BASIC_REAR_BYPASS ; ch++)	// front -> left -> right -> rear
			{
				// Preview
				PPAPI_SVM_SetView(ch, PP_NULL, PP_NULL);
				PPAPI_DISPLAY_Background_Off();

				// Capture YUV Image
				u32BufPAddr = (PP_U32)gpVPUConfig->pBufCapture[0];

				if( PPAPI_VIN_SetCaptureMode(BD_VIN_FMT, ch, path) != eSUCCESS )
                {
                    LOG_WARNING("[(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                    return;
                }

                prgLv = (prgLvUnit * (ch - eVIEWMODE_BASIC_FRONT_BYPASS)) + (prgLvUnit / 2) + 1;
                SetProgressBarLevel(prgLv);
				
				if( PPAPI_VIN_GetCaptureImage(BD_VIN_FMT, bYonly, u32BufPAddr, &u32BufSize, QUAD_0CH, PP_TRUE) != eSUCCESS )
                {
                    LOG_WARNING("[(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
                    return;
                }
				LOG_DEBUG("pAddr[0x%08x (0x%08x)], bufsize[0x%08x]\n", u32BufPAddr, u32BufPAddr+u32BufSize, u32BufSize);

				// Save YUV to SD
				status = PPAPI_FATFS_DelVolume();
				LOG_DEBUG("PPAPI_FATFS_DelVolume status = 0x%x\n", status);
				if(PPAPI_FATFS_InitVolume())
				{
					LOG_DEBUG("error %d\n",PPAPI_FATFS_GetLastError());
					// Error
				}

				if(ch == eVIEWMODE_BASIC_FRONT_BYPASS)		// front
					pfHandle_YUVimage = PPAPI_FATFS_Open("Capture_Front.yuv", "w");
				else if(ch == eVIEWMODE_BASIC_LEFT_BYPASS)	// left
					pfHandle_YUVimage = PPAPI_FATFS_Open("Capture_Left.yuv", "w");
				else if(ch == eVIEWMODE_BASIC_RIGHT_BYPASS)	// right
					pfHandle_YUVimage = PPAPI_FATFS_Open("Capture_Right.yuv", "w");
				else if(ch == eVIEWMODE_BASIC_REAR_BYPASS)	// rear
				pfHandle_YUVimage = PPAPI_FATFS_Open("Capture_Rear.yuv", "w");
				
				LOG_DEBUG("pfHandle_YUVimage = 0x%x\n", pfHandle_YUVimage);
				
				size = PPAPI_FATFS_Write((PP_VOID *)u32BufPAddr, 1, u32BufSize,pfHandle_YUVimage);
				LOG_DEBUG("PPAPI_FATFS_Write status = 0x%x\n", size);
				
				status = PPAPI_FATFS_Close(pfHandle_YUVimage);
				LOG_DEBUG("PPAPI_FATFS_Close status = 0x%x\n", status);

				prgLv = (prgLvUnit * (ch - eVIEWMODE_BASIC_FRONT_BYPASS)) + (prgLvUnit) + 1;
				SetProgressBarLevel(prgLv);

				// Display Pop-Up
				if(status == eSUCCESS)
					PPAPI_DISPLAY_POPUP_On(ePopUp_SaveDone);
				else
					PPAPI_DISPLAY_POPUP_On(ePopUp_NoSaveDone);
				OSAL_sleep(1000);
				PPAPI_DISPLAY_POPUP_Off();
				
			}
			SetProgressBarLevel(10);
			AppTask_SendCmd(CMD_DISPLAY_PROGRESSBAR_TIMER_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
			
			AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
		}
		break;
	}
}

PP_VOID PPAPP_UI_CalibSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eCalib_Auto;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eCalib_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			else if(scene->elem->selIdx == eCalib_Auto)
			{
				AppTask_SendCmd(CMD_UI_DIALOG, TASK_UI, TASK_UI, eDialog_RunMenu, NULL_PTR, 0, 1000);
			}
			else
			{
				AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, scene->elem->selIdx, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			break;

		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_LEFT:
			break;

		case CMD_UI_KEY_RIGHT:
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

STATIC PP_VOID PPAPP_UI_InitManualCalibSetting (PP_VIEWMODE_E IN view)
{
	PP_SCENE_S *scene = PPAPP_UI_GetScene(eScene_ManualCalib);
	
	PPAPI_DISPLAY_DisableAll();
	PPAPI_DISPLAY_Background_Off();
	PPAPI_DISPlAY_UpdateLUT();			
	PPAPI_DISPLAY_MenuList(scene->elem->id);
	PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
	PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
}

PP_VOID PPAPP_UI_ManualCalibSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eManualCalib_Front;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			
			if(scene->elem->selIdx == eManualCalib_Front)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PPAPP_UI_InitManualCalibSetting, PP_NULL);	// front preview
			else if(scene->elem->selIdx == eManualCalib_Left)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_LEFT_BYPASS, PPAPP_UI_InitManualCalibSetting, PP_NULL);	// left preview
			else if(scene->elem->selIdx == eManualCalib_Right)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_RIGHT_BYPASS, PPAPP_UI_InitManualCalibSetting, PP_NULL);	// right preview
			else if(scene->elem->selIdx == eManualCalib_Rear)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_REAR_BYPASS, PPAPP_UI_InitManualCalibSetting, PP_NULL);	// rear preview
			else if(scene->elem->selIdx == eManualCalib_ViewGen)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PPAPP_UI_InitManualCalibSetting, PP_NULL);	// preview
			else
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PPAPP_UI_InitManualCalibSetting, PP_NULL);	// preview
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eManualCalib_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			else
			{
				AppTask_SendCmd(CMD_UI_DIALOG, TASK_UI, TASK_UI, eDialog_RunMenu, NULL_PTR, 0, 1000);
			}
			break;

		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;

			if(scene->elem->selIdx == eManualCalib_Front)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL);		// front preview
			else if(scene->elem->selIdx == eManualCalib_Left)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_LEFT_BYPASS, PP_NULL, PP_NULL);		// left preview
			else if(scene->elem->selIdx == eManualCalib_Right)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_RIGHT_BYPASS, PP_NULL, PP_NULL);		// right preview
			else if(scene->elem->selIdx == eManualCalib_Rear)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_REAR_BYPASS, PP_NULL, PP_NULL);		// rear preview
			else if(scene->elem->selIdx == eManualCalib_ViewGen)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PP_NULL, PP_NULL);	// preview
			
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;

			if(scene->elem->selIdx == eManualCalib_Front)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_FRONT_BYPASS, PP_NULL, PP_NULL);		// front preview
			else if(scene->elem->selIdx == eManualCalib_Left)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_LEFT_BYPASS, PP_NULL, PP_NULL);		// left preview
			else if(scene->elem->selIdx == eManualCalib_Right)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_RIGHT_BYPASS, PP_NULL, PP_NULL);		// right preview
			else if(scene->elem->selIdx == eManualCalib_Rear)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_REAR_BYPASS, PP_NULL, PP_NULL);		// rear preview
			else if(scene->elem->selIdx == eManualCalib_ViewGen)
				PPAPI_SVM_SetView(eVIEWMODE_BASIC_QUAD, PP_NULL, PP_NULL);	// preview

			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			break;

		case CMD_UI_KEY_LEFT:
			if( (scene->elem->selIdx == eManualCalib_Front) || (scene->elem->selIdx == eManualCalib_Right) || (scene->elem->selIdx == eManualCalib_Left) || (scene->elem->selIdx == eManualCalib_Rear) )
			{
				if(*scene->elem->subElem[scene->elem->selIdx].value > 0)
					*scene->elem->subElem[scene->elem->selIdx].value = (*scene->elem->subElem[scene->elem->selIdx].value - 1) % scene->elem->subElem[scene->elem->selIdx].valueNum;
				else
					*scene->elem->subElem[scene->elem->selIdx].value = eManualCalib_View_Max - 1;
				PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			}
			break;

		case CMD_UI_KEY_RIGHT:
			if( (scene->elem->selIdx == eManualCalib_Front) || (scene->elem->selIdx == eManualCalib_Right) || (scene->elem->selIdx == eManualCalib_Left) || (scene->elem->selIdx == eManualCalib_Rear) )
			{
				*scene->elem->subElem[scene->elem->selIdx].value = (*scene->elem->subElem[scene->elem->selIdx].value + 1) % scene->elem->subElem[scene->elem->selIdx].valueNum;
				PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			}
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}
#ifdef CALIB_LIB_USE
extern PP_OFFCALIB_OUT_INFO_S Off_Calib_Out;
#endif
PP_VOID PPAPP_UI_MovePoint (void* IN pArg, PP_U32 IN event)
{
    //PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	STATIC PP_BOOL menuDepth = 0;
	STATIC PP_U32 selPntIdx = 0;
	
	LOG_DEBUG("[%s] %d\n", __FUNCTION__, event);

	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			PPAPI_DISPLAY_DisableAll();
			
			menuDepth = 0;
			selPntIdx = 0;
			PPAPI_DISPLAY_CALIB_Point(apps_pos_point, apps_pos_point_num);
			PPAPI_DISPLAY_CALIB_SelPoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(menuDepth)		// move point
			{
				menuDepth = 0;
				PPAPI_DISPLAY_CALIB_SelPoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			else				// sel point
			{
				menuDepth = 1;
				PPAPI_DISPLAY_CALIB_MovePoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			break;

		case CMD_UI_KEY_UP:
			if(menuDepth)		// move point
			{
				if(apps_pos_point[selPntIdx].u16Y > 0)
					apps_pos_point[selPntIdx].u16Y--;
				#ifdef CALIB_LIB_USE
				Off_Calib_Out.stPcv_Param.stPatternParam[Off_Calib_Out.eCh_sel].featureIpt[selPntIdx*2+1] = (PP_F32)apps_pos_point[selPntIdx].u16Y;
				Off_Calib_Out	.u32Try_cnt=1;
				#endif
				PPAPI_DISPLAY_CALIB_MovePoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			break;

		case CMD_UI_KEY_DOWN:
			if(menuDepth)		// move point
			{
				if(apps_pos_point[selPntIdx].u16Y < 720)	// to be modified.
					apps_pos_point[selPntIdx].u16Y++;
				#ifdef CALIB_LIB_USE
				Off_Calib_Out.stPcv_Param.stPatternParam[Off_Calib_Out.eCh_sel].featureIpt[selPntIdx*2+1] = (PP_F32)apps_pos_point[selPntIdx].u16Y;
				Off_Calib_Out	.u32Try_cnt=1;
				#endif
				PPAPI_DISPLAY_CALIB_MovePoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			break;

		case CMD_UI_KEY_LEFT:
			if(menuDepth)		// move point
			{
				if(apps_pos_point[selPntIdx].u16X > 0)
					apps_pos_point[selPntIdx].u16X--;
				#ifdef CALIB_LIB_USE
				Off_Calib_Out.stPcv_Param.stPatternParam[Off_Calib_Out.eCh_sel].featureIpt[selPntIdx*2] = (PP_F32)apps_pos_point[selPntIdx].u16X;
				Off_Calib_Out	.u32Try_cnt=1;
				#endif

				PPAPI_DISPLAY_CALIB_MovePoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			else				// change point
			{
				if(selPntIdx > 0)
					selPntIdx--;
				else
					selPntIdx = apps_pos_point_num - 1;
				PPAPI_DISPLAY_CALIB_Point(apps_pos_point, apps_pos_point_num);
				PPAPI_DISPLAY_CALIB_SelPoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			break;

		case CMD_UI_KEY_RIGHT:
			if(menuDepth)		// move point
			{
				if(apps_pos_point[selPntIdx].u16X < 1280)	// to be modified.
					apps_pos_point[selPntIdx].u16X++;
				#ifdef CALIB_LIB_USE
				Off_Calib_Out.stPcv_Param.stPatternParam[Off_Calib_Out.eCh_sel].featureIpt[selPntIdx*2] = (PP_F32)apps_pos_point[selPntIdx].u16X;
				Off_Calib_Out	.u32Try_cnt=1;
				#endif

				PPAPI_DISPLAY_CALIB_MovePoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			else				// change point
			{
				if(selPntIdx < (apps_pos_point_num - 1))
					selPntIdx++;
				else
					selPntIdx = 0;
				
				PPAPI_DISPLAY_CALIB_Point(apps_pos_point, apps_pos_point_num);
				PPAPI_DISPLAY_CALIB_SelPoint(apps_pos_point, apps_pos_point_num, selPntIdx);
			}
			break;

		case CMD_UI_KEY_MENU:
			if(!menuDepth)
			{
				
				#ifdef CALIB_LIB_USE
				PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_GET_CAMERA_POSITION);
				AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_UI, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
				#endif
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			break;
	}
}
extern PP_U8 gMulticore_Viewgen_Flag;
PP_VOID PPAPP_UI_ViewGeneration (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPLAY_Background_Off();
			PPAPI_DISPlAY_UpdateLUT();
			
		#ifdef CALIB_LIB_USE
			AppTask_SendCmd(CMD_VIEWGEN_START, TASK_UI, TASK_CALIBRATION, eSVIEWGEN_START, NULL_PTR, 0, 1000);
		#endif
			break;
	}
}

PP_VOID PPAPP_UI_UpdateSetting (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] %d\n", __FUNCTION__, event);
	PP_RESULT_E result;
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eUpdate_FactoryReset;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eUpdate_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			else if(scene->elem->selIdx == eUpdate_FactoryReset)
			{
				PPAPP_Menu_ResetData();
				PPAPP_Menu_SetData();
				result = PPAPP_Menu_SaveData();

				if(result == eSUCCESS)
					AppTask_SendCmd(CMD_DISPLAY_POPUP_TIMER, TASK_UI, TASK_DISPLAY, ePopUp_Done, NULL_PTR, 0, 1000);
				else
					AppTask_SendCmd(CMD_DISPLAY_POPUP_TIMER, TASK_UI, TASK_DISPLAY, ePopUp_Failed, NULL_PTR, 0, 1000);
			}
			else if(scene->elem->selIdx == eUpdate_FwUpdate)
			{
				AppTask_SendCmd(CMD_UI_DIALOG, TASK_UI, TASK_UI, eDialog_Update, NULL_PTR, 0, 1000);
			}
			break;

		case CMD_UI_KEY_UP:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			if(scene->elem->selIdx > 0)
				scene->elem->selIdx--;
			else
				scene->elem->selIdx = scene->elem->subNum - 1;
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			break;

		case CMD_UI_KEY_DOWN:
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_FALSE);
			if(scene->elem->selIdx < (scene->elem->subNum-1))
				scene->elem->selIdx++;
			else
				scene->elem->selIdx = 0;
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE); 
			break;
		case CMD_UI_KEY_LEFT:
			break;

		case CMD_UI_KEY_RIGHT:
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

PP_VOID PPAPP_UI_SystemInfo (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			//scene->elem->selIdx = eSystemInfo_Back;
			PPAPI_DISPLAY_ParsingImage(scene->elem->id);
			PPAPI_DISPLAY_DisableAll();
			PPAPI_DISPlAY_UpdateLUT();
			PPAPI_DISPLAY_Background_On();
			PPAPI_DISPLAY_MenuList(scene->elem->id);
			PPAPI_DISPLAY_SubMenuList(scene->elem->id, (PP_SCENE_SUB_ELEM_S*)&scene->elem->subElem, scene->elem->selIdx);
			PPAPI_DISPLAY_MenuItem(scene->elem->id, scene->elem->selIdx, PP_TRUE);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(scene->elem->selIdx == eSystemInfo_Back)
			{
				AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
			}
			break;

		case CMD_UI_KEY_UP:
			break;

		case CMD_UI_KEY_DOWN:
			break;

		case CMD_UI_KEY_LEFT:
			break;

		case CMD_UI_KEY_RIGHT:
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

PP_VOID PPAPP_UI_Dialog (void* IN pArg, PP_U32 IN event)
{
    PP_SCENE_S *scene = (PP_SCENE_S *)pArg;
	PP_SCENE_S *prevScene = NULL_PTR;
	#ifdef CALIB_LIB_USE
	PP_OFFCALIB_CAMERA_CAPTURE_CH_E ch;
	#endif
	LOG_DEBUG("[%s] evt(%d)\n", __FUNCTION__, event);
	
	switch(event)
	{
		case CMD_UI_INIT_SCENE:
			*scene->elem->subElem[scene->elem->selIdx].value = eDialog_Btn_Cancel;
			PPAPI_DISPLAY_DIALOG_On(scene->elem->selIdx, *scene->elem->subElem[scene->elem->selIdx].value);
			break;
			
		case CMD_UI_KEY_CENTER:
			if(*scene->elem->subElem[scene->elem->selIdx].value == eDialog_Btn_Ok)
			{
				switch(scene->elem->selIdx)
				{
					case eDialog_ManualCalib:
						AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, 1, NULL_PTR, 0, 1000);	// attr: (0:manual calib) (1:move point) (2:view generation)
						AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
						break;

					case eDialog_SaveView:
						PPAPI_DISPLAY_DIALOG_Off();
						
						AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_DoNotOff, NULL_PTR, 0, 1000);
						
						AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_ShutDown, NULL_PTR, 0, 1000);
						OSAL_sleep(1000);	// replace "reboot (system reset)"
						AppTask_SendCmd(CMD_DISPLAY_POPUP_OFF, TASK_UI, TASK_DISPLAY, 0, NULL_PTR, 0, 1000);
						
						#ifdef CALIB_LIB_USE	
						AppTask_SendCmd(CMD_VIEWGEN_START, TASK_UI, TASK_CALIBRATION, eSVIEWGEN_END, NULL_PTR, 0, 1000);
						#endif
						//TODO: add save sequence 
						AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);	// To be deleted
						AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);	// To be deleted

						break;

					case eDialog_RunMenu:
					case eDialog_CalibFailRetry:
						PPAPI_DISPLAY_DisableAll();
						prevScene = scene->prev;
						if(prevScene->elem->id == eScene_CalibSetting)	// auto calibration
						{
							PPAPI_DISPLAY_DisableAll();
							
							PPAPI_DISPLAY_POPUP_On(ePopUp_Calibration);
						#ifdef CALIB_LIB_USE
							PPAPP_Calib_SearchPoint(eCALIB_CAMERA_ALL);		
						#endif
							
						}
						else if(prevScene->elem->id == eScene_ManualCalib)
						{
							switch(prevScene->elem->selIdx)
							{
								case eManualCalib_ViewGen:
									AppTask_SendCmd(CMD_UI_NEXT_SCENE, TASK_UI, TASK_UI, 2, NULL_PTR, 0, 1000);	// attr: (0:manual calib) (1:move point) (2:view generation)
									AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
									break;

								case eManualCalib_Front:
								case eManualCalib_Right:
								case eManualCalib_Left:
								case eManualCalib_Rear:
								#ifdef CALIB_LIB_USE
									if(prevScene->elem->selIdx == eManualCalib_Front)
										ch = eCALIB_CAMERA_FRONT;
									else if(prevScene->elem->selIdx == eManualCalib_Right)
										ch = eCALIB_CAMERA_RIGHT;
									else if(prevScene->elem->selIdx == eManualCalib_Left)
										ch = eCALIB_CAMERA_LEFT;
									else if(prevScene->elem->selIdx == eManualCalib_Rear)
										ch = eCALIB_CAMERA_BACK;
									
									PPAPP_Calib_SearchPoint(ch);
								#endif
									break;
							}
						}
						break;

					case eDialog_Update:
						{
#if(UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)
							FWDN_FLASH_UPDATE_S stFWDNParam[FLASH_UPDATE_SECTION_MAX];
							PP_U32 u32ElementCnt;

							PPAPI_DISPLAY_DIALOG_Off();

							if(PPAPP_UPGRADE_GetParam(stFWDNParam, FLASH_UPDATE_SECTION_MAX, &u32ElementCnt) == eSUCCESS){
								AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_DoNotOff, NULL_PTR, 0, 1000);
								AppTask_SendCmd(CMD_UI_FWDN, TASK_UI, TASK_UI, 0, stFWDNParam, sizeof(FWDN_FLASH_UPDATE_S)*u32ElementCnt, 3000);
							}else{
								LOG_DEBUG("Can not upgrade firmware\n");
								AppTask_SendCmd(CMD_DISPLAY_POPUP_TIMER, TASK_UI, TASK_DISPLAY, ePopUp_NoUpdate, NULL_PTR, 0, 1000);
    							AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);

							}
#elif(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
							PPAPI_DISPLAY_DIALOG_Off();

							AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_UI, TASK_DISPLAY, ePopUp_DoNotOff, NULL_PTR, 0, 1000);
							AppTask_SendCmd(CMD_UI_FWDN, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 3000);
							
#endif
						}
						break;
				}
			}
			else if(*scene->elem->subElem[scene->elem->selIdx].value == eDialog_Btn_Cancel)
			{
				if(scene->elem->selIdx == eDialog_RunMenu)
				{
					PPAPI_DISPLAY_DIALOG_Off();
					AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				}
				else if(scene->elem->selIdx == eDialog_SaveView)
				{	
				#ifdef CALIB_LIB_USE
					AppTask_SendCmd(CMD_VIEWGEN_START, TASK_UI, TASK_CALIBRATION, eSVIEWGEN_END, NULL_PTR, 0, 1000);
				#endif
					AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					
				}
				else if(scene->elem->selIdx == eDialog_Update)
				{
					PPAPI_DISPLAY_DIALOG_Off();
					AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				}
				else if(scene->elem->selIdx == eDialog_ManualCalib)
				{
					PPAPI_DISPLAY_DIALOG_Off();
					#ifdef CALIB_LIB_USE
					PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_END);
					AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_UI, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
					#endif
					AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);

				}
				else if(scene->elem->selIdx == eDialog_CalibFailRetry)
				{
					PPAPI_DISPLAY_DIALOG_Off();
					#ifdef CALIB_LIB_USE
					//PPAPI_Offcalib_Set_Step_Send_Cmd(eOFFCALIB_END);
					//AppTask_SendCmd(CMD_OFF_CALIB_START, TASK_UI, TASK_CALIBRATION, PPAPI_Offcalib_Get_Step_Send_Cmd(), NULL_PTR, 0, 1000);
					#endif
					AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);

				}
				else
				{
					AppTask_SendCmd(CMD_UI_PREV_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
					AppTask_SendCmd(CMD_UI_INIT_SCENE, TASK_UI, TASK_UI, 0, NULL_PTR, 0, 1000);
				}
			}
			else
			{
				LOG_DEBUG("[ERROR] INVALID_ARGUMENT!\n");
			}
			break;

		case CMD_UI_KEY_UP:
			break;

		case CMD_UI_KEY_DOWN:
			break;

		case CMD_UI_KEY_LEFT:
			if(*scene->elem->subElem[scene->elem->selIdx].value > 0)
				(*scene->elem->subElem[scene->elem->selIdx].value)--;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = scene->elem->subElem[scene->elem->selIdx].valueNum - 1;
			PPAPI_DISPLAY_DIALOG_On(scene->elem->selIdx, *scene->elem->subElem[scene->elem->selIdx].value);
			break;

		case CMD_UI_KEY_RIGHT:
				if(*scene->elem->subElem[scene->elem->selIdx].value < (scene->elem->subElem[scene->elem->selIdx].valueNum - 1))
				(*scene->elem->subElem[scene->elem->selIdx].value)++;
			else
				*scene->elem->subElem[scene->elem->selIdx].value = 0;
			PPAPI_DISPLAY_DIALOG_On(scene->elem->selIdx, *scene->elem->subElem[scene->elem->selIdx].value);
			break;

		case CMD_UI_KEY_MENU:
			break;
	}
}

#endif // USE_PP_GUI

//  UPGRADE ---------------------------------------------------------------------------
#if (UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)

#define ISP_FW_NAME_PREFIX				"isp_firm"
#define MAIN_FW_NAME_PREFIX				"main_firm"
#define SVM_LUT_NAME_PREFIX				"svm_lut"
#define MENU_DATA_NAME_PREFIX			"menu_data"
#define GUI_IMG_NAME_PREFIX				"ui_image"
#define CALIB_DATA_MAIN_NAME_PREFIX		"SVMConfig_Main"
#define CALIB_DATA_SUB_NAME_PREFIX		"SVMConfig_Sub"

typedef struct ppFWUP_ELEMENT_S
{
	PP_CHAR *szFileName;
	PP_U32 u32UpgradeSection;
}PP_FWUP_ELEMENT_S;

static PP_U32 PPAPP_UPGRADE_GetVersion(PP_CHAR *pszfilename)
{
	PP_CHAR *ptr1 = strstr(pszfilename, "Ver" );
	PP_CHAR *ptr2;
	PP_S32 num[4];
	PP_S32 i;
	PP_U32 version;
	PP_CHAR szVersion[256];

	if( ptr1 == NULL )
	{
		version = 1;
		return version;
	}

	strcpy(szVersion, ptr1);
	ptr2 = strtok(szVersion, "-");

	if(!(sscanf(ptr2, "Ver%d.%d.%d.%d", &num[0], &num[1], &num[2], &num[3])))
	{
		version = 1;
		return version;
	}

	for( i = 0; i < 4; i++ )
	{
		if( num[i] > 255 )
		{
			version = 1;
			return version;
		}
		else
		{
			if( i == 3 )
			{
				if( num[i] > 254 )
				{
					version = 1;
					return version;
				}
			}
		}
	}

	version =  num[0]<<24;
	version += num[1]<<16;
    version += num[2]<<8;
    version += num[3]<<0;

	if( version == 0 )
		version = 1;

	return version;
}

//static void SendFWUpgradeCmd(void)
PP_RESULT_E PPAPP_UPGRADE_GetParam(PP_VOID IN *pParam, PP_U32 IN u32ElementMax, PP_U32 OUT *pu32ElementCnt)
{
    PP_U32 u32ElementCnt = 0;
    PP_S32 i;
    PP_RESULT_E enRet = eERROR_FAILURE;
    FWDN_FLASH_UPDATE_S IN *pstFWDNParam = (FWDN_FLASH_UPDATE_S *)pParam;

    memset(pstFWDNParam, 0, sizeof(FWDN_FLASH_UPDATE_S)*u32ElementMax);
    *pu32ElementCnt = 0;

	PPAPI_FATFS_DelVolume();
	if(PPAPI_FATFS_InitVolume() == eSUCCESS){
		PP_FNFIND_S stFind;
		PP_U8 u8FileExist = 0;

		PP_FWUP_ELEMENT_S stElement[] = {
				{.szFileName = MAIN_FW_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_MAIN_FW },
				{.szFileName = ISP_FW_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_ISP_FW },
				{.szFileName = SVM_LUT_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_SVM_LUT },
				{.szFileName = MENU_DATA_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_MENU },
				{.szFileName = GUI_IMG_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_GUI_IMG },
				{.szFileName = CALIB_DATA_MAIN_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_CALIB_MAIN },
				{.szFileName = CALIB_DATA_SUB_NAME_PREFIX, .u32UpgradeSection = FLASH_UPDATE_CALIB_SUB },
		};

		if(sizeof(stElement)/sizeof(stElement[0]) > u32ElementMax)
			return eERROR_FAILURE;

		for(i=0;i<sizeof(stElement)/sizeof(stElement[0]);i++){
			u8FileExist = 0;
			if(PPAPI_FATFS_FindFirst("*.bin", &stFind) == eSUCCESS){
				do{
					if(strncmp(stFind.szFileName, stElement[i].szFileName, strlen(stElement[i].szFileName)) == 0){
						u8FileExist = 1;
						break;
					}
				}while(!PPAPI_FATFS_FindNext(&stFind));
			}
			if(u8FileExist){
				strcpy(pstFWDNParam[u32ElementCnt].szFileName, stFind.szFileName);
				pstFWDNParam[u32ElementCnt].u32UpdateSection = stElement[i].u32UpgradeSection;
				pstFWDNParam[u32ElementCnt].u32Verify = 1;
				pstFWDNParam[u32ElementCnt].u32Version = PPAPP_UPGRADE_GetVersion(stFind.szFileName);
				u32ElementCnt++;
			}
		}

		*pu32ElementCnt = u32ElementCnt;
		if(u32ElementCnt){
			enRet = eSUCCESS;
		}else{
			enRet = eERROR_FAILURE;
		}

	}else{
		LOG_CRITICAL("Filesystem initvolume fail(%d)\n", PPAPI_FATFS_GetLastError());
		enRet = eERROR_FAILURE;
	}

	return enRet;
}
#endif


