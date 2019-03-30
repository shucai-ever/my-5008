
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <nds32_intrinsic.h>
#include "pi5008.h"
#include "osal.h"
#include "error.h"
#include "vpu_register.h"
#include "interrupt.h"
#include "utils.h"
#include "proc.h"
#include "dram_config.h"
#include "dram.h"

#include "vin.h"
#include "api_vin.h"

#include "vpu.h"
#include "vpu_user_config.h"
#include "debug.h"

/***************************************************************************************************************************************************************
 * Declaration
 ***************************************************************************************************************************************************************/
#define VPU_GetRegValue(reg)          (*((volatile unsigned int *)(VPU_CONFIG_BASE_ADDR+reg)))
#define VPU_SetRegValue(reg, data)    ((*((volatile unsigned int *)(VPU_CONFIG_BASE_ADDR+reg))) = (unsigned int)(data))

/***************************************************************************************************************************************************************
 * Structure
 ***************************************************************************************************************************************************************/


/***************************************************************************************************************************************************************
 * Global Variable
 ***************************************************************************************************************************************************************/
_VPUConfig gVPUConfig;
_VPUConfig *gpVPUConfig = (_VPUConfig *)&gVPUConfig;

static PP_U32 gu32VPUIntcCnt = 0;
static PP_U32 gu32VPUIntcSubCnt[eIRQ_MAX] = {0, };
/*======================================================================================================================
  = Local function
  ======================================================================================================================*/

//PROC irq
PP_RESULT_E VPU_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stVPU_PROC_irqs[] = {
    { .fn = VPU_PROC_IRQ,    .irqNum = IRQ_VPU0_VECTOR, .next = (void*)0, },
};
//PROC device
PP_RESULT_E VPU_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stVPU_PROC_devices[] = {
    { .pName = "VPU",  .fn = VPU_PROC_DEVICE,    .next = (void*)0, },
};


static PP_U32 gu32OldDMATick, gu32OldHDTick, gu32OldFBTick;

PP_U32 PPDRV_VPU_GetVer(void)
{
    PP_U32 ver = 0;

    ver = VPU_GetRegValue(0x9C);
    //LOG_DEBUG("[VPU] Ver:0x%08X\n", ver);

    return(ver);
}

