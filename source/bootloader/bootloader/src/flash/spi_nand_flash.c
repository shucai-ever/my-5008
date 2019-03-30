#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "spi_memctrl.h"
#include "spi_nand_flash.h"
#include "common.h"
#include "cache.h"
#include "debug.h"


#define CMD_READ_ID					0x9f
#define CMD_GET_FEATURE				0x0f
#define CMD_SET_FEATURE				0x1f
#define CMD_READ_PAGE				0x13
#define CMD_READ_CACHE				0x0b

#define FEATURE_ADDR_STATUS			0xc0

//======================================================
//	Bad block management
#define NAND_BATL_ADDR				0x1000
#define SPI_NAND_FLASH_BLOCK_TOTAL	20						// size limit

typedef struct ppNAND_BATL_S{
	uint8 s8Marker[8];
	uint32 u32LUT[SPI_NAND_FLASH_BLOCK_TOTAL];
}PP_NAND_BATL_S;

static uint8 gs8BATLMarker[8] = {'B','A','T','L','M','A','R','K'};
static PP_NAND_BATL_S gstBATL;
//======================================================

PP_FLASH_NAND_ID_S gstNandFlashIDTable[] = {
		// GigaDevice
		{ .u8ManufacturerID = NAND_MANID_GIGADEVICE, .u8DeviceID0 = 0xD1, .u32TotalSize = 128*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// GigaDevice Rev.B, 128MB, 3.3V
		{ .u8ManufacturerID = NAND_MANID_GIGADEVICE, .u8DeviceID0 = 0xD2, .u32TotalSize = 256*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// GigaDevice Rev.B, 256MB, 3.3V
		// Micron
		{ .u8ManufacturerID = NAND_MANID_MICRON, .u8DeviceID0 = 0x14, .u32TotalSize = 128*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// Micron 128MB, 3.3V
		{ .u8ManufacturerID = NAND_MANID_MICRON, .u8DeviceID0 = 0x24, .u32TotalSize = 256*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// Micron 256MB, 3.3V
		// Winbond
		{ .u8ManufacturerID = NAND_MANID_WINBOND, .u8DeviceID0 = 0xAA, .u32TotalSize = 128*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// Winbond 128MB, 3.3V

};

PP_FLASH_NAND_ID_S gstFlashNandID;

extern uint8 gGlobalBuf[MAX_DATA_SIZE];

//static uint8 gTxBuf[4096 + 8];
//static uint8 gRxBuf[4096 + 8];

static uint8 *gTxBuf;
static uint8 *gRxBuf;

static uint32 gPAGE_SIZE = 2048;
static uint32 gPAGES_BLOCK = 64;
static uint32 gBLOCK_SIZE = (64*2048);
static uint32 gPageCachedFlag = 0;
static uint32 gCachedAddr = 0;


static void snand_flash_wait(void)
{
	uint8 cmd[4];
	uint8 buf[4];

	cmd[0] = CMD_GET_FEATURE;
	cmd[1] = FEATURE_ADDR_STATUS;
	cmd[2] = 0;

	do{
		spi_xfer(eSPI_CHANNEL_FLASH, cmd, buf, 3);
	}while(buf[2] & 1);

}

static sint32 read_from_cache(uint8 *buf, uint32 row_addr, uint32 col_addr, sint32 size)
{

	if(gPAGES_BLOCK & row_addr)col_addr |= (gPAGE_SIZE<<1);

	gTxBuf[0] = CMD_READ_CACHE;
	gTxBuf[1] = ((col_addr>>8)&0xff);
	gTxBuf[2] = ((col_addr>>0)&0xff);
	gTxBuf[3] = 0;


	spi_xfer(eSPI_CHANNEL_FLASH, gTxBuf, gRxBuf, size + 4);
	memcpy(buf, &gRxBuf[4], size);

	return 0;
}

static sint32 page_read(uint8 *buf, uint32 row_addr, uint32 col_addr, sint32 size)
{
	uint8 cmd[5];
	uint32 read_unit = 256;

	//=======================================================
	//	READ TO CACHE
	//=======================================================

	if(gPageCachedFlag == 0 || gCachedAddr != row_addr){

		// send read cmd
		cmd[0] = CMD_READ_PAGE;
		cmd[1] = ((row_addr>>16)&0xff);
		cmd[2] = ((row_addr>>8)&0xff);
		cmd[3] = ((row_addr>>0)&0xff);

		spi_tx(eSPI_CHANNEL_FLASH, cmd, 4);

		snand_flash_wait();

		gCachedAddr = row_addr;
		gPageCachedFlag = 1;
	}

	//=======================================================
	//	READ FROM CACHE
	//=======================================================
	while(size > 0){
		read_unit = (size > read_unit) ? 256 : size;		// spi controller max transfer cnt: 512
		read_from_cache(buf, row_addr, col_addr, read_unit);
		buf += read_unit;
		col_addr += read_unit;
		size -= read_unit;
	}

	return 0;

}

static sint32 page_read_memc(uint8 *buf, uint32 row_addr, uint32 col_addr, sint32 size, uint8 dma)
{
	uint8 cmd[5];
	uint32 read_unit = 256;
	uint8 *ptr;

	//=======================================================
	//	READ TO CACHE
	//=======================================================

	//if(gPageCachedFlag == 0 || gCachedAddr != row_addr){
	if(1){

		// send read cmd
		cmd[0] = CMD_READ_PAGE;
		cmd[1] = ((row_addr>>16)&0xff);
		cmd[2] = ((row_addr>>8)&0xff);
		cmd[3] = ((row_addr>>0)&0xff);

		spi_tx(eSPI_CHANNEL_FLASH, cmd, 4);

		snand_flash_wait();

		gCachedAddr = row_addr;
		gPageCachedFlag = 1;
	}

	//=======================================================
	//	READ FROM CACHE
	//=======================================================

	if(gPAGES_BLOCK & row_addr)col_addr |= (gPAGE_SIZE<<1);

	if(dma){
		spi_memctrl_read_qdma(col_addr, buf, size);
	}else{
		while(size > 0){
			read_unit = (size > read_unit) ? 256 : size;		// spi controller max transfer cnt: 512
			ptr = (uint8 *)(SPI_MEM_BASE_ADDR + col_addr);
			cache_inv_range((uint32 *)ptr, read_unit);
			memcpy(buf, ptr, read_unit);

			buf += read_unit;
			col_addr += read_unit;
			size -= read_unit;
		}
	}

	return 0;

}

sint32 snand_find_id(PP_FLASH_NAND_ID_S *pstID)
{
	uint8 id[4];
	uint32 i;

	// find flash manufacturer id
	snand_flash_read_id(id);
	for(i=0;i<sizeof(gstNandFlashIDTable)/sizeof(gstNandFlashIDTable[0]);i++){
		if(gstNandFlashIDTable[i].u8ManufacturerID == id[2] && gstNandFlashIDTable[i].u8DeviceID0 == id[3]){
			memcpy(pstID, &gstNandFlashIDTable[i], sizeof(PP_FLASH_NAND_ID_S));
			break;
		}
	}
	if(pstID->u8ManufacturerID == 0){
		dbg("error! unsupported flash id(0x%x)\n", id[2]);
		return -1;
	}

	return 0;
}

sint32 snand_flash_init(uint32 freq, uint32 clk_mode, uint8 quad_io)
{

	spi_initialize(eSPI_CHANNEL_FLASH, 0, freq, clk_mode, 8, NULL);

	gTxBuf = (uint8 *)gGlobalBuf;
	gRxBuf = (uint8 *)(gGlobalBuf + MAX_DATA_SIZE/2);

	gPageCachedFlag = 0;

	memset(&gstFlashNandID, 0, sizeof(gstFlashNandID));
	if(snand_find_id(&gstFlashNandID)){
		return -1;
	}
	gPAGE_SIZE = gstFlashNandID.u32PageSize;
	gPAGES_BLOCK = gstFlashNandID.u32EraseBlockSize/gstFlashNandID.u32PageSize;
	gBLOCK_SIZE = gstFlashNandID.u32EraseBlockSize;

	if(gstFlashNandID.u8ManufacturerID == NAND_MANID_GIGADEVICE){
		if(quad_io){
			uint8 feature;
			feature = snand_flash_get_feature(0xb0);
			feature |= 1;
			snand_flash_set_feature(0xb0, feature);
		}
	}

	return 0;
}

sint32 snand_flash_read_id(uint8* id)
{
	uint8 cmd[4];
	uint8 buf[4];
	sint32 size;

	size = 4;
	memset(cmd, 0, sizeof(cmd));
	cmd[0] = CMD_READ_ID;

	spi_xfer(eSPI_CHANNEL_FLASH, cmd, buf, size);

	memcpy(id, buf, 4);

	return 0;
}

sint32 snand_batl_init(void)
{
	sint32 i;

	dbg("NAND BATL Size: %d\n", (int)sizeof(gstBATL));

	for(i=0;i<SPI_NAND_FLASH_BLOCK_TOTAL;i++){
		gstBATL.u32LUT[i] = i;
	}

#if NO_BATL
	memcpy(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker));

#else

	snand_flash_read_memc((uint8 *)&gstBATL, NAND_BATL_ADDR, sizeof(gstBATL), 0);

	if(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) != 0)
		return -1;
#endif

	return 0;

}

sint32 snand_flash_get_feature(uint8 addr)
{
	uint8 cmd[3];
	uint8 feature[3];

	cmd[0] = CMD_GET_FEATURE;
	cmd[1] = addr;
	cmd[2] = 0;
	
	spi_xfer(eSPI_CHANNEL_FLASH, cmd, feature, 3);

	return (uint32)feature[2];

}

void snand_flash_set_feature(uint8 addr, uint8 feature)
{
	uint8 cmd[3];

	cmd[0] = CMD_SET_FEATURE;
	cmd[1] = addr;
	cmd[2] = feature;

	spi_tx(eSPI_CHANNEL_FLASH, cmd, 3);

}

sint32 snand_flash_read(uint8 *buf, uint32 addr, sint32 size)
{
	uint32 read_unit;
	uint32 read_size;
	uint32 row_addr;
	uint32 col_addr;
	uint32 block_addr;

	read_size = 0;

	while(read_size < size){

		row_addr = addr/gPAGE_SIZE;
		col_addr = addr - row_addr * gPAGE_SIZE;
		read_unit = ((size - read_size > (gPAGE_SIZE - col_addr)) ? (gPAGE_SIZE - col_addr) : size - read_size);

		block_addr = gstBATL.u32LUT[(row_addr/gPAGES_BLOCK)];
		row_addr = block_addr * gPAGES_BLOCK + (row_addr & (gPAGES_BLOCK-1));

		page_read(buf, row_addr, col_addr, read_unit);

		buf += read_unit;
		addr += read_unit;
		read_size += read_unit;
		
	}

	return 0;	
}

sint32 snand_flash_read_memc(uint8 *buf, uint32 addr, sint32 size, uint8 dma)
{
	uint32 read_unit;
	uint32 read_size;
	uint32 row_addr;
	uint32 col_addr;
	uint32 block_addr;

	read_size = 0;

	while(read_size < size){

		row_addr = addr/gPAGE_SIZE;
		col_addr = addr - row_addr * gPAGE_SIZE;
		read_unit = ((size - read_size > (gPAGE_SIZE - col_addr)) ? (gPAGE_SIZE - col_addr) : size - read_size);

		block_addr = gstBATL.u32LUT[(row_addr/gPAGES_BLOCK)];
		row_addr = block_addr * gPAGES_BLOCK + (row_addr & (gPAGES_BLOCK-1));

		//page_read(buf, row_addr, col_addr, read_unit);
		page_read_memc(buf, row_addr, col_addr, read_unit, dma);

		buf += read_unit;
		addr += read_unit;
		read_size += read_unit;



	}

	return 0;
}


