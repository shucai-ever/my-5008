//#define OD_MONITOR
//#define MAX_FRAME_NUM 620
//#define MAX_FRAME_NUM 454


#define TOTAL_ZONE_NUM 8

/* Object Detecting parameter */
// Front
#define OD_ZONE01_START_X	24
#define OD_ZONE01_START_Y	72
#define OD_ZONE01_END_X		296
#define OD_ZONE01_END_Y		232

#define OD_ZONE02_START_X	328
#define OD_ZONE02_START_Y	72
#define OD_ZONE02_END_X		584
#define OD_ZONE02_END_Y		232

// Left
#define OD_ZONE03_START_X	688
#define OD_ZONE03_START_Y	40
#define OD_ZONE03_END_X		960
#define OD_ZONE03_END_Y		248

#define OD_ZONE04_START_X	960
#define OD_ZONE04_START_Y	40 
#define OD_ZONE04_END_X		1200
#define OD_ZONE04_END_Y		248

// Rear							
#define OD_ZONE05_START_X	24
#define OD_ZONE05_START_Y	432  // (72+360)
#define OD_ZONE05_END_X		296
#define OD_ZONE05_END_Y		592  // (232+360)

#define OD_ZONE06_START_X	328
#define OD_ZONE06_START_Y	432  // (72+360)
#define OD_ZONE06_END_X		584
#define OD_ZONE06_END_Y		592  // (232+360)

// Right							
#define OD_ZONE07_START_X	688
#define OD_ZONE07_START_Y	416
#define OD_ZONE07_END_X		960
#define OD_ZONE07_END_Y		616

#define OD_ZONE08_START_X	960
#define OD_ZONE08_START_Y	416
#define OD_ZONE08_END_X		1200
#define OD_ZONE08_END_Y		616


// dummy
#define OD_ZONE09_START_X	0xffff
#define OD_ZONE09_START_Y	0xffff
#define OD_ZONE09_END_X		0xffff
#define OD_ZONE09_END_Y		0xffff

#define OD_ZONE10_START_X	0xffff
#define OD_ZONE10_START_Y	0xffff
#define OD_ZONE10_END_X		0xffff
#define OD_ZONE10_END_Y		0xffff

#define OD_ZONE11_START_X	0xffff
#define OD_ZONE11_START_Y	0xffff
#define OD_ZONE11_END_X		0xffff
#define OD_ZONE11_END_Y		0xffff

#define OD_ZONE12_START_X	0xffff
#define OD_ZONE12_START_Y	0xffff
#define OD_ZONE12_END_X		0xffff
#define OD_ZONE12_END_Y		0xffff

#define OD_ZONE13_START_X	0xffff
#define OD_ZONE13_START_Y	0xffff
#define OD_ZONE13_END_X		0xffff
#define OD_ZONE13_END_Y		0xffff

#define OD_ZONE14_START_X	0xffff
#define OD_ZONE14_START_Y	0xffff
#define OD_ZONE14_END_X		0xffff
#define OD_ZONE14_END_Y		0xffff

#define OD_ZONE15_START_X	0xffff
#define OD_ZONE15_START_Y	0xffff
#define OD_ZONE15_END_X		0xffff
#define OD_ZONE15_END_Y		0xffff

#define OD_ZONE16_START_X	0xffff
#define OD_ZONE16_START_Y	0xffff
#define OD_ZONE16_END_X		0xffff
#define OD_ZONE16_END_Y		0xffff


#define OD_ZONE01_MIN_DIST		3
#define OD_ZONE02_MIN_DIST		3
#define OD_ZONE03_MIN_DIST		3
#define OD_ZONE04_MIN_DIST		3
#define OD_ZONE05_MIN_DIST		3
#define OD_ZONE06_MIN_DIST		3
#define OD_ZONE07_MIN_DIST		3
#define OD_ZONE08_MIN_DIST		3
#define OD_ZONE09_MIN_DIST		3
#define OD_ZONE10_MIN_DIST		3
#define OD_ZONE11_MIN_DIST		3
#define OD_ZONE12_MIN_DIST		3
#define OD_ZONE13_MIN_DIST		3
#define OD_ZONE14_MIN_DIST		3
#define OD_ZONE15_MIN_DIST		3
#define OD_ZONE16_MIN_DIST		3

#define OD_ZONE01_MAX_DIST		50
#define OD_ZONE02_MAX_DIST		30
#define OD_ZONE03_MAX_DIST		30
#define OD_ZONE04_MAX_DIST		50
#define OD_ZONE05_MAX_DIST		40
#define OD_ZONE06_MAX_DIST		50
#define OD_ZONE07_MAX_DIST		50
#define OD_ZONE08_MAX_DIST		40
#define OD_ZONE09_MAX_DIST		50
#define OD_ZONE10_MAX_DIST		30
#define OD_ZONE11_MAX_DIST		30
#define OD_ZONE12_MAX_DIST		50
#define OD_ZONE13_MAX_DIST		40
#define OD_ZONE14_MAX_DIST		50
#define OD_ZONE15_MAX_DIST		50
#define OD_ZONE16_MAX_DIST		40

