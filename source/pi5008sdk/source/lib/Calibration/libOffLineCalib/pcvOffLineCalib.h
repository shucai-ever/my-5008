/**
 * \file	pcvOffLineCalib.h
 * 
 * \brief	Off Line Calibration API Header File
 * 
 * \author 	cheonga
 * 
 * \version 0.28.00
 *
 * Copyright (c) 2009 Pixelplus Co.,Ltd. All rights reserved
 */

#ifndef __PCV_OFF_LINE_CALIB_H__
#define __PCV_OFF_LINE_CALIB_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "pcvTypeDef.h"

enum{
	PCV_OFF_LINE_CALIB_SUCCESS,
	PCV_OFF_LINE_CALIB_ERROR_INVALID_ARGUMENT,
	PCV_OFF_LINE_CALIB_ERROR_PATTERN_NOT_FOUND
};

enum{
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE1,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE2,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE3,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE4,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE5,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE6,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE7,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE8,
	PCV_OFF_LINE_CALIB_CALIB_PATTERN_NUM
};

enum{
	PCV_OFF_LINE_CALIB_CAMERA_TYPE_FRONT,
	PCV_OFF_LINE_CALIB_CAMERA_TYPE_LEFT,
	PCV_OFF_LINE_CALIB_CAMERA_TYPE_REAR,
	PCV_OFF_LINE_CALIB_CAMERA_TYPE_RIGHT,
};

#define PCV_OFF_LINE_CALIB_DISTORTION_TABLE_MAX	110
#define PCV_OFF_LINE_CALIB_FEATURE_POINT_MAX 	16
#define PCV_OFF_LINE_CALIB_ROI_COUNT_MAX		4

/* Version Information */
typedef struct PCV_OFF_LINE_CALIB_VER_INFO_T {
	int major;
	int middle;
	int minor;
} PCV_OFF_LINE_CALIB_VER_INFO;

/* Camera Parameters */
typedef struct PCV_OFF_LINE_CALIB_CAMERA_PARAM_T {
	int width;									/* Camera Image Width (pixel) */
	int height;									/* Camera Image Height (pixel) */
	float angX;									/* Camera X-Axis (tilt) Angle (deg) */
	float angY;									/* Camera Y-Axis (roll) Angle (deg) */
	float angZ;									/* Camera Z-Axis (pan) Angle (deg) */
	float posX;									/* Camera X-Axis Position (mm) */
	float posY;									/* Camera Y-Axis Position (mm) */
	float posZ;									/* Camera Z-Axis Position (mm) */
	float focalLenX;							/* Camera X-Axis Focal Length (pixel) */
	float focalLenY;							/* Camera X-Axis Focal Length (pixel) */
	float optCenterX;							/* Camera X-Axis Optical Center (pixel) */
	float optCenterY;							/* Camera Y-Axis Optical Center (pixel) */
	float R[9];									/* Rotation Matrix */
	float T[3];									/* Translation Matrix */
	float *pDistortTable;						/* Pointer of Camera Distortion Table */
	int distortTableSize;						/* Size of Camera Distortion Table */
	int hFlip;									/* Flag of Horizontal Flip (true / false) */
} PCV_OFF_LINE_CALIB_CAMERA_PARAM;

/* Pattern Parameters */
typedef struct PCV_OFF_LINE_CALIB_PATTERN_PARAM_T {
	int	type;									/* Pattern Type */
	float fW;									/* Pattern Space Width (mm) */
	float fL;									/* Pattern Space Length (mm) */
	float fSW;									/* Pattern Space Side Width (mm) */
	float fSL;									/* Pattern Space Side Length (mm) */
	float squareSize;							/* Sqaure Size (mm) */
//	float borderSizeX;							/* X Border Size (mm) */
//	float borderSizeY;							/* Y Border Size (mm) */
//	int squareCountX;							/* Square Pattern X Count */
//	int squareCountY;							/* Square Pattern Y Count */
	int featureCount;												/* Number of Feature Point */
	int roiCount;													/* Number of ROI */
	PcvRect32d roi[PCV_OFF_LINE_CALIB_ROI_COUNT_MAX];				/* ROI of each pattern */
	float featureWpt[PCV_OFF_LINE_CALIB_FEATURE_POINT_MAX*3];		/* World Coordinate of Feature Point */
	float featureIpt[PCV_OFF_LINE_CALIB_FEATURE_POINT_MAX*2];		/* Image Coordinate of Feature Point */
} PCV_OFF_LINE_CALIB_PATTERN_PARAM;

