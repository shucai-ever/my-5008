/*
|---------------------------------------------------------------------------------------------------------
|               R E V I S I O N   H I S T O R Y
|---------------------------------------------------------------------------------------------------------
| Date        Version   Author                              Description
| ----------  --------  ------                              ----------------------------------------------
| 2016-11-07   0.1      S.W Lee								- Draft
| Copyright (C) 2016 LG Innotek Co.,Ltd. All rights reserved
*/

#ifndef MOTIONDETECTOR_H__
#define MOTIONDETECTOR_H__

#include "osal.h"
#include "vpu_api.h"
#include "vpu.h"

#if ((SELECT_TEST_MODE == TEST_MODE_VPU_DMA) || defined(USE_VPU_COMPARE_FB))
/*---------------------------------------------------------------------------*/
/* Defines */
/*---------------------------------------------------------------------------*/
typedef signed char sint8_t;                /* Type Definition (signed char > sint8_t) */
typedef unsigned char uint8_t;              /* Type Definition (unsigned char > uint8_t) */
typedef unsigned char bool_t;               /* Type Definition (unsigned char > bool_t) */
typedef signed short sint16_t;              /* Type Definition (signed short > sint16_t) */
typedef unsigned short uint16_t;            /* Type Definition (unsigned short > uint16_t) */
typedef signed int sint32_t;                /* Type Definition (signed long > sint32_t) */
//typedef unsigned int uint32_t;              /* Type Definition (unsigned long > uint32_t) */
typedef float float32_t;                    /* Type Definition (float > float32_t) */
typedef double double32_t;                  /* Type Definition (double > double32_t) */
typedef void(*pFunc_t) (void);             /* Type Definition (function pointer type > pFunc_t) */
typedef unsigned unsigned_t;

//#define SAVE_BRIEF_LUT
#define LOAD_BRIEF_LUT

#define IMAGE_WIDTH			1280
#define IMAGE_HEIGHT		720

#define FAST_BLOCKNMS_SIZE		10
#define POINTS_LIMIT			1000

typedef enum /* Type definition for Camera Mask */
{
	CAMERA_FRONT = 0U, /* Front camera */
	CAMERA_RIGHT = 1U,	/* Right camera */
	CAMERA_REAR = 2U,	/* Rear camera */
	CAMERA_LEFT = 3U,	/* Left camera */
	OUTPUT_VIDEO = 4U   /* Output video */
} CAMERA_MASK_e;

typedef enum                    /* Type definition for Non Maximum Suppression mode */
{
	NORMAL_NMS = 0U,            /* Normal NMS mode */
	BLOCK_BASED_NMS = 1U,       /* Block Based NMS */	
} NMSMODE_e;

typedef struct ROIRegion_t_ /* Type definition for ROI region */
{
	uint16_t startX;		/* ROI start x coordinate  */
	uint16_t startY;		/* ROI start y coordinate  */
	uint16_t regionWidth;	/* ROI width  */
	uint16_t regionHeight;	/* ROI height  */
} ROIRegion_t;

typedef struct Pos_t_ /* Type definition for 2D Position */
{
	uint16_t x;		/* 2D Position X */
	uint16_t y;		/* 2D Position Y */	
} Pos_t;

typedef struct PosFloat_t_ /* Type definition for 2D Position */
{
	float32_t x;		/* 2D Position X */
	float32_t y;		/* 2D Position Y */
} PosFloat_t;

typedef struct Pos3DFloat_t_ /* Type definition for 2D Position */
{
	float32_t x;		/* 3D Position X */
	float32_t y;		/* 3D Position Y */
	float32_t z;		/* 3D Position Z */
} Pos3DFloat_t;


#define TEST_MODE				1 // 0: OnCal 4Zone / 1: OnCal 16Zone / 2: GAR 16Zone
#define TOTAL_ZONE_NUM			16

typedef struct MotionPairPos_t_ /* Type definition for Motion Position Pair */
{
	Pos_t first;		/* first frame position */
	Pos_t second;		/* second frame position */
} MotionPairPos_t;

typedef struct MotionPairPosFloat_t_ /* Type definition for Motion Position Pair */
{
	PosFloat_t first;		/* first frame position */
	PosFloat_t second;		/* second frame position */
} MotionPairPosFloat_t;