void PPDRV_VPU_DumpReg(void)
{
    PP_U32 i;
    PP_U32 *regs;

    LOG_DEBUG ("########### VPU Control Registers (0xF1420000) ###########\n");
    regs = (PP_U32 *)VPU_CONFIG_BASE_ADDR;
    for (i = 0; i < 64; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
    LOG_DEBUG ("########### VPU Filter Registers (0xF1422000) ###########\n");
    regs = (PP_U32 *)VPU_FILTER_BASE_ADDR;
    for (i = 0; i < 12; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
    LOG_DEBUG ("########### VPU INTC Registers (0xF1421000) ###########\n");
    regs = (PP_U32 *)VPU_INTC_BASE_ADDR;
    for (i = 0; i < 5; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
    LOG_DEBUG ("########### VPU HD2 Registers (0xF1410000) ###########\n");
    regs = (PP_U32 *)VPU_HAMMING_BASE_ADDR;
    for (i = 0; i < 18; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
    LOG_DEBUG ("########### VPU DEBUG Registers (0xF1270020) ###########\n");
    regs = (PP_U32 *)VPU_DEBUG_BASE_ADDR;
    for (i = 0; i < 8; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
    LOG_DEBUG ("########### VPU DMA Registers (0xF1400000) ###########\n");
    regs = (PP_U32 *)VPU_BASE_ADDR;
    for (i = 0; i < 16; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
    LOG_DEBUG ("########### VPU DMA MON Registers (0xF1400600) ###########\n");
    regs = (PP_U32 *)VPU_DMA_MON_BASE_ADDR;
    for (i = 0; i < 16; i++)
    {
        LOG_DEBUG ("\t\t0x%08X : 0x%08X\n", (PP_U32)regs, *regs);
        regs++;
    }
}
ISR(ISR_VPU, num)
{
    _VPUConfig *pVPUConfig = (_VPUConfig *)gpVPUConfig;
    PP_U32 msk = (1 << num);
    PP_S32 inxFrm = 0;
    PP_S32 i;

    PP_U32 u32CurTick;
    PP_U32 u32FCEndPos = 0;

    PP_U32 *pu32BufFast = NULL;
    PP_U32 *pu32BufBrief = NULL;
    PP_U16 *pu16BufFC = NULL;

    BaseType_t xHigherPriorityTaskWoken;

    INTC_irq_clean(num);

    /* Mask and clear HW interrupt vector */
    __nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
    __nds32__mtsr(msk, NDS32_SR_INT_PEND2);

    gu32VPUIntcCnt++;

    u32CurTick = xTaskGetTickCountFromISR();
    //LOG_DEBUG("vpu intr state:%02x, mask:%02x\n", VPU_INTC_REG->state.var, VPU_INTC_REG->intc_mask);

    // ISR sub process
    if (VPU_INTC_REG->state.var & 0x3E) // error state.
    {
        LOG_DEBUG("ERROR! E%02X\n", VPU_INTC_REG->state.var);
    }

    if (VPU_INTC_REG->state.bits.irq_dma_done)
    {
        gu32VPUIntcSubCnt[eIRQ_DMA]++;
        pVPUConfig->u8RunIntcIntervalDMATime = OSAL_tick2ms(u32CurTick - gu32OldDMATick);
        //LOG_DEBUG("%d\n", u32CurTick - gu32OldDMATick);
        gu32OldDMATick = u32CurTick;

        //release semaphore.
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(pVPUConfig->lockDMA, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }

    if (VPU_INTC_REG->state.bits.irq_hd_done)
    {
        gu32VPUIntcSubCnt[eIRQ_HAMMINGD]++;
        pVPUConfig->u8RunIntcIntervalHDTime = OSAL_tick2ms(u32CurTick - gu32OldHDTick);
        //LOG_DEBUG("%d\n", u32CurTick - gu32OldHDTick);
        gu32OldHDTick = u32CurTick;

        VPU_HAMMINGD_CTRL_REG->ctrl.var = 0x02;

        //release semaphore.
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(pVPUConfig->lockHammingD, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }

    if (VPU_INTC_REG->state.bits.irq_done)
    {
        gu32VPUIntcSubCnt[eIRQ_FAST]++;
        pVPUConfig->u8RunIntcIntervalFBTime = OSAL_tick2ms(u32CurTick - gu32OldFBTick);
        //LOG_DEBUG("%d\n", u32CurTick - gu32OldFBTick);
        gu32OldFBTick = u32CurTick;

        VPU_FB_CONFIG_REG->fld.enable = 0;

        inxFrm = pVPUConfig->stFrmInx.inxFrmCur;

        /* get update data from memory to cache. */
        for (i = 0; i < eVPU_ZONE_MAX; i++)
        {
            if(pVPUConfig->pVpuZoneParam[i].isEnable == TRUE)
            {
                pu32BufFast = (PP_U32 *)pVPUConfig->ppVBufFast[inxFrm][i];
                pu32BufBrief = (PP_U32 *)pVPUConfig->ppVBufBrief[inxFrm][i];
                pu16BufFC = (PP_U16 *)pVPUConfig->ppVBufFC[inxFrm][i];

                if( (pu32BufFast == NULL) || (pu32BufBrief == NULL) || (pu16BufFC == NULL) ) continue;

                u32FCEndPos = pVPUConfig->pVpuZoneParam[i].fc_height;
                //register result: (VPU_CTRL_REG->ro_num_feature[i/2]>>(16*(i%2))) & 0xFFFF;
                if( ((VPU_CTRL_REG->ro_num_feature[i/2]>>(16*(i%2))) & 0xFFFF) >= VPU_MAX_FAST_LIMIT_COUNT )
                {
                    //LOG_DEBUG("Over zone:%d, endpos:%d, endFcValue:%d(reg:%d)\n", i, u32FCEndPos, pVPUConfig->u32FCEndValue[inxFrm][i], (VPU_CTRL_REG->ro_num_feature[i/2]>>(16*(i%2))) & 0xFFFF);
                    pVPUConfig->u32FCEndValue[inxFrm][i] = VPU_MAX_FAST_LIMIT_COUNT;
                }
                else
                {
                    //LOG_DEBUG("zone:%d, endpos:%d, endFcValue:%d(reg:%d)\n", i, u32FCEndPos, pVPUConfig->u32FCEndValue[inxFrm][i], (VPU_CTRL_REG->ro_num_feature[i/2]>>(16*(i%2))) & 0xFFFF);
                    if( (pu16BufFC[u32FCEndPos-1]) == ((VPU_CTRL_REG->ro_num_feature[i/2]>>(16*(i%2))) & 0xFFFF) )
                    {
                        pVPUConfig->u32FCEndValue[inxFrm][i] = pu16BufFC[u32FCEndPos-1]; //Need from HammingDistance calculate.
                    }
                    else
                    {
                        //LOG_CRITICAL("Error! zone:%d, endpos:%d, endFcValue:%d( != reg:%d)\n", i, u32FCEndPos, pu16BufFC[u32FCEndPos-1], (VPU_CTRL_REG->ro_num_feature[i/2]>>(16*(i%2))) & 0xFFFF);
                        pVPUConfig->u32FCEndValue[inxFrm][i] = 0; //Need from HammingDistance calculate.
                    }
                }
            }
        }

        //release semaphore.
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(pVPUConfig->lockFast, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }

        /* Set Next Frame param. */
        if(pVPUConfig->runCountFAST > 0)
        {
            pVPUConfig->runCountFAST--;
        }
        if( pVPUConfig->runCountFAST != 0)
        {
            PPDRV_VPU_FAST_UpdateFrmInx(eVPU_FRAME_MAX, &pVPUConfig->stFrmInx);

            PPDRV_VPU_FAST_RunNext(0);
        }

        //release semaphore.
        xHigherPriorityTaskWoken = pdFALSE;
        xSemaphoreGiveFromISR(pVPUConfig->waitFrmUpdate, &xHigherPriorityTaskWoken);
        if(xHigherPriorityTaskWoken == pdTRUE)
        {
            portYIELD_FROM_ISR();
        }
    }


    /* all clear interrupt */
    VPU_INTC_REG->intc_clear = (VPU_INTC_REG->state.var & 0xFF);
    VPU_INTC_REG->intc_clear = 0;
    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

/*======================================================================================================================
  = Export function
  ======================================================================================================================*/

/***************************************************************************************************************************************************************
 * function					:	VPUAPI_FB_initialize

 * param inputSize			:	Input video size (width, height)
 * param limitFeatureCount	:	Limit count of feature point

 * return					:	error code

 * brief
 Initializes the FAST & BRIEF.
 ***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_VPU_FAST_Initialize(void)
{
    PP_U8 bwFAST, bwBREIF;
    //PP_U32 size;
    sys_os_isr_t *old = NULL;

    VPU_DMAMONITOR_REG->ctrl = 0x0000000F;	// enable first
    VPU_DMAMONITOR_REG->ctrl = 0x000000FF;	// clear any time

    VPU_OTFMONITOR_REG->ctrl = 0x0000000F;	// enable first
    VPU_OTFMONITOR_REG->ctrl = 0x000000FF;	// clear any time

    // bandwidth + limit count
    bwFAST = 0x0F;
    bwBREIF = 0x3F;
    VPU_CTRL_REG->bandwidth = (bwFAST << 0) | (bwBREIF << 8) | (VPU_MAX_FAST_LIMIT_COUNT << 16);
    VPU_DMACTRL_REG->ctrl = 0x0F0F0F00;

    /* register isr */
    OSAL_register_isr(IRQ_VPU0_VECTOR, ISR_VPU, old);
    INTC_irq_disable(IRQ_VPU0_VECTOR);
    INTC_irq_config(IRQ_VPU0_VECTOR, IRQ_LEVEL_TRIGGER);
    INTC_irq_clean(IRQ_VPU0_VECTOR);
    INTC_irq_enable(IRQ_VPU0_VECTOR);

    return(eSUCCESS);
}


/***************************************************************************************************************************************************************
 * function					:	VPUAPI_FB_deInitialize

 * param 					:

 * return					:

 * brief
 Deinitializes the FAST & BRIEF.
 ***************************************************************************************************************************************************************/
PP_RESULT_E PPDRV_VPU_FAST_SetScale(VPU_SIZE_T inputSize, VPU_SIZE_T outputSize)
{
    PP_U32 vRatio;
    PP_U32 hRatio;

    vRatio = (256 * inputSize.height) / outputSize.height;
    hRatio = (256 * inputSize.width) / outputSize.width;

    VPU_CTRL_REG->scl_size = (outputSize.height) | (outputSize.width << 16);
    VPU_CTRL_REG->scl_ratio = (vRatio) | (hRatio << 16);

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_FAST_SetInputSize(VPU_SIZE_T inputSize)
{
    // input size
    VPU_CTRL_REG->size = (inputSize.height) | (inputSize.width << 16);

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_FAST_SetROI(VPU_RECT_T roi)
{
    PP_U16 startX, startY, endX, endY;

    startX = roi.x;
    startY = roi.y;
    endX = (roi.x + roi.width) - 1;
    endY = (roi.y + roi.height) - 1;

    VPU_CTRL_REG->roi_start = (startY) | (startX << 16);
    VPU_CTRL_REG->roi_end = (endY) | (endX << 16);

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_FAST_SetConfig(const USER_VPU_FB_CONFIG_U *pVPUFBConfig)
{
    PP_RESULT_E ret = eSUCCESS;
    VPU_FB_CONFIG_U stVPUFBConfig;

    if ( pVPUFBConfig == NULL )
    {
        return(eERROR_VPU_NOT_INITIALIZE);
    }

    /* fix value */
    stVPUFBConfig.var = 0;
    stVPUFBConfig.fld.use_posedge_vsync = 1;
    stVPUFBConfig.fld.enable = 0;

    stVPUFBConfig.fld.fast_n            = pVPUFBConfig->fld.fast_n;
    stVPUFBConfig.fld.ch_sel            = pVPUFBConfig->fld.ch_sel;
    stVPUFBConfig.fld.use_5x3_nms       = pVPUFBConfig->fld.use_5x3_nms;
    stVPUFBConfig.fld.brief_enable      = pVPUFBConfig->fld.brief_enable;
    stVPUFBConfig.fld.roi_enable        = pVPUFBConfig->fld.roi_enable;
    stVPUFBConfig.fld.scl_enable        = pVPUFBConfig->fld.scl_enable;
    stVPUFBConfig.fld.dma2otf_enable    = pVPUFBConfig->fld.dma2otf_enable;
    stVPUFBConfig.fld.otf2dma_enable    = pVPUFBConfig->fld.otf2dma_enable;
    stVPUFBConfig.fld.field_sel    	    = pVPUFBConfig->fld.field_sel;

    VPU_FB_CONFIG_REG->var = stVPUFBConfig.var;
    return(ret);
}

PP_RESULT_E PPDRV_VPU_FAST_GetConfig(USER_VPU_FB_CONFIG_U *pVPUFBConfig)
{
    PP_RESULT_E ret = eSUCCESS;

    if ( pVPUFBConfig == NULL )
    {
        return(eERROR_VPU_NOT_INITIALIZE);
    }

    pVPUFBConfig->var		= VPU_FB_CONFIG_REG->var;

    return(ret);
}

PP_RESULT_E PPDRV_VPU_FAST_SetZone(PP_U8 zoneNumber, VPU_ZONINFO_T *pZoneInfo)
{
    PP_U32 zoneStartX, zoneStartY, zoneEndX, zoneEndY;			// multiple of 8
    PP_U32 reg;
    PP_U32 ch;

    if ( zoneNumber >= eVPU_ZONE_MAX )
    {
        return(eERROR_INVALID_ARGUMENT);
    }

    if ( pZoneInfo->isEnable )
    {
        reg = VPU_CTRL_REG->zone2th[zoneNumber/4];
        ch = zoneNumber % 4;
        reg &= ~(0xFF<<(ch*8));
        reg |= (pZoneInfo->fastThreshold & 0xFF)<<(ch*8);
        VPU_CTRL_REG->zone2th[zoneNumber/4] = reg;

        reg = VPU_CTRL_REG->zone2lut[zoneNumber/8];
        ch = zoneNumber % 8;
        reg &= ~(0xF<<(ch*4));
        reg |= (pZoneInfo->briefLUTIndex & 0xF)<<(ch*4); //briefLUTIndex [0:program1, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4]
        VPU_CTRL_REG->zone2lut[zoneNumber/8] = reg;

        reg = VPU_OTF_FILTER_REG->zone2coef_fast[zoneNumber/8];
        ch = zoneNumber % 8;
        reg &= ~(0xF<<(ch*4));
        reg |= (pZoneInfo->fastFilterIndex & 0xF)<<(ch*4); // fastFilterIndex [ 0:program0, 1:program1, 2:bypass, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4]
        VPU_OTF_FILTER_REG->zone2coef_fast[zoneNumber/8] = reg;

        reg = VPU_OTF_FILTER_REG->zone2coef_brief[zoneNumber/8];
        ch = zoneNumber % 8;
        reg &= ~(0xF<<(ch*4));
        reg |= (pZoneInfo->briefFilterIndex & 0xF)<<(ch*4); //briefFilterIndex [0:program0, 1:program1, 2:bypass, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4]
        VPU_OTF_FILTER_REG->zone2coef_brief[zoneNumber/8] = reg;

        zoneStartX = pZoneInfo->sx/8;
        zoneStartY = pZoneInfo->sy/8;
        zoneEndX = (pZoneInfo->ex-1)/8;
        zoneEndY = (pZoneInfo->ey-1)/8;
        pZoneInfo->fc_height = (((zoneEndY+1)*8) - (zoneStartY*8));
        VPU_CTRL_REG->zone[zoneNumber] = ((zoneStartX << 0) & 0x000000FF) | ((zoneEndX << 8) & 0x0000FF00) | ((zoneStartY << 16) & 0x00FF0000) | ((zoneEndY << 24) & 0xFF000000);

        LOG_DEBUG("__ZONE %d SET__ (%d/%d)(%d/%d)\n", zoneNumber, zoneStartX*8, zoneStartY*8, zoneEndX*8, zoneEndY*8);
        /*
           LOG_DEBUG("__ZONE %d SET__ (%d/%d)(%d/%d) => (%d/%d)(%d/%d) %08x,%08x,%08x,%08x\n", zoneNumber, 
           pZoneInfo->sx, pZoneInfo->sy, pZoneInfo->ex, pZoneInfo->ey, zoneStartX*8, zoneStartY*8, zoneEndX*8, zoneEndY*8,
           VPU_CTRL_REG->zone2th[zoneNumber/4],
           VPU_CTRL_REG->zone2lut[zoneNumber/8],
           VPU_OTF_FILTER_REG->zone2coef_fast[zoneNumber/8],
           VPU_OTF_FILTER_REG->zone2coef_brief[zoneNumber/8]
           );
           */
    }
    else
    {
        VPU_CTRL_REG->zone[zoneNumber] = 0xFFFFFFFF; //0xFFFFFFFF: disable value

        reg = VPU_OTF_FILTER_REG->zone2coef_fast[zoneNumber/8];
        ch = zoneNumber % 8;
        reg &= ~(0xF<<(ch*4));
        reg |= (2 & 0xF)<<(ch*4); // fastFilterIndex [ 0:program0, 1:program1, 2:bypass, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4]
        VPU_OTF_FILTER_REG->zone2coef_fast[zoneNumber/8] = reg;

        reg = VPU_OTF_FILTER_REG->zone2coef_brief[zoneNumber/8];
        ch = zoneNumber % 8;
        reg &= ~(0xF<<(ch*4));
        reg |= (2 & 0xF)<<(ch*4); //briefFilterIndex [0:program0, 1:program1, 2:bypass, 12:prefix1, 13:prefix2, 14:prefix3, 15:prefix4]
        VPU_OTF_FILTER_REG->zone2coef_brief[zoneNumber/8] = reg;
    }

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_BRIEF_SetLut(VPU_BRIEF_LUT_VALUE_T *pBriefLut)
{
    PP_S32 i = 0;
    PP_U32 value = 0x0;

    if ( pBriefLut == NULL )
    {
        return(eERROR_INVALID_ARGUMENT);
    }

    for ( i = 0; i < 128; i+=4 )
    {
        value = ( ((pBriefLut[i+0].y1 & 0x3) << (0+ 0)) | ((pBriefLut[i+0].x1 & 0x3) << (2+ 0)) |
                ((pBriefLut[i+0].y2 & 0x3) << (4+ 0)) | ((pBriefLut[i+0].x2 & 0x3) << (6+ 0)) |
                ((pBriefLut[i+1].y1 & 0x3) << (0+ 8)) | ((pBriefLut[i+1].x1 & 0x3) << (2+ 8)) |
                ((pBriefLut[i+1].y2 & 0x3) << (4+ 8)) | ((pBriefLut[i+1].x2 & 0x3) << (6+ 8)) |
                ((pBriefLut[i+2].y1 & 0x3) << (0+16)) | ((pBriefLut[i+2].x1 & 0x3) << (2+16)) |
                ((pBriefLut[i+2].y2 & 0x3) << (4+16)) | ((pBriefLut[i+2].x2 & 0x3) << (6+16)) |
                ((pBriefLut[i+3].y1 & 0x3) << (0+24)) | ((pBriefLut[i+3].x1 & 0x3) << (2+24)) |
                ((pBriefLut[i+3].y2 & 0x3) << (4+24)) | ((pBriefLut[i+3].x2 & 0x3) << (6+24)) );

        VPU_OTF_BRIEF_LUT_REG->lut[i/4] = value;
    }
    return(eSUCCESS);
}

void PPDRV_VPU_BRIEF_SetProgramFilter (const PP_U8 *pCoef0Array, const PP_U8 *pCoef1Array)
{
    if(pCoef0Array)
    {
        VPU_OTF_FILTER_REG->coef0y0 = (
                ((pCoef0Array[0]&0xF)<<0) |
                ((pCoef0Array[1]&0xF)<<4) |
                ((pCoef0Array[2]&0xF)<<8) |
                ((pCoef0Array[3]&0xF)<<12) |
                ((pCoef0Array[4]&0xF)<<16) );
        VPU_OTF_FILTER_REG->coef0y1 = (
                ((pCoef0Array[5]&0xF)<<0) |
                ((pCoef0Array[6]&0xF)<<4) |
                ((pCoef0Array[7]&0xF)<<8) |
                ((pCoef0Array[8]&0xF)<<12) |
                ((pCoef0Array[9]&0xF)<<16) );

        VPU_OTF_FILTER_REG->coef0y2 = (
                ((pCoef0Array[10]&0xF)<<0) |
                ((pCoef0Array[11]&0xF)<<4) |
                ((pCoef0Array[12]&0xF)<<8) |
                ((pCoef0Array[13]&0xF)<<12) |
                ((pCoef0Array[14]&0xF)<<16) );

        VPU_OTF_FILTER_REG->shift10 = (VPU_OTF_FILTER_REG->shift10 & 0xF) | ((pCoef0Array[15]&0xF)<<4);
    }

    if(pCoef1Array)
    {
        VPU_OTF_FILTER_REG->coef1y0 = (
                ((pCoef1Array[0]&0xF)<<0) |
                ((pCoef1Array[1]&0xF)<<4) |
                ((pCoef1Array[2]&0xF)<<8) |
                ((pCoef1Array[3]&0xF)<<12) |
                ((pCoef1Array[4]&0xF)<<16) );
        VPU_OTF_FILTER_REG->coef1y1 = (
                ((pCoef1Array[5]&0xF)<<0) |
                ((pCoef1Array[6]&0xF)<<4) |
                ((pCoef1Array[7]&0xF)<<8) |
                ((pCoef1Array[8]&0xF)<<12) |
                ((pCoef1Array[9]&0xF)<<16) );

        VPU_OTF_FILTER_REG->coef1y2 = (
                ((pCoef1Array[10]&0xF)<<0) |
                ((pCoef1Array[11]&0xF)<<4) |
                ((pCoef1Array[12]&0xF)<<8) |
                ((pCoef1Array[13]&0xF)<<12) |
                ((pCoef1Array[14]&0xF)<<16) );

        VPU_OTF_FILTER_REG->shift10 = (VPU_OTF_FILTER_REG->shift10 & 0xF0) | ((pCoef0Array[15]&0xF)<<0);
    }

    return;
}

void PPDRV_VPU_FAST_InitFrmInx(void)
{
    _VPUConfig *pVPUConfig = (_VPUConfig *)gpVPUConfig;
    _FrmInx *pFrmInx = (_FrmInx *)&pVPUConfig->stFrmInx;

    pFrmInx->inxFrmPPrev = -1;
    pFrmInx->inxFrmPrev = -1;
    pFrmInx->inxFrmCur = -1;
    pFrmInx->inxFrmLast = -1;
}

void PPDRV_VPU_FAST_UpdateFrmInx(const PP_S32 inxFrmTotalCnt, _FrmInx *pFrmInx)
{
    if(pFrmInx->inxFrmCur != -1)
    {
        pFrmInx->inxFrmPPrev = pFrmInx->inxFrmPrev;
        pFrmInx->inxFrmPrev = pFrmInx->inxFrmLast;
        pFrmInx->inxFrmLast = pFrmInx->inxFrmCur;
    }
    else
    {
        pFrmInx->inxFrmPPrev = pFrmInx->inxFrmPrev;
        pFrmInx->inxFrmPrev = pFrmInx->inxFrmCur;
    }

    pFrmInx->inxFrmCur++;
    pFrmInx->inxFrmCur %= inxFrmTotalCnt;
    //LOG_DEBUG("cur:%d, last:%d, prev:%d, pprev:%d\n", pFrmInx->inxFrmCur, pFrmInx->inxFrmLast, pFrmInx->inxFrmPrev, pFrmInx->inxFrmPPrev);
}

PP_RESULT_E PPDRV_VPU_DMA_DrvWaitDone(PP_U32 u32Timeout)
{
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if( xSemaphoreTake(gpVPUConfig->lockDMA, u32Timeout) == pdFALSE )
    {
        LOG_CRITICAL("Timeout! DMA.\n");
        return(eERROR_FAILURE);
    }

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_FAST_DrvWaitDone(PP_U32 u32Timeout)
{
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if( xSemaphoreTake(gpVPUConfig->lockFast, u32Timeout) == pdFALSE )
    {
        LOG_CRITICAL("Timeout! FAST.\n");
        return(eERROR_FAILURE);
    }

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_HAMMINGD_DrvWaitDone(PP_U32 u32Timeout)
{
    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if( xSemaphoreTake(gpVPUConfig->lockHammingD, u32Timeout) == pdFALSE )
    {
        LOG_CRITICAL("Timeout! HAMMINGD.\n");
        return(eERROR_FAILURE);
    }

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_FAST_Start(const PP_S32 runCount, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    _VPUConfig *pVPUConfig = (_VPUConfig *)gpVPUConfig;
    PP_S32 inxFrm = 0;

    if ( runCount == 0 )
    {
        return(eERROR_VPU_NOT_INITIALIZE);
    }

    PPDRV_VPU_FAST_UpdateFrmInx(eVPU_FRAME_MAX, &pVPUConfig->stFrmInx);
    inxFrm = pVPUConfig->stFrmInx.inxFrmCur;

    VPU_CTRL_REG->xy_base = (PP_U32)pVPUConfig->ppBufFast[inxFrm][0];
    VPU_CTRL_REG->desc_base = (PP_U32)pVPUConfig->ppBufBrief[inxFrm][0];
    VPU_CTRL_REG->fc_base = (PP_U32)pVPUConfig->ppBufFC[inxFrm][0];

    VPU_FB_CONFIG_REG->fld.enable = 1;
    pVPUConfig->runCountFAST = runCount;

    if(u32TimeOut)
    {
        ret = PPDRV_VPU_FAST_DrvWaitDone(u32TimeOut);
    }

    return(ret);
}

PP_RESULT_E PPDRV_VPU_FAST_RunNext(const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    _VPUConfig *pVPUConfig = (_VPUConfig *)gpVPUConfig;
    PP_S32 inxFrm = 0;

    inxFrm = pVPUConfig->stFrmInx.inxFrmCur;

    VPU_CTRL_REG->xy_base = (PP_U32)pVPUConfig->ppBufFast[inxFrm][0];
    VPU_CTRL_REG->desc_base = (PP_U32)pVPUConfig->ppBufBrief[inxFrm][0];
    VPU_CTRL_REG->fc_base = (PP_U32)pVPUConfig->ppBufFC[inxFrm][0];

    VPU_FB_CONFIG_REG->fld.enable = 1;

    if(u32TimeOut)
    {
        ret = PPDRV_VPU_FAST_DrvWaitDone(u32TimeOut);
    }

    return(ret);
}

PP_RESULT_E PPDRV_VPU_FAST_Stop(void)
{
    PP_RESULT_E ret = eSUCCESS;
    _VPUConfig *pVPUConfig = (_VPUConfig *)gpVPUConfig;

    VPU_FB_CONFIG_REG->fld.enable = 0;
    pVPUConfig->runCountFAST = 0;

    PPDRV_VPU_FAST_InitFrmInx();

    return(ret);
}

PP_RESULT_E PPDRV_VPU_DMA_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;

    PP_U32 u32DmaSize = 0;

    //wait previous dma done.

    if( (u32RdPAddr == 0) || (u32RdStride == 0) || (u32WrStride == 0) || (u16RdWidth == 0) || (u16RdHeight == 0) )
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(eERROR_FAILURE);
    }
    if((u16RdWidth & 0xF) != 0)
    {
        LOG_CRITICAL("Invalid dma param. Not align 16byte\n");
        return(eERROR_FAILURE);
    }
    if( (eDMAMinorMode == eDMA_MINOR_EVENBYTE) || (eDMAMinorMode == eDMA_MINOR_ODDBYTE) )
    {
        if(eWDMAType == eWDMA_1D)
        {
            LOG_CRITICAL("Error! Don't support 1D WDMA. Set 2D WDMA\n");
            return(eERROR_FAILURE);
        }
    }

    u32DmaSize = u16RdWidth * u16RdHeight;
    if(u32DmaSize == 0)
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(eERROR_FAILURE);
    }
    if(eRDMAType == eRDMA_2D)
    {
        if(eDMAMinorMode != eDMA_MINOR_COPY)
        {
            if((u16RdWidth & 0x1F) != 0) //32byte align
            {
                LOG_CRITICAL("Invalid dma param. Not align 32byte\n");
                return(eERROR_FAILURE);
            }
        }
    }

    //LOG_DEBUG("Dma mode:%d-%d, typeRW:%d/%d, OTF2DMA type:%d, rd:%08x, strideR%08x, wd:%04x, ht:%04x, wr:%08x, strideW:%08x, time:%d",
    //        eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u32RdStride, u16RdWidth, u16RdHeight, u32WrPAddr, u32WrStride, u32TimeOut);
    //LOG_DEBUG("Dma size:0x%08x\n", u32DmaSize);

    //Clear dma irq_done
    VPU_DMACTRL_REG->ctrl &=  ~0x2; 

    //Set RDMA param.
    VPU_DMACTRL_REG->rdma2d_base = u32RdPAddr;
    VPU_DMACTRL_REG->rdma2d_stride = u32RdStride;
    if(eRDMAType == eRDMA_2D)
    {
        VPU_DMACTRL_REG->rdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
    }
    else //1D RDMA
    {
        VPU_DMACTRL_REG->rdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (1&0xFFFF);
    }
    VPU_DMACTRL_REG->ctrl = 0x0F0F0F00 | ((eDMAMode)&0x7)<<4 | ((eDMAMinorMode)&0x3)<<2; //mode[0:2D dma, 2:RLE Encode, 7:DMA2OTF/OTF2DMA]

    VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;
    VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
    if(eDMAMode == eDMA_OTF)
    {
        VPU_INTC_REG->intc_mask |= (0x80); //mask 1:block, 0:release
        VPU_FB_CONFIG_REG->fld.dma2otf_enable = 1;
    }
    else
    {
        VPU_INTC_REG->intc_mask &= ~(0x80); //mask 1:block, 0:release
        VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;
    }

    //Set WDMA param.
    VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
    if(u32WrPAddr != 0)
    {
        if(eDMAMode == eDMA_OTF)
        {
            VPU_FB_CONFIG_REG->fld.otf2dma_enable = eOTF2DMAType;
        }
        else if(eDMAMode == eDMA_RLE)
        {
            //VPU_DMACTRL_REG->wdma_rlewaitdone_count = 0xFFFFFFFF;
        }

        if(eWDMAType == eWDMA_2D)
        {
            if( (eDMAMinorMode == eDMA_MINOR_EVENBYTE) || (eDMAMinorMode == eDMA_MINOR_ODDBYTE) )
            {
                VPU_DMACTRL_REG->wdma2d_base = u32WrPAddr;
                VPU_DMACTRL_REG->wdma2d_stride = u32WrStride>>1;
                VPU_DMACTRL_REG->wdma2d_size = (((u16RdWidth>>1)&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
                VPU_DMACTRL_REG->ctrl &= ~(1<<7); //1d wdma disable
            }
            else
            {
                VPU_DMACTRL_REG->wdma2d_base = u32WrPAddr;
                VPU_DMACTRL_REG->wdma2d_stride = u32WrStride;
                VPU_DMACTRL_REG->wdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
                VPU_DMACTRL_REG->ctrl &= ~(1<<7); //1d wdma disable

            }
        }
        else //1D WDMA
        {
            VPU_DMACTRL_REG->wdma1d_base = u32WrPAddr;
            VPU_DMACTRL_REG->ctrl |= 1<<7; //1d wdma enable
            VPU_DMACTRL_REG->wdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (0x1&0xFFFF);
        }
    }

    VPU_DMACTRL_REG->ctrl |= 0x1; //b'0 : start

    if( (eDMAMode != eDMA_OTF) )
    {
        if(u32TimeOut)
        {
            ret = PPDRV_VPU_DMA_DrvWaitDone(u32TimeOut);
        }
    }

    return(ret);
}

PP_RESULT_E PPDRV_VPU_DMA_1Dto2D_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U32 u32RdPAddr, const PP_U16 u16RdSize, const PP_U16 u16WrWidth, const PP_U16 u16WrHeight, const PP_U32 u32WrPAddr, const PP_U32 u32WrStride, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;

    //wait previous dma done.

    if( (u32RdPAddr == 0) || (u16RdSize == 0) || (u32WrStride == 0) || (u16WrWidth == 0) || (u16WrHeight == 0) )
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(eERROR_FAILURE);
    }
    if( (eDMAMode != eDMA_2D) || (eRDMAType != eRDMA_1D) || (eWDMAType != eWDMA_2D) || (eDMAMinorMode != eDMA_MINOR_COPY) )
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(eERROR_FAILURE);
    }
    if((u16WrWidth & 0xF) != 0)
    {
        LOG_CRITICAL("Invalid dma param. Not align 16byte\n");
        return(eERROR_FAILURE);
    }

    //LOG_DEBUG("Dma mode:%d-%d, typeRW:%d/%d, OTF2DMA type:%d, rd:%08x, rdSize:%08x, wd:%04x, ht:%04x, wr:%08x, strideW:%08x, time:%d",
    //        eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u16RdSize, u16WrWidth, u16WrHeight, u32WrPAddr, u32WrStride, u32TimeOut);

    //Clear dma irq_done
    VPU_DMACTRL_REG->ctrl &=  ~0x2; 

    //Set RDMA param.
    VPU_DMACTRL_REG->rdma2d_base = u32RdPAddr;
    VPU_DMACTRL_REG->rdma2d_stride = u16RdSize;
    //1D RDMA
    VPU_DMACTRL_REG->rdma2d_size = ((u16RdSize&0xFFFF) << 16) | (1&0xFFFF);
    VPU_DMACTRL_REG->ctrl = 0x0F0F0F00 | ((eDMAMode)&0x7)<<4 | ((eDMAMinorMode)&0x3)<<2; //mode[0:2D dma, 2:RLE Encode, 7:DMA2OTF/OTF2DMA]

    VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;
    VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
    VPU_INTC_REG->intc_mask &= ~(0x80); //mask 1:block, 0:release
    VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;

    //Set WDMA param.
    VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
    if(u32WrPAddr != 0)
    {
        VPU_DMACTRL_REG->wdma2d_base = u32WrPAddr;
        VPU_DMACTRL_REG->wdma2d_stride = u32WrStride;
        VPU_DMACTRL_REG->wdma2d_size = ((u16WrWidth&0xFFFF) << 16) | (u16WrHeight&0xFFFF);
        VPU_DMACTRL_REG->ctrl &= ~(1<<7); //1d wdma disable
    }

    VPU_DMACTRL_REG->ctrl |= 0x1; //b'0 : start

    if( (eDMAMode != eDMA_OTF) )
    {
        if(u32TimeOut)
        {
            ret = PPDRV_VPU_DMA_DrvWaitDone(u32TimeOut);
        }
    }

    return(ret);
}

PP_RESULT_E PPDRV_VPU_DMA_2Dto1D_Start(const _eDMAMODE eDMAMode, const _eDMA_MINORMODE eDMAMinorMode, const _eRDMATYPE eRDMAType, const _eWDMATYPE eWDMAType, const _eOTF2DMATYPE eOTF2DMAType, const PP_U16 u16RdWidth, const PP_U16 u16RdHeight, const PP_U32 u32RdPAddr, const PP_U32 u32RdStride, const PP_U32 u32WrPAddr, const PP_U16 u16WrSize, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;

    //wait previous dma done.

    if( (u32WrPAddr == 0) || (u16WrSize == 0) || (u32RdStride == 0) || (u16RdWidth == 0) || (u16RdHeight == 0) )
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(eERROR_FAILURE);
    }
    if( (eDMAMode != eDMA_2D) || (eRDMAType != eRDMA_2D) || (eWDMAType != eWDMA_1D) || (eDMAMinorMode != eDMA_MINOR_COPY) )
    {
        LOG_CRITICAL("Invalid dma param.\n");
        return(eERROR_FAILURE);
    }
    if((u16RdWidth & 0xF) != 0)
    {
        LOG_CRITICAL("Invalid dma param. Not align 16byte\n");
        return(eERROR_FAILURE);
    }

    //LOG_DEBUG("Dma mode:%d-%d, typeRW:%d/%d, OTF2DMA type:%d, rd:%08x, rdSize:%08x, wd:%04x, ht:%04x, wr:%08x, strideW:%08x, time:%d",
    //        eDMAMode, eDMAMinorMode, eRDMAType, eWDMAType, eOTF2DMAType, u32RdPAddr, u16RdSize, u16RdWidth, u16RdHeight, u32RdPAddr, u32RdStride, u32TimeOut);

    //Clear dma irq_done
    VPU_DMACTRL_REG->ctrl &=  ~0x2; 

    //Set RDMA param.
    VPU_DMACTRL_REG->rdma2d_base = u32RdPAddr;
    VPU_DMACTRL_REG->rdma2d_stride = u32RdStride;
    VPU_DMACTRL_REG->rdma2d_size = ((u16RdWidth&0xFFFF) << 16) | (u16RdHeight&0xFFFF);
    VPU_DMACTRL_REG->ctrl = 0x0F0F0F00 | ((eDMAMode)&0x7)<<4 | ((eDMAMinorMode)&0x3)<<2; //mode[0:2D dma, 2:RLE Encode, 7:DMA2OTF/OTF2DMA]

    VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;
    VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
    VPU_INTC_REG->intc_mask &= ~(0x80); //mask 1:block, 0:release
    VPU_FB_CONFIG_REG->fld.dma2otf_enable = 0;

    //Set WDMA param.
    VPU_FB_CONFIG_REG->fld.otf2dma_enable = 0;
    if(u32WrPAddr != 0)
    {
        VPU_DMACTRL_REG->wdma2d_base = u32WrPAddr;
        VPU_DMACTRL_REG->wdma2d_stride = u16WrSize;
        VPU_DMACTRL_REG->wdma2d_size = ((u16WrSize&0xFFFF) << 16) | (1&0xFFFF);
        VPU_DMACTRL_REG->ctrl &= ~(1<<7); //1d wdma disable
    }

    VPU_DMACTRL_REG->ctrl |= 0x1; //b'0 : start

    if( (eDMAMode != eDMA_OTF) )
    {
        if(u32TimeOut)
        {
            ret = PPDRV_VPU_DMA_DrvWaitDone(u32TimeOut);
        }
    }

    return(ret);
}

PP_RESULT_E PPDRV_VPU_HAMMINGD_Start(_VPUStatus *pVPUStatus, const PP_U32 u32TimeOut)
{
    PP_RESULT_E ret = eSUCCESS;
    PP_S32 zoneNum;
    PP_S32 inxFrmPrev, inxFrmLast;
    PP_U16 u16FCEndValuePrev, u16FCEndValueLast;

    inxFrmLast = pVPUStatus->stFrmInx.inxFrmLast;
    inxFrmPrev = pVPUStatus->stFrmInx.inxFrmPrev;

    /*
       LOG_DEBUG("framinx cur(%d) prev(%d) last(%d)\n",
       pVPUStatus->stFrmInx.inxFrmCur,
       pVPUStatus->stFrmInx.inxFrmPrev,
       pVPUStatus->stFrmInx.inxFrmLast);
       */

    if( (inxFrmLast < 0) || (inxFrmPrev < 0) )
    {
        LOG_CRITICAL("Invalid frame index. Not ready prev or last frame.\n");
        return(eERROR_FAILURE);
    }


    for (zoneNum = 0; zoneNum < eVPU_ZONE_MAX; zoneNum++)
    {
        if(pVPUStatus->pVpuZoneParam[zoneNum].isEnable == TRUE)
        {
            u16FCEndValueLast = pVPUStatus->u32FCEndValue[inxFrmLast][zoneNum];
            u16FCEndValuePrev = pVPUStatus->u32FCEndValue[inxFrmPrev][zoneNum];
            //LOG_DEBUG("Hamming zone:%d, endFcValuePrev:%d endFcValueLast:%d\n", zoneNum, u16FCEndValuePrev, u16FCEndValueLast);

            VPU_HAMMINGD_CTRL_REG->prev_desc_base = (PP_U32)pVPUStatus->ppBufBrief[inxFrmPrev][zoneNum];
            VPU_HAMMINGD_CTRL_REG->curr_desc_base = (PP_U32)pVPUStatus->ppBufBrief[inxFrmLast][zoneNum];
            VPU_HAMMINGD_CTRL_REG->prev_xy_base = (PP_U32)pVPUStatus->ppBufFast[inxFrmPrev][zoneNum];
            VPU_HAMMINGD_CTRL_REG->curr_xy_base = (PP_U32)pVPUStatus->ppBufFast[inxFrmLast][zoneNum];
            VPU_HAMMINGD_CTRL_REG->result_base = (PP_U32)pVPUStatus->pBufHDMatchAddr[zoneNum];

            VPU_HAMMINGD_CTRL_REG->prev_count = u16FCEndValuePrev;
            VPU_HAMMINGD_CTRL_REG->curr_count = u16FCEndValueLast;

            VPU_HAMMINGD_CTRL_REG->ham_th = pVPUStatus->pVpuZoneParam[zoneNum].hd_threshold;

            VPU_HAMMINGD_CTRL_REG->max_th.var.x_th = (pVPUStatus->pVpuZoneParam[zoneNum].match_dist.x_min << 16) | pVPUStatus->pVpuZoneParam[zoneNum].match_dist.x_max;
            VPU_HAMMINGD_CTRL_REG->max_th.var.y_th = (pVPUStatus->pVpuZoneParam[zoneNum].match_dist.y_min << 16) | pVPUStatus->pVpuZoneParam[zoneNum].match_dist.y_max;

            VPU_HAMMINGD_CTRL_REG->min_th.var.x_th = (-2 << 16) | 2;
            VPU_HAMMINGD_CTRL_REG->min_th.var.y_th = (-2 << 16) | 2;

            VPU_HAMMINGD_CTRL_REG->ctrl.var = 0x02;
            VPU_HAMMINGD_CTRL_REG->ctrl.var = 0x01;

            if(u32TimeOut)
            {
                if( (ret = PPDRV_VPU_HAMMINGD_DrvWaitDone(u32TimeOut)) != eSUCCESS)
                {
                    LOG_CRITICAL("Error! TimeOut\n");
                    return(ret);
                }
            }

            pVPUStatus->u32HDMatchResultCount[zoneNum] = VPU_HAMMINGD_CTRL_REG->num_result;
            //LOG_DEBUG("HammingD result count:%d\n", pVPUStatus->u32HDMatchResultCount[zoneNum]);

#if 0
            //print moving status
            {
                PP_U32 i;
                PP_U32 diff_x, diff_y;
                pVPU_MATCHING_RESULT_POS_T move = (pVPU_MATCHING_RESULT_POS_T)pVPUStatus->pVBufHDMatchAddr[zoneNum];
                for (i = 0; i < pVPUStatus->u32HDMatchResultCount[zoneNum]; i++)
                {
                    diff_x = move[i].x2 - move[i].x1;
                    diff_y = move[i].y2 - move[i].y1;
                    LOG_DEBUG ("HammD(%u:%u) : (%u, %u) => (%u, %u) %d/%d\n", zoneNum, i, move[i].x1, move[i].y1, move[i].x2, move[i].y2, diff_x, diff_y);
                }
            }
#endif
        }
    }

    return(ret);
}

PP_RESULT_E PPDRV_VPU_WaitFrmUpdate(PP_S32 timeOutMsec)
{
    PP_U32 u32Timeout = portMAX_DELAY;

    if(timeOutMsec >= 0)
    {
        u32Timeout = OSAL_ms2ticks(timeOutMsec);
    }

    if( xSemaphoreTake(gpVPUConfig->waitFrmUpdate, u32Timeout) == pdFALSE )
    {
        LOG_CRITICAL("Timeout! FrmUpdate.\n");
        return(eERROR_FAILURE);
    }

    return(eSUCCESS);
}

PP_RESULT_E PPDRV_VPU_SetConfig(const PP_S32 eVpuChannel, const VPU_SIZE_T imageInSize)
{
    PP_S32 i;
    PP_S32 vpuBufFrameInx = 0;
    VPU_SIZE_T imageVpuSize;
    VPU_RECT_T roiSize;
    USER_VPU_FB_CONFIG_U stVPUFBConfig;
    PP_S32 bScaleEnable = FALSE;

    PPDRV_VPU_UnsetConfig();

    memset(&gVPUConfig, 0, sizeof(_VPUConfig));

    if( (gVPUConfig.verVPU = PPDRV_VPU_GetVer()) == 0)
    {
        return(eERROR_FAILURE);
    }

    LOG_DEBUG("VPU in path:%s\n", (eVpuChannel == eVPU_CHANNEL_VIN)?"VIN":((eVpuChannel == eVPU_CHANNEL_QUAD)?"QUAD":"SVM") );

    if( (gpVPUConfig->lockFast = xSemaphoreCreateBinary()) == NULL)
    {
        LOG_DEBUG("ERROR! can't create lockFast\n");
        return(eERROR_FAILURE);
    }

    if( (gpVPUConfig->lockHammingD = xSemaphoreCreateBinary()) == NULL)
    {
        LOG_DEBUG("ERROR! can't create lockHammingD\n");
        return(eERROR_FAILURE);
    }

    if( (gpVPUConfig->lockDMA = xSemaphoreCreateBinary()) == NULL)
    {
        LOG_DEBUG("ERROR! can't create lockDMA\n");
        return(eERROR_FAILURE);
    }

    imageVpuSize.width = imageInSize.width;
    imageVpuSize.height = imageInSize.height;

    /* VPU max size is 1280x720 */
    if(imageVpuSize.width >= VPU_MAX_HSIZE) imageVpuSize.width = VPU_MAX_HSIZE;
    if(imageVpuSize.height >= VPU_MAX_VSIZE) imageVpuSize.height = VPU_MAX_VSIZE;

    if( (imageVpuSize.width == 0) || (imageVpuSize.height == 0) )
    {
        LOG_DEBUG("ERROR! can't support resolution(%dx%d)\n", imageVpuSize.width, imageVpuSize.height);
        return(eERROR_FAILURE);
    }

    gpVPUConfig->imageInSize.width = (imageInSize.width);
    gpVPUConfig->imageInSize.height = (imageInSize.height);
    gpVPUConfig->imageVpuSize.width = (imageVpuSize.width);
    gpVPUConfig->imageVpuSize.height = (imageVpuSize.height);
    LOG_DEBUG("VPU image In(%dx%d), VPU(%dx%d)\n", 
            gpVPUConfig->imageInSize.width,
            gpVPUConfig->imageInSize.height,
            gpVPUConfig->imageVpuSize.width,
            gpVPUConfig->imageVpuSize.height);


#ifdef ALLOC_DRAM_RESERVED
    gpVPUConfig->u32BufFastZoneSize = VPU_BUF_FASTZONE_SIZE;
    gpVPUConfig->u32BufFastSize = VPU_BUF_FAST_SIZE;
    LOG_DEBUG("Fast zone size:0x%08x, size:0x%08x(*FrameMax_%d)\n", gpVPUConfig->u32BufFastZoneSize, gpVPUConfig->u32BufFastSize, eVPU_FRAME_MAX);

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if( (gpVPUConfig->ppBufFast[vpuBufFrameInx][0] = (PP_U32 *)gstDramReserved.u32VpuBufFast[vpuBufFrameInx]) == NULL)
        {
            LOG_DEBUG("ERROR! can't allocate memory\n");
            return(eERROR_NO_MEM);
        }
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            gpVPUConfig->ppBufFast[vpuBufFrameInx][i] = (PP_U32 *)((PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][0] + (gpVPUConfig->u32BufFastZoneSize*i));
            gpVPUConfig->ppVBufFast[vpuBufFrameInx][i] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][i]);
            //LOG_DEBUG("Fast zone%d, addr:0x%08x\n", i, (PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][i]);
        }
    }
#else
    gpVPUConfig->u32BufFastZoneSize =
        ((((VPU_MAX_FAST_LIMIT_COUNT + (VPU_FAST_ALIGN_UNITCNT-1))/VPU_FAST_ALIGN_UNITCNT)*VPU_FAST_ALIGN_UNITCNT) * VPU_FAST_UNIT_SIZE);
    gpVPUConfig->u32BufFastSize = (gpVPUConfig->u32BufFastZoneSize * eVPU_ZONE_MAX);
    LOG_DEBUG("Fast zone size:0x%08x, size:0x%08x(*FrameMax_%d)\n", gpVPUConfig->u32BufFastZoneSize, gpVPUConfig->u32BufFastSize, eVPU_FRAME_MAX);

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if( (gpVPUConfig->ppBufFast[vpuBufFrameInx][0] = (PP_U32 *)OSAL_malloc(gpVPUConfig->u32BufFastSize+255)) == NULL)
        {
            LOG_DEBUG("ERROR! can't allocate memory\n");
            return(eERROR_NO_MEM);
        }
        gpVPUConfig->ppBufFast[vpuBufFrameInx][0] = (PP_U32 *)(((PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][0]+255)&0xFFFFFF00); //get 256byte align address
        //LOG_DEBUG("Fast frame:%d buf base:0x%08x\n", vpuBufFrameInx, (PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][0]);
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            gpVPUConfig->ppBufFast[vpuBufFrameInx][i] = (PP_U32 *)((PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][0] + (gpVPUConfig->u32BufFastZoneSize*i));
            gpVPUConfig->ppVBufFast[vpuBufFrameInx][i] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][i]);
            //LOG_DEBUG("Fast zone addr:0x%08x\n", (PP_U32)gpVPUConfig->ppBufFast[vpuBufFrameInx][i]);
        }
    }
#endif // ALLOC_DRAM_RESERVED

#ifdef ALLOC_DRAM_RESERVED
    gpVPUConfig->u32BufBriefZoneSize = VPU_BUF_BRIEFZONE_SIZE;
    gpVPUConfig->u32BufBriefSize = VPU_BUF_BRIEF_SIZE;
    LOG_DEBUG("Brief zone size:0x%08x, size:0x%08x(*FrameMax_%d)\n", gpVPUConfig->u32BufBriefZoneSize, gpVPUConfig->u32BufBriefSize, eVPU_FRAME_MAX);

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if( (gpVPUConfig->ppBufBrief[vpuBufFrameInx][0] = (PP_U32 *)gstDramReserved.u32VpuBufBrief[vpuBufFrameInx]) == NULL)
        {
            LOG_DEBUG("ERROR! can't allocate memory\n");
            return(eERROR_NO_MEM);
        }
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            gpVPUConfig->ppBufBrief[vpuBufFrameInx][i] = (PP_U32 *)((PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][0] + (gpVPUConfig->u32BufBriefZoneSize*i));
            gpVPUConfig->ppVBufBrief[vpuBufFrameInx][i] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][i]);
            //LOG_DEBUG("Brief zone%d addr:0x%08x\n", i, (PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][i]);
        }
    }
