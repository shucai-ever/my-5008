/*
 * wdt.h
 *
 *  Created on: 2017. 3. 28.
 *      Author: ihkong
 */

#ifndef __WDT_H__
#define __WDT_H__

#include "type.h"
#include "system.h"


typedef enum ppWDT_TIMEOUT_E
{
	eWDT_TIMEOUT_POW2_6 = 0,
	eWDT_TIMEOUT_POW2_8,
	eWDT_TIMEOUT_POW2_10,
	eWDT_TIMEOUT_POW2_11,
	eWDT_TIMEOUT_POW2_12,
	eWDT_TIMEOUT_POW2_13,
	eWDT_TIMEOUT_POW2_14,
	eWDT_TIMEOUT_POW2_15,
	eWDT_TIMEOUT_POW2_17,
	eWDT_TIMEOUT_POW2_19,
	eWDT_TIMEOUT_POW2_21,
	eWDT_TIMEOUT_POW2_23,
	eWDT_TIMEOUT_POW2_25,
	eWDT_TIMEOUT_POW2_27,
	eWDT_TIMEOUT_POW2_29,
	eWDT_TIMEOUT_POW2_31,
} PP_WDT_TIMEOUT_E;


typedef void (*WDT_ISR_CALLBACK) (void);

PP_VOID PPDRV_WDT_Initialize(PP_WDT_TIMEOUT_E IN enTimeOut);
PP_VOID PPDRV_WDT_SetISR(WDT_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_WDT_SetEnable(PP_U8 IN u8Enable);
PP_VOID PPDRV_WDT_KeepAlive(PP_VOID);


#endif
