/**
 * \file
 *
 * \brief	Canny Edge Detector driver
 *
 * Copyright (c) 2017 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#include <stdio.h>
#include <nds32_intrinsic.h>

#include "pi5008.h"
#include "osal.h"
#include "utils.h"
#include "vin.h"
#include "interrupt.h"
#include "proc.h"
#include "api_vin.h"
#include "sys_api.h"
#include "cannyedge_register.h"
#include "cannyedge_drv.h"


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#ifdef CANNYEDGE_VECTOR_MSG
#define _REG(x, y)						PPDRV_CANNYEDGE_SetReg((PP_U32)&x, (PP_U32)y)
#else
#define _REG(x, y)						((*(PP_VU32*)(&x)) = (PP_U32)y)
#endif

PP_RESULT_E PPDRV_CANNYEDGE_PROC_ERR_IRQ(sint32 argc, const char **argv);
PP_RESULT_E PPDRV_CANNYEDGE_PROC_DEVICE(sint32 argc, const char **argv);


/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Global Variable
***************************************************************************************************************************************************************/
STATIC PP_CANNYEDGE_REG_S *gpstCANNYEDGE_REG = CANNYEDGE_REG;
STATIC VIN_REG_T *gpstVIN_reg = VIN_CTRL_REG;

STATIC PP_BOOL gbEdge_isInit = FALSE;
STATIC PP_CANNYEDGE_INPUT_E genEdge_curCh = eCANNYEDGE_INPUT_MAX;

STATIC PPDRV_CANNYEDGE_CALLBACK_DONE gpEdge_cbDone = NULL;

STATIC PP_U32 *gu32Edge_outBuf = NULL;
STATIC PP_U16 gu16Edge_vinWidth;
STATIC PP_U16 gu16Edge_vinHeight;
STATIC PP_U16 gu16Edge_voutWidth;
STATIC PP_U16 gu16Edge_voutHeight;
STATIC PP_U16 gu16Edge_inWidth;
STATIC PP_U16 gu16Edge_inHeight;
STATIC PP_U16 gu16Edge_outWidth;
STATIC PP_U16 gu16Edge_outHeight;
STATIC PP_U8  gu8Edge_frameCnt;

STATIC PP_U32 gu32Edge_capturedone = 0;
struct proc_irq_struct stEdge_proc_irqs[]		= {{ .fn = PPDRV_CANNYEDGE_PROC_ERR_IRQ,	.irqNum = IRQ_EDGE_VECTOR,	.next = (PP_VOID*)0, },};
struct proc_device_struct stEdge_proc_devices[]	= {{ .pName = "EDGE",  .fn = PPDRV_CANNYEDGE_PROC_DEVICE,	.next = (PP_VOID*)0, },};

/*======================================================================================================================
 = PROC function
======================================================================================================================*/
PP_RESULT_E PPDRV_CANNYEDGE_PROC_ERR_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	PRINT_PROC_IRQ("EDGE Capture done", IRQ_EDGE_VECTOR, gu32Edge_capturedone);

	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	PP_CANNYEDGE_THRESHOLD_TYPE_E enTHType;
	PP_F32 f32ThHigh, f32ThLow;
	PP_U32 u32Reg;

	if( (argc) && (strcmp(argv[0], stEdge_proc_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return eERROR_FAILURE;
	}
	
	printf("\n%s Device Info -------------\n", stEdge_proc_devices[0].pName);

	printf("---------------------- EDGE info --------------------------------------------------\n");
	printf("  Initialized: %s\n", gbEdge_isInit?"Success":"Fail");
	printf("  Excute: %s\n\n", utilGetBit(gpstCANNYEDGE_REG->vu32Enable, 0)?"Yes":"No");

	if ( !gbEdge_isInit ) return eSUCCESS;

	enTHType = utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 0);
	if ( enTHType == eCANNYEDGE_TH_TYPE_RATIO )
	{
		u32Reg = gpstCANNYEDGE_REG->vu32Thbnd;
		f32ThHigh = utilGetBits(u32Reg, 0, 8) / 256.f;
		f32ThLow = utilGetBits(u32Reg, 8, 8) / 256.f;
	}
	else
	{
		u32Reg = gpstCANNYEDGE_REG->vu32Thval;
		f32ThHigh = utilGetBits(u32Reg, 16, 8);
		f32ThLow = utilGetBits(u32Reg, 24, 8);
	}
	
	printf("  bCapture  ChN    Size        ROI    HScale  VScale  frameCnt  ThType  ThHigh  ThLow\n");
	printf("      %d      %d   %4dx%-4d  %4d,%-4d   %d       %d         %d        %d    %5.2f  %5.2f\n",
								(PP_S32)utilGetBit(gpstCANNYEDGE_REG->vu32Enable, 1),
								genEdge_curCh,
								gu16Edge_inWidth, gu16Edge_inHeight,
								(PP_S32)utilGetBits(gpstCANNYEDGE_REG->vu32Roi, 0, 11), (PP_S32)utilGetBits(gpstCANNYEDGE_REG->vu32Roi, 16, 11)-1,
								(PP_S32)utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 1), (PP_S32)utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 2),
								gu8Edge_frameCnt,
								enTHType, f32ThHigh, f32ThLow);

	return eSUCCESS;
}