#else
    gpVPUConfig->u32BufBriefZoneSize = (VPU_MAX_FAST_LIMIT_COUNT * VPU_BRIEF_UNIT_SIZE);
    gpVPUConfig->u32BufBriefSize = (gpVPUConfig->u32BufBriefZoneSize * eVPU_ZONE_MAX);
    LOG_DEBUG("Brief zone size:0x%08x, size:0x%08x(*FrameMax_%d)\n", gpVPUConfig->u32BufBriefZoneSize, gpVPUConfig->u32BufBriefSize, eVPU_FRAME_MAX);

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if( (gpVPUConfig->ppBufBrief[vpuBufFrameInx][0] = (PP_U32 *)OSAL_malloc(gpVPUConfig->u32BufBriefSize+255)) == NULL)
        {
            LOG_DEBUG("ERROR! can't allocate memory\n");
            return(eERROR_NO_MEM);
        }
        gpVPUConfig->ppBufBrief[vpuBufFrameInx][0] = (PP_U32 *)(((PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][0]+255)&0xFFFFFF00); //get 256byte align address
        //LOG_DEBUG("Brief frame:%d buf base:0x%08x\n", vpuBufFrameInx, (PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][0]);
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            gpVPUConfig->ppBufBrief[vpuBufFrameInx][i] = (PP_U32 *)((PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][0] + (gpVPUConfig->u32BufBriefZoneSize*i));
            gpVPUConfig->ppVBufBrief[vpuBufFrameInx][i] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][i]);
            //LOG_DEBUG("Brief zone addr:0x%08x\n", (PP_U32)gpVPUConfig->ppBufBrief[vpuBufFrameInx][i]);
        }
    }
