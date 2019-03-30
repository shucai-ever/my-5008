/**
 * \file
 *
 * \brief	SVM APIs
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#include <stdio.h>
#include <string.h>

#include "osal.h"
#include "pi5008.h"
#include "utils.h"
#include "debug.h"
#include "interrupt.h"
#include "proc.h"
#include "sub_intr.h"

#include "svm_register.h"
#include "svm_drv.h"


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#define SVM_MIN_WIDTH							(720)
#define SVM_MIN_HEIGHT							(240)
#define SVM_MAX_WIDTH							(1920)
#define SVM_MAX_HEIGHT							(1080)

#define SVM_SAMPLING_COUNT						(8)				//

#ifdef SVM_VECTOR_MSG
#define _REG(x, y)						PPDRV_SVM_SetReg((PP_U32)&x, (PP_U32)y)
#define _RET(x)							(x)
#else
#define _REG(x, y)						((*(PP_VU32*)(&x)) = (PP_U32)y)
	#if (LOG_BUILD_LEVEL != LOG_LVL_NONE)
	#define _RET(x)						PPDRV_SVM_RetMsg(x, __FUNCTION__, __LINE__)
	#else
	#define _RET(x)						(x)
	#endif
#endif

PP_RESULT_E PPDRV_SVM_PROC_ERR_IRQ(PP_S32 argc, const PP_CHAR **argv);
PP_RESULT_E PPDRV_SVM_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
enum {
	eSVMDRV_INTC_ERR_UNDERFLOW = 0,
	eSVMDRV_INTC_ERR_OVERFLOW,
	eSVMDRV_INTC_ERR_AXI_W_RESP_0,
	eSVMDRV_INTC_ERR_AXI_W_RESP_1,
	eSVMDRV_INTC_ERR_OUT_READ_CONFLICT,
	eSVMDRV_INTC_ERR_MAX,
};

typedef enum ppSVMDRV_HORIZONTALSCALE_E
{
	eSVMDRV_HSCALE_ONE = 0,											// not scale
	eSVMDRV_HSCALE_ONE_OVER_TWO,									// 1/2 scale
	eSVMDRV_HSCALE_ONE_OVER_FOUR,									// 1/4 scale
	eSVMDRV_HSCALE_ONE_OVER_EIGHT,									// 1/8 scale
	eSVMDRV_HSCALE_MAX,
} PP_SVMDRV_HORIZONTALSCALE_E;

typedef enum ppSVMDRV_VERTICALSCALE_E
{
	eSVMDRV_VSCALE_ONE = 0,												// not scale
	eSVMDRV_VSCALE_ONE_OVER_TWO,										// 1/2 scale
	eSVMDRV_VSCALE_ONE_OVER_FOUR,										// 1/4 scale
	eSVMDRV_VSCALE_MAX,
} PP_SVMDRV_VERTICALSCALE_E;

typedef enum ppSVMDRV_BOUNDARY_LINE_NUMBER_E	
{
	eSVMDRV_BOUNDARY_LINE_NUM_0	= 0,
	eSVMDRV_BOUNDARY_LINE_NUM_1,
	eSVMDRV_BOUNDARY_LINE_NUM_MAX,
} PP_SVMDRV_BOUNDARY_LINE_NUMBER_E;

typedef enum ppSVMDRV_EDGE_MODE_E
{
	eSVMDRV_EDGE_MODE_FIXED_GAIN = 0,
	eSVMDRV_EDGE_MODE_LUT,
	eSVMDRV_EDGE_MODE_MAX,
} PP_SVMDRV_EDGE_MODE_E;

typedef enum ppSVMDRV_BURST_LENGTH_E
{
	eSVMDRV_BURST_LENGTH_8	= 4,
	eSVMDRV_BURST_LENGTH_16,
	eSVMDRV_BURST_LENGTH_32,
	eSVMDRV_BURST_LENGTH_64,
	eSVMDRV_BURST_LENGTH_128,
	eSVMDRV_BURST_LENGTH_MAX,
} PP_SVMDRV_BURST_LENGTH_E;

typedef enum ppSVMDRV_LUT_RESOLUTION_E									// lut inot resolution type
{
	eSVMDRV_LUT_RES_INPUT = 0,											// LUT resolution is input resolution.
	eSVMDRV_LUT_RES_OUTPUT,												// LUT resolution is output resolution.
	eSVMDRV_LUT_RES_MAX,
} PP_SVMDRV_LUT_RESOLUTION_E;

typedef enum ppSVMDRV_BCLUT_BIT_TYPE_E									// bit count of bc lut
{
	eSVMDRV_BCLUT_5BIT,
	eSVMDRV_BCLUT_8BIT,													// default value
	eSVMDRV_BCLUT_BIT_MAX,
} PP_SVMDRV_BCLUT_BIT_TYPE_E;

/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppSVM_ROI_AREA_S
{
	PP_U16 u16StartYLine;
	PP_U16 u16EndYLine;
} PP_SVM_ROI_AREA_S;

typedef struct ppSVM_BOUNDARY_LINE_AREAR_S
{
	PP_RECT_S stWholeArea;												// area of boundary
	PP_RECT_S stInvalidArea;											// invalied area of boundary (ex. car area)
} PP_SVM_BOUNDARY_LINE_AREAR_S;

typedef struct ppSVM_BOUNDARY_LINE_POS_S
{
	PP_POS_S stUpLeft;													// up & left positon of boundary
	PP_POS_S stUpRight;													// up & right positon of boundary
	PP_POS_S stDownLeft;												// down & left positon of boundary
	PP_POS_S stDownRight;												// down & left positon of boundary
} PP_SVM_BOUNDARY_LINE_POS_S;

typedef struct ppSVM_BOUNDARY_LINE_S									// structur of top view boundary
{
	PP_SVM_BOUNDARY_LINE_AREAR_S stArea;								// whole area & invalid area

	PP_BOOL bEnable[2];													// Positions required two for dynamic blending.
	PP_SVM_BOUNDARY_LINE_POS_S stPos[eSVMDRV_BOUNDARY_LINE_NUM_MAX];
} PP_SVM_BOUNDARY_LINE_S;

typedef struct ppSVM_COLORCOEFR2Y_T									// coefficient for RGB to YCbCr
{
	PP_U8 u8R4y;
	PP_U8 u8G4y;
	PP_U8 u8B4y;

	PP_U8 u8R4cb;
	PP_U8 u8G4cb;
	PP_U8 u8B4cb;

	PP_U8 u8R4cr;
	PP_U8 u8G4cr;
	PP_U8 u8B4cr;
} PP_SVM_COLORCOEF_R2Y_S;

typedef struct ppSVM_COLORCOEF_Y2R_S									// coefficient for YCbCr to RGB
{
	PP_U16 u16Cr4r;
	PP_U8 u8Cr4g;

	PP_U8 u8Cb4g;
	PP_U16 u16Cb4b;
} PP_SVM_COLORCOEF_Y2R_S;

typedef struct ppSVM_COLORCOEF_S
{
	PP_BOOL bWith128;

	PP_SVM_COLORCOEF_R2Y_S stR2y;
	PP_SVM_COLORCOEF_Y2R_S stY2r;
} PP_SVM_COLORCOEF_S;

typedef struct ppSVMDRV_INOUT_S
{
	PP_U16 u16Width;
	PP_U16 u16Height;
	PP_U16 u16VBlank;
	PP_U16 u16HBlank;
	PP_U8 u8Framerate;
	PP_BOOL bInterlace;
} PP_SVMDRV_INOUT_S;

typedef struct ppSVMDRV_CTRL_S
{
	PP_U8 u8InInfoIndex;
	PP_U8 u8OutInfoIndex;
	PP_BOOL bFixedField;
	PP_BOOL bInit;
} PP_SVMDRV_CTRL_S;

typedef struct ppSVMDRV_UNDERFLOW_CTRL_S
{
	PP_U32 u32UnderflowCtrlCheckCnt;
	PP_U32 u32UnderflowCtrlCnt;
	PP_BOOL bUnderflowCtrlStatus;
	PPDRV_SVM_CALLBACK_UNDERFLOW pCb;
} PP_SVMDRV_UNDERFLOW_CTRL_S;


/***************************************************************************************************************************************************************
 * Global Variable
***************************************************************************************************************************************************************/
STATIC PP_U32 gu32SVM_errIntcCnt[eSVMDRV_INTC_ERR_MAX]	= {0};
struct proc_irq_struct stSVM_PROC_irqs[]				= { { .fn = PPDRV_SVM_PROC_ERR_IRQ, .irqNum = IRQ_SVM_VECTOR, .next = (PP_VOID*)0,}, };
struct proc_device_struct stPPDRV_SVM_PROC_DEVICEs[]	= { { .pName = "SVM", .fn = PPDRV_SVM_PROC_DEVICE, .next = (PP_VOID*)0,}, };

STATIC PPDRV_SVM_CALLBACK_VSYNC gpSVM_cbVsync = PP_NULL;
STATIC PPDRV_SVM_CALLBACK_ERROR gpSVM_cbError = PP_NULL;

STATIC PP_SVMREG_CTRL_S				*gpSVM_reg_ctrl		= SVMREG_CTRL;
STATIC PP_SVMREG_BC_STAT_U			*gpSVM_reg_stat		= SVMREG_BC_STAT;
STATIC PP_SVMREG_BC_COEF_S			*gpSVM_reg_coef		= SVMREG_BC_COEF;
STATIC PP_SVMREG_DYNAMIC_BLEND_U	*gpSVM_reg_dynamic	= SVMREG_DYNAMIC_BLEND;
STATIC PP_SVMREG_COLORCOEF_S		*gpSVM_reg_color	= SVMREG_COLOR_COEF;

STATIC PP_SVMDRV_UNDERFLOW_CTRL_S gstUnderflowCtrl = {0,};

STATIC PP_SVMDRV_INOUT_S gstInOutInfo[] = {
	/*vres_720x480i60*/		{720, 240, 22, 138, 60, PP_TRUE},			/* 13.5Mhz */
	/*vres_720x576i50*/		{720, 288, 24, 144, 50, PP_TRUE},			/* 13.5MHz */
	/*vres_960x480i60*/		{960, 240, 22, 184, 60, PP_TRUE},			/* 18MHz */
	/*vres_960x576i50*/		{960, 288, 24, 192, 50, PP_TRUE},			/* 18MHz */
	/*vres_720x480p60*/		{720, 480, 45, 138, 60, PP_FALSE},			/* 27MHz */
	/*vres_720x576p50*/		{720, 576, 49, 144, 50, PP_FALSE},			/* 27MHz */
	/*vres_960x480p60*/		{960, 480, 45, 184, 60, PP_FALSE},			/* 36MHz */
	/*vres_960x576p50*/		{960, 576, 49, 192, 50, PP_FALSE},			/* 36MHz */
	/*vres_1280x720p60*/	{1280, 720, 30, 370, 60, PP_FALSE},			/* 74.25MHz */
	/*vres_1280x720p50*/	{1280, 720, 30, 700, 50, PP_FALSE},			/* 74.25MHz */
	/*vres_1280x720p30*/	{1280, 720, 30, 370, 30, PP_FALSE},			/* 37.125MHz */
	/*vres_1280x720p25*/	{1280, 720, 30, 700, 25, PP_FALSE},			/* 37.125MHz */
	/*vres_1280x960p30*/	{1280, 960, 40, 1195, 30, PP_FALSE},		/* 74.25MHz */
	/*vres_1280x960p25*/	{1280, 960, 40, 1690, 25, PP_FALSE},		/* 74.25MHz */
	/*vres_1920x1080p30*/	{1920, 1080, 45, 280, 30, PP_FALSE},		/* 74.25MHz */
	/*vres_1920x1080p25*/	{1920, 1080, 45, 720, 25, PP_FALSE},		/* 74.25MHz */
	/*vres_800x480p60*/		{800, 480, 94, 278, 60, PP_FALSE},			/* 37.125MHz */
	/*vres_800x480p50*/		{800, 480, 94, 494, 50, PP_FALSE},			/* 37.125MHz */
	/*vres_1024x600p60*/	{1024, 600, 120, 226, 60, PP_FALSE},		/* 54MHz */
	/*vres_1024x600p50*/	{1024, 600, 120, 476, 50, PP_FALSE},		/* 54MHz */
};

STATIC PP_SVMDRV_CTRL_S gstSvmDrvCtrl = {0,};

/*======================================================================================================================
 = callback function
======================================================================================================================*/
PP_S32 PPDRV_SVM_ISR_VsynCB(PP_S32 argc, const PP_CHAR **argv)
{
	if ( gstUnderflowCtrl.u32UnderflowCtrlCheckCnt > 0 && gstUnderflowCtrl.pCb )
	{
		if ( utilGetBit(gpSVM_reg_ctrl->vu32Interrupt, 0) == 0x1 )
		{
			gpSVM_reg_ctrl->vu32Interrupt = utilClearBit(gpSVM_reg_ctrl->vu32Interrupt, 0);
			
			gstUnderflowCtrl.u32UnderflowCtrlCnt++;
			if ( gstUnderflowCtrl.u32UnderflowCtrlCnt == 0xFFFFFFFF )
				gstUnderflowCtrl.u32UnderflowCtrlCnt = gstUnderflowCtrl.u32UnderflowCtrlCheckCnt;
			
			if ( !gstUnderflowCtrl.bUnderflowCtrlStatus && gstUnderflowCtrl.u32UnderflowCtrlCnt >= gstUnderflowCtrl.u32UnderflowCtrlCheckCnt )
			{
				gstUnderflowCtrl.bUnderflowCtrlStatus = PP_TRUE;
				gstUnderflowCtrl.pCb(PP_TRUE);
			}
		}
		else
		{
			gstUnderflowCtrl.u32UnderflowCtrlCnt = 0;
			if ( gstUnderflowCtrl.bUnderflowCtrlStatus ) gstUnderflowCtrl.pCb(PP_FALSE);
			gstUnderflowCtrl.bUnderflowCtrlStatus = PP_FALSE;
		}
	}

	if ( gpSVM_cbVsync ) gpSVM_cbVsync();
	
	return 0;
}

ISR(svm_error_isr, num)
{
	PP_S32 i;
	PP_U32 u32Msk = (1 << num);
	PP_U32 u32Reg;
	PP_BOOL bErr[eSVMDRV_INTC_ERR_MAX];
	
	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~u32Msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(u32Msk, NDS32_SR_INT_PEND2);

	if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 0) )
	{
		u32Reg = gpSVM_reg_ctrl->vu32Interrupt;

		for ( i = eSVMDRV_INTC_ERR_UNDERFLOW; i < eSVMDRV_INTC_ERR_MAX; i++ )
		{
			bErr[i] = utilGetBit(u32Reg, i);
			if ( bErr[i] ) gu32SVM_errIntcCnt[i]++;
		}
		
		if ( gpSVM_cbError )
		{
			if ( bErr[eSVMDRV_INTC_ERR_AXI_W_RESP_0] || bErr[eSVMDRV_INTC_ERR_AXI_W_RESP_1] )
				gpSVM_cbError();
		}

		if ( bErr[eSVMDRV_INTC_ERR_UNDERFLOW] )
			LOG_CRITICAL_ISR("[SVMDRV] UNDERFLOW Error\n");
		
		if ( bErr[eSVMDRV_INTC_ERR_OVERFLOW] )
			LOG_CRITICAL_ISR("[SVMDRV] OVERFLOW Error\n");

		if ( bErr[eSVMDRV_INTC_ERR_AXI_W_RESP_0] )
			LOG_CRITICAL_ISR("[SVMDRV] AXI_W_RESP_0 Error\n");

		if ( bErr[eSVMDRV_INTC_ERR_AXI_W_RESP_1] )
			LOG_CRITICAL_ISR("[SVMDRV] AXI_W_RESP_1 Error\n");

		if ( bErr[eSVMDRV_INTC_ERR_OUT_READ_CONFLICT] )
			LOG_CRITICAL_ISR("[SVMDRV] CONFLICT Error\n");
	}

	if ( gstUnderflowCtrl.u32UnderflowCtrlCheckCnt > 0 && gstUnderflowCtrl.pCb )
		gpSVM_reg_ctrl->vu32Interrupt = utilPutBits(gpSVM_reg_ctrl->vu32Interrupt, 1, 4, 0x0);
	else
		gpSVM_reg_ctrl->vu32Interrupt = utilPutBits(gpSVM_reg_ctrl->vu32Interrupt, 0, 5, 0x0);
	
	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | u32Msk, NDS32_SR_INT_MASK2);
}


