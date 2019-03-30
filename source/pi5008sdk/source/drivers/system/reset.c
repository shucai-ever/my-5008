
#include "nds32_intrinsic.h"
#include "nds32.h"
#include "error.h"
#include "timer.h"
#include "clock.h"
#include "reset.h"
#if 0
#include "debug.h"
#endif

#ifndef VECTOR_BASE
#define VECTOR_BASE	0x20100000
#endif

#define PSW_MSK                                         \
        (PSW_mskGIE | PSW_mskINTL | PSW_mskPOM | PSW_mskIFCON | PSW_mskCPL)
#define PSW_INIT                                        \
        (0x0UL << PSW_offGIE                            \
         | 0x0UL << PSW_offINTL                         \
         | 0x1UL << PSW_offPOM                          \
         | 0x0UL << PSW_offIFCON                        \
         | 0x7UL << PSW_offCPL)

#define IVB_MSK                                         \
        (IVB_mskEVIC | IVB_mskESZ | IVB_mskIVBASE)
#define IVB_INIT                                        \
        ((VECTOR_BASE >> IVB_offIVBASE) << IVB_offIVBASE\
         | 0x1UL << IVB_offESZ                          \
         | 0x0UL << IVB_offEVIC)


#pragma weak c_startup = c_startup_common

void c_startup(void);

/*
 * Default c_startup() function which used for those relocation from LMA to VMA.
 */
static void c_startup_common(void)
{
/* Use compiler builtin memcpy and memset */
#define MEMCPY(des, src, n) __builtin_memcpy ((des), (src), (n))
#define MEMSET(s, c, n) __builtin_memset ((s), (c), (n))

	extern char __data_lmastart;
	extern char __data_start;
	extern char _edata;

#if 0
	extern char __bss_start;
	extern char _end;
	uint32 *buf;
	uint32 i;
#endif

	int size;

	/* Copy data section from ROM to RAM*/
	size = &_edata - &__data_start;
	MEMCPY(&__data_start, &__data_lmastart, size);

#if 0
	size = &_end - &__bss_start;
	size /= 4;
	buf = (uint32 *)&__bss_start;
	for(i=0;i<size;i++){
		if(buf[i] != 0){
			printf("error\n");
		}

	}
#endif
}

static void cpu_init(void)
{
	unsigned int reg;

	/* Set PSW GIE/INTL to 0, superuser & CPL to 7 */
	reg = (__nds32__mfsr(NDS32_SR_PSW) & ~PSW_MSK) | PSW_INIT;
	__nds32__mtsr(reg, NDS32_SR_PSW);
	__nds32__isb();

	/* Set vector size: 16 byte, base: VECTOR_BASE, mode: IVIC */
	reg = (__nds32__mfsr(NDS32_SR_IVB) & ~IVB_MSK) | IVB_INIT;
	__nds32__mtsr(reg, NDS32_SR_IVB);

	/*
	 * Check interrupt priority programmable (IVB.PROG_PRI_LVL)
	 * 0: Fixed priority, 1: Programmable priority
	 */
	if (reg & IVB_mskPROG_PRI_LVL) {
		/* Set PPL2FIX_EN to 0 to enable Programmable Priority Level */
		__nds32__mtsr(0x0, NDS32_SR_INT_CTRL);
	}

	/* Mask and clear hardware interrupts */
	if (reg & IVB_mskIVIC_VER) {
		/* IVB.IVIC_VER >= 1*/
		__nds32__mtsr(0x0, NDS32_SR_INT_MASK2);
		__nds32__mtsr(-1, NDS32_SR_INT_PEND2);
	} else {
		__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK) & ~0xFFFF, NDS32_SR_INT_MASK);
	}

    /* Edge-trigger */
	__nds32__mtsr_isb(0xFFFFFFFF, NDS32_SR_INT_TRIGGER);
    /* Edge-trigger PI5008 (Core0, Coare1) */
    *(vuint32 *)(MISC_BASE_ADDR + 0x0078) = 0xFFFFFFFF;
    *(vuint32 *)(MISC_BASE_ADDR + 0x007C) = 0xFFFFFFFF;

}

