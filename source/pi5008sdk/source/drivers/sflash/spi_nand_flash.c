
#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "spi_memctrl.h"
#include "spi_nand_flash.h"
#include "cache.h"
#include "osal.h"
#include "dma.h"
#include "gpio.h"
#include "common.h"
#include "debug.h"
#include "api_FAT_FTL.h"

#define CMD_READ_ID					0x9f
#define CMD_BLOCK_ERASE				0xd8
#define CMD_GET_FEATURE				0x0f
#define CMD_SET_FEATURE				0x1F
#define CMD_LOAD_PROGRAM			0x02
#define CMD_LOAD_RANDOM				0x84
#define CMD_EXEC_PROGRAM			0x10
#define CMD_WRITE_DISABLE			0x04
#define CMD_WRITE_ENABLE			0x06
#define CMD_READ_PAGE				0x13
#define CMD_READ_CACHE				0x0b

#define FEATURE_ADDR_BP				0xa0
#define FEATURE_ADDR_CFG			0xb0
#define FEATURE_ADDR_STATUS			0xc0



#define SPI_TX_DMA				0
#define GPIO_CS_CTRL			0

static PP_U8 gu8TxBuf[4096 + 8];
static PP_U8 gu8RxBuf[4096 + 8];

static PP_U32 gu32TransferEnd = 0;

static PP_U32 gu32PageSize = 2048;
static PP_U32 gu32PagesBlock = 64;
static PP_U32 gu32BlockSize = (64*2048);
static PP_U32 gu32PageCachedFlag = 0;
static PP_U32 gu32CachedAddr = 0;


PP_FLASH_NAND_ID_S gstNandFlashIDTable[] = {
		// GigaDevice
		{ .u8ManufacturerID = NAND_MANID_GIGADEVICE, .u8DeviceID0 = 0xD1, .u8FTLDeviceID = NAND_DEVICE_GD1G2G, .u32TotalSize = 128*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// GigaDevice Rev.B, 128MB, 3.3V
		{ .u8ManufacturerID = NAND_MANID_GIGADEVICE, .u8DeviceID0 = 0xD2, .u8FTLDeviceID = NAND_DEVICE_GD1G2G, .u32TotalSize = 256*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// GigaDevice Rev.B, 256MB, 3.3V
		// Micron
		{ .u8ManufacturerID = NAND_MANID_MICRON, .u8DeviceID0 = 0x14, .u8FTLDeviceID = NAND_DEVICE_MT29FXG01, .u32TotalSize = 128*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// Micron 128MB, 3.3V
		{ .u8ManufacturerID = NAND_MANID_MICRON, .u8DeviceID0 = 0x24, .u8FTLDeviceID = NAND_DEVICE_MT29FXG01, .u32TotalSize = 256*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// Micron 256MB, 3.3V
		// Winbond
		{ .u8ManufacturerID = NAND_MANID_WINBOND, .u8DeviceID0 = 0xAA, .u8FTLDeviceID = NAND_DEVICE_W25NXG, .u32TotalSize = 128*1024*1024, .u32PageSize = 2048, .u32EraseBlockSize = 128*1024  },	// Winbond 128MB, 3.3V

};

PP_FLASH_NAND_ID_S gstFlashNandID;


static PP_VOID PPDRV_SNAND_FLASH_CallBack(PP_U32 u32Status)
{
	if(u32Status & SPI_INTR_TRANSFER_END){
		gu32TransferEnd = 1;
	}
}

static PP_VOID PPDRV_SNAND_FLASH_SetEcc(PP_U8 u8On)
{
	uint8 feature;
	uint32 retry;
	uint32 check;

	feature = PPDRV_SNAND_FLASH_GetFeature(0xb0);
	if(u8On){
		feature |= (1<<4);
	}else{
		feature &= (~(1<<4));
	}
	PPDRV_SNAND_FLASH_SetFeature(0xb0, feature);

	for(retry = 0; retry < 5; retry++){
		check = PPDRV_SNAND_FLASH_GetFeature(0xb0);
		if(check == feature)break;
	}

}

static PP_VOID PPDRV_SNAND_FLASH_Wait(PP_VOID)
{
	uint8 cmd[4];
	uint8 buf[4];

	cmd[0] = CMD_GET_FEATURE;
	cmd[1] = FEATURE_ADDR_STATUS;
	cmd[2] = 0;

	do{
#if GPIO_CS_CTRL
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
		PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, buf, 3);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	}while(buf[2] & 1);

}

static PP_RESULT_E PPDRV_SNAND_FLASH_WriteEnable(PP_U8 IN u8Enable)
{
	uint8 cmd;

	if(u8Enable)cmd = CMD_WRITE_ENABLE;
	else cmd = CMD_WRITE_DISABLE;

#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	//PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, &cmd, NULL, 1);
	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, &cmd, 1);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return eSUCCESS;

}

static PP_RESULT_E PPDRV_SNAND_FLASH_ReadFromCache(PP_U8* OUT pu8Buf, PP_U32 IN u32RowAddr, PP_U32 IN u32ColAddr, PP_S32 IN s32Size)
{

	if(gu32PagesBlock & u32RowAddr)u32ColAddr |= (gu32PageSize<<1);

	gu8TxBuf[0] = CMD_READ_CACHE;
	gu8TxBuf[1] = ((u32ColAddr>>8)&0xff);
	gu8TxBuf[2] = ((u32ColAddr>>0)&0xff);
	gu8TxBuf[3] = 0;

#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, gu8TxBuf, 4);
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);

	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
	PPDRV_SPI_Rx(eSPI_CHANNEL_FLASH, pu8Buf, s32Size);
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);

#else
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, gu8TxBuf, gu8RxBuf, s32Size + 4);
	memcpy(pu8Buf, &gu8RxBuf[4], s32Size);
