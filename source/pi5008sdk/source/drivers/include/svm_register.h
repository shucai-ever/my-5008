/**
 * \file
 *
 * \brief	SVM Register Map
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __SVMREGISTER_H__
#define __SVMREGISTER_H__

#include "type.h"

/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppSVMREG_BOUNDARY_LINE_AREAR_T
{
	PP_VU32 vu32X;									//0x000	// [10:0] TV2D_XST
															// [26:16] TV2D_WID_M1
															// [30] TV2D#_EN1
															// [31] TV2D#_EN0
	PP_VU32 vu32Y;									//0x004	// [10:0] TV2D_YST 
															// [26:16] TV2D_HEI_M1
} PP_SVMREG_BOUNDARY_LINE_AREAR_S;

typedef struct ppSVMREG_BOUNDARY_LINE_POS_S
{
	PP_VU32 vu32Upleft;								//0x010	// [10:0] TV2D_XUL#
															// [26:16] TV2D_YUL#
	PP_VU32 vu32Upright;							//0x014	// [10:0] TV2D_XUR#
															// [26:16] TV2D_YUR#
	PP_VU32 vu32Downleft;							//0x018	// [10:0] TV2D_XDL0
															// [26:16] TV2D_YDL#
	PP_VU32 vu32Downright;							//0x01c	// [10:0] TV2D_XDR#
															// [26:16] TV2D_YDR#
} PP_SVMREG_BOUNDARY_LINE_POS_S;

typedef struct ppSVMREG_BOUNDARY_LINE_S
{
	PP_SVMREG_BOUNDARY_LINE_AREAR_S stArea[2];		//0x000 ~ 0x00c
	PP_SVMREG_BOUNDARY_LINE_POS_S stPos[2];			//0x010 ~ 0x02c
} PP_SVMREG_BOUNDARY_LINE_S;

typedef struct ppSVMREG_CTRL_S
{
	PP_VU32 vu32Enable;								//0x000	// [0] SVM_EN
															// [1] READ_SYNC_MOD
															// [3] SVM_TESTMODE_EN : input test mode
															// [4] INTERLACE_EN
															// [5] INTERLACE_VBLANK_MODE
															// [6] SHADOW_OFF
															// [7] SVM_TESTMODE_SEL
															// [8] SVM_BIST_EN
															// [9] SVM_ANTI_ROLL_EN
															// [12] FIELD_SECLECT_ON
															// [13] FIELD_VALUE
															// [14] INTER2PROG
															// [15] INTER_NOFIL
															// [16] NTSC_EN
															// [31] AXI_CLEAR
	PP_VU32 vu32InSyncCtrl;							//0x004	// [15:0] OUT_LINE_DELAY
															// [23:16] PIP_LINE_DELAY
															// [31:24] VSYNC_INCR
	PP_VU32 vu32InputSize;							//0x008	// [10:0] SVM_HWIDTH
															// [26:16] SVM_VHEIGHT
	PP_VU32 vu32OutputSize;							//0x00c	// [10:0] SVM_HWIDTH_OUT
															// [26:16] SVM_VHEIGHT_OUT
	PP_VU32 vu32InBlank;							//0x010	// [15:0] SVM_HBLANK
															// [31:16] SVM_VBLANK
	PP_VU32 vu32Wmain;								//0x014	// [0] WMAIN_EN : Front
															// [1] WMAIN_WRAP_EN
															// [2] WMAIN_WRAP_INIT_EN
															// [7:4] WMAIN_BURST_CTRL
															// [8] WMAIN_EN1 : Left
															// [9] WMAIN_EN2 : Right
															// [10] WMAIN_EN3 : Back
	PP_VU32 vu32Wline0;								//0x018	// [0:12] C0_WMAIN_WLINE
															// [16:28] C1_WMAIN_WLINE
	PP_VU32 vu32Wline1;								//0x01c	// [0:12] C2_WMAIN_WLINE
															// [16:28] C3_WMAIN_WLINE
	PP_VU32 vu32OutSyncCtrl;						//0x020	// [15:0] OUT_LINE_DELAY4OUT
															// [23:16] PIP_LINE_DELAY4OUT
															// [24] READ_SYNC_MODE4OUT
	PP_VU32 vu32OutBlank;							//0x024	// [15:0] SVM_HBLANK4OUT
															// [31:16] SVM_VBLANK4OUT
	PP_VU32 vu32Rmain;								//0x028	// [0] RMAIN_EN : Front/Back
															// [1] out_resolution_clamp
															// [8] RMAIN_EN1 : Left/Right
															// [9] OUTPUT_HOLD
	PP_VU32 vu32Interrupt;							//0x02c	// [0] RMAIN_UNDERFLOW
															// [1] WMAIN_OVERFLOW
															// [2] AXI_W_RESP
															// [3] AXI_W_RESP
															// [4] OUT_READ_CONFLICT
															// [6:5] OUT_CURRENT_POSITION
															// [9:8] fb_odd
															// [11:10] fb_even
															// [31] INTERRUPT_EN
	PP_VU32 vu32CamWmainAddr[4];					//0x030	// [31:0] CAM#_WMAIN_START_ADDR
	PP_VU32 vu32FbLutAddr[2];						//0x040	// [31:0] LUT0_START_ADDR#
	PP_VU32 vu32LrLutAddr[2];						//0x048	// [31:0] LUT1_START_ADDR#
	PP_VU32 vu32ChCtrl;								//0x050	// [0] FB_bypss_en
															// [1] FB_bypass_sel
															// [2] LR_bypass_en
															// [3] LR_bypass_sel
															// [4] Quad_en
															// [8] hmirror_en
															// [9] vmirror_en
															// [10] hmirror_en1
															// [11] vmirror_en1
															// [12] hmirror_en2
															// [13] vmirror_en2
															// [14] hmirror_en3
															// [15] vmirror_en3
															// [23:16] lut_ratio. lut ratio for F/B LUT
															// [31:24] lut_ratio2. lut ratio for L/R LUT
	PP_VU32 vu32SamplingCnt;						//0x054	// [7:0] TXCNT
															// [15:8] TYCNT_1
															// [23:16] SOFFSET
															// [31:24] TOFFSET
	PP_VU32 vu32SamplingDistance;					//0x058	// [7:0] DSTW_1
															// [31:16] INCR
	PP_VU32 vu32SizeRatio;							//0x05c	// [9:0] hscl_main_org. (width/width_out) * 2^8
															// [25:16] vscl_main_org. (height/height_out) * 2^8
	PP_VU32 vu32SclDownHoldCnt[2];					//0x060	// [0] [10:0] no_scale_down_hold_cnt
															// [0] [26:16] 1/2_scale_down_hold_cnt
															// [1] [10:0] 1/4_scale_down_hold_cnt
															// [1] [26:16] 1/8_scale_down_hold_cnt
	PP_VU32 vu32ImgAddr;							//0x068	// [31:0] CAR_START_ADDR
	PP_VU32 vu32ImgX;								//0x06c	// [10:0] CAR_XST
															// [15] CAR_ALPHA[0]
															// [26:16] CAR_WID_M1
															// [27:31] CAR_ALPHA[5:1]
	PP_VU32 vu32ImgY;								//0x070	// [10:0] CAR_YST
															// [15] CAR_LAYER_MODE
															// [26:16] CAR_HEI_M1
															// [31] CAR_BLEND_EN
	PP_VU32 vu32ImgCtrl;							//0x074	// [23:0] CAR_MASK_COLOR
															// [30:24] CAR_BURST_CTRL
	PP_VU32 vu32MaskColor;							//0x078	// [23:0] MASK_COLOR
	PP_VU32 vu32BgColor;							//0x07c	// [23:0] BG_COLOR
	PP_VU32 vu32Section0X;							//0x080	// [10:0] SECT0_XST
															// [26:16] SECT0_WID_M1
	PP_VU32 vu32Section0Y;							//0x084	// [10:0] SECT0_YST
															// [26:16] SECT0_HEI_M1
															// [30] SECT_MODE
															// [31] SECT0_EN
	PP_VU32 vu32Section1X;							//0x088	// [10:0] SECT1_XST
															// [26:16] SECT1_WID_M1
	PP_VU32 vu32Section1Y;							//0x08c	// [10:0] SECT1_YST
															// [26:16] SECT1_HEI_M1
															// [31] SECT1_EN
	PP_VU32 vu32Roi[4];								//0x090	// [10:0] C#_START_LINE
															// [26:16] C#_END_LINE
															// [31] C#_AREA_EN
	PP_VU32 vu32Scale;								//0x0a0	// [1:0] cam0_hscl_down
															// [3:2] cam1_hscl_down
															// [5:4] cam2_hscl_down
															// [7:6] cam3_hscl_down
															// [9:8] cam0_vscl_down
															// [11:10] cam1_vscl_down
															// [13:12] cam2_vscl_down
															// [15:14] cam3_vscl_down
															// [18:16] CAM0_ANTI_TAP
															// [22:20] CAM1_ANTI_TAP
															// [26:24] CAM2_ANTI_TAP
															// [30:28] CAM3_ANTI_TAP
	PP_VU32 vu32Fsync;								//0x0a4	// [7:0] st2_latency
															// [8] field_polarity_inv
															// [9] LUT_INVALID_GEN
															// [10] ALPHA_BIT
															// [11] BCGRAD_BIT
															// [12] INVALID_GEN_SMALL
															// [13] ALPHA_OFFSET
															// [14] BCGRAD_OFFSET
															// [15] gain_mode
															// [17:16] CAM0_ANTI_VTAP
															// [19:18] CAM1_ANTI_VTAP
															// [21:20] CAM2_ANTI_VTAP
															// [23:22] CAM3_ANTI_VTAP
	PP_VU32 vu32Reserved_a8[2];						//0x0a8 ~ 0x0ac
	PP_VU32 vu32EdgeLutAddr;						//0x0b0	// LUT4EDGE_START_ADDR
	PP_VU32 vu32OutMode;							//0x0b4	// [1:0] out_mode
															// [2] lut_mode
															// [3] blend_en
															// [4] dynamic_blend_en
															// [5] bc_en
															// [6] bc_extract
															// [15:8] BC_ratio
															// [17:16] auto_morp_en
															// [20:18] morp_speed
															// [21] morp_dir
															// [22] morp_dir2
															// [23] edge_en
															// [24:31] fixed_gain
	PP_VU32 vu32BcLutAddr;							//0x0b8	// [31:0] LUT4BC_START_ADDR
	PP_VU32 vu32NoVideo;							//0x0bc	// [0] C0_NO_VIDEO
															// [1] C1_NO_VIDEO
															// [2] C2_NO_VIDEO
															// [3] C3_NO_VIDEO
															// [4] NO_VIDEO_OUT
															// [31:8] NO_VCOLOR : [23:16] Cr, [15:8] Cb, [7:0] Y
	PP_VU32 vu32LineCnt;							//0x0c0	// [10:0] CURRENT_LINE_COUNT
															// [28:16] GENERATING COUNT
	PP_VU32 vu32Reserved_c4[7];						//0x0c4 ~ 0x0dc
	PP_VU32 vu32BcLutAddrEven;						//0x0e0	// [31:0] LUT4BC_START_ADDR_even
	PP_VU32 vu32EdgeLutAddrEven;					//0x0e4	// [31:0] LUT4EDGE_START_ADDR_even
	PP_VU32 vu32Img2Addr;							//0x0e8	// [31:0] CAR2_START_ADDR
	PP_VU32 vu32Img2X;								//0x0ec	// [10:0] CAR2_XST
															// [15] CAR2_ALPHA[0]
															// [26:16] CAR2_WID_M1
															// [31:27] CAR2_ALPHA[5:1]
	PP_VU32 vu32Img2Y;								//0x0f0	// [10:0] CAR2_YST
															// [15] CAR2_LAYER_MODE
															// [26:16] CAR2_HEI_M1
															// [31] CAR2_BLEND_EN
	PP_VU32 vu32Img2Ctrl;							//0x0f4	// [23:0] CAR2_MASK_COLOR
															// [30:24] CAR2_BURST_CTRL
	PP_VU32 vu32FbY;								//0x0f8	// [15:0] FRONT_Y
															// [31:16] BACK_Y
	PP_VU32 vu32LrY;								//0x0fc	// [15:0] LEFT_Y
															// [31:16] RIGHT_Y
	PP_VU32 vu32Reserved_100[39];					//0x100 ~ 0x198
	PP_VU32 vu32OutAddr[2];							//0x19c	// [31:0] OUT_BASE_ADDR0
													//0x1a0	// [31:0] OUT_BASE_ADDR1
	PP_VU32 vu32OutCtrl;							//0x1a4	// [7:0] out_burst_length
															// [15:8] out_hold_time
															// [23:16] in_burst_length
															// [24:25] FB_num
	PP_VU32 vu32OutAddr2[2];						//0x1a8	// [31:0] OUT_BASE_ADDR2
													//0x1ac	// [31:0] OUT_BASE_ADDR3
	PP_VU32 vu32Lutoffset;							//0x1b0	// [10:0] XOFFSET
															// [26:16] YOFFSET
	PP_VU32 vu32Reserved_1b4[3];					//0x1b4 ~ 0x1bc
	PP_VU32 vu32Crc0[2];							//0x1c0	// [31:0] CRC0
													//0x1c4	// [63:32] CRC0
	PP_VU32 vu32Crc1[2];							//0x1c8	// [31:0] CRC1
													//0x1cc	// [63:32] CRC1
	PP_VU32 vu32Crc2;								//0x1d0	// [31:0] CRC2
	PP_VU32 vu32ImgAddrEven;						//0x1d4	// [31:0] CAR_START_ADDR_even
	PP_VU32 vu32Img2AddrEven;						//0x1d8	// [31:0] CAR2_START_ADDR_even
	PP_VU32 vu32Section2X;							//0x1dc	// [10:0] SECT2_XST
															// [26:16] SECT2_WID_M1
	PP_VU32 vu32Section2Y;							//0x1e0	// [10:0] SECT2_YST
															// [26:16] SECT2_HEI_M1
															// [31] SECT2_EN
	PP_VU32 vu32Section3X;							//0x1e4	// [10:0] SECT3_XST
															// [26:16] SECT3_WID_M1
	PP_VU32 vu32Section3Y;							//0x1e8	// [10:0] SECT3_YST
															// [26:16] SECT3_HEI_M1
															// [31] SECT3_EN
	PP_VU32 vu32FbLutAddrEven[2];					//0x1ec	// [31:0] LUT0_START_ADDR_even#
	PP_VU32 vu32LrLutAddrEven[2];					//0x1f4	// [31:0] LUT1_START_ADDR_even#

	PP_VU32 vu32Reserved_1fc;						//0x1fc
	
	PP_SVMREG_BOUNDARY_LINE_S stBoundaryLine[2];	//[0] 0x200 ~ 0x22c
													//[1] 0x230 ~ 0x25c
										
	PP_VU32 vu32Reserved_260[24];					//0x260 ~ 0x2bc
	
	PP_VU32 vu32AlphaMore[2];						//0x2c0	// [13:0] ALPHA#_TOTAL0
															// [29:16] ALPHA#_TOTAL1
															// [31] ALPHA#_MORE_EN
	PP_VU32 vu32BcgradMore;							//0x2c8	// [13:0] BCGRAD_TOTAL0
															// [29:16] BCGRAD_TOTAL1
															// [31] BCGRAD_MORE_EN
	PP_VU32 vu32Alpha0Addr[2];						//0x2cc	// [31:0] ALPHA0_BASE_ADDR#
	PP_VU32 vu32Alpha1Addr[2];						//0x2d4	// [31:0] ALPHA1_BASE_ADDR#
	PP_VU32 vu32BcgradAddr[2];						//0x2dc	// [31:0] BCGRAD_BASE_ADDR#
	PP_VU32 vu32AlphaMoreEven[2];					//0x2e4	// [13:0] ALPHA#_TOTAL0_even
															// [29:16] ALPHA#_TOTAL1_even
	PP_VU32 vu32BcgradMoreEven;						//0x2ec	// [13:0] BCGRAD_TOTAL0_even
															// [29:16] BCGRAD_TOTAL1_even
	PP_VU32 vu32Alpha0AddrEven[2];					//0x2f0	// [31:0] ALPHA0_BASE_ADDR#_even
	PP_VU32 vu32Alpha1AddrEven[2];					//0x2f8	// [31:0] ALPHA1_BASE_ADDR#_even
	PP_VU32 vu32BcgradAddrEven[2];					//0x300	// [31:0] BCGRAD_BASE_ADDR#_even
	PP_VU32 vu32WinFilter0;							//0x308	// [7:0] COEF00
															// [15:8] COEF01
															// [23:16]COEF02
															// [24] WIN_FILT_EN
	PP_VU32 vu32WinFilter1;							//0x30c	// [7:0] COEF10
															// [15:8] COEF11
															// [23:16]COEF12
	PP_VU32 vu32WinFilter2;							//0x310	// [7:0] COEF20
															// [15:8] COEF21
															// [23:16]COEF22
	PP_VU32 vu32Winoffset;							//0x314	// [5:0] HSYNC_OFFSET
															// [7] HSYNC_OFF_SIGN
															// [13:8] VSYNC_OFFSET
															// [15] VSYNC_OFF_SIGN
}PP_SVMREG_CTRL_S;

typedef union ppSVMREG_BC_STAT_U
{
	PP_VU32 vu32Stat[2][4][3];						// [FB, LR][FL,FR, BR, BL][R, G, B]

	struct
	{
		PP_VU32 vu32FlStatOfFB[3];					//0x100	// [0] [25:0] FL_R0
															// [1] [25:0] FL_G0
															// [2] [25:0] FL_B0
		PP_VU32 vu32FrStatOfFB[3];					//0x10c	// [0] [25:0] FR_R0
															// [1] [25:0] FR_G0
															// [2] [25:0] FR_B0
		PP_VU32 vu32BrStatOfFB[3];					//0x118	// [0] [25:0] BR_R0
															// [1] [25:0] BR_G0
															// [2] [25:0] BR_B0
		PP_VU32 vu32BlStatOfFB[3];					//0x124	// [0] [25:0] BL_R0
															// [1] [25:0] BL_G0
															// [2] [25:0] BL_B0
		PP_VU32 vu32FlStatOfLR[3];					//0x130	// [0] [25:0] FL_R1
															// [1] [25:0] FL_G1
															// [2] [25:0] FL_B1
		PP_VU32 vu32FrStatOfLR[3];					//0x13c	// [0] [25:0] FR_R1
															// [1] [25:0] FR_G1
															// [2] [25:0] FR_B1
		PP_VU32 vu32BrStatOfLR[3];					//0x148	// [0] [25:0] BR_R1
															// [1] [25:0] BR_G1
															// [2] [25:0] BR_B1
		PP_VU32 vu32BlStatOfLR[3];					//0x154	// [0] [25:0] BL_R1
															// [1] [25:0] BL_G1
															// [2] [25:0] BL_B1
	} fld;
} PP_SVMREG_BC_STAT_U;

typedef struct ppSVMREG_BC_COEF_S
{
	PP_VU32 vu32Coef1[3];							//0x160	// [0] [7:0] COEF1_FL4R
															// [0] [15:8] COEF1_FR4R
															// [0] [23:16] COEF1_BR4R
															// [0] [31:24] COEF1_BL4R
															// [1] [7:0] COEF1_FL4G
															// [1] [15:8] COEF1_FR4G
															// [1] [23:16] COEF1_BR4G
															// [1] [31:24] COEF1_BL4G
															// [2] [7:0] COEF1_FL4B
															// [2] [15:8] COEF1_FR4B
															// [2] [23:16] COEF1_BR4B
															// [2] [31:24] COEF1_BL4B
	PP_VU32 vu32Coef3[3];							//0x16c	// [0] [7:0] COEF3_FL4R
															// [0] [15:8] COEF3_FR4R
															// [0] [23:16] COEF3_BR4R
															// [0] [31:24] COEF3_BL4R
															// [1] [7:0] COEF3_FL4G
															// [1] [15:8] COEF3_FR4G
															// [1] [23:16] COEF3_BR4G
															// [1] [31:24] COEF3_BL4G
															// [2] [7:0] COEF3_FL4B
															// [2] [15:8] COEF3_FR4B
															// [2] [23:16] COEF3_BR4B
															// [2] [31:24] COEF3_BL4B
	PP_VU32 vu32Coef2n4[3];							//0x178	// [0] [7:0] COEF2_F4R
															// [0] [15:8] COEF2_B4R
															// [0] [23:16] COEF4_L4R
															// [0] [31:24] COEF4_R4R
															// [1] [7:0] COEF2_F4G
															// [1] [15:8] COEF2_B4G
															// [1] [23:16] COEF4_L4G
															// [1] [31:24] COEF4_R4G
															// [2] [7:0] COEF2_F4B
															// [2] [15:8] COEF2_B4B
															// [2] [23:16] COEF4_L4B
															// [2] [31:24] COEF4_R4B
} PP_SVMREG_BC_COEF_S;

typedef struct ppSVMREG_COLORCOEF_S
{
	PP_VU32 vu32R2y4y;								//0x184	// [7:0] coefr4y
															// [15:8] coefg4y
															// [23:16] coefb4y
															// [24] with128
	PP_VU32 vu32R2y4cb;								//0x188	// [7:0] coefr4cb
															// [15:8] coefg4cb
															// [23:16] coefb4cb
	PP_VU32 vu32R2y4cr;								//0x18c	// [7:0] coefr4cr
															// [15:8] coefg4cr
															// [23:16] coefb4cr
	PP_VU32 vu32Y2r4g;								//0x190	// [7:0] coefcb4g
															// [15:8] coefcr4g
	PP_VU32 vu32Y2r4rnb;							//0x194	// [9:0] coefcr4r
															// [25:16] coefcb4b
} PP_SVMREG_COLORCOEF_S;

typedef union ppSVMREG_DYNAMIC_BLEND_U
{
	PP_VU32		vu32Var;

	struct
	{
		PP_VU32	vu8Coef1	: 8,
				vu8Coef3	: 8,
				vu8Coef6	: 8,
				vu8Coef8	: 8;
	}fld;
} PP_SVMREG_DYNAMIC_BLEND_U;


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/

#define SVMREG_CTRL				((PP_SVMREG_CTRL_S*)			(SVM_BASE_ADDR + 0x0000))
#define SVMREG_BC_STAT			((PP_SVMREG_BC_STAT_U*)			(SVM_BASE_ADDR + 0x0100))
#define SVMREG_BC_COEF			((PP_SVMREG_BC_COEF_S*)			(SVM_BASE_ADDR + 0x0160))
#define SVMREG_COLOR_COEF		((PP_SVMREG_COLORCOEF_S*)		(SVM_BASE_ADDR + 0x0184))
#define SVMREG_DYNAMIC_BLEND	((PP_SVMREG_DYNAMIC_BLEND_U*)	(SVM_BASE_ADDR + 0x0198))

#endif //__SVMREGISTER_H__