/*======================================================================================================================
 = callback function
======================================================================================================================*/
ISR(CannyEdge_isr, num)
{
	PP_U32 u32Msk = (1 << num);
	
	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~u32Msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(u32Msk, NDS32_SR_INT_PEND2);
	if ( utilGetBit(gpstCANNYEDGE_REG->vu32Enable, 2) )
	{
		gu32Edge_capturedone++;
		if ( gpEdge_cbDone ) gpEdge_cbDone();
		gpstCANNYEDGE_REG->vu32Enable = utilClearBit(gpstCANNYEDGE_REG->vu32Enable, 2);
	}

	if ( utilGetBit(gpstCANNYEDGE_REG->vu32Enable, 3) )
	{
		printf("[EDGE] overflow\n");
		gpstCANNYEDGE_REG->vu32Enable = utilClearBit(gpstCANNYEDGE_REG->vu32Enable, 3);
	}
	
	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | u32Msk, NDS32_SR_INT_MASK2);
}

/*======================================================================================================================
 = Local function
======================================================================================================================*/
#ifdef CANNYEDGE_VECTOR_MSG
/*
 * function				:	PPDRV_CANNYEDGE_SetReg

 * param ret			:	register address
 * param line			:	register value

 * return				:

 * brief
	sets register and vector message.
*/
PP_VOID PPDRV_CANNYEDGE_SetReg(PP_U32 u32Addr, PP_U32 u32Value)
{
	PP_U32 u32AddrMsg = u32Addr - CAN_BASE_ADDR + 0x40000;
	PP_U32 u32ValueMsg = u32Value;
	printf("APB_WRITE(19'h%x, 32'h%08x);\n", u32AddrMsg, u32ValueMsg);

	(*((PP_VU32 *)(u32Addr))) = u32Value;
}
#endif