#endif

	return eSUCCESS;
}

static PP_RESULT_E PPDRV_SNAND_FLASH_PageRead(PP_U8* OUT pu8Buf, PP_U32 IN u32RowAddr, PP_U32 u32ColAddr, PP_S32 s32Size)
{
	uint8 cmd[5];
	uint32 read_unit = 256;

	//=======================================================
	//	READ TO CACHE
	//=======================================================

	//if(gu32PageCachedFlag == 0 || gu32CachedAddr != u32RowAddr){
	if(1){

		// send read cmd
		cmd[0] = CMD_READ_PAGE;
		cmd[1] = ((u32RowAddr>>16)&0xff);
		cmd[2] = ((u32RowAddr>>8)&0xff);
		cmd[3] = ((u32RowAddr>>0)&0xff);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
		PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 4);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

		PPDRV_SNAND_FLASH_Wait();

		gu32CachedAddr = u32RowAddr;
		gu32PageCachedFlag = 1;
	}

	//=======================================================
	//	READ FROM CACHE
	//=======================================================
#if 0
	// set plane select bit
	if(gu32PagesBlock & u32RowAddr)u32ColAddr |= (gu32PageSize<<1);

	cmd[0] = CMD_READ_CACHE;
	cmd[1] = ((u32ColAddr>>8)&0xff);
	cmd[2] = ((u32ColAddr>>0)&0xff);
	cmd[3] = 0;

	memcpy(gu8TxBuf, cmd, 4);

	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, gu8TxBuf, gu8RxBuf, s32Size + 4);
	memcpy(pu8Buf, &gu8RxBuf[4], s32Size);
#else
	while(s32Size > 0){
		read_unit = (s32Size > 256) ? 256 : s32Size;		// spi controller max transfer cnt: 512
		PPDRV_SNAND_FLASH_ReadFromCache(pu8Buf, u32RowAddr, u32ColAddr, read_unit);
		pu8Buf += read_unit;
		u32ColAddr += read_unit;
		s32Size -= read_unit;
	}

#endif
	return eSUCCESS;

}

