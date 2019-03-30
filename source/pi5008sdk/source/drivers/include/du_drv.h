/* du_drv.h */
#ifndef __DU_DRV_H__
#define __DU_DRV_H__

#include "type.h"
#include "system.h"
#include "error.h"
#include "common.h"

#ifdef __cplusplus
EXTERN "C" {
#endif


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppDU_LAYER_E {
	eLayer0,
	eLayer1,
	eLayer2,
	eLayer3,
	eLayer4,
	
	eLayer_MAX
} PP_DU_LAYER_E;

typedef enum ppDU_AREA_E {
	eArea0,
	eArea1,
	eArea2,
	eArea3,
	
	eArea_MAX
} PP_DU_AREA_E;

typedef enum ppDU_FORMAT_E {
	eFORMAT_RLE,
	eFORMAT_INDEX,
	eFORMAT_RGB565,
	eFORMAT_RGB888,
	eFORMAT_ARGB8888,
	eFORMAT_RGBA4444,
	eFORMAT_1BIT,
	
	eFORMAT_MAX
} PP_DU_FORMAT_E;

typedef enum ppDU_VIDEO_PATH_E {
	ePATH_SVMOUT_BYPASS_OUT,
	ePATH_SVMIN0_BYPASS_OUT,
	ePATH_SVMIN1_BYPASS_OUT,
	ePATH_SVMIN2_BYPASS_OUT,
	ePATH_SVMIN3_BYPASS_OUT,
	ePATH_SVMOUT_MIXER_OUT,
	ePATH_SVMIN0_MIXER_OUT,
	ePATH_SVMIN1_MIXER_OUT,
	ePATH_SVMIN2_MIXER_OUT,
	ePATH_SVMIN3_MIXER_OUT,
	
	ePATH_VIDEO_MAX
} PP_DU_VIDEO_PATH_E;

typedef enum ppDU_MIXER_PATH_E {
	ePATH_IN_bypass_OUT,
	ePATH_IN_01234_OUT,
	ePATH_IN_12340_OUT,
	ePATH_IN_23401_OUT,
	ePATH_IN_34012_OUT,
	ePATH_IN_40123_OUT,
	ePATH_IN_43210_OUT,
	ePATH_IN_32104_OUT,
	ePATH_IN_21043_OUT,
	ePATH_IN_10432_OUT,
	ePATH_IN_04321_OUT,
	ePATH_IN_32140_OUT,
	
	ePATH_MIXER_MAX
} PP_DU_MIXER_PATH_E;

typedef enum ppDU_RESOLUTION_E {
	eRESOLUTION_720Hi_NTSC,
	eRESOLUTION_720Hi_PAL,
	eRESOLUTION_960Hi_NTSC,
	eRESOLUTION_960Hi_PAL,
	eRESOLUTION_720Hp_NTSC,
	eRESOLUTION_720Hp_PAL,
	eRESOLUTION_960Hp_NTSC,
	eRESOLUTION_960Hp_PAL,
	eRESOLUTION_WVGA,		// HD:800x480
	eRESOLUTION_WSVGA,		// HD:1024x600
	eRESOLUTION_720P,
	eRESOLUTION_960P,
	eRESOLUTION_1080P,
	
	eRESOLUTION_MAX
} PP_DU_RESOLUTION_E;


typedef enum ppDU_SYNC_E {
	eSYNC_EMBEDDED,
	eSYNC_EXTERNAL,
	
	eSYNC_MAX
} PP_DU_SYNC_E;

typedef enum ppDU_BAYER_BIT_E {
	eBAYER_8BIT,
	eBAYER_10BIT,
	
	eBAYER_BIT_MAX
} PP_DU_BAYER_BIT_E;

typedef enum ppDU_CSC_E {
	eCSC_BYPASS,
	eCSC_BT601_to_BT709,
	eCSC_BT709_to_BT601,
	
	eCSC_MAX
} PP_DU_CSC_E;

typedef enum ppPVITX_SRC {
	ePVITX_SRC_DU,
	ePVITX_SRC_QUAD,

	ePVITX_SRC_MAX
} PP_PVITX_SRC;

/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppDU_R2Y_COEF_S {
	PP_U32	y0;
	PP_U32	y1;
	PP_U32	y2;
	PP_U32	y3;
	PP_U32	u0;
	PP_U32	u1;
	PP_U32	u2;
	PP_U32	u3;
	PP_U32	v0;
	PP_U32	v1;
	PP_U32	v2;
	PP_U32	v3;
} PP_DU_R2Y_COEF_S;


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Variable
***************************************************************************************************************************************************************/
typedef PP_VOID (*PP_DU_CALLBACK) (PP_VOID);


/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
PP_U32 PPDRV_DU_GetVersion (PP_VOID);
PP_VOID PPDRV_DU_SetInterrupt (PP_DU_CALLBACK IN callback);
PP_VOID PPDRV_DU_SetVideoPath (PP_DU_VIDEO_PATH_E IN path);
PP_VOID PPDRV_DU_BTO_SetYC8 (PP_DU_RESOLUTION_E IN in_resolution, PP_DU_RESOLUTION_E IN out_resolution, PP_DU_SYNC_E IN sync);
PP_VOID PPDRV_DU_BTO_SetYC16 (PP_DU_RESOLUTION_E IN resolution, PP_DU_SYNC_E IN sync);
PP_VOID PPDRV_DU_BTO_SetRGB24 (PP_DU_RESOLUTION_E IN resolution);
PP_VOID PPDRV_DU_BTO_SetBayer (PP_DU_RESOLUTION_E IN resolution, PP_DU_BAYER_BIT_E IN bit);
PP_VOID PPDRV_DU_BTO_EnableBrightness (PP_BOOL IN enable);
PP_U8 PPDRV_DU_BTO_GetBrightness (PP_VOID);
PP_RESULT_E PPDRV_DU_BTO_SetBrightness (PP_U8 IN brt);
PP_VOID PPDRV_DU_BTO_EnableContrast (PP_BOOL IN enable);
PP_U8 PPDRV_DU_BTO_GetContrast (PP_VOID);
PP_RESULT_E PPDRV_DU_BTO_SetContrast (PP_U8 IN cont);
PP_VOID PPDRV_DU_BTO_EnableSaturation (PP_BOOL IN enable);
PP_U8 PPDRV_DU_BTO_GetSaturation (PP_VOID);
PP_RESULT_E PPDRV_DU_BTO_SetSaturation (PP_U8 IN sat);
PP_VOID PPDRV_DU_OSD_SetMixerPath (PP_DU_MIXER_PATH_E path);
PP_VOID PPDRV_DU_OSD_RunMixer (PP_VOID);
PP_RESULT_E PPDRV_DU_OSD_GetColorLut (PP_DU_LAYER_E IN layer, PP_U32* IN lut);
PP_RESULT_E PPDRV_DU_OSD_SetColorLut (PP_DU_LAYER_E IN layer, PP_U32* IN lut);
PP_VOID PPDRV_DU_OSD_SetLayerSize (PP_DU_LAYER_E IN layer, PP_RECT_S IN rect);
PP_RECT_S PPDRV_DU_OSD_GetLayerSize (PP_DU_LAYER_E IN layer);
PP_VOID PPDRV_DU_OSD_SetLayerColor (PP_DU_LAYER_E IN layer, PP_U32 IN color);
PP_U32 PPDRV_DU_OSD_GetLayercolor (PP_DU_LAYER_E IN layer);
PP_RESULT_E PPDRV_DU_OSD_SetLayerFormat (PP_DU_LAYER_E IN layer, PP_DU_FORMAT_E IN format);
PP_DU_FORMAT_E PPDRV_DU_OSD_GetLayerFormat (PP_DU_LAYER_E IN layer);
PP_VOID PPDRV_DU_OSD_EnableLayer2dDMA (PP_DU_LAYER_E IN layer, PP_BOOL IN enable);
PP_VOID PPDRV_DU_OSD_EnableLayerInterlace (PP_DU_LAYER_E IN layer, PP_BOOL IN enable);
PP_RESULT_E PPDRV_DU_OSD_EnableLayerGlobalAlpha (PP_DU_LAYER_E IN layer, PP_BOOL IN enable);
PP_RESULT_E PPDRV_DU_OSD_SetLayerGlobalAlpha (PP_DU_LAYER_E IN layer, PP_U8 IN value);
PP_RESULT_E PPDRV_DU_OSD_EnableArea (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_BOOL IN enable);
PP_RESULT_E PPDRV_DU_OSD_SetAreaConfig (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U32* IN addr, PP_U32* IN addr2, PP_U32 IN byte, PP_RECT_S IN rect, PP_U32 IN stride, PP_DU_FORMAT_E IN format);
PP_RESULT_E PPDRV_DU_OSD_SetArea (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U32* IN addr, PP_U32* IN addr2, PP_U32 IN byte, PP_U32 IN stride, PP_DU_FORMAT_E IN format);
PP_RESULT_E PPDRV_DU_OSD_SetAreaPosition (PP_DU_LAYER_E IN layer, PP_DU_AREA_E IN area, PP_U16 IN x, PP_U16 IN y, PP_U16 IN w, PP_U16 IN h);
PP_VOID PPDRV_DU_OSD_SetR2Y (PP_DU_LAYER_E IN layer, PP_U32 IN bypass, PP_DU_R2Y_COEF_S IN coef);
PP_VOID PPDRV_DU_PVITX_SelSrc (PP_PVITX_SRC IN src);
PP_U32 PPDRV_DU_OSD_GetFrameCnt (PP_VOID);
PP_VOID PPDRV_DU_SetTestPattern(PP_BOOL IN enb, PP_U8 IN mode, PP_U8 IN sel);
PP_VOID PPDRV_DU_BTO_SetCSC(PP_DU_CSC_E IN csc);
PP_VOID PPDRV_DU_WaitBlank (PP_VOID);
PP_VOID PPDRV_DU_WaitBlank_BT (PP_VOID);
PP_VOID PPDRV_DU_SkipVSync (PP_VOID);

// Used only for internal purposes.
PP_VOID PPDRV_DU_SWreset (PP_VOID);
PP_VOID PPDRV_DU_BTO_SetFreeze(PP_RECT_S IN dispRect);

#ifdef __cplusplus
}
#endif

#endif  // __DU_DRV_H__
