#ifndef _PI5008_BOOTLOADER_H_
#define _PI5008_BOOTLOADER_H_

#include "system.h"

#define ROM_VER_ADDR				(MISC_BASE_ADDR + 0x1f0)	// ???
#ifdef PI5008_FPGA
#define BOOT_ROM_VER				0xF0170718
#else
#define BOOT_ROM_VER				0xA0170718
#endif

#define BOOT_HEADER_SIGN			0xABCD1234
#define BOOT_MODE_ADDR				(SCU_BASE_ADDR + 0x5C)
#define BOOT_INFO_ADDR				(SCU_BASE_ADDR + 0x60)

#define ISP_FW_LOAD_ADDR			0x20000000
#define MAIN_FW_LOAD_ADDR			0x20100000


//#define SUB_CPU_RESET_ADDR			0xF0100158
#define SUB_CPU_RESET_ADDR			0xF0100080
#define SUB_CPU_RESET_VECTOR_ADDR	0xF1500084

#if 0
typedef enum tagBOOT_MODE_E
{
	eBOOT_MODE_NOR_FLASH = 0,
	eBOOT_MODE_NAND_FLASH,
	eBOOT_MODE_SDCARD,
	eBOOT_MODE_SPI,
	eBOOT_MODE_UART,
} BOOT_MODE_E;
#else
typedef enum tagBOOT_MODE_E
{
	eBOOT_MODE_NOR_FLASH = 0,
	eBOOT_MODE_SDCARD,
	eBOOT_MODE_SPI,
	eBOOT_MODE_UART,
	eBOOT_MODE_NAND_FLASH,
	eBOOT_MODE_MAX,
} BOOT_MODE_E;

#endif

	





#endif


