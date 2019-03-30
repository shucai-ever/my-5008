/**
 * \file	pcvSvmView.h
 * 
 * \brief	Svm View Generation API Header File
 * 
 * \author 	cheonga
 * 
 * \version 0.50.00
 * 
 * Copyright (c) 2009 Pixelplus Co.,Ltd. All rights reserved
 */
 
#ifndef __PCV_SVM_VIEW_H__
#define __PCV_SVM_VIEW_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pcvTypeDef.h"

enum{
	PCV_SVM_VIEW_SUCCESS,
	PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT,
	PCV_SVM_VIEW_ERROR_INVALID_CAM_SIZE,
	PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
};

enum {
	PCV_SVM_VIEW_SUB_VIEW_TYPE_2D,
	PCV_SVM_VIEW_SUB_VIEW_TYPE_3D,
	PCV_SVM_VIEW_SUB_VIEW_TYPE_SD,
	PCV_SVM_VIEW_SUB_VIEW_TYPE_UNDISTORT,
	PCV_SVM_VIEW_SUB_VIEW_TYPE_DEG360,
	PCV_SVM_VIEW_SUB_VIEW_TYPE_PERSPECTIVE,
	PCV_SVM_VIEW_SUB_VIEW_TYPE_CYLINDRICAL
};

enum {
	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE,
	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED,
	PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE,
	PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED,
};

enum {
	PCV_SVM_VIEW_FIELD_TYPE_UPPER,
	PCV_SVM_VIEW_FIELD_TYPE_LOWER,
	PCV_SVM_VIEW_FIELD_TYPE_FRAME,
};

enum {
	PCV_SVM_VIEW_CAMERA_TYPE_FRONT,
	PCV_SVM_VIEW_CAMERA_TYPE_LEFT,
	PCV_SVM_VIEW_CAMERA_TYPE_REAR,
	PCV_SVM_VIEW_CAMERA_TYPE_RIGHT,
	PCV_SVM_VIEW_CAMERA_TYPE_ALL
};

enum{
	PCV_SVM_VIEW_PG_LINE_DIR_FORWARD,
	PCV_SVM_VIEW_PG_LINE_DIR_BACKWARD
};

enum{
	PCV_SVM_VIEW_SAMPLE_MODE_NONE,
	PCV_SVM_VIEW_SAMPLE_MODE_8
};

#define PCV_SVM_VIEW_DISTORTION_TABLE_MAX	100
#define PCV_SVM_VIEW_DISTORTION_TABLE_MIN	80

#define PCV_SVM_VIEW_SUB_VIEW_NUM_MAX		4

#define PCV_SVM_VIEW_LUT_INVALID_VAL		0xFFFFFFFF

#define PCV_SVM_VIEW_LUT_OFFSET_X			60
#define PCV_SVM_VIEW_LUT_OFFSET_Y			270

#define PCV_SVM_VIEW_STEER_ANGLE_MIN		-35
#define PCV_SVM_VIEW_STEER_ANGLE_MAX		35

#define PCV_SVM_VIEW_INVALID_PT_VAL			4096

/* Version Information */
typedef struct PCV_SVM_VIEW_VER_INFO_T {
	int major;
	int middle;
	int minor;
} PCV_SVM_VIEW_VER_INFO;

/* Blend Parameters */
typedef struct PCV_SVM_VIEW_BLEND_PARAM_T {
	int shadowAreaFront;				/* Front Position of Shadow Area (mm) */
	int shadowAreaLeft;					/* Left Position of Shadow Area (mm) */
	int shadowAreaRear;					/* Rear Position of Shadow Area (mm) */
	int shadowAreaRight;				/* Right Position of Shadow Area (mm) */ 
	int alpha0DivAngFront;				/* Front Division Angle of Alpha0 Blend Map (deg) */
	int alpha0DivAngRear;				/* Rear Division Angle of Alpha0 Blend Map (deg) */ 
	int alpha0BlendAreaFront;			/* Front Area of Alpha0 Blend Map (mm) */
	int alpha0BlendAreaRear;			/* Rear Area of Alpha0 Blend Map (mm) */ 
	int alpha1DivAngFront;				/* Rear Division Angle of Alpha1 Blend Map (deg) */ 
	int alpha1DivAngRear;				/* Rear Division Angle of Alpha1 Blend Map (deg) */
	int alpha1BlendAreaFront;			/* Front Area of Alpha0 Blend Map (mm) */ 
	int alpha1BlendAreaRear;			/* Rear Area of Alpha0 Blend Map (mm) */
	unsigned char *pPattern;
	int patternWidth;
	int patternHeight;
} PCV_SVM_VIEW_BLEND_PARAM;

