
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "pi5008.h"
#include "osal.h"
#include "vpu_register.h"
#include "utils.h"
#include "vpu.h"
#include "vpu_user_config.h"
#include "debug.h"

//TEST Dynamic Blending
#include "ObjectDetector.h"

const PP_S32 VPU_INPUT_SEL_Default = eVPU_CHANNEL_QUAD; //eVPU_CHANNEL_VIN, eVPU_CHANNEL_QUAD

const VPU_BRIEF_LUT_VALUE_T VPU_BRIEF_LUT_Default[128] = {
	{1, 1, 3, 3}, {2, 1, 3, 3}, {3, 1, 3, 3}, {2, 2, 3, 3}, {3, 2, 3, 3}, {1, 3, 3, 3}, {2, 3, 3, 3}, {0, 0, 3, 3},
	{1, 1, 2, 2}, {3, 0, 2, 3}, {3, 1, 2, 3}, {2, 2, 2, 2}, {0, 3, 1, 2}, {1, 3, 2, 3}, {2, 3, 2, 3}, {3, 3, 2, 3},
	{1, 1, 1, 3}, {2, 1, 1, 3}, {1, 2, 1, 3}, {0, 0, 1, 3}, {3, 2, 1, 3}, {0, 3, 1, 3}, {2, 3, 1, 2}, {3, 3, 1, 1},
	{1, 1, 3, 2}, {2, 1, 3, 2}, {3, 1, 2, 1}, {2, 2, 3, 1}, {3, 2, 3, 1}, {1, 3, 3, 2}, {2, 3, 3, 2}, {0, 1, 3, 2},
	{1, 1, 1, 2}, {2, 1, 2, 2}, {3, 1, 2, 2}, {2, 2, 2, 1}, {3, 3, 2, 1}, {1, 3, 2, 2}, {0, 0, 2, 2}, {0, 1, 2, 2},
	{1, 1, 3, 1}, {2, 1, 3, 1}, {3, 1, 3, 1}, {0, 3, 0, 0}, {3, 3, 3, 0}, {1, 3, 3, 1}, {2, 3, 3, 1}, {0, 0, 3, 1},
	{1, 1, 2, 1}, {2, 1, 2, 1}, {3, 1, 3, 0}, {2, 2, 0, 2}, {3, 2, 0, 2}, {1, 3, 2, 1}, {2, 3, 2, 1}, {3, 3, 2, 1},
	{1, 0, 1, 1}, {2, 1, 1, 1}, {3, 1, 1, 1}, {3, 0, 1, 1}, {3, 2, 1, 1}, {0, 3, 3, 0}, {2, 3, 1, 1}, {3, 3, 0, 0},
	{3, 3, 0, 3}, {2, 2, 0, 0}, {1, 3, 2, 2}, {0, 3, 2, 0}, {3, 2, 0, 0}, {1, 1, 1, 0}, {1, 2, 1, 0}, {3, 0, 0, 3},
	{3, 1, 2, 0}, {2, 1, 0, 3}, {0, 0, 0, 2}, {2, 0, 0, 3}, {1, 0, 0, 2}, {0, 1, 1, 2}, {0, 2, 1, 3}, {2, 3, 1, 3},
	{0, 0, 0, 1}, {2, 1, 1, 2}, {3, 3, 0, 1}, {3, 0, 0, 0}, {0, 1, 1, 0}, {1, 2, 2, 0}, {2, 2, 0, 3}, {3, 1, 0, 2},
	{0, 3, 3, 0}, {0, 2, 2, 0}, {3, 2, 2, 0}, {2, 0, 0, 2}, {1, 0, 1, 3}, {1, 1, 0, 3}, {2, 3, 2, 0}, {1, 3, 2, 0},
	{2, 3, 1, 0}, {1, 2, 2, 0}, {3, 2, 3, 0}, {1, 0, 3, 0}, {3, 0, 3, 0}, {2, 0, 0, 0}, {3, 0, 0, 2}, {0, 3, 3, 0},
	{0, 0, 3, 3}, {3, 3, 3, 2}, {2, 1, 0, 1}, {1, 1, 3, 0}, {1, 2, 2, 2}, {2, 2, 2, 0}, {3, 2, 0, 3}, {2, 3, 1, 2},
	{3, 2, 0, 2}, {1, 0, 0, 2}, {1, 2, 2, 0}, {0, 3, 0, 3}, {0, 1, 2, 0}, {2, 3, 0, 1}, {3, 3, 0, 0}, {3, 0, 0, 3},
	{0, 0, 0, 3}, {1, 2, 1, 0}, {2, 1, 3, 0}, {2, 2, 0, 0}, {1, 1, 1, 1}, {2, 0, 0, 1}, {1, 0, 0, 1}, {0, 1, 0, 3},
};

