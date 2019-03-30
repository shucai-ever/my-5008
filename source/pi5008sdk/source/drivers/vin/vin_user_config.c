#include <stdio.h>
#include <string.h>
#include "board_config.h"
#include "vin.h"
#include "vin_user_config.h"


const int DEF_VIN_MIPI_HSIZE[max_vid_resol][max_vin_datatype] = {
	/*  _RAW6, _RAW7, _RAW8,_RAW10,_RAW12,_RAW14,_YUV8_2xRAW8,_2xRAW10 */
	/*  0:  720x480i@60 */ {     0,     0,     0,     0,     0,     0,     0,     0},
	/*  1:  720x576i@50 */ {     0,     0,     0,     0,     0,     0,     0,     0},
	/*  2:  960x480i@60 */ {     0,     0,     0,     0,     0,     0,     0,     0},
	/*  3:  960x576i@50 */ {     0,     0,     0,     0,     0,     0,     0,     0},
	/*  4:  720x480p@60 */ { 720/8, 720/8, 720/8, 720/4, 720/4, 720/4, 720/4, 720/4},
	/*  5:  720x576p@50 */ { 720/8, 720/8, 720/8, 720/4, 720/4, 720/4, 720/4, 720/4},
	/*  6:  960x480p@60 */ { 960/8, 960/8, 960/8, 960/4, 960/4, 960/4, 960/4, 960/4},
	/*  7:  960x576p@50 */ { 960/8, 960/8, 960/8, 960/4, 960/4, 960/4, 960/4, 960/4},
	/*  8: 1280x720p@60 */ {1296/8,1296/8,1296/8,1296/4,1296/4,1296/4,1280/4,1296/4},
	/*  9: 1280x720p@50 */ {1296/8,1296/8,1296/8,1296/4,1296/4,1296/4,1280/4,1296/4},
	/* 10: 1280x720p@30 */ {1296/8,1296/8,1296/8,1296/4,1296/4,1296/4,1280/4,1296/4},
	/* 11: 1280x720p@25 */ {1296/8,1296/8,1296/8,1296/4,1296/4,1296/4,1280/4,1296/4},
	/* 12: 1280x960p@30 */ {1296/8,1296/8,1296/8,1296/4,1296/4,1296/4,1280/4,1296/4},
	/* 13: 1280x960p@25 */ {1296/8,1296/8,1296/8,1296/4,1296/4,1296/4,1280/4,1296/4},
	/* 14: 1920x1080p@30 */{1936/8,1936/8,1936/8,1936/4,1936/4,1936/4,1920/4,1936/4},
	/* 15: 1920x1080p@25 */{1936/8,1936/8,1936/8,1936/4,1936/4,1936/4,1920/4,1936/4},
	/* 16:  800x480p@30 */ { 800/8, 800/8, 800/8, 800/4, 800/4, 800/4, 800/4, 800/4}, //don't support
	/* 17:  800x480p@25 */ { 800/8, 800/8, 800/8, 800/4, 800/4, 800/4, 800/4, 800/4}, //don't support
	/* 18: 1024x600p@30 */ {1024/8,1024/8,1024/8,1024/4,1024/4,1024/4,1024/4,1024/4}, //don't support
	/* 19: 1024x600p@25 */ {1024/8,1024/8,1024/8,1024/4,1024/4,1024/4,1024/4,1024/4}, //don't support
};

const int DEF_VIN_VACTIVE_SIZE[2][max_vid_resol] = {
	{ // Normal
		/*  0:  720x480i@60 */  240,
		/*  1:  720x576i@50 */  288,
		/*  2:  960x480i@60 */  240,
		/*  3:  960x576i@50 */  288,
		/*  4:  720x480p@60 */  480,
		/*  5:  720x576p@50 */  576,
		/*  6:  960x480p@60 */  480,
		/*  7:  960x576p@50 */  576,
		/*  8: 1280x720p@60 */  720,
		/*  9: 1280x720p@50 */  720,
		/* 10: 1280x720p@30 */  720,
		/* 11: 1280x720p@25 */  720,
		/* 12: 1280x960p@30 */  960,
		/* 13: 1280x960p@25 */  960,
		/* 14: 1920x1080p@30 */1080,
		/* 15: 1920x1080p@25 */1080,
		/* 16:  800x480p@30 */  480, //don't support
		/* 17:  800x480p@25 */  480, //don't support
		/* 18: 1024x600@30 */   600, //don't support
		/* 19: 1024x600@25 */   600, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/*  0:  720x480i@60 */  240+16,
		/*  1:  720x576i@50 */  288+16,
		/*  2:  960x480i@60 */  240+16,
		/*  3:  960x576i@50 */  288+16,
		/*  4:  720x480p@60 */  480+16,
		/*  5:  720x576p@50 */  576+16,
		/*  6:  960x480p@60 */  480+16,
		/*  7:  960x576p@50 */  576+16,
		/*  8: 1280x720p@60 */  720+16,
		/*  9: 1280x720p@50 */  720+16,
		/* 10: 1280x720p@30 */  720+16,
		/* 11: 1280x720p@25 */  720+16,
		/* 12: 1280x960p@30 */  960+16,
		/* 13: 1280x960p@25 */  960+16,
		/* 14: 1920x1080p@30 */1080+16,
		/* 15: 1920x1080p@25 */1080+16,
		/* 16:  800x480p@30 */  480+16, //don't support
		/* 17:  800x480p@25 */  480+16, //don't support
		/* 18: 1024x600@30 */   600+16, //don't support
		/* 19: 1024x600@25 */   600+16, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/*  0:  720x480i@60 */  240+16,
		/*  1:  720x576i@50 */  288+16,
		/*  2:  960x480i@60 */  240+16,
		/*  3:  960x576i@50 */  288+16,
		/*  4:  720x480p@60 */  480+16,
		/*  5:  720x576p@50 */  576+16,
		/*  6:  960x480p@60 */  480+16,
		/*  7:  960x576p@50 */  576+16,
		/*  8: 1280x720p@60 */  720+16,
		/*  9: 1280x720p@50 */  720+16,
		/* 10: 1280x720p@30 */  720+16,
		/* 11: 1280x720p@25 */  720+16,
		/* 12: 1280x960p@30 */  960+16,
		/* 13: 1280x960p@25 */  960+16,
		/* 14: 1920x1080p@30 */1080+16,
		/* 15: 1920x1080p@25 */1080+16,
		/* 16:  800x480p@30 */  480+16, //don't support
		/* 17:  800x480p@25 */  480+16, //don't support
		/* 18: 1024x600@30 */   600+16, //don't support
		/* 19: 1024x600@25 */   600+16, //don't support
	},
#else
	{ // User define
		/*  0:  720x480i@60 */  240+16,
		/*  1:  720x576i@50 */  288+16,
		/*  2:  960x480i@60 */  240+16,
		/*  3:  960x576i@50 */  288+16,
		/*  4:  720x480p@60 */  480+16,
		/*  5:  720x576p@50 */  576+16,
		/*  6:  960x480p@60 */  480+16,
		/*  7:  960x576p@50 */  576+16,
		/*  8: 1280x720p@60 */  720+16,
		/*  9: 1280x720p@50 */  720+16,
		/* 10: 1280x720p@30 */  720+16,
		/* 11: 1280x720p@25 */  720+16,
		/* 12: 1280x960p@30 */  960+16,
		/* 13: 1280x960p@25 */  960+16,
		/* 14: 1920x1080p@30 */1080+16,
		/* 15: 1920x1080p@25 */1080+16,
		/* 16:  800x480p@30 */  480+16, //don't support
		/* 17:  800x480p@25 */  480+16, //don't support
		/* 18: 1024x600@30 */   600+16, //don't support
		/* 19: 1024x600@25 */   600+16, //don't support
	},
#endif
};

const int DEF_VIN_HACTIVE_SIZE[2][max_vid_resol] = {
	{ // Normal
		/*  0:  720x480i@60 */  720,
		/*  1:  720x576i@50 */  720,
		/*  2:  960x480i@60 */  960,
		/*  3:  960x576i@50 */  960,
		/*  4:  720x480p@60 */  720,
		/*  5:  720x576p@50 */  720,
		/*  6:  960x480p@60 */  960,
		/*  7:  960x576p@50 */  960,
		/*  8: 1280x720p@60 */ 1280,
		/*  9: 1280x720p@50 */ 1280,
		/* 10: 1280x720p@30 */ 1280,
		/* 11: 1280x720p@25 */ 1280,
		/* 12: 1280x960p@30 */ 1280,
		/* 13: 1280x960p@25 */ 1280,
		/* 14: 1920x1080p@30 */1920,
		/* 15: 1920x1080p@25 */1920,
		/* 16:  800x480p@30 */  800, //don't support
		/* 17:  800x480p@25 */  800, //don't support
		/* 18: 1024x600p@30 */ 1024, //don't support
		/* 19: 1024x600p@25 */ 1024, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/*  0:  720x480i@60 */  720+16,
		/*  1:  720x576i@50 */  720+16,
		/*  2:  960x480i@60 */  960+16,
		/*  3:  960x576i@50 */  960+16,
		/*  4:  720x480p@60 */  720+16,
		/*  5:  720x576p@50 */  720+16,
		/*  6:  960x480p@60 */  960+16,
		/*  7:  960x576p@50 */  960+16,
		/*  8: 1280x720p@60 */ 1280+16,
		/*  9: 1280x720p@50 */ 1280+16,
		/* 10: 1280x720p@30 */ 1280+16,
		/* 11: 1280x720p@25 */ 1280+16,
		/* 12: 1280x960p@30 */ 1280+16,
		/* 13: 1280x960p@25 */ 1280+16,
		/* 14: 1920x1080p@30 */1920+16,
		/* 15: 1920x1080p@25 */1920+16,
		/* 16:  800x480p@30 */  800+16, //don't support
		/* 17:  800x480p@25 */  800+16, //don't support
		/* 18: 1024x600@30 */  1024+16, //don't support
		/* 19: 1024x600@25 */  1024+16, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/*  0:  720x480i@60 */  720+16,
		/*  1:  720x576i@50 */  720+16,
		/*  2:  960x480i@60 */  960+16,
		/*  3:  960x576i@50 */  960+16,
		/*  4:  720x480p@60 */  720+16,
		/*  5:  720x576p@50 */  720+16,
		/*  6:  960x480p@60 */  960+16,
		/*  7:  960x576p@50 */  960+16,
		/*  8: 1280x720p@60 */ 1280+16,
		/*  9: 1280x720p@50 */ 1280+16,
		/* 10: 1280x720p@30 */ 1280+16,
		/* 11: 1280x720p@25 */ 1280+16,
		/* 12: 1280x960p@30 */ 1280+16,
		/* 13: 1280x960p@25 */ 1280+16,
		/* 14: 1920x1080p@30 */1920+16,
		/* 15: 1920x1080p@25 */1920+16,
		/* 16:  800x480p@30 */  800+16, //don't support
		/* 17:  800x480p@25 */  800+16, //don't support
		/* 18: 1024x600@30 */  1024+16, //don't support
		/* 19: 1024x600@25 */  1024+16, //don't support
	},
#else
	{ // User define
		/*  0:  720x480i@60 */  720+16,
		/*  1:  720x576i@50 */  720+16,
		/*  2:  960x480i@60 */  960+16,
		/*  3:  960x576i@50 */  960+16,
		/*  4:  720x480p@60 */  720+16,
		/*  5:  720x576p@50 */  720+16,
		/*  6:  960x480p@60 */  960+16,
		/*  7:  960x576p@50 */  960+16,
		/*  8: 1280x720p@60 */ 1280+16,
		/*  9: 1280x720p@50 */ 1280+16,
		/* 10: 1280x720p@30 */ 1280+16,
		/* 11: 1280x720p@25 */ 1280+16,
		/* 12: 1280x960p@30 */ 1280+16,
		/* 13: 1280x960p@25 */ 1280+16,
		/* 14: 1920x1080p@30 */1920+16,
		/* 15: 1920x1080p@25 */1920+16,
		/* 16:  800x480p@30 */  800+16, //don't support
		/* 17:  800x480p@25 */  800+16, //don't support
		/* 18: 1024x600@30 */  1024+16, //don't support
		/* 19: 1024x600@25 */  1024+16, //don't support
	},
#endif
};

const int DEF_VIN_HTOTAL_SIZE[2][max_vid_resol] = {
	{ // Normal
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		/* 12: 1280x960p@30 */ 2475,
		/* 13: 1280x960p@25 */ 2970,
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 2475, //don't support
		/* 17:  800x480p@25 */ 2970, //don't support
		/* 18: 1024x600@30 */  2475, //don't support
		/* 19: 1024x600@25 */  2970, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		///* 10: 1280x720p@30 */ 1650, //OV97xx
		/* 10: 1280x720p@30 */ 1500, //AR014
		/* 11: 1280x720p@25 */ 1980,
		/* 12: 1280x960p@30 */ 2475,
		/* 13: 1280x960p@25 */ 2970,
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 1650, //don't support
		/* 17:  800x480p@25 */ 1980, //don't support
		/* 18: 1024x600@30 */  1650, //don't support
		/* 19: 1024x600@25 */  1980, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		/* 12: 1280x960p@30 */ 1650,
		/* 13: 1280x960p@25 */ 2970,
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 1650, //don't support
		/* 17:  800x480p@25 */ 1980, //don't support
		/* 18: 1024x600@30 */  1650, //don't support
		/* 19: 1024x600@25 */  1980, //don't support
	},
#else
	{ // User define
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		/* 12: 1280x960p@30 */ 2475,
		/* 13: 1280x960p@25 */ 2970,
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 1650, //don't support
		/* 17:  800x480p@25 */ 1980, //don't support
		/* 18: 1024x600@30 */  1650, //don't support
		/* 19: 1024x600@25 */  1980, //don't support
	},
#endif
};

const int DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[2][max_vid_resol][2] = {
	{ // Normal
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262,   263},
		/*  1:  720x576i@50 */ { 312,   313},
		/*  2:  960x480i@60 */ { 262,   263},
		/*  3:  960x576i@50 */ { 312,   313},
		/*  4:  720x480p@60 */ { 525,   0},
		/*  5:  720x576p@50 */ { 625,   0},
		/*  6:  960x480p@60 */ { 525,   0},
		/*  7:  960x576p@50 */ { 625,   0},
		/*  8: 1280x720p@60 */ { 750,   0},
		/*  9: 1280x720p@50 */ { 750,   0},
		/* 10: 1280x720p@30 */ { 750, 	0},
		/* 11: 1280x720p@25 */ { 750,   0},
		/* 12: 1280x960p@30 */ {1000,   0},
		/* 13: 1280x960p@25 */ {1000,   0},
		/* 14: 1920x1080p@30 */{1125,   0},
		/* 15: 1920x1080p@25 */{1125,   0},
		/* 16:  800x480p@30 */ {1000,   0}, //don't support
		/* 17:  800x480p@25 */ {1000,   0}, //don't support
		/* 18: 1024x600@30 */  {1000,   0}, //don't support
		/* 19: 1024x600@25 */  {1000,   0}, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262-3,   263-3},
		/*  1:  720x576i@50 */ { 312-3,   313-3},
		/*  2:  960x480i@60 */ { 262-3,   263-3},
		/*  3:  960x576i@50 */ { 312-3,   313-3},
		/*  4:  720x480p@60 */ { 525-3,   0},
		/*  5:  720x576p@50 */ { 625-3,   0},
		/*  6:  960x480p@60 */ { 525-3,   0},
		/*  7:  960x576p@50 */ { 625-3,   0},
		/*  8: 1280x720p@60 */ { 750-6,   0},
		/*  9: 1280x720p@50 */ { 750-6,   0},
		///* 10: 1280x720p@30 */ { 750-6,   0}, //OV97xx
		/* 10: 1280x720p@30 */ { 832-6,   0}, //AR0143
		/* 11: 1280x720p@25 */ { 750-6,   0},
		/* 12: 1280x960p@30 */ {1000-15,   0},
		/* 13: 1280x960p@25 */ {1000-15,   0},
		/* 14: 1920x1080p@30 */{1125-3,   0},
		/* 15: 1920x1080p@25 */{1125-3,   0},
		/* 16:  800x480p@30 */ {1000-3,   0}, //don't support
		/* 17:  800x480p@25 */ {1000-3,   0}, //don't support
		/* 18: 1024x600@30 */  {1000-3,   0}, //don't support
		/* 19: 1024x600@25 */  {1000-3,   0}, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262-3,   263-3},
		/*  1:  720x576i@50 */ { 312-3,   313-3},
		/*  2:  960x480i@60 */ { 262-3,   263-3},
		/*  3:  960x576i@50 */ { 312-3,   313-3},
		/*  4:  720x480p@60 */ { 525-3,   0},
		/*  5:  720x576p@50 */ { 625-3,   0},
		/*  6:  960x480p@60 */ { 525-3,   0},
		/*  7:  960x576p@50 */ { 625-3,   0},
		/*  8: 1280x720p@60 */ { 750-6,   0},
		/*  9: 1280x720p@50 */ { 750-6,   0},
		/* 10: 1280x720p@30 */ {   829,   0},
		/* 11: 1280x720p@25 */ { 750-6,   0},
		/* 12: 1280x960p@30 */ {  1125,   0},
		/* 13: 1280x960p@25 */ {1000-15,   0},
		/* 14: 1920x1080p@30 */{1125-3,   0},
		/* 15: 1920x1080p@25 */{1125-3,   0},
		/* 16:  800x480p@30 */ {1000-3,   0}, //don't support
		/* 17:  800x480p@25 */ {1000-3,   0}, //don't support
		/* 18: 1024x600@30 */  {1000-3,   0}, //don't support
		/* 19: 1024x600@25 */  {1000-3,   0}, //don't support
	},
#else
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262-3,   263-3},
		/*  1:  720x576i@50 */ { 312-3,   313-3},
		/*  2:  960x480i@60 */ { 262-3,   263-3},
		/*  3:  960x576i@50 */ { 312-3,   313-3},
		/*  4:  720x480p@60 */ { 525-3,   0},
		/*  5:  720x576p@50 */ { 625-3,   0},
		/*  6:  960x480p@60 */ { 525-3,   0},
		/*  7:  960x576p@50 */ { 625-3,   0},
		/*  8: 1280x720p@60 */ { 750-6,   0},
		/*  9: 1280x720p@50 */ { 750-6,   0},
		/* 10: 1280x720p@30 */ { 750-6,   0},
		/* 11: 1280x720p@25 */ { 750-6,   0},
		/* 12: 1280x960p@30 */ {1000-15,   0},
		/* 13: 1280x960p@25 */ {1000-15,   0},
		/* 14: 1920x1080p@30 */{1125-3,   0},
		/* 15: 1920x1080p@25 */{1125-3,   0},
		/* 16:  800x480p@30 */ {1000-3,   0}, //don't support
		/* 17:  800x480p@25 */ {1000-3,   0}, //don't support
		/* 18: 1024x600@30 */  {1000-3,   0}, //don't support
		/* 19: 1024x600@25 */  {1000-3,   0}, //don't support
	},
#endif
};

const int DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[2][max_vid_resol][2] = {
	{ // Normal
		/* odd, evne */
		/*  0:  720x480i@60 */ {243, 242},
		/*  1:  720x576i@50 */ {291, 290},
		/*  2:  960x480i@60 */ {243, 242},
		/*  3:  960x576i@50 */ {291, 290},
		/*  4:  720x480p@60 */ {  0,   0},
		/*  5:  720x576p@50 */ {  0,   0},
		/*  6:  960x480p@60 */ {  0,   0},
		/*  7:  960x576p@50 */ {  0,   0},
		/*  8: 1280x720p@60 */ {  0,   0},
		/*  9: 1280x720p@50 */ {  0,   0},
		/* 10: 1280x720p@30 */ {  0,   0},
		/* 11: 1280x720p@25 */ {  0,   0},
		/* 12: 1280x960p@30 */ {  0,   0},
		/* 13: 1280x960p@25 */ {  0,   0},
		/* 14: 1920x1080p@30 */{  0,   0},
		/* 15: 1920x1080p@25 */{  0,   0},
		/* 16:  800x480p@30 */ {  0,   0}, //don't support
		/* 17:  800x480p@25 */ {  0,   0}, //don't support
		/* 18: 1024x600@30 */  {  0,   0}, //don't support
		/* 19: 1024x600@25 */  {  0,   0}, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ {243, 242},
		/*  1:  720x576i@50 */ {291, 290},
		/*  2:  960x480i@60 */ {243, 242},
		/*  3:  960x576i@50 */ {291, 290},
		/*  4:  720x480p@60 */ {  0,   0},
		/*  5:  720x576p@50 */ {  0,   0},
		/*  6:  960x480p@60 */ {  0,   0},
		/*  7:  960x576p@50 */ {  0,   0},
		/*  8: 1280x720p@60 */ {  0,   0},
		/*  9: 1280x720p@50 */ {  0,   0},
		/* 10: 1280x720p@30 */ {  0,   0},
		/* 11: 1280x720p@25 */ {  0,   0},
		/* 12: 1280x960p@30 */ {  0,   0},
		/* 13: 1280x960p@25 */ {  0,   0},
		/* 14: 1920x1080p@30 */{  0,   0},
		/* 15: 1920x1080p@25 */{  0,   0},
		/* 16:  800x480p@30 */ {  0,   0}, //don't support
		/* 17:  800x480p@25 */ {  0,   0}, //don't support
		/* 18: 1024x600@30 */  {  0,   0}, //don't support
		/* 19: 1024x600@25 */  {  0,   0}, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ {243, 242},
		/*  1:  720x576i@50 */ {291, 290},
		/*  2:  960x480i@60 */ {243, 242},
		/*  3:  960x576i@50 */ {291, 290},
		/*  4:  720x480p@60 */ {  0,   0},
		/*  5:  720x576p@50 */ {  0,   0},
		/*  6:  960x480p@60 */ {  0,   0},
		/*  7:  960x576p@50 */ {  0,   0},
		/*  8: 1280x720p@60 */ {  0,   0},
		/*  9: 1280x720p@50 */ {  0,   0},
		/* 10: 1280x720p@30 */ {  0,   0},
		/* 11: 1280x720p@25 */ {  0,   0},
		/* 12: 1280x960p@30 */ {  0,   0},
		/* 13: 1280x960p@25 */ {  0,   0},
		/* 14: 1920x1080p@30 */{  0,   0},
		/* 15: 1920x1080p@25 */{  0,   0},
		/* 16:  800x480p@30 */ {  0,   0}, //don't support
		/* 17:  800x480p@25 */ {  0,   0}, //don't support
		/* 18: 1024x600@30 */  {  0,   0}, //don't support
		/* 19: 1024x600@25 */  {  0,   0}, //don't support
	},
#else
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ {243, 242},
		/*  1:  720x576i@50 */ {291, 290},
		/*  2:  960x480i@60 */ {243, 242},
		/*  3:  960x576i@50 */ {291, 290},
		/*  4:  720x480p@60 */ {  0,   0},
		/*  5:  720x576p@50 */ {  0,   0},
		/*  6:  960x480p@60 */ {  0,   0},
		/*  7:  960x576p@50 */ {  0,   0},
		/*  8: 1280x720p@60 */ {  0,   0},
		/*  9: 1280x720p@50 */ {  0,   0},
		/* 10: 1280x720p@30 */ {  0,   0},
		/* 11: 1280x720p@25 */ {  0,   0},
		/* 12: 1280x960p@30 */ {  0,   0},
		/* 13: 1280x960p@25 */ {  0,   0},
		/* 14: 1920x1080p@30 */{  0,   0},
		/* 15: 1920x1080p@25 */{  0,   0},
		/* 16:  800x480p@30 */ {  0,   0}, //don't support
		/* 17:  800x480p@25 */ {  0,   0}, //don't support
		/* 18: 1024x600@30 */  {  0,   0}, //don't support
		/* 19: 1024x600@25 */  {  0,   0}, //don't support
	},
#endif
};

