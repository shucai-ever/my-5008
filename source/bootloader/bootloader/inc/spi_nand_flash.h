#ifndef _PI5008_SPI_NAND_FLASH_H
#define _PI5008_SPI_NAND_FLASH_H

#include "type.h"

#define NAND_MANID_GIGADEVICE	0xC8
#define NAND_MANID_MICRON		0x2C
#define NAND_MANID_WINBOND		0xEF

typedef struct ppFLASH_NAND_ID_S
{
	uint8 u8ManufacturerID;
	uint8 u8DeviceID0;
	uint8 u8DeviceID1;
	uint8 u8Rsv;
	uint32 u32TotalSize;
	uint32 u32PageSize;
	uint32 u32EraseBlockSize;
}PP_FLASH_NAND_ID_S;

sint32 snand_flash_init(uint32 freq, uint32 clk_mode, uint8 quad_io);
sint32 snand_flash_read_id(uint8* id);
sint32 snand_batl_init(void);
sint32 snand_flash_get_feature(uint8 addr);
void snand_flash_set_feature(uint8 addr, uint8 feature);
sint32 snand_flash_read(uint8 *buf, uint32 addr, sint32 size);
sint32 snand_flash_read_memc(uint8 *buf, uint32 addr, sint32 size, uint8 dma);

extern PP_FLASH_NAND_ID_S gstFlashNandID;

#endif

