/*
 * wdt.c
 *
 *  Created on: 2017. 3. 28.
 *      Author: ihkong
 */

#include "osal.h"
#include "wdt.h"
#include "pi5008.h"
#include "system.h"
#include "type.h"
//#include "sys_api.h"
#include "utils.h"
#include "interrupt.h"

#define WDT_MAGIC_WRITE_PROTECT		(0x5AA5)
#define WDT_MAGIC_RESTART_TIMER		(0xCAFE)

#define WDT_RESET_PERIOD_IDX_POWER2_7		0
#define WDT_RESET_PERIOD_IDX_POWER2_8		1
#define WDT_RESET_PERIOD_IDX_POWER2_9		2
#define WDT_RESET_PERIOD_IDX_POWER2_10		3
#define WDT_RESET_PERIOD_IDX_POWER2_11		4
#define WDT_RESET_PERIOD_IDX_POWER2_12		5
#define WDT_RESET_PERIOD_IDX_POWER2_13		6
#define WDT_RESET_PERIOD_IDX_POWER2_14		7

#define WDT_RESET_PERIOD 	WDT_RESET_PERIOD_IDX_POWER2_14

typedef struct tagWDT_REG_T
{
	vuint32 id;					/* 0x000 			- id */
	vuint32 rsv[3];				/* 0x004 ~ 0x00c	- reserved */
	vuint32 ctrl;				/* 0x010			- control register */
	vuint32 restart;			/* 0x014			- restart register */
	vuint32 write_en;			/* 0x018			- write enable register */
	vuint32 status;				/* 0x01c			- status register */

}WDT_REG_T;

#define WDT_CTRL_REG			((WDT_REG_T*)			(WDT_BASE_ADDR + 0x0000U))


static WDT_REG_T *gpWDT = WDT_CTRL_REG;
static WDT_ISR_CALLBACK gWDTCallback;

ISR(wdt_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);


	reg = gpWDT->status;

    //gpWDT->write_en = WDT_MAGIC_WRITE_PROTECT;
    reg = 1;
    gpWDT->status = reg;

	if(gWDTCallback)gWDTCallback();


	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}




PP_VOID PPDRV_WDT_Initialize(PP_WDT_TIMEOUT_E IN enTimeOut)
{
	sys_os_isr_t *old = NULL;
	uint32 irq_en;

	irq_en = 1;
	gWDTCallback = NULL;

	gpWDT->write_en = WDT_MAGIC_WRITE_PROTECT;
	gpWDT->ctrl = ( ((WDT_RESET_PERIOD & 0x7) << 8) |
					((enTimeOut & 0xf) << 4) |
					(1 << 3) |								// reset enable
					//(0 << 3) |							// reset disable
					((irq_en&1) << 2) |						// interrupt enable
					//(1 << 1) |								// clock source - pclk
					(0 << 1) |							// clock source - extclk
					(0 << 0) 								// enable wdt
					);

	OSAL_register_isr(IRQ_WDT_VECTOR, wdt_isr, old);
    INTC_irq_clean(IRQ_WDT_VECTOR);
    INTC_irq_enable(IRQ_WDT_VECTOR);

	//printf("int mask: 0x%x\n", __nds32__mfsr(NDS32_SR_INT_MASK2));
}

PP_VOID PPDRV_WDT_SetISR(WDT_ISR_CALLBACK IN cbISR)
{
		gWDTCallback = cbISR;

}

PP_VOID PPDRV_WDT_SetEnable(PP_U8 IN u8Enable)
{
	gpWDT->write_en = WDT_MAGIC_WRITE_PROTECT;
	if(u8Enable)
		gpWDT->ctrl |= 1;
	else
		gpWDT->ctrl &= (~1);

}

PP_VOID PPDRV_WDT_KeepAlive(PP_VOID)
{
	//vuint32 reg;

	gpWDT->write_en = WDT_MAGIC_WRITE_PROTECT;
	gpWDT->restart = WDT_MAGIC_RESTART_TIMER;

	//reg = gpWDT->status;
	//printf("keep alive. status: 0x%x\n", reg);
}

