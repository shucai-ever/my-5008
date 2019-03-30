// menu_data.c

#include <string.h>
#include "type.h"
#include "debug.h"
#include "menu_data.h"
#include "application.h"
#include "api_flash.h"

#if defined(USE_PP_GUI)

PP_MENU_DATA_S gMenuDataBackup __attribute__((section ("menu_data_backup"))) = {

					0xCAFE1234							// PP_U32	u32MagicNumber

					, ePgl_Always							// PP_U32	u32PGL
					, eDnmBlending_Off					// PP_U32	u32DnmBlending

					, eSignal_Off						// PP_U32	u32ReverseSignal
					, eReverse_View_Top3DRear			// PP_U32	u32ReserveViewMode
					, eSignal_Off						// PP_U32	u32TurnSignal
					, eTurn_View_Top3DLeftFront			// PP_U32	u32TurnViewMode
					, eSignal_Off						// PP_U32	u32EmergencySignal
					, eEmergency_View_Top3DFront		// PP_U32	u32EmergencyViewMode

					, eStepBox_5						// PP_U32	u32ScreenX
					, eStepBox_5						// PP_U32	u32ScreenY
					, eStepBox_5						// PP_U32	u32Brightness
					, eStepBox_5						// PP_U32	u32Contrast
					, eStepBox_5						// PP_U32	u32Saturation

					, eManualCalib_View_Normal			// PP_U32	u32FrontMirror
					, eManualCalib_View_Normal			// PP_U32	u32LeftMirror
					, eManualCalib_View_Normal			// PP_U32	u32RightMirror
					, eManualCalib_View_Normal			// PP_U32	u32RearMirror

					, eDialog_Btn_Cancel				// PP_U32	u32u32DialogBtn

					, 0									// PP_U32	u32CheckSum

					, {0,}								// PP_U32 reserved

}; // PP_MENU_DATA_S

PP_MENU_DATA_S gMenuData __attribute__((section ("menu_data")));

#endif