/* Camera Parameters */
typedef struct PCV_SVM_VIEW_CAMERA_PARAM_T {
	int width;							/* Camera Image Width (pixel) */
	int height;							/* Camera Image Height (pixel) */
	float angX;							/* Camera X-Axis (tilt) Angle (deg) */
	float angY;							/* Camera Y-Axis (roll) Angle (deg) */
	float angZ;							/* Camera Z-Axis (pan) Angle (deg) */
	float posX;							/* Camera X-Axis Position (mm) */
	float posY;							/* Camera Y-Axis Position (mm) */
	float posZ;							/* Camera Z-Axis Position (mm) */
	float focalLenX;					/* Camera X-Axis Focal Length (pixel) */
	float focalLenY;					/* Camera X-Axis Focal Length (pixel) */
	float optCenterX;					/* Camera X-Axis Optical Center (pixel) */
	float optCenterY;					/* Camera Y-Axis Optical Center (pixel) */
	float *pDistortTable;				/* Pointer of Camera Distortion Table */
	int distortTableSize;				/* Size of Camera Distortion Table */
	float projMat[12];					/* Projection Matrix */
	int hFlip;							/* Flag of Horizontal Flip (true / false) */
} PCV_SVM_VIEW_CAMERA_PARAM;

/* Surface Grid Parameters */
typedef struct PCV_SVM_VIEW_SURFACE_GRID_PARAM_T {
	PcvPoint3d32f wpt;					/* World Coordinate of Surface Grid Point */
	PcvPoint32f ipt;					/* Image Coordinate of Surface Grid Point */
	PcvRect32d iptRect;					/* Image Points of Surface Grid Rectangle */
	int valid;							/* Validity of Surface Grid Point */
	int face;							/* Face of Surface Grid Point */
} PCV_SVM_VIEW_SURFACE_GRID_PARAM;

/* Camera Grid Parameters */
typedef struct PCV_SVM_VIEW_CAMERA_GRID_PARAM_T {
	PcvPoint32f ipt;					/* Image Coordinate of Camera Grid Point */
	int valid;							/* Validity of Camera Grid Point */
	int area;							/* Area information of Camera */
} PCV_SVM_VIEW_CAMERA_GRID_PARAM;

/* View Grid Parameters */
typedef struct PCV_SVM_VIEW_VIEW_GRID_PARAM_T {
	int width;
	int height;
	int xStep;
	int yStep;
} PCV_SVM_VIEW_VIEW_GRID_PARAM;

/* 2D Surface Parameters */
typedef struct PCV_SVM_VIEW_SURFACE_2D_PARAM_T {	
	int wx;												/* 2D Surface X Length (mm) */
	int wy;												/* 2D Surface Y Length (mm) */
	PCV_SVM_VIEW_SURFACE_GRID_PARAM *ptSurfaceGrid;		/* Pointer of 2D Surface Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptFrontCamGrid;		/* Pointer of Front Camera Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptLeftCamGrid;		/* Pointer of Left Camera Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptRearCamGrid;		/* Pointer of Rear Camera Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptRightCamGrid;		/* Pointer of Right Camera Grid */
	PcvPoint32d center;									/* Center Coordinate of 2D Surface (mm) */
} PCV_SVM_VIEW_SURFACE_2D_PARAM;

/* 3D Surface Parameters */
typedef struct PCV_SVM_VIEW_SURFACE_3D_PARAM_T {
	float wx;											/* 3D Surface X Length (mm) */
	float wy;											/* 3D Surface Y Length (mm) */
	float wz;											/* 3D Surface Z Length (mm) */
	float rx;											/* 3D Surface X Curvature */
	float ry;											/* 3D Surface Y Curvature */
	float rz;											/* 3D Surface Z Curvature */
	int zOffset;										/* 3D Surface Z Offset */
	PCV_SVM_VIEW_SURFACE_GRID_PARAM *ptSurfaceGrid;		/* Pointer of 3D Surface Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptFrontCamGrid;		/* Pointer of Front Camera Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptLeftCamGrid;		/* Pointer of Left Camera Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptRearCamGrid;		/* Pointer of Rear Camera Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptRightCamGrid;		/* Pointer of Right Camera Grid */
	PcvPoint32d center;									/* Center Coordinate of 3D Surface (mm)*/
} PCV_SVM_VIEW_SURFACE_3D_PARAM;

