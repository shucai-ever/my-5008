#include <stdio.h>
#include <string.h>
#include "bootloader.h"
#include "system.h"
#include "flash_boot.h"
#include "flash_model.h"
#include "spi_flash.h"
#include "spi_nand_flash.h"
#include "spi_memctrl.h"
#include "spi.h"
#include "cache.h"
#include "debug.h"
#include "common.h"

#define BOOT_RESULT_ADDR			0x13ff0

uint32 spi_nor_flash_boot(void)
{

	SPI_MEM_FR_INFO fr_info;
	//FLASH_HDR_T fhdr;
	PP_FLASH_HDR_S stFHdr;
	uint32 jump_addr = (uint32)-1;
	PP_FLASH_SECT_E enSect;
	uint32 u32Result = 0;
	uint8 u8SectValid = 1;
	//====================================
	// set spi flash quad
	//====================================

	flash_init(PCLK/2, SPI_MODE, 1);

	memset(&fr_info, 0, sizeof(fr_info));

	// QUAD flash mode - MXIC
	fr_info.flash_type = SPI_FLASH_TYPE_NOR;	// nor flash
	fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
	fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_8;
	fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_VAL;
	fr_info.mode_value = 0;
	fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;

	if(gstFlashID.u8AddrMode4Byte){
		fr_info.addr_4byte = 1;
		spi_memctrl_set(QSPI_DIV, QSPI_DELAY, 0xEC, fr_info);
	}else{
		fr_info.addr_4byte = 0;
		spi_memctrl_set(QSPI_DIV, QSPI_DELAY, 0xEB, fr_info);
	}

	spi_memctrl_mode_set(SPI_MEMC_MEMORY_MAP_MODE, 0);
	// read flash header
	memcpy(&stFHdr, (uint8 *)(SPI_MEM_BASE_ADDR), sizeof(PP_FLASH_HDR_S));

	if(stFHdr.u32Sign != BOOT_HEADER_SIGN)goto END_FT;
	if(stFHdr.u32FlashType != SPI_FLASH_TYPE_NOR)goto END_FT;

#ifdef SPI_FLASH_READ_QDMA
	spi_memctrl_mode_set(SPI_MEMC_QDMA_MODE, 0);
#endif

	//=============================================================================
	// ISP firmware load & run
	//=============================================================================
	u8SectValid = 1;
	enSect = eFLASH_SECT_IFW;
	if(!FLASH_VER_ERROR_CHECK(stFHdr.stSect[eFLASH_SECT_IFW].u32Ver)){
		enSect = eFLASH_SECT_IFW;
	}else{
		if(!FLASH_VER_ERROR_CHECK(stFHdr.stSect[eFLASH_SECT_IFW1].u32Ver)){
			enSect = eFLASH_SECT_IFW1;
		}else{
			u8SectValid = 0;
		}
	}

	if(u8SectValid){
		// isp init data loading
		if(!FLASH_VER_ERROR_CHECK(stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32Ver)){
#ifdef SPI_FLASH_READ_QDMA
			// QDMA read
			spi_memctrl_read_qdma(stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32FlashAddr, (uint8 *)stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32Size);
#else
			memcpy((uint8 *)stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32LoadAddr, (uint8 *)(SPI_MEM_BASE_ADDR + stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32FlashAddr), stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32Size);
			cache_wb_range((uint32 *)stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_ISP_DATA].u32Size);
#endif

		}

		// sensor init data loading
		if(!FLASH_VER_ERROR_CHECK(stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32Ver)){
#ifdef SPI_FLASH_READ_QDMA
			// QDMA read
			spi_memctrl_read_qdma(stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32FlashAddr, (unsigned char *)stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32Size);
#else
			memcpy((uint8 *)stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32LoadAddr, (uint8 *)(SPI_MEM_BASE_ADDR + stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32FlashAddr), stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32Size);
			cache_wb_range((uint32 *)stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_SENSOR_DATA].u32Size);
#endif

		}
		
		// isp firmware loading