/*======================================================================================================================
 = Export function
======================================================================================================================*/
PP_RESULT_E PPDRV_CANNYEDGE_SetInput(PP_CANNYEDGE_INPUT_E IN enCh)
{
	PP_BOOL bHScale, bVScale;
	PP_U16 u16Roi[2];
	PP_BOOL bPreEnable = utilClearBit(gpstCANNYEDGE_REG->vu32Enable, 0);
	PP_U32 u32PreW = gu16Edge_inWidth;
	PP_U32 u32PreH = gu16Edge_inHeight;
	PP_S16 s16VinCh = -1;
	PP_S16 s16EdgeCh = -1;
	
	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}
	
	if ( enCh >= eCANNYEDGE_INPUT_MAX )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
		return eERROR_INVALID_ARGUMENT;
	}

	if ( genEdge_curCh == enCh ) return eSUCCESS;

	bHScale = utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 1);
	bVScale = utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 2);
	u16Roi[0] = utilGetBits(gpstCANNYEDGE_REG->vu32Roi, 0, 11);
	u16Roi[1] = utilGetBits(gpstCANNYEDGE_REG->vu32Roi, 16, 11) - 1;

	if ( enCh == eCANNYEDGE_INPUT_SVM )
	{
		if ( PPDRV_SVM_CTRL_GetEnable() )
		{
			gu16Edge_inWidth = gu16Edge_voutWidth;
			gu16Edge_inHeight = gu16Edge_voutHeight;
			gu16Edge_outWidth = gu16Edge_voutWidth;
			gu16Edge_outHeight = gu16Edge_voutHeight;
		}
		else
		{
			gu16Edge_inWidth = gu16Edge_vinWidth;
			gu16Edge_inHeight = gu16Edge_vinHeight;
			gu16Edge_outWidth = gu16Edge_vinWidth;
			gu16Edge_outHeight = gu16Edge_vinHeight;
		}

		s16EdgeCh = 0x4;
	}
	else if ( enCh == eCANNYEDGE_INPUT_QUAD )
	{
		PP_S32 quadWidth, quadHeight;
		_VID_RESOL eQuadResol;

        if ( PPAPI_VIN_GetResol(BD_QUAD_OUT_FMT, &quadWidth, &quadHeight, &eQuadResol) != eSUCCESS )
        {
            LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }

        gu16Edge_inWidth = quadWidth;
        gu16Edge_inHeight = quadHeight;
		gu16Edge_outWidth = quadWidth;
		gu16Edge_outHeight = quadHeight;

		s16VinCh = 0x5;
		s16EdgeCh = 0x5;
	}
	else
	{
		s16EdgeCh = 0x6;
		
		switch ( enCh )
		{
		case eCANNYEDGE_INPUT_CH0:
			s16VinCh = 0x0;
			break;
		case eCANNYEDGE_INPUT_CH1:
			s16VinCh = 0x1;
			break;
		case eCANNYEDGE_INPUT_CH2:
			s16VinCh = 0x2;
			break;
		case eCANNYEDGE_INPUT_CH3:
			s16VinCh = 0x3;
			break;
		default:
			break;
		}
		
		gu16Edge_inWidth = gu16Edge_vinWidth;
		gu16Edge_inHeight = gu16Edge_vinHeight;
		gu16Edge_outWidth = gu16Edge_vinWidth;
		gu16Edge_outHeight = gu16Edge_vinHeight;
	}

	genEdge_curCh = enCh;

	if ( u32PreW != gu16Edge_inWidth || u32PreH != gu16Edge_inHeight )
	{
		if ( bPreEnable )
			gpstCANNYEDGE_REG->vu32Enable = utilClearBit(gpstCANNYEDGE_REG->vu32Enable, 0);
		
		_REG(gpstCANNYEDGE_REG->vu32Size, (((gu16Edge_inHeight - 1) & utilBitMask(11)) << 0) | (((gu16Edge_inWidth - 1) & utilBitMask(11)) << 16));
		_REG(gpstCANNYEDGE_REG->vu32Pcnt, (gu16Edge_inWidth * gu16Edge_inHeight) & utilBitMask(21));
		_REG(gpstCANNYEDGE_REG->vu32Wline, utilPutBits(gpstCANNYEDGE_REG->vu32Wline, 0, 13, (gu16Edge_inHeight*gu8Edge_frameCnt) & utilBitMask(13)));
	}

	if ( s16VinCh >= 0 )
		gpstVIN_reg->rec_ch_sel = utilPutBits(gpstVIN_reg->rec_ch_sel, 16, 4, s16VinCh);

	if ( s16EdgeCh >= 0 )
		_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBits(gpstCANNYEDGE_REG->vu32Ctrl, 4, 3, s16EdgeCh));

	if ( u16Roi[1] > gu16Edge_inHeight - 1 || u16Roi[1] == u32PreH - 1 )
	{
		u16Roi[1] = gu16Edge_inHeight - 1;
		if ( u16Roi[0] >= u16Roi[1] )
			u16Roi[0] = 0;

		PPDRV_CANNYEDGE_SetROI(u16Roi[0], u16Roi[1]);
	}

	if ( bPreEnable && utilGetBit(gpstCANNYEDGE_REG->vu32Enable, 0) == FALSE && (u32PreW != gu16Edge_inWidth || u32PreH != gu16Edge_inHeight) )
		gpstCANNYEDGE_REG->vu32Enable = utilSetBit(gpstCANNYEDGE_REG->vu32Enable, 0);
	
	if ( bHScale )
		gu16Edge_outWidth /= 2;

	gu16Edge_outHeight = u16Roi[1] + 1 - u16Roi[0];
	if ( bVScale )
		gu16Edge_outHeight /= 2;
	
	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetDownScale(PP_BOOL IN bHScale, PP_BOOL IN bVScale)
{
	PP_BOOL bPreHScale, bPreVScale;

	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}

	bPreHScale = utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 1);
	bPreVScale = utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 2);
	
	_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBits(gpstCANNYEDGE_REG->vu32Ctrl, 1, 2, (bHScale << 0) | (bVScale << 1)));
	
	if ( bPreHScale != bHScale )
	{
		if ( bPreHScale )
			gu16Edge_outWidth *= 2;
		else
			gu16Edge_outWidth /= 2;
	}

	if ( bPreVScale != bVScale )
	{
		if ( bPreVScale )
			gu16Edge_outHeight *= 2;
		else
			gu16Edge_outHeight /= 2;
	}

	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetROI(PP_U16 IN u16StartLine, PP_U16 IN u16EndLine)
{
	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}

	if ( u16StartLine > u16EndLine || u16EndLine >= gu16Edge_inHeight )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
		return eERROR_INVALID_ARGUMENT;
	}
	
	_REG(gpstCANNYEDGE_REG->vu32Roi, ((u16StartLine & utilBitMask(11)) << 0) | (((u16EndLine+1) & utilBitMask(11)) << 16));

	gu16Edge_outHeight  = u16EndLine - u16StartLine + 1;

	if ( utilGetBit(gpstCANNYEDGE_REG->vu32Ctrl, 2) )
		gu16Edge_outHeight /= 2;
	
	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetThresholdRatio(PP_F32 IN f32HighTh, PP_F32 IN f32LowTh)
{
	PP_U8 u8HighVal, u8LowVal;

	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}
	
	if ( f32HighTh <= f32LowTh || f32HighTh >= 1.0f || f32LowTh >= 1.0f )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
		return eERROR_INVALID_ARGUMENT;
	}

	u8HighVal = (PP_U8)(f32HighTh * 256.f);
	u8LowVal = (PP_U8)(f32LowTh * 256.f);
	
	_REG(gpstCANNYEDGE_REG->vu32Thbnd, (u8HighVal << 0) | (u8LowVal << 8 ));
	
	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetFixedThreshold(PP_U8 IN u8HighTh, PP_U8 IN u8LowTh)
{
	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}
	
	if ( u8HighTh <= u8LowTh || u8HighTh > 0xff || u8LowTh > 0xff )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
		return eERROR_INVALID_ARGUMENT;
	}
	
	_REG(gpstCANNYEDGE_REG->vu32Thval, utilPutBits(gpstCANNYEDGE_REG->vu32Thval, 16, 16, (u8HighTh << 0) | (u8LowTh << 8)));
	
	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetThresholdType(PP_CANNYEDGE_THRESHOLD_TYPE_E IN enType)
{
	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}
	
	if ( enType >= eCANNYEDGE_TH_TYPE_MAX )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
		return eERROR_INVALID_ARGUMENT;
	}

	_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBit(gpstCANNYEDGE_REG->vu32Ctrl, 0, (PP_U8)enType));
	
	return eSUCCESS;
}

