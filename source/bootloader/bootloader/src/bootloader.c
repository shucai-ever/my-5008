#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "spi_flash.h"
#if defined(SUPPORT_FLASH_BOOT)
#include "flash_boot.h"
#endif
#if defined(SUPPORT_SDCARD_BOOT)
#include "sdcard_boot.h"
#endif
#if defined(SUPPORT_EXTERNAL_BOOT)
#include "ext_boot.h"
#endif
#include "bootloader.h"
#include "common.h"
#include "debug.h"
#include "cache.h"
#include "../inc/ddr_calibration.h"

extern void SetClockPD(void);
uint8 gGlobalBuf[MAX_DATA_SIZE];

#ifdef CLOCK_INIT
extern void	asic_init(void);
#endif // CLOCK_INIT

/*
int jump_and_link(uint32 addr, int a)
{
	__asm__ __volatile__ (
		"li		$r5, #0\n\t"
		"add	$r6, $r5, %0\n\t"
		"jal	$r6\n\t"
		"nop\n\t"

		: : "r" (addr)
	);

}
*/
#if 0
void jump_infinite_loop(uint32 addr){

	*(uint32 *)addr = 0x009200d5;

	__asm__ __volatile__ (
		"li		$r1, #0\n\t"
		"add	$r0, $r1, %0\n\t"
		"jr		$r0\n\t"
		"nop\n\t"
		: : "r" (addr)
	);

}
#endif

//int pattern_test(void)
int pattern_test(uint32 *dram_addr, uint32 size)
{
	uint32 ret = 0;
	int i;

	dbg("pattern test. addr: 0x%x, size: 0x%x\n", (uint32)dram_addr, size);
	size /= 4;

	for(i=0;i<size;i++){
		*(dram_addr + i) = (i*0x55aa)/3 + i;
	}

	*((uint32 *)0xf010006c) = 0xffffffff;
	for(i=0;i<size;i++){
		if(*(dram_addr + i) != ((i*0x55aa)/3 + i)){
			*((uint32 *)0xf010006c) = i;
			ret = 1;
			dbg("ERROR! pattern test. addr: 0x%x, W: 0x%x, R: 0x%x\n", (uint32)dram_addr+i, (i*0x55aa)/3 + i, *(dram_addr + i));
			break;
		}
	}

	return ret;
}
#if defined(SUPPORT_DDR_TEST_MODE)
int get_memory_size(void)
{

    unsigned int rdata;
    int i;
    int size = 0;

    for(i=0;i<6;i++){		// 16, 32, 64, 128, 256, 512 MB
    	*(volatile unsigned int *) (DDR_MEM_BASE+0x1000000*(1<<i)) = 16*(1<<i);
    }
    *(volatile unsigned int *) (DDR_MEM_BASE) = 0x89abcdef;

    for(i=0;i<6;i++){
		rdata = *(volatile unsigned int *) (DDR_MEM_BASE+0x1000000*(1<<i));
		if (rdata == 0x89abcdef) {
			size = (16*(1<<i));
			break;
		}
    }
    if(i == 6)size = 1024;	// 1GB, MAX

    return size;

}

#endif