const int DEF_VIN_MAXLINE_CNT[2][max_vid_resol] = {
	{ // Normal
		/*  0:  720x480i@60 */  525,
		/*  1:  720x576i@50 */  625,
		/*  2:  960x480i@60 */  525,
		/*  3:  960x576i@50 */  625,
		/*  4:  720x480p@60 */  525,
		/*  5:  720x576p@50 */  625,
		/*  6:  960x480p@60 */  525,
		/*  7:  960x576p@50 */  625,
		/*  8: 1280x720p@60 */  750,
		/*  9: 1280x720p@50 */  750,
		/* 10: 1280x720p@30 */  750,
		/* 11: 1280x720p@25 */  750,
		/* 12: 1280x960p@30 */ 1000,
		/* 13: 1280x960p@25 */ 1000,
		/* 14: 1920x1080p@30 */1125,
		/* 15: 1920x1080p@25 */1125,
		/* 16:  800x480p@30 */ 1000, //don't support
		/* 17:  800x480p@25 */ 1000, //don't support
		/* 18: 1024x600@30 */  1000, //don't support
		/* 19: 1024x600@25 */  1000, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/*  0:  720x480i@60 */  525,
		/*  1:  720x576i@50 */  625,
		/*  2:  960x480i@60 */  525,
		/*  3:  960x576i@50 */  625,
		/*  4:  720x480p@60 */  525,
		/*  5:  720x576p@50 */  625,
		/*  6:  960x480p@60 */  525,
		/*  7:  960x576p@50 */  625,
		/*  8: 1280x720p@60 */  750,
		/*  9: 1280x720p@50 */  750,
		/* 10: 1280x720p@30 */  750,
		/* 11: 1280x720p@25 */  750,
		/* 12: 1280x960p@30 */ 1000,
		/* 13: 1280x960p@25 */ 1000,
		/* 14: 1920x1080p@30 */1125,
		/* 15: 1920x1080p@25 */1125,
		/* 16:  800x480p@30 */ 1000, //don't support
		/* 17:  800x480p@25 */ 1000, //don't support
		/* 18: 1024x600@30 */  1000, //don't support
		/* 19: 1024x600@25 */  1000, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/*  0:  720x480i@60 */  525,
		/*  1:  720x576i@50 */  625,
		/*  2:  960x480i@60 */  525,
		/*  3:  960x576i@50 */  625,
		/*  4:  720x480p@60 */  525,
		/*  5:  720x576p@50 */  625,
		/*  6:  960x480p@60 */  525,
		/*  7:  960x576p@50 */  625,
		/*  8: 1280x720p@60 */  750,
		/*  9: 1280x720p@50 */  750,
		/* 10: 1280x720p@30 */  750,
		/* 11: 1280x720p@25 */  750,
		/* 12: 1280x960p@30 */ 1000,
		/* 13: 1280x960p@25 */ 1000,
		/* 14: 1920x1080p@30 */1125,
		/* 15: 1920x1080p@25 */1125,
		/* 16:  800x480p@30 */ 1000, //don't support
		/* 17:  800x480p@25 */ 1000, //don't support
		/* 18: 1024x600@30 */  1000, //don't support
		/* 19: 1024x600@25 */  1000, //don't support
	},
#else
	{ // User define
		/*  0:  720x480i@60 */  525,
		/*  1:  720x576i@50 */  625,
		/*  2:  960x480i@60 */  525,
		/*  3:  960x576i@50 */  625,
		/*  4:  720x480p@60 */  525,
		/*  5:  720x576p@50 */  625,
		/*  6:  960x480p@60 */  525,
		/*  7:  960x576p@50 */  625,
		/*  8: 1280x720p@60 */  750,
		/*  9: 1280x720p@50 */  750,
		/* 10: 1280x720p@30 */  750,
		/* 11: 1280x720p@25 */  750,
		/* 12: 1280x960p@30 */ 1000,
		/* 13: 1280x960p@25 */ 1000,
		/* 14: 1920x1080p@30 */1125,
		/* 15: 1920x1080p@25 */1125,
		/* 16:  800x480p@30 */ 1000, //don't support
		/* 17:  800x480p@25 */ 1000, //don't support
		/* 18: 1024x600@30 */  1000, //don't support
		/* 19: 1024x600@25 */  1000, //don't support
	},
#endif
};

const int DEF_VIN_RECOUT_DATARATE[max_vid_resol] = {
                                     //8bit       16bit
	/*  0:  720x480i@60 */    3, //27Mhz   ,  13.5Mhz
	/*  1:  720x576i@50 */    3, //27Mhz   ,  13.5Mhz
	/*  2:  960x480i@60 */    2, //36Mhz   ,  18Mhz
	/*  3:  960x576i@50 */    2, //36Mhz   ,  18Mhz
	/*  4:  720x480p@60 */    1, //74.25Mhz,  37.125Mhz
	/*  5:  720x576p@50 */    1, //74.25Mhz,  37.125Mhz
	/*  6:  960x480p@60 */    1, //74.25Mhz,  37.125Mhz
	/*  7:  960x576p@50 */    1, //74.25Mhz,  37.125Mhz
	/*  8: 1280x720p@60 */    1, //74.25Mhz,  37.125Mhz
	/*  9: 1280x720p@50 */    1, //74.25Mhz,  37.125Mhz
	/* 10: 1280x720p@30 */    1, //74.25Mhz,  37.125Mhz
	/* 11: 1280x720p@25 */    1, //74.25Mhz,  37.125Mhz
	/* 12: 1280x960p@30 */    0, //148.5Mhz,  74.25Mhz
	/* 13: 1280x960p@25 */    0, //148.5Mhz,  74.25Mhz
	/* 14: 1920x1080p@30 */   0, //148.5Mhz,  74.25Mhz
	/* 15: 1920x1080p@25 */   0, //148.5Mhz,  74.25Mhz
	/* 16:  800x480p@30 */    0, //148.5Mhz,  74.25Mhz //don't support
	/* 17:  800x480p@25 */    0, //148.5Mhz,  74.25Mhz //don't support
	/* 18: 1024x600@30 */     0, //148.5Mhz,  74.25Mhz //don't support
	/* 19: 1024x600@25 */     0, //148.5Mhz,  74.25Mhz //don't support
};

const int DEF_VIN_QUAD_HV_WIN_SIZE[max_vid_resol][2] = {
	/* h, v */
	/*  0:  720x480i@60 */ { 720/2/8, 240/2},
	/*  1:  720x576i@50 */ { 720/2/8, 288/2},
	/*  2:  960x480i@60 */ { 960/2/8, 240/2},
	/*  3:  960x576i@50 */ { 960/2/8, 288/2},
	/*  4:  720x480p@60 */ { 720/2/8, 480/2},
	/*  5:  720x576p@50 */ { 720/2/8, 576/2},
	/*  6:  960x480p@60 */ { 960/2/8, 480/2},
	/*  7:  960x576p@50 */ { 960/2/8, 576/2},
	/*  8: 1280x720p@60 */ {1280/2/8, 720/2},
	/*  9: 1280x720p@50 */ {1280/2/8, 720/2},
	/* 10: 1280x720p@30 */ {1280/2/8, 720/2},
	/* 11: 1280x720p@25 */ {1280/2/8, 720/2},
	/* 12: 1280x960p@30 */ {1280/2/8, 960/2},
	/* 13: 1280x960p@25 */ {1280/2/8, 960/2},
	/* 14: 1920x1080p@30 */{1920/2/8,1080/2},
	/* 15: 1920x1080p@25 */{1920/2/8,1080/2},
	/* 16:  800x480p@30 */ { 800/2/8, 480/2}, //don't support
	/* 17:  800x480p@25 */ { 800/2/8, 480/2}, //don't support
	/* 18: 1024x600@30 */  {1024/2/8, 600/2}, //don't support
	/* 19: 1024x600@25 */  {1024/2/8, 600/2}, //don't support
};

const char *DEF_STR_VIN_RESOL[max_vid_resol] = {
	"vres_720x480i60", 	 	// 0 
	"vres_720x576i50",  		// 1 
	"vres_960x480i60",  		// 2 
	"vres_960x576i50",  		// 3 
	"vres_720x480p60",  		// 4 
	"vres_720x576p50",  		// 5 
	"vres_960x480p60",  		// 6 
	"vres_960x576p50",  		// 7 
	"vres_1280x720p60",  		// 8 
	"vres_1280x720p50",  		// 9 
	"vres_1280x720p30",  		//10 
	"vres_1280x720p25",  		//11 
	"vres_1280x960p30",  		//12 
	"vres_1280x960p25",  		//13 
	"vres_1920x1080p30", 		//14 
	"vres_1920x1080p25"  		//15 
	"vres_800x480p60",  		//16  //don't support
	"vres_800x480p50",  		//17  //don't support
	"vres_1024x600p60",  		//18  //don't support
	"vres_1024x600p50",  		//19  //don't support
};

const char *DEF_STR_VIN_DATATYPE[max_vin_datatype]  = {
	"vin_RAW6",  		// 0 
	"vin_RAW7",  		// 1 
	"vin_RAW8",  		// 2 
	"vin_RAW10",  		// 3 
	"vin_RAW12",  		// 4 
	"vin_RAW14",  		// 5 
	"vin_YUV8_2xRAW8",  		// 6 
	"vin_2xRAW10"   		// 7 
};

const int DEF_VIN_QUAD_RD_HTOTAL_SIZE[2][max_vid_resol] = {
	{ // Normal
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		/* 12: 1280x960p@30 */ 2475,
		/* 13: 1280x960p@25 */ 2970,
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 2475, //don't support
		/* 17:  800x480p@25 */ 2970, //don't support
		/* 18: 1024x600@30 */  2475, //don't support
		/* 19: 1024x600@25 */  2970, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		///* 12: 1280x960p@30 */ 2475, 		//base
		///* 13: 1280x960p@25 */ 2970,      //base
		/* 12: 1280x960p@30 */ 1400, 		//94.5Mhz 960p30:1400(0x578), 960p25:1680(0x690)
		/* 13: 1280x960p@25 */ 1680,        //94.5Mhz 960p30:1400(0x578), 960p25:1680(0x690)
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 1650, //don't support
		/* 17:  800x480p@25 */ 1980, //don't support
		/* 18: 1024x600@30 */  1650, //don't support
		/* 19: 1024x600@25 */  1980, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		/* 12: 1280x960p@30 */ 1650,
		/* 13: 1280x960p@25 */ 2970,
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 1650, //don't support
		/* 17:  800x480p@25 */ 1980, //don't support
		/* 18: 1024x600@30 */  1650, //don't support
		/* 19: 1024x600@25 */  1980, //don't support
	},
#else
	{ // User define
		/*  0:  720x480i@60 */  858,
		/*  1:  720x576i@50 */  864,
		/*  2:  960x480i@60 */ 1144,
		/*  3:  960x576i@50 */ 1152,
		/*  4:  720x480p@60 */  858,
		/*  5:  720x576p@50 */  864,
		/*  6:  960x480p@60 */ 1144,
		/*  7:  960x576p@50 */ 1152,
		/*  8: 1280x720p@60 */ 1650,
		/*  9: 1280x720p@50 */ 1980,
		/* 10: 1280x720p@30 */ 1650,
		/* 11: 1280x720p@25 */ 1980,
		///* 12: 1280x960p@30 */ 2475,      //base
		///* 13: 1280x960p@25 */ 2970,      //base
		/* 12: 1280x960p@30 */ 1400, 		//94.5Mhz 960p30:1400(0x578), 960p25:1680(0x690)
		/* 13: 1280x960p@25 */ 1680,        //94.5Mhz 960p30:1400(0x578), 960p25:1680(0x690)
		/* 14: 1920x1080p@30 */2200,
		/* 15: 1920x1080p@25 */2640,
		/* 16:  800x480p@30 */ 1650, //don't support
		/* 17:  800x480p@25 */ 1980, //don't support
		/* 18: 1024x600@30 */  1650, //don't support
		/* 19: 1024x600@25 */  1980, //don't support
	},
#endif
};

const int DEF_VIN_QUAD_RD_ODD_EVEN_FIELD_VTOTAL_SIZE[2][max_vid_resol][2] = {
	{ // Normal
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262,   263},
		/*  1:  720x576i@50 */ { 312,   313},
		/*  2:  960x480i@60 */ { 262,   263},
		/*  3:  960x576i@50 */ { 312,   313},
		/*  4:  720x480p@60 */ { 525,   0},
		/*  5:  720x576p@50 */ { 625,   0},
		/*  6:  960x480p@60 */ { 525,   0},
		/*  7:  960x576p@50 */ { 625,   0},
		/*  8: 1280x720p@60 */ { 750,   0},
		/*  9: 1280x720p@50 */ { 750,   0},
		/* 10: 1280x720p@30 */ { 750, 	0},
		/* 11: 1280x720p@25 */ { 750,   0},
		/* 12: 1280x960p@30 */ {1000,   0},
		/* 13: 1280x960p@25 */ {1000,   0},
		/* 14: 1920x1080p@30 */{1125,   0},
		/* 15: 1920x1080p@25 */{1125,   0},
		/* 16:  800x480p@30 */ {1000,   0}, //don't support
		/* 17:  800x480p@25 */ {1000,   0}, //don't support
		/* 18: 1024x600@30 */  {1000,   0}, //don't support
		/* 19: 1024x600@25 */  {1000,   0}, //don't support
	},
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262-3,   263-3},
		/*  1:  720x576i@50 */ { 312-3,   313-3},
		/*  2:  960x480i@60 */ { 262-3,   263-3},
		/*  3:  960x576i@50 */ { 312-3,   313-3},
		/*  4:  720x480p@60 */ { 525-3,   0},
		/*  5:  720x576p@50 */ { 625-3,   0},
		/*  6:  960x480p@60 */ { 525-3,   0},
		/*  7:  960x576p@50 */ { 625-3,   0},
		/*  8: 1280x720p@60 */ { 750-6,   0},
		/*  9: 1280x720p@50 */ { 750-6,   0},
		/* 10: 1280x720p@30 */ { 750-6,   0},
		/* 11: 1280x720p@25 */ { 750-6,   0},
		///* 12: 1280x960p@30 */ {1000-15,   0},    //base
		///* 13: 1280x960p@25 */ {1000-15,   0},    //base
		/* 12: 1280x960p@30 */ {1125,   0}, //94.5Mhz 960p30:1125(0x465), 960p25:1125(0x465)
		/* 13: 1280x960p@25 */ {1125,   0}, //94.5Mhz 960p30:1125(0x465), 960p25:1125(0x465)
		/* 14: 1920x1080p@30 */{1125-3,   0},
		/* 15: 1920x1080p@25 */{1125-3,   0},
		/* 16:  800x480p@30 */ {1000-3,   0}, //don't support
		/* 17:  800x480p@25 */ {1000-3,   0}, //don't support
		/* 18: 1024x600@30 */  {1000-3,   0}, //don't support
		/* 19: 1024x600@25 */  {1000-3,   0}, //don't support
	},
#elif ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) )
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262-3,   263-3},
		/*  1:  720x576i@50 */ { 312-3,   313-3},
		/*  2:  960x480i@60 */ { 262-3,   263-3},
		/*  3:  960x576i@50 */ { 312-3,   313-3},
		/*  4:  720x480p@60 */ { 525-3,   0},
		/*  5:  720x576p@50 */ { 625-3,   0},
		/*  6:  960x480p@60 */ { 525-3,   0},
		/*  7:  960x576p@50 */ { 625-3,   0},
		/*  8: 1280x720p@60 */ { 750-6,   0},
		/*  9: 1280x720p@50 */ { 750-6,   0},
		/* 10: 1280x720p@30 */ {   829,   0},
		/* 11: 1280x720p@25 */ { 750-6,   0},
		/* 12: 1280x960p@30 */ {  1125,   0},
		/* 13: 1280x960p@25 */ {1000-15,   0},
		/* 14: 1920x1080p@30 */{1125-3,   0},
		/* 15: 1920x1080p@25 */{1125-3,   0},
		/* 16:  800x480p@30 */ {1000-3,   0}, //don't support
		/* 17:  800x480p@25 */ {1000-3,   0}, //don't support
		/* 18: 1024x600@30 */  {1000-3,   0}, //don't support
		/* 19: 1024x600@25 */  {1000-3,   0}, //don't support
	},
#else
	{ // User define
		/* odd, evne */
		/*  0:  720x480i@60 */ { 262-3,   263-3},
		/*  1:  720x576i@50 */ { 312-3,   313-3},
		/*  2:  960x480i@60 */ { 262-3,   263-3},
		/*  3:  960x576i@50 */ { 312-3,   313-3},
		/*  4:  720x480p@60 */ { 525-3,   0},
		/*  5:  720x576p@50 */ { 625-3,   0},
		/*  6:  960x480p@60 */ { 525-3,   0},
		/*  7:  960x576p@50 */ { 625-3,   0},
		/*  8: 1280x720p@60 */ { 750-6,   0},
		/*  9: 1280x720p@50 */ { 750-6,   0},
		/* 10: 1280x720p@30 */ { 750-6,   0},
		/* 11: 1280x720p@25 */ { 750-6,   0},
		///* 12: 1280x960p@30 */ {1000-15,   0},    //base
		///* 13: 1280x960p@25 */ {1000-15,   0},    //base
		/* 12: 1280x960p@30 */ {1125,   0}, //94.5Mhz 960p30:1125(0x465), 960p25:1125(0x465)
		/* 13: 1280x960p@25 */ {1125,   0}, //94.5Mhz 960p30:1125(0x465), 960p25:1125(0x465)
		/* 14: 1920x1080p@30 */{1125-3,   0},
		/* 15: 1920x1080p@25 */{1125-3,   0},
		/* 16:  800x480p@30 */ {1000-3,   0}, //don't support
		/* 17:  800x480p@25 */ {1000-3,   0}, //don't support
		/* 18: 1024x600@30 */  {1000-3,   0}, //don't support
		/* 19: 1024x600@25 */  {1000-3,   0}, //don't support
	},
#endif
};

////////////////////////////////////////////////////////////////////////////////////////////////

/* define video in type. Normal or User define. The value is array num. */
#define VIDEO_IN_TYPE_NORMAL	(0)
#define VIDEO_IN_TYPE_USER_DEF	(1)

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)

const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_YUV8_2xRAW8; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */

/* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8
};

/* define video in type. Normal or User define. The value is array num. */
const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};

#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
 #if ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_RAW6)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW6; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_RAW6, vin_RAW6, vin_RAW6, vin_RAW6
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_RAW7)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW7; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_RAW7, vin_RAW7, vin_RAW7, vin_RAW7
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_RAW8)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW8; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_RAW8, vin_RAW8, vin_RAW8, vin_RAW8
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_RAW10)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW10; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_RAW10, vin_RAW10, vin_RAW10, vin_RAW10
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_RAW12)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW12; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_RAW12, vin_RAW12, vin_RAW12, vin_RAW12
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_RAW14)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW14; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_RAW14, vin_RAW14, vin_RAW14, vin_RAW14
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_YUV8_2XRAW8)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_YUV8_2xRAW8; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #elif ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_2XRAW10)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_2xRAW10; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_2xRAW10, vin_2xRAW10, vin_2xRAW10, vin_2xRAW10
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF, VIDEO_IN_TYPE_USER_DEF};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #else
  #error "Invalid BD_CAMERA_IN_FMT on MIPI_BAYER"
 #endif
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
 #if ((BD_CAMERA_IN_FMT & MIPI_VID_BIT_MASKBIT) == MIPI_VID_BIT_YUV8_2XRAW8)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_YUV8_2xRAW8; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
	vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8
  };
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
 #else
  #error "Invalid BD_CAMERA_IN_FMT on MIPI_YUV"
 #endif
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) //reserved
# if ( (BD_VID_PARALLEL_IN_FMT & VID_PARALLEL_TYPE_MASKBIT) == VID_PARALLEL_TYPE_VIN_BAYER)
#  if ( (BD_CAMERA_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_BAYER_8BIT )
   const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW8; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
   
   /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
   const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
   	vin_RAW8, vin_RAW8, vin_RAW8, vin_RAW8
   };
   
   /* define video in type. Normal or User define. The value is array num. */
   const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
   const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
#  elif ( (BD_CAMERA_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_BAYER_10BIT )
   const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW10; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
   
   /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
   const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
   	vin_RAW10, vin_RAW10, vin_RAW10, vin_RAW10
   };
   
   /* define video in type. Normal or User define. The value is array num. */
   const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
   const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
#  elif ( (BD_CAMERA_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_BAYER_12BIT )
   const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_RAW12; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
   
   /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
   const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
   	vin_RAW12, vin_RAW12, vin_RAW12, vin_RAW12
   };
   
   /* define video in type. Normal or User define. The value is array num. */
   const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
   const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
#  else
   #error "Invalid Parallel bayer type"
#  endif
# elif ( (BD_VID_PARALLEL_IN_FMT & VID_PARALLEL_TYPE_MASKBIT) == VID_PARALLEL_TYPE_VIN_YUV)
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawTypeMIPI = vin_YUV8_2xRAW8; /* MIPI: vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  
  /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
  const _VIN_DATATYPE PI_BD_CONFIG_VinRawType[4] = {
  	vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8, vin_YUV8_2xRAW8
  };
  
  /* define video in type. Normal or User define. The value is array num. */
  const int PI_BD_CONFIG_VinVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
  const int PI_BD_CONFIG_ResyncVideoType[4] = {VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL, VIDEO_IN_TYPE_NORMAL};
# else
  #error "Invalid Parallel type"
# endif

#else
#error "Invalid Video Type define"
#endif