#define OD_ZONE01_ANGLE1		195
#define OD_ZONE02_ANGLE1		300
#define OD_ZONE03_ANGLE1		20 
#define OD_ZONE04_ANGLE1		160
#define OD_ZONE05_ANGLE1		195
#define OD_ZONE06_ANGLE1		300
#define OD_ZONE07_ANGLE1     	20 
#define OD_ZONE08_ANGLE1		160

// dummy
#define OD_ZONE09_ANGLE1		20 
#define OD_ZONE10_ANGLE1		160
#define OD_ZONE11_ANGLE1    	20 
#define OD_ZONE12_ANGLE1		160
#define OD_ZONE13_ANGLE1		20 
#define OD_ZONE14_ANGLE1		160
#define OD_ZONE15_ANGLE1    	20 
#define OD_ZONE16_ANGLE1		160

#define OD_ZONE01_ANGLE2		240
#define OD_ZONE02_ANGLE2		345
#define OD_ZONE03_ANGLE2		340
#define OD_ZONE04_ANGLE2		200
#define OD_ZONE05_ANGLE2		240
#define OD_ZONE06_ANGLE2		345
#define OD_ZONE07_ANGLE2     	340
#define OD_ZONE08_ANGLE2		200

// dummy
#define OD_ZONE09_ANGLE2		340
#define OD_ZONE10_ANGLE2		200
#define OD_ZONE11_ANGLE2    	340
#define OD_ZONE12_ANGLE2		200
#define OD_ZONE13_ANGLE2		340
#define OD_ZONE14_ANGLE2		200
#define OD_ZONE15_ANGLE2    	340
#define OD_ZONE16_ANGLE2		200


// fast_n
#define FAST_DETEC_NUM			10   

// fast_threshold
#define FAST_ZONE01_THRESH		50
#define FAST_ZONE02_THRESH		50
#define FAST_ZONE03_THRESH		50
#define FAST_ZONE04_THRESH		50
#define FAST_ZONE05_THRESH		50
#define FAST_ZONE06_THRESH		50
#define FAST_ZONE07_THRESH		50
#define FAST_ZONE08_THRESH		50
#define FAST_ZONE09_THRESH		50
#define FAST_ZONE10_THRESH		50
#define FAST_ZONE11_THRESH		50
#define FAST_ZONE12_THRESH		50
#define FAST_ZONE13_THRESH		50
#define FAST_ZONE14_THRESH		50
#define FAST_ZONE15_THRESH		50
#define FAST_ZONE16_THRESH		50

// hamming accelerator MAX_X, MAX_Y
#define MATCHING_XMINDIST_ZONE01_THRESH -50
#define MATCHING_XMAXDIST_ZONE01_THRESH 50
#define MATCHING_YMINDIST_ZONE01_THRESH -50
#define MATCHING_YMAXDIST_ZONE01_THRESH 50

#define MATCHING_XMINDIST_ZONE02_THRESH -50
#define MATCHING_XMAXDIST_ZONE02_THRESH 50
#define MATCHING_YMINDIST_ZONE02_THRESH -50
#define MATCHING_YMAXDIST_ZONE02_THRESH 50

#define MATCHING_XMINDIST_ZONE03_THRESH -50
#define MATCHING_XMAXDIST_ZONE03_THRESH 50
#define MATCHING_YMINDIST_ZONE03_THRESH -50
#define MATCHING_YMAXDIST_ZONE03_THRESH 50

#define MATCHING_XMINDIST_ZONE04_THRESH -50
#define MATCHING_XMAXDIST_ZONE04_THRESH 50
#define MATCHING_YMINDIST_ZONE04_THRESH -50
#define MATCHING_YMAXDIST_ZONE04_THRESH 50

#define MATCHING_XMINDIST_ZONE05_THRESH -50
#define MATCHING_XMAXDIST_ZONE05_THRESH	50
#define MATCHING_YMINDIST_ZONE05_THRESH	-50
#define MATCHING_YMAXDIST_ZONE05_THRESH	50

#define MATCHING_XMINDIST_ZONE06_THRESH -50
#define MATCHING_XMAXDIST_ZONE06_THRESH	50
#define MATCHING_YMINDIST_ZONE06_THRESH	-50
#define MATCHING_YMAXDIST_ZONE06_THRESH	50

#define MATCHING_XMINDIST_ZONE07_THRESH -50
#define MATCHING_XMAXDIST_ZONE07_THRESH	50
#define MATCHING_YMINDIST_ZONE07_THRESH	-50
#define MATCHING_YMAXDIST_ZONE07_THRESH	50

#define MATCHING_XMINDIST_ZONE08_THRESH -50
#define MATCHING_XMAXDIST_ZONE08_THRESH	50
#define MATCHING_YMINDIST_ZONE08_THRESH	-50
#define MATCHING_YMAXDIST_ZONE08_THRESH	50