#endif // ALLOC_DRAM_RESERVED

#ifdef ALLOC_DRAM_RESERVED
    gpVPUConfig->u32BufFCZoneSize = VPU_BUF_FCZONE_SIZE;
    gpVPUConfig->u32BufFCSize = VPU_BUF_FC_SIZE;
    LOG_DEBUG("FC height:%d, zone size:0x%08x, size:0x%08x(*FrameMax_%d)\n", gpVPUConfig->imageVpuSize.height, gpVPUConfig->u32BufFCZoneSize, gpVPUConfig->u32BufFCSize, eVPU_FRAME_MAX);

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if( (gpVPUConfig->ppBufFC[vpuBufFrameInx][0] = (PP_U32 *)gstDramReserved.u32VpuBufFC[vpuBufFrameInx]) == NULL)
        {
            LOG_DEBUG("ERROR! can't allocate memory\n");
            return(eERROR_NO_MEM);
        }
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            gpVPUConfig->ppBufFC[vpuBufFrameInx][i] = (PP_U32 *)((PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][0] + (gpVPUConfig->u32BufFCZoneSize*i));
            gpVPUConfig->ppVBufFC[vpuBufFrameInx][i] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][i]);
            //LOG_DEBUG("FC zone%d addr:0x%08x\n", i, (PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][i]);
        }
    }
