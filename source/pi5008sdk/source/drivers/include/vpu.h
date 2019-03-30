#ifndef __VPU_H__
#define __VPU_H__

#include "type.h"
#include "osal.h"

#ifndef _VER_VPU
#define _VER_VPU "1_000"
#endif //_VER_VPU

/* BRIEF descriptor parameter */
#define BRIEF_PATCH_X_SIZE					24
#define BRIEF_PATCH_Y_SIZE					12
#define BRIEF_DESCRIPTOR_SIZE				128

#define VPU_MAX_FAST_LIMIT_COUNT	(1024) // <= 2^16
#define VPU_MAX_HDMATCH                   (1024)

#define VPU_FAST_UNIT_SIZE		(sizeof(uint32)) // 32bit(x|y)
#define VPU_FAST_ALIGN_UNITCNT		(4) // 128bit = 32bit(x|y) * 4

#define VPU_BRIEF_UNIT_SIZE		(sizeof(uint32)*4) // 128bit

#define VPU_FC_UNIT_SIZE		(sizeof(uint8)*2) // 16bit

/* VPU max size is 1280x720 */
#define VPU_MAX_HSIZE		(1280)
#define VPU_MAX_VSIZE		(720)

typedef enum 
{
    eDMA_2D = 0,
    eDMA_RLE = 2,
    eDMA_OTF = 7
}_eDMAMODE;

typedef enum 
{
    eDMA_MINOR_COPY = 0,
    eDMA_MINOR_EVENBYTE = 2, //0,2,4,...
    eDMA_MINOR_ODDBYTE = 3,  //1,3,5,...
}_eDMA_MINORMODE;

typedef enum 
{
    eRDMA_1D = 0,
    eRDMA_2D
}_eRDMATYPE;

typedef enum 
{
    eWDMA_1D = 0,
    eWDMA_2D
}_eWDMATYPE;

typedef enum 
{
    eOTF2DMA_DISABLE = 0,
    eOTF2DMA_SCLOUT = 1,
    eOTF2DMA_FILTER_FAST = 2,
    eOTF2DMA_FILTER_BRIEF = 3
}_eOTF2DMATYPE;

typedef enum 
{
    eIRQ_FAST = 0,
    eIRQ_DMA,
    eIRQ_HAMMINGD,
    eIRQ_MAX
}_eIRQTYPE;

enum tagVPU_CHANNEL_E
{
	eVPU_CHANNEL_RESERVED0 = 0,
	eVPU_CHANNEL_RESERVED1,
	eVPU_CHANNEL_RESERVED2,
	eVPU_CHANNEL_RESERVED3,
	eVPU_CHANNEL_SVM,
	eVPU_CHANNEL_QUAD,
	eVPU_CHANNEL_RESERVED4,
	eVPU_CHANNEL_VIN,
	eVPU_CHANNEL_MAX,
};

enum tagVPU_FRAME_E
{
	eVPU_FRAME_1ST = 0,
	eVPU_FRAME_2ND,
	eVPU_FRAME_3RD,
	eVPU_FRAME_4TH,
	eVPU_FRAME_MAX,
//	eVPU_FRAME_5TH,
//	eVPU_FRAME_6TH,
//	eVPU_FRAME_7TH,
};

enum tagVPU_ZONE_E
{
	eVPU_ZONE_0 = 0,
	eVPU_ZONE_1,
	eVPU_ZONE_2,
	eVPU_ZONE_3,
	eVPU_ZONE_4,
	eVPU_ZONE_5,
	eVPU_ZONE_6,
	eVPU_ZONE_7,
	eVPU_ZONE_8,
	eVPU_ZONE_9,
	eVPU_ZONE_10,
	eVPU_ZONE_11,
	eVPU_ZONE_12,
	eVPU_ZONE_13,
	eVPU_ZONE_14,
	eVPU_ZONE_15,
	eVPU_ZONE_MAX
};

typedef struct tagVPU_SIZE_T
{
	uint16 width;
	uint16 height;
} VPU_SIZE_T, *pVPU_SIZE_T;

typedef struct tagVPU_RECT_T
{
	uint16 x;
	uint16 y;
	uint16 width;
	uint16 height;
} VPU_RECT_T, *pVPU_RECT_T;

typedef struct tagVPU_FAST_PARAM_T
{
	uint8 number;
	uint8 threshold;
} VPU_FAST_PARAM_T, *pVPU_FAST_PARAM_T;

