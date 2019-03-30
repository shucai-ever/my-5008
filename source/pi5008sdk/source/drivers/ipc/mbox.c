/*
 * mbox.c
 *
 *  Created on: 2017. 10. 17.
 *      Author: ihkong
 */

#include <stdio.h>

#include "osal.h"
#include "mbox.h"
#include "type.h"
#include "utils.h"
#include "debug.h"
#include "pi5008.h"
#include "interrupt.h"
#include "proc.h"

typedef struct tagMBOX_REG_T
{
	vuint32	data[8]; 			// 0x000~0x01C - Data register
	vuint32	rsv[24];			// 0x020~0x07C
	vuint32 ivb[2];				// 0x080~0x084 - Reset vector base register
	vuint32	rsv1[14];			// 0x088~0x0BC
	vuint32 raw_irq[4];			// 0x0C0~0x0CC - Raw irq status register
	vuint32	rsv2[12];			// 0x0D0~0x0FC
	vuint32 irq_msg[4];			// 0x100~0x10C - msg & irq register

} MBOX_REG_T;

#define MBOX_CTRL_REG			((MBOX_REG_T*)			(MBOX_BASE_ADDR))

static MBOX_REG_T *gpMBOX = MBOX_CTRL_REG;
static sint32 gCPUNum = 0;
static MBOX_ISR_CALLBACK gMBOX_ISR = NULL;

ISR(mbox_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpMBOX->irq_msg[gCPUNum];

	if(gMBOX_ISR){
		gMBOX_ISR(reg & (~1));
	}

	gpMBOX->irq_msg[gCPUNum] = 0;

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}



PP_VOID PPDRV_MBOX_Initialize(PP_S32 IN s32CoreNum)
{
	gCPUNum = s32CoreNum;
	gMBOX_ISR = NULL;

    OSAL_register_isr(IRQ_D10_VECTOR, mbox_isr, NULL);
    INTC_irq_config(IRQ_D10_VECTOR, IRQ_LEVEL_TRIGGER);
    INTC_irq_clean(IRQ_D10_VECTOR);
    INTC_irq_enable(IRQ_D10_VECTOR);

}

PP_VOID PPDRV_MBOX_SetISR(MBOX_ISR_CALLBACK IN cbISR)
{
	gMBOX_ISR = cbISR;
}

PP_VOID PPDRV_MBOX_SetData(PP_S32 IN s32Idx, PP_U32 IN u32Data)
{
	gpMBOX->data[s32Idx] = u32Data;
}

PP_U32 PPDRV_MBOX_GetData(PP_S32 IN s32Idx)
{
	return gpMBOX->data[s32Idx];

}

PP_VOID PPDRV_MBOX_SetIRQMsg(PP_S32 IN s32CoreNum, PP_U32 IN u32Msg)
{
	gpMBOX->irq_msg[s32CoreNum] = (u32Msg | 1);
}

//uint32 MBOX_get_irq_msg(sint32 cpu_num)
PP_U32 PPDRV_MBOX_GetIRQMsg(PP_S32 IN s32CoreNum)
{
	return (gpMBOX->irq_msg[s32CoreNum] & (~1));
}

