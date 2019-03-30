
#include <stdio.h>
#include <nds32_intrinsic.h>

#include "system.h"
#include "error.h"
#include "pi5008.h"
#include "timer.h"
#include "debug.h"
#include "utils.h"
#include "osal.h"

#include "interrupt.h"
#include "proc.h"

#define PIT_MODE_NONE                   0
#define PIT_MODE_TIMER32				1
#define PIT_MODE_TIMER16				2
#define PIT_MODE_TIMER8					3
#define PIT_MODE_PWM					4
#define PIT_MODE_PWM_TIMER16_MIX		6
#define PIT_MODE_PWM_TIMER8_MIX			7

/* Interrupt Status Register */
/* Clean Timer interrupt pending bit, write 1 clean */
#define PIT_C0_TMR0_PEND_W1C		0x1
#define PIT_C0_TMR1_PEND_W1C		0x2
#define PIT_C0_TMR2_PEND_W1C		0x4
#define PIT_C0_TMR3_PEND_W1C		0x8

#define PIT_C1_TMR0_PEND_W1C		0x10
#define PIT_C1_TMR1_PEND_W1C		0x20
#define PIT_C1_TMR2_PEND_W1C		0x40
#define PIT_C1_TMR3_PEND_W1C		0x80

#define PIT_C2_TMR0_PEND_W1C		0x100
#define PIT_C2_TMR1_PEND_W1C		0x200
#define PIT_C2_TMR2_PEND_W1C		0x400
#define PIT_C2_TMR3_PEND_W1C		0x800

#define PIT_C3_TMR0_PEND_W1C		0x1000
#define PIT_C3_TMR1_PEND_W1C		0x2000
#define PIT_C3_TMR2_PEND_W1C		0x4000
#define PIT_C3_TMR3_PEND_W1C		0x8000

/* channel 0~3 control register */
/* ChClk*/
#define PIT_CH_CTL_APBCLK		0x8 
/* ChMode*/
#define PIT_CH_CTL_TMR32		0x1
#define PIT_CH_CTL_TMR16		0x2
#define PIT_CH_CTL_TMR8			0x3
#define PIT_CH_CTL_PWM			0x4
#define PIT_CH_CTL_MIX16		0x6
#define PIT_CH_CTL_MIX8			0x7

#define MAX_PIT_DEV_NUM		2
#define MAX_TIMER_NUM		16
#define MAX_PWM_NUM			4


typedef struct ppTIMER_CH_REG_S
{
	vuint32 ctrl;							/* 0x020 		- Channel control register */
	vuint32 reload;							/* 0x024 		- Channel reload register */
	vuint32 counter;						/* 0x028 		- Channel counter register */
	vuint32 rsv;							/* 0x02c 		- Reserved */
}PP_TIMER_CH_REG_S;

typedef struct ppTIMER_REG_S
{
	vuint32 id;								/* 0x000 		- ID and revision register */
	vuint32 rsv0[3];						/* 0x004 ~ 0x00c	- reserved */
	vuint32 cfg;							/* 0x010 		- Configuration register */
	vuint32 irq_enable;						/* 0x014 		- Interrupt enable register */
	vuint32 irq_status;						/* 0x018 		- Interrupt status register */
	vuint32 ch_enable;						/* 0x01c 		- Channel enable register */

	PP_TIMER_CH_REG_S ch_reg[4];				/* 0x020 ~ 0x05c	- Channel register */
}PP_TIMER_REG_T;


#define TIMER0_CTRL_REG			((PP_TIMER_REG_T*)			(TIMER0_BASE_ADDR))
#define TIMER1_CTRL_REG			((PP_TIMER_REG_T*)			(TIMER1_BASE_ADDR))

typedef struct ppPIT_CHANNEL_INFO_S
{
    uint32 idx;             // timer ch
    uint32 sub_idx;         // timer sub ch
    uint32 bit_pos;         // bit position of reload value for timer or pwm low period
    uint32 nbits;           // reload value bits
    TIMER_ISR_CALLBACK TIMERCallback;
}PP_PIT_CHANNEL_INFO_S;


static PP_TIMER_REG_T *gpTimer[] = {
    TIMER0_CTRL_REG,
    TIMER1_CTRL_REG,
};

