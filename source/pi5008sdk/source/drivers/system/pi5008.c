
#include <nds32_intrinsic.h>

#include "osal.h"
#include "pi5008.h"
#include "debug.h"
#include "nds32.h"
#include "cache.h"
#include "error.h"
#include "timer.h"
#include "interrupt.h"
#include "clock.h"

#undef MMU_ENABLE

#define CACHE_NONE              0
#define CACHE_WRITEBACK         2
#define CACHE_WRITETHROUGH      3

#if (defined(CONFIG_CPU_ICACHE_ENABLE) || defined(CONFIG_CPU_DCACHE_ENABLE))
/* Cacheable */
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
#define CACHE_MODE              CACHE_WRITETHROUGH
#else
#define CACHE_MODE              CACHE_WRITEBACK
#endif
#else
/* Uncacheable */
#define CACHE_MODE              CACHE_NONE
#endif


#define MMU_CTL_MSK                                     \
        (MMU_CTL_mskD                                   \
         | MMU_CTL_mskNTC0                              \
         | MMU_CTL_mskNTC1                              \
         | MMU_CTL_mskNTC2                              \
         | MMU_CTL_mskNTC3                              \
         | MMU_CTL_mskTBALCK                            \
         | MMU_CTL_mskMPZIU                             \
         | MMU_CTL_mskNTM0                              \
         | MMU_CTL_mskNTM1                              \
         | MMU_CTL_mskNTM2                              \
         | MMU_CTL_mskNTM3)
/*
 * NTC0: CACHE_MODE, NTC1~NTC3: Non-cacheable
 * MSC_CFG.ADR24 = 0 : NTM0~NTM3 are mapped to partition 0/0/0/0
 * MSC_CFG.ADR24 = 1 : NTM0~NTM3 are mapped to partition 0/1/2/3
 */
#define MMU_CTL_INIT                                    \
        (0x0UL << MMU_CTL_offD                          \
         | (CACHE_MODE) << MMU_CTL_offNTC0              \
         | 0x0UL << MMU_CTL_offNTC1                     \
         | 0x0UL << MMU_CTL_offNTC2                     \
         | 0x0UL << MMU_CTL_offNTC3                     \
         | 0x0UL << MMU_CTL_offTBALCK                   \
         | 0x0UL << MMU_CTL_offMPZIU                    \
         | 0x0UL << MMU_CTL_offNTM0                     \
         | 0x0UL << MMU_CTL_offNTM1                     \
         | 0x0UL << MMU_CTL_offNTM2                     \
         | 0x0UL << MMU_CTL_offNTM3)

#define MMU_CTL_INIT_ADR24                              \
        (MMU_CTL_INIT                                   \
         | 0x0UL << MMU_CTL_offNTM0                     \
         | 0x1UL << MMU_CTL_offNTM1                     \
         | 0x2UL << MMU_CTL_offNTM2                     \
         | 0x3UL << MMU_CTL_offNTM3)

#define CACHE_CTL_MSK                                   \
        (CACHE_CTL_mskIC_EN                             \
         | CACHE_CTL_mskDC_EN                           \
         | CACHE_CTL_mskICALCK                          \
         | CACHE_CTL_mskDCALCK                          \
         | CACHE_CTL_mskDCCWF                           \
         | CACHE_CTL_mskDCPMW)
/* ICache/DCache enable */
#define CACHE_CTL_CACHE_ON                              \
        (0x1UL << CACHE_CTL_offIC_EN                    \
         | 0x1UL << CACHE_CTL_offDC_EN                  \
         | 0x0UL << CACHE_CTL_offICALCK                 \
         | 0x0UL << CACHE_CTL_offDCALCK                 \
         | 0x1UL << CACHE_CTL_offDCCWF                  \
         | 0x1UL << CACHE_CTL_offDCPMW)




/* This must be a leaf function, no child function */
void _nds32_init_mem(void) __attribute__((naked, optimize("Os")));
void _nds32_init_mem(void)
{
	/* NOP */
}

/*
 * Initialize MMU configure and cache ability.
 */