PP_U32* PPDRV_CANNYEDGE_GetBufferAddress(PP_U16* OUT pu16OutWidth, PP_U16* OUT pu16OutHeight)
{
	PP_U32 *pu32Ret;

	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return NULL;
	}
	
	pu32Ret = (PP_U32*)gpstCANNYEDGE_REG->vu32Paddr;
	PPAPI_SYS_CACHE_Invalidate(pu32Ret, gu16Edge_outWidth * gu16Edge_outHeight / 8);

	*pu16OutWidth = gu16Edge_outWidth;
	*pu16OutHeight = gu16Edge_outHeight;
	
	return pu32Ret;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetDoneHandler(PPDRV_CANNYEDGE_CALLBACK_DONE IN callback)
{
	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}
	
	gpEdge_cbDone = callback;
	
	if ( callback )
	{
	    INTC_irq_clean(IRQ_EDGE_VECTOR);
		INTC_irq_enable(IRQ_EDGE_VECTOR);
	}
	else
	{
		INTC_irq_disable(IRQ_EDGE_VECTOR);
	}

	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_SetEnable(PP_BOOL IN bEnable)
{
	if ( !gbEdge_isInit )
	{
		LOG_WARNING("[EDGE(%s_%d)]: eERROR_EDGE_NOT_INITIALIZE\n", __FUNCTION__, __LINE__);
		return eERROR_EDGE_NOT_INITIALIZE;
	}
	
	if ( bEnable )
	{
		_REG(gpstCANNYEDGE_REG->vu32Enable, utilPutBits(gpstCANNYEDGE_REG->vu32Enable, 0, 2, 0x3));
	}
	else
	{
		_REG(gpstCANNYEDGE_REG->vu32Enable, utilPutBits(gpstCANNYEDGE_REG->vu32Enable, 0, 2, 0x0));
		_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBit(gpstCANNYEDGE_REG->vu32Ctrl, 9, 0x1));
	}
	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_Initialize(PP_U16 IN u16VinWidth, PP_U16 IN u16VinHeight, PP_U16 IN u16VoutWidth, PP_U16 IN u16VoutHeight, PP_U8 IN u8FrameBufferCount)
{
	sys_os_isr_t *pIsrOld = NULL;
	PP_U16 u16Width = u16VinWidth;
	PP_U16 u16Height = u16VinHeight;
	
	if ( gbEdge_isInit ) return eSUCCESS;
	
	if ( gu32Edge_outBuf == NULL )
	{
		if ( u16VinWidth < u16VoutWidth )
			u16Width = u16VoutWidth;
		if ( u16VinHeight < u16VoutHeight )
			u16Height = u16VoutHeight;
		
		gu32Edge_outBuf = (PP_U32*)OSAL_malloc(u16Width * u16Height * u8FrameBufferCount / 8);
		if ( gu32Edge_outBuf == NULL )
		{
			LOG_CRITICAL("[EDGE(%s_%d)]: eERROR_NO_MEM\n", __FUNCTION__, __LINE__);
			return eERROR_NO_MEM;
		}
	}

	gbEdge_isInit = TRUE;

	gu16Edge_vinWidth = u16VinWidth;
	gu16Edge_vinHeight = u16VinHeight;
	gu16Edge_voutWidth = u16VoutWidth;
	gu16Edge_voutHeight = u16VoutHeight;
	gu8Edge_frameCnt = u8FrameBufferCount;

	PPDRV_CANNYEDGE_SetEnable(FALSE);

#if (BD_VIN_INTERLACE == 1)
	_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBits(gpstCANNYEDGE_REG->vu32Ctrl, 10, 2, 0x1));