typedef struct ppTIMER_CONTEXT_S
{
	PP_PIT_CHANNEL_INFO_S TimerCh[MAX_TIMER_NUM];
	PP_PIT_CHANNEL_INFO_S PWMCh[MAX_PWM_NUM];
    uint32 nTimer;
    uint32 nPWM;
}PP_TIMER_CONTEXT_S;

STATIC PP_TIMER_CONTEXT_S gCTX[MAX_PIT_DEV_NUM];

STATIC CONST uint8 gTimerIrqVector[MAX_PIT_DEV_NUM] =
{
	IRQ_TIMER0_VECTOR,
    IRQ_TIMER1_VECTOR,
};

ISR(timer_isr0, num);
ISR(timer_isr1, num);

STATIC void *gTimerISR[MAX_PIT_DEV_NUM] = {
		timer_isr0,
		timer_isr1
};

static uint32 gu32TIMERDevInitFlag = 0;	// bit0: dev0, bit1: dev1
static uint32 gu32TIMERIntcCnt[MAX_PIT_DEV_NUM][MAX_TIMER_NUM];

//PROC irq
PP_RESULT_E TIMER0_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stTIMER0_PROC_irqs[] = {
	    { .fn = TIMER0_PROC_IRQ, .irqNum = IRQ_TIMER0_VECTOR,   .next = (void*)0 },
};
PP_RESULT_E TIMER1_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv);
struct proc_irq_struct stTIMER1_PROC_irqs[] = {
	    { .fn = TIMER1_PROC_IRQ, .irqNum = IRQ_TIMER1_VECTOR,   .next = (void*)0 },
};

//PROC device
PP_RESULT_E TIMER_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv);
struct proc_device_struct stTIMER_PROC_devices[] = {
	    { .pName = "TIMER",  .fn = TIMER_PROC_DEVICE,    .next = (void*)0 },
};

STATIC sint32   timer_init(uint8 dev, PP_TIMER_CFG_S *cfg);
STATIC void 	timer_set_callback(uint32 dev, uint32 ch, TIMER_ISR_CALLBACK timer_callback);
STATIC uint32   timer_get_id_revision (uint8 dev);
STATIC uint32   timer_get_irq_status(uint32 dev);
STATIC void     timer_set_irq_status(uint32 dev, uint32 status);
STATIC void     timer_reset (uint32 dev);
STATIC void     timer_start (uint32 dev, uint32 ch);
STATIC void     timer_stop (uint32 dev, uint32 ch);
STATIC void     timer_irq_enable (uint32 dev, uint32 ch, uint32 enable);
STATIC void     timer_set_period(uint8 dev, uint32 ch, uint32 period);
STATIC uint32   timer_get_cnt(uint8 dev, uint32 ch);
STATIC uint32   timer_get_period(uint8 dev, uint32 ch);

STATIC void		pwm_start( uint32 dev, uint32 ch);
STATIC void		pwm_stop( uint32 dev, uint32 ch);
STATIC void     pwm_set_period(uint8 dev, uint32 ch, uint16 high_period, uint16 low_period);


PP_RESULT_E PPDRV_TIMER_Initialize(PP_U32 IN u32Dev, PP_TIMER_CFG_S IN *pstCfg)
{
    uint32 i;
    PP_U32 u32VecId;

    u32VecId = IRQ_TIMER0_VECTOR + u32Dev;

	if ( timer_get_id_revision(u32Dev) != TIMER_ID_REV )
    {
        return eERROR_FAILURE;
    }

    if ( timer_init(u32Dev, (PP_TIMER_CFG_S*)pstCfg) != 1 )
    {
        return eERROR_FAILURE;
    }

   	OS_CPU_Vector_Table[u32VecId] = gTimerISR[u32Dev];


   	for(i=0;i<MAX_TIMER_NUM;i++){
   		gu32TIMERIntcCnt[u32Dev][i] = 0;

   	}

    /*
         * tick ISR init
         */
    /* init trigger mode */
    /* Set edge trigger, falling edge */
    INTC_irq_config(u32VecId, IRQ_LEVEL_TRIGGER); //Must set Level
    /* clean pending */
    INTC_irq_clean(u32VecId);
    /* enable timer interrupt */
    INTC_irq_enable(u32VecId);

    gu32TIMERDevInitFlag |= (1<<u32Dev);

    return eSUCCESS;
}





PP_VOID PPDRV_TIMER_SetISR(PP_U32 IN u32Dev, PP_S32 IN s32Ch, TIMER_ISR_CALLBACK IN cbISR)
{
	timer_set_callback(u32Dev, s32Ch, cbISR);
}

