/*
 * Copyright (c) 2012-2015 Andes Technology Corporation
 * All rights reserved.
 *
 */
#include <nds32_intrinsic.h>
#include "config.h"

#ifndef VECTOR_BASE
#define VECTOR_BASE	0x10000
#endif

#pragma weak syscall_handler         = Default_Handler

__attribute__((unused))
static void Default_Handler()
{
	while (1) ;
}

void __c_init()
{
/* Use compiler builtin memcpy and memset */
#define MEMCPY(des, src, n) __builtin_memcpy ((des), (src), (n))
#define MEMSET(s, c, n) __builtin_memset ((s), (c), (n))

	extern char __data_lmastart;
	extern char __data_start;
	extern char _edata;
	extern char __bss_start;
	extern char _end;
	int size;

	/* Copy data section from ROM to RAM*/
	size = &_edata - &__data_start;
	MEMCPY(&__data_start, &__data_lmastart, size);

	/* Clear bss section */
	size = &_end - &__bss_start;
	MEMSET(&__bss_start, 0, size);
	return;
}

void __cpu_init()
{
	unsigned int tmp;

	/* turn on BTB */
	tmp = 0x0;
	__nds32__mtsr(tmp, NDS32_SR_MISC_CTL);

	/* disable all hardware interrupts */
	__nds32__mtsr(0x0, NDS32_SR_INT_MASK);
	/* INT_MASK2 only exists when IVB.IVIC_VER=1 */
	if (__nds32__mfsr(NDS32_SR_IVB) & (0x1 << 11))
		__nds32__mtsr(0x0, NDS32_SR_INT_MASK2);

	tmp = VECTOR_BASE;

#if defined(USE_C_EXT) && defined(__NDS32_ISA_V3M__)
	/* Use C extension (USE_C_EXT in CLFAGS) insetad of
	 * assembly version, and v3m toolchain is used.
	 */
	tmp |= (1 << 14);	// set vector size: 16 bytes
#endif
#ifdef CFG_EVIC
	tmp |= (1 << 13);	// set EVIC
#endif
	__nds32__mtsr(tmp, NDS32_SR_IVB);


	/* Set PSW */
	/* set PSW.INTL to 0 and PSW.CPL to 7 to allow any priority */
	tmp = __nds32__mfsr(NDS32_SR_PSW);
	tmp &= 0xfffffff9;
	tmp |= 0x70008;
#ifdef CFG_HWZOL
	/* Enable PSW AEN */
	tmp = tmp | 0x2000;
#endif
	__nds32__mtsr_dsb(tmp, NDS32_SR_PSW);

	/* Check interrupt priority programmable*
	* IVB.PROG_PRI_LVL
	*      0: Fixed priority       -- no exist ir18 1r19
	*      1: Programmable priority
	*/
	if (__nds32__mfsr(NDS32_SR_IVB) & 0x01) {
		/* Set PPL2FIX_EN to 0 to enable Programmable
	 	* Priority Level */
		__nds32__mtsr(0x0, NDS32_SR_INT_CTRL);
		/* Check IVIC numbers (IVB.NIVIC) */
	}

	return;
}


void __init()
{
/*----------------------------------------------------------
   !!  Users should NOT add any code before this comment  !!
------------------------------------------------------------*/
	__cpu_init();
	__c_init();     //copy data section, clean bss

	/* Double check ZOL supporting */
	/*
	 * Check whether the CPU configured with ZOL supported.
	 * The MSC_CFG.MSC_EXT($cr4) indicates MSC_CFG2 register exist
	 * and MSC_CFG2($cr7) bit 5 indicates ZOL supporting.
	 */
#ifdef CFG_HWZOL
	if (!((__nds32__mfsr(NDS32_SR_MSC_CFG) & (3 << 30))
	    && (__nds32__mfsr(NDS32_SR_MSC_CFG2) & (1 << 5)))) {
		/* CPU doesn't support ZOL, but build with ZOL supporting. */
		//uart_puts("CPU doesn't support ZOL, but build with ZOL supporting !!\n");
		while(1);
	}
#else
	if ((__nds32__mfsr(NDS32_SR_MSC_CFG) & (3 << 30))
	    && (__nds32__mfsr(NDS32_SR_MSC_CFG2) & (1 << 5))) {
		/* CPU supports ZOL, but build without ZOL supporting. */
		//uart_puts("CPU supports ZOL, but build without ZOL supporting !!\n");
	}
#endif
}