static PP_RESULT_E PPDRV_SNAND_FLASH_PageReadMemc(PP_U8 OUT *pu8Buf, PP_U32 IN u32RowAddr, PP_U32 IN u32ColAddr, PP_S32 IN s32Size, PP_U8 IN u8DMA)
{
	uint8 cmd[5];
	uint32 read_unit = 256;
//	uint8 *ptr;

	//=======================================================
	//	READ TO CACHE
	//=======================================================

	//if(gu32PageCachedFlag == 0 || gu32CachedAddr != u32RowAddr){
	if(1){

		// send read cmd
		cmd[0] = CMD_READ_PAGE;
		cmd[1] = ((u32RowAddr>>16)&0xff);
		cmd[2] = ((u32RowAddr>>8)&0xff);
		cmd[3] = ((u32RowAddr>>0)&0xff);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

		PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 4);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

		PPDRV_SNAND_FLASH_Wait();

		gu32CachedAddr = u32RowAddr;
		gu32PageCachedFlag = 1;
	}

	//=======================================================
	//	READ FROM CACHE
	//=======================================================
#if GPIO_CS_CTRL
	PPDRV_SPI_GPIOCSEnable(eSPI_CHANNEL_FLASH, 0);
#endif

	if(gu32PagesBlock & u32RowAddr)u32ColAddr |= (gu32PageSize<<1);

	if(u8DMA){
		spi_memctrl_read_qdma(u32ColAddr, pu8Buf, s32Size);
	}else{
		while(s32Size > 0){
			read_unit = (s32Size > read_unit) ? 256 : s32Size;		// spi controller max transfer cnt: 512
			//ptr = (uint8 *)(SPI_MEM_BASE_ADDR + u32ColAddr);
			//cache_inv_range((uint32*)ptr, read_unit);
			//memcpy(pu8Buf, ptr, read_unit);
			spi_memctrl_read(u32ColAddr, pu8Buf, read_unit);

			pu8Buf += read_unit;
			u32ColAddr += read_unit;
			s32Size -= read_unit;
		}
	}
#if GPIO_CS_CTRL
	PPDRV_SPI_GPIOCSEnable(eSPI_CHANNEL_FLASH, 1);
#endif

	return eSUCCESS;

}


static PP_RESULT_E PPDRV_SNAND_FLASH_LoadToCache(PP_U8* IN pu8Buf, PP_U32 IN u32RowAddr, PP_U32 IN u32ColAddr, PP_S32 IN s32Size, PP_U8 u8DMAFlag)
{
	PP_U8 *pu8TxPtr;

	if(gu32PagesBlock & u32RowAddr)u32ColAddr |= (gu32PageSize<<1);

	pu8TxPtr = (PP_U8 *)(((PP_U32)gu8TxBuf + 0xf) & (~0xf));

	//gu8TxBuf[0] = CMD_LOAD_PROGRAM;
	pu8TxPtr[0] = CMD_LOAD_RANDOM;
	pu8TxPtr[1] = ((u32ColAddr>>8)&0xff);
	pu8TxPtr[2] = ((u32ColAddr>>0)&0xff);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, pu8TxPtr, 3);
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);


	if(u8DMAFlag){
		cache_wb_range(pu8Buf, s32Size);
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
		if(PPDRV_SPI_TxDMA(eSPI_CHANNEL_FLASH, pu8Buf, s32Size, eDMA_CHANNEL_FLASH_TX, 5000)){
			LOG_DEBUG("[%s]Write DMA Timeout\n", __FUNCTION__);
		}

		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
	}else{
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
		PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, pu8Buf, s32Size);
		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
	}
#else

	memcpy(&pu8TxPtr[3], pu8Buf, s32Size);
	if(u8DMAFlag){
		cache_wb_range(pu8TxPtr, 3+s32Size);
		if(PPDRV_SPI_TxDMA(eSPI_CHANNEL_FLASH, pu8TxPtr, 3 + s32Size, eDMA_CHANNEL_FLASH_TX, 5000)){
			LOG_DEBUG("[%s]Write DMA Timeout\n", __FUNCTION__);
		}
	}else{
		PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, pu8TxPtr, 3 + s32Size);
	}
#endif

	return 0;
}

