/*
 * adc.c
 *
 *  Created on: 2017. 4. 13.
 *      Author: ihkong
 */

#include <stdio.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "type.h"
#include "utils.h"
#include "debug.h"
#include "pi5008.h"
#include "adc.h"


typedef struct tagADC_CFG_T
{
	uint32 adc_range;
	uint32 adc_offset;
	uint32 start_store;
	uint32 stop_store;
	uint32 start_sample;
	uint32 stop_sample;
	uint32 start_load;
	uint32 stop_load;
	uint32 ramp_margin[4];
	uint32 period_cnt;
	uint32 filter_value;
}ADC_CFG_T;


static ADC_REG_T *gpADC = ADC_CTRL_REG;
static ADC_ISR_CALLBACK gADCCallback = NULL;

//////////////////////////////////////////////////////////////////////////////
ISR(adc_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpADC->ADCC_IRQ_ST;
	if(gADCCallback)gADCCallback(reg);

	// clear pending reg ????

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);

}


//void adc_initialize(ADC_CFG_T *cfg, ADC_CALLBACK callback)
PP_VOID PPDRV_ADC_Initialize(PP_VOID)
{
	ADC_CFG_T cfg;

	cfg.adc_range = 0x5;
	cfg.adc_offset = 0x3;
	cfg.start_store = 0x5;
	cfg.stop_store = 0x82;
	cfg.start_sample = 0x0a;
	cfg.stop_sample = 0x7d;
	cfg.start_load = 0xcc;
	cfg.stop_load = 0x1e0;
	cfg.ramp_margin[0] = 0x87;
	cfg.ramp_margin[1] = 0xd6;
	cfg.ramp_margin[2] = 0x1d6;
	cfg.ramp_margin[3] = 0x1ea;
	cfg.period_cnt = 0x1f3;
	cfg.filter_value = 0x0;

	gpADC->ADCC_CFG = (((cfg.adc_range & 0x7)<<8) | (cfg.adc_offset & 0xff));
	gpADC->ADCC_STORE = (((cfg.stop_store & 0x3ff)<<16) | (cfg.start_store & 0x3ff));
	gpADC->ADCC_SAMPLE = (((cfg.stop_sample & 0x3ff)<<16) | (cfg.start_sample & 0x3ff));
	gpADC->ADCC_LOAD = (((cfg.stop_load & 0x3ff)<<16) | (cfg.start_load & 0x3ff));
	gpADC->ADCC_RAMP0 = (((cfg.ramp_margin[1] & 0x3ff)<<16) | (cfg.ramp_margin[0] & 0x3ff));
	gpADC->ADCC_RAMP1 = (((cfg.ramp_margin[3] & 0x3ff)<<16) | (cfg.ramp_margin[2] & 0x3ff));
	gpADC->ADCC_PERIOD = (cfg.period_cnt & 0x3ff);
	gpADC->ADCC_FILTER = (cfg.filter_value & 0x3);

	// power down clear
	gpADC->ADCC_PD = 0;

	gADCCallback = NULL;

	//OSAL_register_isr(IRQ_ADC_VECTOR, adc_isr, NULL);
	//INTC_irq_clean(IRQ_ADC_VECTOR);
	//INTC_irq_enable(IRQ_ADC_VECTOR);
}


//void adc_start(sint32 ch_bits, uint8 block)
PP_VOID PPDRV_ADC_Start(PP_U32 IN u32ChBits, PP_U8 IN u8Block)
{
	gpADC->ADCC_CH = u32ChBits;
	gpADC->ADCC_EN = (1<<8);						// ready clear
	gpADC->ADCC_EN = 1;

	if(u8Block)
		while(gpADC->ADCC_EN & 1);
}

//uint32 adc_get_ready(void)
PP_U32 PPDRV_ADC_GetReady(PP_VOID)
{
	return gpADC->ADCC_READY;
}

//uint32 adc_get_data(sint32 ch)
PP_U32 PPDRV_ADC_GetData(PP_S32 IN s32Ch)
{
	//LOG_DEBUG("ready: 0x%x\n", (unsigned int)gpADC->ADCC_READY);
	return gpADC->ADCC_DATA[s32Ch];
}


#if 0
PP_VOID PPDRV_ADC_SetISR(ADC_ISR_CALLBACK IN cbISR)
{
	gADCCallback = cbISR;
}

PP_VOID PPDRV_ADC_IRQEnable(PP_U32 IN u32Enable)
{
	gpADC->ADCC_IRQ_EN = (u32Enable & 1);
}

PP_U32 PPDRV_ADC_GetIRQStatus(PP_VOID)
{
	return (uint32)gpADC->ADCC_IRQ_ST;
}
#endif