#if ( (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1) )
 /* define video in type. Normal or User define. The value is array num. */
 const int PI_BD_CONFIG_PB_VinVideoType[1] = {VIDEO_IN_TYPE_NORMAL};
 const int PI_BD_CONFIG_PB_ResyncVideoType[1] = {VIDEO_IN_TYPE_NORMAL};
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* define vinPath */
void PI_VIN_GetUserConfigVar(_VIN_PATH *pVin, _VIN_USER_CONFIG_REG *pVinUserReg)
{
    if( (pVin == NULL) && (pVinUserReg == NULL) )
    {
        return;
    }

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
	/* Define In/Out format */
	{/*{{{*/
		pVin->vidIn[0] = vinVidInPath_PVI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[1] = vinVidInPath_PVI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[2] = vinVidInPath_PVI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[3] = vinVidInPath_PVI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidIn[4] = vinVidInPath_PARA0; /* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#else
		pVin->vidIn[4] = vinVidInPath_PVI; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#endif

		pVin->vidPort[0] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[1] = vinVidInPort1; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[2] = vinVidInPort2; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[3] = vinVidInPort3; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#else
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#endif

		pVin->vidResync[0] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[1] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[2] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[3] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */

		pVin->svmIn[0] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[1] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[2] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[3] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */

		pVin->svmPort[0] = vinSvmPort0; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[1] = vinSvmPort1; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[2] = vinSvmPort2; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[3] = vinSvmPort3; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */


		pVin->quadIn[0] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[1] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[2] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[3] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */

		pVin->quadPort[0] = vinQuadPort0; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[1] = vinQuadPort1; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[2] = vinQuadPort2; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[3] = vinQuadPort3; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */

		pVin->vpuOut = vinVpuOutPath_Quad;  	/* vinVpuOutPath_VidIn, vinVpuOutPath_Pb, vinVpuOutPath_Quad, vinVpuOutPath_TP */
		pVin->vpuPort = vinVpuOutPortQuad; 	/* vinVpuOutInPath_VidIn: vinVpuOutPort0, vinVpuOutPort1, vinVpuOutPort2, vinVpuOutPort3 */ /* else: vinVpuOutPort4, vinVpuOutPortQuad, vinVpuOutPortTP */

		pVin->cannyOut = vinCannyOutPath_VidIn; /* vinCannyOutPath_VidIn, vinCannyOutPath_Pb, vinCannyOutPath_Quad, vinCannyOutPath_TP */
		pVin->cannyPort = vinCannyOutPort0; 	/* vinCannyOutInPath_VidIn: vinCannyOutPort0, vinCannyOutPort1, vinCannyOutPort2, vinCannyOutPort3 */ /* else: vinCannyOutPort4, vinCannyOutPortQuad, vinCannyOutPortTP */

		pVin->recOut[0] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[1] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[2] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[3] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */

		pVin->recPort[0] = vinRecOutPort0; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[1] = vinRecOutPort1; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[2] = vinRecOutPort2; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[3] = vinRecOutPort3; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
	}/*}}}*/

#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
	/* Define In/Out format */
	{/*{{{*/
		pVin->vidIn[0] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[1] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[2] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[3] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidIn[4] = vinVidInPath_PARA0; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#else
		pVin->vidIn[4] = vinVidInPath_MIPI; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#endif

		pVin->vidPort[0] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[1] = vinVidInPort1; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[2] = vinVidInPort2; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[3] = vinVidInPort3; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#else
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#endif

		pVin->vidResync[0] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[1] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[2] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[3] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */

		pVin->svmIn[0] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[1] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[2] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[3] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */

		pVin->svmPort[0] = vinSvmPort0; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[1] = vinSvmPort1; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[2] = vinSvmPort2; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[3] = vinSvmPort3; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */


		pVin->quadIn[0] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[1] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[2] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[3] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */

		pVin->quadPort[0] = vinQuadPort0; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[1] = vinQuadPort1; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[2] = vinQuadPort2; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[3] = vinQuadPort3; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */

		pVin->vpuOut = vinVpuOutPath_Quad;  	/* vinVpuOutPath_VidIn, vinVpuOutPath_Pb, vinVpuOutPath_Quad, vinVpuOutPath_TP */
		pVin->vpuPort = vinVpuOutPortQuad; 	/* vinVpuOutInPath_VidIn: vinVpuOutPort0, vinVpuOutPort1, vinVpuOutPort2, vinVpuOutPort3 */ /* else: vinVpuOutPort4, vinVpuOutPortQuad, vinVpuOutPortTP */

		pVin->cannyOut = vinCannyOutPath_VidIn; /* vinCannyOutPath_VidIn, vinCannyOutPath_Pb, vinCannyOutPath_Quad, vinCannyOutPath_TP */
		pVin->cannyPort = vinCannyOutPort0; 	/* vinCannyOutInPath_VidIn: vinCannyOutPort0, vinCannyOutPort1, vinCannyOutPort2, vinCannyOutPort3 */ /* else: vinCannyOutPort4, vinCannyOutPortQuad, vinCannyOutPortTP */

		pVin->recOut[0] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[1] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[2] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[3] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */

		pVin->recPort[0] = vinRecOutPort0; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[1] = vinRecOutPort1; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[2] = vinRecOutPort2; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[3] = vinRecOutPort3; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
	}/*}}}*/

#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
	/* Define In/Out format */
	{/*{{{*/
		pVin->vidIn[0] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[1] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[2] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[3] = vinVidInPath_MIPI; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidIn[4] = vinVidInPath_PARA0; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#else
		pVin->vidIn[4] = vinVidInPath_MIPI; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#endif

		pVin->vidPort[0] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[1] = vinVidInPort2; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[2] = vinVidInPort3; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[3] = vinVidInPort1; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#else
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#endif

		pVin->vidResync[0] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[1] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[2] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[3] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */

		pVin->svmIn[0] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[1] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[2] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[3] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */

		pVin->svmPort[0] = vinSvmPort0; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[1] = vinSvmPort1; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[2] = vinSvmPort2; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[3] = vinSvmPort3; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */


		pVin->quadIn[0] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[1] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[2] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[3] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */

		pVin->quadPort[0] = vinQuadPort0; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[1] = vinQuadPort1; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[2] = vinQuadPort2; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[3] = vinQuadPort3; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */

		pVin->vpuOut = vinVpuOutPath_Quad;  	/* vinVpuOutPath_VidIn, vinVpuOutPath_Pb, vinVpuOutPath_Quad, vinVpuOutPath_TP */
		pVin->vpuPort = vinVpuOutPortQuad; 	/* vinVpuOutInPath_VidIn: vinVpuOutPort0, vinVpuOutPort1, vinVpuOutPort2, vinVpuOutPort3 */ /* else: vinVpuOutPort4, vinVpuOutPortQuad, vinVpuOutPortTP */

		pVin->cannyOut = vinCannyOutPath_VidIn; /* vinCannyOutPath_VidIn, vinCannyOutPath_Pb, vinCannyOutPath_Quad, vinCannyOutPath_TP */
		pVin->cannyPort = vinCannyOutPort0; 	/* vinCannyOutInPath_VidIn: vinCannyOutPort0, vinCannyOutPort1, vinCannyOutPort2, vinCannyOutPort3 */ /* else: vinCannyOutPort4, vinCannyOutPortQuad, vinCannyOutPortTP */

		pVin->recOut[0] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[1] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[2] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[3] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */

		pVin->recPort[0] = vinRecOutPort0; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[1] = vinRecOutPort1; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[2] = vinRecOutPort2; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[3] = vinRecOutPort3; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
	}/*}}}*/

#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL)
# if ( (BD_VID_PARALLEL_IN_FMT & VID_PARALLEL_TYPE_MASKBIT) == VID_PARALLEL_TYPE_VIN_BAYER)
	/* Define In/Out format */
	{/*{{{*/
		pVin->vidIn[0] = vinVidInPath_PARA0; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[1] = vinVidInPath_PARA1; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[2] = vinVidInPath_PARA0; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[3] = vinVidInPath_PARA1; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidIn[4] = vinVidInPath_PARA0; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#else
		pVin->vidIn[4] = vinVidInPath_PARA0; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#endif

		pVin->vidPort[0] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[1] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[2] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[3] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#else
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#endif

		pVin->vidResync[0] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[1] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[2] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[3] = vinVidResyncPath_ISP;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */

		pVin->svmIn[0] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[1] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[2] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[3] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */

		pVin->svmPort[0] = vinSvmPort0; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[1] = vinSvmPort1; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[2] = vinSvmPort2; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[3] = vinSvmPort3; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */


		pVin->quadIn[0] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[1] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[2] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[3] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */

		pVin->quadPort[0] = vinQuadPort0; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[1] = vinQuadPort1; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[2] = vinQuadPort2; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[3] = vinQuadPort3; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */

		pVin->vpuOut = vinVpuOutPath_Quad;  	/* vinVpuOutPath_VidIn, vinVpuOutPath_Pb, vinVpuOutPath_Quad, vinVpuOutPath_TP */
		pVin->vpuPort = vinVpuOutPortQuad; 	/* vinVpuOutInPath_VidIn: vinVpuOutPort0, vinVpuOutPort1, vinVpuOutPort2, vinVpuOutPort3 */ /* else: vinVpuOutPort4, vinVpuOutPortQuad, vinVpuOutPortTP */

		pVin->cannyOut = vinCannyOutPath_VidIn; /* vinCannyOutPath_VidIn, vinCannyOutPath_Pb, vinCannyOutPath_Quad, vinCannyOutPath_TP */
		pVin->cannyPort = vinCannyOutPort0; 	/* vinCannyOutInPath_VidIn: vinCannyOutPort0, vinCannyOutPort1, vinCannyOutPort2, vinCannyOutPort3 */ /* else: vinCannyOutPort4, vinCannyOutPortQuad, vinCannyOutPortTP */

		pVin->recOut[0] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[1] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[2] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[3] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */

		pVin->recPort[0] = vinRecOutPort0; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[1] = vinRecOutPort1; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[2] = vinRecOutPort2; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[3] = vinRecOutPort3; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
	}/*}}}*/

# elif ( (BD_VID_PARALLEL_IN_FMT & VID_PARALLEL_TYPE_MASKBIT) == VID_PARALLEL_TYPE_VIN_YUV)
	/* Define In/Out format */
	{/*{{{*/
		pVin->vidIn[0] = vinVidInPath_PARA0; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[1] = vinVidInPath_PARA1; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[2] = vinVidInPath_PARA0; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
		pVin->vidIn[3] = vinVidInPath_PARA0; 	/* vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidIn[4] = vinVidInPath_PARA0; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#else
		pVin->vidIn[4] = vinVidInPath_PARA0; 	/* PB: vinVidInPath_PARA0, vinVidInPath_PARA1, vinVidInPath_MIPI, vinVidInPath_PVI */
#endif

		pVin->vidPort[0] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[1] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[2] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
		pVin->vidPort[3] = vinVidInPort0; 	/* vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#if (VIDEO_IN_TYPE_PB_SUPPORT == 1)
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#else
		pVin->vidPort[4] = vinVidInPort0; 	/* PB: vinVidInPort0, vinVidInPort1, vinVidInPort2, vinVidInPort3 */
#endif

		pVin->vidResync[0] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[1] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[2] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */
		pVin->vidResync[3] = vinVidResyncPath_VIDEO;  /* vinVidResyncPath_VIDEO, vinVidResyncPath_ISP */

		pVin->svmIn[0] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[1] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[2] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */
		pVin->svmIn[3] = vinSvmInPath_VidIn; 	/* vinSvmInPath_VidIn, vinSvmInPath_Pb, vinSvmInPath_Quad, vinSvmInPath_TP */

		pVin->svmPort[0] = vinSvmPort0; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[1] = vinSvmPort1; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[2] = vinSvmPort2; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */
		pVin->svmPort[3] = vinSvmPort3; 	/* vinSvmInPath_VidIn: vinSvmPort0, vinSvmPort1, vinSvmPort2, vinSvmPort3 */ /* else: vinSvmPort4, vinSvmPortQuad, vinSvmPortTP */


		pVin->quadIn[0] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[1] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[2] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */
		pVin->quadIn[3] = vinQuadInPath_VidIn; 	/* vinQuadInPath_VidIn, vinQuadInPath_Pb, vinQuadInPath_TP */

		pVin->quadPort[0] = vinQuadPort0; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[1] = vinQuadPort1; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[2] = vinQuadPort2; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */
		pVin->quadPort[3] = vinQuadPort3; 	/* vinQuadInPath_VidIn: vinQuadPort0, vinQuadPort1, vinQuadPort2, vinQuadPort3 */ /* else: vinQuadPort4, vinQuadPortTP */

		pVin->vpuOut = vinVpuOutPath_Quad;  	/* vinVpuOutPath_VidIn, vinVpuOutPath_Pb, vinVpuOutPath_Quad, vinVpuOutPath_TP */
		pVin->vpuPort = vinVpuOutPortQuad; 	/* vinVpuOutInPath_VidIn: vinVpuOutPort0, vinVpuOutPort1, vinVpuOutPort2, vinVpuOutPort3 */ /* else: vinVpuOutPort4, vinVpuOutPortQuad, vinVpuOutPortTP */

		pVin->cannyOut = vinCannyOutPath_VidIn; /* vinCannyOutPath_VidIn, vinCannyOutPath_Pb, vinCannyOutPath_Quad, vinCannyOutPath_TP */
		pVin->cannyPort = vinCannyOutPort0; 	/* vinCannyOutInPath_VidIn: vinCannyOutPort0, vinCannyOutPort1, vinCannyOutPort2, vinCannyOutPort3 */ /* else: vinCannyOutPort4, vinCannyOutPortQuad, vinCannyOutPortTP */

		pVin->recOut[0] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[1] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[2] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */
		pVin->recOut[3] = vinRecOutPath_VidIn; /* vinRecOutPath_VidIn, vinRecOutPath_Pb, vinRecOutPath_Quad, vinRecOutPath_TP */

		pVin->recPort[0] = vinRecOutPort0; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[1] = vinRecOutPort1; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[2] = vinRecOutPort2; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
		pVin->recPort[3] = vinRecOutPort3; 	/* vinRecOutInPath_VidIn: vinRecOutPort0, vinRecOutPort1, vinRecOutPort2, vinRecOutPort3 */ /* else: vinRecOutPort4, vinRecOutPortQuad, vinRecOutPortTP */
	}/*}}}*/

# else
# error "Invalid Parallel type"
# endif
#endif

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if( (pVinUserReg == NULL) )
    {
        return;
    }

	/* 0xF0F0_0000: Input Format Selection for Parallel Port */
#if ( (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1) )
	const VIN_PAR_IN_FMT_CONFIG_U PI_BD_CONFIG_PAR_IN_FMT_port = { 
		.param.par0_data_type = ((BD_CAMERA_PB_IN_FMT & MIPI_VID_BIT_MASKBIT)>>MIPI_VID_BIT_BITSHIFT), /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par0_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par0_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par0_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par0_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par0_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
		.param.par1_data_type = ((BD_CAMERA_PB_IN_FMT & MIPI_VID_BIT_MASKBIT)>>MIPI_VID_BIT_BITSHIFT), /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par1_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par1_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par1_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par1_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par1_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
		.param.par2_data_type = ((BD_CAMERA_PB_IN_FMT & MIPI_VID_BIT_MASKBIT)>>MIPI_VID_BIT_BITSHIFT), /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par2_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par2_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par2_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par2_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par2_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
		.param.par3_data_type = ((BD_CAMERA_PB_IN_FMT & MIPI_VID_BIT_MASKBIT)>>MIPI_VID_BIT_BITSHIFT), /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par3_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par3_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par3_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par3_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par3_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
	};
#else
	const VIN_PAR_IN_FMT_CONFIG_U PI_BD_CONFIG_PAR_IN_FMT_port = { 
		.param.par0_data_type = PI_BD_CONFIG_VinRawType[0], /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par0_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par0_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par0_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par0_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par0_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
		.param.par1_data_type = PI_BD_CONFIG_VinRawType[1], /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par1_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par1_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par1_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par1_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par1_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
		.param.par2_data_type = PI_BD_CONFIG_VinRawType[2], /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par2_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par2_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par2_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par2_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par2_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
		.param.par3_data_type = PI_BD_CONFIG_VinRawType[3], /* vin_RAW6 = 0, vin_RAW7, vin_RAW8, vin_RAW10, vin_RAW12, vin_RAW14, vin_YUV8_2xRAW8, vin_2xRAW10 */
		.param.par3_bit_lsb = 0, /* 0 : MSB Mapping as {RAW10, 2'd0}, 1 : LSB Mapping {2'd0, RAW10} */
		.param.par3_bit_swap = 0, /* 0 : Normal, 1 : Bit Swap Enable BIT[11:0] -> BIT[0:11] */
		.param.par3_bt1120_md = 0, /* 0 : BT656 MD, 1 : BT1120 MD */
		.param.par3_sync_md = 0, /* 0 : External H/V Sync, 1 : Embedded Sync */
		.param.par3_byte_swap = 0, /* 0 : Bormal, 1 : Byte Swap */
	};
#endif //( (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1) )
	memcpy(&pVinUserReg->PI_BD_CONFIG_PAR_IN_FMT_port, &PI_BD_CONFIG_PAR_IN_FMT_port, sizeof(VIN_PAR_IN_FMT_CONFIG_U));

    // 0xF0F0_0004
	const VIN_PAR_IN_MUX_CONFIG_U PI_BD_CONFIG_PAR_IN_MUX_val = { 
		/* "MUX Input Selection for Parallel Input port 0
		 * 0 : Single Mode (No MUX Mode)
		 * 1 : 2 Ch MUX Mode (Only Valid for Embedded Sync Mode)
		 * 2 : 4 Ch MUX Mode (Only Valid for Embedded Sync Mode)" */
		.param.par0_in_mux = 0,
		/* "Sync Channel ID Position Control in Input MUX Mode
		 * 0 : Reserved for normal operation, Bit[1:0]" */
		.param.par0_id_md = 4,
		/* "MUX Input Selection for Parallel Input port 1
		 * 0 : Single Mode (No MUX Mode)
		 * 1 : 2 Ch MUX Mode (Only Valid for Embedded Sync Mode)
		 * 2 : 4 Ch MUX Mode (Only Valid for Embedded Sync Mode)" */
		.param.par1_in_mux = 0,
		/* "Sync Channel ID Position Control in Input MUX Mode for Paralle Input Port 1
		 * 0 : Reserved for normal operation" */
		.param.par1_id_md = 4,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PAR_IN_MUX_val, &PI_BD_CONFIG_PAR_IN_MUX_val, sizeof(VIN_PAR_IN_MUX_CONFIG_U));

    // 0xF0F0_0008
	const VIN_MIPI_IN_FMT_CONFIG_U PI_BD_CONFIG_MIPI_IN_FMT_val = { 
		/* "H Size of MIPI Input Channel
		 * 1280/4 : For HD Resolution of Data Type 3 ~ 7
		 * 1920/4 : For FHD Resolution of Data Type 3 ~ 7
		 * 1280/8 : For HD Resolution Data Type 0 ~ 2
		 * 1920/8 : For FHD Resolution Data Type 0 ~ 2" */
		.param.mipi_ch_hsize = (DEF_VIN_MIPI_HSIZE[BD_IN_RESOLUTION][PI_BD_CONFIG_VinRawTypeMIPI]&0x1FF),
		/* "MIPI Virtual Channel Size Selection
		 * 0~3 : Virtual Cahnnel 1 ~ 4 Ch" */
		.param.mipi_vc_size = 3,
		/* "MIPI Virtual Channel Mode Selection
		 * 0 : No VC, 1 Channel Input
		 * 1 : Virtual Cahnnel with Individual F/V/H Sync
		 * 2 : Common F/V + Individual H Mode
		 * 3 : Common H/V/F Mode : H Divide Mode" */
		.param.mipi_vc_md = 2,
		/* "MIPI DATA TYPE Selection
		 * 0 : RAW6, 1 : RAW7, 2 : RAW8, 3 : RAW10, 4 : RAW12, 5 : RAW14,
		 * 6 : YUV8 or 2xRAW8, 7 : 2xRAW10" */
		.param.mipi_data_type = PI_BD_CONFIG_VinRawTypeMIPI,
		/* "MIPI Input Asynchronouse Mode
		 * 0 : Inout clock ratio = 1:1 for RAW6/7/8/RAW10/12/14/YUV8/2xRAW8 Mode
		 * 1 : Inout clock ratio = 2:1 for 2xRAW10 Mode
		 * 2 : Inout clock ratio = 1:2 for RAW6/7/8 Mode (Not Used)" */
		.param.mipi_async_md = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_MIPI_IN_FMT_val, &PI_BD_CONFIG_MIPI_IN_FMT_val, sizeof(VIN_MIPI_IN_FMT_CONFIG_U));

    // 0xF0F0_000C
	const VIN_VID_IN_SEL_CONFIG_U PI_BD_CONFIG_VID_IN_SEL_val = { 
		/* "Video Input Selection for Channel 0
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for PAR 0 ~ 1 Input (For MUX Mode)
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for MIPI/PVI 0 ~ 3 Input" */
		.param.vid0_in_sel_portsel	= pVin->vidPort[0],
		/* "Video Input Selection for Channel 0
		 * Bit[3:2] : Path Selection
		 * 0 : Parallel 0, 1 : Parallel 1, 2 : MIPI, 3 : PVI */
		.param.vid0_in_sel_pathsel	= pVin->vidIn[0],
		/* "Video Input Selection for Channel 1
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for PAR 0 ~ 1 Input (For MUX Mode)
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for MIPI/PVI 0 ~ 3 Input" */
		.param.vid1_in_sel_portsel	= pVin->vidPort[1],
		/* "Video Input Selection for Channel 1
		 * Bit[3:2] : Path Selection
		 * 0 : Parallel 0, 1 : Parallel 1, 2 : MIPI, 3 : PVI */
		.param.vid1_in_sel_pathsel	= pVin->vidIn[1],
		/* "Video Input Selection for Channel 2
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for PAR 0 ~ 1 Input (For MUX Mode)
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for MIPI/PVI 0 ~ 3 Input" */
		.param.vid2_in_sel_portsel	= pVin->vidPort[2],
		/* "Video Input Selection for Channel 2
		 * Bit[3:2] : Path Selection
		 * 0 : Parallel 0, 1 : Parallel 1, 2 : MIPI, 3 : PVI */
		.param.vid2_in_sel_pathsel	= pVin->vidIn[2],
		/* "Video Input Selection for Channel 3
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for PAR 0 ~ 1 Input (For MUX Mode)
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for MIPI/PVI 0 ~ 3 Input" */
		.param.vid3_in_sel_portsel	= pVin->vidPort[3],
		/* "Video Input Selection for Channel 3
		 * Bit[3:2] : Path Selection
		 * 0 : Parallel 0, 1 : Parallel 1, 2 : MIPI, 3 : PVI */
		.param.vid3_in_sel_pathsel	= pVin->vidIn[3],
		/* "Video Input Selection for Channel 4 (For PB)
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for PAR 0 ~ 1 Input (For MUX Mode)
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for MIPI/PVI 0 ~ 3 Input" */
		.param.vid4_in_sel_portsel	= pVin->vidPort[4],
		/* "Video Input Selection for Channel 4 (For PB)
		 * Bit[3:2] : Path Selection
		 * 0 : Parallel 0, 1 : Parallel 1, 2 : MIPI, 3 : PVI */
		.param.vid4_in_sel_pathsel	= pVin->vidIn[4],
		/* "Async Enable for Parallel Input Path
		 * Bit[3:0] is refrenced for Port 3 ~ 0
		 * 0 : Disable,  1 : Enable" */
		.param.par_as_en	= 0xF,
		/* "Async Enable for MIPI Input Path
		 * Bit[3:0] is refrenced for Port 3 ~ 0
		 * 0 : Disable,  1 : Enable" */
		.param.mipi_as_en	= 0xF,
		/* "Async Enable for PVI Input Path
		 * Bit[3:0] is refrenced for Port 3 ~ 0
		 * 0 : Disable,  1 : Enable" */
		.param.pvi_as_en	= 0xF,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_VID_IN_SEL_val, &PI_BD_CONFIG_VID_IN_SEL_val, sizeof(VIN_VID_IN_SEL_CONFIG_U));

    // 0xF0F0_0010
	const VIN_ISP_IN_SEL_CONFIG_U PI_BD_CONFIG_ISP_IN_SEL_val = { 
		/* "Test Pattern Selection for ISP Test Pattern
		 * 0 : Black, 1 : Blue, 2 : Red, 3 : Magenta, 4 : Green, 5 : Cyan, 6 : Yellow, 7 : White,
		 * 8 : Color Bar, 9 : Hatch, 10 : Box, 11 ~  : Mixed Pattern" */
		.param.isp_tst_pt_sel	= 14,
		/* "Resolution Selection for ISP Test Pattern
		 * 0 : 720x480i@60, 1 : 720x576i@50 @13.5M,
		 * 2 : 960x480i@60, 3 : 960x576i@50 
		 * 4 : 720x480p@60, 5 : 720x576p@50 @27MHz,
		 * 6 : 960x480p@60, 7 : 960x576p@25 
		 * 8 : 1280x720p@60, 9 : 1280x720p@50,
		 * 10 : 1280x720p@30, 11 : 1280x720p@25, 
		 * 12 : 1280x960p@30, 13 : 1280x960p@25,
		 * 14 : 1920x1080p@30, 15 : 1920x1080p@25 " */
		.param.isp_tst_mode	= (BD_IN_RESOLUTION & 0xF),
		/* "Test Pattern Enable for ISP Input Path
		 * Bit[3:0] is refrenced for Port 3 ~ 0
		 * 0 : Disable,  1 : Enable" */
		.param.isp_tp_en	= 0,
		/* "ISP Input Async Compensation (Video Input to ISP Clock Asynchronous Interface)
		 * 0 : Disable, 1 : Enable" */
		.param.isp_in_async_en	= 1,
		/* "ISP Input Async Enable (Video Input to ISP Clock  Asynchronous Interface)
		 * 0 : Disable, 1 : Enable" */
		.param.isp_in_async_chg_en = 1,
		/* "ISP Output Async Compensation  (ISP Clock to Video Input Clock Asynchronous Interface)
		 * 0 : Disable, 1 : Enable" */
		.param.isp_out_async_en	= 1,
		/* "ISP Output Async Enable (ISP Clock to Video Input Clock Asynchronous Interface)
		 * 0 : Disable, 1 : Enable" */
		.param.isp_out_async_chg_en = 1,
		/* "Video Input Selection for Video Resync Input Path
		 * Bit[3:0] is refrenced for Port 3 ~ 0
		 * 0 : Video Input,  1 : ISP Output" */
		.param.rec_in_sel = ((pVin->vidResync[3]&1)<<3)|((pVin->vidResync[2]&1)<<2)|((pVin->vidResync[1]&1)<<1)|(pVin->vidResync[0]&1),
		/* "V Resolution Selection for ISP Test Pattern (Short Vector)
		 * 0 : Depends ISP_TST_MODE
		 * 1 : V Active = 16, V Blank = 30 Line fix" */
		.param.isp_tp_tst_sim1 = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_ISP_IN_SEL_val, &PI_BD_CONFIG_ISP_IN_SEL_val, sizeof(VIN_ISP_IN_SEL_CONFIG_U));

    // 0xF0F0_0014
	const VIN_SVM_IN_SEL_CONFIG_U PI_BD_CONFIG_SVM_IN_SEL_val = { 
		/* "SVM Input Selection for Channel 0
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[3:2] = 0
		 * 0 : Input 4 Port (PB), 1 : Quad Output, 2 : Test Pattern Selection for Bit[3:2] = 1" */
		.param.svm0_in_sel_portsel = pVin->svmPort[0],
		/* "SVM Input Selection for Channel 0
		 * Bit[3:2] : Path Selection
		 * 0 : Video Input, 1 : PB or Quad or Test Pattern, 2 : Video input (Reserved), 3 : ISP Output (Reserved) */
		.param.svm0_in_sel_pathsel = ((pVin->svmIn[0] != 0) ? 1:0),
		/* "SVM Input Selection for Channel 1
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[3:2] = 0
		 * 0 : Input 4 Port (PB), 1 : Quad Output, 2 : Test Pattern Selection for Bit[3:2] = 1" */
		.param.svm1_in_sel_portsel = pVin->svmPort[1],
		/* "SVM Input Selection for Channel 1
		 * Bit[3:2] : Path Selection
		 * 0 : Video Input, 1 : PB or Quad or Test Pattern, 2 : Video input (Reserved), 3 : ISP Output (Reserved) */
		.param.svm1_in_sel_pathsel = ((pVin->svmIn[1] != 0) ? 1:0),
		/* "SVM Input Selection for Channel 2
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[3:2] = 0
		 * 0 : Input 4 Port (PB), 1 : Quad Output, 2 : Test Pattern Selection for Bit[3:2] = 1" */
		.param.svm2_in_sel_portsel = pVin->svmPort[2],
		/* "SVM Input Selection for Channel 2
		 * Bit[3:2] : Path Selection
		 * 0 : Video Input, 1 : PB or Quad or Test Pattern, 2 : Video input (Reserved), 3 : ISP Output (Reserved) */
		.param.svm2_in_sel_pathsel = ((pVin->svmIn[2] != 0) ? 1:0),
		/* "SVM Input Selection for Channel 3
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[3:2] = 0
		 * 0 : Input 4 Port (PB), 1 : Quad Output, 2 : Test Pattern Selection for Bit[3:2] = 1" */
		.param.svm3_in_sel_portsel = pVin->svmPort[3],
		/* "SVM Input Selection for Channel 3
		 * Bit[3:2] : Path Selection
		 * 0 : Video Input, 1 : PB or Quad or Test Pattern, 2 : Video input (Reserved), 3 : ISP Output (Reserved) */
		.param.svm3_in_sel_pathsel = ((pVin->svmIn[3] != 0) ? 1:0),
		/* "The Field Polarity Inversion for SVM Input Video
		 * Bit[3:0] is referred for input 3 ~ 0 channel
		 * 0 : Normal
		 * 1 : Polarity Inverted" */
		.param.svm_fld_inv = 0,
		/* "V Sync Output Signal Selection for Interrupt
		 * SEL_VSYNC_MD[1] : Selection interrupt source
		 * 0 : V Sync from Video Input
		 * 1 : V Sync from ISP Output
		 * SEL_VSYNC_MD[0] : Selection Field Latch Timing
		 * 0 : Falling edge of V Sync (End of Video Active)
		 * 1 : Rising edge of V Sync (Start of Video Active)" */
		.param.sel_vsync_md = 0,
		/* "V Resolution Selection for SVM Test Pattern (Short Vector)
		 * 0 : Depends SVM_TP_MODE (0x003C Address)
		 * 1 : V Active = 16, V Blank = 8 Line fixed" */
		.param.svm_tp_tst_sim1 = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_SVM_IN_SEL_val, &PI_BD_CONFIG_SVM_IN_SEL_val, sizeof(VIN_SVM_IN_SEL_CONFIG_U));

    // 0xF0F0_0018
	const VIN_QUAD_CH_SEL_CONFIG_U PI_BD_CONFIG_QUAD_CH_SEL_val = { 
		/* "Quad Input Selection for Channel 0
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Record Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1 : Test Pattern for Bit[2] = 1" */
		.param.quad0_ch_sel_portsel = pVin->quadPort[0], //Front
		/* "Quad Input Selection for Channel 0
		 * [2] : Path Selection
		 * 0 : Video Input, 1 : Test Pattern Input */
		.param.quad0_ch_sel_pathsel = ((pVin->quadIn[0] != 0) ? 1:0),
		/* "Quad Input Selection for Channel 1
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Record Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1 : Test Pattern for Bit[2] = 1" */
		.param.quad1_ch_sel_portsel = pVin->quadPort[1], //Left
		/* "Quad Input Selection for Channel 1
		 * [2] : Path Selection
		 * 0 : Video Input, 1 : Test Pattern Input */
		.param.quad1_ch_sel_pathsel = ((pVin->quadIn[1] != 0) ? 1:0),
		/* "Quad Input Selection for Channel 2
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Record Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1 : Test Pattern for Bit[2] = 1" */
		.param.quad2_ch_sel_portsel = pVin->quadPort[3], //Rear
		/* "Quad Input Selection for Channel 2
		 * [2] : Path Selection
		 * 0 : Video Input, 1 : Test Pattern Input */
		.param.quad2_ch_sel_pathsel = ((pVin->quadIn[3] != 0) ? 1:0),
		/* "Quad Input Selection for Channel 3
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Record Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1 : Test Pattern for Bit[2] = 1" */
		.param.quad3_ch_sel_portsel = pVin->quadPort[2], //Right
		/* "Quad Input Selection for Channel 3
		 * [2] : Path Selection
		 * 0 : Video Input, 1 : Test Pattern Input */
		.param.quad3_ch_sel_pathsel = ((pVin->quadIn[2] != 0) ? 1:0),
		/* "The Field Polarity Inversion for Quad Input Video
		 * Bit[3:0] is referred for input 3 ~ 0 channel
		 * 0 : Normal
		 * 1 : Polarity Inverted" */
		.param.quad_fld_inv = 0,

	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_CH_SEL_val, &PI_BD_CONFIG_QUAD_CH_SEL_val, sizeof(VIN_QUAD_CH_SEL_CONFIG_U));

    // 0xF0F0_001C
	const VIN_REC_CH_SEL_CONFIG_U PI_BD_CONFIG_REC_CH_SEL_val = { 
		/* "Record Output Selection for Channel 0
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1  : Quad, 2 : Test Pattern for Bit[2] = 1" */
		.param.rec0_ch_sel_portsel = pVin->recPort[0],
		/* "Record Output Selection for Channel 0
		 * Bit[2] : Path Selection
		 * 0 : Video Input, 1 : Quad or Test Input */
		.param.rec0_ch_sel_pathsel = ((pVin->recOut[0] != 0) ? 1:0),
		/* "Record Output Selection for Channel 1
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1  : Quad, 2 : Test Pattern for Bit[2] = 1" */
		.param.rec1_ch_sel_portsel = pVin->recPort[1],
		/* "Record Output Selection for Channel 1
		 * Bit[2] : Path Selection
		 * 0 : Video Input, 1 : Quad or Test Input */
		.param.rec1_ch_sel_pathsel = ((pVin->recOut[1] != 0) ? 1:0),
		/* "Record Output Selection for Channel 2
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1  : Quad, 2 : Test Pattern for Bit[2] = 1" */
		.param.rec2_ch_sel_portsel = pVin->recPort[2],
		/* "Record Output Selection for Channel 2
		 * Bit[2] : Path Selection
		 * 0 : Video Input, 1 : Quad or Test Input */
		.param.rec2_ch_sel_pathsel = ((pVin->recOut[2] != 0) ? 1:0),
		/* "Record Output Selection for Channel 3
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[2] = 0
		 * 0 : Input 4 Port (PB), 1  : Quad, 2 : Test Pattern for Bit[2] = 1" */
		.param.rec3_ch_sel_portsel = pVin->recPort[3],
		/* "Record Output Selection for Channel 3
		 * Bit[2] : Path Selection
		 * 0 : Video Input, 1 : Quad or Test Input */
		.param.rec3_ch_sel_pathsel = ((pVin->recOut[3] != 0) ? 1:0),
		/* "Canney Edge Detector Output Selection
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[3:2] = 0
		 * 0 : Input 4 Port (PB), 1 : Quad Output, 2 : Test Pattern Selection for Bit[3:2] = 1" */
		.param.canny_ch_sel_portsel = pVin->cannyPort,
		/* "Canney Edge Detector Output Selection
		 * Bit[3:2] : Path Selection
		 * 0 : Video Input, 1 : PB or Quad or Test Pattern, 2 : Video input (Reserved), 3 : ISP Output (Reserved) */
		.param.canny_ch_sel_pathsel = ((pVin->cannyOut != 0) ? 1:0),
		/* "Field Polarity Selection for Canney Edge detector Field Output
		 * 0 : Bypass (Even Field High), 1 : Inversion (Odd Field High)" */
		.param.canny_fld_pol = 1,
		/* "Field Selection Enalbe of SD interlace mode for Canney Path
		 * 0 : Progressive Mode (No Field Selection)
		 * 1 : Interlace Mode (Vactive Field Selection by CANNEY_FLD_POL)" */
		.param.canny_fld_sel = 0,
		/* "VPU Output Selection
		 * Bit[1:0] : Port Selection
		 * 0 ~ 3 : Input 0 ~ 3 Port Selection for Bit[3:2] = 0
		 * 0 : Input 4 Port (PB), 1 : Quad Output, 2 : Test Pattern Selection for Bit[3:2] = 1" */
		.param.vpu_ch_sel_portsel = pVin->vpuPort,
		/* "VPU Output Selection
		 * Bit[3:2] : Path Selection
		 * 0 : Video Input, 1 : PB or Quad or Test Pattern, 2 : Video input (Reserved), 3 : ISP Output (Reserved) */
		.param.vpu_ch_sel_pathsel = ((pVin->vpuOut != 0) ? 1:0),
		/* "Field Polarity Selection for VPU Field Output
		 * 0 : Bypass(Even Field High), 1 : Inversion (Odd Field High)" */
		.param.vpu_fld_pol = 1,
		/* "Field Selection Enalbe of SD interlace mode for VPU Path
		 * 0 : Progressive Mode (No Field Selection)
		 * 1 : Interlace Mode (Vactive Field Selection by CANNEY_FLD_POL)" */
		.param.vpu_fld_sel = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_REC_CH_SEL_val, &PI_BD_CONFIG_REC_CH_SEL_val, sizeof(VIN_REC_CH_SEL_CONFIG_U));

    // 0xF0F0_0038
	const VIN_SVM_TP_SEL_CONFIG_U PI_BD_CONFIG_SVM_TP_SEL_val = { 
		/* "SVM Test Pattern Selection for Channel 0
		 * 0 : Black, 1 : Blue, 2 : Red, 3 : Magenta, 4 : Green, 5 : Cyan, 6 : Yellow, 7 : White,
		 * 8 : Color Bar, 9 : Hatch, 10 : Box, 11 ~  : Mixed Pattern" */
		.param.svm0_tp_sel = 0,
		/* "SVM Test Pattern Selection for Channel 1
		 * 0 : Black, 1 : Blue, 2 : Red, 3 : Magenta, 4 : Green, 5 : Cyan, 6 : Yellow, 7 : White,
		 * 8 : Color Bar, 9 : Hatch, 10 : Box, 11 ~  : Mixed Pattern" */
		.param.svm1_tp_sel = 0,
		/* "SVM Test Pattern Selection for Channel 2
		 * 0 : Black, 1 : Blue, 2 : Red, 3 : Magenta, 4 : Green, 5 : Cyan, 6 : Yellow, 7 : White,
		 * 8 : Color Bar, 9 : Hatch, 10 : Box, 11 ~  : Mixed Pattern" */
		.param.svm2_tp_sel = 0,
		/* "SVM Test Pattern Selection for Channel 3
		 * 0 : Black, 1 : Blue, 2 : Red, 3 : Magenta, 4 : Green, 5 : Cyan, 6 : Yellow, 7 : White,
		 * 8 : Color Bar, 9 : Hatch, 10 : Box, 11 ~  : Mixed Pattern" */
		.param.svm3_tp_sel = 0,
		/* "SVM Test Pattern H/V Resoution for Channel 0
		 * 0 : 720x480i@60, 1 : 720x576i@50 @13.5M,
		 * 2 : 960x480i@60, 3 : 960x576i@50 @18M 
		 * 4 : 720x480p@60, 5 : 720x576p@50 @27MHz,
		 * 6 : 960x480p@60, 7 : 960x576p@25 @36MHz 
		 * 8 : 1280x720p@60, 9 : 1280x720p@50,
		 * 10 : 1280x720p@30, 11 : 1280x720p@25, @74.25M 
		 * 12 : 1280x960p@30, 13 : 1280x960p@25, 
		 * 14 : 1920x1080p@30, 15 : 1920x1080p@25 @74.25M" */
		.param.svm0_tp_mode = (BD_IN_RESOLUTION & 0xF),
		/* "SVM Test Pattern H/V Resoution for Channel 1
		 * 0 : 720x480i@60, 1 : 720x576i@50 @13.5M,
		 * 2 : 960x480i@60, 3 : 960x576i@50 @18M 
		 * 4 : 720x480p@60, 5 : 720x576p@50 @27MHz,
		 * 6 : 960x480p@60, 7 : 960x576p@25 @36MHz 
		 * 8 : 1280x720p@60, 9 : 1280x720p@50,
		 * 10 : 1280x720p@30, 11 : 1280x720p@25, @74.25M 
		 * 12 : 1280x960p@30, 13 : 1280x960p@25, 
		 * 14 : 1920x1080p@30, 15 : 1920x1080p@25 @74.25M" */
		.param.svm1_tp_mode = (BD_IN_RESOLUTION & 0xF),
		/* "SVM Test Pattern H/V Resoution for Channel 2
		 * 0 : 720x480i@60, 1 : 720x576i@50 @13.5M,
		 * 2 : 960x480i@60, 3 : 960x576i@50 @18M 
		 * 4 : 720x480p@60, 5 : 720x576p@50 @27MHz,
		 * 6 : 960x480p@60, 7 : 960x576p@25 @36MHz 
		 * 8 : 1280x720p@60, 9 : 1280x720p@50,
		 * 10 : 1280x720p@30, 11 : 1280x720p@25, @74.25M 
		 * 12 : 1280x960p@30, 13 : 1280x960p@25, 
		 * 14 : 1920x1080p@30, 15 : 1920x1080p@25 @74.25M" */
		.param.svm2_tp_mode = (BD_IN_RESOLUTION & 0xF),
		/* "SVM Test Pattern H/V Resoution for Channel 3
		 * 0 : 720x480i@60, 1 : 720x576i@50 @13.5M,
		 * 2 : 960x480i@60, 3 : 960x576i@50 @18M 
		 * 4 : 720x480p@60, 5 : 720x576p@50 @27MHz,
		 * 6 : 960x480p@60, 7 : 960x576p@25 @36MHz 
		 * 8 : 1280x720p@60, 9 : 1280x720p@50,
		 * 10 : 1280x720p@30, 11 : 1280x720p@25, @74.25M 
		 * 12 : 1280x960p@30, 13 : 1280x960p@25, 
		 * 14 : 1920x1080p@30, 15 : 1920x1080p@25 @74.25M" */
		.param.svm3_tp_mode = (BD_IN_RESOLUTION & 0xF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_SVM_TP_SEL_val, &PI_BD_CONFIG_SVM_TP_SEL_val, sizeof(VIN_SVM_TP_SEL_CONFIG_U));

	const VIN_VID_IN_CTRL_CONFIG_U PI_BD_CONFIG_VID_IN_CTRL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0040
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.vid_rd_vsize = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.vid_rd_fld_pol = 0,
			/* "Video input buffer mode
			 * 0 : 4 Pixels Mode for Normal
			 * 1 : 8 Pixels Mode for MIPI Raw 6/7/8 with MIPI_ASYNC_MD = 0 " */
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
		    .param.vid_rd_raw_md = ( ( PI_BD_CONFIG_VinRawType[0] == vin_RAW8) ? 0x1:0),
#else
			.param.vid_rd_raw_md = 0,
#endif
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.vid_rd_hsize = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running during V Blank,
			 * 2 : H Sync Off during V Blank" */
			.param.vid_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.vid_rd_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.vid_rd_en = 1,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0050
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.vid_rd_vsize = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.vid_rd_fld_pol = 0,
			/* "Video input buffer mode
			 * 0 : 4 Pixels Mode for Normal
			 * 1 : 8 Pixels Mode for MIPI Raw 6/7/8 with MIPI_ASYNC_MD = 0 " */
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
		    .param.vid_rd_raw_md = ( ( PI_BD_CONFIG_VinRawType[1] == vin_RAW8) ? 0x1:0),
#else
			.param.vid_rd_raw_md = 0,
#endif
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.vid_rd_hsize = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running during V Blank,
			 * 2 : H Sync Off during V Blank" */
			.param.vid_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.vid_rd_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.vid_rd_en = 1,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0060
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.vid_rd_vsize = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.vid_rd_fld_pol = 0,
			/* "Video input buffer mode
			 * 0 : 4 Pixels Mode for Normal
			 * 1 : 8 Pixels Mode for MIPI Raw 6/7/8 with MIPI_ASYNC_MD = 0 " */
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
		    .param.vid_rd_raw_md = ( ( PI_BD_CONFIG_VinRawType[2] == vin_RAW8) ? 0x1:0),
#else
			.param.vid_rd_raw_md = 0,
#endif
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.vid_rd_hsize = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running during V Blank,
			 * 2 : H Sync Off during V Blank" */
			.param.vid_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.vid_rd_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.vid_rd_en = 1,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0070
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.vid_rd_vsize = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.vid_rd_fld_pol = 0,
			/* "Video input buffer mode
			 * 0 : 4 Pixels Mode for Normal
			 * 1 : 8 Pixels Mode for MIPI Raw 6/7/8 with MIPI_ASYNC_MD = 0 " */
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
		    .param.vid_rd_raw_md = ( ( PI_BD_CONFIG_VinRawType[3] == vin_RAW8) ? 0x1:0),
#else
			.param.vid_rd_raw_md = 0,
#endif
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.vid_rd_hsize = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running during V Blank,
			 * 2 : H Sync Off during V Blank" */
			.param.vid_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.vid_rd_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.vid_rd_en = 1,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_IN_CTRL_val, PI_BD_CONFIG_VID_IN_CTRL_val, sizeof(VIN_VID_IN_CTRL_CONFIG_U)*4);

	const VIN_VID_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_VID_IN_HTOTAL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0044
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.vid_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Video Input Data Swap Mode
			 * [5:4] : Swap Cb/Cr Swap
			 * 0 : Normal (For Bayer)
			 * 1 : 8Bits Cb/Cr Swap 
			 * 2 : 8 Bits Y0/Y1 Swap
			 * 3 : 10 Bits Cb/Cr Swap (Reserved)
			 * [3:2] : Swap Position Control {Bayer or Y/C Pair => Pixels Unit}
			 * 0 : Vin0/1/2/3
			 * 1 : Vin1/0/3/2
			 * 2 : Vin2/3/0/1
			 * 3 : Vin3/2/1/0
			 * [1:0] : Y/C Swap {Sub pixel unit}
			 * 0 : No Swap
			 * 1 : 8 Bits MSB/LSB Swap = {In[7:0], In[15:8]}
			 * 2 : 10 Bits MSB/LSB Swap= {In[9:0], In[19:10]}" */
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
			.param.vid_rd_swap = 0,
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
			.param.vid_rd_swap = ( (PI_BD_CONFIG_VinRawType[0] == vin_YUV8_2xRAW8) ? 0x10:0),
#else
			.param.vid_rd_swap = 0,
#endif

		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0054
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.vid_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Video Input Data Swap Mode
			 * [5:4] : Swap Cb/Cr Swap
			 * 0 : Normal (For Bayer)
			 * 1 : 8Bits Cb/Cr Swap 
			 * 2 : 8 Bits Y0/Y1 Swap
			 * 3 : 10 Bits Cb/Cr Swap (Reserved)
			 * [3:2] : Swap Position Control {Bayer or Y/C Pair => Pixels Unit}
			 * 0 : Vin0/1/2/3
			 * 1 : Vin1/0/3/2
			 * 2 : Vin2/3/0/1
			 * 3 : Vin3/2/1/0
			 * [1:0] : Y/C Swap {Sub pixel unit}
			 * 0 : No Swap
			 * 1 : 8 Bits MSB/LSB Swap = {In[7:0], In[15:8]}
			 * 2 : 10 Bits MSB/LSB Swap= {In[9:0], In[19:10]}" */
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
			.param.vid_rd_swap = 0,
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
			.param.vid_rd_swap = ( (PI_BD_CONFIG_VinRawType[1] == vin_YUV8_2xRAW8) ? 0x10:0),
#else
			.param.vid_rd_swap = 0,
#endif
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0064
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.vid_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Video Input Data Swap Mode
			 * [5:4] : Swap Cb/Cr Swap
			 * 0 : Normal (For Bayer)
			 * 1 : 8Bits Cb/Cr Swap 
			 * 2 : 8 Bits Y0/Y1 Swap
			 * 3 : 10 Bits Cb/Cr Swap (Reserved)
			 * [3:2] : Swap Position Control {Bayer or Y/C Pair => Pixels Unit}
			 * 0 : Vin0/1/2/3
			 * 1 : Vin1/0/3/2
			 * 2 : Vin2/3/0/1
			 * 3 : Vin3/2/1/0
			 * [1:0] : Y/C Swap {Sub pixel unit}
			 * 0 : No Swap
			 * 1 : 8 Bits MSB/LSB Swap = {In[7:0], In[15:8]}
			 * 2 : 10 Bits MSB/LSB Swap= {In[9:0], In[19:10]}" */
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
			.param.vid_rd_swap = 0,
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
			.param.vid_rd_swap = ( (PI_BD_CONFIG_VinRawType[1] == vin_YUV8_2xRAW8) ? 0x10:0),
#else
			.param.vid_rd_swap = 0,
#endif
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0074
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.vid_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Video Input Data Swap Mode
			 * [5:4] : Swap Cb/Cr Swap
			 * 0 : Normal (For Bayer)
			 * 1 : 8Bits Cb/Cr Swap 
			 * 2 : 8 Bits Y0/Y1 Swap
			 * 3 : 10 Bits Cb/Cr Swap (Reserved)
			 * [3:2] : Swap Position Control {Bayer or Y/C Pair => Pixels Unit}
			 * 0 : Vin0/1/2/3
			 * 1 : Vin1/0/3/2
			 * 2 : Vin2/3/0/1
			 * 3 : Vin3/2/1/0
			 * [1:0] : Y/C Swap {Sub pixel unit}
			 * 0 : No Swap
			 * 1 : 8 Bits MSB/LSB Swap = {In[7:0], In[15:8]}
			 * 2 : 10 Bits MSB/LSB Swap= {In[9:0], In[19:10]}" */
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
			.param.vid_rd_swap = 0,
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
			.param.vid_rd_swap = ( (PI_BD_CONFIG_VinRawType[3] == vin_YUV8_2xRAW8) ? 0x10:0),
#else
			.param.vid_rd_swap = 0,
#endif
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_IN_HTOTAL_val, PI_BD_CONFIG_VID_IN_HTOTAL_val, sizeof(VIN_VID_IN_HTOTAL_CONFIG_U)*4);

	const VIN_VID_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_VID_IN_VTOTAL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0048
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0058
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0068
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0078
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.vid_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_IN_VTOTAL_val, PI_BD_CONFIG_VID_IN_VTOTAL_val, sizeof(VIN_VID_IN_VTOTAL_CONFIG_U)*4);

	const VIN_VID_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_VID_IN_FLD_OS_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_004C
			/* "Selection of F transition Line for Odd Field
			 * VID_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * VID_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][0]&0x3FFF),
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_005C
			/* "Selection of F transition Line for Odd Field
			 * VID_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * VID_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][0]&0x3FFF),
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_006C
			/* "Selection of F transition Line for Odd Field
			 * VID_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * VID_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][0]&0x3FFF),
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_007C
			/* "Selection of F transition Line for Odd Field
			 * VID_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * VID_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.vid_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][0]&0x3FFF),
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_IN_FLD_OS_val, PI_BD_CONFIG_VID_IN_FLD_OS_val, sizeof(VIN_VID_IN_FLD_OS_CONFIG_U)*4);

	const VIN_REC_IN_CTRL_CONFIG_U PI_BD_CONFIG_REC_IN_CTRL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0080
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.resync_rd_vsize =
			    (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.resync_rd_fld_pol = 0,
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.resync_rd_hsize =
			    (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running
			 * 2 : H Sync Off" */
			.param.resync_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.resync_rd_int_md	= ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.resync_rd_en = 1,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0090
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.resync_rd_vsize =
			    (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.resync_rd_fld_pol = 0,
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.resync_rd_hsize =
			    (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running
			 * 2 : H Sync Off" */
			.param.resync_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.resync_rd_int_md	= ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.resync_rd_en = 1,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_00A0
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.resync_rd_vsize =
			    (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.resync_rd_fld_pol = 0,
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.resync_rd_hsize =
			    (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running
			 * 2 : H Sync Off" */
			.param.resync_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.resync_rd_int_md	= ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.resync_rd_en = 1,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_00B0
			/* "Video Read V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.resync_rd_vsize =
			    (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.resync_rd_fld_pol = 0,
			/* "Video Read H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.resync_rd_hsize =
			    (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_ResyncVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
			/* "Video Vblank Operation Mode
			 * 0 : Bypass,
			 * 1 : H Sync Free running
			 * 2 : H Sync Off" */
			.param.resync_rd_vblk_md = 1,
			/* "Video Input Format Selection
			 * 0 : Progressive Mode, 1 : Interlace Mode" */
			.param.resync_rd_int_md	= ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Input Enable
			 * 0 : Disable,  1 : Enable" */
			.param.resync_rd_en = 1,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_REC_IN_CTRL_val, PI_BD_CONFIG_REC_IN_CTRL_val, sizeof(VIN_REC_IN_CTRL_CONFIG_U)*4);

	const VIN_REC_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_REC_IN_HTOTAL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0084
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.resync_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x3FFF),
			/* " Brightness Selection for Input Channel
			 * 0xFF : Brightness High
			 * 0x80 : Default
			 * 0x00 : Brightness Low" */
			.param.resync_brt_ctrl = 0x80,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0094
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.resync_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x3FFF),
			/* " Brightness Selection for Input Channel
			 * 0xFF : Brightness High
			 * 0x80 : Default
			 * 0x00 : Brightness Low" */
			.param.resync_brt_ctrl = 0x80,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_00A4
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.resync_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x3FFF),
			/* " Brightness Selection for Input Channel
			 * 0xFF : Brightness High
			 * 0x80 : Default
			 * 0x00 : Brightness Low" */
			.param.resync_brt_ctrl = 0x80,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_00B4
			/* "Video Input Enable
			 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
			 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
			 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
			 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
			 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
			 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
			 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
			.param.resync_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x3FFF),
			/* " Brightness Selection for Input Channel
			 * 0xFF : Brightness High
			 * 0x80 : Default
			 * 0x00 : Brightness Low" */
			.param.resync_brt_ctrl = 0x80,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_REC_IN_HTOTAL_val, PI_BD_CONFIG_REC_IN_HTOTAL_val, sizeof(VIN_REC_IN_HTOTAL_CONFIG_U)*4);

	const VIN_REC_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_REC_IN_VTOTAL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0088
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0098
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_00A8
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_00B8
			/* "Video read V Total Size for Odd Field
			 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Video Read V Total Size for Even Field
			 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
			 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
			 * 750 : For HD 30p/25p/60p/50p,
			 * 825 : For 1280x800 HD 30p/25p/60p/50p
			 * 1000 : For 1280x960 HD 30p/25p
			 * 1125 : For FHD 30p/25p" */
			.param.resync_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_REC_IN_VTOTAL_val, PI_BD_CONFIG_REC_IN_VTOTAL_val, sizeof(VIN_REC_IN_VTOTAL_CONFIG_U)*4);

	const VIN_REC_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_REC_IN_FLD_OS_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_008C
			/* "Selection of F transition Line for Odd Field
			 * REC_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * REC_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_009C
			/* "Selection of F transition Line for Odd Field
			 * REC_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * REC_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_00AC
			/* "Selection of F transition Line for Odd Field
			 * REC_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * REC_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_00BC
			/* "Selection of F transition Line for Odd Field
			 * REC_RD_VSIZE + 2 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][0]&0x3FFF),
			/* "Selection of F transition Line for Even Field
			 * REC_RD_VSIZE + 3 for Interlace
			 * 0 for Progressive" */
			.param.resync_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION][1]&0x3FFF),
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_REC_IN_FLD_OS_val, PI_BD_CONFIG_REC_IN_FLD_OS_val, sizeof(VIN_REC_IN_FLD_OS_CONFIG_U)*4);

	const VIN_PAR_VSYNC_CTRL_CONFIG_U PI_BD_CONFIG_PAR_VSYNC_CTRL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_00C0
			/* "Parallel Input V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.par_vav_size = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Parallel Input VAV End Operation Mode
			 * 0 : with VAV Size Only, 1 : VAV Size + End of Vsync" */
			.param.par_vav_end_md = 1,
			/* "Parallel Input Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.par_fld_pol = 0,
			/* "Parallel Input VAV Active Line Start Offset
			 * 0 : Normal, No V Start Offset" */
			.param.par_vsv_os = 0,
			/* "Parallel Input Vsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_vsv_pol = 0,
			/* "External Vsync Mode Selection
			 * 0 : Bypass,  1 : Vstart Detection & Vend by Register" */
			.param.par_vsync_md = 0,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_00C8
			/* "Parallel Input V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.par_vav_size = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Parallel Input VAV End Operation Mode
			 * 0 : with VAV Size Only, 1 : VAV Size + End of Vsync" */
			.param.par_vav_end_md = 1,
			/* "Parallel Input Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.par_fld_pol = 0,
			/* "Parallel Input VAV Active Line Start Offset
			 * 0 : Normal, No V Start Offset" */
			.param.par_vsv_os = 0,
			/* "Parallel Input Vsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_vsv_pol = 0,
			/* "External Vsync Mode Selection
			 * 0 : Bypass,  1 : Vstart Detection & Vend by Register" */
			.param.par_vsync_md = 0,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_00D0
			/* "Parallel Input V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.par_vav_size = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Parallel Input VAV End Operation Mode
			 * 0 : with VAV Size Only, 1 : VAV Size + End of Vsync" */
			.param.par_vav_end_md = 1,
			/* "Parallel Input Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.par_fld_pol = 0,
			/* "Parallel Input VAV Active Line Start Offset
			 * 0 : Normal, No V Start Offset" */
			.param.par_vsv_os = 0,
			/* "Parallel Input Vsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_vsv_pol = 0,
			/* "External Vsync Mode Selection
			 * 0 : Bypass,  1 : Vstart Detection & Vend by Register" */
			.param.par_vsync_md = 0,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_00D8
			/* "Parallel Input V Active Size
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.par_vav_size = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x3FFF),
			/* "Parallel Input VAV End Operation Mode
			 * 0 : with VAV Size Only, 1 : VAV Size + End of Vsync" */
			.param.par_vav_end_md = 1,
			/* "Parallel Input Field Polarity Selection
			 * 0 : Even Field High, 1: Odd Field High" */
			.param.par_fld_pol = 0,
			/* "Parallel Input VAV Active Line Start Offset
			 * 0 : Normal, No V Start Offset" */
			.param.par_vsv_os = 0,
			/* "Parallel Input Vsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_vsv_pol = 0,
			/* "External Vsync Mode Selection
			 * 0 : Bypass,  1 : Vstart Detection & Vend by Register" */
			.param.par_vsync_md = 0,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_PAR_VSYNC_CTRL_val, PI_BD_CONFIG_PAR_VSYNC_CTRL_val, sizeof(VIN_PAR_VSYNC_CTRL_CONFIG_U)*4);

	const VIN_PAR_HSYNC_CTRL_CONFIG_U PI_BD_CONFIG_PAR_HSYNC_CTRL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_00C4
			/* "Parallel Input H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.par_hav_size = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
			/* "HAV Active Pixel Start Offset
			 * 0 : Normal, No H Start Offset" */
			.param.par_hav_os = 0,
			/* "Hsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_hav_pol = 0,
			/* "External Hsync Mode Selection
			 * 0 : Bypass,  1 : Hstart Detection" */
			.param.par_hsync_md = 0,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_00CC
			/* "Parallel Input H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.par_hav_size = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
			/* "HAV Active Pixel Start Offset
			 * 0 : Normal, No H Start Offset" */
			.param.par_hav_os = 0,
			/* "Hsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_hav_pol = 0,
			/* "External Hsync Mode Selection
			 * 0 : Bypass,  1 : Hstart Detection" */
			.param.par_hsync_md = 0,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_00D4
			/* "Parallel Input H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.par_hav_size = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
			/* "HAV Active Pixel Start Offset
			 * 0 : Normal, No H Start Offset" */
			.param.par_hav_os = 0,
			/* "Hsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_hav_pol = 0,
			/* "External Hsync Mode Selection
			 * 0 : Bypass,  1 : Hstart Detection" */
			.param.par_hsync_md = 0,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_00DC
			/* "Parallel Input H Active Size
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.par_hav_size = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
			/* "HAV Active Pixel Start Offset
			 * 0 : Normal, No H Start Offset" */
			.param.par_hav_os = 0,
			/* "Hsync Polarity
			 * 0 : Rising = V Start, 1 : Falling = V Start" */
			.param.par_hav_pol = 0,
			/* "External Hsync Mode Selection
			 * 0 : Bypass,  1 : Hstart Detection" */
			.param.par_hsync_md = 0,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_PAR_HSYNC_CTRL_val, PI_BD_CONFIG_PAR_HSYNC_CTRL_val, sizeof(VIN_PAR_HSYNC_CTRL_CONFIG_U)*4);

	const VIN_QUAD_CTRL_CONFIG_U PI_BD_CONFIG_QUAD_CTRL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0100
		        .param.quad_v_ofst = 1,
			/* "Quad Input Mode
			 * 0 : Progessive, 1 : Interlace" */
			.param.quad_intl_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Quad HLPF Mode Selection for C
			 * 0 : Bypass" */
			.param.quad_hlpf_md_c = 1,
			/* "Quad HLPF Mode Selection for Y
			 * 0 : Bypass,  1 : " */
			.param.quad_hlpf_md_y = 1,
			/* "Htotal size option of SVM Test Pattern for CH0 (Only 0xF0F00100)
			 * 0 : No Offset
			 * 1 ~ 7 : + 1 ~ 7 Pixels Offset
			 * 8 ~ 15 : -8 ~ -1 Pixels Offset
			 * (Example, TP_CHG = 0xE => HDp60 : 1650 - 2= 1648 Pixels)" */
			.param.svm_tp0_chg = 0,
			/* "Htotal size option of SVM Test Pattern for CH1 (Only 0xF0F00100)
			 * 0 : No Offset
			 * 1 ~ 7 : + 1 ~ 7 Pixels Offset
			 * 8 ~ 15 : -8 ~ -1 Pixels Offset
			 * (Example, TP_CHG = 0xE => HDp60 : 1650 - 2= 1648 Pixels)" */
			.param.svm_tp1_chg = 0,
			/* "Htotal size option of SVM Test Pattern for CH2 (Only 0xF0F00100)
			 * 0 : No Offset
			 * 1 ~ 7 : + 1 ~ 7 Pixels Offset
			 * 8 ~ 15 : -8 ~ -1 Pixels Offset
			 * (Example, TP_CHG = 0xE => HDp60 : 1650 - 2= 1648 Pixels)" */
			.param.svm_tp2_chg = 0,
			/* "Htotal size option of SVM Test Pattern for CH3 (Only 0xF0F00100)
			 * 0 : No Offset
			 * 1 ~ 7 : + 1 ~ 7 Pixels Offset
			 * 8 ~ 15 : -8 ~ -1 Pixels Offset
			 * (Example, TP_CHG = 0xE => HDp60 : 1650 - 2= 1648 Pixels)" */
			.param.svm_tp3_chg = 0,

		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0110
		        .param.quad_v_ofst = 1,
			/* "Quad Input Mode
			 * 0 : Progessive, 1 : Interlace" */
			.param.quad_intl_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Quad HLPF Mode Selection for C
			 * 0 : Bypass" */
			.param.quad_hlpf_md_c = 1,
			/* "Quad HLPF Mode Selection for Y
			 * 0 : Bypass,  1 : " */
			.param.quad_hlpf_md_y = 1,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0120
		        .param.quad_v_ofst = 1,
			/* "Quad Input Mode
			 * 0 : Progessive, 1 : Interlace" */
			.param.quad_intl_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Quad HLPF Mode Selection for C
			 * 0 : Bypass" */
			.param.quad_hlpf_md_c = 1,
			/* "Quad HLPF Mode Selection for Y
			 * 0 : Bypass,  1 : " */
			.param.quad_hlpf_md_y = 1,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0130
		        .param.quad_v_ofst = 1,
			/* "Quad Input Mode
			 * 0 : Progessive, 1 : Interlace" */
			.param.quad_intl_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Quad HLPF Mode Selection for C
			 * 0 : Bypass" */
			.param.quad_hlpf_md_c = 1,
			/* "Quad HLPF Mode Selection for Y
			 * 0 : Bypass,  1 : " */
			.param.quad_hlpf_md_y = 1,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_QUAD_CTRL_val, PI_BD_CONFIG_QUAD_CTRL_val, sizeof(VIN_QUAD_CTRL_CONFIG_U)*4);