typedef struct PCV_SVM_VIEW_SURFACE_CAM_PARAM_T {
	float wx;											/* CAM Surface X Length wx */
	float wy;											/* CAM Surface Y Length wy */
	float dist;											/* CAM Surface Distance */
	PCV_SVM_VIEW_SURFACE_GRID_PARAM *ptSurfaceGrid;		/* Pointer of 3D Surface Grid */
	PCV_SVM_VIEW_CAMERA_GRID_PARAM *ptCamGrid;			/* Pointer of Camera Grid */
} PCV_SVM_VIEW_SURFACE_CAM_PARAM;

/* MD View Parameters */
typedef struct PCV_SVM_VIEW_MD_VIEW_PARAM_T {
	int camType;										/* Camera Type (PCV_SVM_VIEW_CAMERA_TYPE_FRONT / 
																		PCV_SVM_VIEW_CAMERA_TYPE_LEFT /	
																		PCV_SVM_VIEW_CAMERA_TYPE_REAR /
																		PCV_SVM_VIEW_CAMERA_TYPE_RIGHT) */
	int width;											/* View Width */
	int height;											/* View Height */
	float scaleX;										/* View X Scale */
	float scaleY;										/* View Y Scale */
	unsigned int *pSurfaceGridIndex;								/* Pointer of Surface Grid Index */
	PCV_SVM_VIEW_CAMERA_PARAM tVirtualCamParam;			/* Virtual Camera Parameter */
	int hFlip;											/* Horizontal Mirror */
} PCV_SVM_VIEW_MD_VIEW_PARAM;

/* Cam View Parameters */
typedef struct PCV_SVM_VIEW_CAM_VIEW_PARAM_T {
	int viewType;										/* View Type */
	int camType;										/* Camera Type (PCV_SVM_VIEW_CAMERA_TYPE_FRONT /
																		PCV_SVM_VIEW_CAMERA_TYPE_LEFT /
																		PCV_SVM_VIEW_CAMERA_TYPE_REAR /
																		PCV_SVM_VIEW_CAMERA_TYPE_RIGHT) */
	int width;											/* View Width */
	int height;											/* View Height */
	float scaleX;										/* View X Scale */
	float scaleY;										/* View Y Scale */
	unsigned int *pSurfaceGridIndex;								/* Pointer of Surface Grid Index */
	PCV_SVM_VIEW_SURFACE_CAM_PARAM tSurfaceParam;		/* Surface Parameter */
	PCV_SVM_VIEW_CAMERA_PARAM tVirtualCamParam;			/* Virtual Camera Parameter */
	int hFlip;											/* Horizontal Mirror */
	int bLdc;											/* Select Lens Distortion Correction */
	int bPgl;											/* PGL Select */
	int pglDir;											/* PGL Direction */
} PCV_SVM_VIEW_CAM_VIEW_PARAM;

/* Static PG Parameters */
typedef struct PCV_SVM_VIEW_STATIC_PG_PARAM_T
{
	int	dir;											/* PG Direction Type (PCV_SVM_VIEW_PG_DIR_TYPE_FORWARD,
																		PCV_SVM_VIEW_PG_DIR_TYPE_BACKWARD) */
	int	numPoints;										/* Number of Points per Line*/
	int	dist[3];										/* Distance (mm) */
	int	distMin;										/* Min. Distance (mm) */
	int	width;											/* Width Between Left and Right PG Line (mm) */
	PcvPoint32f *wpts[3];								/* Pointer of World Points for PG Line (Near/Middle/Far) */
	PcvPoint32f *ipts[3];								/* Pointer of Image Points for PG Line (Near/Middle/Far) */
} PCV_SVM_VIEW_STATIC_PG_PARAM;

