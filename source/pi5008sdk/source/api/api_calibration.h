#ifndef __API_CALIBRATION_H__
#define __API_CALIBRATION_H__
#ifdef CALIB_LIB_USE
#include "type.h"
#include "error.h"

#include "pcvOffLineCalib.h"
#include "pcvSvmView.h"
#include "viewmode_config.h"
#include "common.h"
#include "api_svm_mem.h"


#define CAPTURE_YONLY					(1)
#define CAPTURE_UYVY					(0)

typedef enum ppOFFCALIB_CAMERA_PATTERN_TYPE_SEL_E
{
	eCALIB_PATTERN_MAIN = 0,
	eCALIB_PATTERN_SUB,
	eMAX_CALIB_PATTERN
}PP_OFFCALIB_CAMERA_PATTERN_TYPE_SEL_E;


typedef enum ppOFFCALIB_CAMERA_CAPTURE_CH_E
{
	eCALIB_CAMERA_FRONT = 0,
	eCALIB_CAMERA_LEFT,
	eCALIB_CAMERA_RIGHT,
	eCALIB_CAMERA_BACK,
	eCALIB_CAMERA_ALL,
	eMAX_CALIB_CAMERA_CHANNEL   
}PP_OFFCALIB_CAMERA_CAPTURE_CH_E;

typedef enum ppOFFCALIB_PROCESS_STEP_E
{
	eOFFCALIB_WAIT_CMD=0, // 0x0
	eOFFCALIB_START, //0x01
	eOFFCALIB_CAPTURE,//0x02
	eOFFCALIB_GET_CNF,//0x03
	eOFFCALIB_GET_WOLRD_POINT,//0x04
	eOFFCALIB_FIND_PATTERN,//0x05
	eOFFCALIB_GET_INTRINSIC_PARAM,//0x06
	eOFFCALIB_GET_CAMERA_POSITION,//0x07
	eOFFCALIB_GET_FEATURE_POINT,//0x08
	eOFFCALIB_END,//0x09
} PP_OFFCALIB_PROCESS_STEP_E;


typedef enum ppSECTION_VIEWGEN_PROCESS_STEP_E
{
	eSVIEWGEN_WAIT_CMD=0, // 0x00
	eSVIEWGEN_START ,//0x01
	
	eSVIEWGEN_PREPARE_MAKE,// 0x02
	eSVIEWGEN_GET_FB_LUT,// 0x03
	eSVIEWGEN_GET_LR_LUT,// 0x04	
	eSVIEWGEN_UPDATE,// 0x05


	
	eSVIEWGEN_END, // 0x06
	
	
} PP_SECTION_VIEWGEN_PROCESS_STEP_E;


typedef struct ppOFFCALIB_CAP_YUV_INFO_S
{
	PP_U32 u32YuvBufAddr; 
	PP_U32 u32YuvBufSize; 
	PP_U8  u8Yonly; //0:UYVY 1:Y only 
} PP_OFFCALIB_CAP_YUV_INFO_S;


typedef struct ppOFFCALIB_CAPTURE_BUF_INFO_S
{	
	PP_U32 u32Capture_Buf_Addr[eMAX_CALIB_CAMERA_CHANNEL];
	PP_U32 u32Capture_Buf_Size[eMAX_CALIB_CAMERA_CHANNEL];
} PP_OFFCALIB_CAPTURE_BUF_INFO_S;


typedef struct ppOFFCALIB_PCV_PARAM_S
{
	PCV_OFF_LINE_CALIB_PATTERN_PARAM stPatternParam[eMAX_CALIB_CAMERA_CHANNEL];	
	PCV_OFF_LINE_CALIB_CAMERA_PARAM stCamParam[eMAX_CALIB_CAMERA_CHANNEL];
} PP_OFFCALIB_PCV_PARAM_S;


typedef struct ppOFFCALIB_OUT_INFO_S
{	
	PP_OFFCALIB_PROCESS_STEP_E eOffcalib_Progress_Step;
	PP_OFFCALIB_CAMERA_CAPTURE_CH_E eCh_sel;
	
	PP_U32 u32Try_cnt;

	PP_OFFCALIB_CAPTURE_BUF_INFO_S stOffcalib_Capture_Buf;

	PP_OFFCALIB_PCV_PARAM_S stPcv_Param;

} PP_OFFCALIB_OUT_INFO_S;




typedef struct ppVIEWGEN_PROCESS_INFO_S
{	
	PP_SECTION_VIEWGEN_PROCESS_STEP_E eSViewgen_Progress_Step;
	
} PP_VIEWGEN_PROCESS_INFO_S;