#if 0
	const VIN_QUAD_HVSCL_CONFIG_U PI_BD_CONFIG_QUAD_HVSCL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0104
			/* "Quad Scaling Ratio Selection for Horizontal Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_hscl = 0x8000,
			/* "Quad Scaling Ratio Selection for Vertical Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_vscl = 0x8000,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0114
			/* "Quad Scaling Ratio Selection for Horizontal Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_hscl = 0x8000,
			/* "Quad Scaling Ratio Selection for Vertical Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_vscl = 0x8000,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0124
			/* "Quad Scaling Ratio Selection for Horizontal Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_hscl = 0x8000,
			/* "Quad Scaling Ratio Selection for Vertical Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_vscl = 0x8000,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0134
			/* "Quad Scaling Ratio Selection for Horizontal Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_hscl = 0x8000,
			/* "Quad Scaling Ratio Selection for Vertical Direction
			 * 0xFFFF : Bypass,  0x8000 : 1/2 Scale, 0x4000 : 1/4 Scale" */
			.param.quad_vscl = 0x8000,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_QUAD_HVSCL_val, PI_BD_CONFIG_QUAD_HVSCL_val, sizeof(VIN_QUAD_HVSCL_CONFIG_U)*4);
#endif

	const VIN_QUAD_HVDEL_CONFIG_U PI_BD_CONFIG_QUAD_HVDEL_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0108
			/* "Quad Start Selection for Horizontal Direction
			 * 0x0 : No Crop" */
			.param.quad_hdelay = 0,
			/* "Quad Start Selection for Vertical Direction
			 * 0x0 : No Crop" */
			.param.quad_vdelay = 0,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0118
			/* "Quad Start Selection for Horizontal Direction
			 * 0x0 : No Crop" */
			.param.quad_hdelay = 0,
			/* "Quad Start Selection for Vertical Direction
			 * 0x0 : No Crop" */
			.param.quad_vdelay = 0,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0128
			/* "Quad Start Selection for Horizontal Direction
			 * 0x0 : No Crop" */
			.param.quad_hdelay = 0,
			/* "Quad Start Selection for Vertical Direction
			 * 0x0 : No Crop" */
			.param.quad_vdelay = 0,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0138
			/* "Quad Start Selection for Horizontal Direction
			 * 0x0 : No Crop" */
			.param.quad_hdelay = 0,
			/* "Quad Start Selection for Vertical Direction
			 * 0x0 : No Crop" */
			.param.quad_vdelay = 0,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_QUAD_HVDEL_val, PI_BD_CONFIG_QUAD_HVDEL_val, sizeof(VIN_QUAD_HVDEL_CONFIG_U)*4);

	const VIN_QUAD_HVACT_CONFIG_U PI_BD_CONFIG_QUAD_HVACT_val[4] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_010C
			/* "Quad Active Size for Horizontal Direction
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.quad_hactive = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
			/* "Quad Active Size for Vertical Direction
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.quad_vactive = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[0]][BD_IN_RESOLUTION]&0x7FF),
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_011C
			/* "Quad Active Size for Horizontal Direction
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.quad_hactive = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
			/* "Quad Active Size for Vertical Direction
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.quad_vactive = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[1]][BD_IN_RESOLUTION]&0x7FF),
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_012C
			/* "Quad Active Size for Horizontal Direction
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.quad_hactive = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
			/* "Quad Active Size for Vertical Direction
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.quad_vactive = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[2]][BD_IN_RESOLUTION]&0x7FF),
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_013C
			/* "Quad Active Size for Horizontal Direction
			 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
			.param.quad_hactive = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
			/* "Quad Active Size for Vertical Direction
			 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
			 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
			 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
			 * 1080 : For FHD" */
			.param.quad_vactive = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_VinVideoType[3]][BD_IN_RESOLUTION]&0x7FF),
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_QUAD_HVACT_val, PI_BD_CONFIG_QUAD_HVACT_val, sizeof(VIN_QUAD_HVACT_CONFIG_U)*4);