/* Dynamic PG Parameters */
typedef struct PCV_SVM_VIEW_DYNAMIC_PG_PARAM_T
{
	int	dir;											/* PG Direction Type (PCV_SVM_VIEW_PG_LINE_DIR_FORWARD,
																		PCV_SVM_VIEW_PG_LINE_DIR_BACKWARD) */
	int	numPoints;										/* Number of Points per Line*/
	int	wheelBase;										/* Wheel Base of Car (mm) */
	int	wheelBaseCy;									/* Center Y of Wheel Base (mm) */
	int	steerAngle;										/* Steering Angle (deg.) */
	int	distLRMin;										/* Min. Distance for Left/Right Line (mm) */
	int	distSideMin;									/* Min. Distance for Side Guide Line (mm) */
	int	distLRMax;										/* Max. Distance for Left/Right Guide Line (mm) */
	int	distSideMax;									/* Max. Distance for Side Guide Line (mm) */
	int	width;											/* Width Between Left and Right PG Line (mm) */
	PcvPoint32f *pLeftWpts;								/* Pointer of World Points for Left PG Line */
	PcvPoint32f *pRightWpts;							/* Pointer of World Points for Right PG Line */
	PcvPoint32f *pSideWpts;								/* Pointer of World Points for Side PG Line */
	PcvPoint32f *pLeftIpts;								/* Pointer of Image Points for Left PG Line */
	PcvPoint32f *pRightIpts;							/* Pointer of Image Points for Right PG Line */
	PcvPoint32f *pSideIpts;								/* Pointer of Image Points for Side PG Line */
} PCV_SVM_VIEW_DYNAMIC_PG_PARAM;

/*
	pcvSvmView_GetVersion 
		- Description
		  Get SvmView library version
		- Argument
		  ptVerInfo [OUT] : pointer of struct PCV_SVM_VIEW_VER_INFO
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetVersion(PCV_SVM_VIEW_VER_INFO *ptVerInfo);

/*
	pcvSvmView_Set2dSurface
		- Description
		  Make 2D Surface
		- Argument
		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_Set2dSurface(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam);

/*
	pcvSvmView_Set3dSurface
		- Description
		  Make 3D Surface
		- Argument
		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_Set3dSurface(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam);

/*
	pcvSvmView_SetCylindricalSurface
		- Desctiption
		  Make Cylindrical Surface
		- Argument
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
*/
extern int pcvSvmView_SetCylindricalSurface(PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam);

/*
	pcvSvmView_SetCameraParameter
		- Description
		  Set Camera Parameters
		- Argument
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_SetCameraParameter(PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam);

/*
	pcvSvmView_GetCameraProjectPoint
		- Description
		  Get Projected Image Points from World Points for Camera
		- Argument
		  pWPoint [IN] : Pointer of world points (x,y,z)
		  pIPoint [OUT] : Pointer of view image points (x,y)
		  pValid [OUT] : Validity of points
		  nCount [IN] : number of points
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetCameraProjectPoint(PcvPoint3d32f *pWPoint, PcvPoint32f *pIPoint, int *pValid, int nCount, PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam);

/*
	pcvSvmView_GetMdViewProjectPoint
		- Description
		  Get Projected Image Points from World Points for SD/2D/3D view
		- Argument
		  pWPoint [IN] : Pointer of world points (x,y,z)
		  pIPoint [OUT] : Pointer of view image points (x,y)
		  pValid [OUT] : Validity of points
		  nCount [IN] : number of points
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetMdViewProjectPoint(PcvPoint3d32f *pWPoint, PcvPoint32f *pIPoint, int *pValid, int nCount, PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam);

/*
	pcvSvmView_Set2dSurfaceProjectCamera
		- Description
		  Project camera to 2D Surface
		- Argument
		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  nCamType [IN] : Camera type (PCV_SVM_VIEW_CAMERA_TYPE_FRONT /
										PCV_SVM_VIEW_CAMERA_TYPE_LEFT /
										PCV_SVM_VIEW_CAMERA_TYPE_REAR /
										PCV_SVM_VIEW_CAMERA_TYPE_RIGHT)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_Set2dSurfaceProjectCamera(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam,
		PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
		int nCamType);

/*
	pcvSvmView_Set2dSurfaceProjectView
		- Description
		  Project view to 2D Surface
		- Argument
		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : Sample Mode (PCV_SVM_VIEW_SAMPLE_MODE_NONE /
										PCV_SVM_VIEW_SAMPLE_MODE_8)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_Set2dSurfaceProjectView(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam,
		PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
		int nScanType,
		int nField,
		int nSampleMode);

/*
	pcvSvmView_Set3dSurfaceProjection
		- Description
		  Project camera to 3D Surface
		- Argument
		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  nCamType [IN] : Camera type (PCV_SVM_VIEW_CAMERA_TYPE_FRONT /
										PCV_SVM_VIEW_CAMERA_TYPE_LEFT /
										PCV_SVM_VIEW_CAMERA_TYPE_REAR /
										PCV_SVM_VIEW_CAMERA_TYPE_RIGHT)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_Set3dSurfaceProjectCamera(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
		PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
		int nCamType);

/*
	pcvSvmView_Set3dSurfaceProjectView
		- Description
		  Project view to 3D Surface
		- Argument
		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : Sample Mode (PCV_SVM_VIEW_SAMPLE_MODE_NONE /
										PCV_SVM_VIEW_SAMPLE_MODE_8)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_Set3dSurfaceProjectView(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
		PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
		int nScanType,
		int nField,
		int nSampleMode);

/*
	pcvSvmView_SetUndistortSurfaceProjectView
		- Description
		  Project view to undistort Surface
		- Argument
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : Sample Mode (PCV_SVM_VIEW_SAMPLE_MODE_NONE /
										PCV_SVM_VIEW_SAMPLE_MODE_8)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
*/
extern int pcvSvmView_SetUndistortSurfaceProjectView(PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
		PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam,
		int nScanType,
		int nField,
		int nSampleMode);