#define MATCHING_XMINDIST_ZONE09_THRESH -50
#define MATCHING_XMAXDIST_ZONE09_THRESH 50
#define MATCHING_YMINDIST_ZONE09_THRESH -50
#define MATCHING_YMAXDIST_ZONE09_THRESH 50

#define MATCHING_XMINDIST_ZONE10_THRESH -50
#define MATCHING_XMAXDIST_ZONE10_THRESH 50
#define MATCHING_YMINDIST_ZONE10_THRESH -50
#define MATCHING_YMAXDIST_ZONE10_THRESH 50

#define MATCHING_XMINDIST_ZONE11_THRESH -50
#define MATCHING_XMAXDIST_ZONE11_THRESH 50
#define MATCHING_YMINDIST_ZONE11_THRESH -50
#define MATCHING_YMAXDIST_ZONE11_THRESH 50

#define MATCHING_XMINDIST_ZONE12_THRESH -50
#define MATCHING_XMAXDIST_ZONE12_THRESH 50
#define MATCHING_YMINDIST_ZONE12_THRESH -50
#define MATCHING_YMAXDIST_ZONE12_THRESH 50

#define MATCHING_XMINDIST_ZONE13_THRESH	-50
#define MATCHING_XMAXDIST_ZONE13_THRESH 50
#define MATCHING_YMINDIST_ZONE13_THRESH	-50
#define MATCHING_YMAXDIST_ZONE13_THRESH	50

#define MATCHING_XMINDIST_ZONE14_THRESH	-50
#define MATCHING_XMAXDIST_ZONE14_THRESH 50
#define MATCHING_YMINDIST_ZONE14_THRESH	-50
#define MATCHING_YMAXDIST_ZONE14_THRESH	50

#define MATCHING_XMINDIST_ZONE15_THRESH	-50
#define MATCHING_XMAXDIST_ZONE15_THRESH 50
#define MATCHING_YMINDIST_ZONE15_THRESH	-50
#define MATCHING_YMAXDIST_ZONE15_THRESH	50

#define MATCHING_XMINDIST_ZONE16_THRESH	-50
#define MATCHING_XMAXDIST_ZONE16_THRESH 50
#define MATCHING_YMINDIST_ZONE16_THRESH	-50
#define MATCHING_YMAXDIST_ZONE16_THRESH	50

// hamming threshold
#define MATCHING_ZONE01_THRESH		18
#define MATCHING_ZONE02_THRESH		18
#define MATCHING_ZONE03_THRESH		18
#define MATCHING_ZONE04_THRESH		18
#define MATCHING_ZONE05_THRESH		18
#define MATCHING_ZONE06_THRESH		18
#define MATCHING_ZONE07_THRESH		18
#define MATCHING_ZONE08_THRESH		18
#define MATCHING_ZONE09_THRESH		18
#define MATCHING_ZONE10_THRESH		18
#define MATCHING_ZONE11_THRESH		18
#define MATCHING_ZONE12_THRESH		18
#define MATCHING_ZONE13_THRESH		18
#define MATCHING_ZONE14_THRESH		18
#define MATCHING_ZONE15_THRESH		18
#define MATCHING_ZONE16_THRESH		18


typedef struct Point_t_ /* Type definition for 2D Position */
{
	uint16_t x;		/* 2D Position X */
	uint16_t y;		/* 2D Position Y */	
} Point_t;

typedef struct OptFlowPairPos_t_ /* Type definition for Motion Position Pair */
{
	Point_t first;		/* first frame position */
	Point_t second;		/* second frame position */
} OptFlowPairPos_t;

typedef struct ObjectDetectParam_t_
{
	uint16_t     roiStartX;
	uint16_t     roiStartY;
	uint16_t     roiWidth;
	uint16_t     roiHeight;
	uint16_t     minAngNear;
	uint16_t     maxAngNear;
	uint16_t     minAngAway;
	uint16_t     maxAngAway;
} ObjectDetectParam_t;

typedef struct BoundBoxRect_t_ /* Type definition for Motion Position Pair */
{
	Point_t start;		/* bound box start position */
	Point_t end;		    /* bound box end position */
} BoundBoxRect_t;


void    ObjectDetector_InitParam(ObjectDetectParam_t * odParam);

/*
void    ObjectDetector_Version(uint32_t * odVer);
: bit[31:24] : 1st version No.
: bit[23:16] : 2nd version No.
: bit[15:0] : 3rd version No.
*/
void    ObjectDetector_Version(uint32_t * odVer);

#if 0
void	odDynamicBlend (IplImage * cvInput, uint8_t dynBlendEn, OptFlowPairPos_t ** inMotionPair, uint16_t * numMotion, ObjectDetectParam_t * odParam, uint8_t *blendCorner);
#else
void	odDynamicBlend (uint8_t dynBlendEn, OptFlowPairPos_t ** inMotionPair, uint16_t * numMotion, ObjectDetectParam_t * odParam, uint8_t *blendCorner);
#endif