typedef struct tagVPU_BRIEF_LUT_VALUE_T
{
	uint8 x1;
	uint8 y1;
	uint8 x2;
	uint8 y2;
} VPU_BRIEF_LUT_VALUE_T, *pVPU_BRIEF_LUT_VALUE_T;

typedef struct tagVPU_BRIEF_LUT_T
{
	VPU_BRIEF_LUT_VALUE_T lut[128];
} VPU_BRIEF_LUT_T, *pVPU_BRIEF_LUT_T;

typedef struct tagVPU_FAST_RESULT_POS_T
{
	uint16 y;
	uint16 x;
} VPU_FAST_RESULT_POS_T, *pVPU_FAST_RESULT_POS_T;

typedef struct tagVPU_BRIEF_RESULT_DESC_T
{
	uint32 descriptor[4];
} VPU_BRIEF_RESULT_DESC_T, *pVPU_BRIEF_RESULT_DESC_T;

typedef struct tagVPU_DISTANCE_RECT_T
{
	sint16 x_min;
	sint16 x_max;
	sint16 y_min;
	sint16 y_max;
} VPU_DISTANCE_RECT_T, *pVPU_DISTANCE_RECT_T;

typedef struct tagVPU_HAMMING_DISTANCE_RESULT_POS_T
{
	uint16 y2;
	uint16 x2;
	uint16 y1;
	uint16 x1;
} VPU_MATCHING_RESULT_POS_T, *pVPU_MATCHING_RESULT_POS_T;

#if 0	// 3.0
typedef struct tagVPU_ADDR_T
{
	struct {
		uint32 *addr;
		uint32 size;
	} fast[2];						// xy base address for FAST result

	struct {
		uint32 *addr;
		uint32 size;
	} brief[2];						// descriptor base address for BRIEF result

	struct {
		uint32 *addr;
		uint32 size;
	} srcHD;				// source base address for hamming distance
	
	struct {
		uint32 *addr;
		uint32 size;
	} dstHD;				// destination base address for hamming distance
} VPU_ADDR_T, *pVPU_ADDR_T;
#else	// 3.2
typedef union tagUSER_VPU_FB_CONFIG_U
{
	uint32		var;

	struct
	{
		uint32	reserved0		:	1,	// 0
			reserved1		:	3,	// 1-3
			fast_n			:	4,	// 4-7
			reserved2		:	8,	// 8-15
			ch_sel			:	3,	// 16-18
			use_5x3_nms		:	1,	// 19
			brief_enable		:	1,	// 20
			roi_enable		:	1,	// 21
			scl_enable		:	1,	// 22
			dma2otf_enable		:	1,	// 23
			reserved3		:	1,	// 24
			otf2dma_enable		:	2,	// 25-26
			reserved4		:   	3, 	// 27-29
			field_sel			:   2; 	// 30-31
	}fld;
} USER_VPU_FB_CONFIG_U, *pUSER_VPU_FB_CONFIG_U;


typedef union tagVPU_FB_CONFIG_U
{
	uint32		var;

	struct
	{
		uint32	enable				:	1,	// 0
				dummy				:	3,	// 1-3
				fast_n				:	4,	// 4-7
				reserved1			:	8,	// 8-15
				ch_sel				:	3,	// 16-18
				use_5x3_nms			:	1,	// 19
				brief_enable		:	1,	// 20
				roi_enable			:	1,	// 21
				scl_enable			:	1,	// 22
				dma2otf_enable		:	1,	// 23
				use_posedge_vsync	:	1,	// 24
				otf2dma_enable		:	2,	// 25-26
				reserved2			:   3, 	// 27-29
				field_sel			:   2; 	// 30-31
	}fld;
} VPU_FB_CONFIG_U, *pVPU_FB_CONFIG_U;

typedef union tagVPU_FB_CONFIG
{
	uint32		var;

	struct
	{
		uint32	enable				:	1,	// 0
				dummy				:	3,	// 1-3
				fast_n				:	4,	// 4-7
				reserved1			:	8,	// 8-15
				ch_sel				:	3,	// 16-18
				use_5x3_nms			:	1,	// 19
				brief_enable		:	1,	// 20
				roi_enable			:	1,	// 21
				scl_enable			:	1,	// 22
				dma2otf_enable		:	1,	// 23
				use_posedge_vsync	:	1,	// 24
				otf2dma_enable		:	2,	// 25-26
				reserved2			:   5; 	// 27-31
	}fld;
}REG_VPU_FB_CONFIG;