/*
	pcvSvmView_SetCylindricalSurfaceProjectCamera
		- Desctiption
		  Project camera to cylindrical Surface
		- Argument
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
*/
extern int pcvSvmView_SetCylindricalSurfaceProjectCamera(PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
		PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam);

/*
	pcvSvmView_SetCylindricalSurfaceProjectView
		- Description
		  Project view to cylindrical Surface
		- Argument
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : Sample Mode (PCV_SVM_VIEW_SAMPLE_MODE_NONE /
										PCV_SVM_VIEW_SAMPLE_MODE_8)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
*/
extern int pcvSvmView_SetCylindricalSurfaceProjectView(PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam,
		int nScanType,
		int nField,
		int nSampleMode);

/*
	pcvSvmView_Get2dSurfaceGridBufSize
		- Description
		  Get buffer size for 2D surface grid
		- Argument
		  None
		- return
		  Buffer size for 2D surface grid
*/
extern int pcvSvmView_Get2dSurfaceGridBufSize();

/*
	pcvSvmView_Get3dSurfaceGridBufSize
		- Description
		  Get buffer size for 3D surface grid
		- Argument
		  None
		- return
		  Buffer size for 3D surface grid
*/
extern int pcvSvmView_Get3dSurfaceGridBufSize();

/*
	pcvSvmView_GetCamViewGridBufSize
		- Description
		  Get CAM view grid buffer size
		- Argument
		  None
		- return
		  Grid buffer size of CAM view
*/
extern int pcvSvmView_GetCamViewGridBufSize();

/*
	pcvSvmView_GetViewGridParam
		- Description
		  Get view grid parameters
		- Argument
		  ptViewGridParam [OUT] : Pointer of struct PCV_SVM_VIEW_VIEW_GRID_PARAM
		  nViewWidth [IN] : view width
		  nHeight [IN] : view height
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nSampleMode [IN] : Sample Mode (PCV_SVM_VIEW_SAMPLE_MODE_NONE /
										PCV_SVM_VIEW_SAMPLE_MODE_8)
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetViewGridParam(PCV_SVM_VIEW_VIEW_GRID_PARAM *ptViewGridParam, int nViewWidth, int nViewHeight, int nScanType, int nSampleMode);

/*
	pcvSvmView_GetBlendPatternXSize
	pcvSvmView_GetBlendPatternYSize
		- Description
		  Get blend pattern size
		- Argument
		  None
		- return
		  Blend pattern size
*/
extern int pcvSvmView_GetBlendPatternXSize();
extern int pcvSvmView_GetBlendPatternYSize();