#if 0
const VPU_BRIEF_LUT_VALUE_T VPU_BRIEF_LUT_Default[128] = {
	{-10,-4,4,-2},  {-9,-4,8,-2},  {-8,-4,12,-2},  {-9,-3,8,2},   {-8,-3,12,2},   {-10,-2,4,6},   {-9,-2,8,6},   {-11,-5,12,6},
	{-6,-4,3,-3},   {-4,-5,7,-2},  {-4,-4,11,-2},  {-5,-3,7,1},   {-7,-2,10,1},    {-6,-2,3,6},   {-5,-2,7,6},    {-4,-2,11,6},
	{-2,-4,2,-2},   {-1,-4,6,-2},  {-2,-3,10,-2},  {-3,-5,6,2},    {0,-3,10,2},    {-3,-2,2,6},   {-1,-2,6,5},     {0,-2,10,4},
	{-10,0,4,-3},    {-9,0,8,-3},   {-8,0,11,-4},   {-9,1,8,0},    {-8,1,12,0},    {-10,2,4,5},    {-9,2,8,5},    {-11,0,12,5}, 
	{-6,0,2,-3},     {-5,0,7,-3},   {-4,0,11,-3},   {-5,1,7,0},    {-4,2,11,0},     {-6,2,3,5},   {-7,-1,7,5},     {-7,0,11,5}, 
	{-10,4,4,-4},    {-9,4,8,-4},   {-8,4,12,-4}, {-11,6,5,-1},   {-8,6,12,-1},    {-10,6,4,4},    {-9,6,8,4},    {-11,3,12,4}, 
	{-6,4,3,-4},     {-5,4,7,-4},   {-4,4,12,-5},   {-5,5,5,1},     {-4,5,9,1},     {-6,6,3,4},    {-5,6,7,4},     {-4,6,11,4}, 
	{-2,3,2,-4},     {-1,4,6,-4},    {0,4,10,-4},    {0,3,6,0},     {0,5,10,0},     {-3,6,4,3},    {-1,6,6,4},       {0,6,9,3}, 
	{0,2,-11,-2},   {-1,1,-7,-5},   {-2,2,-1,-3},  {-3,2,3,-5},     {0,1,5,-5},   {-2,0,10,-5}, {-2,1,-10,-1},     {0,-1,-7,2}, 
	{0,0,7,-1},       {-1,0,9,2},  {-3,-1,-11,5}, {-1,-1,-7,6},   {-2,-1,-3,5},     {-3,0,2,5},    {-3,1,6,6},     {-1,2,10,6}, 
	{1,-1,-11,-4},   {3,0,-6,-3},    {4,2,-3,-4},  {4,-1,1,-5},     {1,0,6,-5},    {2,1,11,-5},   {3,1,-11,2},      {4,0,-7,1}, 
	{1,2,8,-1},      {1,1,11,-1},     {4,1,-9,3},  {3,-1,-7,5},    {2,-1,-2,6},      {2,0,1,6},     {3,2,7,3},      {2,2,11,3}, 
	{-5,2,-10,-5},  {-6,1,-5,-5},    {-4,1,0,-5}, {-6,-1,4,-5},   {-4,-1,8,-5},   {-5,-1,9,-5}, {-4,-1,-11,1},     {-7,2,0,-1}, 
	{-7,-1,4,2},     {-4,2,12,1},   {-5,0,-11,4},  {-6,0,-4,3},    {-6,1,-1,5},     {-5,1,3,3},    {-4,1,5,6},     {-5,2,10,5}, 
	{8,1,-11,-3},   {6,-1,-7,-3},    {6,1,-1,-5},   {5,2,1,-2},     {5,0,7,-5},     {7,2,9,-4},  {8,2,-11,-1},     {8,-1,-3,2}, 
	{5,-1,1,2},      {6,1,10,-1},     {7,0,-8,3},   {7,1,-7,3},     {6,0,-2,4},     {7,-1,1,4},    {6,-1,5,4},       {5,0,9,6}
};
#endif

const PP_U8 VPU_BRIEF_COEF0FILTER[16] = { 3, 4, 5, 4, 3, 4, 5, 8, 5, 4, 3, 4, 5, 4, 3, 6}; //[15] = 6 ; shift
const PP_U8 VPU_BRIEF_COEF1FILTER[16] = { 0, 0, 0, 0, 0, 0, 0, 64, 0, 0, 0, 0, 0, 0, 0, 6}; //[15] = 6 ; shift

//fast_n
const PP_U8 FAST_N_NUM = 9; //9:default

#if ( ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P) || \
	((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P) || \
	((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P) )

//Single ---> compare VPU TEST SW data
#if 0
//Single : 1 rect division. 1280x720
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		//70, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		100, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		//((16*8)/8), ((16*8)/8), (((720-16)*8)/8), (((240-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 11*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
};
#endif