#else
    gpVPUConfig->u32BufFCZoneSize = (VPU_MAX_VSIZE * VPU_FC_UNIT_SIZE);
    gpVPUConfig->u32BufFCSize = (gpVPUConfig->u32BufFCZoneSize * eVPU_ZONE_MAX);
    LOG_DEBUG("FC height:%d, zone size:0x%08x, size:0x%08x(*FrameMax_%d)\n", gpVPUConfig->imageVpuSize.height, gpVPUConfig->u32BufFCZoneSize, gpVPUConfig->u32BufFCSize, eVPU_FRAME_MAX);

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if( (gpVPUConfig->ppBufFC[vpuBufFrameInx][0] = (PP_U32 *)OSAL_malloc(gpVPUConfig->u32BufFCSize+255)) == NULL)
        {
            LOG_DEBUG("ERROR! can't allocate memory\n");
            return(eERROR_NO_MEM);
        }
        gpVPUConfig->ppBufFC[vpuBufFrameInx][0] = (PP_U32 *)(((PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][0]+255)&0xFFFFFF00); //get 256byte align address
        //LOG_DEBUG("FC frame:%d buf base:0x%08x\n", vpuBufFrameInx, (PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][0]);
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            gpVPUConfig->ppBufFC[vpuBufFrameInx][i] = (PP_U32 *)((PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][0] + (gpVPUConfig->u32BufFCZoneSize*i));
            gpVPUConfig->ppVBufFC[vpuBufFrameInx][i] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][i]);
            //LOG_DEBUG("FC zone addr:0x%08x\n", (PP_U32)gpVPUConfig->ppBufFC[vpuBufFrameInx][i]);
        }
    }