PP_VOID *pcvMalloc(PP_U32 IN size);
PP_VOID pcvFree(PP_VOID IN *ptr);
PP_VOID *PPAPI_Lib_Ext_Malloc(PP_U32 IN size);
PP_VOID PPAPI_Lib_Ext_Free(PP_VOID IN *ptr);
PP_VOID PPAPI_Lib_Debug_Memory(PP_VOID);
PP_OFFCALIB_CAMERA_PATTERN_TYPE_SEL_E PPAPI_Offcalib_Get_Pattern_Select(PP_VOID);
PP_RESULT_E PPAPI_Viewgen_Get_View_Blend_Param(PCV_SVM_VIEW_BLEND_PARAM IN *view_blend_param);
PP_RESULT_E PPAPI_Viewgen_Get_View_Sur_2D_Param(PCV_SVM_VIEW_SURFACE_2D_PARAM IN *view_sur_2d_param);
PP_RESULT_E PPAPI_Viewgen_Get_View_Sur_3D_Param(PCV_SVM_VIEW_SURFACE_3D_PARAM IN *view_sur_3d_param);
PP_RESULT_E PPAPI_Offcalib_Get_Cam_Intrinsic_Param(PCV_OFF_LINE_CALIB_CAMERA_PARAM IN * imp,PP_U8 IN camera_ch);
PP_VOID PPAPI_Offcalib_Debug_Print_FeaturePoint(PCV_OFF_LINE_CALIB_PATTERN_PARAM IN * pat_param);
PP_VOID PPAPI_Offcalib_Debug_Print_Position(PCV_OFF_LINE_CALIB_CAMERA_PARAM IN * imp);
PP_RESULT_E PPAPI_Offcalib_Get_Cnf(PCV_OFF_LINE_CALIB_PATTERN_PARAM  IN * pat_param,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN camera_ch);
PP_VOID PPAPI_Offcalib_Save_Cam_Ch(PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch);
PP_OFFCALIB_CAMERA_CAPTURE_CH_E PPAPI_Offcalib_Get_Cam_Ch(PP_VOID);
PP_VOID PPAPI_Offcalib_Set_Step_Send_Cmd(PP_OFFCALIB_PROCESS_STEP_E  IN step);
PP_OFFCALIB_PROCESS_STEP_E PPAPI_Offcalib_Get_Step_Send_Cmd(PP_VOID);
PP_VOID PPAPI_Offcalib_BMP_Image_Save_SD(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch);
PP_VOID PPAPI_Offcalib_Capture_YUV_Image_Save_SD(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch);
PP_VOID PPAPI_Offcalib_Capture_YUV_Image(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E  IN ch, PP_U8 bYonly);
PP_S32 PPAPI_Viewgen_Get_Cam_Param(PCV_SVM_VIEW_CAMERA_PARAM  IN * camera_param,PP_U8  IN camera_ch);
PP_VOID PPAPI_Offcalib_Capture_YUV_Imgae_Save_Buf(PP_OFFCALIB_CAP_YUV_INFO_S  IN *YuvBufInfo,PP_OFFCALIB_CAMERA_CAPTURE_CH_E	IN ch);
PP_RESULT_E PPAPI_Viewgen_Get_External_Value(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_Top_2d_FB_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_Top_2d_LR_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_3D_FB_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_SD_FB_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_UNDIS_FB_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_CYLIND_FB_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_3D_LR_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_SD_LR_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_UNDIS_LR_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Make_RS_CYLIND_LR_LUT(PP_VOID);
PP_VOID PPAPI_Viewgen_Free_Global_Values(PP_VOID);
PP_VOID PPAPI_Viewgen_Free_2DTOP_Values(PP_VOID);
PP_VOID PPAPI_Viewgen_Free_3D_Values(PP_VOID);
PP_VOID PPAPI_Viewgen_Free_SD_Values(PP_VOID);
PP_VOID PPAPI_Viewgen_Free_UNDIS_Values(PP_VOID);
PP_VOID PPAPI_Viewgen_Free_CYLIND_Values(PP_VOID);
PP_RESULT_E PPAPI_Section_Viewgen_Prepare_Make(PP_U32 section_num,PP_U32 swing_degree);
PP_VOID PPAPI_Section_Viewgen_Set_Step_Send_Cmd(PP_SECTION_VIEWGEN_PROCESS_STEP_E  IN step);
PP_SECTION_VIEWGEN_PROCESS_STEP_E PPAPI_Section_Viewgen_Get_Step_Send_Cmd(PP_VOID);
PP_VOID PPAPI_Section_Viewgen_Update(PP_U32 IN u32Section_num, PP_U32  IN FB_ADDR,PP_U32  IN LR_ADDR);
PP_VOID PPAPI_Lib_Progressbar_Processing(PP_VOID);
PP_VOID PPAPI_Lib_Calib_Initialize(PP_VOID);
PP_U32 PPAPI_Section_Viewgen_Get_ViewType(PP_U32 section_num);
PP_U32 PPAPI_Section_Viewgen_Get_CamType (PP_U32 section_num);
PP_VOID *pvMemMangCALIBMalloc( size_t xWantedSize );
PP_VOID vMemMangCALIBFree( PP_VOID *pv );
size_t xMemMangCALIBGetFreeSize( PP_VOID );
size_t xMemMangCALIBGetMinimumEverFreeSize( PP_VOID );
PP_VOID prvMemMangCALIBInit( PP_VOID );
void prvMemMangCALIBInit_All( void );
PP_VOID PPAPI_Lib_GetCameraProjectPoint(PP_VOID);
STATIC PP_BOOL PPAPI_Viewgen_SectionLUT_Update(PP_U32 IN u32SectionIndex, PP_SVMMEM_SECTION_DATA_E IN enSectionType, PP_U32* IN pu32SectionData, PP_U32 u32SectionDataSize);
STATIC PP_U32 PPAPI_Viewgen_SectionLUT_GetSamplingDataSize(PP_U16 IN u16SectionWidth, PP_U16 IN u16SectionHeight);





#ifdef __cplusplus
}
#endif





