PP_U32 PPDRV_TIMER_IRQMask(PP_U32 IN u32Dev)
{
    return INTC_irq_mask(gTimerIrqVector[u32Dev]);
}

PP_VOID PPDRV_TIMER_IRQUnMask(PP_U32 IN u32Mask)
{
    INTC_irq_unmask(u32Mask);

    return;
}

PP_VOID PPDRV_TIMER_IRQClear(PP_U32 IN u32Dev, PP_S32 IN s32Ch)
{
    timer_set_irq_status(u32Dev, 0x1 << (5 * s32Ch));
    INTC_irq_clean(gTimerIrqVector[u32Dev]);

    return;
}

PP_VOID PPDRV_TIMER_SetPeriod(PP_U32 IN u32Dev, PP_S32 IN s32Ch, PP_U32 IN u32Period)
{
    timer_set_period(u32Dev, s32Ch, u32Period);

    return;
}



PP_VOID PPDRV_TIMER_IRQControl(PP_U32 IN u32Dev, PP_S32 IN s32Ch, PP_U32 IN u32Enable)
{
    timer_irq_enable(u32Dev, s32Ch, u32Enable);

    return;
}

PP_VOID PPDRV_TIMER_Start(PP_U32 IN u32Dev, PP_S32 IN s32Ch)
{
    timer_start(u32Dev, s32Ch);

    return;
}


PP_VOID PPDRV_TIMER_Stop(PP_U32 IN u32Dev, PP_S32 IN s32Ch)
{
    timer_stop(u32Dev, s32Ch);

    return;
}

PP_VOID PPDRV_TIMER_Reset(PP_U32 IN u32Dev)
{
    timer_reset(u32Dev);

    return;
}

PP_U32 PPDRV_TIMER_CountRead(PP_U32 IN u32Dev, PP_S32 IN s32Ch)
{
    return timer_get_cnt(u32Dev, s32Ch);
}

PP_U32 PPDRV_TIMER_IRQStatus(PP_U32 IN u32Dev)
{
    return timer_get_irq_status(u32Dev);
}

PP_VOID PPDRV_PWM_Start(PP_U32 IN u32Dev, PP_S32 IN s32Ch)
{
    pwm_start(u32Dev, s32Ch);

    return;
}


PP_VOID PPDRV_PWM_Stop(PP_U32 IN u32Dev, PP_S32 IN s32Ch)
{
    pwm_stop(u32Dev, s32Ch);

    return;
}

PP_VOID PPDRV_PWM_SetPeriod(PP_U32 IN u32Dev, PP_S32 IN s32Ch, PP_U16 IN u16HighPeriod, PP_U16 IN u16LowPeriod)
{
    pwm_set_period(u32Dev, s32Ch, u16HighPeriod, u16LowPeriod);

	return;
}

PP_VOID PPDRV_TIMER_AddProc(PP_VOID)
{
    //register proc infomation.
	SYS_PROC_addIrq(stTIMER0_PROC_irqs);
	SYS_PROC_addIrq(stTIMER1_PROC_irqs);
	SYS_PROC_addDevice(stTIMER_PROC_devices);

}

STATIC void show_timer_info(uint8 dev)
{
	sint32 i;

	LOG_DEBUG("dev: %d, timer ch: %d, pwm ch: %d\n", dev, gCTX[dev].nTimer, gCTX[dev].nPWM);
	for(i=0;i<gCTX[dev].nTimer;i++){
		LOG_DEBUG("[dev: %d, ch%d]Timer idx: %d, sub: %d, bits: %d\n", dev, i, gCTX[dev].TimerCh[i].idx, gCTX[dev].TimerCh[i].sub_idx, gCTX[dev].TimerCh[i].nbits);
	}


	for(i=0;i<gCTX[dev].nPWM;i++){
		LOG_DEBUG("[dev: %d, ch%d]PWM idx: %d, sub: %d, bits: %d\n", dev, i, gCTX[dev].PWMCh[i].idx, gCTX[dev].PWMCh[i].sub_idx, gCTX[dev].PWMCh[i].nbits);
	}
}

