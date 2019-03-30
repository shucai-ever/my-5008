
#include <stdio.h>
#include <string.h>

#include "pi5008.h"
#include "type.h"
#include "osal.h"
#include "interrupt.h"
#include "nds32.h"

#include "proc.h"
#include "sub_intr.h"
#include "common.h"

#include "task_manager.h"

#include "pvirx_func.h"
#include "pvirx_table.h"
#include "pvirx_user_config.h"

extern sint32	PPDRV_SVM_ISR_VsynCB(PP_S32 argc, const PP_CHAR **argv);

INTR_REG_T	*gpSUBINTC[] = {
  SUB_INTR0_CTRL_REG,
  SUB_INTR1_CTRL_REG,
  SUB_INTR2_CTRL_REG,
  SUB_INTR3_CTRL_REG,
};

void sub_intc_control (const int ctlNum, int en)
{
	if(ctlNum > 3) return;
	gpSUBINTC[ctlNum]->control = (0x0	<< INTR_CONTROL_VAM_BIT		) | 
						(~en	<< INTR_CONTROL_GMASK_BIT	) |
						(0x0	<< INTR_CONTROL_VECTORED_BIT) |
						(~en	<< INTR_CONTROL_nENBIRQ_BIT	) |
						(~en	<< INTR_CONTROL_nENBFIQ_BIT	) ;
}

//inline void GIE_ENABLE()
//void GIE_ENABLE()
void sub_intc_enable(const int ctlNum)
{
	if(ctlNum > 3) return;
	gpSUBINTC[ctlNum]->control = (0x0	<< INTR_CONTROL_VAM_BIT		) | 
						(0x0	<< INTR_CONTROL_GMASK_BIT	) |
						(0x0	<< INTR_CONTROL_VECTORED_BIT) |
						(0x0	<< INTR_CONTROL_nENBIRQ_BIT	) |
						(0x0	<< INTR_CONTROL_nENBFIQ_BIT	) ;
}

//inline void GIE_DISABLE()
void sub_intc_disable(const int ctlNum)
{
	if(ctlNum > 3) return;
	gpSUBINTC[ctlNum]->control = (0x0	<< INTR_CONTROL_VAM_BIT		) | 
						(0x1	<< INTR_CONTROL_GMASK_BIT	) |
						(0x0	<< INTR_CONTROL_VECTORED_BIT) |
						(0x1	<< INTR_CONTROL_nENBIRQ_BIT	) |
						(0x1	<< INTR_CONTROL_nENBFIQ_BIT	) ;
}

unsigned int get_sub_intr_src (const int ctlNum)
{
	if(ctlNum > 3) return 0;
	return gpSUBINTC[ctlNum]->pend;
}

void clr_sub_intr_src (const int ctlNum, unsigned int src)
{
	if(ctlNum > 3) return;
	gpSUBINTC[ctlNum]->i_ispc = src;
}