#else//CALIB_LIB_USE



#include "type.h"
#include "error.h"








PP_VOID PPAPI_Lib_Calib_Initialize(PP_VOID);
PP_U32 PPAPI_Section_Viewgen_Get_ViewType(PP_U32 section_num);
PP_U32 PPAPI_Section_Viewgen_Get_CamType (PP_U32 section_num);



#endif ////CALIB_LIB_USE


typedef struct ppCNF_TAG_CAMERA_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;

	PP_U32 width;
	PP_U32 height;

	PP_F32 fx;
	PP_F32 fy;

	PP_F32 cx;
	PP_F32 cy;

	PP_U32 distortTableSize;
	PP_F32 distortTable[100];

	PP_F32 posx;
	PP_F32 posy;
	PP_F32 posz;

	PP_F32 angx;
	PP_F32 angy;
	PP_F32 angz;
	
	PP_U32 patternType;
	PP_F32 patternW;//width
	PP_F32 patternL;//length
	PP_F32 patternSW; //side width
	PP_F32 patternSL; //side length

	PP_F32 patternSquareSize;
	
	PP_U32 patternRoi0Left;
	PP_U32 patternRoi0Right;
	PP_U32 patternRoi0Top;
	PP_U32 patternRoi0Bottom;
	
	PP_U32 patternRoi1Left;
	PP_U32 patternRoi1Right;
	PP_U32 patternRoi1Top;
	PP_U32 patternRoi1Bottom;
	
	PP_U32 patternRoi2Left;
	PP_U32 patternRoi2Right;
	PP_U32 patternRoi2Top;
	PP_U32 patternRoi2Bottom;

	PP_U32 patternRoi3Left;
	PP_U32 patternRoi3Right;
	PP_U32 patternRoi3Top;
	PP_U32 patternRoi3Bottom;
	
}PP_CNF_TAG_CAMERA_PARAM_S;
typedef struct ppCNF_TAG_BLEND_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;

	PP_S32 shadowAreaFront;
	PP_S32 shadowAreaLeft;
	PP_S32 shadowAreaRear;
	PP_S32 shadowAreaRight;

	PP_S32 alpha0DivAngFront_2d;
	PP_S32 alpha0DivAngRear_2d;
	PP_S32 alpha0BlendAreaFront_2d;
	PP_S32 alpha0BlendAreaRear_2d;	

	PP_S32 alpha1DivAngFront_2d;
	PP_S32 alpha1DivAngRear_2d;
	PP_S32 alpha1BlendAreaFront_2d;
	PP_S32 alpha1BlendAreaRear_2d;
	
	PP_S32 alpha0DivAngFront_3d;
	PP_S32 alpha0DivAngRear_3d;
	PP_S32 alpha0BlendAreaFront_3d;
	PP_S32 alpha0BlendAreaRear_3d;

	PP_S32 alpha1DivAngFront_3d;
	PP_S32 alpha1DivAngRear_3d;
	PP_S32 alpha1BlendAreaFront_3d;
	PP_S32 alpha1BlendAreaRear_3d;	

}PP_CNF_TAG_BLEND_PARAM_S;
typedef struct ppCNF_TAG_2D_SURFACE_PARAM_S
{	
	PP_U32 tag;
	PP_U32 length;
	
	PP_U32 wx;
	PP_U32 wy;
	PP_U32 centerx;
	PP_U32 centery;

}PP_CNF_TAG_2D_SURFACE_PARAM_S;
typedef struct ppCNF_TAG_3D_SURFACE_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;

	PP_F32 wx;
	PP_F32 wy;
	PP_F32 wz;
	
	PP_F32 rx;
	PP_F32 ry;
	PP_F32 rz;
	
	PP_U32 zOffset;
	
	PP_U32 cx;
	PP_U32 cy;

}PP_CNF_TAG_3D_SURFACE_PARAM_S;

