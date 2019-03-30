
#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "spi_flash.h"
#include "cache.h"
#include "dma.h"
#include "debug.h"
//#define SPI_DIRECT_IO_SUPPORT	1

#define CMD_READ_ID					0x9f

#define CMD_ERASE_4K				0x20
#define CMD_ERASE_32K				0x52
#define CMD_ERASE_64K				0xd8

#define CMD_WRITE_STATUS			0x01
#define CMD_PAGE_PROGRAM			0x02
#define CMD_WRITE_DISABLE			0x04
#define CMD_READ_STATUS				0x05
#define CMD_READ_CONFIG				0x15
#define CMD_WRITE_ENABLE			0x06
#define CMD_PAGE_PROGRAM_QUAD_IO	0x38

#define CMD_READ_SLOW				0x03
#define CMD_READ_FAST				0x0b
#define CMD_READ_DUAL_IO			0xbb
#define CMD_READ_QUAD_IO			0xeb

// CMD 4BYTE Address
#define CMD_READ_SLOW_4B			0x13
#define CMD_READ_FAST_4B			0x0c
#define CMD_PAGE_PROGRAM_4B			0x12

#define CMD_ERASE_4K_4B				0x21
#define CMD_ERASE_32K_4B			0x5c
#define CMD_ERASE_64K_4B			0xdc


#define FLASH_ERASE_SECTOR			(4*1024)
#define FLASH_ERASE_BLOCK_32K		(32*1024)
#define FLASH_ERASE_BLOCK_64K		(64*1024)


PP_FLASH_ID_S gstFlashIDTable[] = {
		// MXIC
		{ .u8ManufacturerID = NOR_MANID_MXIC, .u8DeviceID0 = 0x20, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// MXIC, 3.3V
		{ .u8ManufacturerID = NOR_MANID_MXIC, .u8DeviceID0 = 0x25, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// MXIC, 1.8V
		// ISSI
		{ .u8ManufacturerID = NOR_MANID_ISSI, .u8DeviceID0 = 0x60, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// ISSI, 3.3V
		{ .u8ManufacturerID = NOR_MANID_ISSI, .u8DeviceID0 = 0x70, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// ISSI, 1.8V

};

PP_FLASH_ID_S gstFlashID;

static uint8 gTxBuf[512 + 8];
static uint8 gRxBuf[512 + 8];

static uint32 gTransferEnd = 0;

static void sf_callback(uint32 status)
{
	if(status & SPI_INTR_TRANSFER_END){
		gTransferEnd = 1;
	}
}

static sint32 flash_cmd_wait_ready(uint32 timeout)
{
	sint32 status;
	
	do{
		status = PPDRV_FLASH_GetStatus();
		//printf("status: 0x%x\n", status);
	}while(status & 1);

	return 1;

}

static sint32 flash_page_read(uint8 *buf, uint32 addr, sint32 size)
{
	uint8 cmd[5];

	cmd[0] = CMD_READ_FAST;
	cmd[1] = ((addr>>16)&0xff);
	cmd[2] = ((addr>>8)&0xff);
	cmd[3] = ((addr>>0)&0xff);
	cmd[4] = 0; // dummy

	memcpy(gTxBuf, cmd, 5);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, gTxBuf, gRxBuf, size + 5);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	memcpy(buf, &gRxBuf[5], size);

	return 0;

}


static sint32 flash_page_read_4B(uint8 *buf, uint32 addr, sint32 size)
{
	uint8 cmd[6];

	cmd[0] = CMD_READ_FAST_4B;
	cmd[1] = ((addr>>24)&0xff);
	cmd[2] = ((addr>>16)&0xff);
	cmd[3] = ((addr>>8)&0xff);
	cmd[4] = ((addr>>0)&0xff);
	cmd[5] = 0; // dummy

	memcpy(gTxBuf, cmd, 6);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, gTxBuf, gRxBuf, size + 6);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	memcpy(buf, &gRxBuf[6], size);

	return 0;

}

static sint32 flash_write_enable(uint8 enable)
{
	uint8 cmd;

	if(enable)cmd = CMD_WRITE_ENABLE;
	else cmd = CMD_WRITE_DISABLE;

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	//PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, &cmd, NULL, 1);
	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, &cmd, 1);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return 0;

}

static sint32 flash_page_program(uint8 *buf, uint32 addr, sint32 size)
{
	uint8 cmd[4];

	cmd[0] = CMD_PAGE_PROGRAM;
	cmd[1] = ((addr>>16)&0xff);
	cmd[2] = ((addr>>8)&0xff);
	cmd[3] = ((addr>>0)&0xff);

	memcpy(gTxBuf, cmd, 4);
	memcpy(&gTxBuf[4], buf, size);
	size += 4;

	flash_write_enable(1);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, gTxBuf, size);

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	flash_cmd_wait_ready(1);

	flash_write_enable(0);

	return 0;

}