void initSUBIntr(const int ctlNum)
{
	unsigned int tmp;

	if(ctlNum > 3) return;

	gpSUBINTC[ctlNum]->control = (0x0	<< INTR_CONTROL_VAM_BIT		) | 
						(0x1	<< INTR_CONTROL_GMASK_BIT	) |
						(0x0	<< INTR_CONTROL_VECTORED_BIT) |
						(0x1	<< INTR_CONTROL_nENBIRQ_BIT	) |
						(0x1	<< INTR_CONTROL_nENBFIQ_BIT	) ;

	if(ctlNum == 0) //PVI Rx
	{/*{{{*/
		gpSUBINTC[ctlNum]->mode = ALL_IRQ_SUBINTR0;
		gpSUBINTC[ctlNum]->mask = 0xFFFFFFFF;    // ALL masking

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
        //if undef, disable irq. So don't detect & monitoring job. Just only initialize table setting.
        {
            gpSUBINTC[ctlNum]->mask &= ~( (INTR_MASK << IRQ_PVI_RX_0) | 	
                    (INTR_MASK << IRQ_PVI_RX_1) | 	
                    (INTR_MASK << IRQ_PVI_RX_2) | 	
                    (INTR_MASK << IRQ_PVI_RX_3));
        }
#endif
		gpSUBINTC[ctlNum]->level = (INTR_LEVEL << IRQ_PVI_RX_0		) | 	
			(INTR_LEVEL << IRQ_PVI_RX_1		) | 	
			(INTR_LEVEL << IRQ_PVI_RX_2		) | 	
			(INTR_LEVEL << IRQ_PVI_RX_3		);

		gpSUBINTC[ctlNum]->polarity = 0xFFFFFFF0;    // falling edge and high level(b[3:0])
	}/*}}}*/
	else if(ctlNum == 1) //DIAGNOSYS
	{/*{{{*/
		gpSUBINTC[ctlNum]->mode = ALL_IRQ_SUBINTR1;
		gpSUBINTC[ctlNum]->mask = 0xFFFFFFFF;    // ALL masking

		gpSUBINTC[ctlNum]->mask &= ~( (INTR_MASK << IRQ_GADC1_DIAG) | 	
				(INTR_MASK << IRQ_GADC0_DIAG) | 	
				(INTR_MASK << IRQ_FREEZE_DU_CH3) | 	
				(INTR_MASK << IRQ_FREEZE_DU_CH2) | 	
				(INTR_MASK << IRQ_FREEZE_DU_CH1) | 	
				(INTR_MASK << IRQ_FREEZE_DU_CH0) | 	
				(INTR_MASK << IRQ_LOSS_SYNC_CH4) | 	
				(INTR_MASK << IRQ_LOSS_SYNC_CH3) | 	
				(INTR_MASK << IRQ_LOSS_SYNC_CH2) | 	
				(INTR_MASK << IRQ_LOSS_SYNC_CH1) | 	
				(INTR_MASK << IRQ_LOSS_SYNC_CH0) | 	
				(INTR_MASK << IRQ_LOSS_FMT_CH4) | 	
				(INTR_MASK << IRQ_LOSS_FMT_CH3) | 	
				(INTR_MASK << IRQ_LOSS_FMT_CH2) | 	
				(INTR_MASK << IRQ_LOSS_FMT_CH1) | 	
				(INTR_MASK << IRQ_LOSS_FMT_CH0) | 	
				(INTR_MASK << IRQ_GEN_SYNC) | 	
				(INTR_MASK << IRQ_GEN_VREFLOSS) | 	
				(INTR_MASK << IRQ_CRC_CH4) | 	
				(INTR_MASK << IRQ_CRC_CH3) | 	
				(INTR_MASK << IRQ_CRC_CH2) | 	
				(INTR_MASK << IRQ_CRC_CH1) | 	
				(INTR_MASK << IRQ_CRC_CH0) | 	
				(INTR_MASK << IRQ_CRC_QUAD));

		gpSUBINTC[ctlNum]->level = (INTR_LEVEL << IRQ_GADC1_DIAG		) | 	
			(INTR_LEVEL << IRQ_GADC0_DIAG		) | 	
			(INTR_LEVEL << IRQ_FREEZE_DU_CH3		) | 	
			(INTR_LEVEL << IRQ_FREEZE_DU_CH2		) | 	
			(INTR_LEVEL << IRQ_FREEZE_DU_CH1		) | 	
			(INTR_LEVEL << IRQ_FREEZE_DU_CH0		) | 	
			(INTR_LEVEL << IRQ_LOSS_SYNC_CH4		) |
			(INTR_LEVEL << IRQ_LOSS_SYNC_CH3		) |
			(INTR_LEVEL << IRQ_LOSS_SYNC_CH2		) |
			(INTR_LEVEL << IRQ_LOSS_SYNC_CH1		) |
			(INTR_LEVEL << IRQ_LOSS_SYNC_CH0		) |
			(INTR_LEVEL << IRQ_LOSS_FMT_CH4		) | 	
			(INTR_LEVEL << IRQ_LOSS_FMT_CH3		) | 	
			(INTR_LEVEL << IRQ_LOSS_FMT_CH2		) | 	
			(INTR_LEVEL << IRQ_LOSS_FMT_CH1		) | 	
			(INTR_LEVEL << IRQ_LOSS_FMT_CH0		) | 	
			(INTR_LEVEL << IRQ_GEN_SYNC		) | 	
			(INTR_LEVEL << IRQ_GEN_VREFLOSS		) | 	
			(INTR_LEVEL << IRQ_CRC_CH4		) | 	
			(INTR_LEVEL << IRQ_CRC_CH3		) | 	
			(INTR_LEVEL << IRQ_CRC_CH2		) | 	
			(INTR_LEVEL << IRQ_CRC_CH1		) | 	
			(INTR_LEVEL << IRQ_CRC_CH0		) | 	
			(INTR_LEVEL << IRQ_CRC_QUAD		) ;

		gpSUBINTC[ctlNum]->polarity = 0x003C003F;    // 1:falling/low, 0:rising/high
	}/*}}}*/
#if 0
	else if(ctlNum == 2) //ISP
	{/*{{{*/
		gpSUBINTC[ctlNum]->mode = ALL_IRQ_SUBINTR0;
		gpSUBINTC[ctlNum]->mask = 0xFFFFFFFF;    // ALL masking

		gpSUBINTC[ctlNum]->mask &= ~( (INTR_MASK << IRQ_ISP_0) | 	
				(INTR_MASK << IRQ_ISP_1) | 	
				(INTR_MASK << IRQ_ISP_2) | 	
				(INTR_MASK << IRQ_ISP_3));

		gpSUBINTC[ctlNum]->level = (INTR_EDGE << IRQ_ISP_0		) | 	
			(INTR_EDGE << IRQ_ISP_1		) | 	
			(INTR_EDGE << IRQ_ISP_2		) | 	
			(INTR_EDGE << IRQ_ISP_3		) ;

		gpSUBINTC[ctlNum]->polarity = 0xFFFFFFFF;    // falling edge and high level(b[11:8])
	}/*}}}*/
#endif
	else if(ctlNum == 3) //VSYNC
	{/*{{{*/
		gpSUBINTC[ctlNum]->mode = ALL_IRQ_SUBINTR3;
		gpSUBINTC[ctlNum]->mask = 0xFFFFFFFF;    // ALL masking

		gpSUBINTC[ctlNum]->mask &= ~( (INTR_MASK << IRQ_VSYNC_QUAD) | 	
				(INTR_MASK << IRQ_VSYNC_VIN0) | 	
				(INTR_MASK << IRQ_VSYNC_VIN1) | 	
				(INTR_MASK << IRQ_VSYNC_VIN2) | 	
				(INTR_MASK << IRQ_VSYNC_VIN3) | 	
				(INTR_MASK << IRQ_VSYNC_VIN4) | 	
				(INTR_MASK << IRQ_VSYNC_SVM) | 	
				(INTR_MASK << IRQ_VSYNC_DU));
		gpSUBINTC[ctlNum]->level = ((INTR_EDGE << IRQ_VSYNC_QUAD) | 	
				(INTR_EDGE << IRQ_VSYNC_VIN0) | 	
				(INTR_EDGE << IRQ_VSYNC_VIN1) | 	
				(INTR_EDGE << IRQ_VSYNC_VIN2) | 	
				(INTR_EDGE << IRQ_VSYNC_VIN3) | 	
				(INTR_EDGE << IRQ_VSYNC_VIN4) | 	
				(INTR_EDGE << IRQ_VSYNC_SVM) | 	
				(INTR_EDGE << IRQ_VSYNC_DU));

		gpSUBINTC[ctlNum]->polarity = 0xFFFFFFFF;    // falling edge and high level
	}/*}}}*/
	else
	{
		printf("Invalid ctlNum(%d)\n", ctlNum);
		return;
	}

	// IRQ
	tmp = gpSUBINTC[ctlNum]->i_isprB;
	gpSUBINTC[ctlNum]->i_ispc = tmp;		// Interupt clear
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 gu32SubIntcCnt[4] = {0, };

uint32 gu32PviRxIntcCnt[4] = {0, };

#if 0
uint32 gu32ISPIntcCnt[4] = {0, };
#endif

uint32 gu32DiagIntcCnt[IRQ_GADC1_DIAG+1] = {0, };

uint32 gu32VSYNCIntcCnt[IRQ_VSYNC_DU+1] = {0, };

//PROC irq
PP_RESULT_E IRQSUB0_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stIRQSUB0_PROC_irqs[] = {
	    { .fn = IRQSUB0_PROC_IRQ,	.irqNum = IRQ_0_VECTOR,		.next = (void*)0, },
};
PP_RESULT_E IRQSUB1_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stIRQSUB1_PROC_irqs[] = {
	    { .fn = IRQSUB1_PROC_IRQ,   .irqNum = IRQ_1_VECTOR,		.next = (void*)0, },
};
#if 0
int IRQSUB2_PROC_IRQ(int argc, const char **argv);
struct proc_irq_struct stIRQSUB2_PROC_irqs[] = {
	    { .fn = IRQSUB2_PROC_IRQ,   .irqNum = IRQ_2_VECTOR,		.next = (void*)0, },
};
#endif
PP_RESULT_E IRQSUB3_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stIRQSUB3_PROC_irqs[] = {
	    { .fn = IRQSUB3_PROC_IRQ,   .irqNum = IRQ_24_VECTOR,	.next = (void*)0, },
};


PP_RESULT_E IRQSUB0_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("IRQSUB0", IRQ_0_VECTOR, gu32SubIntcCnt[0]);

	return(eSUCCESS);
}
PP_RESULT_E IRQSUB1_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("IRQSUB1", IRQ_1_VECTOR, gu32SubIntcCnt[1]);

	return(eSUCCESS);
}
#if 0
PP_RESULT_E IRQSUB2_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("IRQSUB2", IRQ_2_VECTOR, gu32SubIntcCnt[2]);

	return(eSUCCESS);
}
#endif
PP_RESULT_E IRQSUB3_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("IRQSUB3", IRQ_24_VECTOR, gu32SubIntcCnt[3]);

	return(eSUCCESS);
}