/*
	pcvSvmView_GetBlendPattern
		- Description
		  Get blend pattern parameters
		- Argument
		  ptBlendParam [OUT] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetBlendPattern(PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam);

/*
	pcvSvmView_GetSdBlendLut
		- Description
		  Get blend LUT for SD view
		- Argument
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  pBlendLut [OUT] : Pointer of blend LUT
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_NONE /
		  	  	  	  	  PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetSdBlendLut(PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam, unsigned char *pBlendLut, int nScanType, int nField, int nSampleMode);

/*
	pcvSvmView_Get2dAlphaBlendLut
		- Description
		  Get Alpha blend LUT for top 2D preview
		- Argument
		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
		  pBlendLut [OUT] : Pointer of blend LUT
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_NONE /
		  	  	  	  	  PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get2dAlphaBlendLut(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam,
		PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
		PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
		unsigned char *pProcBuf,
		unsigned char *pBlendLut,
		int nScanType,
		int nField,
		int nSampleMode);

/*
	pcvSvmView_Get2dBlendLut
		- Description
		  Get blend LUT for 2D view
		- Argument
		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
		  pBlendLut [OUT] : Pointer of blend LUT
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_NONE /
		  	  	  	  	  PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get2dBlendLut(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam,
		PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
		PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
		unsigned char *pProcBuf,
		unsigned char *pBlendLut,
		int nScanType,
		int nField,
		int nSampleMode);

/*
	pcvSvmView_Get3dBlendLut
		- Description
		  Get blend LUT for 3D view
		- Argument
		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
		  pBlendLut [OUT] : Pointer of blend LUT
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_NONE /
		  	  	  	  	  PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get3dBlendLut(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
		PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
		PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
		unsigned char *pProcBuf,
		unsigned char *pBlendLut,
		int nScanType,
		int nField,
		int nSampleMode);

/*
	pcvSvmView_GetSdFrViewLut
		- Description
		  Get SD FR view LUT
		- Argument
		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
		  pViewLut [OUT] : Pointer of front-rear view LUT data
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetSdFrViewLut(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
	PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
  	pcvSvmView_GetSdLrViewLut
  		- Description
  		  Get SD LR view LUT
  		- Argument
  		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
  		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
  		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
  		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
  		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
  		  pViewLut [OUT] : Pointer of left-right view LUT data
  		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
  		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
  		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetSdLrViewLut(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
	PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
  	pcvSvmView_Get2dFrViewLut
  		- Description
  		  Get 2D FR view LUT
  		- Argument
  		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
  		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
  		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
  		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
  		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
  		  pViewLut [OUT] : Pointer of front-rear or left-right view LUT data
  		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
  		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
  		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_CAM_SIZE
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get2dFrViewLut(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptFrontCamParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptRearCamParam,
	PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
  	pcvSvmView_Get2dLrViewLut
  		- Description
  		  Get 2D LR view LUT
  		- Argument
  		  ptSurface2dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_2D_PARAM
  		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
  		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
  		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
  		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
  		  pViewLut [OUT] : Pointer of front-rear or left-right view LUT data
  		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
   		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
  		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_CAM_SIZE
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get2dLrViewLut(PCV_SVM_VIEW_SURFACE_2D_PARAM *ptSurface2dParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptLeftCamParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptRightCamParam,
	PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
  	pcvSvmView_Get3dFrViewLut
  		- Description
  		  Get 3D FR view LUT
  		- Argument
  		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
  		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
  		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
  		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
  		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
  		  pViewLut [OUT] : Pointer of front-rear or left-right view LUT data
  		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
  		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
  		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_CAM_SIZE
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get3dFrViewLut(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptFrontCamParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptRearCamParam,
	PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
  	pcvSvmView_Get3dLrViewLut
  		- Description
  		  Get 3D LR view LUT
  		- Argument
  		  ptSurface3dParam [IN] : Pointer of struct PCV_SVM_VIEW_SURFACE_3D_PARAM
  		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
  		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
  		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
  		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
  		  pViewLut [OUT] : Pointer of front-rear or left-right view LUT data
  		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
  		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
  		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_CAM_SIZE
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_Get3dLrViewLut(PCV_SVM_VIEW_SURFACE_3D_PARAM *ptSurface3dParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptLeftCamParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptRightCamParam,
	PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
	pcvSvmView_GetMdViewStaticGuideLine
		- Description
		  Get static parking guide line for MD view
		- Argument
		  ptStaticPGParam [IN] : Pointer of struct PCV_SVM_VIEW_STATIC_PG_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetMdViewStaticGuideLine(PCV_SVM_VIEW_STATIC_PG_PARAM *ptStaticPGParam, PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam, PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam);

/*
	pcvSvmView_GetMdViewDynamicGuideLine
		- Description
		  Get dynamic parking guide line for MD view
		- Argument
		  ptDynamicPGParam [IN] : Pointer of struct PCV_SVM_VIEW_DYNAMIC_PG_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_MD_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetMdViewDynamicGuideLine(PCV_SVM_VIEW_DYNAMIC_PG_PARAM *ptDynamicPGParam, PCV_SVM_VIEW_MD_VIEW_PARAM *ptViewParam, PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam);

/*
	pcvSvmView_GetCamFrViewLut
		- Description
		  Get CAM view FR LUT
		- Argument
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
		  pViewLut [OUT] : Pointer of front-rear or left-right view LUT data
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetCamFrViewLut(PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
	PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
	pcvSvmView_GetCamLrViewLut
		- Description
		  Get CAM view LR LUT
		- Argument
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		  pProcBuf [X] : Pointer of processing buffer (size = view grid width * view grid Height * 8)
		  pViewLut [OUT] : Pointer of front-rear or left-right view LUT data
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
										PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nFieldType [IN] : Field type (PCV_SVM_VIEW_FIELD_TYPE_UPPER /
										PCV_SVM_VIEW_FIELD_TYPE_LOWER /
										PCV_SVM_VIEW_FIELD_TYPE_FRAME)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_NONE /
		  	  	  	  	  PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
*/
extern int pcvSvmView_GetCamLrViewLut(PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
	PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam,
	unsigned char *pProcBuf,
	unsigned char *pViewLut,
	int nScanType,
	int nField,
	int nSampleMode);