#if 1
//Quad
#if 1 //TEST Dynamic Blending
//VPU_ZONINFO_T StockZoneInfoHexa1[16] = 
//Quad : 4 anormaly rect division. 1280x720
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE01_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE01_START_X, OD_ZONE01_START_Y, OD_ZONE01_END_X, OD_ZONE01_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE01_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE01_THRESH), (MATCHING_XMAXDIST_ZONE01_THRESH), (MATCHING_YMINDIST_ZONE01_THRESH), (MATCHING_YMAXDIST_ZONE01_THRESH) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE02_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE02_START_X, OD_ZONE02_START_Y, OD_ZONE02_END_X, OD_ZONE02_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE02_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE02_THRESH), (MATCHING_XMAXDIST_ZONE02_THRESH), (MATCHING_YMINDIST_ZONE02_THRESH), (MATCHING_YMAXDIST_ZONE02_THRESH) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE03_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE03_START_X, OD_ZONE03_START_Y, OD_ZONE03_END_X, OD_ZONE03_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE03_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE03_THRESH), (MATCHING_XMAXDIST_ZONE03_THRESH), (MATCHING_YMINDIST_ZONE03_THRESH), (MATCHING_YMAXDIST_ZONE03_THRESH) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE04_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE04_START_X, OD_ZONE04_START_Y, OD_ZONE04_END_X, OD_ZONE04_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE04_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE04_THRESH), (MATCHING_XMAXDIST_ZONE04_THRESH), (MATCHING_YMINDIST_ZONE04_THRESH), (MATCHING_YMAXDIST_ZONE04_THRESH) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE05_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE05_START_X, OD_ZONE05_START_Y, OD_ZONE05_END_X, OD_ZONE05_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE05_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE05_THRESH), (MATCHING_XMAXDIST_ZONE05_THRESH), (MATCHING_YMINDIST_ZONE05_THRESH), (MATCHING_YMAXDIST_ZONE05_THRESH) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE06_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE06_START_X, OD_ZONE06_START_Y, OD_ZONE06_END_X, OD_ZONE06_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE06_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE06_THRESH), (MATCHING_XMAXDIST_ZONE06_THRESH), (MATCHING_YMINDIST_ZONE06_THRESH), (MATCHING_YMAXDIST_ZONE06_THRESH) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE07_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE07_START_X, OD_ZONE07_START_Y, OD_ZONE07_END_X, OD_ZONE07_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE07_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE07_THRESH), (MATCHING_XMAXDIST_ZONE07_THRESH), (MATCHING_YMINDIST_ZONE07_THRESH), (MATCHING_YMAXDIST_ZONE07_THRESH) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		FAST_ZONE08_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE08_START_X, OD_ZONE08_START_Y, OD_ZONE08_END_X, OD_ZONE08_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE08_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE08_THRESH), (MATCHING_XMAXDIST_ZONE08_THRESH), (MATCHING_YMINDIST_ZONE08_THRESH), (MATCHING_YMAXDIST_ZONE08_THRESH) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE09_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE09_START_X, OD_ZONE09_START_Y, OD_ZONE09_END_X, OD_ZONE09_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE09_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE09_THRESH), (MATCHING_XMAXDIST_ZONE09_THRESH), (MATCHING_YMINDIST_ZONE09_THRESH), (MATCHING_YMAXDIST_ZONE09_THRESH) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE10_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE10_START_X, OD_ZONE10_START_Y, OD_ZONE10_END_X, OD_ZONE10_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE10_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE10_THRESH), (MATCHING_XMAXDIST_ZONE10_THRESH), (MATCHING_YMINDIST_ZONE10_THRESH), (MATCHING_YMAXDIST_ZONE10_THRESH) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE11_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE11_START_X, OD_ZONE11_START_Y, OD_ZONE11_END_X, OD_ZONE11_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE11_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE11_THRESH), (MATCHING_XMAXDIST_ZONE11_THRESH), (MATCHING_YMINDIST_ZONE11_THRESH), (MATCHING_YMAXDIST_ZONE11_THRESH) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE12_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE12_START_X, OD_ZONE12_START_Y, OD_ZONE12_END_X, OD_ZONE12_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE12_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE12_THRESH), (MATCHING_XMAXDIST_ZONE12_THRESH), (MATCHING_YMINDIST_ZONE12_THRESH), (MATCHING_YMAXDIST_ZONE12_THRESH) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE13_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE13_START_X, OD_ZONE13_START_Y, OD_ZONE13_END_X, OD_ZONE13_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE13_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE13_THRESH), (MATCHING_XMAXDIST_ZONE13_THRESH), (MATCHING_YMINDIST_ZONE13_THRESH), (MATCHING_YMAXDIST_ZONE13_THRESH) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE14_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE14_START_X, OD_ZONE14_START_Y, OD_ZONE14_END_X, OD_ZONE14_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE14_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE14_THRESH), (MATCHING_XMAXDIST_ZONE14_THRESH), (MATCHING_YMINDIST_ZONE14_THRESH), (MATCHING_YMAXDIST_ZONE14_THRESH) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE15_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE15_START_X, OD_ZONE15_START_Y, OD_ZONE15_END_X, OD_ZONE15_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE15_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE15_THRESH), (MATCHING_XMAXDIST_ZONE15_THRESH), (MATCHING_YMINDIST_ZONE15_THRESH), (MATCHING_YMAXDIST_ZONE15_THRESH) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		FALSE, //isEnable, 
		FAST_ZONE16_THRESH, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		OD_ZONE16_START_X, OD_ZONE16_START_Y, OD_ZONE16_END_X, OD_ZONE16_END_Y, //sx, sy, ex, ey,
		0, //fc_height;
		MATCHING_ZONE16_THRESH, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (MATCHING_XMINDIST_ZONE16_THRESH), (MATCHING_XMAXDIST_ZONE16_THRESH), (MATCHING_YMINDIST_ZONE16_THRESH), (MATCHING_YMAXDIST_ZONE16_THRESH) } 
	},/*}}}*/
};
#if 0
//VPU_ZONINFO_T StockZoneInfoHexa1[16] = 
//Quad : 4 anormaly rect division. 1280x720
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		25, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		96, 80, 424, 160, //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-8), (32), (0), (96) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		30, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		224, 160, 320, 336, //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-8), (32), (0), (152) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		30, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		320, 160, 424, 336, //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-32), (8), (0), (152) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		25, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		424, 120, 544, 336, //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(96+640), (96+360), (264+640), (224+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(264+640), (80+360), (384+640), (224+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(384+640), (96+360), (544+640), (224+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+640), (224+360), (480+640), (296+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-96), (0), (-24), (24) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		TRUE, //isEnable, 
		35, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(96+640), (96    ), (224+640), (336    ), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-8), (32), (0), (96) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(224+640), (136    ), (320+640), (336    ), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-8), (32), (0), (152) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		TRUE, //isEnable, 
		35, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320+640), (136    ), (424+640), (336    ), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-32), (8), (0), (152) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		TRUE, //isEnable, 
		25, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(424+640), (96    ), (544+640), (336    ), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(96    ), (96+360), (264    ), (224+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		TRUE, //isEnable, 
		45, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(264    ), (80+360), (384    ), (224+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		TRUE, //isEnable, 
		40, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(384    ), (96+360), (544    ), (224+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		TRUE, //isEnable, 
		30, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(128    ), (224+360), (416    ), (296+360), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (0), (96), (-24), (24) } 
	},/*}}}*/
};
#endif
#else
//VPU_ZONINFO_T StockZoneInfoHexa2[16] =  
//Quad : 4 rect division. 1280x720
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (16    ), (320    ), (176    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320    ), (16    ), (624    ), (176    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (176    ), (320    ), (352    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320    ), (176    ), (624    ), (352    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+640), (16+0), (320+640), (176+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320+640), (16+0), (624+640), (176+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+640), (176+0), (320+640), (352+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320+640), (176+0), (624+640), (352+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (16+352), (320), (176+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320), (16+352), (624), (176+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (176+352), (320), (352+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320), (176+352), (624), (352+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+640), (16+352), (320+640), (176+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320+640), (16+352), (624+640), (176+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+640), (176+352), (320+640), (352+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(320+640), (176+352), (624+640), (352+352), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-50), (50), (-50), (50) } 
	},/*}}}*/
};
#endif
#endif