#endif // ALLOC_DRAM_RESERVED

#ifdef ALLOC_DRAM_RESERVED
    gpVPUConfig->u32BufHDMatchSize = VPU_BUF_HDMATCHZONE_SIZE;
    LOG_DEBUG("HDMatch size:0x%08x(*Zone_%d)\n", gpVPUConfig->u32BufHDMatchSize, eVPU_ZONE_MAX);
    for(i = 0; i < eVPU_ZONE_MAX; i++)
    {
        gpVPUConfig->pBufHDMatchAddr[i] = (pVPU_MATCHING_RESULT_POS_T)gstDramReserved.u32VpuBufHDMatch[i];
        gpVPUConfig->pVBufHDMatchAddr[i] = (pVPU_MATCHING_RESULT_POS_T)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->pBufHDMatchAddr[i]);
        //LOG_DEBUG("HDMatch zone%d, addr:0x%08x\n", i, (PP_U32)gpVPUConfig->pBufHDMatchAddr[i]);
    }
#else
    gpVPUConfig->u32BufHDMatchSize = (VPU_MAX_HDMATCH * sizeof(VPU_MATCHING_RESULT_POS_T));
    LOG_DEBUG("HDMatch size:0x%08x(*Zone_%d)\n", gpVPUConfig->u32BufHDMatchSize, eVPU_ZONE_MAX);
    for(i = 0; i < eVPU_ZONE_MAX; i++)
    {
        gpVPUConfig->pBufHDMatchAddr[i] = (pVPU_MATCHING_RESULT_POS_T)OSAL_malloc(gpVPUConfig->u32BufHDMatchSize+255);
        gpVPUConfig->pBufHDMatchAddr[i] = (pVPU_MATCHING_RESULT_POS_T)((((PP_U32)gpVPUConfig->pBufHDMatchAddr[i])+255)&0xFFFFFF00); //get 256byte align address
        gpVPUConfig->pVBufHDMatchAddr[i] = (pVPU_MATCHING_RESULT_POS_T)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->pBufHDMatchAddr[i]);
    }
#endif

    /* for capture. define max size(1920x1080x16bit) */
#ifdef ALLOC_DRAM_RESERVED
    gpVPUConfig->u32BufCaptureSize = VPU_RESERVED_BUFFSIZE;
    LOG_DEBUG("ImageCaptureBuf0 size:0x%08x\n", gpVPUConfig->u32BufCaptureSize);
    if( (gpVPUConfig->pBufCapture[0] = (PP_U32 *)gstDramReserved.u32CaptureRsvBuff[0]) == NULL)
    {
        LOG_DEBUG("ERROR! can't allocate memory\n");
        return(eERROR_NO_MEM);
    }
    gpVPUConfig->pVBufCapture[0] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->pBufCapture[0]);