typedef struct BRIEFDecs_t_ /* Type definition for BRIEF Descriptor */
{
	uint16_t x;		/* 2D Position X */
	uint16_t y;		/* 2D Position Y */
	uint8_t desc[BRIEF_DESCRIPTOR_SIZE / 8];	/* Descriptor */
} BRIEFDecs_t;

typedef struct FASTParam_t_
{
	uint8_t numFAST;
	uint8_t threshFAST;
	uint8_t NMSmode;
	ROIRegion_t roiZoneFast;
	uint16_t * lineIntegrateNum;
} FastParam_t;

typedef struct BRIEFParam_t_
{
	uint8_t patchSizeX;
	uint8_t patchSizeY;
	uint16_t descriptorSize;
	sint16_t * BRIEF_LUT;
} BRIEFParam_t;

typedef struct MotionMatchParam_t_
{
	uint16_t matchingTh;
	sint16_t distXMinTh;
	sint16_t distXMaxTh;
	sint16_t distYMinTh;
	sint16_t distYMaxTh;
} MotionMatchParam_t;

typedef struct MotionResult_t_
{
	uint32_t totalNumFeture1st;
	uint32_t totalNumFeture2nd;
	uint32_t totalNumMotion;
} MotionResult_t;

/*---------------------------------------------------------------------------*/
/* Interfaces */
/*---------------------------------------------------------------------------*/
void MotionDetector_InitParam(FastParam_t * fastParam, BRIEFParam_t * briefParam, MotionMatchParam_t * matchingParam);

sint16_t * MotionDetector_BRIEFpattern(uint16_t descSize, uint8_t patchSizeW, uint8_t patchSizeH);

void MotionDetector_FASTwithZone(uint8_t * img, uint16_t w, uint16_t h, FastParam_t * fastZoneParam, ROIRegion_t roiRegionFast, uint8_t totalNumZone, Pos_t ** outFeaturesPosZone, uint32_t * outTotalNumFeture);

Pos_t * MotionDetector_MatchingwithLineCnt(BRIEFDecs_t * BRIEFPos1, uint16_t TotalNumFeture1, BRIEFDecs_t * BRIEFPos2, uint16_t TotalNumFeture2,
	uint16_t descSize, MotionMatchParam_t matchingParam, FastParam_t fastParam, uint16_t * numMotionPair);

void MotionDetector_3x5Filter(uint8_t * in, uint8_t * out, uint16_t w, uint16_t h, uint8_t * filt);


// No use function
Pos_t * MotionDetector_FAST(uint8_t * img, uint16_t w, uint16_t h, uint8_t numFAST, uint8_t threshFAST, ROIRegion_t roiRegionFast, uint8_t NMSMode, uint32_t * outTotalNumFeture);
BRIEFDecs_t * MotionDetector_BRIEF(uint32_t FrameNum, uint32_t ZoneNum, uint8_t * img, uint16_t w, uint16_t h, Pos_t * FeaturesPos, uint16_t TotalNumFeture, BRIEFParam_t briefParam, uint16_t * outTotalBRIEFNum);
MotionPairPos_t * MotionDetector_Matching(BRIEFDecs_t * BRIEFPos1, uint16_t TotalNumFeture1, BRIEFDecs_t * BRIEFPos2, uint16_t TotalNumFeture2, uint16_t descSize, uint16_t matchingTh, uint16_t distMinTh, uint16_t distMaxTh, uint16_t * numMotionPair);
MotionPairPos_t * MotionDetector_DetectMotion(uint8_t * preImg, uint8_t * curImg, uint16_t w, uint16_t h, uint8_t camNum, FastParam_t fastParam, BRIEFParam_t briefParam, MotionMatchParam_t matchingParam, MotionResult_t * result);

void MotionDetectionTest (uint8_t *input1, uint8_t *input2);

#endif /* (SELECT_TEST_MODE == TEST_MODE_VPU_DMA) || defined(USE_VPU_COMPARE_FB) */

#endif /* MOTIONDETECTOR_H__ */
/*---------------------------------------------------------------------------*/
/* End Of File */
/*---------------------------------------------------------------------------*/