/*
 * Vectors initialization. This means to copy exception handler code to
 * vector entry base address.
 */
static void vector_init(void)
{
	extern unsigned int OS_Int_Vectors, OS_Int_Vectors_End;

	if ((unsigned int)&OS_Int_Vectors != VECTOR_BASE) {
		volatile unsigned int *vector_srcptr = &OS_Int_Vectors;
		volatile unsigned int *vector_dstptr = (unsigned int *)VECTOR_BASE;

		/* copy vector table to VECTOR_BASE */
		while (vector_srcptr != &OS_Int_Vectors_End)
			*vector_dstptr++ = *vector_srcptr++;
	}
}

void clock_powerdown(const unsigned int u32PowerUpBit, const unsigned int u32PowerDownBit)
{

    /* power up bit */
    {/*{{{*/
        if(u32PowerUpBit & CLK_ENNUM_QSPI)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00000080;  //Qspi 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_TIMERS)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00008000;  //timers 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_WDT)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x00800000;  //wdt 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_UART)
        {
            *(volatile unsigned int *)0xf0000004 &= (unsigned int)~0x80000000;  //uart 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_GADC)
        {
            *(volatile unsigned int *)0xf0000008 &= (unsigned int)~0x00000080;  //gadc 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_SVM)
        {
            *(volatile unsigned int *)0xf0000008 &= (unsigned int)~0x00800000;  //svm 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_PVIRX)
        {
            *(volatile unsigned int *)0xf000000C &= (unsigned int)~0x80808080;  //pvirx 0:en, 1:dis
            *(volatile unsigned int *)0xf0000010 &= (unsigned int)~0x80808080;  //vadc 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_ISP)
        {
            *(volatile unsigned int *)0xf0000014 &= (unsigned int)~0x80808080;  //isp 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_VIN)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00000080;  //vin 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_VPU)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00008000;  //vpu 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_DU)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x00800000;  //du 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_PVITX)
        {
            *(volatile unsigned int *)0xf0000018 &= (unsigned int)~0x80000000;  //pvitx rd 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x80000000;  //pvitx wr 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00000080;  //pvitx fifo 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00008000;  //pvitx vdac 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_RO)
        {
            *(volatile unsigned int *)0xf000001C &= (unsigned int)~0x00800000;  //recfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00000080;  //recfmt_2x_clock 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00008000;  //recfmt_2x_mph0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x00800000;  //recfmt_2x_mph1 0:en, 1:dis

            *(volatile unsigned int *)0xf0100054 = (unsigned int)0x02020202;  //set pad output.
        }
        if(u32PowerUpBit & CLK_ENNUM_QUAD)
        {
            *(volatile unsigned int *)0xf0000020 &= (unsigned int)~0x80000000;  //quad 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x00000080;  //quad_2x 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_DOUT)
        {
            *(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x00008000;  //disfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 &= (unsigned int)~0x80000000;  //disfmt_2x 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_MIPI)
        {
            *(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x00008000;  //rxbyteclkhs 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x00800000;  //mipi_lane 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 &= (unsigned int)~0x80000000;  //mipi_pixel 0:en, 1:dis
            *(volatile unsigned int *)0xf000002C &= (unsigned int)~0x00000080;  //mipi cci 0:en, 1:dis
        }
#if 0 //always up
        if(u32PowerUpBit & CLK_ENNUM_DDR)
        {
            *(volatile unsigned int *)0xf000002C &= (unsigned int)~0x80808000;  //ddr 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_JTAG)
        {
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00000080;  //jtag 0:en, 1:dis
        }
#endif
        if(u32PowerUpBit & CLK_ENNUM_I2S)
        {
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00008000;  //i2S 0:en, 1:dis
        }
        if(u32PowerUpBit & CLK_ENNUM_PARAVIN)
        {
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x00800000;  //par0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000030 &= (unsigned int)~0x80000000;  //par1 0:en, 1:dis
        }
    }/*}}}*/
    /* power down bit */
    {/*{{{*/
        if(u32PowerDownBit & CLK_ENNUM_QSPI)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00000080;  //Qspi 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_TIMERS)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00008000;  //timers 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_WDT)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x00800000;  //wdt 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_UART)
        {
            *(volatile unsigned int *)0xf0000004 |= (unsigned int)0x80000000;  //uart 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_GADC)
        {
            *(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00000080;  //gadc 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_SVM)
        {
            *(volatile unsigned int *)0xf0000008 |= (unsigned int)0x00800000;  //svm 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_PVIRX)
        {
            *(volatile unsigned int *)0xf000000C |= (unsigned int)0x80808080;  //pvirx 0:en, 1:dis
            *(volatile unsigned int *)0xf0000010 |= (unsigned int)0x80808080;  //vadc 0:en, 1:dis

            /* power down adc */
            *(volatile unsigned int *)0xf0700054 |= (unsigned int)0x00000010;  // power down
            *(volatile unsigned int *)0xf0701054 |= (unsigned int)0x00000010;  // power down
            *(volatile unsigned int *)0xf0702054 |= (unsigned int)0x00000010;  // power down
            *(volatile unsigned int *)0xf0703054 |= (unsigned int)0x00000010;  // power down
        }
        if(u32PowerDownBit & CLK_ENNUM_ISP)
        {
            *(volatile unsigned int *)0xf0000014 |= (unsigned int)0x80808080;  //isp 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_VIN)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00000080;  //vin 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_VPU)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00008000;  //vpu 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_DU)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x00800000;  //du 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_PVITX)
        {
            *(volatile unsigned int *)0xf0000018 |= (unsigned int)0x80000000;  //pvitx rd 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x80000000;  //pvitx wr 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x00000080;  //pvitx fifo 0:en, 1:dis
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x00008000;  //pvitx vdac 0:en, 1:dis

            /* power down dac */
            *(volatile unsigned int *)0xf0100070 |= (unsigned int)0x00100000;  // power down
        }
        if(u32PowerDownBit & CLK_ENNUM_RO)
        {
            *(volatile unsigned int *)0xf000001C |= (unsigned int)0x00800000;  //recfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00000080;  //recfmt_2x_clock 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00008000;  //recfmt_2x_mph0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x00800000;  //recfmt_2x_mph1 0:en, 1:dis

            *(volatile unsigned int *)0xf0100054 = (unsigned int)0x01010101;  //set pad input.
        }
        if(u32PowerDownBit & CLK_ENNUM_QUAD)
        {
            *(volatile unsigned int *)0xf0000020 |= (unsigned int)0x80000000;  //quad 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 |= (unsigned int)0x00000080;  //quad_2x 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_DOUT)
        {
            *(volatile unsigned int *)0xf0000024 |= (unsigned int)0x00008000;  //disfmt 0:en, 1:dis
            *(volatile unsigned int *)0xf0000024 |= (unsigned int)0x80000000;  //disfmt_2x 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_MIPI)
        {
            *(volatile unsigned int *)0xf0000028 |= (unsigned int)0x00008000;  //rxbyteclkhs 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 |= (unsigned int)0x00800000;  //mipi_lane 0:en, 1:dis
            *(volatile unsigned int *)0xf0000028 |= (unsigned int)0x80000000;  //mipi_pixel 0:en, 1:dis
            *(volatile unsigned int *)0xf000002C |= (unsigned int)0x00000080;  //mipi cci 0:en, 1:dis
        }
#if 0 //always up
        if(u32PowerDownBit & CLK_ENNUM_DDR)
        {
            *(volatile unsigned int *)0xf000002C |= (unsigned int)0x80808000;  //ddr 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_JTAG)
        {
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00000080;  //jtag 0:en, 1:dis
        }
#endif
        if(u32PowerDownBit & CLK_ENNUM_I2S)
        {
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00008000;  //i2S 0:en, 1:dis
        }
        if(u32PowerDownBit & CLK_ENNUM_PARAVIN)
        {
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x00800000;  //par0 0:en, 1:dis
            *(volatile unsigned int *)0xf0000030 |= (unsigned int)0x80000000;  //par1 0:en, 1:dis
        }
    }/*}}}*/

    return;
}

void SetClockPD(void)
{
    unsigned int u32PowerSupportBit = 0;
    unsigned int u32PowerUpBit = 0;
    unsigned int u32PowerDownBit = 0;

    u32PowerSupportBit = 
        CLK_ENNUM_QSPI          |
        CLK_ENNUM_TIMERS        |
        CLK_ENNUM_WDT           |
        CLK_ENNUM_UART          |
        CLK_ENNUM_GADC          |
        CLK_ENNUM_SVM           |
        CLK_ENNUM_PVIRX         |
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
        //CLK_ENNUM_ISP           | //controlled by core1. must comment
#endif
#ifdef BD_SLT
        CLK_ENNUM_ISP           | //controlled by core1. must comment
#endif
        CLK_ENNUM_VIN           |
        CLK_ENNUM_VPU           |
        CLK_ENNUM_DU            |
        CLK_ENNUM_PVITX         |
        CLK_ENNUM_RO            |
        CLK_ENNUM_QUAD          |
        CLK_ENNUM_DOUT          |
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE == VIDEO_IN_TYPE_MIPI_YUV)
        //CLK_ENNUM_MIPI          | //controlled by core1. must comment
#endif
#ifdef BD_SLT
        CLK_ENNUM_MIPI          | //controlled by core1. must comment
#endif
        CLK_ENNUM_I2S           |
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) || (VIDEO_IN_TYPE_PB_SUPPORT == 1)
        CLK_ENNUM_PARAVIN       |
#endif
        0;


    u32PowerUpBit = 
        CLK_ENNUM_QSPI          |
        CLK_ENNUM_TIMERS        |
        CLK_ENNUM_WDT           |
        CLK_ENNUM_UART          |
        CLK_ENNUM_GADC          |
        CLK_ENNUM_SVM           |
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PVI)
        CLK_ENNUM_PVIRX         |
#endif
#ifdef BD_SLT
        CLK_ENNUM_ISP           | //controlled by core1. must comment
#endif
        CLK_ENNUM_VIN           |
        CLK_ENNUM_VPU           |
        CLK_ENNUM_DU            |
#if ( (BD_PVITX_OUT_FMT & PVITX_SRC_MASKBIT) != PVITX_SRC_NONE)
        CLK_ENNUM_PVITX         |
#endif
        CLK_ENNUM_RO            |
        CLK_ENNUM_QUAD          |
        CLK_ENNUM_DOUT          |
#ifdef BD_SLT
        CLK_ENNUM_MIPI          | //controlled by core1. must comment
#endif
        CLK_ENNUM_I2S           |
#if (VIDEO_IN_TYPE == VIDEO_IN_TYPE_PARALLEL) || (VIDEO_IN_TYPE_PB_SUPPORT == 1)
        CLK_ENNUM_PARAVIN       |
#endif
        0;

    u32PowerUpBit = u32PowerSupportBit & u32PowerUpBit;
    u32PowerDownBit = u32PowerSupportBit & (unsigned int)~u32PowerUpBit;

    clock_powerdown(u32PowerUpBit, u32PowerDownBit);
}
///////////////// PI5008 Clock PowerDown control /////////////////////////////

/*
 * NDS32 reset handler to reset all devices sequentially and call application
 * entry function.
 */
void reset(void)
{
	extern void asic_init (void);
	extern void hardware_init(void);
    extern void freertos_init(void);
	extern void main(void);

    SetClockPD();

#if defined(USE_ASIC_INIT)
    asic_init();
#endif

	/*
	 * Initialize CPU to a post-reset state, ensuring the ground doesn't
	 * shift under us while we try to set things up.
	*/
	cpu_init();

	/*
	 * Initialize LMA/VMA sections.
	 * Relocation for any sections that need to be copied from LMA to VMA.
	 */
	c_startup();

	/* Copy vector table to vector base address */
	vector_init();

	/* Call platform specific hardware initialization */
	hardware_init();

#if !defined(FPGA_ASIC_TOP)
	ClockInit();
#endif

    /* Setup the OS system required initialization */
	freertos_init();

	/* Application enrty function */
	main();

	/* Never go back here! */
	while(1);
}