#if 0
    // 0xF0F0_0150
	const VIN_QUAD_WIN_SIZE_CONFIG_U PI_BD_CONFIG_QUAD_WIN_SIZE_val = { 
			/* "Quad Window Vsize (1 Lines Domain) = Active V Size / 2
			 * 540 for FHD, 360 for HD" */
			.param.quad_win_vsize = 360,
			/* "Quad Window Hsize (8 Pixels Domain) = Active H Size / 2
			 * 960/8 = 120 for FHD, 640/8 = 80 for HD" */
			.param.quad_win_hsize = 80,
			/* "Quad Frame Memory Size
			 * 0 : 1 Page, ~ 3 : 4 Page" */
			.param.quad_wr_page = 3,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_WIN_SIZE_val, &PI_BD_CONFIG_QUAD_WIN_SIZE_val, sizeof(VIN_QUAD_WIN_SIZE_CONFIG_U));
#endif

    // 0xF0F0_0154
	const VIN_QUAD_WIN_WR_EN_CONFIG_U PI_BD_CONFIG_QUAD_WIN_WR_EN_val = { 
		/* "Quad DDR Write Dummy Period
		 * 0 : For Normal Operation" */
		.param.quad_ddr_wr_delay = 0,
		/* "Quad DDR Read Dummy Period
		 * 0 : For Normal Operation" */
		.param.quad_ddr_rd_delay = 0,
		/* "Quad Write Interlace to Progreeive Mode
		 * 0 : Normal, 1 : Even Field Write Skip(Odd Only Write)" */
		.param.quad_wr_i2p_en = 0,
		/* "Quad Write Skip Mode, [23:20] stand for 4 Quad Input Write
		 * 0 : Normal, 1 : Write Frame by Frame (1/2 Rate Write)
		 * => Skip Mode use for HD 60p to HD/FHD 30p Conversion" */
		.param.quad_wr_period = 0,
		/* "Quad Write Mode, [27:24] stand for 4 Quad Input Write
		 * 0 : Progressive, 1 : Interlace Mode" */
		.param.quad_wr_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:0xF),
		/* "Quad Window Write Enable, [31:28] stand for 4 Quad Input
		 * 0 : Disable, 1 : Enable" */
		.param.quad_wr_en = 0x0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_WIN_WR_EN_val, &PI_BD_CONFIG_QUAD_WIN_WR_EN_val, sizeof(VIN_QUAD_WIN_WR_EN_CONFIG_U));

    // 0xF0F0_0158
	const VIN_QUAD_WIN_RD_EN_CONFIG_U PI_BD_CONFIG_QUAD_WIN_RD_EN_val = { 
		/* Selection of Quad Window Boundary Color */
		.param.quad_bnd_col = 3,
		/* Selection of Quad Window Background Color */
		.param.quad_bgd_col = 0,
		/* Selection of Quad Window Blank Color */
		.param.quad_blk_col = 0,
		/* "Quad Window Boudary Width Selection
		 * 0 : Narrow, 1 : Wide" */
		.param.quad_win_bnd_size = 0,
		/* "Quad Window Boundary Enable
		 * [3:0] is referenced for Win 3 ~ 0
		 * 0 : Disable, 1 : Enable" */
		.param.quad_win_bnd_en = 0,
		/* "Quad Window Blank Enable
		 * [3:0] is referenced for Win 3 ~ 0
		 * 0 : Normal, 1 : Window Blank Enable" */
		.param.quad_win_blk_en = 0,
		/* "Quad Window Read Enable
		 * [3:0] is referenced for Win 3 ~ 0
		 * 0 : Disable, 1 : Enable" */
		.param.quad_win_en = 0xF,
		/* "Quad Read P2I Operation Mode (Field Inversion effect)
		 * 0 : Odd 1st, 1 : Even 1st" */
		.param.quad_rd_p2i_fld = 0,
		/* "Quad Read P2I Enable for Interlace output mode
		 * 0 : Normal, 1 : Odd/Even Line Read in Interlace Mode (Progressive to Interlace Read Option)" */
		.param.quad_rd_p2i_en = 0,
		/* "Quad Read Mode
		 * 0 : Progressive, 1 : Interlace Mode" */
		.param.quad_rd_int_md = ((BD_QUAD_OUT_RESOLUTION>=vres_720x480p60)?0:1),
		/* "Quad Read Enable
		 * 0 : Disable, 1 : Enable" */
		.param.quad_rd_en = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_WIN_RD_EN_val, &PI_BD_CONFIG_QUAD_WIN_RD_EN_val, sizeof(VIN_QUAD_WIN_RD_EN_CONFIG_U));

    // 0xF0F0_015C
	const VIN_QUAD_WIN_HVOS_CONFIG_U PI_BD_CONFIG_QUAD_WIN_HVOS_val = { 
		/* Quad Window V Start Offset */
		.param.quad_win_vos = 0,
		/* Quad Window H Start Offset */
		.param.quad_win_hos = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_WIN_HVOS_val, &PI_BD_CONFIG_QUAD_WIN_HVOS_val, sizeof(VIN_QUAD_WIN_HVOS_CONFIG_U));

    // 0xF0F0_0160
	const VIN_QUAD_RD_HVSIZE_CONFIG_U PI_BD_CONFIG_QUAD_RD_HVSIZE_val = { 
		/* "Quad Output V Active Size
		 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
		 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
		 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
		 * 1080 : For FHD" */
		.param.quad_rd_vsize = (DEF_VIN_VACTIVE_SIZE[0][BD_QUAD_OUT_RESOLUTION]&0x7FF),
		/* "Quad Output H Active Size
		 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
		.param.quad_rd_hsize = (DEF_VIN_HACTIVE_SIZE[0][BD_QUAD_OUT_RESOLUTION]&0x7FF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_RD_HVSIZE_val, &PI_BD_CONFIG_QUAD_RD_HVSIZE_val, sizeof(VIN_QUAD_RD_HVSIZE_CONFIG_U));

    // 0xF0F0_0164
	const VIN_QUAD_RD_HTOTAL_CONFIG_U PI_BD_CONFIG_QUAD_RD_HTOTAL_val = { 
		/* "Video Input Enable
		 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
		 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
		 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
		 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
		 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
		 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
		 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
		.param.quad_rd_htotal = (DEF_VIN_QUAD_RD_HTOTAL_SIZE[0][BD_QUAD_OUT_RESOLUTION]),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_RD_HTOTAL_val, &PI_BD_CONFIG_QUAD_RD_HTOTAL_val, sizeof(VIN_QUAD_RD_HTOTAL_CONFIG_U));

    // 0xF0F0_0168
	const VIN_QUAD_RD_VTOTAL_CONFIG_U PI_BD_CONFIG_QUAD_RD_VTOTAL_val = { 
		/* "Video read V Total Size for Odd Field
		 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
		 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
		 * 750 : For HD 30p/25p/60p/50p,
		 * 825 : For 1280x800 HD 30p/25p/60p/50p
		 * 1000 : For 1280x960 HD 30p/25p
		 * 1125 : For FHD 30p/25p" */
		.param.quad_rd_vtotal_odd = (DEF_VIN_QUAD_RD_ODD_EVEN_FIELD_VTOTAL_SIZE[0][BD_QUAD_OUT_RESOLUTION][0]&0x3FFF),
		/* "Video Read V Total Size for Even Field
		 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
		 * same with QUAD_RD_VTOTAL_ODD for Progressive" */
		.param.quad_rd_vtotal_even = (DEF_VIN_QUAD_RD_ODD_EVEN_FIELD_VTOTAL_SIZE[0][BD_QUAD_OUT_RESOLUTION][1]&0x3FFF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_RD_VTOTAL_val, &PI_BD_CONFIG_QUAD_RD_VTOTAL_val, sizeof(VIN_QUAD_RD_VTOTAL_CONFIG_U));

    // 0xF0F0_016C
	const VIN_QUAD_RD_FLD_OS_CONFIG_U PI_BD_CONFIG_QUAD_RD_FLD_OS_val = { 
		/* "Selection of F transition Line for Odd Field
		 * QUAD_RD_VSIZE + 2 for Interlace
		 * 0 for Progressive" */
		.param.quad_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[0][BD_QUAD_OUT_RESOLUTION][0]&0x3FFF),
		/* "Selection of F transition Line for Even Field
		 * QUAD_RD_VSIZE + 3 for Interlace
		 * 0 for Progressive" */
		.param.quad_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[0][BD_QUAD_OUT_RESOLUTION][1]&0x3FFF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_RD_FLD_OS_val, &PI_BD_CONFIG_QUAD_RD_FLD_OS_val, sizeof(VIN_QUAD_RD_FLD_OS_CONFIG_U));

    // 0xF0F0_0170
	const VIN_QUAD_OUT_FMT_CONFIG_U PI_BD_CONFIG_QUAD_OUT_FMT_val = { 
		/* "Quad Output Clock0 Mode Selection
		 * Bit[1] : Frequency Selection
		 * 1 : 1x Clock for 16 Bits, 0 : 2x Clock for 8 Bits
		 * Bit[0] : Phase Selection
		 * 0 : Bypass, 1 : Inversion" */
		.param.quad_out_clk0_md = ( (((BD_RO_OUT_FMT&VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT&VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT)) ? 0<<1:1<<1) | 0,
		/* "Quad Output Clock1 Mode Selection
		 * Bit[1] : Frequency Selection
		 * 1 : 1x Clock for 16 Bits, 0 : 2x Clock for 8 Bits
		 * Bit[0] : Phase Selection
		 * 0 : Bypass, 1 : Inversion" */
		.param.quad_out_clk1_md = ( (((BD_RO_OUT_FMT&VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT&VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT)) ? 0<<1:1<<1) | 0,
		/* "Quad Output Sync Code Type
		 * 1 : BT656 Type, 0 : BT1120 Type" */
		.param.quad_out_bt656_md = ( (((BD_QUAD_OUT_FMT&VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_QUAD_OUT_FMT&VID_RESOL_MASKBIT) == VID_RESOL_SD960H)) ? 1:0),
		/* "Quad Output Field Inversion for Quad Digital Output
		 * 0 : Normal, 1 : Polarity Inversion" */
		.param.quad_out_fld_inv	= 0,
		/* "Output Y/C Inversion Enable
		 * 0 : Normal, 1 : Y/C Inversion
		 * For 16 Bits Mode
		 * 0 : QUAD_DATA1 (Y Data), QUAD_DATA0 (C Data),
		 * 1 : QUAD_DATA1 (C Data), QUAD_DATA0 (Y Data),
		 * For 8 Bits Mode
		 * 0 : Cb/Y/Cr/Y Sequence,1 : Y/Cb/Y/Cr Sequence" */
		.param.quad_out_yc_inv = 0,
		/* "Active Data Limitation Enable
		 * 0 : High Limit as 0xFF, Low Limit as 0x00 for Active Data
		 * 1 : High Limit as 0xF0, Low Limit as 0x10 for Active Data" */
		.param.quad_out_lim_en = 0,
		/* "Quad Output 16BIT Mode
		 * 0 : 8 Bits Mode, 1 : 16 Bits Mode" */
		.param.quad_out_16bit_md = ( (((BD_RO_OUT_FMT&VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT&VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT)) ? 0:1),

		/* "Quad Sync Code Information
		 * 0 : SAV/EAV Protect Code Bit[7] = 1 (Normal),
		 * 1 : SAV/EAV Protect Code Bit[7] = 0 (No Video)" */
		.param.quad_out_validb = 0,
		/* "Quad to VPU Field Selection Mode
		 * 0 : Progressive Mode (No Field Selection)
		 * 1 : Interlace Mode (Vactive Field Selection by QUAD_VPU_FLD_POL)" */
		.param.quad_vpu_fld_sel	 = 0,
		/* "Quad to VPU Field Polarity Inversion
		 * 0 : Bypass, 1 : Inversion" */
		.param.quad_vpu_fld_pol	= 0,
		/* "Quad Output Async Clock Compensation Enable (When REC Port use for Quad Output)
		 * 0 : Disable, 1 : Enable" */
		.param.quad_out_async_chg_en = 1,
		/* "Quad Output Async Enable (When REC Port use for Quad Output)
		 * 0 : Disable, 1 : Enable" */
		.param.quad_out_async_en = 1,
		/* "Quad to VPU/REC/CANNY Async Clock Compensation Enable (Reserved 0, only valid when same clock rate)
		 * 0 : Disable, 1 : Enable" */
		.param.quad_rec_async_chg_en = 1,
		/* "Quad to VPU/REC/CANNY Async Enable (Reserved 0, Only reasonable when same clock rate)
		 * 0 : Disable, 1 : Enable" */
		.param.quad_rec_async_en = 1,
		/* "Quad Output Sync Code Type  for Quad to PVI Tx Output
		 * 1 : BT656 Type, 0 : BT1120 Type" */
		.param.quad_pvi_out_bt656_md = ( (((BD_QUAD_OUT_FMT&VID_RESOL_MASKBIT) == VID_RESOL_SD720H) || ((BD_QUAD_OUT_FMT&VID_RESOL_MASKBIT) == VID_RESOL_SD960H)) ? 1:0),
		/* "Quad Output Field Inversion for  for Quad to PVI Tx Output
		 * 0 : Normal, 1 : Polarity Inversion" */
		.param.quad_pvi_out_fld_inv = 0,
		/* "Output Y/C Inversion Enable  for Quad to PVI Tx Output
		 * 0 : Normal, 1 : Y/C Inversion
		 * For 16 Bits Mode
		 * 0 : QUAD_DATA1 (Y Data), QUAD_DATA0 (C Data),
		 * 1 : QUAD_DATA1 (C Data), QUAD_DATA0 (Y Data),
		 * For 8 Bits Mode
		 * 0 : Cb/Y/Cr/Y Sequence,1 : Y/Cb/Y/Cr Sequence" */
		.param.quad_pvi_out_yc_inv = 0,
		/* "Active Data Limitation Enable for Quad to PVI Tx Output
		 * 0 : High Limit as 0xFF, Low Limit as 0x00 for Active Data
		 * 1 : High Limit as 0xF0, Low Limit as 0x10 for Active Data" */
		.param.quad_pvi_out_lim_en = 0,
		/* "Quad to PVI Async Clock Compensation Enable
		 * 0 : Disable, 1 : Enable" */
		.param.quad_pvi_async_chg_en = 1,
		/* "Quad to PVI Async Enable
		 * 0 : Disable, 1 : Enable" */
		.param.quad_pvi_async_en = 1,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_OUT_FMT_val, &PI_BD_CONFIG_QUAD_OUT_FMT_val, sizeof(VIN_QUAD_OUT_FMT_CONFIG_U));

    // 0xF0F0_0174
	const VIN_QUAD_FULL_MD_CONFIG_U PI_BD_CONFIG_QUAD_FULL_MD_val = { 
		/* "Write Page information for Channel 0
		 * [3:2] : For Even Field
		 * [1:0] : For Odd Field or Progressive Mode" */
		.param.quad0_wr_page = 0,  
		/* "Write Page information for Channel 1
		 * [3:2] : For Even Field
		 * [1:0] : For Odd Field or Progressive Mode" */
		.param.quad1_wr_page = 0,  
		/* "Write Page information for Channel 2
		 * [3:2] : For Even Field
		 * [1:0] : For Odd Field or Progressive Mode" */
		.param.quad2_wr_page = 0,  
		/* "Write Page information for Channel 3
		 * [3:2] : For Even Field
		 * [1:0] : For Odd Field or Progressive Mode" */
		.param.quad3_wr_page = 0,  
		/* Strobe & Capture Sequence
		 * 1. Set to ""1"" for QUAD_SINGLE_MD
		 * 2. Set to ""1"" for QUAD_WR_STROBE
		 * 3. Wait strobe is detected (QUAD_WR_STROBE will be gone to 0)
		 * 4. Capture Done when quad_wr_page is updated and (quad_wr_busy_det = 0)
		 *  -> QUAD_WR_STROBE = 0 and wr_busy_det = 0 : Capture is done
		 * 5. DMA for captured Video @ MEMLOC + Capture Size x Page # (Page # = 0 ~ 3)
		 *     Capture Size for HD Quad : 1280 / 2 x 720 / 2 x 16 Bits
		 *     Capture Size for FHD Quad : 1920 / 2 x 1080 / 2 x 16 Bits
		 *     Capture Size for HD Full : 1920 x 1080 x 16 Bits" */
		/* "Quad Write Strobe Enable
		 * [3:0] is stand for Quad Ch 3 ~ Quad Ch 0
		 * 0 : Strobe Detected
		 * 1 : Strobe Detect (Auto Clear when strobe start)" */
		.param.quad_wr_strobe = 0,
		/* "Quad Write Start Status
		 * [3:0] is stand for Quad Ch 3 ~ Ch 0
		 * 0 : Write / Capture Done
		 * 1 : Write /Capture Start & Busy" */
		.param.quad_wr_busy_det = 0,
		/* "Selection for Quad Operation Mode
		 * [3:0] is stand for Ch 3 ~ Ch0
		 *  0 : Live Mode, 1 : Strobe Mode by WR_STROBE Control */
		.param.quad_single_md = 0,
		/* "Selection for Quad / 1Ch Full Mode
		 *  0 : 4 Ch Quad Mode,  1 : 1 Ch Full Mode" */
		.param.quad_full_md = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_FULL_MD_val, &PI_BD_CONFIG_QUAD_FULL_MD_val, sizeof(VIN_QUAD_FULL_MD_CONFIG_U));

    // 0xF0F0_0180
	const VIN_REC_FMT_CTRL_CONFIG_U PI_BD_CONFIG_REC_FMT_CTRL_val = { 
		/* "Record Output Channel ID Enable
		 * 0 : Disable, 1 : Enable" */
		.param.rec_chid_en = 1,
		/* "Record Output Async Compensation
		 * 0 : Disable, 1 : Enable" */
		.param.rec_async_chg_en	= 1,
		/* "Record Output Async Enable
		 * 0 : Disable, 1 : Enable" */
		.param.rec_async_en = 1,
		/* "BT656/BT1120 Data Value Limit 8bit 1 ~ 254:1, 16~240:0 */
		.param.bt1120_lim_rec = 0,
		/* CH Multiplexing Mode(0 : 1CH Mode, 1 : 2CH Mode, 2 : 4CH Mode) */
		.param.ch_mux_md = (BD_RO_OUT_MUXCH_CNT/2),
		/* "Video Data Data Rate Control
		 * (0 : 74.25MHz for 16bit/148.5MHz for 8bit, 1 : 37.125MHz for 16bit/74.25MHz for 8bit,
		 * 2 : 18MHz for 16bit/36MHz for 8bit, 3 : 13.5Mhz for 16bit/27MHz for 8bit)" */
		.param.outfmt_rate = (DEF_VIN_RECOUT_DATARATE[BD_RO_OUT_RESOLUTION]&0x3),
		/* BT656/BT1120 YC change:1 */
		.param.outfmt_yc_inv = 0,
		/* Record Format HD, BT656 Standard:1, BT1120 Standard:0 */
#if	( ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT) )
		.param.outfmt_bt656 = 1,
#else
		.param.outfmt_bt656 = 0,
#endif
		/* Record Format HD Bit Width 16bit:1, 8bit:0 */
#if	( ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT) )
		.param.outfmt_16bit = 0,
#else
		.param.outfmt_16bit = 1,
#endif
		/* SD Field Polarity Control */
		.param.outfmt_fld_pol = 0,
		/* SD 50/60Hz (0 : 60Hz, 1 : 50Hz) */
		.param.sd_pal_ntb = (((BD_RO_OUT_RESOLUTION == vres_720x576i50)||(BD_RO_OUT_RESOLUTION == vres_960x576i50))? 1:0),
		/* "SD 960H Mode Selection
		 * 0 : Not 960H Mode, 1 : SD 960H Mode"*/
		.param.sd_960h_md = (((BD_RO_OUT_RESOLUTION == vres_960x480i60)||(BD_RO_OUT_RESOLUTION == vres_960x576i50))? 1:0),
		/* "Selection for P2I Mode
		 * [31:28] is refered Record Path 0 ~ 3
		 * 0 : Disable,  1 : Enable" */
		.param.sd_p2i_en = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_REC_FMT_CTRL_val, &PI_BD_CONFIG_REC_FMT_CTRL_val, sizeof(VIN_REC_FMT_CTRL_CONFIG_U));

    // 0xF0F0_0184
	const VIN_REC0_CH_SEL_CONFIG_U PI_BD_CONFIG_REC0_CH_SEL_val = { 
#if	( ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT) )
		/* "4CH Multiplexing 1st Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_a_0 = 0,
		/* "4CH Multiplexing 2nd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_b_0 = 2,
		/* "4CH Multiplexing 3rd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_c_0 = 4,
		/* "4CH Multiplexing 4th Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_d_0 = 6,
#else
		/* "4CH Multiplexing 1st Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_a_0 = 0,
		/* "4CH Multiplexing 2nd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_b_0 = 2,
		/* "4CH Multiplexing 3rd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_c_0 = 4,
		/* "4CH Multiplexing 4th Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_d_0 = 6,
#endif
		/* Record Channel ID Number Selection for CH0 */
		.param.chid_num_0 = 0,
		/* Record Channel ID Number Selection for CH1 */
		.param.chid_num_1 = 1,
		/* Record Channel ID Number Selection for CH2 */
		.param.chid_num_2 = 2,
		/* Record Channel ID Number Selection for CH3 */
		.param.chid_num_3 = 3,
		/* "REC_OUT_FMT Clock Phase Control
		 * [5] : 0 -> Multi-phase Clock Output Mode , 1 -> No Multi-Phase Clock Mode 
		 * (FPGA : support only 1 -> No Multi-Phase Clock Mode
		 * [4] : 0 -> 148.5MHz , 1 -> 74.25/37.125MHz 
		 * [3] : 148.5MHz Mode (0 -> Normal Clock , 1 -> Inverted Clock ), 
		 * 74.25/37125MHz Mode (0 -> 148.5MHz Latch , 1 -> Inverted 148.5MHz Latch)
		 * [2] : 0 -> 74.25MHz Clock Mode, 1 -> 37.125MHz Clock Mode
		 * [1:0] : 74.25/37.135MHz Clock Mode Phase Control
		 * (0 : 74.25MHz Clock Mode Normal Clock, 37.125M Clock 0degree Phase
		 * 1 : 74.25MHz Clock Mode Inverted Clock, 37.125M Clock 90degree Phase
		 * 2 : 74.25MHz Clock Mode Normal Clock, 37.125M Clock 180degree Phase
		 * 3 : 74.25MHz Clock Mode Inverted Clock, 37.125M Clock 270degree Phase)" */
#if defined(FPGA_ASIC_TOP)
        .param.rec_clk_phase = 0x30,
#else
# if	((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB)
        /* 8bit */
#  if ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_HD960P)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   else
#    error "Invalid ro clk"
#   endif
#  elif ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_SDH720)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   else
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   endif
#  else
#   if (BD_RO_OUT_MUXCH_CNT <= 1)
        /* 37.125Mhz */
        .param.rec_clk_phase = 0x14,
#   elif (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   else
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   endif
#  endif
# else
        /* 16bit */
#  if ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_HD960P)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   else
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   endif
#  elif ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_SDH720)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 37.125Mhz */
        .param.rec_clk_phase = 0x14,
#   else
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   endif
#  else
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 37.125Mhz */
        .param.rec_clk_phase = 0x14,
#   else
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   endif
#  endif
# endif
#endif
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_REC0_CH_SEL_val, &PI_BD_CONFIG_REC0_CH_SEL_val, sizeof(VIN_REC0_CH_SEL_CONFIG_U));

    // 0xF0F0_0188
	const VIN_REC1_CH_SEL_CONFIG_U PI_BD_CONFIG_REC1_CH_SEL_val = { 
#if	( ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EXT) )
		/* "4CH Multiplexing 1st Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_a_0 = 0,
		/* "4CH Multiplexing 2nd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_b_0 = 2,
		/* "4CH Multiplexing 3rd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_c_0 = 4,
		/* "4CH Multiplexing 4th Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_d_0 = 6,
#else
		/* "4CH Multiplexing 1st Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_a_0 = 1,
		/* "4CH Multiplexing 2nd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_b_0 = 3,
		/* "4CH Multiplexing 3rd Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_c_0 = 5,
		/* "4CH Multiplexing 4th Data Register
		 * (0 : CH0 Y, 1 : CH0 C, 2 : CH1 Y, 3 : CH1 C, 4 : CH2 Y, 5 : CH2 C, 6 : CH3 Y, 7 : CH3 C)" */
		.param.ch_sel_d_0 = 7,
#endif
		/* Record Channel ID Number Selection for CH0 */
		.param.chid_num_0 = 0,
		/* Record Channel ID Number Selection for CH1 */
		.param.chid_num_1 = 1,
		/* Record Channel ID Number Selection for CH2 */
		.param.chid_num_2 = 2,
		/* Record Channel ID Number Selection for CH3 */
		.param.chid_num_3 = 3,
		/* "REC_OUT_FMT Clock Phase Control
		 * [5] : 0 -> Multi-phase Clock Output Mode , 1 -> No Multi-Phase Clock Mode 
		 * (FPGA : support only 1 -> No Multi-Phase Clock Mode
         * [4] : 0 -> 148.5MHz , 1 -> 74.25/37.125MHz 
         * [3] : 148.5MHz Mode (0 -> Normal Clock , 1 -> Inverted Clock ), 
         * 74.25/37125MHz Mode (0 -> 148.5MHz Latch , 1 -> Inverted 148.5MHz Latch)
         * [2] : 0 -> 74.25MHz Clock Mode, 1 -> 37.125MHz Clock Mode
         * [1:0] : 74.25/37.135MHz Clock Mode Phase Control
         * (0 : 74.25MHz Clock Mode Normal Clock, 37.125M Clock 0degree Phase
         * 1 : 74.25MHz Clock Mode Inverted Clock, 37.125M Clock 90degree Phase
         * 2 : 74.25MHz Clock Mode Normal Clock, 37.125M Clock 180degree Phase
         * 3 : 74.25MHz Clock Mode Inverted Clock, 37.125M Clock 270degree Phase)" */
#if defined(FPGA_ASIC_TOP)
        .param.rec_clk_phase = 0x30,
#else
# if	((BD_RO_OUT_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB)
        /* 8bit */
#  if ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_HD960P)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   else
#    error "Invalid ro clk"
#   endif
#  elif ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_SDH720)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   else
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   endif
#  else
#   if (BD_RO_OUT_MUXCH_CNT <= 1)
        /* 37.125Mhz */
        .param.rec_clk_phase = 0x14,
#   elif (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   else
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   endif
#  endif
# else
        /* 16bit */
#  if ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_HD960P)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   else
        /* 148.5Mhz */
        .param.rec_clk_phase = 0x00,
#   endif
#  elif ((BD_RO_OUT_FMT & VID_RESOL_MASKBIT) >= VID_RESOL_SDH720)
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 37.125Mhz */
        .param.rec_clk_phase = 0x14,
#   else
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   endif
#  else
#   if (BD_RO_OUT_MUXCH_CNT <= 2)
        /* 37.125Mhz */
        .param.rec_clk_phase = 0x14,
#   else
        /* 74.25Mhz */
        .param.rec_clk_phase = 0x10,
#   endif
#  endif
# endif
#endif
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_REC1_CH_SEL_val, &PI_BD_CONFIG_REC1_CH_SEL_val, sizeof(VIN_REC1_CH_SEL_CONFIG_U));

    // 0xF0F0_01C0
	const VIN_PAR_PB_FMT_CONFIG_U PI_BD_CONFIG_PAR_PB_FMT_val = { 
		/* "Parallel PB Input Mode Control
		 * [10] : Input Sync Mode
		 * 0 : BT1120 Mode (FF/00/00/XY), 1 : BT656 for 16 Bits Mode (Reserved)
		 * [9:8] : Embeded Sync Position Selection
		 * 0 : Bit[7:0], 1 : Bit[15:08], 2 : Bit[23:16]
		 * [7] : 0 : External Sync Mode, 1 : Embeded Sync Mode
		 * [6:4] : Input Byte Swapping Control for in[23:0] = {P0[11:0], P1[11:0]} => {R, G, B}
		 * 0 : {R / G / B}, 1 : {G / B / R}, 2 : {B / R / G},
		 * 4 : {B / G / R}, 5 : {G / R / B}, 6 : {R / B / G},
		 * 3 : {8'd0, in[23:16], in[11:4]} @ LSB_BYTE  = 0, {8'd0, in[19:12], in[7:0]} @ LSB_BYTE = 1
		 * 7 : {8'd0, in[11:4], in[23:16]} @ LSB_BYTE  = 0, {8'd0, in[7:0], in[19:12]} @ LSB_BYTE = 1
		 * [3] : Data Bit Swap
		 * 0 : Normal, 1 : Bit Swap as In[23:0] = In[0:23]
		 * [2] : Color Space Conversion Enablefor PB
		 * 0 : Bypass, 1 : Color Space Conversion Enable
		 * [1] : PB Input Type
		 * 0 : RGB 24 Bits or YUV 24 Bits
		 * 1 : YC 16 Bits Type
		 * [0] : Input Byte Mode (LSB_BYTE)" */
		.param.par_pb_fmt = ( (0<<10) | ((0&0x3)<<8) | (0<<7) | ((0&0x7)<<4) | (0<<3) | 
#if ( (BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_RGB24 ) 
        (1<<2) |
#else
        (0<<2) |
#endif
#if ( ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC16_EMB) || ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC16_EMB) )
        (1<<1) |
#else
        (0<<1) |
#endif
        (0<<0) ),
		/* "Parallel 16 / 24 Bits Input Mode
		 * 0 : Normal Parallel 8 Bits Mode
		 * 1 : Parallel 16/24 Bits Mode" */
#if ( ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) || ((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT) == VID_TYPE_YC8_EMB) )
		.param.par_pb_input_mode = 0,
#else
		.param.par_pb_input_mode = 1,
#endif
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PAR_PB_FMT_val, &PI_BD_CONFIG_PAR_PB_FMT_val, sizeof(VIN_PAR_PB_FMT_CONFIG_U));

	const VIN_PAR_PB_YCOEF_CONFIG_U PI_BD_CONFIG_PAR_PB_YCOEF_val[4] = { 
		{ //COEF0/*{{{*/ // 0xF0F0_01D0
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ycoef = 0x0000006D,
		},/*}}}*/
		{ //COEF1/*{{{*/ // 0xF0F0_01D4
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ycoef = 0x0000016E,
		},/*}}}*/
		{ //COEF2/*{{{*/ // 0xF0F0_01D8
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ycoef = 0x00000025,
		},/*}}}*/
		{ //COEF3/*{{{*/ // 0xF0F0_01DC
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ycoef = 0x00000000,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_PAR_PB_YCOEF_val, PI_BD_CONFIG_PAR_PB_YCOEF_val, sizeof(VIN_PAR_PB_YCOEF_CONFIG_U)*4);

	const VIN_PAR_PB_UCOEF_CONFIG_U PI_BD_CONFIG_PAR_PB_UCOEF_val[4] = { 
		{ //COEF0/*{{{*/ // 0xF0F0_01E0
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ucoef = 0x00000FC4,
		},/*}}}*/
		{ //COEF1/*{{{*/ // 0xF0F0_01E4
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ucoef = 0x00000F36,
		},/*}}}*/
		{ //COEF2/*{{{*/ // 0xF0F0_01E8
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ucoef = 0x00000106,
		},/*}}}*/
		{ //COEF3/*{{{*/ // 0xF0F0_01EC
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_ucoef = 0x00010000,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_PAR_PB_UCOEF_val, PI_BD_CONFIG_PAR_PB_UCOEF_val, sizeof(VIN_PAR_PB_UCOEF_CONFIG_U)*4);

	const VIN_PAR_PB_VCOEF_CONFIG_U PI_BD_CONFIG_PAR_PB_VCOEF_val[4] = { 
		{ //COEF0/*{{{*/ // 0xF0F0_01F0
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_vcoef = 0x00000106,
		},/*}}}*/
		{ //COEF1/*{{{*/ // 0xF0F0_01F4
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_vcoef = 0x00000F12,
		},/*}}}*/
		{ //COEF2/*{{{*/ // 0xF0F0_01F8
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_vcoef = 0x00000FE8,
		},/*}}}*/
		{ //COEF3/*{{{*/ // 0xF0F0_01FC
			/* "PB Color Space Converter Coefficient
			 * Y = ((YCOEF0 x R + YCOEF1 x G + YCOEF2 x B)/32 + YCOEF3)
			 * U = ((UCOEF0 x R + UCOEF1 x G + UCOEF2 x B)/32 + UCOEF3)
			 * V = ((VCOEF0 x R + VCOEF1 x G + VCOEF2 x B)/32 + UCOEF3)" */
			.param.par_pb_vcoef = 0x00010000,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_PAR_PB_VCOEF_val, PI_BD_CONFIG_PAR_PB_VCOEF_val, sizeof(VIN_PAR_PB_VCOEF_CONFIG_U)*4);

    // 0xF0F0_0200
	const VIN_GENLOCK_LOW_WIDTH_CONFIG_U PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val = { 
		/* Genlock Pulse Start Duration */
		.param.genlock_low_width = 0,
		/* "Genlock Pulse Source Selection
		 * 0 : By GENLOCK_PULSE_EN
		 * 1 : By V Sync Slave" */
		.param.genlock_save_sel	= 0,
		/* "Genlock Pulse High Polarity Selection
		 * 0 : High Polarity = 1
		 * 1 : High Polarity = 0" */
		.param.genlock_pulse_pol = 0,
		/* "Genlock Pulse Enable by Register
		 * 0 : One Time Genlock Pulse Clear
		 * 1 : One Time Genlock Pulse Enable" */
		.param.genlock_pulse_en	= 0,
		/* "Genlock Enable
		 * 0 : Auto Genlock Pulse Disable
		 * 1 : Auto Genlock Pulse Enable" */
		.param.genlock_en = 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val, &PI_BD_CONFIG_GENLOCK_LOW_WIDTH_val, sizeof(VIN_GENLOCK_LOW_WIDTH_CONFIG_U));

    // 0xF0F0_0204
	const VIN_GENLOCK_HIGH_WIDTH_CONFIG_U PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val = { 
		/* Genlock Pulse End Duration */
		.param.genlock_high_width = 0,
		/* "Genlock Slave Sync Selection
		 * [2] : Sync Type Selection
		 * 0 : V Sync, 1 : F Sync
		 * [1:0] is referenced for Video Input Port 3 ~ 0
		 * 0 : Video Input Port 0, ~ 3 : Video Input Port 3" */
		.param.genlock_save_sel	= 0,
		/* "Genlock Enable
		 * 0 : Auto Genlock Pulse Disable
		 * 1 : Auto Genlock Pulse Enable" */
		.param.genlock_save_pol	= 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val, &PI_BD_CONFIG_GENLOCK_HIGH_WIDTH_val, sizeof(VIN_GENLOCK_HIGH_WIDTH_CONFIG_U));

    // 0xF0F0_0220
	const VIN_GENLOCK_START_OS_CONFIG_U PI_BD_CONFIG_GENLOCK_START_OS_val = { 
		/* "Genlock Check Start Offset from Reference Vsync transition
		 * Start Offset = V Blank Width - Allowed Difference Skew / 2" */
		.param.genlock_start_os	= 0,
		/* "Genlock Refrence Input Selection
		 * 0 : Video Input 0 V Sync
		 * 1 : Video Input 1 V Sync
		 * 2 : Video Input 2 V Sync
		 * 3 : Video Input 3 V Sync
		 * 4 : Genlock Refrence Pulse by 0xF0F0_0200 ~ 0xF0F0_0204 register setting" */
		.param.genlock_ref_sel = 0,
		/* "Genlock Locking Check Enable
		 * [3:0] is referenced for Video Input 3 ~ 0
		 * 0 : Genlock Check Disable
		 * 1 : Genlock Check Enable" */
		.param.genlock_chk_en = 0xF,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_GENLOCK_START_OS_val, &PI_BD_CONFIG_GENLOCK_START_OS_val, sizeof(VIN_GENLOCK_START_OS_CONFIG_U));

    // 0xF0F0_0224
	const VIN_GENLOCK_END_OS_CONFIG_U PI_BD_CONFIG_GENLOCK_END_OS_val = { 
		/* "Genlock Check End Offset from Reference Vsync transition
		 * End Offset = V Blank Width + Allowed Difference Skew / 2
		 * Genlock_det is high when ch vsync transition time is mid of strt to end offset" */
		.param.genlock_end_os = 0,
		/* "Bit[0] : Reference V Sync Start Field Selection for Start (For Interlace Option)
		 * Bit[1] : Genlock Check Field Selection for Check (For Interlace Option)
		 * 0 : Fsync = 0
		 * 1 : Fsync = 1" */
		.param.genlock_fld_sel = 0,
		/* "Reference V Sync Transition Selection for Start Time Selection
		 * 0 : Falling Edge
		 * 1 : Rising Edge" */
		.param.genlock_ref_pol = 0,
		/* "Channel V Sync Transition Selection for Genlock Check
		 * 0 : Falling Edge
		 * 1 : Rising Edge" */
		.param.genlock_chk_pol = 1,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_GENLOCK_END_OS_val, &PI_BD_CONFIG_GENLOCK_END_OS_val, sizeof(VIN_GENLOCK_END_OS_CONFIG_U));

    // 0xF0F0_0240
#if ( (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1) )
	const VIN_VID_IN_CTRL_CONFIG_U PI_BD_CONFIG_PB_IN_CTRL_val = { 
		/* "Video Read V Active Size
		 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
		 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
		 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
		 * 1080 : For FHD" */
		.param.vid_rd_vsize = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION]&0x7FF),
		/* "Video Field Polarity Selection
		 * 0 : Even Field High, 1: Odd Field High" */
		.param.vid_rd_fld_pol = 0,
		/* "Video input buffer mode
		 * 0 : 4 Pixels Mode for Normal
		 * 1 : 8 Pixels Mode for MIPI Raw 6/7/8 with MIPI_ASYNC_MD = 0 " */
#if ( (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV) )
        .param.vid_rd_raw_md = ((((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT) <= vin_RAW8) ? 0x1:0), 
#else
		.param.vid_rd_raw_md = 0,
#endif
		/* "Video Read H Active Size
		 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
		.param.vid_rd_hsize = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION]&0x7FF),
		/* "Video Vblank Operation Mode
		 * 0 : Bypass,
		 * 1 : H Sync Free running during V Blank,
		 * 2 : H Sync Off during V Blank" */
		.param.vid_rd_vblk_md = 1,
		/* "Video Input Format Selection
		 * 0 : Progressive Mode, 1 : Interlace Mode" */
		.param.vid_rd_int_md = ((BD_PB_IN_RESOLUTION>=vres_720x480p60)?0:1),
		/* "Video Input Enable
		 * 0 : Disable,  1 : Enable" */
		.param.vid_rd_en = 1,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PB_IN_CTRL_val, &PI_BD_CONFIG_PB_IN_CTRL_val, sizeof(VIN_VID_IN_CTRL_CONFIG_U));

    // 0xF0F0_0244
	const VIN_VID_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_PB_IN_HTOTAL_val = { 
		/* "Video Input Enable
		 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
		 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
		 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
		 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
		 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
		 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
		 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
		.param.vid_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION]&0x3FFF),
		/* "Video Input Data Swap Mode
		 * [5:4] : Swap Cb/Cr Swap
		 * 0 : Normal (For Bayer)
		 * 1 : 8Bits Cb/Cr Swap 
		 * 2 : 8 Bits Y0/Y1 Swap
		 * 3 : 10 Bits Cb/Cr Swap (Reserved)
		 * [3:2] : Swap Position Control {Bayer or Y/C Pair => Pixels Unit}
		 * 0 : Vin0/1/2/3
		 * 1 : Vin1/0/3/2
		 * 2 : Vin2/3/0/1
		 * 3 : Vin3/2/1/0
		 * [1:0] : Y/C Swap {Sub pixel unit}
		 * 0 : No Swap
		 * 1 : 8 Bits MSB/LSB Swap = {In[7:0], In[15:8]}
		 * 2 : 10 Bits MSB/LSB Swap= {In[9:0], In[19:10]}" */
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER)
		.param.vid_rd_swap = ( ((((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT) == vin_RAW10) || (((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT) == vin_RAW12)) ? 0x10:0),
#elif (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
		.param.vid_rd_swap = ( (((BD_VID_PB_IN_FMT & VID_TYPE_MASKBIT)>>VID_TYPE_BITSHIFT) == vin_YUV8_2xRAW8) ? 0x10:0),
#else
		.param.vid_rd_swap = 0,
#endif
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PB_IN_HTOTAL_val, &PI_BD_CONFIG_PB_IN_HTOTAL_val, sizeof(VIN_VID_IN_HTOTAL_CONFIG_U));

    // 0xF0F0_0248
	const VIN_VID_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_PB_IN_VTOTAL_val = { 
		/* "Video read V Total Size for Odd Field
		 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
		 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
		 * 750 : For HD 30p/25p/60p/50p,
		 * 825 : For 1280x800 HD 30p/25p/60p/50p
		 * 1000 : For 1280x960 HD 30p/25p
		 * 1125 : For FHD 30p/25p" */
		.param.vid_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][0]&0x3FFF),
		/* "Video Read V Total Size for Even Field
		 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
		 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
		 * 750 : For HD 30p/25p/60p/50p,
		 * 825 : For 1280x800 HD 30p/25p/60p/50p
		 * 1000 : For 1280x960 HD 30p/25p
		 * 1125 : For FHD 30p/25p" */
		.param.vid_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][1]&0x3FFF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PB_IN_VTOTAL_val, &PI_BD_CONFIG_PB_IN_VTOTAL_val, sizeof(VIN_VID_IN_VTOTAL_CONFIG_U));

    // 0xF0F0_024C
	const VIN_VID_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_PB_IN_FLD_OS_val = { 
		/* "Selection of F transition Line for Odd Field
		 * VID_RD_VSIZE + 2 for Interlace
		 * 0 for Progressive" */
		.param.vid_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][0]&0x3FFF),
		/* "Selection of F transition Line for Even Field
		 * VID_RD_VSIZE + 3 for Interlace
		 * 0 for Progressive" */
		.param.vid_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][0]&0x3FFF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PB_IN_FLD_OS_val, &PI_BD_CONFIG_PB_IN_FLD_OS_val, sizeof(VIN_VID_IN_FLD_OS_CONFIG_U));

    // 0xF0F0_0250
	const VIN_REC_IN_CTRL_CONFIG_U PI_BD_CONFIG_PBREC_IN_CTRL_val = { 
		/* "Video Read V Active Size
		 * 240 : For 720 or 960 SD 60i, 288 : For 720 or 960 SD 50i,
		 * 480 : For 720 or 960 SD 60p, 576 : For 720 or 960 SD 50p,
		 * 720 : For 720 HD, 800 : For 800 HD, 960 : For 960 HD,
		 * 1080 : For FHD" */
		.param.resync_rd_vsize = (DEF_VIN_VACTIVE_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION]&0x7FF),
		/* "Video Field Polarity Selection
		 * 0 : Even Field High, 1: Odd Field High" */
		.param.resync_rd_fld_pol = 0,
		/* "Video Read H Active Size
		 * 720 : For 720 SD, 960 : For 960SD, 1280 : For HD, 1920 : For FHD" */
		.param.resync_rd_hsize = (DEF_VIN_HACTIVE_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION]&0x7FF),
		/* "Video Vblank Operation Mode
		 * 0 : Bypass,
		 * 1 : H Sync Free running
		 * 2 : H Sync Off" */
		.param.resync_rd_vblk_md = 1,
		/* "Video Input Format Selection
		 * 0 : Progressive Mode, 1 : Interlace Mode" */
		.param.resync_rd_int_md	= ((BD_PB_IN_RESOLUTION>=vres_720x480p60)?0:1),
		/* "Video Input Enable
		 * 0 : Disable,  1 : Enable" */
		.param.resync_rd_en = 1,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PBREC_IN_CTRL_val, &PI_BD_CONFIG_PBREC_IN_CTRL_val, sizeof(VIN_REC_IN_CTRL_CONFIG_U));

    // 0xF0F0_0254
	const VIN_REC_IN_HTOTAL_CONFIG_U PI_BD_CONFIG_PBREC_IN_HTOTAL_val = { 
		/* "Video Input Enable
		 * 858 for 720 SD 60i/60p, 864 for 720 SD 50i/50p
		 * 1144 for 960 SD 60i/60p, 1152 for 960 SD 50i/50p
		 * 1650 : For 720 HD 30p/60p,  1980 : For 720 HD 25p/50p
		 * 3000 : For 800 HD 30p, 3600 : For 800 HD 25p,
		 * 1500 : For 800 HD 60p, 1800 : For 800 HD 50p,
		 * 2475 : For 960 HD 30p, 2970 : For 960 HD 25p,
		 * 2200 : For FHD 30p, 2640 : For FHD 25p" */
		.param.resync_rd_htotal = (DEF_VIN_HTOTAL_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION]&0x3FFF),
		/* " Brightness Selection for Input Channel
		 * 0xFF : Brightness High
		 * 0x80 : Default
		 * 0x00 : Brightness Low" */
		.param.resync_brt_ctrl = 0x80,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PBREC_IN_HTOTAL_val, &PI_BD_CONFIG_PBREC_IN_HTOTAL_val, sizeof(VIN_REC_IN_HTOTAL_CONFIG_U));

    // 0xF0F0_0258
	const VIN_REC_IN_VTOTAL_CONFIG_U PI_BD_CONFIG_PBREC_IN_VTOTAL_val = { 
		/* "Video read V Total Size for Odd Field
		 * 262 : For 720 or 960 SD 60i, 312 : For 720 or 960 SD 50i,
		 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
		 * 750 : For HD 30p/25p/60p/50p,
		 * 825 : For 1280x800 HD 30p/25p/60p/50p
		 * 1000 : For 1280x960 HD 30p/25p
		 * 1125 : For FHD 30p/25p" */
		.param.resync_rd_vtotal_odd = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][0]&0x3FFF),
		/* "Video Read V Total Size for Even Field
		 * 263 : For 720 or 960 SD 60i, 313 : For 720 or 960 SD 50i,
		 * 525 : For 720 or 960 SD 60p, 625 : For 720 or 960 SD 50p,
		 * 750 : For HD 30p/25p/60p/50p,
		 * 825 : For 1280x800 HD 30p/25p/60p/50p
		 * 1000 : For 1280x960 HD 30p/25p
		 * 1125 : For FHD 30p/25p" */
		.param.resync_rd_vtotal_even = (DEF_VIN_ODD_EVEN_FIELD_VTOTAL_SIZE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][1]&0x3FFF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PBREC_IN_VTOTAL_val, &PI_BD_CONFIG_PBREC_IN_VTOTAL_val, sizeof(VIN_REC_IN_VTOTAL_CONFIG_U));

    // 0xF0F0_025C
	const VIN_REC_IN_FLD_OS_CONFIG_U PI_BD_CONFIG_PBREC_IN_FLD_OS_val = { 
		/* "Selection of F transition Line for Odd Field
		 * REC_RD_VSIZE + 2 for Interlace
		 * 0 for Progressive" */
		.param.resync_rd_fld_os_odd = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][0]&0x3FFF),
		/* "Selection of F transition Line for Even Field
		 * REC_RD_VSIZE + 3 for Interlace
		 * 0 for Progressive" */
		.param.resync_rd_fld_os_even = (DEF_VIN_ODD_EVEN_FIELD_F_TRANSITION_LINE[PI_BD_CONFIG_PB_VinVideoType[0]][BD_PB_IN_RESOLUTION][1]&0x3FFF),
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_PBREC_IN_FLD_OS_val, &PI_BD_CONFIG_PBREC_IN_FLD_OS_val, sizeof(VIN_REC_IN_FLD_OS_CONFIG_U));
#endif //( (VIDEO_IN_TYPE != VIDEO_IN_TYPE_PARALLEL) && (VIDEO_IN_TYPE_PB_SUPPORT == 1) )

	const VIN_VID_CRC_CTRL_CONFIG_U PI_BD_CONFIG_VID_CRC_CTRL_val[6] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0300
			/* CRC Sum Reference value for crc bist checking of current frame */
			.param.vid_crc_ref_data	= 0,
			/* "CRC Detection Period Selection
			 * 0 : 1 Frame Period
			 * ~ 7 : 8 Frame Period" */
			.param.vid_crc_fr_period = 0,
			/* "CRC Detection Field Selection @ Interlace Mode
			 * 0 : Odd Field (fsync = 0)
			 * 1 : Even Field (fsync = 1)" */
			.param.vid_crc_fld_sel = 0,
			/* "CRC Video Input Mode
			 * 0 : Progressive Mode
			 * 1 : Interlace Mode" */
			.param.vid_crc_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "CRC ROI Region Selection Mode
			 * 0 : CRC Detect for Inside of ROI Region
			 * 1 : CRC Detect for Outside of ROI Region" */
			.param.vid_crc_roi_md = 0,
			/* "CRC ROI Region Enable
			 * 0 : Disable ROI Area Selection
			 * 1 : Enable ROI Area Selection" */
			.param.vid_crc_roi_en = 0,
			/* "CRC Bist Enable
			 * 0 : Disable CRC BIST Detection
			 * 1 : Enable CRC BIST Detection" */
			.param.vid_crc_bist_en = 1,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0310
			/* CRC Sum Reference value for crc bist checking of current frame */
			.param.vid_crc_ref_data	= 0,
			/* "CRC Detection Period Selection
			 * 0 : 1 Frame Period
			 * ~ 7 : 8 Frame Period" */
			.param.vid_crc_fr_period = 0,
			/* "CRC Detection Field Selection @ Interlace Mode
			 * 0 : Odd Field (fsync = 0)
			 * 1 : Even Field (fsync = 1)" */
			.param.vid_crc_fld_sel = 0,
			/* "CRC Video Input Mode
			 * 0 : Progressive Mode
			 * 1 : Interlace Mode" */
			.param.vid_crc_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "CRC ROI Region Selection Mode
			 * 0 : CRC Detect for Inside of ROI Region
			 * 1 : CRC Detect for Outside of ROI Region" */
			.param.vid_crc_roi_md = 0,
			/* "CRC ROI Region Enable
			 * 0 : Disable ROI Area Selection
			 * 1 : Enable ROI Area Selection" */
			.param.vid_crc_roi_en = 0,
			/* "CRC Bist Enable
			 * 0 : Disable CRC BIST Detection
			 * 1 : Enable CRC BIST Detection" */
			.param.vid_crc_bist_en = 1,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0320
			/* CRC Sum Reference value for crc bist checking of current frame */
			.param.vid_crc_ref_data	= 0,
			/* "CRC Detection Period Selection
			 * 0 : 1 Frame Period
			 * ~ 7 : 8 Frame Period" */
			.param.vid_crc_fr_period = 0,
			/* "CRC Detection Field Selection @ Interlace Mode
			 * 0 : Odd Field (fsync = 0)
			 * 1 : Even Field (fsync = 1)" */
			.param.vid_crc_fld_sel = 0,
			/* "CRC Video Input Mode
			 * 0 : Progressive Mode
			 * 1 : Interlace Mode" */
			.param.vid_crc_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "CRC ROI Region Selection Mode
			 * 0 : CRC Detect for Inside of ROI Region
			 * 1 : CRC Detect for Outside of ROI Region" */
			.param.vid_crc_roi_md = 0,
			/* "CRC ROI Region Enable
			 * 0 : Disable ROI Area Selection
			 * 1 : Enable ROI Area Selection" */
			.param.vid_crc_roi_en = 0,
			/* "CRC Bist Enable
			 * 0 : Disable CRC BIST Detection
			 * 1 : Enable CRC BIST Detection" */
			.param.vid_crc_bist_en = 1,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0330
			/* CRC Sum Reference value for crc bist checking of current frame */
			.param.vid_crc_ref_data	= 0,
			/* "CRC Detection Period Selection
			 * 0 : 1 Frame Period
			 * ~ 7 : 8 Frame Period" */
			.param.vid_crc_fr_period = 0,
			/* "CRC Detection Field Selection @ Interlace Mode
			 * 0 : Odd Field (fsync = 0)
			 * 1 : Even Field (fsync = 1)" */
			.param.vid_crc_fld_sel = 0,
			/* "CRC Video Input Mode
			 * 0 : Progressive Mode
			 * 1 : Interlace Mode" */
			.param.vid_crc_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "CRC ROI Region Selection Mode
			 * 0 : CRC Detect for Inside of ROI Region
			 * 1 : CRC Detect for Outside of ROI Region" */
			.param.vid_crc_roi_md = 0,
			/* "CRC ROI Region Enable
			 * 0 : Disable ROI Area Selection
			 * 1 : Enable ROI Area Selection" */
			.param.vid_crc_roi_en = 0,
			/* "CRC Bist Enable
			 * 0 : Disable CRC BIST Detection
			 * 1 : Enable CRC BIST Detection" */
			.param.vid_crc_bist_en = 1,
		},/*}}}*/
		{ //VIN4/*{{{*/ // 0xF0F0_0340
			/* CRC Sum Reference value for crc bist checking of current frame */
			.param.vid_crc_ref_data	= 0,
			/* "CRC Detection Period Selection
			 * 0 : 1 Frame Period
			 * ~ 7 : 8 Frame Period" */
			.param.vid_crc_fr_period = 0,
			/* "CRC Detection Field Selection @ Interlace Mode
			 * 0 : Odd Field (fsync = 0)
			 * 1 : Even Field (fsync = 1)" */
			.param.vid_crc_fld_sel = 0,
			/* "CRC Video Input Mode
			 * 0 : Progressive Mode
			 * 1 : Interlace Mode" */
			.param.vid_crc_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "CRC ROI Region Selection Mode
			 * 0 : CRC Detect for Inside of ROI Region
			 * 1 : CRC Detect for Outside of ROI Region" */
			.param.vid_crc_roi_md = 0,
			/* "CRC ROI Region Enable
			 * 0 : Disable ROI Area Selection
			 * 1 : Enable ROI Area Selection" */
			.param.vid_crc_roi_en = 0,
			/* "CRC Bist Enable
			 * 0 : Disable CRC BIST Detection
			 * 1 : Enable CRC BIST Detection" */
			.param.vid_crc_bist_en = 1,
		},/*}}}*/
		{ //VIN5/*{{{*/ // 0xF0F0_0350
			/* CRC Sum Reference value for crc bist checking of current frame */
			.param.vid_crc_ref_data	= 0,
			/* "CRC Detection Period Selection
			 * 0 : 1 Frame Period
			 * ~ 7 : 8 Frame Period" */
			.param.vid_crc_fr_period = 0,
			/* "CRC Detection Field Selection @ Interlace Mode
			 * 0 : Odd Field (fsync = 0)
			 * 1 : Even Field (fsync = 1)" */
			.param.vid_crc_fld_sel = 0,
			/* "CRC Video Input Mode
			 * 0 : Progressive Mode
			 * 1 : Interlace Mode" */
			.param.vid_crc_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "CRC ROI Region Selection Mode
			 * 0 : CRC Detect for Inside of ROI Region
			 * 1 : CRC Detect for Outside of ROI Region" */
			.param.vid_crc_roi_md = 0,
			/* "CRC ROI Region Enable
			 * 0 : Disable ROI Area Selection
			 * 1 : Enable ROI Area Selection" */
			.param.vid_crc_roi_en = 0,
			/* "CRC Bist Enable
			 * 0 : Disable CRC BIST Detection
			 * 1 : Enable CRC BIST Detection" */
			.param.vid_crc_bist_en = 1,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_CRC_CTRL_val, PI_BD_CONFIG_VID_CRC_CTRL_val, sizeof(VIN_VID_CRC_CTRL_CONFIG_U)*6);

	const VIN_VID_CRC_STRT_CONFIG_U PI_BD_CONFIG_VID_CRC_STRT_val[6] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0304
			/* "Quad CRC ROI Region Selection for Horizontal Start Pixel
			 * 0 : No Start Offset" */
			.param.vid_crc_hstrt = 0,
			/* "Quad CRC ROI Region Selection for Vertical Start Line
			 * 0 : No Start Offset" */
			.param.vid_crc_vstrt = 0,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0314
			/* "Quad CRC ROI Region Selection for Horizontal Start Pixel
			 * 0 : No Start Offset" */
			.param.vid_crc_hstrt = 0,
			/* "Quad CRC ROI Region Selection for Vertical Start Line
			 * 0 : No Start Offset" */
			.param.vid_crc_vstrt = 0,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0324
			/* "Quad CRC ROI Region Selection for Horizontal Start Pixel
			 * 0 : No Start Offset" */
			.param.vid_crc_hstrt = 0,
			/* "Quad CRC ROI Region Selection for Vertical Start Line
			 * 0 : No Start Offset" */
			.param.vid_crc_vstrt = 0,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0334
			/* "Quad CRC ROI Region Selection for Horizontal Start Pixel
			 * 0 : No Start Offset" */
			.param.vid_crc_hstrt = 0,
			/* "Quad CRC ROI Region Selection for Vertical Start Line
			 * 0 : No Start Offset" */
			.param.vid_crc_vstrt = 0,
		},/*}}}*/
		{ //VIN4/*{{{*/ // 0xF0F0_0344
			/* "Quad CRC ROI Region Selection for Horizontal Start Pixel
			 * 0 : No Start Offset" */
			.param.vid_crc_hstrt = 0,
			/* "Quad CRC ROI Region Selection for Vertical Start Line
			 * 0 : No Start Offset" */
			.param.vid_crc_vstrt = 0,
		},/*}}}*/
		{ //VIN5/*{{{*/ // 0xF0F0_0354
			/* "Quad CRC ROI Region Selection for Horizontal Start Pixel
			 * 0 : No Start Offset" */
			.param.vid_crc_hstrt = 0,
			/* "Quad CRC ROI Region Selection for Vertical Start Line
			 * 0 : No Start Offset" */
			.param.vid_crc_vstrt = 0,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_CRC_STRT_val, PI_BD_CONFIG_VID_CRC_STRT_val, sizeof(VIN_VID_CRC_STRT_CONFIG_U)*6);

	const VIN_VID_CRC_END_CONFIG_U PI_BD_CONFIG_VID_CRC_END_val[6] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0308
			/* "Quad CRC ROI Region Selection for Horizontal End Pixel
			 * 1920 : No End Offset" */
			.param.vid_crc_hend = (DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
			/* "Quad CRC ROI Region Selection for Vertical End Line
			 * 1080 : No End Offset" */
			.param.vid_crc_vend = (DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0318
			/* "Quad CRC ROI Region Selection for Horizontal End Pixel
			 * 1920 : No End Offset" */
			.param.vid_crc_hend = (DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
			/* "Quad CRC ROI Region Selection for Vertical End Line
			 * 1080 : No End Offset" */
			.param.vid_crc_vend = (DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0328
			/* "Quad CRC ROI Region Selection for Horizontal End Pixel
			 * 1920 : No End Offset" */
			.param.vid_crc_hend = (DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
			/* "Quad CRC ROI Region Selection for Vertical End Line
			 * 1080 : No End Offset" */
			.param.vid_crc_vend = (DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0338
			/* "Quad CRC ROI Region Selection for Horizontal End Pixel
			 * 1920 : No End Offset" */
			.param.vid_crc_hend = (DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
			/* "Quad CRC ROI Region Selection for Vertical End Line
			 * 1080 : No End Offset" */
			.param.vid_crc_vend = (DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
		},/*}}}*/
		{ //VIN4/*{{{*/ // 0xF0F0_0348
			/* "Quad CRC ROI Region Selection for Horizontal End Pixel
			 * 1920 : No End Offset" */
			.param.vid_crc_hend = (DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
			/* "Quad CRC ROI Region Selection for Vertical End Line
			 * 1080 : No End Offset" */
			.param.vid_crc_vend = (DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
		},/*}}}*/
		{ //VIN5/*{{{*/ // 0xF0F0_0358
			/* "Quad CRC ROI Region Selection for Horizontal End Pixel
			 * 1920 : No End Offset" */
			.param.vid_crc_hend = (DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
			/* "Quad CRC ROI Region Selection for Vertical End Line
			 * 1080 : No End Offset" */
			.param.vid_crc_vend = (DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION]&0x7FF)-1,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_VID_CRC_END_val, PI_BD_CONFIG_VID_CRC_END_val, sizeof(VIN_VID_CRC_END_CONFIG_U)*6);

    // 0xF0F0_03C0
	const VIN_QUAD_DDR_CRC_CTRL_CONFIG_U PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val = { 
		/* "Quad Read CRC Checker Data Type Selection
		 * 0 : Reserved" */
		.param.quad_crc_ddrro_mode = 0,
		/* "DDR Read CRC Checker Enable for Quad Path
		 * 0 : Normal
		 * 1 : DDR Read CRC Checker Enable (Test Purpose Only)" */
		.param.quad_cc_ddrro_en	= 0,
		/* "DDR Write CRC Checker Enable for Quad Path
		 * 0 : Normal
		 * 1 : DDR Write CRC Checker Enable (Test Purpose Only)" */
		.param.quad_cc_ddrwo_en	= 0,
	};
	memcpy(&pVinUserReg->PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val, &PI_BD_CONFIG_QUAD_DDR_CRC_CTRL_val, sizeof(VIN_QUAD_DDR_CRC_CTRL_CONFIG_U));

	const VIN_SYNC_VIN_HSIZE_CONFIG_U PI_BD_CONFIG_SYNC_VIN_HSIZE_val[5] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0440
			/* "Sync Hsize Selection for H Size Lock detection
			 * 858 : For 720H SD 60i, 864 : for 720H SD 50i
			 * 1144 : For 960H SD 60i, 1152 : for 960H SD 50i
			 * 1650 : For 720 HD 30p, 1980 : for 720 HD 25p (37.125M)
			 * 1650 : For 720 HD 60p, 1980 : for 720 HD 50p (74.25M)
			 * 2200 : For 1080 HD 60p, 2640 : for 1080 HD 50p (74.25M)" */
			.param.sync_hsize = (DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]&0x3FFF),
			/* "Selection Format Detection Enable for Video Input 0
			 * [3] : V Active Size Detection Enable for Format Detection
			 * [2] : H Acitve Size Detection Enable for Format Detection
			 * [1] : 1 Frame Size Detection Enable for Format Detection
			 * [0] : 1 H Size Detection Enable for Format Detection (If 1H Cycle is variated, set to ""0"" such as OV9716)
			 * 0 : Format Detection Disable
			 * 1 : Format Detection Enable" */
			.param.fomat_det_en = 0xE,
			/* "Sync Period Detection Threshold Selection
			 * 0 : +/- 6.25 % Offset
			 * 1 : +/- 12.5 % Offset" */
			.param.sync_det_th = 0,
			/* "Input Format Defininition
			 * 0 : Progressive
			 * 1 : Interlace" */
			.param.sync_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Loss Detection Enable for Video Input 0
			 * 0 : Video Loss Detection Disable
			 * 1 : Video Loss Detection Enable" */
			.param.vloss_det_en = 1,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_044C
			/* "Sync Hsize Selection for H Size Lock detection
			 * 858 : For 720H SD 60i, 864 : for 720H SD 50i
			 * 1144 : For 960H SD 60i, 1152 : for 960H SD 50i
			 * 1650 : For 720 HD 30p, 1980 : for 720 HD 25p (37.125M)
			 * 1650 : For 720 HD 60p, 1980 : for 720 HD 50p (74.25M)
			 * 2200 : For 1080 HD 60p, 2640 : for 1080 HD 50p (74.25M)" */
			.param.sync_hsize = (DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]&0x3FFF),
			/* "Selection Format Detection Enable for Video Input 0
			 * [3] : V Active Size Detection Enable for Format Detection
			 * [2] : H Acitve Size Detection Enable for Format Detection
			 * [1] : 1 Frame Size Detection Enable for Format Detection
			 * [0] : 1 H Size Detection Enable for Format Detection (If 1H Cycle is variated, set to ""0"" such as OV9716)
			 * 0 : Format Detection Disable
			 * 1 : Format Detection Enable" */
			.param.fomat_det_en = 0xE,
			/* "Sync Period Detection Threshold Selection
			 * 0 : +/- 6.25 % Offset
			 * 1 : +/- 12.5 % Offset" */
			.param.sync_det_th = 0,
			/* "Input Format Defininition
			 * 0 : Progressive
			 * 1 : Interlace" */
			.param.sync_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Loss Detection Enable for Video Input 0
			 * 0 : Video Loss Detection Disable
			 * 1 : Video Loss Detection Enable" */
			.param.vloss_det_en = 1,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0458
			/* "Sync Hsize Selection for H Size Lock detection
			 * 858 : For 720H SD 60i, 864 : for 720H SD 50i
			 * 1144 : For 960H SD 60i, 1152 : for 960H SD 50i
			 * 1650 : For 720 HD 30p, 1980 : for 720 HD 25p (37.125M)
			 * 1650 : For 720 HD 60p, 1980 : for 720 HD 50p (74.25M)
			 * 2200 : For 1080 HD 60p, 2640 : for 1080 HD 50p (74.25M)" */
			.param.sync_hsize = (DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]&0x3FFF),
			/* "Selection Format Detection Enable for Video Input 0
			 * [3] : V Active Size Detection Enable for Format Detection
			 * [2] : H Acitve Size Detection Enable for Format Detection
			 * [1] : 1 Frame Size Detection Enable for Format Detection
			 * [0] : 1 H Size Detection Enable for Format Detection (If 1H Cycle is variated, set to ""0"" such as OV9716)
			 * 0 : Format Detection Disable
			 * 1 : Format Detection Enable" */
			.param.fomat_det_en = 0xE,
			/* "Sync Period Detection Threshold Selection
			 * 0 : +/- 6.25 % Offset
			 * 1 : +/- 12.5 % Offset" */
			.param.sync_det_th = 0,
			/* "Input Format Defininition
			 * 0 : Progressive
			 * 1 : Interlace" */
			.param.sync_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Loss Detection Enable for Video Input 0
			 * 0 : Video Loss Detection Disable
			 * 1 : Video Loss Detection Enable" */
			.param.vloss_det_en = 1,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0464
			/* "Sync Hsize Selection for H Size Lock detection
			 * 858 : For 720H SD 60i, 864 : for 720H SD 50i
			 * 1144 : For 960H SD 60i, 1152 : for 960H SD 50i
			 * 1650 : For 720 HD 30p, 1980 : for 720 HD 25p (37.125M)
			 * 1650 : For 720 HD 60p, 1980 : for 720 HD 50p (74.25M)
			 * 2200 : For 1080 HD 60p, 2640 : for 1080 HD 50p (74.25M)" */
			.param.sync_hsize = (DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]&0x3FFF),
			/* "Selection Format Detection Enable for Video Input 0
			 * [3] : V Active Size Detection Enable for Format Detection
			 * [2] : H Acitve Size Detection Enable for Format Detection
			 * [1] : 1 Frame Size Detection Enable for Format Detection
			 * [0] : 1 H Size Detection Enable for Format Detection (If 1H Cycle is variated, set to ""0"" such as OV9716)
			 * 0 : Format Detection Disable
			 * 1 : Format Detection Enable" */
			.param.fomat_det_en = 0xE,
			/* "Sync Period Detection Threshold Selection
			 * 0 : +/- 6.25 % Offset
			 * 1 : +/- 12.5 % Offset" */
			.param.sync_det_th = 0,
			/* "Input Format Defininition
			 * 0 : Progressive
			 * 1 : Interlace" */
			.param.sync_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Loss Detection Enable for Video Input 0
			 * 0 : Video Loss Detection Disable
			 * 1 : Video Loss Detection Enable" */
			.param.vloss_det_en = 1,
		},/*}}}*/
		{ //VIN4/*{{{*/ // 0xF0F0_0470
			/* "Sync Hsize Selection for H Size Lock detection
			 * 858 : For 720H SD 60i, 864 : for 720H SD 50i
			 * 1144 : For 960H SD 60i, 1152 : for 960H SD 50i
			 * 1650 : For 720 HD 30p, 1980 : for 720 HD 25p (37.125M)
			 * 1650 : For 720 HD 60p, 1980 : for 720 HD 50p (74.25M)
			 * 2200 : For 1080 HD 60p, 2640 : for 1080 HD 50p (74.25M)" */
			.param.sync_hsize = (DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]&0x3FFF),
			/* "Selection Format Detection Enable for Video Input 0
			 * [3] : V Active Size Detection Enable for Format Detection
			 * [2] : H Acitve Size Detection Enable for Format Detection
			 * [1] : 1 Frame Size Detection Enable for Format Detection
			 * [0] : 1 H Size Detection Enable for Format Detection (If 1H Cycle is variated, set to ""0"" such as OV9716)
			 * 0 : Format Detection Disable
			 * 1 : Format Detection Enable" */
			.param.fomat_det_en = 0xE,
			/* "Sync Period Detection Threshold Selection
			 * 0 : +/- 6.25 % Offset
			 * 1 : +/- 12.5 % Offset" */
			.param.sync_det_th = 0,
			/* "Input Format Defininition
			 * 0 : Progressive
			 * 1 : Interlace" */
			.param.sync_int_md = ((BD_IN_RESOLUTION>=vres_720x480p60)?0:1),
			/* "Video Loss Detection Enable for Video Input 0
			 * 0 : Video Loss Detection Disable
			 * 1 : Video Loss Detection Enable" */
			.param.vloss_det_en = 1,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_SYNC_VIN_HSIZE_val, PI_BD_CONFIG_SYNC_VIN_HSIZE_val, sizeof(VIN_SYNC_VIN_HSIZE_CONFIG_U)*5);

	const VIN_SYNC_VIN_FSIZE_CONFIG_U PI_BD_CONFIG_SYNC_VIN_FSIZE_val[5] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0444
			/* "Sync Frame Size Selection for F Size Lock Detection
			 * 858 x 525 = 0x06DF92 for 480 SD 60i (13.5M) or 720 x 480 = 0x054600 for HxVactive
			 * 864 x 625 = 0x083D60 for 576 SD 50i (13.5M)
			 * 858 x 525 = 0x06DF92 for 480 SD 60p (27M)
			 * 864 x 625 = 0x083D60 for 576 SD 50p (27M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60i (18M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50i (18M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60p (36M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50p (36M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 30p (37.125M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 25p (37.125M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 60p (74.25M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 50p (74.25M)
			 * 2475 x 1000 = 0x25C3F8 for 960 HD 30p (74.25M)
			 * 2970 x 1000 = 0x2D5190 for 960 HD 25p (74.25M)
			 * 2200 x 1125 = 0x25C3F8 for 1080 FHD 30p (74.25M)
			 * 2640 x 1125 = 0x2D5190 for 1080 FHD 25p (74.25M)" */
			.param.sync_fsize = ((DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]*DEF_VIN_MAXLINE_CNT[0][BD_IN_RESOLUTION])&0x00FFFFFF),
			/* "Sync F Size Mode Selection Option
			 * 0 : Frame Rate Check for FSIZE (Htotal x Vtotal for Frame Rate)
			 * 1 : Frame Active Size Check for FSIZE (Hactive x Vactive for HV Active Size)" */
			.param.sync_hvact_md = 0,
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0450
			/* "Sync Frame Size Selection for F Size Lock Detection
			 * 858 x 525 = 0x06DF92 for 480 SD 60i (13.5M) or 720 x 480 = 0x054600 for HxVactive
			 * 864 x 625 = 0x083D60 for 576 SD 50i (13.5M)
			 * 858 x 525 = 0x06DF92 for 480 SD 60p (27M)
			 * 864 x 625 = 0x083D60 for 576 SD 50p (27M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60i (18M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50i (18M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60p (36M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50p (36M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 30p (37.125M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 25p (37.125M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 60p (74.25M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 50p (74.25M)
			 * 2475 x 1000 = 0x25C3F8 for 960 HD 30p (74.25M)
			 * 2970 x 1000 = 0x2D5190 for 960 HD 25p (74.25M)
			 * 2200 x 1125 = 0x25C3F8 for 1080 FHD 30p (74.25M)
			 * 2640 x 1125 = 0x2D5190 for 1080 FHD 25p (74.25M)" */
			.param.sync_fsize = ((DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]*DEF_VIN_MAXLINE_CNT[0][BD_IN_RESOLUTION])&0x00FFFFFF),
			/* "Sync F Size Mode Selection Option
			 * 0 : Frame Rate Check for FSIZE (Htotal x Vtotal for Frame Rate)
			 * 1 : Frame Active Size Check for FSIZE (Hactive x Vactive for HV Active Size)" */
			.param.sync_hvact_md = 0,
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_045C
			/* "Sync Frame Size Selection for F Size Lock Detection
			 * 858 x 525 = 0x06DF92 for 480 SD 60i (13.5M) or 720 x 480 = 0x054600 for HxVactive
			 * 864 x 625 = 0x083D60 for 576 SD 50i (13.5M)
			 * 858 x 525 = 0x06DF92 for 480 SD 60p (27M)
			 * 864 x 625 = 0x083D60 for 576 SD 50p (27M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60i (18M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50i (18M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60p (36M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50p (36M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 30p (37.125M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 25p (37.125M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 60p (74.25M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 50p (74.25M)
			 * 2475 x 1000 = 0x25C3F8 for 960 HD 30p (74.25M)
			 * 2970 x 1000 = 0x2D5190 for 960 HD 25p (74.25M)
			 * 2200 x 1125 = 0x25C3F8 for 1080 FHD 30p (74.25M)
			 * 2640 x 1125 = 0x2D5190 for 1080 FHD 25p (74.25M)" */
			.param.sync_fsize = ((DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]*DEF_VIN_MAXLINE_CNT[0][BD_IN_RESOLUTION])&0x00FFFFFF),
			/* "Sync F Size Mode Selection Option
			 * 0 : Frame Rate Check for FSIZE (Htotal x Vtotal for Frame Rate)
			 * 1 : Frame Active Size Check for FSIZE (Hactive x Vactive for HV Active Size)" */
			.param.sync_hvact_md = 0,
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_0468
			/* "Sync Frame Size Selection for F Size Lock Detection
			 * 858 x 525 = 0x06DF92 for 480 SD 60i (13.5M) or 720 x 480 = 0x054600 for HxVactive
			 * 864 x 625 = 0x083D60 for 576 SD 50i (13.5M)
			 * 858 x 525 = 0x06DF92 for 480 SD 60p (27M)
			 * 864 x 625 = 0x083D60 for 576 SD 50p (27M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60i (18M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50i (18M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60p (36M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50p (36M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 30p (37.125M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 25p (37.125M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 60p (74.25M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 50p (74.25M)
			 * 2475 x 1000 = 0x25C3F8 for 960 HD 30p (74.25M)
			 * 2970 x 1000 = 0x2D5190 for 960 HD 25p (74.25M)
			 * 2200 x 1125 = 0x25C3F8 for 1080 FHD 30p (74.25M)
			 * 2640 x 1125 = 0x2D5190 for 1080 FHD 25p (74.25M)" */
			.param.sync_fsize = ((DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]*DEF_VIN_MAXLINE_CNT[0][BD_IN_RESOLUTION])&0x00FFFFFF),
			/* "Sync F Size Mode Selection Option
			 * 0 : Frame Rate Check for FSIZE (Htotal x Vtotal for Frame Rate)
			 * 1 : Frame Active Size Check for FSIZE (Hactive x Vactive for HV Active Size)" */
			.param.sync_hvact_md = 0,
		},/*}}}*/
		{ //VIN4/*{{{*/ // 0xF0F0_0474
			/* "Sync Frame Size Selection for F Size Lock Detection
			 * 858 x 525 = 0x06DF92 for 480 SD 60i (13.5M) or 720 x 480 = 0x054600 for HxVactive
			 * 864 x 625 = 0x083D60 for 576 SD 50i (13.5M)
			 * 858 x 525 = 0x06DF92 for 480 SD 60p (27M)
			 * 864 x 625 = 0x083D60 for 576 SD 50p (27M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60i (18M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50i (18M)
			 * 1144 x 525 = 0x092A18 for 480 SD 60p (36M)
			 * 1152 x 625 = 0x0AFC80 for 576 SD 50p (36M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 30p (37.125M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 25p (37.125M)
			 * 1650 x 750 = 0x12E1FC for 720 HD 60p (74.25M)
			 * 1980 x 750 = 0x16A8C8 for 720 HD 50p (74.25M)
			 * 2475 x 1000 = 0x25C3F8 for 960 HD 30p (74.25M)
			 * 2970 x 1000 = 0x2D5190 for 960 HD 25p (74.25M)
			 * 2200 x 1125 = 0x25C3F8 for 1080 FHD 30p (74.25M)
			 * 2640 x 1125 = 0x2D5190 for 1080 FHD 25p (74.25M)" */
			.param.sync_fsize = ((DEF_VIN_HTOTAL_SIZE[0][BD_IN_RESOLUTION]*DEF_VIN_MAXLINE_CNT[0][BD_IN_RESOLUTION])&0x00FFFFFF),
			/* "Sync F Size Mode Selection Option
			 * 0 : Frame Rate Check for FSIZE (Htotal x Vtotal for Frame Rate)
			 * 1 : Frame Active Size Check for FSIZE (Hactive x Vactive for HV Active Size)" */
			.param.sync_hvact_md = 0,
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_SYNC_VIN_FSIZE_val, PI_BD_CONFIG_SYNC_VIN_FSIZE_val, sizeof(VIN_SYNC_VIN_FSIZE_CONFIG_U)*5);

	const VIN_SYNC_VIN_HVACT_CONFIG_U PI_BD_CONFIG_SYNC_VIN_HVACT_val[5] = { 
		{ //VIN0/*{{{*/ // 0xF0F0_0448
			/* "Sync H Active Size
			 * 720 for SD720, 960 for SD960, 1280 for HD, 1920 for FHD @ Bayer 10/12/16/20, and YUV Mode
			 * 720/2 for SD720, 960/2 for SD960, 1280/2 for HD, 1920/2 for FHD @ Bayer 6/7/8 Mode" */
			.param.sync_hact = DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION],
			/* "Sync V Active Size
			 * 240 for NTSC, 288 for PAL, 720 for HD, 1080 for FHD" */
			.param.sync_vact = DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION],
		},/*}}}*/
		{ //VIN1/*{{{*/ // 0xF0F0_0454
			/* "Sync H Active Size
			 * 720 for SD720, 960 for SD960, 1280 for HD, 1920 for FHD @ Bayer 10/12/16/20, and YUV Mode
			 * 720/2 for SD720, 960/2 for SD960, 1280/2 for HD, 1920/2 for FHD @ Bayer 6/7/8 Mode" */
			.param.sync_hact = DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION],
			/* "Sync V Active Size
			 * 240 for NTSC, 288 for PAL, 720 for HD, 1080 for FHD" */
			.param.sync_vact = DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION],
		},/*}}}*/
		{ //VIN2/*{{{*/ // 0xF0F0_0460
			/* "Sync H Active Size
			 * 720 for SD720, 960 for SD960, 1280 for HD, 1920 for FHD @ Bayer 10/12/16/20, and YUV Mode
			 * 720/2 for SD720, 960/2 for SD960, 1280/2 for HD, 1920/2 for FHD @ Bayer 6/7/8 Mode" */
			.param.sync_hact = DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION],
			/* "Sync V Active Size
			 * 240 for NTSC, 288 for PAL, 720 for HD, 1080 for FHD" */
			.param.sync_vact = DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION],
		},/*}}}*/
		{ //VIN3/*{{{*/ // 0xF0F0_046C
			/* "Sync H Active Size
			 * 720 for SD720, 960 for SD960, 1280 for HD, 1920 for FHD @ Bayer 10/12/16/20, and YUV Mode
			 * 720/2 for SD720, 960/2 for SD960, 1280/2 for HD, 1920/2 for FHD @ Bayer 6/7/8 Mode" */
			.param.sync_hact = DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION],
			/* "Sync V Active Size
			 * 240 for NTSC, 288 for PAL, 720 for HD, 1080 for FHD" */
			.param.sync_vact = DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION],
		},/*}}}*/
		{ //VIN4/*{{{*/ // 0xF0F0_0478
			/* "Sync H Active Size
			 * 720 for SD720, 960 for SD960, 1280 for HD, 1920 for FHD @ Bayer 10/12/16/20, and YUV Mode
			 * 720/2 for SD720, 960/2 for SD960, 1280/2 for HD, 1920/2 for FHD @ Bayer 6/7/8 Mode" */
			.param.sync_hact = DEF_VIN_HACTIVE_SIZE[0][BD_IN_RESOLUTION],
			/* "Sync V Active Size
			 * 240 for NTSC, 288 for PAL, 720 for HD, 1080 for FHD" */
			.param.sync_vact = DEF_VIN_VACTIVE_SIZE[0][BD_IN_RESOLUTION],
		},/*}}}*/
	};
	memcpy(pVinUserReg->PI_BD_CONFIG_SYNC_VIN_HVACT_val, PI_BD_CONFIG_SYNC_VIN_HVACT_val, sizeof(VIN_SYNC_VIN_HVACT_CONFIG_U)*5);

}