ISR(timer_isr0, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	uint32 i;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpTimer[eTIMER_DEV_0]->irq_status;

	for(i=0;i<gCTX[eTIMER_DEV_0].nTimer;i++){
		if(reg & (1<<(gCTX[eTIMER_DEV_0].TimerCh[i].idx*4 + gCTX[eTIMER_DEV_0].TimerCh[i].sub_idx))){
			//printf("ISR. Timer ch: %d\n", i);
			if(gCTX[eTIMER_DEV_0].TimerCh[i].TIMERCallback)gCTX[eTIMER_DEV_0].TimerCh[i].TIMERCallback();

	   		gu32TIMERIntcCnt[eTIMER_DEV_0][i]++;

	   		//LOG_DEBUG_ISR("ISR. Timer dev: 0, ch: %d\n", i);

		}
	}
	gpTimer[eTIMER_DEV_0]->irq_status = reg;

    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

ISR(timer_isr1, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	uint32 i;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpTimer[eTIMER_DEV_1]->irq_status;

	for(i=0;i<gCTX[eTIMER_DEV_1].nTimer;i++){
		if(reg & (1<<(gCTX[eTIMER_DEV_1].TimerCh[i].idx*4 + gCTX[eTIMER_DEV_1].TimerCh[i].sub_idx))){
			//printf("ISR. Timer ch: %d\n", i);
			if(gCTX[eTIMER_DEV_1].TimerCh[i].TIMERCallback)gCTX[eTIMER_DEV_1].TimerCh[i].TIMERCallback();
	   		gu32TIMERIntcCnt[eTIMER_DEV_1][i]++;

		}
	}
	gpTimer[eTIMER_DEV_1]->irq_status = reg;

    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}



STATIC sint32 timer_init(uint8 dev, PP_TIMER_CFG_S *cfg)
{
	sint32 i;
	sint32 timer_ch;
	sint32 pwm_ch;

	gpTimer[dev]->ch_enable = 0;
	gpTimer[dev]->irq_enable = 0;

	memset(&gCTX[dev], 0, sizeof(PP_TIMER_CONTEXT_S));
    gCTX[dev].nTimer = 0;
	timer_ch = gCTX[dev].nTimer;

    gCTX[dev].nPWM = 0;
	pwm_ch = gCTX[dev].nPWM;

	for (i = 0; i < 4; i++)
    {

		if(cfg->enTimerMode[i] == ePIT_MODE_TIMER)
        {
	        gCTX[dev].TimerCh[i].idx = i;
	        gCTX[dev].TimerCh[i].sub_idx = 0;
	        gCTX[dev].TimerCh[i].bit_pos = 0;
	        gCTX[dev].TimerCh[i].nbits = 32;

	        cfg->enTimerMode[i] = PIT_MODE_TIMER32;

	        timer_ch++;
	    }
        else if(cfg->enTimerMode[i] == ePIT_MODE_PWM)
        {
			gCTX[dev].PWMCh[i].idx = i;
			gCTX[dev].PWMCh[i].sub_idx = 3;
			gCTX[dev].PWMCh[i].bit_pos = 0;
			gCTX[dev].PWMCh[i].nbits = 16;

			cfg->enTimerMode[i] = PIT_MODE_PWM;

			pwm_ch++;
		}


#if 0
		if(cfg->enTimerMode[i] == PIT_MODE_TIMER32)
        {
	        gCTX[dev].TimerCh[timer_ch].idx = i;
	        gCTX[dev].TimerCh[timer_ch].sub_idx = 0;
	        gCTX[dev].TimerCh[timer_ch].bit_pos = 0;
	        gCTX[dev].TimerCh[timer_ch].nbits = 32;
	        timer_ch++;
	    }
        else if(cfg->enTimerMode[i] == PIT_MODE_PWM)
        {
			gCTX[dev].PWMCh[pwm_ch].idx = i;
			gCTX[dev].PWMCh[pwm_ch].sub_idx = 3;
			gCTX[dev].PWMCh[pwm_ch].bit_pos = 0;
			gCTX[dev].PWMCh[pwm_ch].nbits = 16;
			pwm_ch++;
		}

        else if(cfg->u32TimerMode[i] == PIT_MODE_TIMER16)
        {
	        gCTX[dev].TimerCh[timer_ch].idx = i;
	        gCTX[dev].TimerCh[timer_ch].sub_idx = 0;
	        gCTX[dev].TimerCh[timer_ch].bit_pos = 0;
	        gCTX[dev].TimerCh[timer_ch].nbits = 16;
	        timer_ch++;
	        gCTX[dev].TimerCh[timer_ch].idx = i;
	        gCTX[dev].TimerCh[timer_ch].sub_idx = 1;
	        gCTX[dev].TimerCh[timer_ch].bit_pos = 16;
	        gCTX[dev].TimerCh[timer_ch].nbits = 16;
	        timer_ch++;
		}
        else if(cfg->u32TimerMode[i] == PIT_MODE_TIMER8)
        {
			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 0;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 0;
			gCTX[dev].TimerCh[timer_ch].nbits = 8;
			timer_ch++;
			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 1;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 8;
			gCTX[dev].TimerCh[timer_ch].nbits = 8;
			timer_ch++;
			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 2;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 16;
			gCTX[dev].TimerCh[timer_ch].nbits = 8;
			timer_ch++;
			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 3;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 24;
			gCTX[dev].TimerCh[timer_ch].nbits = 8;
			timer_ch++;
		}
        else if(cfg->u32TimerMode[i] == PIT_MODE_PWM_TIMER16_MIX)
        {
			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 0;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 0;
			gCTX[dev].TimerCh[timer_ch].nbits = 16;
			timer_ch++;

			gCTX[dev].PWMCh[pwm_ch].idx = i;
			gCTX[dev].PWMCh[pwm_ch].sub_idx = 3;
			gCTX[dev].PWMCh[pwm_ch].bit_pos = 16;
			gCTX[dev].PWMCh[pwm_ch].nbits = 8;
			pwm_ch++;
		}
        else if(cfg->u32TimerMode[i] == PIT_MODE_PWM_TIMER8_MIX)
        {
			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 0;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 0;
			gCTX[dev].TimerCh[timer_ch].nbits = 8;
			timer_ch++;

			gCTX[dev].TimerCh[timer_ch].idx = i;
			gCTX[dev].TimerCh[timer_ch].sub_idx = 1;
			gCTX[dev].TimerCh[timer_ch].bit_pos = 8;
			gCTX[dev].TimerCh[timer_ch].nbits = 8;
			timer_ch++;

			gCTX[dev].PWMCh[pwm_ch].idx = i;
			gCTX[dev].PWMCh[pwm_ch].sub_idx = 3;
			gCTX[dev].PWMCh[pwm_ch].bit_pos = 16;
			gCTX[dev].PWMCh[pwm_ch].nbits = 8;
			pwm_ch++;
		}

#endif

		gpTimer[dev]->ch_reg[i].ctrl = (((cfg->u32PWMInitVal&1)<<4) | (0<<3) | cfg->enTimerMode[i]);	// External clock
		//gpTimer[dev]->ch_reg[i].ctrl = (((cfg->u32PWMInitVal&1)<<4) | (1<<3) | cfg->u32TimerMode[i]);	// APB clock

	}
	gCTX[dev].nTimer = timer_ch;
	gCTX[dev].nPWM = pwm_ch;
	cfg->u32nTimerCh = gCTX[dev].nTimer;
	cfg->u32nPWMCh = gCTX[dev].nPWM;

	//gCTX[dev].TIMERCallback = timer_callback;

	if(gCTX[dev].nPWM > MAX_PWM_NUM)
    {
		LOG_CRITICAL("Error! Too many pwm initialized\n");
		return -1;
	}
	if(gCTX[dev].nTimer > MAX_TIMER_NUM)
    {
		LOG_CRITICAL("Error! Too many timer initialized\n");
		return -1;
	}

	//show_timer_info(dev);

	return 1;
}

STATIC void timer_set_callback(uint32 dev, uint32 ch, TIMER_ISR_CALLBACK timer_callback)
{
	gCTX[dev].TimerCh[ch].TIMERCallback = timer_callback;
}

STATIC uint32 timer_get_id_revision (uint8 dev)
{
    return gpTimer[dev]->id;
}


STATIC uint32 timer_get_irq_status(uint32 dev)
{
    return gpTimer[dev]->irq_status;
}

STATIC void timer_set_irq_status(uint32 dev, uint32 status)
{
    gpTimer[dev]->irq_status = status;
}

STATIC void timer_reset (uint32 dev)
{
    gpTimer[dev]->irq_enable = 0; /* disable all timer interrupt */
    gpTimer[dev]->ch_enable = 0; /* disable all timer */
    gpTimer[dev]->irq_status = 0; /* clear pending events */

    gpTimer[dev]->ch_reg[0].reload = 0; /* clean channel 0 reload */
    gpTimer[dev]->ch_reg[1].reload = 0; /* clean channel 1 reload */
    gpTimer[dev]->ch_reg[2].reload = 0; /* clean channel 2 reload */
    gpTimer[dev]->ch_reg[3].reload = 0; /* clean channel 3 reload */
}

STATIC void timer_start (uint32 dev, uint32 ch)
{
    /* 	config channel mode 	 */
	/* 	32 bits timer, APB clock */
   // gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].ctrl = (PIT_CH_CTL_APBCLK | PIT_CH_CTL_TMR32);

    /* 	enable channel 	 */
    //gpTimer[dev]->ch_enable |= (0x1 << (5 * (ch)));
	gpTimer[dev]->ch_enable |= (0x1 <<(gCTX[dev].TimerCh[ch].idx*4 + gCTX[dev].TimerCh[ch].sub_idx));
}


STATIC void timer_stop (uint32 dev, uint32 ch)
{
    /* 	disable channel 	 */
    //gpTimer[dev]->ch_enable &= ~(0x1 << (5 * (ch)));
	gpTimer[dev]->ch_enable &= ~(0x1 <<(gCTX[dev].TimerCh[ch].idx*4 + gCTX[dev].TimerCh[ch].sub_idx));
}



STATIC void timer_irq_enable (uint32 dev, uint32 ch, uint32 enable)
{
    if (enable)
    {
        //gpTimer[dev]->irq_enable |= (0x1 << (5 * (ch)));
    	gpTimer[dev]->irq_enable |= (0x1 <<(gCTX[dev].TimerCh[ch].idx*4 + gCTX[dev].TimerCh[ch].sub_idx));
    }
    else
    {
        //gpTimer[dev]->irq_enable &= ~(0x1 << (5 * (ch)));
    	gpTimer[dev]->irq_enable &= ~(0x1 <<(gCTX[dev].TimerCh[ch].idx*4 + gCTX[dev].TimerCh[ch].sub_idx));
    }
}


STATIC void timer_set_period(uint8 dev, uint32 ch, uint32 period)
{
    //gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].reload = period;
	gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].reload =
			utilPutBits(gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].reload, gCTX[dev].TimerCh[ch].bit_pos, gCTX[dev].TimerCh[ch].nbits, period);
}