static PP_RESULT_E PPDRV_SNAND_FLASH_PageWrite(PP_U8* IN pu8Buf, PP_U32 u32RowAddr, PP_U32 u32ColAddr, PP_S32 s32Size, PP_U8 u8DMAFlag)
{
	uint8 cmd[5];
	uint32 write_unit = 256;

	//=======================================================
	//	READ TO CACHE
	//=======================================================
	//if(gu32PageCachedFlag == 0 || gu32CachedAddr != u32RowAddr){
	if(1){

		// send read cmd
		cmd[0] = CMD_READ_PAGE;
		cmd[1] = ((u32RowAddr>>16)&0xff);
		cmd[2] = ((u32RowAddr>>8)&0xff);
		cmd[3] = ((u32RowAddr>>0)&0xff);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

		PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 4);
#if GPIO_CS_CTRL
		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

		PPDRV_SNAND_FLASH_Wait();

		gu32CachedAddr = u32RowAddr;
		gu32PageCachedFlag = 1;
	}


	//=======================================================
	//	LOAD TO CACHE
	//=======================================================

	PPDRV_SNAND_FLASH_WriteEnable(1);

	while(s32Size > 0){
		write_unit = (s32Size > write_unit) ? 256 : s32Size;		// spi controller max transfer cnt: 512
		PPDRV_SNAND_FLASH_LoadToCache(pu8Buf, u32RowAddr, u32ColAddr, write_unit, u8DMAFlag);
		pu8Buf += write_unit;
		u32ColAddr += write_unit;
		s32Size -= write_unit;
	}

	gu32PageCachedFlag = 0;
	//=======================================================
	//	PROGRAM EXEC (WRITE TO ARRAY)
	//=======================================================
	cmd[0] = CMD_EXEC_PROGRAM;
	cmd[1] = ((u32RowAddr>>16)&0xff);
	cmd[2] = ((u32RowAddr>>8)&0xff);
	cmd[3] = ((u32RowAddr>>0)&0xff);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 4);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif


	PPDRV_SNAND_FLASH_Wait();

	PPDRV_SNAND_FLASH_WriteEnable(0);

	return 0;

}




static PP_RESULT_E PPDRV_SNAND_FLASH_FindID(PP_FLASH_NAND_ID_S *pstID)
{
	uint8 id[8];
	uint32 i;

	// find flash manufacturer id
	PPDRV_SNAND_FLASH_ReadID(id);
	for(i=0;i<sizeof(gstNandFlashIDTable)/sizeof(gstNandFlashIDTable[0]);i++){
		if(gstNandFlashIDTable[i].u8ManufacturerID == id[2] && gstNandFlashIDTable[i].u8DeviceID0 == id[3]){
			memcpy(pstID, &gstNandFlashIDTable[i], sizeof(PP_FLASH_NAND_ID_S));
			break;
		}
	}
	if(pstID->u8ManufacturerID == 0){
		LOG_DEBUG("error! unsupported flash id(0x%x)\n", id[2]);
		return eERROR_FAILURE;
	}

	LOG_DEBUG("Serial NAND Flash. Id: 0x%x, 0x%x, 0x%x, 0x%x\n", id[0], id[1], id[2], id[3]);
	LOG_DEBUG("Flash size: 0x%x, Page size: 0x%x, Block size: 0x%x\n", pstID->u32TotalSize, pstID->u32PageSize, pstID->u32EraseBlockSize );
	return eSUCCESS;
}

