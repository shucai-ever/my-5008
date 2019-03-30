#ifndef _PI5008_TIMER_H
#define _PI5008_TIMER_H

#include "system.h"
#include "type.h"


#define TIMER_ID_REV        			(0x03031000)

typedef enum ppPIT_MODE_E
{
	ePIT_MODE_TIMER = 0,
	ePIT_MODE_PWM,
}PP_PIT_MODE_E;

typedef enum ppTIMER_DEV_E
{
    eTIMER_DEV_0 = 0U,
    eTIMER_DEV_1,
}PP_TIMER_DEV_E;

typedef enum ppTIMER_CH_E
{
    eTIMER_CH_0 = 0U,
    eTIMER_CH_1,
    eTIMER_CH_2,
    eTIMER_CH_3
}PP_TIMER_CH_E;

typedef struct ppTIMER_CFG_S {
	PP_PIT_MODE_E enTimerMode[4];   	// IN
    PP_U32 u32PWMInitVal;				// IN
    PP_U32 u32nTimerCh;      			// OUT
    PP_U32 u32nPWMCh;        			// OUT
} PP_TIMER_CFG_S;

typedef PP_VOID (*TIMER_ISR_CALLBACK) (PP_VOID);


PP_RESULT_E PPDRV_TIMER_Initialize(PP_U32 IN u32Dev, PP_TIMER_CFG_S IN *pstCfg);
PP_VOID PPDRV_TIMER_SetISR(PP_U32 IN u32Dev, PP_S32 IN s32Ch, TIMER_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_TIMER_SetPeriod(PP_U32 IN u32Dev, PP_S32 IN s32Ch, PP_U32 IN u32Period);
PP_VOID PPDRV_TIMER_IRQControl(PP_U32 IN u32Dev, PP_S32 IN s32Ch, PP_U32 IN u32Enable);
PP_VOID PPDRV_TIMER_Start(PP_U32 IN u32Dev, PP_S32 IN s32Ch);
PP_VOID PPDRV_TIMER_Stop(PP_U32 IN u32Dev, PP_S32 IN s32Ch);
PP_VOID PPDRV_TIMER_Reset(PP_U32 IN u32Dev);
PP_U32 PPDRV_TIMER_CountRead(PP_U32 IN u32Dev, PP_S32 IN s32Ch);
PP_U32 PPDRV_TIMER_IRQMask(PP_U32 IN u32Dev);
PP_VOID PPDRV_TIMER_IRQUnMask(PP_U32 IN u32Mask);
PP_VOID PPDRV_TIMER_IRQClear(PP_U32 IN u32Dev, PP_S32 IN s32Ch);


PP_VOID PPDRV_PWM_Start(PP_U32 IN u32Dev, PP_S32 IN s32Ch);
PP_VOID PPDRV_PWM_Stop(PP_U32 IN u32Dev, PP_S32 IN s32Ch);
PP_VOID PPDRV_PWM_SetPeriod(PP_U32 IN u32Dev, PP_S32 IN s32Ch, PP_U16 IN u16HighPeriod, PP_U16 IN u16LowPeriod);

PP_VOID PPDRV_TIMER_AddProc(PP_VOID);

#endif