#elif     ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)

#if ((BD_VPU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
//Single ---> compare VPU TEST SW data
#if 1
//Single : 1 rect division. 720x480
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		//70, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		100, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((720-16)*8)/8), (((240-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 11*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
};
#endif

#if 0
//Quad
//Quad : 4 rect division. 720x480
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (16    ), (176    ), (56    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176    ), (16    ), (344    ), (56    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (56    ), (176    ), (120    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176    ), (56    ), (344    ), (120    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+0), (176+360), (56+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (16+0), (344+360), (56+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (56+0), (176+360), (120+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (56+0), (344+360), (120+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (16+120), (176), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176), (16+120), (344), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (56+120), (176), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176), (56+120), (344), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+120), (176+360), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (16+120), (344+360), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (56+120), (176+360), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (56+120), (344+360), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
};
#endif

#elif ((BD_VPU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)

//Single ---> compare VPU TEST SW data
#if 1
//Single : 1 rect division. 720x576
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		//70, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		100, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((720-16)*8)/8), (((288-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 11*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((720-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
};
#endif

#if 0
//Quad
//Quad : 4 rect division. 720x576
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (16    ), (176    ), (64    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176    ), (16    ), (344    ), (64    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (64    ), (176    ), (144    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176    ), (64    ), (344    ), (144    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+0), (176+360), (64+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (16+0), (344+360), (64+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (64+0), (176+360), (144+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (64+0), (344+360), (144+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (16+144), (176), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176), (16+144), (344), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (64+144), (176), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176), (64+144), (344), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+144), (176+360), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (16+144), (344+360), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (64+144), (176+360), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(176+360), (64+144), (344+360), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
};
#endif

#endif

#elif     ((BD_VPU_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)

#if ((BD_VPU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_NTSC_30)
//Single
#if 1
//Single : 1 rect division. 960x480
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		//70, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		100, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((960-16)*8)/8), (((240-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 11*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
};
#endif

#if 0
//Quad
//Quad : 4 rect division. 960x480
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (16    ), (240    ), (56    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240    ), (16    ), (344    ), (56    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (56    ), (240    ), (120    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240    ), (56    ), (344    ), (120    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+0), (240+360), (56+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (16+0), (344+360), (56+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (56+0), (240+360), (120+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (56+0), (344+360), (120+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (16+120), (240), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240), (16+120), (344), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (56+120), (240), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240), (56+120), (344), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+120), (240+360), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (16+120), (344+360), (56+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (56+120), (240+360), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (56+120), (344+360), (120+120), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
};
#endif

#elif ((BD_VPU_IN_FMT & VID_FRAME_MASKBIT) == VID_FRAME_PAL_25)

//Single
#if 1
//Single : 1 rect division. 960x576
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		//70, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		100, 2, 12, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((960-16)*8)/8), (((288-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 11*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		FALSE, //isEnable, 
		70, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		((16*8)/8), ((16*8)/8), (((1280-16)*8)/8), (((960-16)*8)/8), //sx, sy, ex, ey,
		0, //fc_height;
		10, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-72), (72), (-72), (72) } 
	},/*}}}*/
};
#endif

#if 0
//Quad
//Quad : 4 rect division. 960x576
const VPU_ZONINFO_T VPU_ZONE_PARAM[16] = {
/* 1 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (16    ), (240    ), (64    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 2 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240    ), (16    ), (344    ), (64    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 3 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16    ), (64    ), (240    ), (144    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 4 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240    ), (64    ), (344    ), (144    ), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 5 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+0), (240+360), (64+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 6 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (16+0), (344+360), (64+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 7 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (64+0), (240+360), (144+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 8 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (64+0), (344+360), (144+0), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 9 */	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (16+144), (240), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 10 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240), (16+144), (344), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 11 */{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16), (64+144), (240), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 12*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240), (64+144), (344), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 13*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 12, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (16+144), (240+360), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 14*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (16+144), (344+360), (64+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 15*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(16+360), (64+144), (240+360), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
/* 16*/	{/*{{{*/
		TRUE, //isEnable, 
		100, 2, 0, 13, //fastThreshold, fastFilterIndex, briefLUTIndex, briefFilterIndex,
		(240+360), (64+144), (344+360), (144+144), //sx, sy, ex, ey,
		0, //fc_height;
		15, //hd_threshold;
		//VPU_DISTANCE_RECT_T match_dist;
		//{ x_min, x_max, y_min, y_max }
		{ (-16), (16), (-16), (16) } 
	},/*}}}*/
};
#endif

#endif


#endif


#if 0
VPU_ZONINFO_T StockZoneInfoSingle2[16] = {{FALSE, SINGLE1_FAST_ZONE01_TREHSH, 2, 0, 0, }/*{{{*/
										, {TRUE, SINGLE1_FAST_ZONE01_TREHSH, 2, 0, 0, SINGLE1_FAST_ZONE01_START_X, SINGLE1_FAST_ZONE01_START_Y, SINGLE1_FAST_ZONE01_END_X, SINGLE1_FAST_ZONE01_END_Y, 0, SINGLE1_MATCHING_ZONE01_THRESH, {SINGLE1_MATCHING_XMINDIST_ZONE01_THRESH, SINGLE1_MATCHING_XMAXDIST_ZONE01_THRESH, SINGLE1_MATCHING_YMINDIST_ZONE01_THRESH, SINGLE1_MATCHING_YMAXDIST_ZONE01_THRESH}}
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										};/*}}}*/

