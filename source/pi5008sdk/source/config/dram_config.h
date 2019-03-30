/*
 * dram_reserved.h
 *
 *  Created on: 2017. 12. 15.
 *      Author: ihkong
 */

#ifndef _DRAM_RESERVED_H_
#define _DRAM_RESERVED_H_

#include "board_config.h"
#include "vpu.h"
#include "api_svm_mem.h"

typedef struct 
{
    PP_U32 u32AllocStartAddr;
    PP_U32 u32AllocatedSize;

    PP_U32 u32AddrSVM;
    PP_U32 u32SizeSVM;
	PP_U32 u32SvmInBuff[4];
	PP_U32 u32SvmOutBuff[SVMMEM_OUT_BUFFER_COUNT];
	PP_U32 u32SvmData[SVMMEM_VIEW_PAGE_FLIPPING_COUNT];
	PP_U32 **u32SvmCommonSectionData;
	PP_U32 *u32SVMSectionMergeData;

	PP_U32 u32AddrDisplay;
	PP_U32 u32SizeDisplay;

    PP_U32 u32AddrQUAD;
    PP_U32 u32SizeQUAD;
    PP_U32 u32QuadRsvCh[4];

    PP_U32 u32AddrVPU;
    PP_U32 u32SizeVPU;
    PP_U32 u32CaptureRsvBuff[1];
    PP_U32 u32VpuBufFast[eVPU_FRAME_MAX];
    PP_U32 u32VpuBufBrief[eVPU_FRAME_MAX];
    PP_U32 u32VpuBufFC[eVPU_FRAME_MAX];
    PP_U32 u32VpuBufHDMatch[eVPU_ZONE_MAX];

	PP_U32 *u32SvmCacheData;
	PP_U32 u32SizeSvmCacheData;

	PP_U32 *u32AddrSwingCar;
	PP_U32 u32SizeSwingCar;

	PP_U32 u32AddrCalibrationLib;
	PP_U32 u32SizeCalibrationLib;

}_gstDramReserved;

#define ALIGN_128BIT(x)	        (((x)+15)&0xFFFFFFF0) // 128bit(16Byte) align
#define ALIGN_256BYTE(x)	    (((x)+255)&0xFFFFFF00) // 256Byte align

//////////////////// SVM reserved /////////////////////////////////////
#if	  ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
		#define SVM_RESERVED_IN_BUFFERSIZE		(ALIGN_128BIT(2*720*288*SVMMEM_IN_BUFFER_COUNT))
#elif ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
		#define SVM_RESERVED_IN_BUFFERSIZE		(ALIGN_128BIT(2*960*288*SVMMEM_IN_BUFFER_COUNT))
#elif ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
		#define SVM_RESERVED_IN_BUFFERSIZE		(ALIGN_128BIT(2*1280*720*SVMMEM_IN_BUFFER_COUNT))
#elif ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
		#define SVM_RESERVED_IN_BUFFERSIZE		(ALIGN_128BIT(2*1280*960*SVMMEM_IN_BUFFER_COUNT))
#elif ((BD_SVM_IN_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
		#define SVM_RESERVED_IN_BUFFERSIZE		(ALIGN_128BIT(2*1920*1080*SVMMEM_IN_BUFFER_COUNT))
#else
		#define SVM_RESERVED_IN_BUFFERSIZE		(ALIGN_128BIT(2*1280*720*SVMMEM_IN_BUFFER_COUNT))
#endif

#if	  ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*720*288))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*960*288))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH720)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*720*576))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH960)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*960*576))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*1280*720))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*1280*960))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*1920*1080))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD800_480P)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*800*480))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1024_600P)
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*1024*600))
#else
		#define SVM_RESERVED_OUT_BUFFERSIZE		(ALIGN_128BIT(2*1280*720))
#endif
//////////////////////////////////////////////////////////////////////////////

//////////////////// Calibration reserved /////////////////////////////////////
#if	  ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*720*288))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*960*288))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH720)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*720*576))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH960)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*960*576))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*1280*720))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*1280*960))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*1920*1080))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD800_480P)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*800*480))
#elif ((BD_SVM_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1024_600P)
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*1024*600))
#else
		#define CB_RESERVED_LUTMERGE_BUFFERSIZE		(ALIGN_128BIT(4*1280*720))
#endif
//////////////////////////////////////////////////////////////////////////////

