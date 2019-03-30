/*
 * intc.c
 *
 *  Created on: 2017. 3. 31.
 *      Author: ibkim
 */

/*---------------------------------------------------------------------------*/
/* Includes */
/*---------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "pi5008.h"
#include "type.h"
#include "error.h"
#include "system.h"
#include "cache.h"
#include "uart_drv.h"
#include "dma.h"
#include "debug.h"
#include "du_drv.h"
#include "debug.h"
#include "osal.h"
#include "utils.h"
#include "interrupt.h"

void INTC_initialize(void)
{
    return;
}

void INTC_swi_enable(void)
{
    SR_SETB32(NDS32_SR_INT_MASK2, IRQ_SWI_VECTOR);

    return;
}

void INTC_swi_disable(void)
{
    SR_CLRB32(NDS32_SR_INT_MASK2,IRQ_SWI_VECTOR);

    return;
}

void INTC_swi_clean(void)
{
    SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);

    return;
}

void INTC_swi_trigger(void)
{
    SR_SETB32(NDS32_SR_INT_PEND,INT_PEND_offSWI);

    return;
}

uint32 INTC_irq_mask(sint32 IN vecId)
{
    uint32 prv_msk = __nds32__mfsr(NDS32_SR_INT_MASK2);

    if (vecId == -1)
    {
        __nds32__mtsr(0, NDS32_SR_INT_MASK2);
    }
    else if (vecId < 32)
    {
        SR_CLRB32(NDS32_SR_INT_MASK2, vecId);
    }
    else
    {
        LOG_CRITICAL("vecId:%d, is invalid!\r\n", (int)vecId);
        prv_msk = 0xFFFFFFFFU;
    }

    return prv_msk;
}

void INTC_irq_unmask(uint32 IN mask)
{
    __nds32__mtsr(mask , NDS32_SR_INT_MASK2);

    return;
}

void INTC_irq_clean(sint32 IN vecId)
{
    if (vecId == IRQ_SWI_VECTOR)
    {
        SR_CLRB32(NDS32_SR_INT_PEND, INT_PEND_offSWI);
    }
    else
    {
        /* PEND2 is W1C */
        SR_SETB32(NDS32_SR_INT_PEND2, vecId);
    }

    return;
}

void INTC_irq_clean_all(void)
{
    __nds32__mtsr(-1,NDS32_SR_INT_PEND2);

    return;
}

void INTC_irq_disable(uint32 IN vecId)
{
    SR_CLRB32(NDS32_SR_INT_MASK2,vecId);

    return;
}

void INTC_irq_disable_all(void)
{
    __nds32__mtsr(0x0,NDS32_SR_INT_MASK2);

    return;
}

void INTC_irq_enable(sint32 IN vecId)
{
    SR_SETB32(NDS32_SR_INT_MASK2,vecId);

    return;
}

void INTC_irq_set_priority(uint32 IN prio1, uint32 IN prio2)
{
    __nds32__mtsr(prio1, NDS32_SR_INT_PRI);
    __nds32__mtsr(prio2, NDS32_SR_INT_PRI2);

    return;
}

void INTC_irq_config(sint32 IN vecId, uint32 IN edge)
{
	//uint8 pos = vecId % 8;
	//vuint32 reg = MISC_BASE_ADDR + 0x100 + ((vecId / 8) * 4);
	//vuint32 reg = MISC_BASE_ADDR + 0x148 + ((vecId / 8) * 4);

	uint8 pos = vecId;
	vuint32 reg = (MISC_BASE_ADDR + 0x78);

	SetRegValue(reg, utilPutBit(GetRegValue(reg), pos, edge & utilBitMask(1)));
	
    return;
}

uint32 INTC_get_all_pend(void)
{
    return __nds32__mfsr(NDS32_SR_INT_PEND2);
}
