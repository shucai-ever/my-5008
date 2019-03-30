#ifndef _PI5008_SPI_FLASH_H
#define _PI5008_SPI_FLASH_H

#include "type.h"


#define FLASH_PAGE_SIZE				(256)
#define FLASH_ERASE_SECTOR			(4*1024)
#define FLASH_ERASE_BLOCK_32K		(32*1024)
#define FLASH_ERASE_BLOCK_64K		(64*1024)

#define NOR_MANID_MXIC			0xC2
#define NOR_MANID_ISSI			0x9D

typedef struct ppFLASH_ID_S
{
	uint8 u8ManufacturerID;
	uint8 u8DeviceID0;
	uint8 u8DeviceID1;
	uint8 u8AddrMode4Byte;
	uint32 u32TotalSize;
	uint32 u32PageSize;
	uint32 u32EraseBlockSize;
}PP_FLASH_ID_S;

sint32 flash_init(uint32 freq, uint32 clk_mode, uint8 quad_io);
sint32 flash_find_id(PP_FLASH_ID_S *pstID);
sint32 flash_read_id(uint8 *id);
sint32 flash_read_status(void);
sint32 flash_write_status(uint8 status);
sint32 flash_write_enable(uint8 enable);
sint32 flash_read(uint8 *buf, uint32 addr, sint32 size);
sint32 flash_write(uint8 *buf, uint32 addr, sint32 size);
sint32 flash_erase(uint32 addr, sint32 len, sint32 erase_size);

extern PP_FLASH_ID_S gstFlashID;

#endif