#else
	_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBits(gpstCANNYEDGE_REG->vu32Ctrl, 10, 2, 0x0));
#endif
	
	PPDRV_CANNYEDGE_SetFixedThreshold(0x14, 0x8);
	PPDRV_CANNYEDGE_SetThresholdRatio(0.7f, 0.4f);
	_REG(gpstCANNYEDGE_REG->vu32Thval, utilPutBits(gpstCANNYEDGE_REG->vu32Thval, 0, 16, 0x814));
	PPDRV_CANNYEDGE_SetThresholdType(eCANNYEDGE_TH_TYPE_RATIO);
	
	PPDRV_CANNYEDGE_SetInput(eCANNYEDGE_INPUT_CH0);

	_REG(gpstCANNYEDGE_REG->vu32Wbase, (PP_U32)gu32Edge_outBuf);
#if 1
	_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBits(gpstCANNYEDGE_REG->vu32Ctrl, 8, 2, 0x3));
#else
	_REG(gpstCANNYEDGE_REG->vu32Ctrl, utilPutBits(gpstCANNYEDGE_REG->vu32Ctrl, 8, 2, 0x2));
#endif

	PPDRV_CANNYEDGE_SetDownScale(FALSE, FALSE);
	PPDRV_CANNYEDGE_SetROI(0, gu16Edge_inHeight-1);

	OSAL_register_isr(IRQ_EDGE_VECTOR, CannyEdge_isr, pIsrOld);
	INTC_irq_disable(IRQ_EDGE_VECTOR);
	INTC_irq_config(IRQ_EDGE_VECTOR, IRQ_LEVEL_TRIGGER);
    INTC_irq_clean(IRQ_EDGE_VECTOR);

	SYS_PROC_addIrq(stEdge_proc_irqs);
	SYS_PROC_addDevice(stEdge_proc_devices);

	return eSUCCESS;
}

PP_RESULT_E PPDRV_CANNYEDGE_InitializeByConfig(PP_U8 IN u8FrameBufferCount)
{
	PP_S32 s32VinWidth, s32VinHeight, s32VoutWidth, s32VoutHeight;
 	_VID_RESOL eVinResol, eVoutResol;

    if ( PPAPI_VIN_GetResol(BD_VIN_FMT, &s32VinWidth, &s32VinHeight, &eVinResol) != eSUCCESS )
    {
        LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }
    if ( PPAPI_VIN_GetResol(BD_DU_IN_FMT, &s32VoutWidth, &s32VoutHeight, &eVoutResol) != eSUCCESS )
    {
        LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

	return PPDRV_CANNYEDGE_Initialize(s32VinWidth, s32VinHeight, s32VoutWidth, s32VoutHeight, u8FrameBufferCount);
}

PP_VOID PPDRV_CANNYEDGE_Deinitialize(PP_VOID)
{
	if ( gu32Edge_outBuf )
	{
		OSAL_free(gu32Edge_outBuf);
		gu32Edge_outBuf = NULL;
	}

	gbEdge_isInit = FALSE;
}
