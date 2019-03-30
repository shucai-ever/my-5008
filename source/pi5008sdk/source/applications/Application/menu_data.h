#ifndef __MENU_DATA_H__
#define __MENU_DATA_H__

#include <stddef.h>
#include "type.h"
#include "error.h"

#ifdef __cplusplus
EXTERN "C" {
#endif

#pragma pack(1)
typedef struct ppMENU_DATA_S {

	PP_U32	u32MagicNumber;

	PP_U32	u32PGL;
	PP_U32	u32DnmBlending;
	
	PP_U32	u32ReverseSignal;
	PP_U32	u32ReserveViewMode;
	PP_U32	u32TurnSignal;
	PP_U32	u32TurnViewMode;
	PP_U32	u32EmergencySignal;
	PP_U32	u32EmergencyViewMode;

	PP_U32	u32ScreenX;
	PP_U32	u32ScreenY;
	PP_U32	u32Brightness;
	PP_U32	u32Contrast;
	PP_U32	u32Saturation;

	PP_U32	u32FrontMirror;
	PP_U32	u32LeftMirror;
	PP_U32	u32RightMirror;
	PP_U32	u32RearMirror;

	PP_U32	u32DialogBtn;

	PP_U32	u32CheckSum;

	PP_U32 u32Reserved[1024-19];	// 4KB fix.

} PP_MENU_DATA_S;
#pragma pack()

#define MENU_DATA_SIZE				( sizeof(PP_MENU_DATA_S) )
#define MENU_DATA_VALID_SIZE		( offsetof(PP_MENU_DATA_S, u32Reserved) )
#define MENU_DATA_NUM				( (offsetof(PP_MENU_DATA_S, u32CheckSum) / sizeof(PP_U32)) + 1 )
#define MENU_DATA_MAGIC_NUMBER		(0xCAFE1234)

extern PP_MENU_DATA_S gMenuData;
extern PP_MENU_DATA_S gMenuDataBackup;

#ifdef __cplusplus
}
#endif

#endif // __MENU_DATA_H__