VPU_ZONINFO_T StockZoneInfoQuad1[16] = {{TRUE, QUAD1_FAST_ZONE01_TREHSH, 2, 0, 2, QUAD1_FAST_ZONE01_START_X, QUAD1_FAST_ZONE01_START_Y, QUAD1_FAST_ZONE01_END_X, QUAD1_FAST_ZONE01_END_Y, 0, QUAD1_MATCHING_ZONE01_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE01_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE01_THRESH, QUAD1_MATCHING_YMINDIST_ZONE01_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE01_THRESH}}/*{{{*/
										, {TRUE, QUAD1_FAST_ZONE02_TREHSH, 2, 0, 2, QUAD1_FAST_ZONE02_START_X, QUAD1_FAST_ZONE02_START_Y, QUAD1_FAST_ZONE02_END_X, QUAD1_FAST_ZONE02_END_Y, 0, QUAD1_MATCHING_ZONE02_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE02_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE02_THRESH, QUAD1_MATCHING_YMINDIST_ZONE02_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE02_THRESH}}
										, {TRUE, QUAD1_FAST_ZONE03_TREHSH, 2, 0, 2, QUAD1_FAST_ZONE03_START_X, QUAD1_FAST_ZONE03_START_Y, QUAD1_FAST_ZONE03_END_X, QUAD1_FAST_ZONE03_END_Y, 0, QUAD1_MATCHING_ZONE03_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE03_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE03_THRESH, QUAD1_MATCHING_YMINDIST_ZONE03_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE03_THRESH}}
										, {TRUE, QUAD1_FAST_ZONE04_TREHSH, 2, 0, 2, QUAD1_FAST_ZONE04_START_X, QUAD1_FAST_ZONE04_START_Y, QUAD1_FAST_ZONE04_END_X, QUAD1_FAST_ZONE04_END_Y, 0, QUAD1_MATCHING_ZONE04_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE04_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE04_THRESH, QUAD1_MATCHING_YMINDIST_ZONE04_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE04_THRESH}}
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										};/*}}}*/
VPU_ZONINFO_T StockZoneInfoQuad2[16] = {{FALSE, QUAD1_FAST_ZONE01_TREHSH, 2, 0, 0, }/*{{{*/
										, {FALSE, }
										, {TRUE, QUAD1_FAST_ZONE01_TREHSH, 2, 0, 0, QUAD1_FAST_ZONE01_START_X, QUAD1_FAST_ZONE01_START_Y, QUAD1_FAST_ZONE01_END_X, QUAD1_FAST_ZONE01_END_Y, 0, QUAD1_MATCHING_ZONE01_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE01_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE01_THRESH, QUAD1_MATCHING_YMINDIST_ZONE01_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE01_THRESH}}
										, {TRUE, QUAD1_FAST_ZONE02_TREHSH, 2, 0, 0, QUAD1_FAST_ZONE02_START_X, QUAD1_FAST_ZONE02_START_Y, QUAD1_FAST_ZONE02_END_X, QUAD1_FAST_ZONE02_END_Y, 0, QUAD1_MATCHING_ZONE02_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE02_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE02_THRESH, QUAD1_MATCHING_YMINDIST_ZONE02_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE02_THRESH}}
										, {TRUE, QUAD1_FAST_ZONE03_TREHSH, 2, 0, 0, QUAD1_FAST_ZONE03_START_X, QUAD1_FAST_ZONE03_START_Y, QUAD1_FAST_ZONE03_END_X, QUAD1_FAST_ZONE03_END_Y, 0, QUAD1_MATCHING_ZONE03_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE03_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE03_THRESH, QUAD1_MATCHING_YMINDIST_ZONE03_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE03_THRESH}}
										, {TRUE, QUAD1_FAST_ZONE04_TREHSH, 2, 0, 0, QUAD1_FAST_ZONE04_START_X, QUAD1_FAST_ZONE04_START_Y, QUAD1_FAST_ZONE04_END_X, QUAD1_FAST_ZONE04_END_Y, 0, QUAD1_MATCHING_ZONE04_THRESH, {QUAD1_MATCHING_XMINDIST_ZONE04_THRESH, QUAD1_MATCHING_XMAXDIST_ZONE04_THRESH, QUAD1_MATCHING_YMINDIST_ZONE04_THRESH, QUAD1_MATCHING_YMAXDIST_ZONE04_THRESH}}
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										};/*}}}*/

