
#include <stdio.h>
#include <nds32_intrinsic.h>
#include "pi5008.h"
#include "dma.h"
#include "debug.h"
#include "utils.h"
#include "nds32.h"
#include "interrupt.h"
#include "osal.h"


typedef struct ppDMA_STATUS_S
{
	bool bDone;
	bool bAbort;
	bool bError;
}PP_DMA_STATUS_S;

static DMA_REG_T *gpDMA = DMA_CTRL_REG;
static PP_DMA_STATUS_S gstStatus[DMA_MAX_CHANNEL];
static SemaphoreHandle_t ghLock[DMA_MAX_CHANNEL];

ISR(dma_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	int i;
	BaseType_t xHigherPriorityTaskWoken;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpDMA->irq_status;
	//LOG_DEBUG("irq status: 0x%x\n", reg);

	if(reg&0xff){
		LOG_DEBUG("dma abort: 0x%x\n", reg);
	}
	for(i=0;i<DMA_MAX_CHANNEL;i++){
		if(reg & (1<<i)){				// ERROR
			gstStatus[i].bError = TRUE;
		}else if(reg & (1<<(i+8))){		// ABORT
			gstStatus[i].bAbort = TRUE;
		}else if(reg & (1<<(i+16))){	// DONE
			gstStatus[i].bDone = TRUE;
		}

		if(gstStatus[i].bError || gstStatus[i].bAbort || gstStatus[i].bDone){
			//release semaphore.
			xHigherPriorityTaskWoken = pdFALSE;
			xSemaphoreGiveFromISR(ghLock[i], &xHigherPriorityTaskWoken);
			if(xHigherPriorityTaskWoken == pdTRUE)
			{
				portYIELD_FROM_ISR();
			}
		}
	}

	gpDMA->irq_status = reg;
	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);

}

static void dma_sw_reset(void)
{
	gpDMA->ctrl = 1;
}



PP_VOID PPDRV_DMA_Initialize(PP_VOID)
{
	uint32 i;

	OSAL_register_isr(IRQ_DMA_VECTOR, dma_isr, NULL);
    INTC_irq_config(IRQ_DMA_VECTOR, IRQ_LEVEL_TRIGGER); //Must set Level
    INTC_irq_enable(IRQ_DMA_VECTOR);

	dma_sw_reset();
	memset(&gstStatus, 0, sizeof(gstStatus));

	for(i=0;i<DMA_MAX_CHANNEL;i++){
		if( (ghLock[i] = xSemaphoreCreateBinary()) == NULL)
		{
			LOG_DEBUG("ERROR! can't create lock\n");
		}
	}
}



PP_VOID PPDRV_DMA_SetConfig(PP_S32 IN s32Ch, PP_DMA_CONFIG_S IN *pstCfg)
{

	gpDMA->ch_reg[s32Ch].ctrl = pstCfg->ctrl.value;					// set chn control register
	gpDMA->ch_reg[s32Ch].src = pstCfg->u32SrcAddr;			// set source address
	gpDMA->ch_reg[s32Ch].dst = pstCfg->u32DstAddr;			// set destination address
	gpDMA->ch_reg[s32Ch].size = pstCfg->u32TrasnferSize;	// set transfer size
	gpDMA->ch_reg[s32Ch].llp = pstCfg->u32LLPPoint;		// set linked list pointer

}


PP_RESULT_E PPDRV_DMA_Start(PP_S32 IN s32Ch, PP_U32 IN u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;

	memset(&gstStatus, 0, sizeof(gstStatus));
	gpDMA->ch_reg[s32Ch].ctrl = utilSetBit( gpDMA->ch_reg[s32Ch].ctrl, 0 );

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[s32Ch], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			ret = eERROR_TIMEOUT;
			PPDRV_DMA_Abort(s32Ch);
		}else{
			if(gstStatus[s32Ch].bAbort || gstStatus[s32Ch].bError){
				ret = eERROR_FAILURE;
			}else{
				ret = eSUCCESS;
			}
		}
    }

    return ret;

}

PP_RESULT_E PPDRV_DMA_Wait(PP_S32 IN s32Ch, PP_U32 IN u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[s32Ch], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			ret = eERROR_TIMEOUT;
			PPDRV_DMA_Abort(s32Ch);
		}else{
			if(gstStatus[s32Ch].bAbort || gstStatus[s32Ch].bError){
				ret = eERROR_FAILURE;
			}else{
				ret = eSUCCESS;
			}
		}
    }

    return ret;

}


//uint32 dma_abort(uint32 ch)
PP_VOID PPDRV_DMA_Abort(PP_S32 IN s32Ch)
{
	gpDMA->ch_abort = utilSetBit(gpDMA->ch_abort, s32Ch);
}


PP_U32 PPDRV_DMA_GetDone(PP_S32 IN s32Ch, PP_RESULT_E OUT *penResult)
{
	uint32 ret = 0;

	if(penResult)*penResult = eERROR_FAILURE;
	if(utilGetBit(gpDMA->ch_reg[s32Ch].ctrl, 0) == 0){
		ret = 1;
		if(!(gstStatus[s32Ch].bAbort || gstStatus[s32Ch].bError)){
			if(penResult)*penResult = eSUCCESS;
		}
	}
	return ret;

}

PP_RESULT_E PPDRV_DMA_M2M_Word(PP_S32 IN s32Ch, PP_U32 IN *SrcAddr, PP_U32 IN *DstAddr, PP_U32 IN u32Size, PP_U32 IN u32Timeout)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_DMA_CONFIG_S cfg;

	if (SrcAddr == NULL || DstAddr == NULL)
	{
	    return eERROR_FAILURE;
	}

	if(s32Ch >= SYS_DMA_MAX_CHANNEL)
	{
	    return eERROR_FAILURE;
	}

	if(!PPDRV_DMA_GetDone(s32Ch, NULL))
	{
	    return eERROR_FAILURE;
	}

	memset(&cfg, 0, sizeof(cfg));

	cfg.ctrl.ctl.priority = 0;
	cfg.ctrl.ctl.src_burst_size = DMA_BSIZE_32;
	cfg.ctrl.ctl.src_width = DMA_WIDTH_WORD;
	cfg.ctrl.ctl.dst_width = DMA_WIDTH_WORD;
	cfg.ctrl.ctl.src_mode = 0;
	cfg.ctrl.ctl.dst_mode = 0;
	cfg.ctrl.ctl.src_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.dst_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.src_req_sel = 0;
	cfg.ctrl.ctl.dst_req_sel = 0;
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;

	cfg.u32SrcAddr = (unsigned int)SrcAddr;
	cfg.u32DstAddr = (unsigned int)DstAddr;
	cfg.u32TrasnferSize = (u32Size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;


	PPDRV_DMA_SetConfig(s32Ch, &cfg);
	ret = PPDRV_DMA_Start(s32Ch, u32Timeout);

	return ret;
}