unsigned int bootloader_main(void)
{
	unsigned int jump_addr = 0xffffffff;
	uint32 bootmode = 0;

	cache_disable();

	bootmode = *(vuint32 *)(BOOT_MODE_ADDR);
	bootmode = (bootmode >> 12)&0x7;

	*(vuint32 *)(MISC_BASE_ADDR + 0x10) = 0x01020302; 	//SDCARD sck in/out
	*(vuint32 *)(MISC_BASE_ADDR + 0x18) = 0x03030302; 	//QSPI sck in/out
	*(vuint32 *)(MISC_BASE_ADDR + 0x60) = 0;			// set to default ( bootrom use this register bit8~15 )

#if 0
// ===============================================================================
// temp for boot time measure
	uart_init(115200, 3 /*data 8bit*/, 0/*stop 1bit*/, 0/*no parity*/);
	uart_outbyte('B');
	uart_outbyte('\r');
	uart_outbyte('\n');
// ===============================================================================
#endif

#if defined(DEBUG_UART)
	uart_init(115200, 3 /*data 8bit*/, 0/*stop 1bit*/, 0/*no parity*/);

	// boot mode select
	dbg("[bootloader]boot mode: %d\n", bootmode);
	// temp - parallel isp. desrializer pdwn control
	//*(vuint32 *)(0xf0100004) = 0x02010201;
#endif

	//====================================
	// set pll
	//====================================
#ifdef CLOCK_INIT
	asic_init();
    SetClockPD();

	//====================================
	// set ddr controller
	//====================================
#if defined(DDR2_INIT)
	extern void ddr2_set(void);
	ddr2_set();
#elif defined(DDR3_INIT)
	extern void ddr3_set(void);
	ddr3_set();
#else
    #error "unknown ddr type"
#endif

#if defined(DDR2_INIT)
    ddr_calibration(DDR_CAL_WLEVEL | DDR_CAL_RLEVEL); // ddr calibration
#elif defined(DDR3_INIT)
    // ihkong - 0914
#ifndef DDR_AUTO_LEVELING
	ddr_calibration(DDR_CAL_WLEVEL | DDR_CAL_RLEVEL); // ddr calibration
#endif
    //ddr_calibration(DDR_CAL_GDS | DDR_CAL_RDQDQS | DDR_CAL_WDQDQS ); // ddr calibration
    //ddr_calibration(DDR_CAL_GDS ); // ddr calibration
#else
    #error "unknown ddr type"
#endif

#endif // CLOCK_INIT

/////////////////////////////////////////////////////////////////////////////
	    
	cache_enable(); //must do this position.

#if defined(SUPPORT_DDR_TEST_MODE)
	// set GPIO0[1] output mode & high
	*(vuint32 *)(GPIO0_BASE_ADDR + 0x24) |= 0x2;	// GPIO0[1] high
	*(vuint32 *)(GPIO0_BASE_ADDR + 0x28) |= 0x2;	// GPIO0[1] dir -> output
	*(vuint32 *)(MISC_BASE_ADDR) &= (~0xff00);
	*(vuint32 *)(MISC_BASE_ADDR) |= 0x200;			// IO PAD for GPIO0[1] -> output

	// check ddr test mode
	if(!(*(vuint32 *)(GPIO0_BASE_ADDR + 0x20) & 1)){
		dbg("ddr test mode begin\n");
		if(pattern_test((uint32 *)DDR_MEM_BASE, (uint32)(get_memory_size()*1024*1024))){	// full size test
			dbg("Error!!! ddr test fail\n");
			*(vuint32 *)(GPIO0_BASE_ADDR + 0x24) &= (~0x2);	// GPIO0[1] low
			while(1);
		}
		dbg("ddr test done\n");
	}else{
		if(pattern_test((uint32 *)DDR_MEM_BASE, 0x10000)){
			dbg("Error!!! ddr test fail\n");
			*(vuint32 *)(GPIO0_BASE_ADDR + 0x24) &= (~0x2);	// GPIO0[1] low
			while(1);
		}
	}
#else

	if(pattern_test((uint32 *)DDR_MEM_BASE, 0x10000)){
		dbg("Error!!! ddr test fail\n");
		while(1);
	}
#endif

#if defined(PI5008_FPGA)
	// wait until ddr init done - fpga
	dbg("waiting ddr init...\n");
	while(!(*(vuint32 *)(BOOT_INFO_ADDR) & 0x8));
	dbg("waiting ddr init done\n");
#endif

#if defined(SUPPORT_FLASH_BOOT)
	if(bootmode == eBOOT_MODE_NOR_FLASH){
		dbg("nor flash boot\n");
		jump_addr = spi_nor_flash_boot();
	}

	if(bootmode == eBOOT_MODE_NAND_FLASH){
		dbg("nand flash boot\n");
		jump_addr = spi_nand_flash_boot();
	}
#endif

#if defined(SUPPORT_SDCARD_BOOT)
	if(bootmode == eBOOT_MODE_SDCARD){
		dbg("sdcard boot\n");
		jump_addr = sdcard_boot();
	}
#endif

#if defined(SUPPORT_EXTERNAL_BOOT)
	if(bootmode == eBOOT_MODE_SPI){
		//dbg("external spi boot\n");
		jump_addr = external_boot(eBOOT_MODE_SPI);
	}else if(bootmode == eBOOT_MODE_UART){
		//dbg("external uart boot\n");
		jump_addr = external_boot(eBOOT_MODE_UART);
	}
#endif

	if(jump_addr == 0xffffffff){	// Boot fail
		cache_disable();
		dbg("bootloader boot fail\n");
		//while(1);
		return 0;
	}

	dbg("[bootloader]jump addr: 0x%x\n", jump_addr);

#if 0
// ===============================================================================
// temp for boot time measure
	uart_outbyte('|');
	uart_outbyte('\r');
	uart_outbyte('\n');
// ===============================================================================
#endif	

	cache_disable();

	return jump_addr;
}


#if defined(DEBUG_UART)
//==================================================
// overriding function for printf redirection -> uart
//==================================================
#undef putchar
inline int putchar(int c)
{

    /* LF -> CR+LF due to some terminal programs */
    if ( c == '\n' )
        putchar( '\r' );

    uart_outbyte(c);

	return c;
}


__attribute__((used))
void nds_write(const unsigned char *buf, int size)
{	
	int    i;	
	for (i = 0; i < size; i++)		
		putchar(buf[i]);
}
#endif