static sint32 flash_page_program_dma(uint8 *buf, uint32 addr, sint32 size)
{
	uint8 cmd[4];

	cmd[0] = CMD_PAGE_PROGRAM;
	cmd[1] = ((addr>>16)&0xff);
	cmd[2] = ((addr>>8)&0xff);
	cmd[3] = ((addr>>0)&0xff);

	memcpy(gTxBuf, cmd, 4);
	memcpy(&gTxBuf[4], buf, size);
	size += 4;

	flash_write_enable(1);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	gTransferEnd = 0;
	cache_wb_range((uint32 *)gTxBuf, size);
//	PPDRV_SPI_TxDMA(eSPI_CHANNEL_FLASH, gTxBuf, size, eDMA_CHANNEL_FLASH_TX, 0);
//	while(PPDRV_SPI_DMAGetDone(eSPI_CHANNEL_FLASH, eDMA_CHANNEL_FLASH_TX) != eSUCCESS);
	if(PPDRV_SPI_TxDMA(eSPI_CHANNEL_FLASH, gTxBuf, size, eDMA_CHANNEL_FLASH_TX, 5000)){
		LOG_DEBUG("[%s]Write DMA Timeout\n", __FUNCTION__);
	}

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	flash_cmd_wait_ready(1);

	flash_write_enable(0);

	return 0;

}

static sint32 flash_page_program_4B(uint8 *buf, uint32 addr, sint32 size)
{
	uint8 cmd[5];

	cmd[0] = CMD_PAGE_PROGRAM_4B;
	cmd[1] = ((addr>>24)&0xff);
	cmd[2] = ((addr>>16)&0xff);
	cmd[3] = ((addr>>8)&0xff);
	cmd[4] = ((addr>>0)&0xff);

	memcpy(gTxBuf, cmd, 5);
	memcpy(&gTxBuf[5], buf, size);
	size += 5;

	flash_write_enable(1);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, gTxBuf, size);

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	flash_cmd_wait_ready(1);

	flash_write_enable(0);

	return 0;

}

static sint32 flash_page_program_dma_4B(uint8 *buf, uint32 addr, sint32 size)
{
	uint8 cmd[5];

	cmd[0] = CMD_PAGE_PROGRAM_4B;
	cmd[1] = ((addr>>24)&0xff);
	cmd[2] = ((addr>>16)&0xff);
	cmd[3] = ((addr>>8)&0xff);
	cmd[4] = ((addr>>0)&0xff);

	memcpy(gTxBuf, cmd, 5);
	memcpy(&gTxBuf[5], buf, size);
	size += 5;

	flash_write_enable(1);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	gTransferEnd = 0;
	cache_wb_range((uint32 *)gTxBuf, size);
//	PPDRV_SPI_TxDMA(eSPI_CHANNEL_FLASH, gTxBuf, size, eDMA_CHANNEL_FLASH_TX, 0);
//	while(PPDRV_SPI_DMAGetDone(eSPI_CHANNEL_FLASH, eDMA_CHANNEL_FLASH_TX) != eSUCCESS);
	if(PPDRV_SPI_TxDMA(eSPI_CHANNEL_FLASH, gTxBuf, size, eDMA_CHANNEL_FLASH_TX, 5000)){
		LOG_DEBUG("[%s]Write DMA Timeout\n", __FUNCTION__);
	}

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	flash_cmd_wait_ready(1);

	flash_write_enable(0);

	return 0;

}