VPU_ZONINFO_T StockZoneInfoOcta1[16] = {{TRUE, OCTA1_FAST_ZONE01_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE01_START_X, OCTA1_FAST_ZONE01_START_Y, OCTA1_FAST_ZONE01_END_X, OCTA1_FAST_ZONE01_END_Y, 0, OCTA1_MATCHING_ZONE01_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE01_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE01_THRESH, OCTA1_MATCHING_YMINDIST_ZONE01_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE01_THRESH}}/*{{{*/
										, {TRUE, OCTA1_FAST_ZONE02_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE02_START_X, OCTA1_FAST_ZONE02_START_Y, OCTA1_FAST_ZONE02_END_X, OCTA1_FAST_ZONE02_END_Y, 0, OCTA1_MATCHING_ZONE02_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE02_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE02_THRESH, OCTA1_MATCHING_YMINDIST_ZONE02_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE02_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE03_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE03_START_X, OCTA1_FAST_ZONE03_START_Y, OCTA1_FAST_ZONE03_END_X, OCTA1_FAST_ZONE03_END_Y, 0, OCTA1_MATCHING_ZONE03_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE03_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE03_THRESH, OCTA1_MATCHING_YMINDIST_ZONE03_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE03_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE04_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE04_START_X, OCTA1_FAST_ZONE04_START_Y, OCTA1_FAST_ZONE04_END_X, OCTA1_FAST_ZONE04_END_Y, 0, OCTA1_MATCHING_ZONE04_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE04_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE04_THRESH, OCTA1_MATCHING_YMINDIST_ZONE04_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE04_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE05_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE05_START_X, OCTA1_FAST_ZONE05_START_Y, OCTA1_FAST_ZONE05_END_X, OCTA1_FAST_ZONE05_END_Y, 0, OCTA1_MATCHING_ZONE05_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE05_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE05_THRESH, OCTA1_MATCHING_YMINDIST_ZONE05_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE05_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE06_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE06_START_X, OCTA1_FAST_ZONE06_START_Y, OCTA1_FAST_ZONE06_END_X, OCTA1_FAST_ZONE06_END_Y, 0, OCTA1_MATCHING_ZONE06_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE06_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE06_THRESH, OCTA1_MATCHING_YMINDIST_ZONE06_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE06_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE07_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE07_START_X, OCTA1_FAST_ZONE07_START_Y, OCTA1_FAST_ZONE07_END_X, OCTA1_FAST_ZONE07_END_Y, 0, OCTA1_MATCHING_ZONE07_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE07_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE07_THRESH, OCTA1_MATCHING_YMINDIST_ZONE07_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE07_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE08_TREHSH, 2, 0, 2, OCTA1_FAST_ZONE08_START_X, OCTA1_FAST_ZONE08_START_Y, OCTA1_FAST_ZONE08_END_X, OCTA1_FAST_ZONE08_END_Y, 0, OCTA1_MATCHING_ZONE08_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE08_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE08_THRESH, OCTA1_MATCHING_YMINDIST_ZONE08_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE08_THRESH}}
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										};/*}}}*/
VPU_ZONINFO_T StockZoneInfoOcta2[16] = {{FALSE, OCTA1_FAST_ZONE01_TREHSH, 2, 0, 0, }/*{{{*/
										, {FALSE, }
										, {TRUE, OCTA1_FAST_ZONE01_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE01_START_X, OCTA1_FAST_ZONE01_START_Y, OCTA1_FAST_ZONE01_END_X, OCTA1_FAST_ZONE01_END_Y, 0, OCTA1_MATCHING_ZONE01_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE01_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE01_THRESH, OCTA1_MATCHING_YMINDIST_ZONE01_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE01_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE02_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE02_START_X, OCTA1_FAST_ZONE02_START_Y, OCTA1_FAST_ZONE02_END_X, OCTA1_FAST_ZONE02_END_Y, 0, OCTA1_MATCHING_ZONE02_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE02_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE02_THRESH, OCTA1_MATCHING_YMINDIST_ZONE02_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE02_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE03_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE03_START_X, OCTA1_FAST_ZONE03_START_Y, OCTA1_FAST_ZONE03_END_X, OCTA1_FAST_ZONE03_END_Y, 0, OCTA1_MATCHING_ZONE03_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE03_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE03_THRESH, OCTA1_MATCHING_YMINDIST_ZONE03_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE03_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE04_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE04_START_X, OCTA1_FAST_ZONE04_START_Y, OCTA1_FAST_ZONE04_END_X, OCTA1_FAST_ZONE04_END_Y, 0, OCTA1_MATCHING_ZONE04_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE04_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE04_THRESH, OCTA1_MATCHING_YMINDIST_ZONE04_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE04_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE05_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE05_START_X, OCTA1_FAST_ZONE05_START_Y, OCTA1_FAST_ZONE05_END_X, OCTA1_FAST_ZONE05_END_Y, 0, OCTA1_MATCHING_ZONE05_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE05_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE05_THRESH, OCTA1_MATCHING_YMINDIST_ZONE05_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE05_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE06_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE06_START_X, OCTA1_FAST_ZONE06_START_Y, OCTA1_FAST_ZONE06_END_X, OCTA1_FAST_ZONE06_END_Y, 0, OCTA1_MATCHING_ZONE06_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE06_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE06_THRESH, OCTA1_MATCHING_YMINDIST_ZONE06_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE06_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE07_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE07_START_X, OCTA1_FAST_ZONE07_START_Y, OCTA1_FAST_ZONE07_END_X, OCTA1_FAST_ZONE07_END_Y, 0, OCTA1_MATCHING_ZONE07_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE07_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE07_THRESH, OCTA1_MATCHING_YMINDIST_ZONE07_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE07_THRESH}}
										, {TRUE, OCTA1_FAST_ZONE08_TREHSH, 2, 0, 0, OCTA1_FAST_ZONE08_START_X, OCTA1_FAST_ZONE08_START_Y, OCTA1_FAST_ZONE08_END_X, OCTA1_FAST_ZONE08_END_Y, 0, OCTA1_MATCHING_ZONE08_THRESH, {OCTA1_MATCHING_XMINDIST_ZONE08_THRESH, OCTA1_MATCHING_XMAXDIST_ZONE08_THRESH, OCTA1_MATCHING_YMINDIST_ZONE08_THRESH, OCTA1_MATCHING_YMAXDIST_ZONE08_THRESH}}
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										, {FALSE, }
										};/*}}}*/