#else
    gpVPUConfig->u32BufCaptureSize = (((1920 * 1080 * 2) + 255) & 0xFFFFFF00);
    LOG_DEBUG("ImageCaptureBuf0 size:0x%08x\n", gpVPUConfig->u32BufCaptureSize);
    if( (gpVPUConfig->pBufCapture[0] = (PP_U32 *)OSAL_malloc(gpVPUConfig->u32BufCaptureSize+255)) == NULL)
    {
        LOG_DEBUG("ERROR! can't allocate memory\n");
        return(eERROR_NO_MEM);
    }
    gpVPUConfig->pBufCapture[0] = (PP_U32 *)(((PP_U32)gpVPUConfig->pBufCapture[0]+255)&0xFFFFFF00); //get 256byte align address
    gpVPUConfig->pVBufCapture[0] = (PP_U32 *)ADDR_NON_CACHEABLE((PP_U32)gpVPUConfig->pBufCapture[0]);
#endif

    PPDRV_VPU_FAST_InitFrmInx();

    if( (gpVPUConfig->waitFrmUpdate = xSemaphoreCreateBinary()) == NULL)
    {
        LOG_DEBUG("ERROR! can't create waitFast\n");
        return(eERROR_FAILURE);
    }

    if( PPDRV_VPU_FAST_Initialize() != eSUCCESS)
    {
        LOG_DEBUG("ERROR! can't Intialize FAST\n");
        PPDRV_VPU_UnsetConfig();
        return(eERROR_FAILURE);
    }

    // Set scale
    if( (imageInSize.width > imageVpuSize.width) || (imageInSize.height > imageVpuSize.height) )
    {
        LOG_DEBUG("Scale(%dx%d->%dx%d)\n", imageInSize.width, imageInSize.height, imageVpuSize.width, imageVpuSize.height);
        PPDRV_VPU_FAST_SetScale(imageInSize, imageVpuSize);
        bScaleEnable = TRUE;
    }
    else
    {
        bScaleEnable = FALSE;
    }

    // channel
    PPDRV_VPU_FAST_SetInputSize(imageInSize); 

    // roiSize
    roiSize.x = 0; roiSize.y = 0; roiSize.width = imageVpuSize.width; roiSize.height = imageVpuSize.height;
    memcpy(&gpVPUConfig->roiFastSize, &roiSize, sizeof(roiSize));
    PPDRV_VPU_FAST_SetROI(roiSize);

    // set fast config
    PPDRV_VPU_FAST_GetConfig(&stVPUFBConfig);
    stVPUFBConfig.fld.fast_n = FAST_N_NUM;
    stVPUFBConfig.fld.ch_sel = eVpuChannel;
    stVPUFBConfig.fld.use_5x3_nms = FALSE;
    stVPUFBConfig.fld.brief_enable = TRUE;
    stVPUFBConfig.fld.roi_enable = FALSE;
    stVPUFBConfig.fld.scl_enable = bScaleEnable;
    stVPUFBConfig.fld.dma2otf_enable = FALSE;
    stVPUFBConfig.fld.otf2dma_enable = FALSE;
    if( (imageInSize.height == 240) || (imageInSize.height == 288) ) //interlace
    {    
        stVPUFBConfig.fld.field_sel = (0<<1)|TRUE;
    }    
    else
    {
        stVPUFBConfig.fld.field_sel = FALSE;
    }
    PPDRV_VPU_FAST_SetConfig(&stVPUFBConfig);

    // zone 
    gpVPUConfig->pVpuZoneParam = (VPU_ZONINFO_T *)VPU_ZONE_PARAM;
    for(i = 0; i < eVPU_ZONE_MAX; i++)
    {
        PPDRV_VPU_FAST_SetZone(i, (VPU_ZONINFO_T *)&gpVPUConfig->pVpuZoneParam[i]);
    }

    // brief LUT //
    PPDRV_VPU_BRIEF_SetLut((VPU_BRIEF_LUT_VALUE_T *)&VPU_BRIEF_LUT_Default);

    PPDRV_VPU_BRIEF_SetProgramFilter (VPU_BRIEF_COEF0FILTER, VPU_BRIEF_COEF1FILTER);

    VPU_INTC_REG->intc_mask = 0; //mask 1:block, 0:release

    //register proc infomation.
    SYS_PROC_addIrq(stVPU_PROC_irqs);
    SYS_PROC_addDevice(stVPU_PROC_devices);

    gpVPUConfig->u8Initialized = TRUE;

    return(eSUCCESS);
}

void PPDRV_VPU_UnsetConfig(void)
{

#ifndef ALLOC_DRAM_RESERVED
    PP_S32 vpuBufFrameInx = 0;

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if(gpVPUConfig->ppBufFast[vpuBufFrameInx][0])
        {
            OSAL_free(gpVPUConfig->ppBufFast[vpuBufFrameInx][0]);
        }
    }

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if(gpVPUConfig->ppBufBrief[vpuBufFrameInx][0])
        {
            OSAL_free(gpVPUConfig->ppBufBrief[vpuBufFrameInx][0]);
        }
    }

    for(vpuBufFrameInx = 0; vpuBufFrameInx < eVPU_FRAME_MAX; vpuBufFrameInx++)
    {
        if(gpVPUConfig->ppBufFC[vpuBufFrameInx][0])
        {
            OSAL_free(gpVPUConfig->ppBufFC[vpuBufFrameInx][0]);
        }
    }
#endif // ALLOC_DRAM_RESERVED

#ifndef ALLOC_DRAM_RESERVED
    {
        PP_S32 i;
        for(i = 0; i < eVPU_ZONE_MAX; i++)
        {
            if(gpVPUConfig->pBufHDMatchAddr[i])
            {
                OSAL_free(gpVPUConfig->pBufHDMatchAddr[i]);
            }
        }

        for(vpuBufFrameInx = 0; vpuBufFrameInx < 1; vpuBufFrameInx++)
        {
            if(gpVPUConfig->pBufCapture[vpuBufFrameInx])
            {
                OSAL_free(gpVPUConfig->pBufCapture[vpuBufFrameInx]);
            }
        }
    }
#endif // ALLOC_DRAM_RESERVED
}

PP_RESULT_E PPDRV_VPU_GetStatus(_VPUStatus *pVPUStatus)
{
    PP_RESULT_E ret = eSUCCESS;

    if ( pVPUStatus == NULL )
    {
        return(eERROR_VPU_NOT_INITIALIZE);
    }

    pVPUStatus->u32BufFastZoneSize = gpVPUConfig->u32BufFastZoneSize;
    pVPUStatus->u32BufFastSize = gpVPUConfig->u32BufFastSize;
    memcpy(pVPUStatus->ppBufFast, gpVPUConfig->ppBufFast, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);
    memcpy(pVPUStatus->ppVBufFast, gpVPUConfig->ppVBufFast, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);

    pVPUStatus->u32BufBriefZoneSize = gpVPUConfig->u32BufBriefZoneSize;
    pVPUStatus->u32BufBriefSize = gpVPUConfig->u32BufBriefSize;
    memcpy(pVPUStatus->ppBufBrief, gpVPUConfig->ppBufBrief, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);
    memcpy(pVPUStatus->ppVBufBrief, gpVPUConfig->ppVBufBrief, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);

    pVPUStatus->u32BufFCZoneSize = gpVPUConfig->u32BufFCZoneSize;
    pVPUStatus->u32BufFCSize = gpVPUConfig->u32BufFCSize;
    memcpy(pVPUStatus->ppBufFC, gpVPUConfig->ppBufFC, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);
    memcpy(pVPUStatus->ppVBufFC, gpVPUConfig->ppVBufFC, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);

    memcpy(pVPUStatus->u32FCEndValue, gpVPUConfig->u32FCEndValue, sizeof(PP_U32)*eVPU_FRAME_MAX*eVPU_ZONE_MAX);

    pVPUStatus->u32BufHDMatchSize = gpVPUConfig->u32BufHDMatchSize;
    memcpy(pVPUStatus->pBufHDMatchAddr, gpVPUConfig->pBufHDMatchAddr, sizeof(PP_U32)*eVPU_ZONE_MAX);
    memcpy(pVPUStatus->pVBufHDMatchAddr, gpVPUConfig->pVBufHDMatchAddr, sizeof(PP_U32)*eVPU_ZONE_MAX);
    memcpy(pVPUStatus->u32HDMatchResultCount, gpVPUConfig->u32HDMatchResultCount, sizeof(PP_U32)*eVPU_ZONE_MAX);

    pVPUStatus->u32BufCaptureSize = gpVPUConfig->u32BufCaptureSize;
    pVPUStatus->pBufCapture[0] = gpVPUConfig->pBufCapture[0];
    pVPUStatus->pVBufCapture[0] = gpVPUConfig->pVBufCapture[0];

    memcpy(&pVPUStatus->roiFastSize, &gpVPUConfig->roiFastSize, sizeof(VPU_RECT_T));

    memcpy(&pVPUStatus->stFrmInx, &gpVPUConfig->stFrmInx, sizeof(_FrmInx));
    pVPUStatus->pVpuZoneParam = gpVPUConfig->pVpuZoneParam;

    return(ret);
}