static PP_RESULT_E PPDRV_FLASH_FindID(PP_FLASH_ID_S *pstID)
{
	uint8 id[8];
	uint32 i;

	// find flash manufacturer id
	PPDRV_FLASH_ReadID(id);
	for(i=0;i<sizeof(gstFlashIDTable)/sizeof(gstFlashIDTable[0]);i++){
		if(gstFlashIDTable[i].u8ManufacturerID == id[1] && gstFlashIDTable[i].u8DeviceID0 == id[2]){
			memcpy(pstID, &gstFlashIDTable[i], sizeof(PP_FLASH_ID_S));
			pstID->u32TotalSize = 1024 * 1024 * (1<<((id[3]&0xf)-1)) / 8;
			if(pstID->u32TotalSize >= (32*1024*1024)){
				pstID->u8AddrMode4Byte = 1;
			}else{
				pstID->u8AddrMode4Byte = 0;
			}
			break;
		}
	}
	if(pstID->u8ManufacturerID == 0){
		LOG_DEBUG("error! unsupported flash id(0x%x)\n", id[1]);
		return eERROR_FAILURE;
	}

	LOG_DEBUG("Serial NAND Flash. Id: 0x%x, 0x%x, 0x%x, 0x%x\n", id[0], id[1], id[2], id[3]);
	LOG_DEBUG("Flash size: 0x%x, Page size: 0x%x, Block size: 0x%x\n", pstID->u32TotalSize, pstID->u32PageSize, pstID->u32EraseBlockSize );
	return eSUCCESS;
}

PP_RESULT_E PPDRV_FLASH_Initialize(PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U8 IN u8QuadIO)
{
	sint32 status;
	
	PPDRV_SPI_Initialize(eSPI_CHANNEL_FLASH, 0, u32Freq, u32ClkMode, 8);
	PPDRV_SPI_SetISR(eSPI_CHANNEL_FLASH, sf_callback);

#if defined(SPI_DIRECT_IO_SUPPORT)
	if(PPDRV_SPI_GPIOCSEnable(eSPI_CHANNEL_FLASH, 1) < 0)printf("Direct IO not support\n");
#endif

	memset(&gstFlashID, 0, sizeof(gstFlashID));
	if(PPDRV_FLASH_FindID(&gstFlashID)){
		return eERROR_FAILURE;

	}

	status = PPDRV_FLASH_GetStatus();
	if( ((status>>6)&1) != u8QuadIO){	// need quad mode set
		flash_write_enable(1);
		do{
			status = PPDRV_FLASH_GetStatus();
		}while(!(status & 2));

		// set quad mode
		if(u8QuadIO)
			status |= (1<<6);
		else
			status &= (~(1<<6));

		PPDRV_FLASH_SetStatus(status);
		flash_cmd_wait_ready(1);

		flash_write_enable(0);
	}

	return eSUCCESS;
}

PP_VOID PPDRV_FLASH_WriteProtect(PP_U8 IN u8On)
{
	sint32 status;

	flash_write_enable(1);
	do{
		status = PPDRV_FLASH_GetStatus();
	}while(!(status & 2));

	status = PPDRV_FLASH_GetStatus();
	if(u8On){
		status |= (0xf<<2);
	}else{
		status &= (~(0xf<<2));
	}

	PPDRV_FLASH_SetStatus(status);
	flash_cmd_wait_ready(1);

	flash_write_enable(0);

}

PP_RESULT_E PPDRV_FLASH_ReadID(PP_U8* OUT pu8ID)
{
	uint8 cmd[4];
	uint8 buf[4];
	sint32 size;

	size = 4;
	memset(cmd, 0, sizeof(cmd));
	cmd[0] = CMD_READ_ID;
	
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, buf, size);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	memcpy(pu8ID, buf, 4);

	return eSUCCESS;
}