VPU_ZONINFO_T StockZoneInfoHexa1[16] = {{TRUE, HEXA1_FAST_ZONE01_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE01_START_X, HEXA1_FAST_ZONE01_START_Y, HEXA1_FAST_ZONE01_END_X, HEXA1_FAST_ZONE01_END_Y, 0, HEXA1_MATCHING_ZONE01_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE01_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE01_THRESH, HEXA1_MATCHING_YMINDIST_ZONE01_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE01_THRESH}}/*{{{*/
										, {TRUE, HEXA1_FAST_ZONE02_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE02_START_X, HEXA1_FAST_ZONE02_START_Y, HEXA1_FAST_ZONE02_END_X, HEXA1_FAST_ZONE02_END_Y, 0, HEXA1_MATCHING_ZONE02_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE02_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE02_THRESH, HEXA1_MATCHING_YMINDIST_ZONE02_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE02_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE03_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE03_START_X, HEXA1_FAST_ZONE03_START_Y, HEXA1_FAST_ZONE03_END_X, HEXA1_FAST_ZONE03_END_Y, 0, HEXA1_MATCHING_ZONE03_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE03_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE03_THRESH, HEXA1_MATCHING_YMINDIST_ZONE03_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE03_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE04_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE04_START_X, HEXA1_FAST_ZONE04_START_Y, HEXA1_FAST_ZONE04_END_X, HEXA1_FAST_ZONE04_END_Y, 0, HEXA1_MATCHING_ZONE04_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE04_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE04_THRESH, HEXA1_MATCHING_YMINDIST_ZONE04_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE04_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE05_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE05_START_X, HEXA1_FAST_ZONE05_START_Y, HEXA1_FAST_ZONE05_END_X, HEXA1_FAST_ZONE05_END_Y, 0, HEXA1_MATCHING_ZONE05_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE05_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE05_THRESH, HEXA1_MATCHING_YMINDIST_ZONE05_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE05_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE06_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE06_START_X, HEXA1_FAST_ZONE06_START_Y, HEXA1_FAST_ZONE06_END_X, HEXA1_FAST_ZONE06_END_Y, 0, HEXA1_MATCHING_ZONE06_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE06_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE06_THRESH, HEXA1_MATCHING_YMINDIST_ZONE06_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE06_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE07_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE07_START_X, HEXA1_FAST_ZONE07_START_Y, HEXA1_FAST_ZONE07_END_X, HEXA1_FAST_ZONE07_END_Y, 0, HEXA1_MATCHING_ZONE07_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE07_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE07_THRESH, HEXA1_MATCHING_YMINDIST_ZONE07_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE07_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE08_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE08_START_X, HEXA1_FAST_ZONE08_START_Y, HEXA1_FAST_ZONE08_END_X, HEXA1_FAST_ZONE08_END_Y, 0, HEXA1_MATCHING_ZONE08_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE08_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE08_THRESH, HEXA1_MATCHING_YMINDIST_ZONE08_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE08_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE09_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE09_START_X, HEXA1_FAST_ZONE09_START_Y, HEXA1_FAST_ZONE09_END_X, HEXA1_FAST_ZONE09_END_Y, 0, HEXA1_MATCHING_ZONE09_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE09_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE09_THRESH, HEXA1_MATCHING_YMINDIST_ZONE09_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE09_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE10_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE10_START_X, HEXA1_FAST_ZONE10_START_Y, HEXA1_FAST_ZONE10_END_X, HEXA1_FAST_ZONE10_END_Y, 0, HEXA1_MATCHING_ZONE10_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE10_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE10_THRESH, HEXA1_MATCHING_YMINDIST_ZONE10_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE10_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE11_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE11_START_X, HEXA1_FAST_ZONE11_START_Y, HEXA1_FAST_ZONE11_END_X, HEXA1_FAST_ZONE11_END_Y, 0, HEXA1_MATCHING_ZONE11_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE11_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE11_THRESH, HEXA1_MATCHING_YMINDIST_ZONE11_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE11_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE12_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE12_START_X, HEXA1_FAST_ZONE12_START_Y, HEXA1_FAST_ZONE12_END_X, HEXA1_FAST_ZONE12_END_Y, 0, HEXA1_MATCHING_ZONE12_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE12_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE12_THRESH, HEXA1_MATCHING_YMINDIST_ZONE12_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE12_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE13_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE13_START_X, HEXA1_FAST_ZONE13_START_Y, HEXA1_FAST_ZONE13_END_X, HEXA1_FAST_ZONE13_END_Y, 0, HEXA1_MATCHING_ZONE13_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE13_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE13_THRESH, HEXA1_MATCHING_YMINDIST_ZONE13_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE13_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE14_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE14_START_X, HEXA1_FAST_ZONE14_START_Y, HEXA1_FAST_ZONE14_END_X, HEXA1_FAST_ZONE14_END_Y, 0, HEXA1_MATCHING_ZONE14_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE14_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE14_THRESH, HEXA1_MATCHING_YMINDIST_ZONE14_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE14_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE15_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE15_START_X, HEXA1_FAST_ZONE15_START_Y, HEXA1_FAST_ZONE15_END_X, HEXA1_FAST_ZONE15_END_Y, 0, HEXA1_MATCHING_ZONE15_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE15_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE15_THRESH, HEXA1_MATCHING_YMINDIST_ZONE15_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE15_THRESH}}
										, {TRUE, HEXA1_FAST_ZONE16_TREHSH, 2, 0, 2, HEXA1_FAST_ZONE16_START_X, HEXA1_FAST_ZONE16_START_Y, HEXA1_FAST_ZONE16_END_X, HEXA1_FAST_ZONE16_END_Y, 0, HEXA1_MATCHING_ZONE16_THRESH, {HEXA1_MATCHING_XMINDIST_ZONE16_THRESH, HEXA1_MATCHING_XMAXDIST_ZONE16_THRESH, HEXA1_MATCHING_YMINDIST_ZONE16_THRESH, HEXA1_MATCHING_YMAXDIST_ZONE16_THRESH}}
										};/*}}}*/