typedef struct ppCNF_TAG_CYLINDRICAL_SURFACE_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;
	
	PP_U32 wx;
	PP_U32 wy;
	PP_U32 dist;

}PP_CNF_TAG_CYLINDRICAL_SURFACE_PARAM_S;

typedef struct ppCNF_TAG_STATIC_PGL_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;
	
	PP_U32 numPoint;
	PP_U32 distMin;	
	PP_U32 distNearMax;
	PP_U32 distMiddleMax;	
	PP_U32 distFarMax;
	PP_U32 width;

}PP_CNF_TAG_STATIC_PGL_PARAM_S;
typedef struct ppCNF_TAG_DYNAMIC_PGL_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;
	
	PP_U32 numPoint;
	PP_U32 wheelBase;	
	PP_U32 wheelBaseCy;	
	PP_U32 distLRMin;
	PP_U32 distSideMin;	
	PP_U32 distLRMax;
	PP_U32 distSideMax;
	PP_U32 width;

}PP_CNF_TAG_DYNAMIC_PGL_PARAM_S;


typedef struct ppCNF_MD_SUBVIEW_PARAM_S
{
	PP_U32 viewType;	
	PP_U32 camType;
	PP_U32 scanType;	

	PP_U32 width;
	PP_U32 height;

	PP_F32 scaleX;
	PP_F32 scaleY;	
	
	PP_F32 virCamPosX;
	PP_F32 virCamPosY;
	PP_F32 virCamPosZ;
	
	PP_F32 virCamAngX;
	PP_F32 virCamAngY;
	PP_F32 virCamAngZ;
	
	PP_F32 virCamFocalLen;

	PP_U32 cx;
	PP_U32 cy;
	
	PP_U32 hFlip;

	PP_U32 ldc;
	
}PP_CNF_MD_SUBVIEW_PARAM_S;

typedef struct ppCNF_TAG_MD_VIEW_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;

	PP_U32 viewCount;
	
	PP_CNF_MD_SUBVIEW_PARAM_S subView[8]; // smoh check modif array num
	//PP_CNF_MD_SUBVIEW_PARAM_S *pSubView; 

}PP_CNF_TAG_MD_VIEW_PARAM_S;



typedef struct ppCNF_SWING_VIEW_PARAM_S
{
	PP_F32 virCamPosX;
	PP_F32 virCamPosY;
	PP_F32 virCamPosZ;
	PP_F32 virCamAngX;
	PP_F32 virCamAngY;
	PP_F32 virCamAngZ;

}PP_CNF_SWING_VIEW_PARAM_S;

typedef struct ppCNF_TAG_SWING_VIEW_PARAM_S
{
	PP_U32 tag;
	PP_U32 length;

	PP_U32 morphing;
	PP_U32 staticViewOnly;
	PP_U32 staticViewFBLRLut;
	PP_U32 nonftlSave;
	PP_U32 degreeInterval;
	PP_U32 flbInterval;
	PP_U32 staticView[360];
	PP_CNF_SWING_VIEW_PARAM_S CamParam[360];
	

}PP_CNF_TAG_SWING_VIEW_PARAM_S;



/* refer only below structure */
typedef struct ppCNF_TOTAL_BIN_FORMAT_S
{
	PP_CNF_TAG_CAMERA_PARAM_S frontCamera;	
	PP_CNF_TAG_CAMERA_PARAM_S rearCamera;
	PP_CNF_TAG_CAMERA_PARAM_S leftCamera;
	PP_CNF_TAG_CAMERA_PARAM_S rightCamera;
	
	PP_CNF_TAG_BLEND_PARAM_S blend;
	
	PP_CNF_TAG_2D_SURFACE_PARAM_S Surface_2d;
	
	PP_CNF_TAG_3D_SURFACE_PARAM_S Surface_3d;
	
	PP_CNF_TAG_CYLINDRICAL_SURFACE_PARAM_S Surface_Cylindrical;
	
	PP_CNF_TAG_STATIC_PGL_PARAM_S staticParkingGuide;
	
	PP_CNF_TAG_DYNAMIC_PGL_PARAM_S dynamicParkingGuide;
	
	PP_CNF_TAG_MD_VIEW_PARAM_S mdViewHeader;
	
	PP_CNF_TAG_SWING_VIEW_PARAM_S swingView;
	
}PP_CNF_TOTAL_BIN_FORMAT_S;









#endif // __API_CALIBRATION_H__

