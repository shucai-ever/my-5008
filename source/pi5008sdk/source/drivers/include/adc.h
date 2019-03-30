/*
 * adc.h
 *
 *  Created on: 2017. 4. 13.
 *      Author: ihkong
 */

#ifndef _PI5008_ADC_H_
#define _PI5008_ADC_H_

#include "system.h"
#include "type.h"
#include "error.h"

#define ADC_MAX_CH		2

#define ADC_CH0			(1<<0)
#define ADC_CH1			(1<<1)

typedef struct tagADC_REG_T
{
	vuint32	ADCC_DATA[ADC_MAX_CH]; 		// 0x000 - ADC data register
	vuint32 ADCC_DIAG0;					// 0x018 - Diagnosis control 0
	vuint32 ADCC_DIAG1;					// 0x01C - Diagnosis control 1
	vuint32 ADCC_CH;					// 0x020 - ADC channel register - [7:0] bitwise
	vuint32 rsv1[3];					// 0x024~0x02C
	vuint32 ADCC_READY;					// 0x030 - ADC ready register - [7:0] bitwise
	vuint32 ADCC_PD;					// 0x034 - ADC power down register
	vuint32 rsv2[2];					// 0x038~0x03C
	vuint32 ADCC_EN;					// 0x040 - ADC enable - [8]: ready clear, [0]: enable
	vuint32 rsv3;						// 0x044
	vuint32 ADCC_IRQ_EN;				// 0x048
	vuint32 ADCC_IRQ_ST;				// 0x04C
	vuint32 rsv4[4];					// 0x050~0x0x5C
	vuint32 ADCC_CFG;					// 0x060 - ADC analog configure - [10:8]: adc range, [7:0]: adc offset
	vuint32 ADCC_STORE;					// 0x064 - ADC Analog store pulse control - [25:16]: stop store, [9:0}: start store
	vuint32 ADCC_SAMPLE;				// 0x068 - ADC analog sample pulse control - [25:16]: stop sample, [9:0}: start sample
	vuint32 ADCC_LOAD;					// 0x06C - ADC analog load pulse control - [25:16]: stop load, [9:0}: start load
	vuint32 ADCC_RAMP0;					// 0x070 - ADC analog ramp0 pulse control - [25:16]: vramp1 margin, [9:0}: vramp0 margin
	vuint32 ADCC_RAMP1;					// 0x074 - ADC analog ramp0 pulse control - [25:16]: vramp3 margin, [9:0}: vramp2 margin
	vuint32 ADCC_PERIOD;				// 0x078 - ADC analog period control - [9:0]: period count
	vuint32 ADCC_FILTER;				// 0x07C - ADC analog filter control - [1:0]: filter value;

}ADC_REG_T;

#define ADC_CTRL_REG			((ADC_REG_T*)			(ADC_BASE_ADDR))

typedef PP_VOID (*ADC_ISR_CALLBACK) (PP_U32 u32Status);

#if 0
void adc_initialize(ADC_CFG_T *cfg, ADC_CALLBACK callback);
void adc_deinitialize(void);
void adc_start(sint32 ch_bits, uint8 block);
uint32 adc_get_data(sint32 ch);
void adc_irq_enable(uint32 enable);
uint32 adc_get_irq_status(void);
#else
PP_VOID PPDRV_ADC_Initialize(PP_VOID);
PP_VOID PPDRV_ADC_Start(PP_U32 IN u32ChBits, PP_U8 IN u8Block);
PP_U32 PPDRV_ADC_GetReady(PP_VOID);
PP_U32 PPDRV_ADC_GetData(PP_S32 IN s32Ch);
#if 0
PP_VOID PPDRV_ADC_SetISR(ADC_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_ADC_IRQEnable(PP_U32 IN u32Enable);
PP_U32 PPDRV_ADC_GetIRQStatus(PP_VOID);
#endif
#endif
// diagnosis ?????

#endif