//PP_RESULT_E PPDRV_SNAND_FLASH_Initialize(PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U8 IN u8QuadIO, PP_U32 IN u32PageSize, PP_U32 IN u32PagesBlock)
PP_RESULT_E PPDRV_SNAND_FLASH_Initialize(PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U8 IN u8QuadIO)
{
	sint32 value;

	PP_UNUSED_ARG(value);

	PPDRV_SPI_Initialize(eSPI_CHANNEL_FLASH, 0, u32Freq, u32ClkMode, 8);
	PPDRV_SPI_SetISR(eSPI_CHANNEL_FLASH, PPDRV_SNAND_FLASH_CallBack);

	gu32PageCachedFlag = 0;

#if GPIO_CS_CTRL
	PPDRV_SPI_GPIOCSEnable(eSPI_CHANNEL_FLASH, 1);
#endif
	memset(&gstFlashNandID, 0, sizeof(gstFlashNandID));
	if(PPDRV_SNAND_FLASH_FindID(&gstFlashNandID)){
		return eERROR_FAILURE;

	}

	gu32PageSize = gstFlashNandID.u32PageSize;
	gu32PagesBlock = gstFlashNandID.u32EraseBlockSize / gstFlashNandID.u32PageSize;
	gu32BlockSize = gstFlashNandID.u32EraseBlockSize;

	if(gstFlashNandID.u8ManufacturerID == NAND_MANID_GIGADEVICE){
		if(u8QuadIO){
			uint8 feature;
			feature = PPDRV_SNAND_FLASH_GetFeature(0xb0);
			feature |= 1;
			PPDRV_SNAND_FLASH_SetFeature(0xb0, feature);
		}
	}
	LOG_DEBUG("snand get feature(0xa0): 0x%x\n", PPDRV_SNAND_FLASH_GetFeature(0xa0));
	LOG_DEBUG("snand get feature(0xb0): 0x%x\n", PPDRV_SNAND_FLASH_GetFeature(0xb0));
	LOG_DEBUG("snand get feature(0xc0): 0x%x\n", PPDRV_SNAND_FLASH_GetFeature(0xc0));

#if 0	//NAND_ECC_OFF
	value = PPDRV_SNAND_FLASH_GetFeature(0xb0);
	value &= ~(1<<4);
	PPDRV_SNAND_FLASH_SetFeature(0xb0, value);

#endif


	return eSUCCESS;
}

PP_RESULT_E PPDRV_SNAND_FLASH_ReadID(PP_U8* OUT pu8ID)
{
	uint8 cmd[4];
	uint8 buf[4];
	sint32 size;

	size = 4;
	memset(cmd, 0, sizeof(cmd));
	cmd[0] = CMD_READ_ID;
#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, buf, size);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	memcpy(pu8ID, buf, 4);

	return eSUCCESS;
}

PP_U8 PPDRV_SNAND_FLASH_GetFeature(PP_U8 IN u8Addr)
{
	uint8 cmd[3];
	uint8 feature[3];

	cmd[0] = CMD_GET_FEATURE;
	cmd[1] = u8Addr;
	cmd[2] = 0;
#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, feature, 3);
#if GPIO_CS_CTRL
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return feature[2];

}

PP_RESULT_E PPDRV_SNAND_FLASH_SetFeature(PP_U8 IN u8Addr, PP_U8 IN u8Feature)
{
	uint8 cmd[3];
	
	cmd[0] = CMD_SET_FEATURE;
	cmd[1] = u8Addr;
	cmd[2] = u8Feature;
#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 3);
#if GPIO_CS_CTRL
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return eSUCCESS;

}


PP_RESULT_E PPDRV_SNAND_FLASH_ReadPage(PP_U8* OUT pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32ColAddr, PP_U32 IN u32Size)
{
	PP_U32 u32RowAddr;

	u32RowAddr = u32BlockAddr * gu32PagesBlock + u32PageAddr;

	PPDRV_SNAND_FLASH_PageRead(pu8Buf, u32RowAddr, u32ColAddr, u32Size);

	return eSUCCESS;
}

PP_RESULT_E PPDRV_SNAND_FLASH_ReadSpare(PP_U8* OUT pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Offset, PP_U32 IN u32Size)
{
	PP_U32 u32RowAddr;
	PP_U32 u32ColAddr;

	u32RowAddr = u32BlockAddr * gu32PagesBlock + u32PageAddr;
	u32ColAddr = gu32PageSize + u32Offset;

	//PPDRV_SNAND_FLASH_PageRead(buf, row_addr, col_addr, size);
	PPDRV_SNAND_FLASH_PageReadMemc(pu8Buf, u32RowAddr, u32ColAddr, u32Size, 0);
	return 0;
}


PP_RESULT_E PPDRV_SNAND_FLASH_ReadPageMemc(PP_U8* OUT pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32ColAddr, PP_U32 IN u32Size, PP_U8 IN u8DMA)
{
	PP_U32 u32RowAddr;

	u32RowAddr = u32BlockAddr * gu32PagesBlock + u32PageAddr;
	PPDRV_SNAND_FLASH_PageReadMemc(pu8Buf, u32RowAddr, u32ColAddr, u32Size, u8DMA);

	return eSUCCESS;
}