STATIC uint32 timer_get_cnt(uint8 dev, uint32 ch)
{
    if (ch >= gCTX[dev].nTimer)
    {
        return 0;
    }

	//return gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].counter;
    return utilGetBits( gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].counter, gCTX[dev].TimerCh[ch].bit_pos, gCTX[dev].TimerCh[ch].nbits );
}

STATIC uint32 timer_get_period(uint8 dev, uint32 ch)
{
    //return gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].reload;
	return utilGetBits( gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].reload, gCTX[dev].TimerCh[ch].bit_pos, gCTX[dev].TimerCh[ch].nbits );
}

STATIC void	pwm_start( uint32 dev, uint32 ch)
{
	gpTimer[dev]->ch_enable |= (0x1 <<(gCTX[dev].PWMCh[ch].idx*4 + gCTX[dev].PWMCh[ch].sub_idx));
}

STATIC void	pwm_stop( uint32 dev, uint32 ch)
{
	gpTimer[dev]->ch_enable &= ~(0x1 <<(gCTX[dev].PWMCh[ch].idx*4 + gCTX[dev].PWMCh[ch].sub_idx));
}

STATIC void pwm_set_period(uint8 dev, uint32 ch, uint16 high_period, uint16 low_period)
{
	gpTimer[dev]->ch_reg[gCTX[dev].PWMCh[ch].idx].reload =
		utilPutBits(gpTimer[dev]->ch_reg[gCTX[dev].PWMCh[ch].idx].reload, gCTX[dev].PWMCh[ch].bit_pos, gCTX[dev].PWMCh[ch].nbits, low_period);						// pwm low reload

	gpTimer[dev]->ch_reg[gCTX[dev].PWMCh[ch].idx].reload =
		utilPutBits(gpTimer[dev]->ch_reg[gCTX[dev].PWMCh[ch].idx].reload, gCTX[dev].PWMCh[ch].bit_pos + gCTX[dev].PWMCh[ch].nbits, gCTX[dev].PWMCh[ch].nbits, high_period);	// pwm high reload
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PP_RESULT_E TIMER0_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	sint8 name[16];
	uint32 dev, ch;
//	PRINT_PROC_IRQ(NAME, NUM, CNT, INTERVAL_TIME)


	dev = 0;
	for(ch=0;ch<MAX_TIMER_NUM;ch++){
		sprintf(name,"TIMER-%02d:%02d", dev, ch);
		PRINT_PROC_IRQ(name, gTimerIrqVector[dev], gu32TIMERIntcCnt[dev][ch]);

	}

	return(eSUCCESS);
}