//////////////////// VIN_QUAD reserved /////////////////////////////////////
/* Quad 1ch size = ((16bit*Horizon*Vertical)/4(quad)*4(page)) */
#if	((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD720H)
        #define QUAD_RESERVED_CHSIZE	(((2*720*576)>>2)<<2) 
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SD960H)
        #define QUAD_RESERVED_CHSIZE	(((2*960*576)>>2)<<2)
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH720)
        #define QUAD_RESERVED_CHSIZE	(((2*720*576)>>2)<<2) 
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_SDH960)
        #define QUAD_RESERVED_CHSIZE	(((2*960*576)>>2)<<2)
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD720P)
        #define QUAD_RESERVED_CHSIZE	(((2*1280*720)>>2)<<2)
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD960P)
        #define QUAD_RESERVED_CHSIZE	(((2*1280*960)>>2)<<2)
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1080P)
        #define QUAD_RESERVED_CHSIZE	(((2*1920*1080)>>2)<<2)
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD800_480P)
        #define QUAD_RESERVED_CHSIZE	(((2*800*480)>>2)<<2)
#elif ((BD_QUAD_OUT_FMT & VID_RESOL_MASKBIT) == VID_RESOL_HD1024_600P)
        #define QUAD_RESERVED_CHSIZE	(((2*1024*600)>>2)<<2)
#else
        #define QUAD_RESERVED_CHSIZE	(((2*720*576)>>2)<<2)
#endif
#define QUAD_RESERVED_SIZE	    (ALIGN_128BIT((QUAD_RESERVED_CHSIZE)*4)) // (x 4ch) & 128bit align
//////////////////////////////////////////////////////////////////////////////

//////////////////// VPU reserved /////////////////////////////////////
#define VPU_RESERVED_BUFFSIZE	(ALIGN_128BIT(2*1920*1080)) //capture,..etc
#define VPU_BUF_FASTZONE_SIZE   (ALIGN_128BIT((((VPU_MAX_FAST_LIMIT_COUNT+(VPU_FAST_ALIGN_UNITCNT-1))/VPU_FAST_ALIGN_UNITCNT)*VPU_FAST_ALIGN_UNITCNT)*VPU_FAST_UNIT_SIZE))
#define VPU_BUF_FAST_SIZE       (ALIGN_128BIT(VPU_BUF_FASTZONE_SIZE*eVPU_ZONE_MAX))
#define VPU_BUF_BRIEFZONE_SIZE  (ALIGN_128BIT(VPU_MAX_FAST_LIMIT_COUNT*VPU_BRIEF_UNIT_SIZE))
#define VPU_BUF_BRIEF_SIZE      (ALIGN_128BIT(VPU_BUF_BRIEFZONE_SIZE*eVPU_ZONE_MAX))
#define VPU_BUF_FCZONE_SIZE     (ALIGN_128BIT(VPU_MAX_VSIZE*VPU_FC_UNIT_SIZE))
#define VPU_BUF_FC_SIZE         (ALIGN_128BIT(VPU_BUF_FCZONE_SIZE*eVPU_ZONE_MAX))
#define VPU_BUF_HDMATCHZONE_SIZE    (ALIGN_128BIT(VPU_MAX_HDMATCH*(sizeof(VPU_MATCHING_RESULT_POS_T))))
#define VPU_BUF_HDMATCH_SIZE    (ALIGN_128BIT(VPU_BUF_HDMATCHZONE_SIZE*eVPU_ZONE_MAX))

#define VPU_RESERVED_SIZE	    ((VPU_RESERVED_BUFFSIZE*1)+(VPU_BUF_FAST_SIZE*eVPU_FRAME_MAX)+(VPU_BUF_BRIEF_SIZE*eVPU_FRAME_MAX)+(VPU_BUF_FC_SIZE*eVPU_FRAME_MAX)+(VPU_BUF_HDMATCH_SIZE*eVPU_FRAME_MAX)) // 128bit align buff * 1
//////////////////////////////////////////////////////////////////////////////

//////////////////// DISP reserved /////////////////////////////////////
#define DISPLAY_RESERVED_BUFFSIZE		(11534336)	// 11MB
#define DISPLAY_RESERVED_SIZE			(ALIGN_128BIT(DISPLAY_RESERVED_BUFFSIZE))		// // 128bit align
//////////////////////////////////////////////////////////////////////////////

//////////////////// SWING CAR reserved /////////////////////////////////////
#define SWING_CAR_RESERVED_BUFFSIZE		(17406362)	// 16.6MB
#define SWING_CAR_RESERVED_SIZE			(ALIGN_128BIT(SWING_CAR_RESERVED_BUFFSIZE))		// // 128bit align
//////////////////////////////////////////////////////////////////////////////

//////////////////// Calibration LIB reserved /////////////////////////////////////
#define CALIB_RESERVED_BUFFSIZE	(32*1024*1024)	// 32MByte 32*1024*1024

//////////////////////////////////////////////////////////////////////////////

#endif /* _DRAM_RESERVED_H_ */