//PROC device
PP_RESULT_E IRQSUBx_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stIRQSUBx_PROC_devices[] = {
	    { .pName = "IRQSUBx",  .fn = IRQSUBx_PROC_DEVICE,    .next = (void*)0, },
};

PP_RESULT_E IRQSUBx_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	int i;
	    
	if( (argc) && (strcmp(argv[0], stIRQSUBx_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}
	printf("\n%s Device Info -------------\n", stIRQSUBx_PROC_devices[0].pName);
	printf("Sub  mode     mask     level    polarity control  pend     i_ispc\n");

	for(i = 0; i < 4; i++)
	{
		printf("Sub%d ", i);
		printf("%08x ", gpSUBINTC[i]->mode);
		printf("%08x ", gpSUBINTC[i]->mask);
		printf("%08x ", gpSUBINTC[i]->level);
		printf("%08x ", gpSUBINTC[i]->polarity);
		printf("%08x ", gpSUBINTC[i]->control);
		printf("%08x ", gpSUBINTC[i]->pend);
		printf("%08x\n", gpSUBINTC[i]->i_ispc);
	}

	return(eSUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ISR(sub_irq0_isr, num)
{
	const int ctlNum = 0;
	uint32 msk = (1 << num);
	uint32 src;
	int chanAddr;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2); // clear int pending

	gu32SubIntcCnt[0]++;

	src = get_sub_intr_src (ctlNum); 
	clr_sub_intr_src (ctlNum, src);
	//printf("%s: src = 0x%x\n", __FUNCTION__, src);

	/* PVI RX isr processing */
	for(chanAddr = 0; chanAddr < MAX_PVIRX_CHANCNT; chanAddr++)
	{
		if(src & (1<<(IRQ_PVI_RX_0+chanAddr)))
		{
			gu32PviRxIntcCnt[chanAddr]++;

			/* callback function of PVIRX(). */
			PPDRV_PVIRX_Isr(-1, chanAddr);
		}
	}

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}


ISR(sub_irq1_isr, num)
{
	const int ctlNum = 1;
	uint32 msk = (1 << num);
	uint32 src, polarity = gpSUBINTC[ctlNum]->polarity;
	int i;

    int bFlagDisplayFreezeStatus = FALSE;
    unsigned char u8DisplayFreezeStatus[4];

    int bFlagInvalidCamStatus = FALSE;
    unsigned char u8InvalidCamStatus[4];

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2); // clear int pending

	gu32SubIntcCnt[1]++;

	src = get_sub_intr_src (ctlNum); 
	clr_sub_intr_src (ctlNum, src);
	//printf("%s: src = 0x%08x\n", __FUNCTION__, src);

	/* callback function of Diagnosis. */
	for(i = IRQ_CRC_QUAD; i <= IRQ_GADC1_DIAG; i++)
	{
		if(src & (1<<i))
		{
			gu32DiagIntcCnt[i]++;
		}
	}

	if(src & (1<<IRQ_CRC_QUAD))
	{/*{{{*/

		//printf("Diagnosis -> crc_freeze(quad):");
		if(polarity & (1<<IRQ_CRC_QUAD))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_CRC_QUAD);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_CRC_QUAD);
		}
	}/*}}}*/
	if(src & (1<<IRQ_CRC_CH0))
	{/*{{{*/
		//printf("Diagnosis -> crc_freeze(ch0):");
		if(polarity & (1<<IRQ_CRC_CH0))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_CRC_CH0);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_CRC_CH0);
		}
	}/*}}}*/
	if(src & (1<<IRQ_CRC_CH1))
	{/*{{{*/
		//printf("Diagnosis -> crc_freeze(ch1):");
		if(polarity & (1<<IRQ_CRC_CH1))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_CRC_CH1);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_CRC_CH1);
		}
	}/*}}}*/
	if(src & (1<<IRQ_CRC_CH2))
	{/*{{{*/
		//printf("Diagnosis -> crc_freeze(ch2):");
		if(polarity & (1<<IRQ_CRC_CH2))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_CRC_CH2);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_CRC_CH2);
		}
	}/*}}}*/
	if(src & (1<<IRQ_CRC_CH3))
	{/*{{{*/
		//printf("Diagnosis -> crc_freeze(ch3):");
		if(polarity & (1<<IRQ_CRC_CH3))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_CRC_CH3);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_CRC_CH3);
		}
	}/*}}}*/
	if(src & (1<<IRQ_CRC_CH4))
	{/*{{{*/
		//printf("Diagnosis -> crc_freeze(ch4):");
		if(polarity & (1<<IRQ_CRC_CH4))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_CRC_CH4);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_CRC_CH4);
		}
	}/*}}}*/
	if(src & (1<<IRQ_GEN_VREFLOSS))
	{/*{{{*/
		//printf("Diagnosis -> gen vrefloss:");
		if(polarity & (1<<IRQ_GEN_VREFLOSS))
		{
			//printf("sync reference\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_GEN_VREFLOSS);
		}
		else
		{
			//printf("loss reference\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_GEN_VREFLOSS);
		}
	}/*}}}*/
	if(src & (1<<IRQ_GEN_SYNC))
	{/*{{{*/
		//printf("Diagnosis -> gen sync:");
		if(polarity & (1<<IRQ_GEN_SYNC))
		{
			//printf("unlock\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_GEN_SYNC);
		}
		else
		{
			//printf("lock\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_GEN_SYNC);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_FMT_CH0))
	{/*{{{*/
        bFlagInvalidCamStatus = TRUE;

		//printf("Diagnosis -> loss fmt(ch0):");
		if(polarity & (1<<IRQ_LOSS_FMT_CH0))
		{
			//printf("get fmt\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_FMT_CH0);
		}
		else
		{
			//printf("loss fmt\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_FMT_CH0);
        }
    }/*}}}*/
	if(src & (1<<IRQ_LOSS_FMT_CH1))
	{/*{{{*/
        bFlagInvalidCamStatus = TRUE;

		//printf("Diagnosis -> loss fmt(ch1):");
		if(polarity & (1<<IRQ_LOSS_FMT_CH1))
		{
			//printf("get fmt\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_FMT_CH1);
		}
		else
		{
			//printf("loss fmt\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_FMT_CH1);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_FMT_CH2))
	{/*{{{*/
        bFlagInvalidCamStatus = TRUE;

		//printf("Diagnosis -> loss fmt(ch2):");
		if(polarity & (1<<IRQ_LOSS_FMT_CH2))
		{
			//printf("get fmt\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_FMT_CH2);
		}
		else
		{
			//printf("loss fmt\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_FMT_CH2);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_FMT_CH3))
	{/*{{{*/
        bFlagInvalidCamStatus = TRUE;

		//printf("Diagnosis -> loss fmt(ch3):");
		if(polarity & (1<<IRQ_LOSS_FMT_CH3))
		{
			//printf("get fmt\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_FMT_CH3);
		}
		else
		{
			//printf("loss fmt\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_FMT_CH3);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_FMT_CH4))
	{/*{{{*/
		//printf("Diagnosis -> loss fmt(ch4):");
		if(polarity & (1<<IRQ_LOSS_FMT_CH4))
		{
			//printf("get fmt\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_FMT_CH4);
		}
		else
		{
			//printf("loss fmt\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_FMT_CH4);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_SYNC_CH0))
	{/*{{{*/
		//printf("Diagnosis -> loss sync(ch0):");
		if(polarity & (1<<IRQ_LOSS_SYNC_CH0))
		{
			//printf("sync\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_SYNC_CH0);
		}
		else
		{
			//printf("loss\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_SYNC_CH0);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_SYNC_CH1))
	{/*{{{*/
		//printf("Diagnosis -> loss sync(ch1):");
		if(polarity & (1<<IRQ_LOSS_SYNC_CH1))
		{
			//printf("sync\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_SYNC_CH1);
		}
		else
		{
			//printf("loss\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_SYNC_CH1);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_SYNC_CH2))
	{/*{{{*/
		//printf("Diagnosis -> loss sync(ch2):");
		if(polarity & (1<<IRQ_LOSS_SYNC_CH2))
		{
			//printf("sync\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_SYNC_CH2);
		}
		else
		{
			//printf("loss\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_SYNC_CH2);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_SYNC_CH3))
	{/*{{{*/
		//printf("Diagnosis -> loss sync(ch3):");
		if(polarity & (1<<IRQ_LOSS_SYNC_CH3))
		{
			//printf("sync\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_SYNC_CH3);
		}
		else
		{
			//printf("loss\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_SYNC_CH3);
		}
	}/*}}}*/
	if(src & (1<<IRQ_LOSS_SYNC_CH4))
	{/*{{{*/
		//printf("Diagnosis -> loss sync(ch4):");
		if(polarity & (1<<IRQ_LOSS_SYNC_CH4))
		{
			//printf("sync\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_LOSS_SYNC_CH4);
		}
		else
		{
			//printf("loss\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_LOSS_SYNC_CH4);
		}
	}/*}}}*/
	if(src & (1<<IRQ_FREEZE_DU_CH0))
	{/*{{{*/
        bFlagDisplayFreezeStatus = TRUE;

		//printf("Diagnosis -> freeze du(ch0):");
		if(polarity & (1<<IRQ_FREEZE_DU_CH0))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_FREEZE_DU_CH0);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_FREEZE_DU_CH0);
		}
	}/*}}}*/
	if(src & (1<<IRQ_FREEZE_DU_CH1))
	{/*{{{*/
        bFlagDisplayFreezeStatus = TRUE;

		//printf("Diagnosis -> freeze du(ch1):");
		if(polarity & (1<<IRQ_FREEZE_DU_CH1))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_FREEZE_DU_CH1);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_FREEZE_DU_CH1);
		}
	}/*}}}*/
	if(src & (1<<IRQ_FREEZE_DU_CH2))
	{/*{{{*/
        bFlagDisplayFreezeStatus = TRUE;

		//printf("Diagnosis -> freeze du(ch2):");
		if(polarity & (1<<IRQ_FREEZE_DU_CH2))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_FREEZE_DU_CH2);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_FREEZE_DU_CH2);
		}
	}/*}}}*/
	if(src & (1<<IRQ_FREEZE_DU_CH3))
	{/*{{{*/
        bFlagDisplayFreezeStatus = TRUE;

		//printf("Diagnosis -> freeze du(ch3):");
		if(polarity & (1<<IRQ_FREEZE_DU_CH3))
		{
			//printf("unfreeze\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_FREEZE_DU_CH3);
		}
		else
		{
			//printf("freeze\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_FREEZE_DU_CH3);
		}
	}/*}}}*/
	if(src & (1<<IRQ_GADC0_DIAG))
	{/*{{{*/
		//printf("Diagnosis -> gadc0:");
		if(polarity & (1<<IRQ_GADC0_DIAG))
		{
			//printf("normal\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_GADC0_DIAG);
		}
		else
		{
			//printf("below threshold\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_GADC0_DIAG);
		}
	}/*}}}*/
	if(src & (1<<IRQ_GADC1_DIAG))
	{/*{{{*/
		//printf("Diagnosis -> gadc1:");
		if(polarity & (1<<IRQ_GADC1_DIAG))
		{
			//printf("normal\n");
			//printf("low,falling\n");
			polarity &= ~(1<<IRQ_GADC1_DIAG);
		}
		else
		{
			//printf("above threshold\n");
			//printf("high,rising\n");
			polarity |= (1<<IRQ_GADC1_DIAG);
		}
	}/*}}}*/

	gpSUBINTC[ctlNum]->polarity = polarity;    // 1:falling/low, 0:rising/high


    if(bFlagDisplayFreezeStatus)
    {
        for(i = 0; i < 4; i++)
        {
            if(polarity & (1<<(IRQ_FREEZE_DU_CH0+i))) { u8DisplayFreezeStatus[i] = 1; /*freeze*/ }
            else { u8DisplayFreezeStatus[i] = 0; /*unfreeze*/ }
        }

        AppTask_SendCmdFromISR(CMD_EMERGENCY_DISPLAY_FREEZE, (PP_U16)-1, TASK_EMERGENCY, 0, u8DisplayFreezeStatus, sizeof(unsigned char)*4);
    }

    if(bFlagInvalidCamStatus)
    {
        for(i = 0; i < 4; i++)
        {
            if(polarity & (1<<(IRQ_LOSS_FMT_CH0+i))) { u8InvalidCamStatus[i] = 1; /*invalid*/ }
            else { u8InvalidCamStatus[i] = 0; /*valid*/ }
        }

#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
        AppTask_SendCmdFromISR(CMD_EMERGENCY_INVALID_CAMERA, (PP_U16)-1, TASK_EMERGENCY, 0, u8InvalidCamStatus, sizeof(unsigned char)*4);
#endif //(VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
    }

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

#if 0
ISR(sub_irq2_isr, num)
{
	const int ctlNum = 2;
	uint32 msk = (1 << num);
	uint32 src;
	int chanAddr;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2); // clear int pending

	gu32SubIntcCnt[2]++;

	src = get_sub_intr_src (ctlNum); 
	clr_sub_intr_src (ctlNum, src);
	//printf("%s: src = 0x%x\n", __FUNCTION__, src);

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}
#endif

/* register vsync call back function. */
typedef struct {
	const char *cmd;
	int (*fh)(int argc, const char **argv);
}callback_struct;
static callback_struct stSubIrq3Callbacks[IRQ_VSYNC_DU+1] =
{       
	/*VSYNC_QUAD*/	{ .cmd = "Vsync_Quad", 	.fh = NULL },
	/*VSYNC_VIN0*/	{ .cmd = "Vsync_Vin0", 	.fh = NULL },
	/*VSYNC_VIN1*/	{ .cmd = "Vsync_Vin1", 	.fh = NULL },
	/*VSYNC_VIN2*/	{ .cmd = "Vsync_Vin2", 	.fh = NULL },
	/*VSYNC_VIN3*/	{ .cmd = "Vsync_Vin3", 	.fh = NULL },
	/*VSYNC_VIN4*/	{ .cmd = "Vsync_Vin4", 	.fh = NULL },
	/*VSYNC_SVM*/	{ .cmd = "Vsync_SVM", 	.fh = PPDRV_SVM_ISR_VsynCB },
	/*VSYNC_DU*/	{ .cmd = "Vsync_DU", 	.fh = NULL },
};

ISR(sub_irq3_isr, num)
{
	const int ctlNum = 3;
	uint32 msk = (1 << num);
	uint32 src;
	int i;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2); // clear int pending

	gu32SubIntcCnt[3]++;

	src = get_sub_intr_src (ctlNum); 
	clr_sub_intr_src (ctlNum, src);
	//printf("%s: src = 0x%x\n", __FUNCTION__, src);

	for(i = IRQ_VSYNC_QUAD; i <= IRQ_VSYNC_DU; i++)
	{
		if(src & (1<<i))
		{
			gu32VSYNCIntcCnt[i]++;

			/* callback function of vsync. */
			if(stSubIrq3Callbacks[i].fh)
			{
				//printf("Vsync Callback:%s\n", stSubIrq3Callbacks[i].cmd);
				stSubIrq3Callbacks[i].fh(0, NULL);
			}
		}
	}


	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//PROC irq
PP_RESULT_E VSYNC_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stVSYNC_PROC_irqs[] = {
	    { .fn = VSYNC_PROC_IRQ,	.irqNum = 400/*IRQ_24_VECTOR*/,   .next = (void*)0, },
};

const char *strVsyncName[IRQ_VSYNC_DU+1] = { 
        "QUAD",
        "VIN0",
        "VIN1",
        "VIN2",
        "VIN3",
        "VIN4",
        "SVM",
        "DU",
};

PP_RESULT_E VSYNC_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
        int i, irqNum;
        char vsyncName[20];

        //      PRINT_PROC_IRQ(NAME, NUM, CNT)
        for(i = IRQ_VSYNC_QUAD; i <= IRQ_VSYNC_DU; i++)
        {
                irqNum = 400/*IRQ_24_VECTOR*/+i;

                sprintf(vsyncName, "VSYNC_%s", strVsyncName[i]);
                PRINT_PROC_IRQ(vsyncName, irqNum, gu32VSYNCIntcCnt[i]);
        }

        return(eSUCCESS);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SUB_INTR_Initialize(void)
{
    /* initialize sub_intc controller */
    {
        LOG_DEBUG("initialize sub intc0~3 \n");
        initSUBIntr(0); //PVI Rx
        initSUBIntr(1); //Diagnosis
        //initSUBIntr(2); //ISP ---> don't do. controll by CORE1
        initSUBIntr(3); //VSYNC
    }
    /* register SUB_INTC(0) */
    {
        isr_t *dummy = NULL_PTR;
        //printf("register sub intc0(PVIRX) \n");
        OSAL_register_isr(IRQ_0_VECTOR, sub_irq0_isr, dummy);
        INTC_irq_clean(IRQ_0_VECTOR);
        INTC_irq_config(IRQ_0_VECTOR, IRQ_LEVEL_TRIGGER);
        INTC_irq_enable(IRQ_0_VECTOR);
        //printf("register sub intc1(DIAGNOSIS) \n");
        OSAL_register_isr(IRQ_1_VECTOR, sub_irq1_isr, dummy);
        INTC_irq_clean(IRQ_1_VECTOR);
        INTC_irq_config(IRQ_1_VECTOR, IRQ_LEVEL_TRIGGER);
        INTC_irq_enable(IRQ_1_VECTOR);
#if 0 //ISP ---> don't do. controll by CORE1
        printf("register sub intc2(ISP) \n");
        OSAL_register_isr(IRQ_2_VECTOR, sub_irq2_isr, dummy);
        INTC_irq_clean(IRQ_2_VECTOR);
        INTC_irq_config(IRQ_2_VECTOR, IRQ_LEVEL_TRIGGER);
        INTC_irq_enable(IRQ_2_VECTOR);
#endif
        //printf("register sub intc3(VSYNC) \n");
        OSAL_register_isr(IRQ_24_VECTOR, sub_irq3_isr, dummy);
        INTC_irq_clean(IRQ_24_VECTOR);
        INTC_irq_config(IRQ_24_VECTOR, IRQ_LEVEL_TRIGGER);
        INTC_irq_enable(IRQ_24_VECTOR);

        //register proc infomation.
        SYS_PROC_addIrq(stIRQSUB0_PROC_irqs);
        SYS_PROC_addIrq(stIRQSUB1_PROC_irqs);
        //SYS_PROC_addIrq(stIRQSUB2_PROC_irqs); //ISP ---> don't do. controll by CORE1
        SYS_PROC_addIrq(stIRQSUB3_PROC_irqs);

        SYS_PROC_addDevice(stIRQSUBx_PROC_devices);
    }
    /* enable SUB_INTC(0) */
    {
        //printf("enable sub intc0(PVIRX) \n");
        sub_intc_enable(0);
        //printf("enable sub intc1(DIAGNOSIS) \n");
        sub_intc_enable(1);
#if 0 //ISP ---> don't do. controll by CORE1
        printf("enable sub intc2(ISP) \n");
        sub_intc_enable(2);
#endif
        //printf("enable sub intc3(VSYNC) \n");
        sub_intc_enable(3);
    }

    //register proc infomation.
    SYS_PROC_addIrq(stVSYNC_PROC_irqs);

}

PP_VOID PPDRV_VSYNC_GetCurrentCnt(PP_U32 *pu32VsyncCnt)
{
    int i;

    for(i = IRQ_VSYNC_QUAD; i <= IRQ_VSYNC_DU; i++)
    {
        if(pu32VsyncCnt)
        {
            *pu32VsyncCnt = gu32VSYNCIntcCnt[i];
            pu32VsyncCnt++;
        }
    }
}