/*======================================================================================================================
 = PROC function
======================================================================================================================*/
PP_RESULT_E PPDRV_SVM_PROC_ERR_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PRINT_PROC_IRQ("SVM underflow", IRQ_SVM_VECTOR, gu32SVM_errIntcCnt[eSVMDRV_INTC_ERR_UNDERFLOW]);
	PRINT_PROC_IRQ("SVM overflow", IRQ_SVM_VECTOR, gu32SVM_errIntcCnt[eSVMDRV_INTC_ERR_OVERFLOW]);
	PRINT_PROC_IRQ("SVM axi_w_resp_0", IRQ_SVM_VECTOR, gu32SVM_errIntcCnt[eSVMDRV_INTC_ERR_AXI_W_RESP_0]);
	PRINT_PROC_IRQ("SVM axi_w_resp_0", IRQ_SVM_VECTOR, gu32SVM_errIntcCnt[eSVMDRV_INTC_ERR_AXI_W_RESP_1]);
	PRINT_PROC_IRQ("SVM out_read_conflict", IRQ_SVM_VECTOR, gu32SVM_errIntcCnt[eSVMDRV_INTC_ERR_OUT_READ_CONFLICT]);

	return eSUCCESS;
}

PP_RESULT_E PPDRV_SVM_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	PP_U8 u8ChCtrl, u8Outmode;
	PP_BOOL bUseLut = PP_FALSE;
	char szViewmode[64] = {0,};
	PP_U8 u8SamplingWidth, u8SamplingHeight;
	PP_U32 u32SamplingSize;
	
	if( (argc) && (strcmp(argv[0], stPPDRV_SVM_PROC_DEVICEs[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return eERROR_FAILURE;
	}

#if 0	
	u8SamplingWidth = ((PP_U8)(((float)utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) / 2.0f / 8) + 0.5f) * 2) + 1;
	if ( u8SamplingWidth % 2 != 0 ) u8SamplingWidth++;
	u8SamplingHeight = ((PP_U8)(((float)utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11) / 2.0f / 8) + 0.5f) * 2) + 1;
#else
	u8SamplingWidth = (PP_U8)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) / 8) + 1;
	if ( u8SamplingWidth % 2 != 0 ) u8SamplingWidth++;
	u8SamplingHeight = (PP_U8)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11) / 8) + 1;
#endif
	u32SamplingSize = u8SamplingWidth * u8SamplingHeight * 4;

	u8ChCtrl = (PP_U8)utilGetBits(gpSVM_reg_ctrl->vu32ChCtrl, 0, 5);
	u8Outmode = (PP_U8)utilClearBit(utilGetBits(gpSVM_reg_ctrl->vu32OutMode, 0, 7), 4);
	if ( u8ChCtrl == 0x5 && u8Outmode == 0x0 ) strcpy(szViewmode, "front bypass");
	else if ( u8ChCtrl == 0x5 && u8Outmode == 0x1 ) strcpy(szViewmode, "left bypass");
	else if ( u8ChCtrl == 0xd && u8Outmode == 0x1 ) strcpy(szViewmode, "right bypass");
	else if ( u8ChCtrl == 0x7 && u8Outmode == 0x0 ) strcpy(szViewmode, "rear bypass");
	else if ( u8ChCtrl == 0x10 && u8Outmode == 0x3 ) strcpy(szViewmode, "quad");
	else if ( u8ChCtrl == 0x0 && u8Outmode == 0x6F ) strcpy(szViewmode, "lut");
	else strcpy(szViewmode, "unknown");

	if ( u8ChCtrl == 0x0 && u8Outmode == 0x6F ) bUseLut = PP_TRUE;
	
	printf("\n%s Device Info -------------\n", stPPDRV_SVM_PROC_DEVICEs[0].pName);
	printf("  viersion : %x\n", PPDRV_SVM_CTRL_GetVersion());
	printf("  initialized: %s\n", gstSvmDrvCtrl.bInit?"success":"fail");
	printf("  excute: %s\n\n", utilGetBit(gpSVM_reg_ctrl->vu32Enable, 0)?"yes":"no");
	if ( gstSvmDrvCtrl.bInit && utilGetBit(gpSVM_reg_ctrl->vu32Enable, 0) )
	{
		printf("  ---------------------- Input ---------------------- \n");
		printf("  scan: %s\n", utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4)?"interlcae":"progressive");
		printf("  Resolution: %d x %d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11), (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11));
		printf("  hblank: %d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32InBlank, 0, 16));
		printf("  vblank: %d\n\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32InBlank, 16, 16));
		printf("\t\t\tch0\t\tch1\t\tch2\t\tch3\n");
		printf("  enable:\t\t%d\t\t%d\t\t%d\t\t%d\n", (PP_U32)utilGetBit(gpSVM_reg_ctrl->vu32Wmain, 0),
												 		(PP_U32)utilGetBit(gpSVM_reg_ctrl->vu32Wmain, 8),
												 		(PP_U32)utilGetBit(gpSVM_reg_ctrl->vu32Wmain, 9),
												 		(PP_U32)utilGetBit(gpSVM_reg_ctrl->vu32Wmain, 10));
		printf("  buf size:\t\t%d\t\t%d\t\t%d\t\t%d\n", (PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32Wline0, 0, 13) * 2),
														(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32Wline0, 16, 13) * 2),
														(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32Wline1, 0, 13) * 2),
														(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32Wline1, 16, 13) * 2));
		printf("  buf addr:\t\t0x%08X\t0x%08X\t0x%08X\t0x%08X\n", gpSVM_reg_ctrl->vu32CamWmainAddr[0],
																	gpSVM_reg_ctrl->vu32CamWmainAddr[1],
																	gpSVM_reg_ctrl->vu32CamWmainAddr[2],
																	gpSVM_reg_ctrl->vu32CamWmainAddr[3]);
		printf("  replace color:\t%s\t\t%s\t\t%s\t\t%s\n", utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, 0)?"on":"off",
														utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, 1)?"on":"off",
														utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, 2)?"on":"off",
														utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, 3)?"on":"off");

		printf("\n  ---------------------- Output ---------------------- \n");
		printf("  Resolution: %d x %d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32OutputSize, 0, 11), (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32OutputSize, 16, 11));
		printf("  hblank: %d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32OutBlank, 0, 16));
		printf("  vblank: %d\n\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32OutBlank, 16, 16));
		printf("  replace color: %s\n", (PP_U32)utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, 4)?"on":"off");
		printf("  hold: %s\n", (PP_U32)utilGetBit(gpSVM_reg_ctrl->vu32Rmain, 9)?"on":"off");
		printf("\t\t\tbuf0\t\tbuf1\t\tbuf2\t\tbuf3\n");
		printf("  enable:\t\t%d\t\t%d\t\t%d\t\t%d\n", PP_TRUE,
												 		PP_TRUE,
												 		utilGetBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2)?PP_TRUE: PP_FALSE,
												 		utilGetBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2)==2?PP_TRUE: PP_FALSE);
		printf("  buf size:\t\t%d\t\t%d\t\t%d\t\t%d\n", (PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11) * 2),
														(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11) * 2),
														(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2)?utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11) * 2:0),
														(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2)==2?utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 0, 11) * utilGetBits(gpSVM_reg_ctrl->vu32InputSize, 16, 11) * 2:0));
		printf("  buf addr:\t\t0x%08X\t0x%08X\t0x%08X\t0x%08X\n", gpSVM_reg_ctrl->vu32OutAddr[0],
																	gpSVM_reg_ctrl->vu32OutAddr[1],
																	utilGetBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2)?gpSVM_reg_ctrl->vu32OutAddr2[0]:0,
																	utilGetBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2)==2?gpSVM_reg_ctrl->vu32OutAddr2[1]:0);

		printf("\n  ---------------------- View ---------------------- \n");
		printf("  view mode: %s\n", szViewmode);
		printf("  bg color: Y(0x%x), Cb(0x%x), Cr(0x%x)\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BgColor, 0, 8),
														(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BgColor, 8, 8),
														(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BgColor, 16, 8));
		printf("  title: x(%d), y(%d)\n", (PP_U32)(utilGetBit(gpSVM_reg_ctrl->vu32Winoffset, 7)?utilGetBits(gpSVM_reg_ctrl->vu32Winoffset, 0, 6) * -1:utilGetBits(gpSVM_reg_ctrl->vu32Winoffset, 0, 6)),
									(PP_U32)(utilGetBit(gpSVM_reg_ctrl->vu32Winoffset, 15)?utilGetBits(gpSVM_reg_ctrl->vu32Winoffset, 8, 6) * -1:utilGetBits(gpSVM_reg_ctrl->vu32Winoffset, 8, 6)));
		printf("  edge enhancement: %s, value(%d)\n", utilGetBit(gpSVM_reg_ctrl->vu32OutMode, 23)?"on":"off", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32OutMode, 24, 8));
		printf("  dynamic blend: %s, fl_coef(0x%x), fr_coef(0x%x), bl_coef(0x%x), br(0x%x)\n", utilGetBit(gpSVM_reg_ctrl->vu32OutMode, 4)?"on":"off",
																					(PP_U32)utilGetBits(gpSVM_reg_dynamic->vu32Var, 0, 8),
																					(PP_U32)utilGetBits(gpSVM_reg_dynamic->vu32Var, 8, 8),
																					(PP_U32)utilGetBits(gpSVM_reg_dynamic->vu32Var, 16, 8),
																					(PP_U32)utilGetBits(gpSVM_reg_dynamic->vu32Var, 24, 8));
		
		if ( utilGetBit(gpSVM_reg_ctrl->vu32Section0Y, 31) || utilGetBit(gpSVM_reg_ctrl->vu32Section1Y, 31) ||
			utilGetBit(gpSVM_reg_ctrl->vu32Section2Y, 31) || utilGetBit(gpSVM_reg_ctrl->vu32Section3Y, 31) )
		{
			printf("  \t\tx\ty\twidth\theight\n");
			if ( utilGetBit(gpSVM_reg_ctrl->vu32Section0Y, 31) )
				printf("  section 0:\t%d\t%d\t%d\t%d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section0X, 0, 11), (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section0Y, 0, 11),
															(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section0X, 16, 11) + 1, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section0Y, 0, 11) + 1);

			if ( utilGetBit(gpSVM_reg_ctrl->vu32Section1Y, 31) )
				printf("  section 1:\t%d\t%d\t%d\t%d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section1X, 0, 11), (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section1Y, 0, 11),
															(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section1X, 16, 11) + 1, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section1Y, 0, 11) + 1); 

			if ( utilGetBit(gpSVM_reg_ctrl->vu32Section2Y, 31) )
				printf("  section 2:\t%d\t%d\t%d\t%d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section2X, 0, 11), (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section2Y, 0, 11),
															(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section2X, 16, 11) + 1, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section2Y, 0, 11) + 1); 

			if ( utilGetBit(gpSVM_reg_ctrl->vu32Section3Y, 31) )
				printf("  section 2:\t%d\t%d\t%d\t%d\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section3X, 0, 11), (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section3Y, 0, 11),
															(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section3X, 16, 11) + 1, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Section3Y, 0, 11) + 1);
		}

		if ( bUseLut )
		{
			printf("  \t\t\taddr0\t\taddr1\t\tsize\t\tmorphing ratio\n");
			printf("  fb odd lut:\t\t0x%08X\t0x%08X\t%d\t\t0x%X\n", gpSVM_reg_ctrl->vu32FbLutAddr[0], gpSVM_reg_ctrl->vu32FbLutAddr[1], u32SamplingSize, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ChCtrl, 16, 8));
			if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
				printf("  fb even lut:\t\t0x%08X\t0x%08X\t%d\t\t0x%X\n", gpSVM_reg_ctrl->vu32FbLutAddrEven[0], gpSVM_reg_ctrl->vu32FbLutAddrEven[1], u32SamplingSize, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ChCtrl, 16, 8));
			printf("  lr odd lut:\t\t0x%08X\t0x%08X\t%d\t\t0x%X\n", gpSVM_reg_ctrl->vu32LrLutAddr[0], gpSVM_reg_ctrl->vu32LrLutAddr[1], u32SamplingSize, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ChCtrl, 24, 8));
			if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
				printf("  lr even lut:\t\t0x%08X\t0x%08X\t%d\t\t0x%X\n", gpSVM_reg_ctrl->vu32LrLutAddrEven[0], gpSVM_reg_ctrl->vu32LrLutAddrEven[1], u32SamplingSize, (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ChCtrl, 24, 8));
			printf("  bc odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32BcLutAddr, u32SamplingSize);
			if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
				printf("  bc even lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32BcLutAddrEven, u32SamplingSize);
			
			if ( utilGetBit(gpSVM_reg_ctrl->vu32AlphaMore[0], 31) )
			{
				printf("  a0s0 odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha0Addr[0], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMore[0], 0, 14) * 64);
				if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
					printf("  a0s0 even lut:\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha0AddrEven[0], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMoreEven[0], 0, 14) * 64);
				printf("  a0s1 odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha0Addr[1], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMore[0], 16, 14) * 64);
				if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
					printf("  a0s1 even lut:\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha0AddrEven[1], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMoreEven[0], 16, 14) * 64);
			}
			if ( utilGetBit(gpSVM_reg_ctrl->vu32AlphaMore[1], 31) )
			{
				printf("  a1s0 odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha1Addr[0], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMore[1], 0, 14) * 64);
				if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
					printf("  a1s0 even lut:\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha1AddrEven[0], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMoreEven[1], 0, 14) * 64);
				printf("  a1s1 odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha1Addr[1], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMore[1], 16, 14) * 64);
				if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
					printf("  a1s1 even lut:\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32Alpha1AddrEven[1], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32AlphaMoreEven[1], 16, 14) * 64);
			}
			if ( utilGetBit(gpSVM_reg_ctrl->vu32BcgradMore, 31) )
			{
				printf("  grad odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32BcgradAddr[0], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BcgradMore, 0, 14) * 64);
				if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
					printf("  grad even lut:\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32BcgradAddrEven[0], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BcgradMoreEven, 0, 14) * 64);
				printf("  grad odd lut:\t\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32BcgradAddr[1], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BcgradMore, 16, 14) * 64);
				if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
					printf("  grad even lut:\t0x%08X\tX\t\t%d\t\tX\n", gpSVM_reg_ctrl->vu32BcgradAddrEven[1], (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32BcgradMoreEven, 16, 14) * 64);
			}
		}
	}

	if ( utilGetBit(gpSVM_reg_ctrl->vu32ImgY, 31) || utilGetBit(gpSVM_reg_ctrl->vu32Img2Y, 31) )
	{
		printf("\n  Img mask color: Y(0x%X), Cb(0x%X), Cr(0x%X)\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgCtrl, 16, 8),
															(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgCtrl, 8, 8),
															(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgCtrl, 0, 8));
		if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
			printf("  \t\tx\ty\twidth\theight\talpha\todd addr\t\teven addr\n");
		else
			printf("  \t\tx\ty\twidth\theight\talpha\taddr\n");
			
		if ( utilGetBit(gpSVM_reg_ctrl->vu32ImgY, 31) )
		{
			if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
			{
				printf("  img 0:\t%d\t%d\t%d\t%d\t%d\t0x%08X\t0x%08X\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgX, 0, 11),
																		(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 0, 11),
																		(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgX, 16, 11) + 1,
																		(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 16, 11) + 1,
																		(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 27, 5) << 1 | utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 15, 1)),
																		gpSVM_reg_ctrl->vu32ImgAddr, gpSVM_reg_ctrl->vu32ImgAddrEven);
			}
			else
			{
				printf("  img 0:\t%d\t%d\t%d\t%d\t%d\t0x%08X\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgX, 0, 11),
																	(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 0, 11),
																	(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgX, 16, 11) + 1,
																	(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 16, 11) + 1,
																	(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 27, 5) << 1 | utilGetBits(gpSVM_reg_ctrl->vu32ImgY, 15, 1)),
																	gpSVM_reg_ctrl->vu32ImgAddr);
			}
		}

		if ( utilGetBit(gpSVM_reg_ctrl->vu32Img2Y, 4) )
		{
			if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 4) )
			{
				printf("  img 1:\t%d\t%d\t%d\t%d\t%d\t0x%08X\t0x%08X\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2X, 0, 11),
																		(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 0, 11),
																		(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2X, 16, 11) + 1,
																		(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 16, 11) + 1,
																		(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 27, 5) << 1 | utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 15, 1)),
																		gpSVM_reg_ctrl->vu32Img2Addr, gpSVM_reg_ctrl->vu32Img2AddrEven);
			}
			else
			{
				printf("  img 1:\t%d\t%d\t%d\t%d\t%d\t0x%08X\n", (PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2X, 0, 11),
																	(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 0, 11),
																	(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2X, 16, 11) + 1,
																	(PP_U32)utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 16, 11) + 1,
																	(PP_U32)(utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 27, 5) << 1 | utilGetBits(gpSVM_reg_ctrl->vu32Img2Y, 15, 1)),
																	gpSVM_reg_ctrl->vu32Img2Addr);
			}										
		}
	}
	
	printf("\n");

	return eSUCCESS;
}

/*======================================================================================================================
 = Local function
======================================================================================================================*/

#if !defined(SVM_VECTOR_MSG) && (LOG_BUILD_LEVEL != LOG_LVL_NONE)
/*
 * function				:	PPDRV_SVM_RetMsg

 * param u32Ret			:	error code
 * param cFunction		:	function name
 * param u32Line		:	code line number

 * return				:	error code

 * brief
	debuging message.
*/
STATIC PP_RESULT_E PPDRV_SVM_RetMsg(PP_S32 u32Ret, const PP_CHAR* cFunction, PP_U32 u32Line)
{
	PP_CHAR cError[32];

	if ( u32Ret == eSUCCESS ) return u32Ret;

	switch (u32Ret)
	{
	case eERROR_NO_MEM:
		strcpy(cError, "eERROR_NO_MEM");
		break;
	case eERROR_INVALID_ARGUMENT:
		strcpy(cError, "eERROR_INVALID_ARGUMENT");
		break;
	case eERROR_SVM_NOT_INITIALIZE:
		strcpy(cError, "eERROR_SVM_NOT_INITIALIZE");
		break;
	case eERROR_SVM_VIEWMODE_NOT_SELECTED:
		strcpy(cError, "eERROR_SVM_VIEWMODE_NOT_SELECTED");
		break;
	case eERROR_SVM_LIMIT_VIEWMODE:
		strcpy(cError, "eERROR_SVM_LIMIT_VIEWMODE");
		break;
	case eERROR_SVM_NOT_CREATED_VIEWMODE:
		strcpy(cError, "eERROR_SVM_NOT_CREATED_VIEWMODE");
		break;
	case eERROR_SVM_UNUSE_LUT_VIEWMODE:
		strcpy(cError, "eERROR_SVM_UNUSE_LUT_VIEWMODE");
		break;
	case eERROR_SVM_UNUSE_IMG_VIEWMODE:
		strcpy(cError, "eERROR_SVM_UNUSE_IMG_VIEWMODE");
		break;
	case eERROR_SVM_RUNNING:
		strcpy(cError, "eERROR_SVM_RUNNING");
		break;
	case eERROR_SYS_DMA_ERROR:
		strcpy(cError, "eERROR_SYS_DMA_ERROR");
		break;
	case eERROR_SVM_MORPHING_RUNNING:
		strcpy(cError, "eERROR_SVM_MORPHING_RUNNING");
		break;
	case eERROR_SVM_INVALID_LUT:
		strcpy(cError, "eERROR_SVM_INVALID_LUT");
		break;
	case eERROR_SVM_RESOUTION_P2I:
		strcpy(cError, "eERROR_SVM_RESOUTION_P2I");
		break;
	case eERROR_INVALID_ALIGN:
		strcpy(cError, "eERROR_INVALID_ALIGN");
		break;
	default:
		break;
	}

	LOG_WARNING("[SVMDRV (%s_%d)] : %s\n", cFunction, u32Line, cError);

	return u32Ret;
}
#endif

#ifdef SVM_VECTOR_MSG
/*
 * function				:	PPDRV_SVM_SetReg

 * param ret			:	register address
 * param line			:	register value

 * return				:

 * brief
	sets register and vector message.
*/
PP_VOID PPDRV_SVM_SetReg(PP_U32 u32Addr, PP_U32 u32Value)
{
	PP_U32 u32AddrMsg = u32Addr - SVM_BASE_ADDR + 0x3000;
	PP_U32 u32ValueMsg = u32Value;
	switch (u32AddrMsg)
	{
	case 0x3030:
	case 0x3034:
	case 0x3038:
	case 0x303C:
	case 0x3040:
	case 0x3044:
	case 0x3048:
	case 0x304C:
	case 0x3068:
	case 0x30B8:
	case 0x30E8:
	case 0x319C:
	case 0x31A0:
		break;
	default:
		if ( u32AddrMsg == 0x3000 )
			u32ValueMsg = utilClearBit(u32ValueMsg, 2);
		printf("i2c_mst_byte_wr(7'h1e,32'h%x, 32'h%08x);\n", u32AddrMsg, u32ValueMsg);
		break;
	}

	(*((PP_VU32 *)(u32Addr))) = u32Value;
}
#endif

/*
 * function					:	PPDRV_SVM_SetInOutResolution

 * param u16InputWidth		:	input width
 * param u16InputHeight		:	input height
 * param u16OutputWidth		:	output width
 * param u16OutputHeight	:	output height

 * return					:	error code

 * brief
	Sets the size information of svm.
*/

STATIC PP_VOID PPDRV_SVM_SetInOutResolution(PP_U16 u16InputWidth, PP_U16 u16InputHeight, PP_U16 u16OutputWidth, PP_U16 u16OutputHeight)
{
	PP_U16 u16Hscl = 0x0, u16Vscl = 0x0;
	
	u16Hscl = (PP_U16)(((double)u16InputWidth / (double)u16OutputWidth) * 256.f);
	u16Vscl = (PP_U16)(((double)u16InputHeight / (double)u16OutputHeight) * 256.f);

	_REG(gpSVM_reg_ctrl->vu32SizeRatio, (u16Hscl & utilBitMask(10)) | ((u16Vscl & utilBitMask(10)) << 16));
}

/*
 * function					:	PPDRV_SVM_SetResolution

 * param u16InputWidth		:	input width
 * param u16InputHeight		:	input height
 * param u16OutputWidth		:	output width
 * param u16OutputHeight	:	output height

 * return					:

 * brief
	Sets the size information of svm.
*/

STATIC PP_VOID PPDRV_SVM_SetResolution(PP_U16 u16InputWidth, PP_U16 u16InputHeight, PP_U16 u16OutputWidth, PP_U16 u16OutputHeight)
{
	PP_U8 u8SOffset = 0x0, u8TOffset = 0x0;
	PP_U8 u8TXCnt = 0x0, u8TYCnt = 0x0;
	PP_U8 u8Dstw = 0x0;
	PP_U16 u16Incr = 0x0;

#if 1
	u8SOffset = 0;
	u8TOffset = 0;

	u8TXCnt = (u16OutputWidth / SVM_SAMPLING_COUNT ) + 1;
	u8TYCnt = (u16OutputHeight / SVM_SAMPLING_COUNT ) + 1;
#else
	u8SOffset = (u16OutputWidth / 2) % SVM_SAMPLING_COUNT;
	u8TOffset = (u16OutputHeight / 2) % SVM_SAMPLING_COUNT;

	u8TXCnt = ((PP_U8)(((float)u16OutputWidth / 2.0f / SVM_SAMPLING_COUNT) + 0.5f) * 2) + 1;
	u8TYCnt = ((PP_U8)(((float)u16OutputHeight / 2.0f / SVM_SAMPLING_COUNT) + 0.5f) * 2) + 1 - 1;
#endif

	u16Incr = 32768 / SVM_SAMPLING_COUNT;
	u8Dstw = SVM_SAMPLING_COUNT - 1;

	PPDRV_SVM_SetInOutResolution(u16InputWidth, u16InputHeight, u16OutputWidth, u16OutputHeight);

	_REG(gpSVM_reg_ctrl->vu32InputSize, (u16InputWidth & utilBitMask(11)) | ((u16InputHeight & utilBitMask(11)) << 16));
	_REG(gpSVM_reg_ctrl->vu32OutputSize, (u16OutputWidth & utilBitMask(11)) | ((u16OutputHeight & utilBitMask(11)) << 16));
	_REG(gpSVM_reg_ctrl->vu32SamplingCnt, (u8TXCnt & utilBitMask(8)) | ((u8TYCnt & utilBitMask(8)) << 8) | ((u8SOffset & utilBitMask(8)) << 16) | ((u8TOffset & utilBitMask(8)) << 24));
	_REG(gpSVM_reg_ctrl->vu32SamplingDistance, (u8Dstw & utilBitMask(8)) | ((u16Incr & utilBitMask(16)) << 16));
}

/*
 * function					:	PPDRV_SVM_UpdateInput
 
 * return					:	PP_VOID

 * brief
	update input buffer of channels.
*/
STATIC PP_VOID PPDRV_SVM_UpdateInput(PP_VOID)
{
	if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 0) == PP_FALSE )
	{
		PPDRV_SVM_IN_SetEnable(PP_TRUE, PP_TRUE, PP_TRUE, PP_TRUE);
		//OSAL_sleep(100);
	}
}