PP_RESULT_E PPDRV_VPU_Initialize(void)
{
    PP_S32 vinWidth, vinHeight;
    _VID_RESOL eVidResol;
    PP_S32 eVpuChannel = VPU_INPUT_SEL_Default;

    VPU_SIZE_T imageInSize;

    if(eVpuChannel == eVPU_CHANNEL_VIN)
    {
        if( PPDRV_VIN_GetResol(BD_VIN_FMT, &vinWidth, &vinHeight, &eVidResol) != eSUCCESS )
        {
            LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
    }
    else if(eVpuChannel == eVPU_CHANNEL_QUAD)
    {
        if( PPDRV_VIN_GetResol(BD_QUAD_OUT_FMT, &vinWidth, &vinHeight, &eVidResol) != eSUCCESS )
        {
            LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
    }
    else if(eVpuChannel == eVPU_CHANNEL_SVM)
    {
        if( PPDRV_VIN_GetResol(BD_SVM_OUT_FMT, &vinWidth, &vinHeight, &eVidResol) != eSUCCESS )
        {
            LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
            return eERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        LOG_WARNING("[EDGE(%s_%d)]: eERROR_INVALID_ARGUMENT\n", __FUNCTION__, __LINE__);
        return eERROR_INVALID_ARGUMENT;
    }

    imageInSize.width = vinWidth;
    imageInSize.height = vinHeight;

    return(PPDRV_VPU_SetConfig(eVpuChannel, imageInSize));

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PP_RESULT_E VPU_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
    //	PRINT_PROC_IRQ(NAME, NUM, CNT)

    PRINT_PROC_IRQ("VPU", IRQ_VPU0_VECTOR, gu32VPUIntcCnt);
    PRINT_PROC_IRQ("(VPU_FAST)", IRQ_VPU0_VECTOR, gu32VPUIntcSubCnt[eIRQ_FAST]);
    PRINT_PROC_IRQ("(VPU_DMA)", IRQ_VPU0_VECTOR, gu32VPUIntcSubCnt[eIRQ_DMA]);
    PRINT_PROC_IRQ("(VPU_HAMD)", IRQ_VPU0_VECTOR, gu32VPUIntcSubCnt[eIRQ_HAMMINGD]);

    return(eSUCCESS);
}

PP_RESULT_E VPU_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
    _VPUConfig *pVPUConfig = (_VPUConfig *)gpVPUConfig;

    PP_S32 i;
    PP_U32 reg;

    if( (argc) && (strcmp(argv[0], stVPU_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
    {
        return(eERROR_FAILURE);
    }
    printf("\n%s Device Info -------------\n", stVPU_PROC_devices[0].pName);

    printf("### VPU info(Driver Version : v%s) ### \n", _VER_VPU);

    printf("---------------------- VPU info ---------------------\n");
    printf("  VPUVer:0x%04X (Initialized:%s)\n", pVPUConfig->verVPU, ((gpVPUConfig->u8Initialized == 0)?"Fail":"Success")); 

    if( pVPUConfig->verVPU == 0) return(eERROR_FAILURE);

    printf("------------------ vpu setting info -------------------------\n");
    printf("  Frame Cnt: %d\n", eVPU_FRAME_MAX);
    printf("  Zone Cnt: %d\n", eVPU_ZONE_MAX);
    printf("  Image Size: \n");
    printf("    Input     VPUsize   roiSize\n");
    printf("    %04dx%04d %04dx%04d %04dx%04d_%04dx%04d\n", 
            pVPUConfig->imageInSize.width,
            pVPUConfig->imageInSize.height,
            pVPUConfig->imageVpuSize.width,
            pVPUConfig->imageVpuSize.height,
            pVPUConfig->roiFastSize.x,
            pVPUConfig->roiFastSize.y,
            pVPUConfig->roiFastSize.width,
            pVPUConfig->roiFastSize.height);
    printf("  Scale: \n");
    printf("    h_scale  v_scale  h_ratio  v_ratio\n");
    reg = VPU_CTRL_REG->scl_size;
    printf("    0x%06x 0x%06x", reg>>16, (reg & 0xFFFF)); 
    reg = VPU_CTRL_REG->scl_ratio;
    printf(" 0x%06x 0x%06x\n", reg >>16, reg&0xFFFF);

    printf("  bandwidth: \n");
    printf("    fast brief counter_limit\n");
    reg = VPU_CTRL_REG->bandwidth;
    printf("    %4x %5x %13d\n", (reg&0xFF), (reg>>8)&0xFF, reg>>16);

    printf("  Buffer Size: \n");
    printf("    FastZone BriefZone FCZone\n");
    printf("    0x%06x 0x%06x  0x%06x\n", 
            pVPUConfig->u32BufFastSize,
            pVPUConfig->u32BufBriefSize,
            pVPUConfig->u32BufFCSize);
    printf("------------------ zone parameter info -------------------------\n");
    printf("  Ch En FastTh FastFilter BriefLUT BriefFilter area                fc_height HdTh matchDist\n");
    for(i = 0; i < eVPU_ZONE_MAX; i++)
    {
        printf("  %2d %2d %6x %10d %8d %11d %04dx%04d_%04dx%04d %9d %4x (%04d_%04d|%04d_%04d)\n", i,
                pVPUConfig->pVpuZoneParam[i].isEnable,
                pVPUConfig->pVpuZoneParam[i].fastThreshold,
                pVPUConfig->pVpuZoneParam[i].fastFilterIndex,
                pVPUConfig->pVpuZoneParam[i].briefLUTIndex,
                pVPUConfig->pVpuZoneParam[i].briefFilterIndex,
                pVPUConfig->pVpuZoneParam[i].sx,
                pVPUConfig->pVpuZoneParam[i].sy,
                pVPUConfig->pVpuZoneParam[i].ex,
                pVPUConfig->pVpuZoneParam[i].ey,
                pVPUConfig->pVpuZoneParam[i].fc_height,
                pVPUConfig->pVpuZoneParam[i].hd_threshold,
                pVPUConfig->pVpuZoneParam[i].match_dist.x_min,
                pVPUConfig->pVpuZoneParam[i].match_dist.x_max,
                pVPUConfig->pVpuZoneParam[i].match_dist.y_min,
                pVPUConfig->pVpuZoneParam[i].match_dist.y_max);
    }

    printf("------------------ VPU Frame process index info -------------------------\n");
    printf("  current prev pprev last doneCount intcIntervalDMA intcIntervalHD intcIntervalFB\n");
    printf("  %7d %4d %5d %4d %9d %15d %14d %14d\n", 
            pVPUConfig->stFrmInx.inxFrmCur,
            pVPUConfig->stFrmInx.inxFrmPrev,
            pVPUConfig->stFrmInx.inxFrmPPrev,
            pVPUConfig->stFrmInx.inxFrmLast,
            pVPUConfig->stFrmInx.frmDoneCount,
            pVPUConfig->u8RunIntcIntervalDMATime,
            pVPUConfig->u8RunIntcIntervalHDTime,
            pVPUConfig->u8RunIntcIntervalFBTime);

    printf("------------------ VPU Currnet FAST(x,y)-brief info -------------------------\n");
    {
        _VPUStatus stVPUStatus;

        PPDRV_VPU_GetStatus(&stVPUStatus);
        //print HW info
        {/*{{{*/
            PP_S32 i;
            PP_S32 inxFrm;
            PP_U32 *pu32BufFast;
            PP_U32 *pu32BufBrief;
            PP_U16 *pu16BufFC;
            PP_U32 u32FCEndPos;

            inxFrm = stVPUStatus.stFrmInx.inxFrmPrev;
            {
                for (i = 0; i < eVPU_ZONE_MAX; i++)
                {
                    if(stVPUStatus.pVpuZoneParam[i].isEnable == 0) continue;
                    printf("  Frm(%d), Zone(%d) :\n", inxFrm, i);

                    pu32BufFast = (PP_U32 *)stVPUStatus.ppVBufFast[inxFrm][i];
                    pu32BufBrief = (PP_U32 *)stVPUStatus.ppVBufBrief[inxFrm][i];
                    pu16BufFC = (PP_U16 *)stVPUStatus.ppVBufFC[inxFrm][i];
                    if( (pu32BufFast == NULL) || (pu32BufBrief == NULL) || (pu16BufFC == NULL) ) continue;

                    u32FCEndPos = stVPUStatus.pVpuZoneParam[i].fc_height;
                    printf("    endpos:%d, endFcValue:%d\n", u32FCEndPos, pu16BufFC[u32FCEndPos-1]);

#if 0
                    {
                        PP_U32 *pBufPos;
                        PP_U32 bitPos;
                        PP_U8 bValue;
                        PP_S32 w, h, k;

                        memset(&stVPUStatus.pVBufCapture[0], 0, stVPUStatus.u32BufCaptureSize);
                        pBufPos = (PP_U32 *)&stVPUStatus.pVBufCapture[0];
                        for (k = 0; k < pu16BufFC[u32FCEndPos-1]; k++)
                        {
                            PP_S32 x, y;
                            PP_U32 *d = (PP_U32 *)&(pu32BufBrief[k*4]);
                            x = ((pu32BufFast[k] & 0xFFFF0000) >> 16);
                            y = ((pu32BufFast[k] & 0x0000FFFF));
                            printf ("      %04d (%4d,%4d) = %08x%08x%08x%08x\n", k, x, y, d[3], d[2], d[1], d[0]);
                            bitPos = (y*pVPUConfig->imageVpuSize.width) + x;
                            pBufPos[(bitPos / (sizeof(PP_U32)*8))] |= 1<<(bitPos%(sizeof(PP_U32)*8));
                        }

                        for(w = 0; w < pVPUConfig->imageVpuSize.width; w++)
                        {
                            printf("%3d:", w);
                            for(h = 0; h < pVPUConfig->imageVpuSize.height; h++)
                            {
                                bitPos = (w*pVPUConfig->imageVpuSize.width) + h;
                                bValue = pBufPos[(bitPos / (sizeof(PP_U32)*8))] & (1<<(bitPos%(sizeof(PP_U32)*8)));
                                printf("%c", ((bValue == 1)?'X':'.'));
                            }
                            printf("\n");
                        }
                    }
#endif
                }
            }
        }/*}}}*/
    }

    printf("------------------ VPU Currnet hammingDistance:(x,y)->(x,y) info -------------------------\n");
    {
        _VPUStatus stVPUStatus;

        PPDRV_VPU_GetStatus(&stVPUStatus);
        PPDRV_VPU_HAMMINGD_Start(&stVPUStatus, 1000);
        //print HW info
        {/*{{{*/
            PP_U32 i;
            for (i = 0; i < eVPU_ZONE_MAX; i++)
            {
                if(stVPUStatus.pVpuZoneParam[i].isEnable == 0) continue;

                if(stVPUStatus.u32HDMatchResultCount[i] > 0)
                {
                    printf("  Moved exist on Zone%d :\n", i);
#if 0
                    {
                        PP_U32 j;
                        pVPU_MATCHING_RESULT_POS_T move;
                        PP_U32 diff_x, diff_y;
                        move = (pVPU_MATCHING_RESULT_POS_T)stVPUStatus.pVBufHDMatchAddr[i];
                        for (j = 0; j < stVPUStatus.u32HDMatchResultCount[i]; j++)
                        {
                            diff_x = move[j].x2 - move[j].x1;
                            diff_y = move[j].y2 - move[j].y1;
                            LOG_DEBUG ("  %4d : (%u, %u) => (%u, %u) %d/%d\n", j, move[j].x1, move[j].y1, move[j].x2, move[j].y2, diff_x, diff_y);
                        }
                    }
#endif
                }
            }
        }/*}}}*/
    }

    printf("--------------------------------------------------------------\n");
    return(eSUCCESS);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