PP_U8 PPDRV_FLASH_GetStatus(PP_VOID)
{
	uint8 cmd[2];
	uint8 status[2];
	
	cmd[0] = CMD_READ_STATUS;
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, status, 2);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return status[1];

}

PP_RESULT_E PPDRV_FLASH_SetStatus(PP_U8 IN u8Status)
{
	uint8 cmd[2];
	
	cmd[0] = CMD_WRITE_STATUS;
	cmd[1] = u8Status;

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	//PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, NULL, 2);
	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 2);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return eSUCCESS;

}

PP_U8 PPDRV_FLASH_GetConfig(PP_VOID)
{
	uint8 cmd[2];
	uint8 config[2];

	cmd[0] = CMD_READ_CONFIG;
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
	PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, config, 2);
#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return config[1];


}

PP_RESULT_E PPDRV_FLASH_SetConfig(PP_U8 IN u8Config)
{
	uint8 cmd[4];

	cmd[0] = CMD_WRITE_STATUS;
	cmd[1] = (uint8)PPDRV_FLASH_GetStatus();
	cmd[2] = u8Config;

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif

	PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, 3);

#if defined(SPI_DIRECT_IO_SUPPORT)
	PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif

	return eSUCCESS;
}


PP_RESULT_E PPDRV_FLASH_Read(PP_U8* OUT pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	uint32 read_unit;
	uint32 read_size;

	read_size = 0;
	read_unit = ((s32Size > gstFlashID.u32PageSize) ? gstFlashID.u32PageSize : s32Size);


	while(read_size < s32Size){

		//printf("[%s]write size: %d, written: %d, total size: %d\n",__FUNCTION__, write_unit, written_size, size);	

		read_unit = (((s32Size - read_size) > gstFlashID.u32PageSize) ? gstFlashID.u32PageSize : (s32Size - read_size));
		if(read_unit > (gstFlashID.u32PageSize - (u32Addr % gstFlashID.u32PageSize)))read_unit = (gstFlashID.u32PageSize - (u32Addr % gstFlashID.u32PageSize));

		if(gstFlashID.u8AddrMode4Byte){
			flash_page_read_4B(pu8Buf, u32Addr, read_unit);
		}else{
			flash_page_read(pu8Buf, u32Addr, read_unit);
		}
		pu8Buf += read_unit;
		u32Addr += read_unit;
		read_size += read_unit;
		
	}

	return 0;	
}


PP_RESULT_E PPDRV_FLASH_Write(PP_U8* IN pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	uint32 write_unit;
	uint32 written_size;

	written_size = 0;
	write_unit = ((s32Size > gstFlashID.u32PageSize) ? gstFlashID.u32PageSize : s32Size);


	while(written_size < s32Size){

		//printf("[%s]write size: %d, written: %d, total size: %d\n",__FUNCTION__, write_unit, written_size, size);	

		write_unit = (((s32Size - written_size) > gstFlashID.u32PageSize) ? gstFlashID.u32PageSize : (s32Size - written_size));
		if(write_unit > (gstFlashID.u32PageSize - (u32Addr % gstFlashID.u32PageSize)))write_unit = (gstFlashID.u32PageSize - (u32Addr % gstFlashID.u32PageSize));
		if(gstFlashID.u8AddrMode4Byte){
			flash_page_program_4B(pu8Buf, u32Addr, write_unit);
		}else{
			flash_page_program(pu8Buf, u32Addr, write_unit);
		}
		pu8Buf += write_unit;
		u32Addr += write_unit;
		written_size += write_unit;
		
	}

	return eSUCCESS;
}