#ifdef SPI_FLASH_READ_QDMA
		// QDMA read
		spi_memctrl_read_qdma(stFHdr.stSect[enSect].u32FlashAddr, (uint8 *)stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
		cache_inv_range((uint32 *)stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
#else
		memcpy((uint8 *)stFHdr.stSect[enSect].u32LoadAddr, (uint8 *)(SPI_MEM_BASE_ADDR + stFHdr.stSect[enSect].u32FlashAddr), stFHdr.stSect[enSect].u32Size);
		cache_wb_range((uint32 *)stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
#endif

		// ISP core reset vector 0x20000000
		*(volatile uint32 *) SUB_CPU_RESET_VECTOR_ADDR = 0x2000;
		
		// ISP core reset disable
		*(volatile uint32 *) SUB_CPU_RESET_ADDR = 0x00000001;

		u32Result = enSect;

		LOOPDELAY_MSEC(1);
	}
	
	//=============================================================================
	// main firmware load
	//=============================================================================
	u8SectValid = 1;
	enSect = eFLASH_SECT_MFW;
	if(!FLASH_VER_ERROR_CHECK(stFHdr.stSect[eFLASH_SECT_MFW].u32Ver)){
		enSect = eFLASH_SECT_MFW;
	}else{
		if(!FLASH_VER_ERROR_CHECK(stFHdr.stSect[eFLASH_SECT_MFW1].u32Ver)){
			enSect = eFLASH_SECT_MFW1;
		}else{
			u8SectValid = 0;
		}
	}

	if(u8SectValid){
		//_delay(0x1000000);
#ifdef SPI_FLASH_READ_QDMA
		// QDMA read
		dbg("flash addr:0x%x, load addr: 0x%x, size: 0x%x\n", stFHdr.stSect[enSect].u32FlashAddr, stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
		spi_memctrl_read_qdma(stFHdr.stSect[enSect].u32FlashAddr, (uint8 *)stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
		cache_inv_range((uint32 *)stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
#else
		memcpy((uint8 *)stFHdr.stSect[enSect].u32LoadAddr, (uint8 *)(SPI_MEM_BASE_ADDR + stFHdr.stSect[enSect].u32FlashAddr), stFHdr.stSect[enSect].u32Size);
		cache_wb_range((uint32 *)stFHdr.stSect[enSect].u32LoadAddr, stFHdr.stSect[enSect].u32Size);
#endif
		
		jump_addr = stFHdr.stSect[enSect].u32LoadAddr;
		u32Result |= ((enSect&0xff)<<8);
		
	}

#if 0
	// set spi controller & mem controller to single mode

	flash_init(9281250, 0, 0);
	fr_info.dummy_size = 0;
	fr_info.mode_size = 0;
	fr_info.mode_out_state = 0;
	fr_info.mode_value = 0;
	fr_info.read_type = 0;
	spi_memctrl_set(4, 0x3, fr_info);
#endif

END_FT:

	*(volatile unsigned int *)BOOT_RESULT_ADDR = u32Result;
	// jump
	return jump_addr;
	
	
}

uint32 spi_nand_flash_boot(void)
{
	PP_FLASH_HDR_S stFHdr;
	uint32 jump_addr = (uint32)-1;
	SPI_MEM_FR_INFO fr_info;
	uint8 support_qdma;

#ifdef SPI_FLASH_READ_QDMA	
	support_qdma = 1;
#else
	support_qdma = 0;
#endif

	if(snand_flash_init(PCLK/2, SPI_MODE, 1) < 0){
		goto END_FT;
	}

	dbg("flash manufacturer: 0x%x\n", gstFlashNandID.u8ManufacturerID);

	memset(&fr_info, 0, sizeof(fr_info));

	fr_info.flash_type = SPI_FLASH_TYPE_NAND;
	fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_0;
	fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_NONE;
	fr_info.mode_value = 0;
	fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;

	if(gstFlashNandID.u8ManufacturerID == NAND_MANID_GIGADEVICE)
		fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_8;
	else // micron, winbond
		fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;

	spi_memctrl_set(QSPI_DIV, QSPI_DELAY, 0xEB, fr_info);
	spi_memctrl_mode_set(SPI_MEMC_MEMORY_MAP_MODE, 0);

	snand_flash_read_memc((uint8 *)&stFHdr, 0, sizeof(PP_FLASH_HDR_S), 0);

	if(stFHdr.u32Sign != BOOT_HEADER_SIGN)goto END_FT;
	if(stFHdr.u32FlashType != SPI_FLASH_TYPE_NAND)goto END_FT;

	if(snand_batl_init() < 0)goto END_FT;

	if(support_qdma)
		spi_memctrl_mode_set(SPI_MEMC_QDMA_MODE, 0);
	else
		spi_memctrl_mode_set(SPI_MEMC_MEMORY_MAP_MODE, 0);

	// Load secondary bootloader
	if(stFHdr.stSect[eFLASH_SECT_BL2].u32Ver != 0xffffffff && stFHdr.stSect[eFLASH_SECT_BL2].u32Ver > 0){
		//_delay(0x1000000);
		snand_flash_read_memc((uint8 *)stFHdr.stSect[eFLASH_SECT_BL2].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_BL2].u32FlashAddr, stFHdr.stSect[eFLASH_SECT_BL2].u32Size, support_qdma);
		if(!support_qdma)
			cache_wb_range((uint32 *)stFHdr.stSect[eFLASH_SECT_BL2].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_BL2].u32Size);
		else
			cache_inv_range((uint32 *)stFHdr.stSect[eFLASH_SECT_BL2].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_BL2].u32Size);

		jump_addr = stFHdr.stSect[eFLASH_SECT_BL2].u32LoadAddr;

		dbg("main fw load. load addr: 0x%x, flash addr: 0x%x, size: %d\n", stFHdr.stSect[eFLASH_SECT_BL2].u32LoadAddr, stFHdr.stSect[eFLASH_SECT_BL2].u32FlashAddr, stFHdr.stSect[eFLASH_SECT_BL2].u32Size);

	}
	
END_FT:	

	dbg("sign: 0x%x, type: 0x%x, bl2 ver: 0x%x, jump_addr: 0x%x\n", stFHdr.u32Sign, stFHdr.u32FlashType, stFHdr.stSect[eFLASH_SECT_BL2].u32Ver, jump_addr);
	return jump_addr;
	
	
}