#ifdef MMU_ENABLE
static void mmu_init(void)
{
#ifndef __NDS32_ISA_V3M__
	unsigned int reg;

	/* MMU initialization: NTC0~NTC3, NTM0~NTM3 */
	reg = (__nds32__mfsr(NDS32_SR_MMU_CTL) & ~MMU_CTL_MSK) | MMU_CTL_INIT;

	if (__nds32__mfsr(NDS32_SR_MSC_CFG) & MSC_CFG_mskADR24)
		reg = (__nds32__mfsr(NDS32_SR_MMU_CTL) & ~MMU_CTL_MSK) | MMU_CTL_INIT_ADR24;
	else
		reg = (__nds32__mfsr(NDS32_SR_MMU_CTL) & ~MMU_CTL_MSK) | MMU_CTL_INIT;

	__nds32__mtsr(reg, NDS32_SR_MMU_CTL);
	__nds32__dsb();
#endif
}
#endif

/*
 * Platform specific initialization
 */
static void plf_init(void)
{
	/* Set default Hardware interrupts priority */
	__nds32__mtsr(PRI1_DEFAULT, NDS32_SR_INT_PRI);
	__nds32__mtsr(PRI2_DEFAULT, NDS32_SR_INT_PRI2);

	/* Mask all HW interrupts except SWI */
    __nds32__mtsr(1 << IRQ_SWI_VECTOR, NDS32_SR_INT_MASK2);

    /* Reset the PIT (timers) */
    PPDRV_TIMER_Reset(eTIMER_DEV_0);
    //PPDRV_TIMER_Reset(eTIMER_DEV_1);

    INTC_irq_config(IRQ_SWI_VECTOR, IRQ_LEVEL_TRIGGER); //Must set Level
    INTC_swi_clean();
    INTC_swi_enable();
}



/*
 * All PI5008 hardware initialization
 */
void hardware_init(void)
{
#ifdef MMU_ENABLE
	mmu_init();                     /* mmu/cache */
#endif
	plf_init();                     /* Perform any platform specific initializations */

#if (defined(CONFIG_CPU_ICACHE_ENABLE) || defined(CONFIG_CPU_DCACHE_ENABLE))
	unsigned int reg;

	/* Invalid ICache */
	nds32_icache_flush();

	/* Invalid DCache */
	nds32_dcache_invalidate();

	/* Enable I/Dcache */
	reg = (__nds32__mfsr(NDS32_SR_CACHE_CTL) & ~CACHE_CTL_MSK) | CACHE_CTL_CACHE_ON;
	__nds32__mtsr(reg, NDS32_SR_CACHE_CTL);
#endif
}

static PP_U8 gu8Heap[configTOTAL_HEAP_SIZE];
void freertos_init(void)
{
    void *tickIsr;
    PP_TIMER_CFG_S stCfg;

    #ifdef MPU_SUPPORT
    extern signed portBASE_TYPE prvSetupMPU(void);
    prvSetupMPU();
#endif

#if configUSE_PREEMPTION == 0
    extern void vNonPreemptiveTick();
    tickIsr = vNonPreemptiveTick;
#else
    extern void vPreemptiveTick();
    tickIsr = vPreemptiveTick;
#endif

    /* System tick init */
    stCfg.enTimerMode[eTIMER_CH_0] = ePIT_MODE_TIMER;
    stCfg.enTimerMode[eTIMER_CH_1] = ePIT_MODE_TIMER;
    stCfg.enTimerMode[eTIMER_CH_2] = ePIT_MODE_TIMER;
    stCfg.enTimerMode[eTIMER_CH_3] = ePIT_MODE_TIMER;
    
    if ( PPDRV_TIMER_Initialize(SYS_OS_TIMER_DEV, &stCfg) != eSUCCESS )
    {
        OSAL_system_panic(eERROR_FAILURE);
    }

    PPDRV_TIMER_SetISR(SYS_OS_TIMER_DEV, SYS_OS_TIMER_CH, tickIsr);
    PPDRV_TIMER_Stop(SYS_OS_TIMER_DEV, SYS_OS_TIMER_CH);
    PPDRV_TIMER_SetPeriod(SYS_OS_TIMER_DEV, SYS_OS_TIMER_CH, (u32TimerClk / configTICK_RATE_HZ));
    PPDRV_TIMER_IRQControl(SYS_OS_TIMER_DEV, SYS_OS_TIMER_CH, 1);
    PPDRV_TIMER_Start(SYS_OS_TIMER_DEV, SYS_OS_TIMER_CH);

    OSAL_SetHeapConfig(gu8Heap, configTOTAL_HEAP_SIZE);

}