/*
	pcvSvmView_GetCamBlendLut
		- Description
		  Get blend LUT for CAM view
		- Argument
		  pBlendLut [OUT] : Pointer of blend LUT data
		  nViewWidth [IN] : View image width
		  nViewHeight [IN] : View image height
	 	  nCamType : Camera type - 	(PCV_SVM_VIEW_CAMERA_TYPE_FRONT /
									PCV_SVM_VIEW_CAMERA_TYPE_LEFT /
									PCV_SVM_VIEW_CAMERA_TYPE_REAR /
									PCV_SVM_VIEW_CAMERA_TYPE_RIGHT /
		  nScanType [IN] : Scan type (	PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_PROGRESSIVE /
									PCV_SVM_VIEW_SCAN_TYPE_PROGRESSIVE_TO_INTERLACED /
									PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_PROGRESSIVE /
									PCV_SVM_VIEW_SCAN_TYPE_INTERLACED_TO_INTERLACED)
		  nSampleMode [IN] : PCV_SVM_VIEW_SAMPLE_MODE_NONE /
		  	  	  	  	  PCV_SVM_VIEW_SAMPLE_MODE_8
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_ERROR_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetCamBlendLut(unsigned char *pBlendLut, int nViewWidth, int nViewHeight, int nCamType, int nScanType, int nSampleMode);

/*
	pcvSvmView_GetCamViewStaticGuideLine
		- Description
		  Get static parking guide line for the CAM view
		- Argument
		  ptStaticPGParam [IN] : Pointer of struct PCV_SVM_VIEW_STATIC_PG_PARAM
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetCamViewStaticGuideLine(PCV_SVM_VIEW_STATIC_PG_PARAM *ptStaticPGParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
	PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam);

/*
	pcvSvmView_GetCamViewDynamicGuideLine
		- Description
		  Get dynamic parking guide line for the CAM view
		- Argument
		  ptDynamicPGParam [IN] : Pointer of struct PCV_SVM_VIEW_DYNAMIC_PG_PARAM
		  ptCamParam [IN] : Pointer of struct PCV_SVM_VIEW_CAMERA_PARAM
		  ptViewParam [IN] : Pointer of struct PCV_SVM_VIEW_CAM_VIEW_PARAM
		  ptBlendParam [IN] : Pointer of struct PCV_SVM_VIEW_BLEND_PARAM
		- return
		  PCV_SVM_VIEW_SUCCESS
		  PCV_SVM_VIEW_INVALID_ARGUMENT
		  PCV_SVM_VIEW_ERROR_INVALID_VIEW_SIZE
*/
extern int pcvSvmView_GetCamViewDynamicGuideLine(PCV_SVM_VIEW_DYNAMIC_PG_PARAM *ptDynamicPGParam,
	PCV_SVM_VIEW_CAMERA_PARAM *ptCamParam,
	PCV_SVM_VIEW_CAM_VIEW_PARAM *ptViewParam,
	PCV_SVM_VIEW_BLEND_PARAM *ptBlendParam);

#ifdef __cplusplus
}
#endif

#endif /* __PCV_SVM_VIEW_H__ */