typedef struct tagVPU_OUTPUT_T
{
	uint32 runCount;
	uint32 pprevFrameIdx;
	uint32 prevFrameIdx;
	uint32 curFrameIdx;
	uint32 nextFrameIdx;
	uint32 limitFeatureCount;
	uint32 fast_size;
	uint32 *fast_addr[eVPU_FRAME_MAX][eVPU_ZONE_MAX];						// xy base address for FAST result
	uint32 fast_count[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
	uint32 brief_size;
	uint32 *brief_addr[eVPU_FRAME_MAX][eVPU_ZONE_MAX];						// descriptor base address for BRIEF result
	uint32 fc_size;
	uint32 *fc_addr[eVPU_FRAME_MAX][eVPU_ZONE_MAX];							// zone base counts for FAST result
	uint32 fc_count[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
	uint32 hd_size;
	uint32 *hd_addr;
	uint32 match_size;
	pVPU_MATCHING_RESULT_POS_T match_addr[eVPU_ZONE_MAX];									// destination base address for hamming distance match
	uint32 match_count[eVPU_ZONE_MAX];												
	uint32 *capture_addr[2];									
} VPU_OUTPUT_T, *pVPU_OUTPUT_T;
#endif
typedef struct
{
	bool IN isEnable;
	uint32 fastThreshold;
	uint32 fastFilterIndex; // 0:program0, 1:program1, 2:bypass, [12:prefix1 > 13:prefix2 > 14:prefix3 > 15:prefix4] > NoiseCancel
	uint32 briefLUTIndex; 	// 0:program1, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4
	uint32 briefFilterIndex; // 0:program0, 1:program1, 2:bypass, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4
	uint32 sx;
	uint32 sy;
	uint32 ex;
	uint32 ey;
	uint32 fc_height;
	uint32 hd_threshold;
	VPU_DISTANCE_RECT_T match_dist;
} VPU_ZONINFO_T, *pVPU_ZONEINFO_T;

extern VPU_FB_CONFIG_U gVPU_FB_config;
extern VPU_OUTPUT_T g_VPUOutput;
extern pVPU_OUTPUT_T g_pVPUOutput;
extern VPU_OUTPUT_T g_VPUOutputAC;
extern pVPU_OUTPUT_T g_pVPUOutputAC;
extern pVPU_ZONEINFO_T g_pVPUZoneInfo;
extern uint32 compare_count, run_count;

typedef struct
{
    int inxFrmCur;
    int inxFrmPrev;
    int inxFrmPPrev;
    int inxFrmLast;
    int frmDoneCount;
}_FrmInx;

typedef struct
{
    uint8 u8Initialized;
    uint32 verVPU;

    VPU_SIZE_T imageInSize;
    VPU_SIZE_T imageVpuSize;

    SemaphoreHandle_t lockFast;
    SemaphoreHandle_t lockHammingD;
    SemaphoreHandle_t lockDMA;

    uint32 u8RunIntcIntervalDMATime;
    uint32 u8RunIntcIntervalHDTime;
    uint32 u8RunIntcIntervalFBTime;

    int runCountFAST;

    uint32 u32BufFastZoneSize;
    uint32 u32BufFastSize;
    uint32 *ppBufFast[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 *ppVBufFast[eVPU_FRAME_MAX][eVPU_ZONE_MAX];

    uint32 u32BufBriefZoneSize;
    uint32 u32BufBriefSize;
    uint32 *ppBufBrief[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 *ppVBufBrief[eVPU_FRAME_MAX][eVPU_ZONE_MAX];

    uint32 u32BufFCZoneSize;
    uint32 u32BufFCSize;
    uint32 *ppBufFC[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 *ppVBufFC[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 u32FCEndValue[eVPU_FRAME_MAX][eVPU_ZONE_MAX];

    uint32 u32BufHDMatchSize;
    VPU_MATCHING_RESULT_POS_T *pBufHDMatchAddr[eVPU_ZONE_MAX];
    VPU_MATCHING_RESULT_POS_T *pVBufHDMatchAddr[eVPU_ZONE_MAX];
    uint32 u32HDMatchResultCount[eVPU_ZONE_MAX];												
    uint32 u32BufCaptureSize;
    uint32 *pBufCapture[1];
    uint32 *pVBufCapture[1];

    VPU_RECT_T roiFastSize;

    _FrmInx stFrmInx;
    VPU_ZONINFO_T *pVpuZoneParam;

    SemaphoreHandle_t waitFrmUpdate;

}_VPUConfig;

typedef struct
{
    uint32 u32BufFastZoneSize;
    uint32 u32BufFastSize;
    uint32 *ppBufFast[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 *ppVBufFast[eVPU_FRAME_MAX][eVPU_ZONE_MAX];

    uint32 u32BufBriefZoneSize;
    uint32 u32BufBriefSize;
    uint32 *ppBufBrief[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 *ppVBufBrief[eVPU_FRAME_MAX][eVPU_ZONE_MAX];

    uint32 u32BufFCZoneSize;
    uint32 u32BufFCSize;
    uint32 *ppBufFC[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 *ppVBufFC[eVPU_FRAME_MAX][eVPU_ZONE_MAX];
    uint32 u32FCEndValue[eVPU_FRAME_MAX][eVPU_ZONE_MAX];

    uint32 u32BufHDMatchSize;
    VPU_MATCHING_RESULT_POS_T *pBufHDMatchAddr[eVPU_ZONE_MAX];
    VPU_MATCHING_RESULT_POS_T *pVBufHDMatchAddr[eVPU_ZONE_MAX];
    uint32 u32HDMatchResultCount[eVPU_ZONE_MAX];												

    uint32 u32BufCaptureSize;
    uint32 *pBufCapture[2];
    uint32 *pVBufCapture[2];

    VPU_RECT_T roiFastSize;

    _FrmInx stFrmInx;
    VPU_ZONINFO_T *pVpuZoneParam;

}_VPUStatus;


typedef volatile struct											// 0xF1420000
{
	unsigned int 	enable 							: 1,		// CONFIG
			reserved1						: 3,
			fast_n							: 4,
			reserved2						: 8,
			ch_sel							: 3,
			use_5x3_nms						: 1,
			brief_enable					: 1,
			roi_enable						: 1,
			scl_enable						: 1,
			dma2otf_enable					: 1,
			use_vsync_posedge				: 1,
			otf2dma_enable					: 1,
			reserved3						: 2,
			manual_flush					: 1,
			auto_flush						: 1,
			reserved4						: 2;
	unsigned int	height							: 16,		// SIZE
			width							: 16;
	unsigned int	bw_fast							: 8,		// CONFIG1
			bw_brief						: 8,
			counter_limit					: 16;
	unsigned int	roi_start_y						: 16,		// ROI_START
			roi_start_x						: 16;
	unsigned int	roi_end_y						: 16,		// ROI_END
			roi_end_x						: 16;
	unsigned int	xy_base;									// XY_BASE
	unsigned int	desc_base;									// DESC_BASE
	unsigned int	fc_base;									// FC_BASE
	unsigned int	scl_height						: 16,		// SCL_SIZE
			scl_width						: 16;
	unsigned int	ver_ratio						: 16,		// SCL_RATIO
			hor_ratio						: 16;
	unsigned int	lut_zone0						: 4,		// ZONE2LUT0
			lut_zone1						: 4,
			lut_zone2						: 4,
			lut_zone3						: 4,
			lut_zone4               		: 4,
			lut_zone5               		: 4,
			lut_zone6               		: 4,
			lut_zone7               		: 4;
	unsigned int	lut_zone8						: 4,		// ZONE2LUT1
			lut_zone9						: 4,
			lut_zone10						: 4,
			lut_zone11						: 4,
			lut_zone12              		: 4,
			lut_zone13              		: 4,
			lut_zone14              		: 4,
			lut_zone15              		: 4;
	unsigned int	fast_th_zone0					: 8,		// ZONE2TH0
			fast_th_zone1					: 8,
			fast_th_zone2					: 8,
			fast_th_zone3					: 8;
	unsigned int	fast_th_zone4					: 8,		// ZONE2TH1
			fast_th_zone5					: 8,
			fast_th_zone6					: 8,
			fast_th_zone7					: 8;
	unsigned int	fast_th_zone8					: 8,		// ZONE2TH2
			fast_th_zone9					: 8,
			fast_th_zone10					: 8,
			fast_th_zone11					: 8;
	unsigned int	fast_th_zone12					: 8,		// ZONE2TH3
			fast_th_zone13					: 8,
			fast_th_zone14					: 8,
			fast_th_zone15					: 8;
	unsigned int	sox_zone0						: 8,		// ZONE0
			eox_zone0						: 8,
			soy_zone0						: 8,
			eoy_zone0						: 8;
	unsigned int	sox_zone1						: 8,		// ZONE1
			eox_zone1						: 8,
			soy_zone1						: 8,
			eoy_zone1						: 8;
	unsigned int	sox_zone2						: 8,		// ZONE2
			eox_zone2						: 8,
			soy_zone2						: 8,
			eoy_zone2						: 8;
	unsigned int	sox_zone3						: 8,		// ZONE3
			eox_zone3						: 8,
			soy_zone3						: 8,
			eoy_zone3						: 8;
	unsigned int	sox_zone4						: 8,		// ZONE4
			eox_zone4						: 8,
			soy_zone4						: 8,
			eoy_zone4						: 8;
	unsigned int	sox_zone5						: 8,		// ZONE5
			eox_zone5						: 8,
			soy_zone5						: 8,
			eoy_zone5						: 8;
	unsigned int	sox_zone6						: 8,		// ZONE6
			eox_zone6						: 8,
			soy_zone6						: 8,
			eoy_zone6						: 8;
	unsigned int	sox_zone7						: 8,		// ZONE7
			eox_zone7						: 8,
			soy_zone7						: 8,
			eoy_zone7						: 8;
	unsigned int	sox_zone8						: 8,		// ZONE8
			eox_zone8						: 8,
			soy_zone8						: 8,
			eoy_zone8						: 8;
	unsigned int	sox_zone9						: 8,		// ZONE9
			eox_zone9						: 8,
			soy_zone9						: 8,
			eoy_zone9						: 8;
	unsigned int	sox_zone10						: 8,		// ZONE10
			eox_zone10						: 8,
			soy_zone10						: 8,
			eoy_zone10						: 8;
	unsigned int	sox_zone11						: 8,		// ZONE11
			eox_zone11						: 8,
			soy_zone11						: 8,
			eoy_zone11						: 8;
	unsigned int	sox_zone12						: 8,		// ZONE12
			eox_zone12						: 8,
			soy_zone12						: 8,
			eoy_zone12						: 8;
	unsigned int	sox_zone13						: 8,		// ZONE13
			eox_zone13						: 8,
			soy_zone13						: 8,
			eoy_zone13						: 8;
	unsigned int	sox_zone14						: 8,		// ZONE14
			eox_zone14						: 8,
			soy_zone14						: 8,
			eoy_zone14						: 8;
	unsigned int	sox_zone15						: 8,		// ZONE15
			eox_zone15						: 8,
			soy_zone15						: 8,
			eoy_zone15						: 8;
	unsigned int	reserved5;              		
	unsigned int	pix_data						: 8,		// RO_STATE		0xF1420084
			pix_hsync						: 1,
			pix_vsync						: 1,
			reserved6						: 2,
			frame_busy						: 1,
			last							: 1,
			reserved7						: 18;
	unsigned int	reserved8;
	unsigned int	frame_counter;								// RO_FRAME_COUNTER	
	unsigned int	irq_done						: 1,		// RO_ERROR		0xF1420090
			irq_count_limit					: 1,
			irq_from_fast_ored				: 1,
			irq_pixel_blocked				: 1,
			irq_from_scl					: 1,
			irq_from_bus_request_pending	: 1,
			flag_not_fully_flushed			: 2,
			safe_enable_sync				: 1,
			reserved9						: 23;
	unsigned int	ro_irq_fast;
	unsigned int	ro_irq_count_limit;
	unsigned int	version;
	unsigned int	ro_active_count;
	unsigned int	ro_blank_cnt;
	unsigned int	ro_htotal_cnt;
	unsigned int	ro_vtotal_cnt;
	unsigned int	ro_active_int_count; 
	unsigned int	ro_blank_int_cnt;    
	unsigned int	ro_htotal_int_cnt;   
	unsigned int	ro_vtotal_int_cnt;   
	unsigned int	num_zone0						: 16,		// ZONE01		0xF14200C0
			num_zone1						: 16;
	unsigned int    num_zone2                       : 16, 		// ZONE23
			num_zone3                       : 16; 
	unsigned int    num_zone4                       : 16, 		// ZONE45
			num_zone5                       : 16;
	unsigned int    num_zone6                       : 16, 		// ZONE67
			num_zone7                       : 16;
	unsigned int    num_zone8                       : 16, 		// ZONE89 
			num_zone9                       : 16;                 
	unsigned int    num_zone10                      : 16,       // ZONE1011 
			num_zone11                      : 16;                 
	unsigned int    num_zone12                      : 16,       // ZONE1213 
			num_zone13                      : 16;                 
	unsigned int    num_zone14                      : 16,       // ZONE1415 
			num_zone15                      : 16;
	unsigned int	num_prev_zone0					: 16,		// ZONE01		0xF14200E0
			num_prev_zone1					: 16;
	unsigned int    num_prev_zone2                  : 16, 		// ZONE23
			num_prev_zone3                  : 16; 
	unsigned int    num_prev_zone4                  : 16, 		// ZONE45
			num_prev_zone5                  : 16;
	unsigned int    num_prev_zone6                  : 16, 		// ZONE67
			num_prev_zone7                  : 16;
	unsigned int    num_prev_zone8                  : 16, 		// ZONE89 
			num_prev_zone9                  : 16;                 
	unsigned int    num_prev_zone10                 : 16,       // ZONE1011 
			num_prev_zone11                 : 16;                 
	unsigned int    num_prev_zone12                 : 16,       // ZONE1213 
			num_prev_zone13                 : 16;                 
	unsigned int    num_prev_zone14                 : 16,       // ZONE1415 
			num_prev_zone15                 : 16;
} VPU_CONFIG_T;

typedef volatile struct											// 0xF1421000
{
	unsigned int    intc_mask;
	unsigned int    intc_clear;
	unsigned int    intc_negedge;
	unsigned int    reserved1;
	union
	{
		unsigned int 		var;
		struct
		{
			unsigned int	irq_done						: 1,		// INTC_STATE		
					irq_count_limit_over			: 1,
					irq_fast_error					: 1,
					irq_buffer_full					: 1,
					irq_scl_setting_error			: 1,
					irq_bus_error					: 1,
					irq_hd_done						: 1,
					irq_dma_done					: 1,
					reserved2						: 24;
		} bits;
	} state;
} VPU_INTC_T;

typedef volatile struct											// 0xF1421000
{
	unsigned int    intc_mask;
	unsigned int    intc_clear;
	unsigned int    intc_negedge;
	unsigned int    reserved1;
	union
	{
		unsigned int 		var;
		struct
		{
			unsigned int	irq_done						: 1,		// INTC_STATE		
					irq_count_limit_over			: 1,
					irq_fast_error					: 1,
					irq_buffer_full					: 1,
					irq_scl_setting_error			: 1,
					irq_bus_error					: 1,
					irq_hd_done						: 1,
					irq_dma_done					: 1,
					reserved2						: 24;
		} bits;
	} state;
} REG_VPU_INTC;


typedef volatile struct											// 0xF1422000
{
	unsigned int	coef0_x0y0						: 4,		// COEF0Y0
			coef0_x1y0						: 4,
			coef0_x2y0						: 4,
			coef0_x3y0						: 4,
			coef0_x4y0              		: 4,
			reserved1	              		: 12;
	unsigned int	coef0_x0y1						: 4,		// COEF0Y1
			coef0_x1y1						: 4,
			coef0_x2y1						: 4,
			coef0_x3y1						: 4,
			coef0_x4y1              		: 4,
			reserved2	              		: 12;
	unsigned int	coef0_x0y2						: 4,		// COEF0Y2
			coef0_x1y2						: 4,
			coef0_x2y2						: 4,
			coef0_x3y2						: 4,
			coef0_x4y2              		: 4,
			reserved3	              		: 12;
	unsigned int    coef0_shift                		: 4,       	// SHIFT10 
			coef1_shift                 	: 4,
			reserved4						: 24;
	unsigned int	coef1_x0y0						: 4,		// COEF1Y0
			coef1_x1y0						: 4,
			coef1_x2y0						: 4,
			coef1_x3y0						: 4,
			coef1_x4y0              		: 4,
			reserved5	              		: 12;
	unsigned int	coef1_x0y1						: 4,		// COEF1Y1
			coef1_x1y1						: 4,
			coef1_x2y1						: 4,
			coef1_x3y1						: 4,
			coef1_x4y1              		: 4,
			reserved6	              		: 12;
	unsigned int	coef1_x0y2						: 4,		// COEF1Y2
			coef1_x1y2						: 4,
			coef1_x2y2						: 4,
			coef1_x3y2						: 4,
			coef1_x4y2              		: 4,
			reserved7	              		: 12;
	unsigned int	coef_fast_zone0					: 4,		// COEF_FAST0
			coef_fast_zone1					: 4,
			coef_fast_zone2					: 4,
			coef_fast_zone3					: 4,
			coef_fast_zone4              	: 4,
			coef_fast_zone5              	: 4,
			coef_fast_zone6              	: 4,
			coef_fast_zone7              	: 4;
	unsigned int	coef_fast_zone8					: 4,		// COEF_FAST1
			coef_fast_zone9					: 4,
			coef_fast_zone10				: 4,
			coef_fast_zone11				: 4,
			coef_fast_zone12              	: 4,
			coef_fast_zone13              	: 4,
			coef_fast_zone14              	: 4,
			coef_fast_zone15              	: 4;
	unsigned int	coef_brief_zone0				: 4,		// COEF_BRIEF0
			coef_brief_zone1				: 4,
			coef_brief_zone2				: 4,
			coef_brief_zone3				: 4,
			coef_brief_zone4              	: 4,
			coef_brief_zone5              	: 4,
			coef_brief_zone6              	: 4,
			coef_brief_zone7              	: 4;
	unsigned int	coef_brief_zone8				: 4,		// COEF_BRIEF1
			coef_brief_zone9				: 4,
			coef_brief_zone10				: 4,
			coef_brief_zone11				: 4,
			coef_brief_zone12              	: 4,
			coef_brief_zone13              	: 4,
			coef_brief_zone14              	: 4,
			coef_brief_zone15              	: 4;
} VPU_FILTER_T;

typedef volatile struct
{
	unsigned int 	prev_desc_base;								// PREV_DESC_BASE	0xF1410000
	unsigned int 	prev_xy_base;								// PREV_XY_BASE
	unsigned int 	curr_desc_base;								// CURR_DESC_BASE
	unsigned int 	curr_xy_base;								// CURR_XY_BASE
	unsigned int 	prev_count						: 12,		// PREV_COUNT
			reserved1						: 20;		
	unsigned int 	curr_count						: 12,		// CURR_COUNT	
			reserved2						: 20;
	unsigned int 	ham_th							: 8,		// HAM_TH
			reserved3						: 24;
	union
	{
		struct
		{
			int 		x_th;
			int 		y_th;
		} var;
		struct
		{
			int 		x_th_to						: 16,		// MAX_X_TH
					x_th_from					: 16;
			int 		y_th_to						: 16,		// MAX_Y_TH
					y_th_from					: 16;
		} bits;
	} max_th;
	union
	{
		unsigned int 		var;
		struct
		{
			unsigned int	start							: 1,		// CTRL
					clear_interrupt					: 1,
					reserved8						: 30;
		} bits;
	} ctrl;
	unsigned int	result_base;								// RESULT_BASE
	union
	{
		struct
		{
			int 		x_th;
			int 		y_th;
		} var;
		struct
		{
			int 		x_th_to						: 16,		// MAX_X_TH
					x_th_from					: 16;
			int 		y_th_to						: 16,		// MAX_Y_TH
					y_th_from					: 16;
		} bits;
	} min_th;
	unsigned int	reserved13[3];
	unsigned int 	num_result						: 16,		// RO_RESULT_COUNT 0xF1410040
			reserved14						: 16;
	unsigned int 	busy							: 1,		// RO_BUSY
			reserved15						: 31;
} VPU_HAMMING_DIST_T;

typedef volatile struct
{
	unsigned int 	prev_desc_base;								// PREV_DESC_BASE	0xF1410000
	unsigned int 	prev_xy_base;								// PREV_XY_BASE
	unsigned int 	curr_desc_base;								// CURR_DESC_BASE
	unsigned int 	curr_xy_base;								// CURR_XY_BASE
	unsigned int 	prev_count						: 12,		// PREV_COUNT
			reserved1						: 20;		
	unsigned int 	curr_count						: 12,		// CURR_COUNT	
			reserved2						: 20;
	unsigned int 	ham_th							: 8,		// HAM_TH
			reserved3						: 24;
	union
	{
		struct
		{
			int 		x_th;
			int 		y_th;
		} var;
		struct
		{
			int 		x_th_to						: 16,		// MAX_X_TH
					x_th_from					: 16;
			int 		y_th_to						: 16,		// MAX_Y_TH
					y_th_from					: 16;
		} bits;
	} max_th;
	union
	{
		unsigned int 		var;
		struct
		{
			unsigned int	start							: 1,		// CTRL
					clear_interrupt					: 1,
					reserved8						: 30;
		} bits;
	} ctrl;
	unsigned int	result_base;								// RESULT_BASE
	union
	{
		struct
		{
			int 		x_th;
			int 		y_th;
		} var;
		struct
		{
			int 		x_th_to						: 16,		// MAX_X_TH
					x_th_from					: 16;
			int 		y_th_to						: 16,		// MAX_Y_TH
					y_th_from					: 16;
		} bits;
	} min_th;
	unsigned int	reserved13[3];
	unsigned int 	num_result						: 16,		// RO_RESULT_COUNT 0xF1410040
			reserved14						: 16;
	unsigned int 	busy							: 1,		// RO_BUSY
			reserved15						: 31;
} REG_VPU_HAMMING_DIST_T;

extern VPU_CONFIG_T *g_pVPU_CONFIG;
extern VPU_INTC_T *g_pVPU_INTC;
extern VPU_FILTER_T *g_pVPU_FILTER;
extern VPU_HAMMING_DIST_T *g_pVPU_HAMMING;

PP_U32 PPDRV_VPU_GetVer(void);
void PPDRV_VPU_DumpReg(void);
PP_RESULT_E PPDRV_VPU_FAST_Initialize(void);
PP_RESULT_E PPDRV_VPU_FAST_SetScale(VPU_SIZE_T inputSize, VPU_SIZE_T outputSize);
PP_RESULT_E PPDRV_VPU_FAST_SetInputSize(VPU_SIZE_T inputSize);
PP_RESULT_E PPDRV_VPU_FAST_SetROI(VPU_RECT_T roi);
PP_RESULT_E PPDRV_VPU_FAST_SetConfig(const USER_VPU_FB_CONFIG_U *pVPUFBConfig);
PP_RESULT_E PPDRV_VPU_FAST_GetConfig(USER_VPU_FB_CONFIG_U *pVPUFBConfig);
PP_RESULT_E PPDRV_VPU_FAST_SetZone(PP_U8 zoneNumber, VPU_ZONINFO_T *pZoneInfo);
PP_RESULT_E PPDRV_VPU_BRIEF_SetLut(VPU_BRIEF_LUT_VALUE_T *pBriefLut);
void PPDRV_VPU_BRIEF_SetProgramFilter(const PP_U8 *pCoef0Array, const PP_U8 *pCoef1Array);
void PPDRV_VPU_FAST_InitFrmInx(void);
void PPDRV_VPU_FAST_UpdateFrmInx(const PP_S32 inxFrmTotalCnt, _FrmInx *pFrmInx);
PP_RESULT_E PPDRV_VPU_FAST_Start(const PP_S32 runCount, const PP_U32 u32TimeOut);
PP_RESULT_E PPDRV_VPU_FAST_RunNext(const PP_U32 u32TimeOut);
PP_RESULT_E PPDRV_VPU_FAST_Stop(void);
PP_RESULT_E PPDRV_VPU_DMA_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut);
PP_RESULT_E PPDRV_VPU_DMA_1Dto2D_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U32 u32RdPAddr, const PP_U16 u16RdSize, const PP_U16 u16WrWidth, const PP_U16 u16WrHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut);
PP_RESULT_E PPDRV_VPU_DMA_2Dto1D_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U32 u32WrPAddr, const PP_U16 u16WrSize, const PP_U32 u32TimeOut);
PP_RESULT_E PPDRV_VPU_HAMMINGD_Start(_VPUStatus *pVPUStatus, const PP_U32 u32TimeOut);
PP_RESULT_E PPDRV_VPU_WaitFrmUpdate(PP_S32 timeOutMsec);
PP_RESULT_E PPDRV_VPU_SetConfig(const PP_S32 eVpuChannel, const VPU_SIZE_T imageInSize);
void PPDRV_VPU_UnsetConfig(void);
PP_RESULT_E PPDRV_VPU_GetStatus(_VPUStatus *pVPUStatus);
PP_RESULT_E PPDRV_VPU_Initialize(void);

#endif // __VPU_H__