/*
sint32 snand_flash_write(uint8 *buf, uint32 addr, sint32 size)
{
	uint32 write_unit;
	uint32 written_size;
	uint32 row_addr;
	uint32 col_addr;


	written_size = 0;

	while(written_size < size){

		row_addr = addr/gu32PageSize;
		col_addr = addr - row_addr * gu32PageSize;
		write_unit = ((size - written_size > (gu32PageSize - col_addr)) ? (gu32PageSize - col_addr) : size - written_size);

		page_write(buf, row_addr, col_addr, write_unit);

		buf += write_unit;
		addr += write_unit;
		written_size += write_unit;

	}


	return 0;

}
*/
PP_RESULT_E PPDRV_SNAND_FLASH_WritePage(PP_U8* IN pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Size)
{
	PP_U32 u32RowAddr;

	u32RowAddr = u32BlockAddr * gu32PagesBlock + u32PageAddr;

	PPDRV_SNAND_FLASH_PageWrite(pu8Buf, u32RowAddr, 0, u32Size, 0);


	return eSUCCESS;

}

PP_RESULT_E PPDRV_SNAND_FLASH_WriteSpare(PP_U8* IN pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Offset, PP_U32 IN u32Size)
{
	PP_U32 u32RowAddr;
	PP_U32 u32ColAddr;

	u32RowAddr = u32BlockAddr * gu32PagesBlock + u32PageAddr;
	u32ColAddr = gu32PageSize + u32Offset;

	PPDRV_SNAND_FLASH_PageWrite(pu8Buf, u32RowAddr, u32ColAddr, u32Size, 0);

	return eSUCCESS;
}

PP_RESULT_E PPDRV_SNAND_FLASH_WritePageDMA(PP_U8* IN pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Size)
{
	PP_U32 u32RowAddr;

	u32RowAddr = u32BlockAddr * gu32PagesBlock + u32PageAddr;

	PPDRV_SNAND_FLASH_PageWrite(pu8Buf, u32RowAddr, 0, u32Size, 1);

	return eSUCCESS;
}

PP_RESULT_E PPDRV_SNAND_FLASH_EraseBlock(PP_U32 u32BlockAddr)
{
	PP_U32 u32RowAddr;
	PP_U8 u8Cmd[4];


	u32RowAddr = u32BlockAddr * gu32PagesBlock;
	//row_addr = addr/gu32PageSize;

	PPDRV_SNAND_FLASH_WriteEnable(1);

	u8Cmd[0] = CMD_BLOCK_ERASE;
	u8Cmd[1] = ((u32RowAddr>>16)&0xff);
	u8Cmd[2] = ((u32RowAddr>>8)&0xff);
	u8Cmd[3] = ((u32RowAddr>>0)&0xff);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	//PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, NULL, 4);
	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, u8Cmd, 4);

#if GPIO_CS_CTRL
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	PPDRV_SNAND_FLASH_Wait();

	PPDRV_SNAND_FLASH_WriteEnable(0);

	gu32PageCachedFlag = 0;

	return eSUCCESS;
}

PP_VOID PPDRV_SNAND_FLASH_UnlockBlock(PP_VOID)
{
	PPDRV_SNAND_FLASH_SetFeature(0xa0, 0);
}

PP_U8 PPDRV_SNAND_FLASH_CheckBad(PP_U32 u32BlockAddr)
{
	PP_U8 u8Buf[4];
	PP_U8 u8Ret;

	PPDRV_SNAND_FLASH_SetEcc(0);
	PPDRV_SNAND_FLASH_ReadSpare(u8Buf, u32BlockAddr, 0, 0, 4);

	if(u8Buf[0] != 0xff){
		u8Ret = eERROR_FAILURE;
	}else{
		u8Ret = eSUCCESS;
	}
	PPDRV_SNAND_FLASH_SetEcc(1);

	return u8Ret;
}