VPU_ZONINFO_T StockZoneInfoHexa2[16] = {{FALSE, HEXA2_FAST_ZONE01_TREHSH, 2, 0, 0, }/*{{{*/
										, {TRUE, HEXA2_FAST_ZONE01_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE01_START_X, HEXA2_FAST_ZONE01_START_Y, HEXA2_FAST_ZONE01_END_X, HEXA2_FAST_ZONE01_END_Y, 0, HEXA2_MATCHING_ZONE01_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE01_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE01_THRESH, HEXA2_MATCHING_YMINDIST_ZONE01_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE01_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE02_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE02_START_X, HEXA2_FAST_ZONE02_START_Y, HEXA2_FAST_ZONE02_END_X, HEXA2_FAST_ZONE02_END_Y, 0, HEXA2_MATCHING_ZONE02_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE02_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE02_THRESH, HEXA2_MATCHING_YMINDIST_ZONE02_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE02_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE03_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE03_START_X, HEXA2_FAST_ZONE03_START_Y, HEXA2_FAST_ZONE03_END_X, HEXA2_FAST_ZONE03_END_Y, 0, HEXA2_MATCHING_ZONE03_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE03_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE03_THRESH, HEXA2_MATCHING_YMINDIST_ZONE03_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE03_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE04_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE04_START_X, HEXA2_FAST_ZONE04_START_Y, HEXA2_FAST_ZONE04_END_X, HEXA2_FAST_ZONE04_END_Y, 0, HEXA2_MATCHING_ZONE04_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE04_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE04_THRESH, HEXA2_MATCHING_YMINDIST_ZONE04_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE04_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE05_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE05_START_X, HEXA2_FAST_ZONE05_START_Y, HEXA2_FAST_ZONE05_END_X, HEXA2_FAST_ZONE05_END_Y, 0, HEXA2_MATCHING_ZONE05_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE05_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE05_THRESH, HEXA2_MATCHING_YMINDIST_ZONE05_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE05_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE06_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE06_START_X, HEXA2_FAST_ZONE06_START_Y, HEXA2_FAST_ZONE06_END_X, HEXA2_FAST_ZONE06_END_Y, 0, HEXA2_MATCHING_ZONE06_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE06_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE06_THRESH, HEXA2_MATCHING_YMINDIST_ZONE06_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE06_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE07_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE07_START_X, HEXA2_FAST_ZONE07_START_Y, HEXA2_FAST_ZONE07_END_X, HEXA2_FAST_ZONE07_END_Y, 0, HEXA2_MATCHING_ZONE07_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE07_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE07_THRESH, HEXA2_MATCHING_YMINDIST_ZONE07_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE07_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE08_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE08_START_X, HEXA2_FAST_ZONE08_START_Y, HEXA2_FAST_ZONE08_END_X, HEXA2_FAST_ZONE08_END_Y, 0, HEXA2_MATCHING_ZONE08_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE08_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE08_THRESH, HEXA2_MATCHING_YMINDIST_ZONE08_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE08_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE09_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE09_START_X, HEXA2_FAST_ZONE09_START_Y, HEXA2_FAST_ZONE09_END_X, HEXA2_FAST_ZONE09_END_Y, 0, HEXA2_MATCHING_ZONE09_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE09_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE09_THRESH, HEXA2_MATCHING_YMINDIST_ZONE09_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE09_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE10_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE10_START_X, HEXA2_FAST_ZONE10_START_Y, HEXA2_FAST_ZONE10_END_X, HEXA2_FAST_ZONE10_END_Y, 0, HEXA2_MATCHING_ZONE10_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE10_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE10_THRESH, HEXA2_MATCHING_YMINDIST_ZONE10_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE10_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE11_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE11_START_X, HEXA2_FAST_ZONE11_START_Y, HEXA2_FAST_ZONE11_END_X, HEXA2_FAST_ZONE11_END_Y, 0, HEXA2_MATCHING_ZONE11_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE11_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE11_THRESH, HEXA2_MATCHING_YMINDIST_ZONE11_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE11_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE12_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE12_START_X, HEXA2_FAST_ZONE12_START_Y, HEXA2_FAST_ZONE12_END_X, HEXA2_FAST_ZONE12_END_Y, 0, HEXA2_MATCHING_ZONE12_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE12_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE12_THRESH, HEXA2_MATCHING_YMINDIST_ZONE12_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE12_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE13_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE13_START_X, HEXA2_FAST_ZONE13_START_Y, HEXA2_FAST_ZONE13_END_X, HEXA2_FAST_ZONE13_END_Y, 0, HEXA2_MATCHING_ZONE13_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE13_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE13_THRESH, HEXA2_MATCHING_YMINDIST_ZONE13_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE13_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE14_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE14_START_X, HEXA2_FAST_ZONE14_START_Y, HEXA2_FAST_ZONE14_END_X, HEXA2_FAST_ZONE14_END_Y, 0, HEXA2_MATCHING_ZONE14_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE14_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE14_THRESH, HEXA2_MATCHING_YMINDIST_ZONE14_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE14_THRESH}}
										, {TRUE, HEXA2_FAST_ZONE15_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE15_START_X, HEXA2_FAST_ZONE15_START_Y, HEXA2_FAST_ZONE15_END_X, HEXA2_FAST_ZONE15_END_Y, 0, HEXA2_MATCHING_ZONE15_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE15_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE15_THRESH, HEXA2_MATCHING_YMINDIST_ZONE15_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE15_THRESH}}
										//, {TRUE, HEXA2_FAST_ZONE16_TREHSH, 2, 0, 0, HEXA2_FAST_ZONE16_START_X, HEXA2_FAST_ZONE16_START_Y, HEXA2_FAST_ZONE16_END_X, HEXA2_FAST_ZONE16_END_Y, 0, HEXA2_MATCHING_ZONE16_THRESH, {HEXA2_MATCHING_XMINDIST_ZONE16_THRESH, HEXA2_MATCHING_XMAXDIST_ZONE16_THRESH, HEXA2_MATCHING_YMINDIST_ZONE16_THRESH, HEXA2_MATCHING_YMAXDIST_ZONE16_THRESH}}
										};/*}}}*/
#endif