/*
 * function					:	PPDRV_SVM_SetMaskColor

 * param u8Y				:	Y
 * param u8Cbcr				:	Cb & Cr

 * return					:	PP_VOID

 * brief
	Sets the maskcolor of the SVM.
	Maskcolor is used in an invalid lut areas.
	Cb should be equal to Cr.
*/
STATIC PP_VOID PPDRV_SVM_SetMaskColor(PP_U8 IN u8Y, PP_U8 IN u8Cbcr)
{
	_REG(gpSVM_reg_ctrl->vu32MaskColor, (u8Cbcr) | (u8Cbcr << 8) | (u8Y <<16));
}

STATIC PP_U32 PPDRV_SVM_GetPLLClock(PP_S32 s32Num)
{
	PP_U32 m, n;
	PP_U32 u32Clk;
	PP_VU32 vu32Reg;
	
	vu32Reg = *(PP_VU32 *)(SCU_BASE_ADDR + 0x50 + s32Num*4);
	
	if(s32Num == 0){
		m = ((vu32Reg >> 7) & 1) + 1;
		n = vu32Reg & 0x7f;
	}else{
		m = ((vu32Reg >> 26) & 3) + 1;
		n = ((vu32Reg >> 20) & 0x3f);	
	}
	
	u32Clk = (XIN * n) / m;
	
	return u32Clk;
}

STATIC PP_U32 PPDRV_SVM_GetDUClock(PP_VOID)
{
	PP_VU32 vu32Reg;
	PP_S32 s32Div;
	PP_U32 u32Outclk;
	PP_U32 u32Srcclk;

	//core cpu
	vu32Reg = *(PP_VU32 *)(SCU_BASE_ADDR + 0x18);
	
	s32Div = ((vu32Reg>>(16)) & 0x7);
	if(s32Div == 0){
		s32Div = 4;
		u32Srcclk = PPDRV_SVM_GetPLLClock(1);
	}else if(s32Div == 1){
		s32Div = 4;
		u32Srcclk = PPDRV_SVM_GetPLLClock(2);
	}else if(s32Div == 2){
		s32Div = 8;
		u32Srcclk = PPDRV_SVM_GetPLLClock(1);
	}else if(s32Div == 3){
		s32Div = 8;
		u32Srcclk = PPDRV_SVM_GetPLLClock(2);
	}else if(s32Div == 4){
		s32Div = 16;
		u32Srcclk = PPDRV_SVM_GetPLLClock(1);
	}else if(s32Div == 5){
		s32Div = 16;
		u32Srcclk = PPDRV_SVM_GetPLLClock(2);
	}else if(s32Div == 6){
		s32Div = 64;
		u32Srcclk = PPDRV_SVM_GetPLLClock(0);
	}
	u32Outclk = u32Srcclk / s32Div;
	
	return u32Outclk;
}


/*======================================================================================================================
 = Export function
======================================================================================================================*/