/*
	pcvOffLineCalib_GetVersion 
		- Description
		  Get Off Line Calibration library version
		- Argument
		  ptVerInfo [OUT] : pointer of struct PCV_SVM_VIEW_VER_INFO
		- Return
		  PCV_OFF_LINE_CALIB_SUCCESS
		  PCV_OFF_LINE_CALIB_ERROR_INVALID_ARGUMENT

*/
extern int pcvOffLineCalib_GetVersion(PCV_OFF_LINE_CALIB_VER_INFO *ptVerInfo);

/*
	pcvOffLineCalib_GetFeatureWorldPoint
		- Description
		  Get World Coordinates for Feature Points
		- Argument
		  nCamType [IN] : camera type
		  ptPatternParam [IN/OUT] : Pointer of struct PCV_OFF_LINE_CALIB_PATTERN_PARAM
		- Return
		  NONE
*/
extern void pcvOffLineCalib_GetFeatureWorldPoint(int nCamType, PCV_OFF_LINE_CALIB_PATTERN_PARAM *ptPatternParam);

/*
	pcvOffLineCalib_FindCalibrationPattern
		- Description
		  Find the Calibration Pattern & the Image Coordinate of Feature Points
		- Argument
		  pY [IN] : Pointer of camera image data
		  nWidth [IN] : Image width
		  nHeight [IN] : Image height
		  ptPatternParam [IN/OUT] : Pointer of struct PCV_OFF_LINE_CALIB_PATTERN_PARAM
		  pProcBuf [X] : Pointer of process buffer (size = camera width * camera height * 9)
		  nType : Pattern type - (PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE1/
		  	  	  	  	  	  	  .../
		  	  	  	  	  	  	  PCV_OFF_LINE_CALIB_CALIB_PATTERN_TYPE7)
		- Return
		  PCV_OFF_LINE_CALIB_SUCCESS
		  PCV_OFF_LINE_CALIB_ERROR_INVALID_ARGUMENT
		  PCV_OFF_LINE_CALIB_ERROR_PATTERN_NOT_FOUND
*/
extern int pcvOffLineCalib_FindCalibrationPattern(unsigned char *pY, int nWidth, int nHeight, PCV_OFF_LINE_CALIB_PATTERN_PARAM *ptPatternParam, unsigned char *pProcBuf, int nType);

/*
	pcvOffLineCalib_GetCameraPosition
		- Description
		  Get Camera Position & Angle from Feature Points
		- Argument
		  ptCamParam [IN] : Pointer of struct PCV_OFF_LINE_CALIB_CAMERA_PARAM
		  ptPatternParam [IN/OUT] : Pointer of struct PCV_OFF_LINE_CALIB_PATTERN_PARAM
		- Return
		  PCV_OFF_LINE_CALIB_SUCCESS
		  PCV_OFF_LINE_CALIB_ERROR_INVALID_ARGUMENT
*/
extern int pcvOffLineCalib_GetCameraPosition(PCV_OFF_LINE_CALIB_CAMERA_PARAM *ptCamParam, PCV_OFF_LINE_CALIB_PATTERN_PARAM *ptPatternParam);

#ifdef __cplusplus
}
#endif

#endif	/*__PCV_OFF_LINE_CALIB_H__*/