PP_RESULT_E TIMER1_PROC_IRQ(PP_S32 argc, const PP_CHAR **argv)
{
	sint8 name[16];
	uint32 dev, ch;
//	PRINT_PROC_IRQ(NAME, NUM, CNT, INTERVAL_TIME)


	dev = 1;
	for(ch=0;ch<MAX_TIMER_NUM;ch++){
		sprintf(name,"TIMER-%02d:%02d", dev, ch);
		PRINT_PROC_IRQ(name, gTimerIrqVector[dev], gu32TIMERIntcCnt[dev][ch]);

	}


	return(eSUCCESS);
}

PP_RESULT_E TIMER_PROC_DEVICE(PP_S32 argc, const PP_CHAR **argv)
{
	uint32 dev, ch;
	uint32 idx;

	if( (argc) && (strcmp(argv[0], stTIMER_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}

	printf("\n%s Device Info -------------\n", stTIMER_PROC_devices[0].pName);

	/* TODO add device info printf */

	for(dev=0;dev<MAX_PIT_DEV_NUM;dev++){

		printf("[Device %d]\n", dev);
		if(!((gu32TIMERDevInitFlag>>dev)&1)){
			printf("Not initialized\n");
			continue;
		}

		printf("Num of Timer: %d\n", gCTX[dev].nTimer);
		printf("Num of PWM: %d\n", gCTX[dev].nPWM);

		for(ch=0;ch<gCTX[eTIMER_DEV_0].nTimer;ch++){
			idx = (gCTX[dev].TimerCh[ch].idx*4 + gCTX[dev].TimerCh[ch].sub_idx);

			printf("Timer ch %d\n", ch);
			printf("\t%4s %8s %4s %8s %8s %8s\n", "idx", "sub_idx", "bits", "ch_en", "irq_en", "period");
			printf("\t%4d %8d %4d %8d %8d %8x\n",	gCTX[dev].TimerCh[ch].idx,
													gCTX[dev].TimerCh[ch].sub_idx,
													gCTX[dev].TimerCh[ch].nbits,
													(sint32)((gpTimer[dev]->ch_enable>>idx)&1),
													(sint32)((gpTimer[dev]->irq_enable>>idx)&1),
													(sint32)(utilGetBits( gpTimer[dev]->ch_reg[gCTX[dev].TimerCh[ch].idx].reload, gCTX[dev].TimerCh[ch].bit_pos, gCTX[dev].TimerCh[ch].nbits )) );
		}

		for(ch=0;ch<gCTX[eTIMER_DEV_0].nPWM;ch++){
			idx = (gCTX[dev].PWMCh[ch].idx*4 + gCTX[dev].PWMCh[ch].sub_idx);

			printf("PWM ch %d\n", ch);
			printf("idx        sub_idx    bits       ch_en      period_h   period_l\n");
			printf("\t%4s %8s %4s %8s %8s %8s\n", "idx", "sub_idx", "bits", "ch_en", "period_h", "period_l");
			printf("\t%4d %8d %4d %8d %8x %8x\n",	gCTX[dev].PWMCh[ch].idx,
													gCTX[dev].PWMCh[ch].sub_idx,
													gCTX[dev].PWMCh[ch].nbits,
													(sint32)((gpTimer[dev]->ch_enable>>idx)&1),
													(sint32)(utilGetBits(gpTimer[dev]->ch_reg[gCTX[dev].PWMCh[ch].idx].reload, gCTX[dev].PWMCh[ch].bit_pos + gCTX[dev].PWMCh[ch].nbits, gCTX[dev].PWMCh[ch].nbits)),
													(sint32)(utilGetBits(gpTimer[dev]->ch_reg[gCTX[dev].PWMCh[ch].idx].reload, gCTX[dev].PWMCh[ch].bit_pos, gCTX[dev].PWMCh[ch].nbits)) );

		}
	}

	return(eSUCCESS);
}