PP_RESULT_E PPDRV_FLASH_WriteDMA(PP_U8* IN pu8Buf, PP_U32 IN u32Addr, PP_S32 s32Size)
{
	uint32 write_unit;
	uint32 written_size;

	written_size = 0;
	write_unit = ((s32Size > gstFlashID.u32PageSize) ? gstFlashID.u32PageSize : s32Size);


	while(written_size < s32Size){

		//printf("[%s]write size: %d, written: %d, total size: %d\n",__FUNCTION__, write_unit, written_size, size);

		write_unit = (((s32Size - written_size) > gstFlashID.u32PageSize) ? gstFlashID.u32PageSize : (s32Size - written_size));
		if(write_unit > (gstFlashID.u32PageSize - (u32Addr % gstFlashID.u32PageSize)))write_unit = (gstFlashID.u32PageSize - (u32Addr % gstFlashID.u32PageSize));
		if(gstFlashID.u8AddrMode4Byte){
			flash_page_program_dma_4B(pu8Buf, u32Addr, write_unit);
		}else{
			flash_page_program_dma(pu8Buf, u32Addr, write_unit);
		}
		pu8Buf += write_unit;
		u32Addr += write_unit;
		written_size += write_unit;

	}

	return eSUCCESS;
}

PP_RESULT_E PPDRV_FLASH_Erase(PP_U32 IN u32Addr, PP_S32 IN s32Len, PP_S32 IN s32EraseUnitSize)
{
	uint8 cmd[8];
	uint32 cmd_size = 4;

	if(gstFlashID.u8AddrMode4Byte){
		if(s32EraseUnitSize == FLASH_ERASE_SECTOR)
			cmd[0] = CMD_ERASE_4K_4B;
		else if(s32EraseUnitSize == FLASH_ERASE_BLOCK_32K)
			cmd[0] = CMD_ERASE_32K_4B;
		else if(s32EraseUnitSize == FLASH_ERASE_BLOCK_64K)
			cmd[0] = CMD_ERASE_64K_4B;
		else
			return -1;

		cmd_size = 5;
	}else{
		if(s32EraseUnitSize == FLASH_ERASE_SECTOR)
			cmd[0] = CMD_ERASE_4K;
		else if(s32EraseUnitSize == FLASH_ERASE_BLOCK_32K)
			cmd[0] = CMD_ERASE_32K;
		else if(s32EraseUnitSize == FLASH_ERASE_BLOCK_64K)
			cmd[0] = CMD_ERASE_64K;
		else
			return -1;

		cmd_size = 4;
	}

	if(u32Addr % s32EraseUnitSize || s32Len % s32EraseUnitSize){
		printf("Warning. Erase address/length not multiple of erase size\n");
	}
	

	while(s32Len > 0){
		//printf("[%s]addr: 0x%x, erase size: %d\n",__FUNCTION__, addr, erase_size);
		if(gstFlashID.u8AddrMode4Byte){
			cmd[1] = ((u32Addr>>24)&0xff);
			cmd[2] = ((u32Addr>>16)&0xff);
			cmd[3] = ((u32Addr>>8)&0xff);
			cmd[4] = ((u32Addr>>0)&0xff);
		}else{
			cmd[1] = ((u32Addr>>16)&0xff);
			cmd[2] = ((u32Addr>>8)&0xff);
			cmd[3] = ((u32Addr>>0)&0xff);
		}

		flash_write_enable(1);
#if defined(SPI_DIRECT_IO_SUPPORT)
		PPDRV_SPI_CSActivate(eSPI_CHANNEL_FLASH);
#endif
		//PPDRV_SPI_TxRx(eSPI_CHANNEL_FLASH, cmd, NULL, 4);
		PPDRV_SPI_Tx(eSPI_CHANNEL_FLASH, cmd, cmd_size);
#if defined(SPI_DIRECT_IO_SUPPORT)
		PPDRV_SPI_CSDeActivate(eSPI_CHANNEL_FLASH);
#endif
		
		flash_cmd_wait_ready(1);

		flash_write_enable(0);

		s32Len -= s32EraseUnitSize;
		u32Addr += s32EraseUnitSize;
	}
	return 0;	
}