PP_U32 PPDRV_SVM_CTRL_GetVersion(PP_VOID)
{
	return GetRegValue(SVM_BASE_ADDR + 0x0400);
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetAddress

 * param enChannel			:	number of camera
 * param pu32Addr			:	address

 * return					:	error code

 * brief
	Sets the memory address of input channel data.
	The memory address of each channel is determined by SVMAPI_initialize().
	This function can be used when it is necessary to change the address value of the channel for additional function.
	If you do not use the set memory, you must free() it.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetAddress(PP_CHANNEL_E IN enChannel, PP_U32* IN pu32Addr)
{
	PP_U32 u32AddrInt = (PP_U32)pu32Addr;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enChannel >= eCHANNEL_MAX || pu32Addr == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( pu32Addr && ((u32AddrInt & 0xf) != 0x0) && ((u32AddrInt & 0xf) != 0x8) )
		return _RET(eERROR_INVALID_ALIGN);

	_REG(gpSVM_reg_ctrl->vu32CamWmainAddr[enChannel], pu32Addr);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_GetAddress

 * param enChannel			:	number of camera

 * return					:	memory address of input channel data

 * brief
	Gets the memory address of input channel data.
	The memory address of each channel is determined by SVMAPI_initialize().
***************************************************************************************************************************************************************/
PP_U32* PPDRV_SVM_IN_GetAddress(PP_CHANNEL_E IN enChannel)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return PP_NULL;

	return (PP_U32*)gpSVM_reg_ctrl->vu32CamWmainAddr[enChannel];
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_INOUT_SetReplaceColor

 * param u8Y				:	Y
 * param u8Cb				:	Cb
 * param u8Cr				:	Cr

 * return					:	PP_VOID

 * brief
 	Sets color of no video.
 	This color applies to input and output of no video. (PPDRV_SVM_IN_SetReplaceColorEnable() and PPDRV_SVM_OUT_SetReplaceColorEnable())
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_INOUT_SetReplaceColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr)
{
	_REG(gpSVM_reg_ctrl->vu32NoVideo, utilPutBits(gpSVM_reg_ctrl->vu32NoVideo, 8, 24, (u8Y << 0) | (u8Cb << 8) | (u8Cr << 16)));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetReplaceColorEnable

 * param enChannel			:	number of camera
 * param bOn				:	enable or disable

 * return					:	PP_VOID

 * brief
 	Sets whether replace color of channel data is on or off.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_IN_SetReplaceColorEnable(PP_CHANNEL_E IN enChannel, PP_BOOL IN bOn)
{	
	_REG(gpSVM_reg_ctrl->vu32NoVideo, utilPutBit(gpSVM_reg_ctrl->vu32NoVideo, enChannel, bOn));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetReplaceColorEnable

 * param enChannel			:	number of camera

 * return					:	enable or disable

 * brief
 	Gets whether replace color of channel data is on or off.
***************************************************************************************************************************************************************/
PP_BOOL	PPDRV_SVM_IN_GetReplaceColorEnable(PP_CHANNEL_E IN enChannel)
{
	return utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, enChannel);
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetBlank

 * param u16HBlank			:	horizontal blank
 * param u16VBlank			:	vertical blank

 * return					:

 * brief
 	Sets horizontal and vertical blank of output.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_IN_SetBlank(PP_U16 IN u16HBlank, PP_U16 IN u16VBlank)
{
	PP_U8 u8PipLineDelay = u16VBlank/2-4;
	PP_U16 u16OutLineDaly = 0;
		
	_REG(gpSVM_reg_ctrl->vu32InBlank, (u16HBlank & utilBitMask(16)) | ((u16VBlank & utilBitMask(16)) << 16));
	_REG(gpSVM_reg_ctrl->vu32InSyncCtrl, utilPutBits(gpSVM_reg_ctrl->vu32InSyncCtrl, 0, 24, (u16OutLineDaly & utilBitMask(16)) | ((u8PipLineDelay & utilBitMask(8)) << 16)));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetMirroring

 * param enChannel			:	number of camera
 * param bHorizontal		:	horizontal flipping
 * param bVertical			:	vertical flipping

 * return					:	error code

 * brief
	Sets flipping of the camera.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetMirroring(PP_CHANNEL_E IN enChannel, PP_BOOL IN bHorizontal, PP_BOOL IN bVertical)
{
	PP_U8 u8Pos = 0x0;
	PP_U8 u8Value = 0x0;

	if ( enChannel >= eCHANNEL_MAX ) return _RET(eERROR_INVALID_ARGUMENT);

	u8Pos = 0x8 + (enChannel * 0x2);
	if ( bHorizontal && bVertical )
	{
		u8Value = 0x3;
	}
	else if ( bHorizontal && !bVertical )
	{
		u8Value = 0x1;
	}
	else if ( !bHorizontal && bVertical )
	{
		u8Value = 0x2;
	}
	else
	{
		u8Value = 0x0;
	}

	if ( gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].bInterlace )
	{
		PP_U32 fsyncPolarity = GetRegValue(VIN_BASE_ADDR+0x14);
		if ( bVertical )
			SetRegValue(VIN_BASE_ADDR+0x14, utilSetBit(fsyncPolarity, 16 + enChannel));
		else
			SetRegValue(VIN_BASE_ADDR+0x14, utilClearBit(fsyncPolarity, 16 + enChannel));
	}

	_REG(gpSVM_reg_ctrl->vu32ChCtrl, utilPutBits(gpSVM_reg_ctrl->vu32ChCtrl, u8Pos, 2, u8Value));
	
	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetAntiAliasing

 * param enChannel			:	number of camera
 * param enHorizotal		:	anti-aliasing filter tap number of horizotal
 * param enVertical			:	anti-aliasing filter tap number of vertical

 * return					:	error code

 * brief
 	Sets the anti-alaising filter tap number.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetAntiAliasing(PP_CHANNEL_E IN enChannel, PP_SVMDRV_ANTI_ALIASING_STRENGTH_H_E IN enHorizotal, PP_SVMDRV_ANTI_ALIASING_STRENGTH_V_E IN enVertical)
{
	PP_U8 u8HPos, u8VPos;
	
	if ( enChannel >= eCHANNEL_MAX || enHorizotal >= eSVMDRV_AA_H_MAX || enVertical >= eSVMDRV_AA_V_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	u8HPos = (enChannel * 4) + 16;
	u8VPos = (enChannel * 2) + 16;

	_REG(gpSVM_reg_ctrl->vu32Scale, utilPutBits(gpSVM_reg_ctrl->vu32Scale, u8HPos, 3, enHorizotal));
	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBits(gpSVM_reg_ctrl->vu32Fsync, u8VPos, 2, enVertical));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetBurstLength

 * param enBurstLength		:	burst length (eSVMDRV_BURST_LENGTH_16 ~ eSVMDRV_BURST_LENGTH_128)

 * return					:	error code

 * brief
 	Sets the burst size of the write main.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetBurstLength(PP_SVMDRV_BURST_LENGTH_E IN enBurstLength)
{
	if ( enBurstLength < eSVMDRV_BURST_LENGTH_16 || enBurstLength > eSVMDRV_BURST_LENGTH_128 )
		return _RET(eERROR_INVALID_ARGUMENT);

	_REG(gpSVM_reg_ctrl->vu32Wmain, utilPutBits(gpSVM_reg_ctrl->vu32Wmain, 4, 4, enBurstLength));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function						:	PPDRV_SVM_IN_SetHoldTime

 * param u16NoScale				:	hold time of no scale
 * param u16OneOverTwoScale		:	hold time of 1/2 scale
 * param u16OneOverFourScale	:	hold time of 1/4 scale
 * param u16OneOverEightScale	:	hold time of 1/8 scale

 * return PP_VOID				:

 * brief
 	Sets the hold time of the horizontal scale.
	It guarantees priority of read operation by delaying write.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_IN_SetHoldTime(PP_U16 IN u16NoScale, PP_U16 IN u16OneOverTwoScale, PP_U16 IN u16OneOverFourScale, PP_U16 IN u16OneOverEightScale)
{
	_REG(gpSVM_reg_ctrl->vu32SclDownHoldCnt[0], (u16NoScale & utilBitMask(11)) | ((u16OneOverTwoScale & utilBitMask(11)) << 16));
	_REG(gpSVM_reg_ctrl->vu32SclDownHoldCnt[1], (u16OneOverFourScale & utilBitMask(11)) | ((u16OneOverEightScale & utilBitMask(11)) << 16));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetEnable

 * param bFrontEnable		:	enable or disable of front input channel
 * param bLeftEnable		:	enable or disable of left input channel
 * param bRightEnable		:	enable or disable of right input channel
 * param bRearEnable		:	enable or disable of back input channel

 * return					:	error code

 * brief
	Enable or disable of the input data.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetEnable(PP_BOOL IN bFrontEnable, PP_BOOL IN bLeftEnable, PP_BOOL IN bRightEnable, PP_BOOL IN bRearEnable)
{
	PP_U8 u8Burst = 0x0;
	PP_U8 u8InitEn = 0x0;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	u8Burst = utilGetBits(gpSVM_reg_ctrl->vu32Wmain, 4, 4);
	u8InitEn = utilGetBit(gpSVM_reg_ctrl->vu32Wmain, 2);

	_REG(gpSVM_reg_ctrl->vu32Wmain, (bFrontEnable << 0) | (0x1 << 1) | (u8InitEn << 2 ) | ((u8Burst & utilBitMask(4)) << 4) | (bLeftEnable << 8) | (bRightEnable << 9) | (bRearEnable << 10));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetRoi

 * param enChannel			:	number of camera
 * param stRoi				:	information of roi

 * return					:	error code

 * brief
	Sets the ROI of the SVM.
	ROI is used to reduce MBW(Memory Band Width).
	Use only specific area.
	StartYLine and endYLine is operated by multiples of 4.

	------------------------------
	|        not used area       |
	------------------------------	startYLine
	|                            |
	|          ROI area          |
	|                            |
	------------------------------	endYLine
	|        not used area       |
	------------------------------
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetRoi(PP_CHANNEL_E IN enChannel, PP_SVM_ROI_AREA_S IN stRoi)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enChannel >= eCHANNEL_MAX ||
		 stRoi.u16StartYLine > gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height || stRoi.u16EndYLine > gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height || stRoi.u16StartYLine > stRoi.u16EndYLine )
		return _RET(eERROR_INVALID_ARGUMENT);

	_REG(gpSVM_reg_ctrl->vu32Roi[enChannel], (stRoi.u16StartYLine & utilBitMask(11)) | ((stRoi.u16EndYLine & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32Roi[enChannel], 31) << 31));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetRoiEnable

 * param enChannel			:	number of camera
 * param bEnable			:	enable or disable

 * return					:	error code

 * brief
	Enable or disable of the ROI.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetRoiEnable(PP_CHANNEL_E IN enChannel, PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enChannel >= eCHANNEL_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	_REG(gpSVM_reg_ctrl->vu32Roi[enChannel], utilPutBit(gpSVM_reg_ctrl->vu32Roi[enChannel], 31, bEnable));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetHorizontalScale

 * param enChannel			:	number of camera
 * param enScale			:	index of horizontal scale

 * return					:	error code

 * brief
	Sets the horizontal scale of the SVM.
	hscale is used to reduce MBW(Memory Band Width).

	ex)
	------------------------------                            ---------------
	|                            |    1/2 or 1/4 or 1/8scale  |              |
	|                            |    ------------------->    |              |
	|                            |                            |              |
	|                            |                            |              |
	------------------------------                            ----------------
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetHorizontalScale(PP_CHANNEL_E IN enChannel, PP_SVMDRV_HORIZONTALSCALE_E IN enScale)
{
	PP_U8 u8Pos = 0x0;
	PP_U8 u8ScaleValue;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

 	if ( enChannel >= eCHANNEL_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);
	
	switch (enScale)
	{
	case eSVMDRV_HSCALE_ONE_OVER_TWO:		u8ScaleValue = 2; break;
	case eSVMDRV_HSCALE_ONE_OVER_FOUR:		u8ScaleValue = 4; break;
	case eSVMDRV_HSCALE_ONE_OVER_EIGHT:	u8ScaleValue = 8; break;
	default:							u8ScaleValue = 1; break;
	}
	
	if ( (enScale >= eSVMDRV_HSCALE_MAX) ||
		 (gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Width % u8ScaleValue != 0) ||
		 ((gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Width / u8ScaleValue) % 8 != 0) )
	{
		return _RET(eERROR_INVALID_ARGUMENT);
	}
	
	u8Pos = enChannel * 0x2;
	_REG(gpSVM_reg_ctrl->vu32Scale, utilPutBits(gpSVM_reg_ctrl->vu32Scale, u8Pos, 2, enScale));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetVerticalScale

 * param enChannel			:	number of camera
 * param enScale			:	index of vertical scale

 * return					:	error code

 * brief
	Sets the vertical scale of the SVM.
	vscale is used to reduce MBW(Memory Band Width).

	ex)
	------------------------------
	|                            |    1/2 or 1/4 scale  ------------------------------
	|                            |    ------------->    |                            |
	|                            |                      |                            |
	|                            |                      ------------------------------
	------------------------------
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_IN_SetVerticalScale(PP_CHANNEL_E IN enChannel, PP_SVMDRV_VERTICALSCALE_E IN enScale)
{
	PP_U8 u8ScaleValue;
	PP_U8 u8Pos = 0x0;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	switch (enScale)
	{
	case eSVMDRV_VSCALE_ONE:			u8ScaleValue = 1; break;
	case eSVMDRV_VSCALE_ONE_OVER_TWO:	u8ScaleValue = 2; break;
	case eSVMDRV_VSCALE_ONE_OVER_FOUR:	u8ScaleValue = 4; break;
	default:						return _RET(eERROR_INVALID_ARGUMENT);
	}

 	if ( enChannel >= eCHANNEL_MAX ||
	     (gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height % u8ScaleValue != 0) ||
		 ((gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height / u8ScaleValue) % 4 != 0))
		return _RET(eERROR_INVALID_ARGUMENT);
	
	u8Pos = enChannel * 0x2 + 0x8;
	_REG(gpSVM_reg_ctrl->vu32Scale, utilPutBits(gpSVM_reg_ctrl->vu32Scale, u8Pos, 2, enScale));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetTestMode

 * param bEnable			:	enable or disable

 * return					:

 * brief
	Enable or disable of the input test mode.
	The test mode uses the input data of the DRAM.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_IN_SetTestMode(PP_BOOL IN bEnable)
{
#if 0	// input test mode TG
	PP_U32 u32Value = utilPutBit(gpSVM_reg_ctrl->vu32Enable, 3, bEnable);
	u32Value = utilPutBit(u32Value, 7, bEnable);

	_REG(gpSVM_reg_ctrl->vu32Enable, u32Value);
#else	// sensor TG
	_REG(gpSVM_reg_ctrl->vu32Enable, utilPutBit(gpSVM_reg_ctrl->vu32Enable, 3, bEnable));
#endif
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_IN_SetWrappingInitEnable

 * param bEnable			:	enable or disable

 * return					:

 * brief
	Initialize wrapping address to start address at frame start.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_IN_SetWrappingInitEnable(PP_BOOL IN bEnable)
{
	_REG(gpSVM_reg_ctrl->vu32Wmain, utilPutBit(gpSVM_reg_ctrl->vu32Wmain, 2, bEnable));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetImage

 * param enImgNum			:	number of image
 * param stRect				:	position for drawing image
 * param u8Alpha			:	transparency (0 ~ 63)

 * return					:	error code

 * brief
	Sets the image information of the SVM.
	X position vlaue should be an even number greater than or equal to 2.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetImage(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_RECT_S IN stRect, PP_U8 IN u8Alpha)
{
	PP_BOOL bLayerMode = PP_FALSE;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enImgNum >= eSVMDRV_IMG_NUM_MAX || (stRect.u16Width % 2 != 0) || stRect.u16Width < 2 || stRect.u16Height == 0 )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( u8Alpha > 63 ) u8Alpha = 63;

	if ( stRect.u16X != 0 && stRect.u16X % 2 != 0 ) --stRect.u16X;

	if ( enImgNum ==  eSVMDRV_IMG_NUM_0 )
	{
		_REG(gpSVM_reg_ctrl->vu32ImgX, (stRect.u16X & utilBitMask(11)) | (((stRect.u16Width - 1) & utilBitMask(11)) << 16) |
								 ((u8Alpha & utilBitMask(1)) << 15) | (((u8Alpha >> 1) & utilBitMask(5)) << 27));
		_REG(gpSVM_reg_ctrl->vu32ImgY, (stRect.u16Y & utilBitMask(11)) | (bLayerMode << 15) | (((stRect.u16Height - 1) & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32ImgY, 31) << 31));
	}
	else
	{
		_REG(gpSVM_reg_ctrl->vu32Img2X, (stRect.u16X & utilBitMask(11)) | (((stRect.u16Width - 1) & utilBitMask(11)) << 16)  |
								  ((u8Alpha & utilBitMask(1)) << 15) | (((u8Alpha >> 1) & utilBitMask(5)) << 27));
		_REG(gpSVM_reg_ctrl->vu32Img2Y, (stRect.u16Y & utilBitMask(11)) | (bLayerMode << 15) | (((stRect.u16Height - 1) & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32Img2Y, 31) << 31));
	}

	return eSUCCESS;
}

PP_VOID PPDRV_SVM_CTRL_SetImageMaskColor(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr)
{
	PP_U32 u32MaskColor = (u8Cr) | (u8Cb << 8) | (u8Y <<16);

	if ( enImgNum ==  eSVMDRV_IMG_NUM_0 )
		_REG(gpSVM_reg_ctrl->vu32ImgCtrl, utilPutBits(gpSVM_reg_ctrl->vu32ImgCtrl, 0, 24, u32MaskColor));
	else
		_REG(gpSVM_reg_ctrl->vu32Img2Ctrl, utilPutBits(gpSVM_reg_ctrl->vu32Img2Ctrl, 0, 24, u32MaskColor));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetImageBurstLength

 * param enImgNum			:	number of image
 * param u8BurstLength		:	burst length

 * return					:	error code

 * brief
 	Sets the burst size of the image.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetImageBurstLength(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U8 IN u8BurstLength)
{
	if ( u8BurstLength > 127 || enImgNum >= eSVMDRV_IMG_NUM_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( enImgNum == eSVMDRV_IMG_NUM_0 )
	{
		_REG(gpSVM_reg_ctrl->vu32ImgCtrl, utilPutBits(gpSVM_reg_ctrl->vu32ImgCtrl, 24, 7, u8BurstLength));
	}
	else
	{
		_REG(gpSVM_reg_ctrl->vu32Img2Ctrl, utilPutBits(gpSVM_reg_ctrl->vu32Img2Ctrl, 24, 7, u8BurstLength));
	}

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetImageAddr

 * param enImgNum			:	number of image
 * param pu32Addr4odd		:	progressive output case - address of image memory
 								interlace output case - address of odd field image memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field image memory

 * return					:	error code

 * brief
	Sets the memory address of image. 
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetImageAddr(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even)
{
	PP_U32 u32Addr4oddInt = (PP_U32)pu32Addr4odd;
	PP_U32 u32Addr4evenInt = (PP_U32)pu32Addr4even;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enImgNum >= eSVMDRV_IMG_NUM_MAX ||	pu32Addr4odd == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( (pu32Addr4odd && ((u32Addr4oddInt & 0xf) != 0x0) && ((u32Addr4oddInt & 0xf) != 0x8)) ||
		 (pu32Addr4even && ((u32Addr4evenInt & 0xf) != 0x0) && ((u32Addr4evenInt & 0xf) != 0x8)) )
		return _RET(eERROR_INVALID_ALIGN);

	if ( enImgNum == eSVMDRV_IMG_NUM_0 )
	{
		_REG(gpSVM_reg_ctrl->vu32ImgAddr, pu32Addr4odd);
		_REG(gpSVM_reg_ctrl->vu32ImgAddrEven, pu32Addr4even);
	}
	else
	{
		_REG(gpSVM_reg_ctrl->vu32Img2Addr, pu32Addr4odd);
		_REG(gpSVM_reg_ctrl->vu32Img2AddrEven, pu32Addr4even);
	}

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetImageAddr

 * param enImgNum			:	number of image
 * param enFiled			:	odd (progressive or odd of interlace) or even (even of interlace)

 * return					:	error code

 * brief
	Gets the memory address of image. 
***************************************************************************************************************************************************************/
PP_U32* PPDRV_SVM_CTRL_GetImageAddr(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_FIELD_E IN enField)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return PP_NULL;

	if ( enImgNum >= eSVMDRV_IMG_NUM_MAX || enField >= eFIELD_MAX )
		return PP_NULL;

	if ( enImgNum == eSVMDRV_IMG_NUM_0 )
	{
		if ( enField == eFIELD_ODD )
			return (PP_U32*)gpSVM_reg_ctrl->vu32ImgAddr;
		else
			return (PP_U32*)gpSVM_reg_ctrl->vu32ImgAddrEven;
	}
	else
	{
		if ( enField == eFIELD_ODD )
			return (PP_U32*)gpSVM_reg_ctrl->vu32Img2Addr;
		else
			return (PP_U32*)gpSVM_reg_ctrl->vu32Img2AddrEven;
	}

	return PP_NULL;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetImageEable

 * param enImgNum			:	number of image
 * param bImg0Enable		:	enable or disable of image0
 * param bImg1Enable		:	enable or disable of image1

 * return					:	error code

 * brief
	Enable or disable of the image object.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetImageEnable(PP_SVMDRV_IMG_NUMBER_E IN enImgNum, PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enImgNum >= eSVMDRV_IMG_NUM_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( enImgNum == eSVMDRV_IMG_NUM_0 )
		_REG(gpSVM_reg_ctrl->vu32ImgY, utilPutBit(gpSVM_reg_ctrl->vu32ImgY, 31, bEnable));
	else
		_REG(gpSVM_reg_ctrl->vu32Img2Y, utilPutBit(gpSVM_reg_ctrl->vu32Img2Y, 31, bEnable));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetEdgeEnhancementMode

 * param enMode				:	mode (tagPP_SVMDRV_EDGE_MODE_E)

 * return					:	error code

 * brief
	Sets the mode of edge enhancement.
	mode 0 uses brightness control lut. ([31:26])
	mode 1 uses fixed gain.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetEdgeEnhancementMode(PP_SVMDRV_EDGE_MODE_E IN enMode)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enMode >= eSVMDRV_EDGE_MODE_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBit(gpSVM_reg_ctrl->vu32Fsync, 15, enMode));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetEdgeEnhancementFixedGain

 * param u8Gain				:	fixed gain

 * return					:	PP_VOID

 * brief
	Sets the fixed gain of edge enhancement.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_SetEdgeEnhancementFixedGain(PP_U8 IN u8Gain)
{
	_REG(gpSVM_reg_ctrl->vu32OutMode, utilPutBits(gpSVM_reg_ctrl->vu32OutMode, 24, 8, u8Gain));
}

/***************************************************************************************************************************************************************
 * function					:	PPDEV_SVM_CTRL_SetEdgeEnhancementLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of edge enhancemant lut memory
 								interlace output case - address of odd field edge enhancemant lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field edge enhancemant lut memory

 * return					:	error code

 * brief
	Sets the memory address of brightnees control lut. 
***************************************************************************************************************************************************************/
PP_RESULT_E PPDEV_SVM_CTRL_SetEdgeEnhancementLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even)
{
	PP_U32 u32Addr4oddInt = (PP_U32)pu32Addr4odd;
	PP_U32 u32Addr4evenInt = (PP_U32)pu32Addr4even;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( pu32Addr4odd == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( (pu32Addr4odd && ((u32Addr4oddInt & 0xf) != 0x0) && ((u32Addr4oddInt & 0xf) != 0x8)) ||
		 (pu32Addr4even && ((u32Addr4evenInt & 0xf) != 0x0) && ((u32Addr4evenInt & 0xf) != 0x8)) )
		return _RET(eERROR_INVALID_ALIGN);

	_REG(gpSVM_reg_ctrl->vu32EdgeLutAddr, pu32Addr4odd);
	_REG(gpSVM_reg_ctrl->vu32EdgeLutAddrEven, pu32Addr4even);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetEdgeEnhancementLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of edge enhancemant lut memory
 								interlace output case - address of odd field edge enhancemant lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field edge enhancemant lut memory

 * return					:	error code

 * brief
	Gets the memory address of front_back lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_GetEdgeEnhancementLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( pu32Addr4odd == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	*pu32Addr4odd = (PP_U32*)gpSVM_reg_ctrl->vu32EdgeLutAddr;
	if ( pu32Addr4even )
		*pu32Addr4even = (PP_U32*)gpSVM_reg_ctrl->vu32EdgeLutAddrEven;

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetEdgeEnhancementEnable

 * param bEnable			:	enable or disable

 * return					:	error code

 * brief
	Enable or disable of edge enhancement
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetEdgeEnhancementEnable(PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	_REG(gpSVM_reg_ctrl->vu32OutMode, utilPutBit(gpSVM_reg_ctrl->vu32OutMode, 23, bEnable));

	return eSUCCESS;
}

PP_RESULT_E PPDRV_SVM_CTRL_SetWindowFilterCoefficient(PP_U8 IN u8Coef00, PP_U8 IN u8Coef01, PP_U8 IN u8Coef02,
																		   PP_U8 IN u8Coef10, PP_U8 IN u8Coef11, PP_U8 IN u8Coef12,
																		   PP_U8 IN u8Coef20, PP_U8 IN u8Coef21, PP_U8 IN u8Coef22)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	_REG(gpSVM_reg_ctrl->vu32WinFilter0, utilPutBits(gpSVM_reg_ctrl->vu32WinFilter0, 0, 24, (u8Coef00 << 0) | (u8Coef01 << 8) | (u8Coef01 << 16)));
	_REG(gpSVM_reg_ctrl->vu32WinFilter1, utilPutBits(gpSVM_reg_ctrl->vu32WinFilter1, 0, 24, (u8Coef10 << 0) | (u8Coef11 << 8) | (u8Coef11 << 16)));
	_REG(gpSVM_reg_ctrl->vu32WinFilter2, utilPutBits(gpSVM_reg_ctrl->vu32WinFilter2, 0, 24, (u8Coef20 << 0) | (u8Coef21 << 8) | (u8Coef21 << 16)));

	return eSUCCESS;
}

PP_RESULT_E PPDRV_SVM_CTRL_SetWindowFilterEnable(PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	_REG(gpSVM_reg_ctrl->vu32WinFilter0, utilPutBit(gpSVM_reg_ctrl->vu32WinFilter0, 24, bEnable));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetDynamicblendingCoefficient

 * param u8Coef1			:	coef of 1st bc range alpha
 * param u8Coef3			:	coef of 3rd bc range alpha
 * param u8Coef6			:	coef of 6th bc range alpha
 * param u8Coef8			:	coef of 8th bc range alpha

 * return					:	PP_VOID

 * brief
	Sets the coefficient of dynamic blending.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_SetDynamicblendingCoefficient(PP_U8 u8Coef1, PP_U8 u8Coef3, PP_U8 u8Coef6, PP_U8 u8Coef8)
{
	_REG(gpSVM_reg_dynamic->vu32Var, (u8Coef1 << 0) | (u8Coef3 << 8) | (u8Coef6 << 16) | (u8Coef8 << 24));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetDynamicblendingEnable

 * param bEnable			:	enable or disable

 * return					:	error code

 * brief
	Enable or disable of dynamic blending
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetDynamicblendingEnable(PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	_REG(gpSVM_reg_ctrl->vu32OutMode, utilPutBit(gpSVM_reg_ctrl->vu32OutMode, 4, bEnable));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetAutoMorphingSpeed

 * param u8Speed			:	speed of morphing (0~7)

 * return					:	error code

 * brief
	Sets the speed for morphing.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetAutoMorphingSpeed(PP_U8 IN u8Speed)
{
	if ( u8Speed > 7 )
		return _RET(eERROR_INVALID_ARGUMENT);

	_REG(gpSVM_reg_ctrl->vu32OutMode, utilPutBits(gpSVM_reg_ctrl->vu32OutMode, 18, 3, u8Speed));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBCCoefficient

 * param pstCoefBC			:	coefficient information of brightness control

 * return					:	error code

 * brief
	Update coefficient of brightness control.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetBCCoefficient(PP_SVMDRV_BCCOEF_S* IN pstCoefBC)
{
	PP_U8 i = 0;
	PP_BOOL bEnable = utilGetBit(gpSVM_reg_ctrl->vu32OutMode, 5);

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return eERROR_SVM_NOT_INITIALIZE;

	if ( pstCoefBC == PP_NULL )
		return eERROR_INVALID_ARGUMENT;

	if ( bEnable )
	{
		for ( i = eSVMDRV_COLOR_R; i < eSVMDRV_COLOR_MAX; i++ )
		{
			if ( pstCoefBC->u8Coef2[i][eSVMDRV_FB_PART_F] > 127 ) pstCoefBC->u8Coef2[i][eSVMDRV_FB_PART_F] = 127;
			if ( pstCoefBC->u8Coef2[i][eSVMDRV_FB_PART_B] > 127 ) pstCoefBC->u8Coef2[i][eSVMDRV_FB_PART_B] = 127;
			if ( pstCoefBC->u8Coef4[i][eSVMDRV_LR_PART_L] > 127 ) pstCoefBC->u8Coef4[i][eSVMDRV_LR_PART_L] = 127;
			if ( pstCoefBC->u8Coef4[i][eSVMDRV_LR_PART_R] > 127 ) pstCoefBC->u8Coef4[i][eSVMDRV_LR_PART_R] = 127;
		}

		for ( i = eSVMDRV_COLOR_R; i < eSVMDRV_COLOR_MAX; i++ )
		{
			_REG(gpSVM_reg_coef->vu32Coef1[i], (pstCoefBC->u8Coef1[i][eSVMDRV_SUBPART_FL]) | (pstCoefBC->u8Coef1[i][eSVMDRV_SUBPART_FR] << 8) | (pstCoefBC->u8Coef1[i][eSVMDRV_SUBPART_BR] << 16) | (pstCoefBC->u8Coef1[i][eSVMDRV_SUBPART_BL] << 24));
			_REG(gpSVM_reg_coef->vu32Coef3[i], (pstCoefBC->u8Coef3[i][eSVMDRV_SUBPART_FL]) | (pstCoefBC->u8Coef3[i][eSVMDRV_SUBPART_FR] << 8) | (pstCoefBC->u8Coef3[i][eSVMDRV_SUBPART_BR] << 16) | (pstCoefBC->u8Coef3[i][eSVMDRV_SUBPART_BL] << 24));
			_REG(gpSVM_reg_coef->vu32Coef2n4[i], (pstCoefBC->u8Coef2[i][eSVMDRV_FB_PART_F]) | (pstCoefBC->u8Coef2[i][eSVMDRV_FB_PART_B] << 8) | (pstCoefBC->u8Coef4[i][eSVMDRV_LR_PART_L] << 16) | (pstCoefBC->u8Coef4[i][eSVMDRV_LR_PART_R] << 24));
		}
	}
	else
	{
		for ( i = eSVMDRV_COLOR_R; i < eSVMDRV_COLOR_MAX; i++ )
		{
			_REG(gpSVM_reg_coef->vu32Coef1[i], 0x40404040);
			_REG(gpSVM_reg_coef->vu32Coef3[i], 0x40404040);
			_REG(gpSVM_reg_coef->vu32Coef2n4[i], 0x40404040);
		}
	}

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetBCState

 * param					:	

 * return					:	on or off of BC

 * brief
	Get BC state.
***************************************************************************************************************************************************************/
PP_BOOL PPDRV_SVM_CTRL_GetBCState(PP_VOID)
{
	return utilGetBit(gpSVM_reg_ctrl->vu32OutMode, 5);
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBCRatio

 * param u8Ratio			:	ratio of brightness control

 * return					:	PP_VOID

 * brief
	Update ratio of brightness control.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_SetBCRatio(PP_U8 IN u8Ratio)
{
	_REG(gpSVM_reg_ctrl->vu32OutMode, utilPutBits(gpSVM_reg_ctrl->vu32OutMode, 8, 8, u8Ratio));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetMorphingRatio

 * param u8FBLUTRatio		:	morphing ratio of FB LUT
 * param u8LRLUTRatio		:	morphing ratio of LR LUT

 * return					:	PP_VOID

 * brief
	Update ratio of morphing ratio.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_SetMorphingRatio(PP_U8 IN u8FBLUTRatio, PP_U8 IN u8LRLUTRatio)
{
	_REG(gpSVM_reg_ctrl->vu32ChCtrl, utilPutBits(gpSVM_reg_ctrl->vu32ChCtrl, 16, 16, u8FBLUTRatio << 0 | u8LRLUTRatio << 8));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetFBLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of fb lut memory
 								interlace output case - address of odd field fb lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field fb lut memory
  * param enNum				:	morphing number

 * return					:	error code

 * brief
	Sets the memory address of front_back lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetFBLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum)
{
	PP_U32 u32Addr4oddInt = (PP_U32)pu32Addr4odd;
	PP_U32 u32Addr4evenInt = (PP_U32)pu32Addr4even;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( (pu32Addr4odd && ((u32Addr4oddInt & 0xf) != 0x0) && ((u32Addr4oddInt & 0xf) != 0x8)) ||
		 (pu32Addr4even && ((u32Addr4evenInt & 0xf) != 0x0) && ((u32Addr4evenInt & 0xf) != 0x8)) )
		return _RET(eERROR_INVALID_ALIGN);

	_REG(gpSVM_reg_ctrl->vu32FbLutAddr[enNum], pu32Addr4odd);
	_REG(gpSVM_reg_ctrl->vu32FbLutAddrEven[enNum], pu32Addr4even);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetFBLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of fb lut memory
 								interlace output case - address of odd field fb lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field fb lut memory
 * param enNum				:	morphing number

 * return					:	error code

 * brief
	Gets the memory address of front_back lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_GetFBLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum)
{
	if ( pu32Addr4odd == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	*pu32Addr4odd = (PP_U32*)gpSVM_reg_ctrl->vu32FbLutAddr[enNum];
	if ( pu32Addr4even )
		*pu32Addr4even = (PP_U32*)gpSVM_reg_ctrl->vu32FbLutAddrEven[enNum];

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetLRLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of lr lut memory
 								interlace output case - address of odd field lr lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field lr lut memory
 * param enNum				:	morphing number
  
 * return					:	error code

 * brief
	Sets the memory address of left_right lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetLRLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum)
{
	PP_U32 u32Addr4oddInt = (PP_U32)pu32Addr4odd;
	PP_U32 u32Addr4evenInt = (PP_U32)pu32Addr4even;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( (pu32Addr4odd && ((u32Addr4oddInt & 0xf) != 0x0) && ((u32Addr4oddInt & 0xf) != 0x8)) ||
		 (pu32Addr4even && ((u32Addr4evenInt & 0xf) != 0x0) && ((u32Addr4evenInt & 0xf) != 0x8)) )
		return _RET(eERROR_INVALID_ALIGN);

	_REG(gpSVM_reg_ctrl->vu32LrLutAddr[enNum], pu32Addr4odd);
	_REG(gpSVM_reg_ctrl->vu32LrLutAddrEven[enNum], pu32Addr4even);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetLRLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of lr lut memory
 								interlace output case - address of odd field lr lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field lr lut memory
 * param enNum				:	morphing number

 * return					:	error code

 * brief
	Gets the memory address of left_right lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_GetLRLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even, PP_SVMDRV_MORPHING_NUM_E enNum)
{
	if ( pu32Addr4odd == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	*pu32Addr4odd = (PP_U32*)gpSVM_reg_ctrl->vu32LrLutAddr[enNum];
	if ( pu32Addr4even )
		*pu32Addr4even = (PP_U32*)gpSVM_reg_ctrl->vu32LrLutAddrEven[enNum];

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetUnuseInvalidArea

 * param bEnable			:	enable or disable

 * return					:	error code

 * brief
	Enable or disable of the invalid lut.
	Invalid lut is not used for unnecessary parts using alpha of bc lut.
	Invalid lut is used to reduce MBW(Memory Band Width).
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetUnuseInvalidArea(PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBit(gpSVM_reg_ctrl->vu32Fsync, 9, bEnable));
	// small invalid gen
	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBit(gpSVM_reg_ctrl->vu32Fsync, 12, bEnable));

	return eSUCCESS;
}
 
/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBCLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of bc lut memory
 								interlace output case - address of odd field bc lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field bc lut memory

 * return					:	error code

 * brief
	Sets the memory address of brightnees control lut. 
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetBCLUTAddress(PP_U32* IN pu32Addr4odd, PP_U32* IN pu32Addr4even)
{
	PP_U32 u32Addr4oddInt = (PP_U32)pu32Addr4odd;
	PP_U32 u32Addr4evenInt = (PP_U32)pu32Addr4even;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( (pu32Addr4odd && ((u32Addr4oddInt & 0xf) != 0x0) && ((u32Addr4oddInt & 0xf) != 0x8)) ||
		 (pu32Addr4even && ((u32Addr4evenInt & 0xf) != 0x0) && ((u32Addr4evenInt & 0xf) != 0x8)) )
		return _RET(eERROR_INVALID_ALIGN);

	_REG(gpSVM_reg_ctrl->vu32BcLutAddr, pu32Addr4odd);
	_REG(gpSVM_reg_ctrl->vu32BcLutAddrEven, pu32Addr4even);

	if ( pu32Addr4odd && !gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].bInterlace )
		PPDRV_SVM_CTRL_SetUnuseInvalidArea(PP_TRUE);
	else
		PPDRV_SVM_CTRL_SetUnuseInvalidArea(PP_FALSE);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetBCLUTAddress

 * param pu32Addr4odd		:	progressive output case - address of bc lut memory
 								interlace output case - address of odd field bc lut memory
 * param pu32Addr4even		:	progressive output case - not used
 								interlace output case - address of even field bc lut memory

 * return					:	error code

 * brief
	Gets the memory address of brightness control lut. 
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_GetBCLUTAddress(PP_U32** OUT pu32Addr4odd, PP_U32** OUT pu32Addr4even)
{
	if ( pu32Addr4odd == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);

	*pu32Addr4odd = (PP_U32*)gpSVM_reg_ctrl->vu32BcLutAddr;
	if ( pu32Addr4even )
		*pu32Addr4even = (PP_U32*)gpSVM_reg_ctrl->vu32BcLutAddrEven;

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBCAdditionalLUT

 * param enType				:	type of additional lut
 * param stLutInfo			:	information of additional lut

 * return					:	error code

 * brief
	Sets additional lut of brightnees control lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetBCAdditionalLUT(PP_SVMDRV_BC_ADDITIONAL_LUT_E enType, PP_SVMDRV_BC_ADDITIONAL_LUT_S* IN stLutInfo)
{
	PP_S32 i, j;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);
	
	if ( stLutInfo )
	{
		for ( i = eFIELD_ODD; i < eFIELD_MAX; i++ )
		{
			for ( j = eSVMDRV_BC_ADD_LUT_SUBCORE_0; j < eSVMDRV_BC_ADD_LUT_SUBCORE_MAX; j++ )
			{
				if ( ((PP_U32)stLutInfo->pu32Addr[i][j] & 0xf) != 0x0 )
					return _RET(eERROR_INVALID_ALIGN);
			}
		}

		if ( enType == eSVMDRV_BC_ADD_LUT_ALPHA_0 )
		{
			_REG(gpSVM_reg_ctrl->vu32Alpha0Addr[eSVMDRV_BC_ADD_LUT_SUBCORE_0], stLutInfo->pu32Addr[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_0]);
			_REG(gpSVM_reg_ctrl->vu32Alpha0Addr[eSVMDRV_BC_ADD_LUT_SUBCORE_1], stLutInfo->pu32Addr[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_1]);

			_REG(gpSVM_reg_ctrl->vu32AlphaMoreEven[0], ((stLutInfo->u16TotalCnt[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_0] & utilBitMask(14)) << 0) |
														((stLutInfo->u16TotalCnt[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_1] & utilBitMask(14)) << 16) );
			_REG(gpSVM_reg_ctrl->vu32Alpha0AddrEven[eSVMDRV_BC_ADD_LUT_SUBCORE_0], stLutInfo->pu32Addr[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_0]);
			_REG(gpSVM_reg_ctrl->vu32Alpha0AddrEven[eSVMDRV_BC_ADD_LUT_SUBCORE_1], stLutInfo->pu32Addr[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_1]);
			
			_REG(gpSVM_reg_ctrl->vu32AlphaMore[0], ((stLutInfo->u16TotalCnt[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_0] & utilBitMask(14)) << 0) |
													((stLutInfo->u16TotalCnt[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_1] & utilBitMask(14)) << 16) |
													(PP_TRUE << 31) );
		}
		else if ( enType == eSVMDRV_BC_ADD_LUT_ALPHA_1 )
		{
			_REG(gpSVM_reg_ctrl->vu32Alpha1Addr[eSVMDRV_BC_ADD_LUT_SUBCORE_0], stLutInfo->pu32Addr[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_0]);
			_REG(gpSVM_reg_ctrl->vu32Alpha1Addr[eSVMDRV_BC_ADD_LUT_SUBCORE_1], stLutInfo->pu32Addr[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_1]);

			_REG(gpSVM_reg_ctrl->vu32AlphaMoreEven[1], ((stLutInfo->u16TotalCnt[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_0] & utilBitMask(14)) << 0) |
														((stLutInfo->u16TotalCnt[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_1] & utilBitMask(14)) << 16) );
			_REG(gpSVM_reg_ctrl->vu32Alpha1AddrEven[eSVMDRV_BC_ADD_LUT_SUBCORE_0], stLutInfo->pu32Addr[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_0]);
			_REG(gpSVM_reg_ctrl->vu32Alpha1AddrEven[eSVMDRV_BC_ADD_LUT_SUBCORE_1], stLutInfo->pu32Addr[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_1]);
			_REG(gpSVM_reg_ctrl->vu32AlphaMore[1], ((stLutInfo->u16TotalCnt[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_0] & utilBitMask(14)) << 0) |
													((stLutInfo->u16TotalCnt[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_1] & utilBitMask(14)) << 16) |
													(PP_TRUE << 31) );
		}
		else if ( enType == eSVMDRV_BC_ADD_LUT_GRADIENT )
		{
			_REG(gpSVM_reg_ctrl->vu32BcgradAddr[eSVMDRV_BC_ADD_LUT_SUBCORE_0], stLutInfo->pu32Addr[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_0]);
			_REG(gpSVM_reg_ctrl->vu32BcgradAddr[eSVMDRV_BC_ADD_LUT_SUBCORE_1], stLutInfo->pu32Addr[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_1]);

			_REG(gpSVM_reg_ctrl->vu32BcgradMoreEven, ((stLutInfo->u16TotalCnt[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_0] & utilBitMask(14)) << 0) |
													 ((stLutInfo->u16TotalCnt[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_1] & utilBitMask(14)) << 16) );
			_REG(gpSVM_reg_ctrl->vu32BcgradAddrEven[eSVMDRV_BC_ADD_LUT_SUBCORE_0], stLutInfo->pu32Addr[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_0]);
			_REG(gpSVM_reg_ctrl->vu32BcgradAddrEven[eSVMDRV_BC_ADD_LUT_SUBCORE_1], stLutInfo->pu32Addr[eFIELD_EVEN][eSVMDRV_BC_ADD_LUT_SUBCORE_1]);
			_REG(gpSVM_reg_ctrl->vu32BcgradMore, ((stLutInfo->u16TotalCnt[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_0] & utilBitMask(14)) << 0) |
												 ((stLutInfo->u16TotalCnt[eFIELD_ODD][eSVMDRV_BC_ADD_LUT_SUBCORE_1] & utilBitMask(14)) << 16) |
												 (PP_TRUE << 31) );
		}
	}
	else
	{
		if ( enType == eSVMDRV_BC_ADD_LUT_ALPHA_0 )
			_REG(gpSVM_reg_ctrl->vu32AlphaMore[0], PP_FALSE);
		else if ( enType == eSVMDRV_BC_ADD_LUT_ALPHA_1 )
			_REG(gpSVM_reg_ctrl->vu32AlphaMore[1], PP_FALSE);
		else if ( enType == eSVMDRV_BC_ADD_LUT_GRADIENT )
			_REG(gpSVM_reg_ctrl->vu32BcgradMore, PP_FALSE);
	}
	
	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetLUTType

 * param enType				:	type of LUT resolution

 * return					:	error code

 * brief
	Set lut type of fb and lr lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetLUTType(PP_SVMDRV_LUT_RESOLUTION_E IN enType)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enType >= eSVMDRV_LUT_RES_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( enType == eSVMDRV_LUT_RES_INPUT )
	{
		PPDRV_SVM_SetInOutResolution(gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Width, gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height,
									 gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Width, gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height);
		_REG(gpSVM_reg_ctrl->vu32Rmain, utilClearBit(gpSVM_reg_ctrl->vu32Rmain, 1));
	}
	else
	{
		PPDRV_SVM_SetInOutResolution(gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Width, gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].u16Height,
									 gstInOutInfo[gstSvmDrvCtrl.u8OutInfoIndex].u16Width, gstInOutInfo[gstSvmDrvCtrl.u8OutInfoIndex].u16Height);
		_REG(gpSVM_reg_ctrl->vu32Rmain, utilSetBit(gpSVM_reg_ctrl->vu32Rmain, 1));
	}
	
	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetLUTOffset

 * param u16XOffset			:	x offset of fb & lr lut
 * param u16YOffset			:	y offset of fb & lr lut

 * return					:	error code

 * brief
	Set the x and y offset of fb and lr lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetLUTOffset(PP_U16 IN u16XOffset, PP_U16 IN u16YOffset)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	u16XOffset = u16XOffset & utilBitMask(10);
	u16YOffset = u16YOffset & utilBitMask(10);

	_REG(gpSVM_reg_ctrl->vu32Lutoffset, (u16XOffset << 0) | (u16YOffset << 16) );

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBCLUTType

 * param enBitCnt			:	bit count of FB & LR LUT (5bit or 8bit)

 * return					:	error code

 * brief
	Sets the information of bc lut.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetBCLUTType(PP_SVMDRV_BCLUT_BIT_TYPE_E IN enBitCnt)
{	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enBitCnt >= eSVMDRV_BCLUT_BIT_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBit(gpSVM_reg_ctrl->vu32Fsync, 10, enBitCnt));
	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBit(gpSVM_reg_ctrl->vu32Fsync, 11, enBitCnt));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetOutputMode

 * param mode				:	output mode

 * return					:	error code

 * brief
	Sets the enOutputMode of the SVM.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetOutputMode(PP_SVMDRV_OUTPUTMODE_E enOutputMode)
{
	PP_BOOL bDynamicBlend = utilGetBit(gpSVM_reg_ctrl->vu32OutMode, 4);
	PP_U32 u32ChCtrl = 0x0, u32Outmode = 0x0;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enOutputMode >= eSVMDRV_OUTPUTMODE_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	switch (enOutputMode)
	{
	case eSVMDRV_OUTPUTMODE_BYPASS_FRONT:
		u32ChCtrl = 0x5;
		u32Outmode = 0x0;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_OUTPUT);
		break;
	case eSVMDRV_OUTPUTMODE_BYPASS_LEFT:
		u32ChCtrl = 0x5;
		u32Outmode = 0x1;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_OUTPUT);
		break;
	case eSVMDRV_OUTPUTMODE_BYPASS_RIGHT:
		u32ChCtrl = 0xd;
		u32Outmode = 0x1;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_OUTPUT);
		break;
	case eSVMDRV_OUTPUTMODE_BYPASS_REAR:
		u32ChCtrl = 0x7;
		u32Outmode = 0x0;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_OUTPUT);
		break;
	case eSVMDRV_OUTPUTMODE_QUAD:
		u32ChCtrl = 0x10;
		u32Outmode = 0x3;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_OUTPUT);
		break;
	case eSVMDRV_OUTPUTMODE_LUT_FB:
		u32ChCtrl = 0x0;
		u32Outmode = 0x6C;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_INPUT);
		break;
	case eSVMDRV_OUTPUTMODE_LUT_LR:
		u32ChCtrl = 0x0;
		u32Outmode = 0x6D;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_INPUT);
		break;
	case eSVMDRV_OUTPUTMODE_LUT_FB_LR:
		u32ChCtrl = 0x0;
		bDynamicBlend = PP_FALSE;
		u32Outmode = 0x03;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_INPUT);
		break;
	default:	// eSVMDRV_OUTPUTMODE_LUT_FB_LR_BC
		u32ChCtrl = 0x0;
		u32Outmode = 0x6F;
		PPDRV_SVM_CTRL_SetLUTType(eSVMDRV_LUT_RES_INPUT);
		break;
	}

	u32Outmode = utilPutBit(u32Outmode, 4, bDynamicBlend);
	_REG(gpSVM_reg_ctrl->vu32ChCtrl, utilPutBits(gpSVM_reg_ctrl->vu32ChCtrl, 0, 5, u32ChCtrl));
	_REG(gpSVM_reg_ctrl->vu32OutMode, utilPutBits(gpSVM_reg_ctrl->vu32OutMode, 0, 7, u32Outmode));

	if ( enOutputMode <= eSVMDRV_OUTPUTMODE_BYPASS_REAR &&
		 gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].bInterlace && !gstInOutInfo[gstSvmDrvCtrl.u8OutInfoIndex].bInterlace )
	{
		if ( gstSvmDrvCtrl.bFixedField )
			_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 14));
		else
			_REG(gpSVM_reg_ctrl->vu32Enable, utilSetBit(gpSVM_reg_ctrl->vu32Enable, 14));
	}

	PPDRV_SVM_IN_SetEnable(PP_TRUE, PP_TRUE, PP_TRUE, PP_TRUE);
	
	if ( enOutputMode == eSVMDRV_OUTPUTMODE_LUT_FB )
		PPDRV_SVM_OUT_SetEnable(PP_TRUE, PP_FALSE);
	else if ( enOutputMode == eSVMDRV_OUTPUTMODE_LUT_LR )
		PPDRV_SVM_OUT_SetEnable(PP_FALSE, PP_TRUE);
	else
		PPDRV_SVM_OUT_SetEnable(PP_TRUE, PP_TRUE);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBoundaryLine

 * param enBoundaryNumber	:	number of boundary (0~3)
 * param stBoundary			:	arae and position of boundary

 * return					:	error code

 * brief
	Sets the boundary of 2d top view.
	boundary is used instead of blending of 4 channels in 2D top view.
	4 channels are divided into boundary lines.
	Area requires whole area and invalied area.
	Position requires 2 for dynamic.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetBoundaryLine(PP_SVMDRV_BOUNDARY_LINE_NUMBER_E IN enBoundaryNumber, PP_SVM_BOUNDARY_LINE_S IN stBoundary)
{
	PP_S32 i;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( enBoundaryNumber >= eSVMDRV_BOUNDARY_LINE_NUM_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( stBoundary.bEnable[0] || stBoundary.bEnable[1] )
	{
		_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stArea[0].vu32X,
			((stBoundary.stArea.stWholeArea.u16X & utilBitMask(11)) << 0)  | (((stBoundary.stArea.stWholeArea.u16Width  - 1) & utilBitMask(11)) << 16));
		_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stArea[0].vu32Y,
			((stBoundary.stArea.stWholeArea.u16Y & utilBitMask(11)) << 0)  | (((stBoundary.stArea.stWholeArea.u16Height - 1) & utilBitMask(11)) << 16));

		_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stArea[1].vu32X,
			((stBoundary.stArea.stInvalidArea.u16X & utilBitMask(11)) << 0)  | (((stBoundary.stArea.stInvalidArea.u16Width - 1) & utilBitMask(11)) << 16));
		_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stArea[1].vu32Y,
			((stBoundary.stArea.stInvalidArea.u16Y & utilBitMask(11)) << 0)  | (((stBoundary.stArea.stInvalidArea.u16Height - 1) & utilBitMask(11)) << 16));
	}
	
	for ( i = 0; i < 2; i++ )
	{
		if ( stBoundary.bEnable[i] )
		{
			_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stPos[i].vu32Upleft,
				((stBoundary.stPos[i].stUpLeft.u16X & utilBitMask(11)) << 0)  | ((stBoundary.stPos[i].stUpLeft.u16Y & utilBitMask(11)) << 16));
			_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stPos[i].vu32Upright,
				((stBoundary.stPos[i].stUpRight.u16X & utilBitMask(11)) << 0)  | ((stBoundary.stPos[i].stUpRight.u16Y & utilBitMask(11)) << 16));
			_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stPos[i].vu32Downleft,
				((stBoundary.stPos[i].stDownLeft.u16X & utilBitMask(11)) << 0)  | ((stBoundary.stPos[i].stDownLeft.u16Y & utilBitMask(11)) << 16));
			_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stPos[i].vu32Downright,
				((stBoundary.stPos[i].stDownRight.u16X & utilBitMask(11)) << 0)  | ((stBoundary.stPos[i].stDownRight.u16Y & utilBitMask(11)) << 16));
		}
	}

	_REG(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stArea[0].vu32X,
		utilPutBits(gpSVM_reg_ctrl->stBoundaryLine[enBoundaryNumber].stArea[0].vu32X, 30, 2, (stBoundary.bEnable[0] << 0) | (stBoundary.bEnable[1] << 1)));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetRGBStatistics

 * param pstStats			:	porinter of statistics structur

 * return					:	PP_VOID

 * brief
	Gets statistics value from the output of SVM.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_GetRGBStatistics(PP_SVMDRV_RGB_STATISTICS_S* OUT pstStats)
{
	PP_S32 i = 0, j = 0, k = 0;

	if ( pstStats == PP_NULL )
		return _RET(eERROR_INVALID_ARGUMENT);
	
	for ( i = eSVMDRV_OUTPUT_PART_FB; i < eSVMDRV_OUTPUT_PART_MAX; i++ )
	{
		for ( j = eSVMDRV_SUBPART_FL; j < eSVMDRV_SUBPART_MAX; j++ )
		{
			for ( k = eSVMDRV_COLOR_R; k < eSVMDRV_COLOR_MAX; k++ )
				pstStats->u32Value[i][j][k] = gpSVM_reg_stat->vu32Stat[i][j][k] & utilBitMask(26);
		}
	}

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetLuminanceAverage

 * param pu16Front			:	luminance average of front
 * param pu16Left			:	luminance average of left
 * param pu16Rright			:	luminance average of right
 * param pu16Rear			:	luminance average of back

 * return					:	PP_VOID

 * brief
	Gets luminance average value from the output of SVM.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_GetLuminanceAverage(PP_U16* OUT pu16Front, PP_U16* OUT pu16Left, PP_U16* OUT pu16Right, PP_U16* OUT pu16Rear)
{
	*pu16Front = utilGetBits(gpSVM_reg_ctrl->vu32FbY, 0, 16);
	*pu16Rear = utilGetBits(gpSVM_reg_ctrl->vu32FbY, 16, 16);
	*pu16Left = utilGetBits(gpSVM_reg_ctrl->vu32LrY, 0, 16);
	*pu16Right = utilGetBits(gpSVM_reg_ctrl->vu32LrY, 16, 16);
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetEnable

 * param bEnable			:	enable or disable

 * return					:	error code

 * brief
	Enable or disable of the SVM. If set to disable, it is front bypass.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_CTRL_SetEnable(PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( bEnable && gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].bInterlace )
		_REG(gpSVM_reg_ctrl->vu32Enable, utilSetBit(gpSVM_reg_ctrl->vu32Enable, 4));
	else
		_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 4));

	_REG(gpSVM_reg_ctrl->vu32Enable, utilPutBit(gpSVM_reg_ctrl->vu32Enable, 0, bEnable));

	if ( !bEnable && gstInOutInfo[gstSvmDrvCtrl.u8InInfoIndex].bInterlace )
		_REG(gpSVM_reg_ctrl->vu32Enable, utilPutBit(gpSVM_reg_ctrl->vu32Enable, 4,  PP_FALSE));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_GetEnable

 * param bEnable			:	

 * return					:	enable or disable

 * brief
	Gets enable or disable of the SVM.
***************************************************************************************************************************************************************/
PP_BOOL PPDRV_SVM_CTRL_GetEnable(PP_VOID)
{
	return utilGetBit(gpSVM_reg_ctrl->vu32Enable, 0);
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetColorCoefficient

 * param stCoefColor		:	information of color coefficient

 * return					:	PP_VOID

 * brief
	Sets the color coefficient of the SVM.
	yuv to rgb or rgb to yuv
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_SetColorCoefficient(PP_SVM_COLORCOEF_S* IN pstCoefColor)
{
	_REG(gpSVM_reg_color->vu32R2y4y, (pstCoefColor->stR2y.u8R4y) | (pstCoefColor->stR2y.u8G4y << 8) | (pstCoefColor->stR2y.u8B4y << 16) | (pstCoefColor->bWith128 << 24));
	_REG(gpSVM_reg_color->vu32R2y4cb, (pstCoefColor->stR2y.u8R4cb) | (pstCoefColor->stR2y.u8G4cb << 8) | (pstCoefColor->stR2y.u8B4cb << 16));
	_REG(gpSVM_reg_color->vu32R2y4cr, (pstCoefColor->stR2y.u8R4cr) | (pstCoefColor->stR2y.u8G4cr << 8) | (pstCoefColor->stR2y.u8B4cr << 16));

	_REG(gpSVM_reg_color->vu32Y2r4g, (pstCoefColor->stY2r.u8Cb4g) | (pstCoefColor->stY2r.u8Cr4g << 8));
	_REG(gpSVM_reg_color->vu32Y2r4rnb, (pstCoefColor->stY2r.u16Cr4r & utilBitMask(9)) | ((pstCoefColor->stY2r.u16Cb4b & utilBitMask(9)) << 16));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetAddress

 * param enChannel			:	number of frame buffer
 * param pu32Addr			:	address

 * return					:	error code

 * brief
	Sets the memory address of output frame data.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetAddress(PP_SVMDRV_OUT_FRAMEBUF_NUM_E IN enOutFBNum, PP_U32* IN pu32Addr)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	if ( (((PP_U32)pu32Addr & 0xf) != 0x0) && (((PP_U32)pu32Addr & 0xf) != 0x8) )
		return _RET(eERROR_INVALID_ALIGN);

	if ( enOutFBNum < 2 )
		_REG(gpSVM_reg_ctrl->vu32OutAddr[enOutFBNum], pu32Addr);
	else
		_REG(gpSVM_reg_ctrl->vu32OutAddr2[enOutFBNum-2], pu32Addr);

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_GetAddress

 * param enChannel			:	number of frame buffer

 * return					:	memory address of input channel data

 * brief
	Gets the memory address of output frame data..
***************************************************************************************************************************************************************/
PP_U32* PPDRV_SVM_OUT_GetAddress(PP_SVMDRV_OUT_FRAMEBUF_NUM_E IN enOutFBNum)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return PP_NULL;

	if ( enOutFBNum < 2 )
		return (PP_U32*)gpSVM_reg_ctrl->vu32OutAddr[enOutFBNum];
	else
		return (PP_U32*)gpSVM_reg_ctrl->vu32OutAddr2[enOutFBNum-2];
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetReplaceColorEnable

 * param bOn				:	enable or disable

 * return					:	PP_VOID

 * brief
	Sets whether replace color is on or off.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_OUT_SetReplaceColorEnable(PP_BOOL IN bOn)
{
	_REG(gpSVM_reg_ctrl->vu32NoVideo, utilPutBit(gpSVM_reg_ctrl->vu32NoVideo, 4, bOn));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_GetReplaceColorEnable

 * param					:

 * return					:	

 * brief
	Gets whether replace color of output data is on or off.
***************************************************************************************************************************************************************/
PP_BOOL PPDRV_SVM_OUT_GetReplaceColorEnable(PP_VOID)
{
	return utilGetBit(gpSVM_reg_ctrl->vu32NoVideo, 4);
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_CTRL_SetBackgroundColor

 * param u8Y				:	y
 * param u8Cb				:	cb
 * param u8Cr				:	cr

 * return					:	PP_VOID

 * brief
	Sets the background color of the SVM.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_CTRL_SetBackgroundColor(PP_U8 IN u8Y, PP_U8 IN u8Cb, PP_U8 IN u8Cr)
{
	_REG(gpSVM_reg_ctrl->vu32BgColor, (u8Cr) | (u8Cb << 8) | (u8Y <<16));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetBlank

 * param u16HBlank			:	horizontal blank
 * param u16VBlank			:	vertical blank

 * return					:

 * brief
 	Sets horizontal and vertical blank of output.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_OUT_SetBlank(PP_U16 IN u16HBlank, PP_U16 IN u16VBlank)
{
	PP_U8 u8PipLineDelay = u16VBlank/2-4;
	PP_U16 u16OutLineDaly = 0;
	
	_REG(gpSVM_reg_ctrl->vu32OutBlank, (u16HBlank & utilBitMask(16)) | ((u16VBlank & utilBitMask(16)) << 16));
	_REG(gpSVM_reg_ctrl->vu32OutSyncCtrl, utilPutBits(gpSVM_reg_ctrl->vu32OutSyncCtrl, 0, 24, (u16OutLineDaly & utilBitMask(16)) | ((u8PipLineDelay & utilBitMask(8)) << 16)));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetWriteBurstLength

 * param enBurstLength		:	burst length (eSVMDRV_BURST_LENGTH_8 ~ eSVMDRV_BURST_LENGTH_128)

 * return					:	error code

 * brief
 	Sets the burst size of the write output dram.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetWriteBurstLength(PP_SVMDRV_BURST_LENGTH_E IN enBurstLength)
{
	PP_U8 u8Len = 0;

	switch ( enBurstLength )
	{
	case eSVMDRV_BURST_LENGTH_8:	u8Len = 8; break;
	case eSVMDRV_BURST_LENGTH_16:	u8Len = 16; break;
	case eSVMDRV_BURST_LENGTH_32:	u8Len = 32; break;
	case eSVMDRV_BURST_LENGTH_64:	u8Len = 64; break;
	case eSVMDRV_BURST_LENGTH_128:	u8Len = 128; break;
	default: return _RET(eERROR_INVALID_ARGUMENT);
	}
	
	_REG(gpSVM_reg_ctrl->vu32OutCtrl, utilPutBits(gpSVM_reg_ctrl->vu32OutCtrl, 0, 8, u8Len));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetReadBurstLength

 * param enBurstLength		:	burst length (eSVMDRV_BURST_LENGTH_8 ~ eSVMDRV_BURST_LENGTH_128)

 * return					:	error code

 * brief
 	Sets the burst size of the read output dram.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetReadBurstLength(PP_SVMDRV_BURST_LENGTH_E IN enBurstLength)
{
	PP_U8 u8Len = 0;

	switch ( enBurstLength )
	{
	case eSVMDRV_BURST_LENGTH_8:	u8Len = 8; break;
	case eSVMDRV_BURST_LENGTH_16:	u8Len = 16; break;
	case eSVMDRV_BURST_LENGTH_32:	u8Len = 32; break;
	case eSVMDRV_BURST_LENGTH_64:	u8Len = 64; break;
	case eSVMDRV_BURST_LENGTH_128:	u8Len = 128; break;
	default: return _RET(eERROR_INVALID_ARGUMENT);
	}
	
	_REG(gpSVM_reg_ctrl->vu32OutCtrl, utilPutBits(gpSVM_reg_ctrl->vu32OutCtrl, 16, 8, u8Len));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetHoldTime

 * param u8HoldTime			:	hold time

 * return PP_VOID			:

 * brief
 	Sets the hold time of the output dram.
	It guarantees priority of read operation by delaying write.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_OUT_SetHoldTime(PP_U8 IN u8HoldTime)
{
	_REG(gpSVM_reg_ctrl->vu32OutCtrl, utilPutBits(gpSVM_reg_ctrl->vu32OutCtrl, 8, 8, u8HoldTime));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetSection

 * param enSectionNumber	:	number of section for output
 * param stSectionRect		:	rect of section

 * return					:	error code

 * brief
	Sets the section of the SVM.
	Section is a function that outputs only a specific rectangle.

	ex)
	------------------------------
	|                            | output area
	|  ------------              |
	|  |          |              |
	|  |section0  |  ------------|
	|  |          |  |           |
	|  ------------  | section1  | Output only the section area.
	|                |           |
	------------------------------
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetSection(PP_SVMDRV_SECTION_NUMBER_E IN enSectionNumber, PP_RECT_S IN stSectionRect)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

 	if ( enSectionNumber >= eSVMDRV_SECTION_NUM_MAX ||
		 (stSectionRect.u16X + stSectionRect.u16Width > gstInOutInfo[gstSvmDrvCtrl.u8OutInfoIndex].u16Width) || (stSectionRect.u16Y + stSectionRect.u16Height) > gstInOutInfo[gstSvmDrvCtrl.u8OutInfoIndex].u16Height )
		return _RET(eERROR_INVALID_ARGUMENT);

	switch (enSectionNumber)
	{
	case eSVMDRV_SECTION_NUM_0:
		_REG(gpSVM_reg_ctrl->vu32Section0X, (stSectionRect.u16X & utilBitMask(11)) | (((stSectionRect.u16Width - 1) & utilBitMask(11)) << 16));
		_REG(gpSVM_reg_ctrl->vu32Section0Y, (stSectionRect.u16Y & utilBitMask(11)) | (((stSectionRect.u16Height - 1) & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32Section0Y, 31) << 31));
		break;
	case eSVMDRV_SECTION_NUM_1:
		_REG(gpSVM_reg_ctrl->vu32Section1X, (stSectionRect.u16X & utilBitMask(11)) | (((stSectionRect.u16Width - 1) & utilBitMask(11)) << 16));
		_REG(gpSVM_reg_ctrl->vu32Section1Y, (stSectionRect.u16Y & utilBitMask(11)) | (((stSectionRect.u16Height - 1) & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32Section1Y, 31) << 31));
		break;
	case eSVMDRV_SECTION_NUM_2:
		_REG(gpSVM_reg_ctrl->vu32Section2X, (stSectionRect.u16X & utilBitMask(11)) | (((stSectionRect.u16Width - 1) & utilBitMask(11)) << 16));
		_REG(gpSVM_reg_ctrl->vu32Section2Y, (stSectionRect.u16Y & utilBitMask(11)) | (((stSectionRect.u16Height - 1) & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32Section2Y, 31) << 31));
		break;
	case eSVMDRV_SECTION_NUM_3:
		_REG(gpSVM_reg_ctrl->vu32Section3X, (stSectionRect.u16X & utilBitMask(11)) | (((stSectionRect.u16Width - 1) & utilBitMask(11)) << 16));
		_REG(gpSVM_reg_ctrl->vu32Section3Y, (stSectionRect.u16Y & utilBitMask(11)) | (((stSectionRect.u16Height - 1) & utilBitMask(11)) << 16) | (utilGetBit(gpSVM_reg_ctrl->vu32Section3Y, 31) << 31));
		break;
	default:
		break;
	}

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetSectionEnable

 * param enSectionNumber	:	number of section for output
 * param bEnable			:	enable or disable

 * return					:	error code

 * brief
	Enable or disable of the section function.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetSectionEnable(PP_SVMDRV_SECTION_NUMBER_E IN enSectionNumber, PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

 	if ( enSectionNumber >= eSVMDRV_SECTION_NUM_MAX )
		return _RET(eERROR_INVALID_ARGUMENT);

	switch (enSectionNumber)
	{
	case eSVMDRV_SECTION_NUM_0:
		_REG(gpSVM_reg_ctrl->vu32Section0Y, utilPutBit(gpSVM_reg_ctrl->vu32Section0Y, 31, bEnable));
		break;
	case eSVMDRV_SECTION_NUM_1:
		_REG(gpSVM_reg_ctrl->vu32Section1Y, utilPutBit(gpSVM_reg_ctrl->vu32Section1Y, 31, bEnable));
		break;
	case eSVMDRV_SECTION_NUM_2:
		_REG(gpSVM_reg_ctrl->vu32Section2Y, utilPutBit(gpSVM_reg_ctrl->vu32Section2Y, 31, bEnable));
		break;
	case eSVMDRV_SECTION_NUM_3:
		_REG(gpSVM_reg_ctrl->vu32Section3Y, utilPutBit(gpSVM_reg_ctrl->vu32Section3Y, 31, bEnable));
		break;
	default:
		break;
	}

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function				:	PPDRV_SVM_ShadowReg

 * param bEnable		:	enable or disable

 * return				:	PP_VOID

 * brief
	Sets the shadow register.
***************************************************************************************************************************************************************/

PP_VOID PPDRV_SVM_CTRL_ShadowReg(PP_BOOL bEnable)
{
	if ( utilGetBit(gpSVM_reg_ctrl->vu32Enable, 0) )
		_REG(gpSVM_reg_ctrl->vu32Enable, utilPutBit(gpSVM_reg_ctrl->vu32Enable, 6, bEnable));
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetTilt

 * param s8X				:	x offset position
 * param s8Y				:	y offset position

 * return					:	error code

 * brief
 	Set the output offset.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetTilt(PP_S8 IN s8X, PP_S8 IN s8Y)
{
	PP_U8 u8Xoffset, u8Yoffset;
	PP_BOOL isPlusX = PP_FALSE, isPlusY = PP_FALSE;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	u8Xoffset = abs(s8X);
	u8Yoffset = abs(s8Y);

	if ( u8Xoffset > 63 || u8Xoffset < -63 || u8Yoffset > 63 || u8Yoffset < -63 )
		return _RET(eERROR_INVALID_ARGUMENT);

	if ( s8X < 0 )
		isPlusX = PP_TRUE;
	if ( s8Y < 0 )
		isPlusY = PP_TRUE;

	_REG(gpSVM_reg_ctrl->vu32Winoffset, (u8Xoffset << 0) | (isPlusX << 7) | (u8Yoffset << 8) | (isPlusY << 15));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function								:	PPDRV_SVM_OUT_SetHold

 * param bEnable						:	enable or disable

 * return								:	error code

 * brief
	Pause the output.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetHold(PP_BOOL IN bEnable)
{
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);
	
	_REG(gpSVM_reg_ctrl->vu32Rmain, utilPutBit(gpSVM_reg_ctrl->vu32Rmain, 9, bEnable));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_GetHoldOddFrameBufferAddress

 * param enFiled			:	odd (progressive or odd of interlace) or even (even of interlace)

 * return					:	framebuffer address

 * brief
	Get the framebuffer address when the output hold.
***************************************************************************************************************************************************************/
PP_U32* PPDRV_SVM_OUT_GetHoldFrameBufferAddress(PP_FIELD_E IN enField)
{
	PP_U8 u8FBNum;
	
	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return PP_NULL;

	if ( enField == eFIELD_ODD )
		u8FBNum = utilGetBits(gpSVM_reg_ctrl->vu32Interrupt, 8, 2);
	else if ( enField == eFIELD_EVEN )
		u8FBNum = utilGetBits(gpSVM_reg_ctrl->vu32Interrupt, 10, 2);

	if ( u8FBNum < 2 )
		return (PP_U32*)gpSVM_reg_ctrl->vu32OutAddr[u8FBNum];
	else
		return (PP_U32*)gpSVM_reg_ctrl->vu32OutAddr2[u8FBNum-2];
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_OUT_SetEnable

 * param bFBEnable			:	enable or disable of front/back output data
 * param bLREnable			:	enable or disable of left/right output data

 * return					:	error code

 * brief
	Enable or disable of the output data.
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_OUT_SetEnable(PP_BOOL IN bFBEnable, PP_BOOL IN bLREnable)
{
	PP_U8 u8Burst = 0x0;
	PP_BOOL bMode = 0x0;
	PP_BOOL bHold = 0x0;

	if ( gstSvmDrvCtrl.bInit == PP_FALSE )
		return _RET(eERROR_SVM_NOT_INITIALIZE);

	u8Burst = utilGetBits(gpSVM_reg_ctrl->vu32Rmain, 4, 4);
	bMode = utilGetBit(gpSVM_reg_ctrl->vu32Rmain, 1);
	bHold = utilGetBit(gpSVM_reg_ctrl->vu32Rmain, 9);

	_REG(gpSVM_reg_ctrl->vu32Rmain, (bFBEnable << 0) | bMode << 1 | ((u8Burst & utilBitMask(4)) << 4) | (bLREnable << 8) | (bHold << 9));

	return eSUCCESS;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_ISR_SetVsyncHandler

 * param callback			:	pointer function for callback

 * return					:

 * brief
	Sets pointer callback of the output vsync.
	If callbac is PP_NULL, the interrupt is disabled.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_ISR_SetVsyncHandler(PPDRV_SVM_CALLBACK_VSYNC IN callback)
{
	gpSVM_cbVsync = callback;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_ISR_SetErrorHandler

 * param callback			:	pointer function for callback

 * return					:

 * brief
	Sets pointer callback of the interrupt for svm.
	If callbac is PP_NULL, the interrupt is disabled.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_ISR_SetErrorHandler(PPDRV_SVM_CALLBACK_ERROR IN callback)
{	
	gpSVM_cbError = callback;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_SetUnderflowCtrlHandler

 * param u32CheckCnt		:	count to check underflow error 
 * param callback			:	pointer function for callback

 * return					:

 * brief
	Set pointer callback of the unerflow error.
	The u32CtrlCnt is 0 or callbac is PP_NULL, the underflow check is disabled.
***************************************************************************************************************************************************************/
PP_VOID PPDRV_SVM_SetUnderflowCtrlHandler(PP_U32 IN u32CheckCnt, PPDRV_SVM_CALLBACK_UNDERFLOW IN callback)
{
	gstUnderflowCtrl.u32UnderflowCtrlCheckCnt = u32CheckCnt;
	gstUnderflowCtrl.pCb = callback;
	gstUnderflowCtrl.bUnderflowCtrlStatus = PP_FALSE;
	gstUnderflowCtrl.u32UnderflowCtrlCnt = 0;
}

/***************************************************************************************************************************************************************
 * function					:	PPDRV_SVM_GetUnderflowCtrlContinuousCount

 * param callback			:

 * return					:	continuous count of the underflow error

 * brief
	Get continuous count of the unerflow error. 
***************************************************************************************************************************************************************/
PP_U32 PPDRV_SVM_GetUnderflowCtrlContinuousCount(PP_VOID)
{
	return gstUnderflowCtrl.u32UnderflowCtrlCnt;
}

/***************************************************************************************************************************************************************
 * function					:	SVMAPI_initialize

 * param enInput			:	enumeration of input Resolution
 * param enOutput			:	enumeration of output Resolution
 * param u8InFrameBufCnt	:	count of input frame buffer
 * param u8OutFrameBufCnt	:	count of output frame buffer

 * return					:	Error code

 * brief
	Initializes the SVM(Surround View Monitor).
	Memory size of each channel = u16InputWidth * u16InputHeight * pixel size of yuv422 format * u8InputFrameCount
***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_SVM_Initialize(_VID_RESOL enInput, _VID_RESOL enOutput, PP_U8 u8InFrameBufCnt, PP_U8 u8OutFrameBufCnt)
{
	PP_U16 u16InputLine = 0;
	sys_os_isr_t *pstOldIsr = PP_NULL;
	PP_U32 u32DUClock;
	
	if ( gstSvmDrvCtrl.bInit ) return eSUCCESS;
	if ( enInput >= max_vid_resol || enOutput >= max_vid_resol ) return _RET(eERROR_INVALID_ARGUMENT);
	if ( u8InFrameBufCnt < 3 || u8OutFrameBufCnt < 2 || u8OutFrameBufCnt > 4 ) return _RET(eERROR_INVALID_ARGUMENT);

	memset(&gstSvmDrvCtrl, 0, sizeof(PP_SVMDRV_CTRL_S));

	u32DUClock = PPDRV_SVM_GetDUClock();
	if ( u32DUClock == 47250000 )
	{
		if ( enOutput == vres_1280x960p30 )
		{
			gstInOutInfo[enOutput].u16HBlank = 120;
			gstInOutInfo[enOutput].u16VBlank = 165;
		}
		else if ( enOutput == vres_1280x960p25 )
		{
			gstInOutInfo[enOutput].u16HBlank = 400;
			gstInOutInfo[enOutput].u16VBlank = 165;
		}
		else
		{
			return _RET(eERROR_INVALID_ARGUMENT);
		}
	}

	if ( gstInOutInfo[enInput].u8Framerate != gstInOutInfo[enOutput].u8Framerate )
	{
		if ( gstInOutInfo[enInput].u8Framerate > gstInOutInfo[enOutput].u8Framerate && gstInOutInfo[enInput].u8Framerate % gstInOutInfo[enOutput].u8Framerate == 0 )
		{
			if ( gstInOutInfo[enInput].bInterlace && !gstInOutInfo[enOutput].bInterlace )	// interlace to progressive
				gstSvmDrvCtrl.bFixedField = PP_TRUE;
		}
	}

	if ( gstSvmDrvCtrl.bFixedField )
		_REG(gpSVM_reg_ctrl->vu32Enable, utilSetBit(gpSVM_reg_ctrl->vu32Enable, 12));
	else
		_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 12));

	if ( gstInOutInfo[enInput].bInterlace )
	{
		_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 9));
		if ( gstInOutInfo[enInput].u8Framerate == 60 )
			_REG(gpSVM_reg_ctrl->vu32Enable, utilSetBit(gpSVM_reg_ctrl->vu32Enable, 16));
		else
			_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 16));
	}
	else
	{
		_REG(gpSVM_reg_ctrl->vu32Enable, utilSetBit(gpSVM_reg_ctrl->vu32Enable, 9));
		_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 16));
	}

	PPDRV_SVM_IN_SetBlank(gstInOutInfo[enOutput].u16HBlank, gstInOutInfo[enOutput].u16VBlank);
	PPDRV_SVM_OUT_SetBlank(gstInOutInfo[enOutput].u16HBlank, gstInOutInfo[enOutput].u16VBlank);

	gstSvmDrvCtrl.u8InInfoIndex = enInput;
	gstSvmDrvCtrl.u8OutInfoIndex = enOutput;
	gstSvmDrvCtrl.bInit = PP_TRUE;

	PPDRV_SVM_SetResolution(gstInOutInfo[enInput].u16Width, gstInOutInfo[enInput].u16Height, gstInOutInfo[enOutput].u16Width, gstInOutInfo[enOutput].u16Height);

	/* burst size of write main */
	PPDRV_SVM_IN_SetBurstLength(eSVMDRV_BURST_LENGTH_128);

	/* burst size of output buffer */
	PPDRV_SVM_OUT_SetWriteBurstLength(eSVMDRV_BURST_LENGTH_64);
	PPDRV_SVM_OUT_SetReadBurstLength(eSVMDRV_BURST_LENGTH_64);

	/* burst size of image */
	PPDRV_SVM_CTRL_SetImageBurstLength(eSVMDRV_IMG_NUM_0, 64);
	PPDRV_SVM_CTRL_SetImageBurstLength(eSVMDRV_IMG_NUM_1, 64);

	/* write hold time for scale down case */
	PPDRV_SVM_IN_SetHoldTime(32, 300, 600, 750);
	PPDRV_SVM_OUT_SetHoldTime(8);

	PPDRV_SVM_UpdateInput();

	PPDRV_SVM_CTRL_SetAutoMorphingSpeed(5);
	PPDRV_SVM_SetMaskColor(0x0, 0x0);

	PPDRV_SVM_CTRL_SetEdgeEnhancementMode(eSVMDRV_EDGE_MODE_FIXED_GAIN);

	PPDRV_SVM_CTRL_SetBCLUTType(eSVMDRV_BCLUT_8BIT);

	if ( gstInOutInfo[enInput].bInterlace )
		PPDRV_SVM_CTRL_SetLUTOffset(30, 135);
	else
		PPDRV_SVM_CTRL_SetLUTOffset(60, 270);

	PPDRV_SVM_CTRL_SetBCRatio(0x80);

	/* wrapping line */
	u16InputLine = gstInOutInfo[enInput].u16Height * u8InFrameBufCnt;
	u16InputLine = u16InputLine & utilBitMask(13);
	_REG(gpSVM_reg_ctrl->vu32Wline0, (u16InputLine << 0) | (u16InputLine << 16));
	_REG(gpSVM_reg_ctrl->vu32Wline1, (u16InputLine << 0) | (u16InputLine << 16));
	
	_REG(gpSVM_reg_ctrl->vu32OutCtrl, utilPutBits(gpSVM_reg_ctrl->vu32OutCtrl, 24, 2, u8OutFrameBufCnt-2));

	/* fixed value */
	_REG(gpSVM_reg_ctrl->vu32Enable, utilClearBit(gpSVM_reg_ctrl->vu32Enable, 5));
	_REG(gpSVM_reg_ctrl->vu32Enable, utilSetBit(gpSVM_reg_ctrl->vu32Enable, 1));
	_REG(gpSVM_reg_ctrl->vu32OutSyncCtrl, utilSetBit(gpSVM_reg_ctrl->vu32OutSyncCtrl, 24));
	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBits(gpSVM_reg_ctrl->vu32Fsync, 0, 8, 0x18));
	_REG(gpSVM_reg_ctrl->vu32OutMode, utilSetBit(gpSVM_reg_ctrl->vu32OutMode, 6));		// Enable BC range

	/* input frame rate control */
#if 0
	_REG(gpSVM_reg_ctrl->vu32InSyncCtrl, utilPutBits(gpSVM_reg_ctrl->vu32InSyncCtrl, 24, 8, 128));	// 1/2 frame
	_REG(gpSVM_reg_ctrl->vu32Enable, utilPutBit(gpSVM_reg_ctrl->vu32Enable, 2, 1));					// on
#else
	_REG(gpSVM_reg_ctrl->vu32Enable, utilPutBit(gpSVM_reg_ctrl->vu32Enable, 2, 0));					// off
#endif

#if 0
	/* ALPHA_OFFSET & BCGRAD_OFFSET */
	_REG(gpSVM_reg_ctrl->vu32Fsync, utilPutBits(gpSVM_reg_ctrl->vu32Fsync, 13, 2, 0x3));
#endif

	/* Interrupt */
	OSAL_register_isr(IRQ_SVM_VECTOR, svm_error_isr, pstOldIsr);
	INTC_irq_config(IRQ_SVM_VECTOR, IRQ_EDGE_TRIGGER);
    INTC_irq_clean(IRQ_SVM_VECTOR);
	INTC_irq_enable(IRQ_SVM_VECTOR);
	_REG(gpSVM_reg_ctrl->vu32Interrupt, utilPutBits(gpSVM_reg_ctrl->vu32Interrupt, 0, 5, 0x0));
	_REG(gpSVM_reg_ctrl->vu32Interrupt, utilSetBit(gpSVM_reg_ctrl->vu32Interrupt, 31));

	/* proc */
	SYS_PROC_addIrq(stSVM_PROC_irqs);
	SYS_PROC_addDevice(stPPDRV_SVM_PROC_DEVICEs);

	return eSUCCESS;
}
