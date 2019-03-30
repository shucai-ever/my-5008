#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "system.h"
#include "clock.h"
#include "spi_flash.h"
#include "spi_nand_flash.h"
#include "spi_memctrl.h"
#include "api_flash.h"
#include "osal.h"
#include "cache.h"
#include "sys_api.h"
#include "board_config.h"
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
#include "api_FAT_FTL.h"
#endif

#define NO_BATL		0
#define MAX_BAD_BLOCK_NUM	20

typedef struct ppFLASH_INIT_S{
	PP_U32 u32Div;
	PP_U32 u32Dly;
	PP_U32 u32PageSize;			// nand flash only
	PP_U32 u32EraseBlockSize;	// nand flash only
	PP_U32 u32PagesBlocks;
	PP_FLASH_TYPE_E enFlashType;
}PP_FLASH_INIT_S;

static PP_FLASH_INIT_S gstFlash;
static PP_U8 *gpReadBuf;

PP_FLASH_HDR_S gstFlashHeader = {0};
PP_FLASH_HDR_S gstFlashFTLHeader = {0};
//======================================================
//	Bad block management - NAND flash only
typedef struct ppNAND_BATL_S{
	PP_S8 s8Marker[8];
	PP_U32 u32LUT[SPI_NAND_NON_FTL_BLOCKS];
}PP_NAND_BATL_S;

static PP_S8 gs8BATLMarker[8] = {'B','A','T','L','M','A','R','K'};
static PP_NAND_BATL_S gstBATL;
static PP_U32 u32NandTotalValidBlocks = SPI_NAND_NON_FTL_BLOCKS;
//======================================================

static PP_VOID snand_batl_init(PP_VOID);
static PP_RESULT_E snand_write(PP_U8* IN pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
static PP_RESULT_E snand_read(PP_U8* OUT pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size, PP_U8 IN u8DMA);


static PP_S32 snand_next_valid_block(PP_U8 *pu8BadTable, PP_S32 s32Index)
{
	PP_S32 i;

	for(i=s32Index;i<SPI_NAND_NON_FTL_BLOCKS;i++){
		if(pu8BadTable[i] != 1)break;
	}

	return i;
}

static PP_VOID snand_batl_init(PP_VOID)
{
	PP_S32 i;
	PP_U32 u32BadCnt = 0;

	LOG_DEBUG("NAND BATL Size: %lu\n", sizeof(gstBATL));

	for(i=0;i<SPI_NAND_NON_FTL_BLOCKS;i++){
		gstBATL.u32LUT[i] = i;
	}
#if NO_BATL
	memcpy(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker));
#else
	snand_read((PP_U8 *)&gstBATL, NAND_BATL_ADDR, sizeof(gstBATL), 0);

	if(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) != 0){
		PP_U8 *pu8Buf = NULL;
		PP_U8 *pu8BadTable = NULL;
		PP_S32 s32FindIndex;

		LOG_DEBUG("Making bad block translation table\n");
		pu8Buf = (PP_U8 *)OSAL_malloc(gstFlashNandID.u32EraseBlockSize);
		configASSERT(pu8Buf);

		pu8BadTable = (PP_U8 *)OSAL_malloc(SPI_NAND_NON_FTL_BLOCKS);
		configASSERT(pu8BadTable);

		// find bad block
		for(i=0;i<SPI_NAND_NON_FTL_BLOCKS;i++){
			if(PPDRV_SNAND_FLASH_CheckBad(i) != eSUCCESS){
				pu8BadTable[i] = 1;
				u32BadCnt++;
				LOG_DEBUG("%dth block is bad\n", i);
				configASSERT(u32BadCnt < MAX_BAD_BLOCK_NUM);
			}else{
				pu8BadTable[i] = 0;
			}
		}

		// make block address translation lut
		s32FindIndex = 0;
		for(i=0;i<SPI_NAND_NON_FTL_BLOCKS;i++){
			gstBATL.u32LUT[i] = snand_next_valid_block(pu8BadTable, s32FindIndex);
			s32FindIndex = gstBATL.u32LUT[i]+1;
		}

		memcpy(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker));
		// Block 0 should not be bad block.
		configASSERT(gstBATL.u32LUT[0] == 0);

		u32NandTotalValidBlocks = SPI_NAND_NON_FTL_BLOCKS - u32BadCnt;

		snand_read(pu8Buf, 0, gstFlashNandID.u32EraseBlockSize, 0);
		PPDRV_SNAND_FLASH_EraseBlock(0);
		memcpy(&pu8Buf[NAND_BATL_ADDR], (PP_U8 *)&gstBATL, sizeof(gstBATL));

		// write batl to flash
		snand_write(pu8Buf, 0, gstFlashNandID.u32EraseBlockSize);

		if(pu8Buf)OSAL_free(pu8Buf);
		if(pu8BadTable)OSAL_free(pu8BadTable);
	}else{
		for(i=0;i<SPI_NAND_NON_FTL_BLOCKS;i++){
			if(gstBATL.u32LUT[i] != i+u32BadCnt){
				u32BadCnt++;
				LOG_DEBUG("%dth block is bad\n", i);
			}
		}
		
	}

	LOG_DEBUG("%d bad blocks found\n", u32BadCnt);
#if 0
	for(i=0;i<SPI_NAND_NON_FTL_BLOCKS;i++){
		LOG_DEBUG("Block %d -> %d\n", i, gstBATL.u32LUT[i]);
	}
#endif
#endif

}


static PP_RESULT_E snand_write(PP_U8* IN pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	PP_U32 u32WriteOffset;
	PP_U32 u32BlockAddr;
	PP_U32 u32PageAddr;
	PP_S32 s32WriteSize;

	while(s32Size){
		u32WriteOffset = u32Addr & (gstFlash.u32PageSize - 1);
		u32BlockAddr = (u32Addr - u32WriteOffset) / gstFlash.u32EraseBlockSize;
		u32PageAddr = ((u32Addr - u32WriteOffset) & (gstFlash.u32EraseBlockSize-1))/gstFlash.u32PageSize;
		s32WriteSize = ((gstFlash.u32PageSize - u32WriteOffset) > s32Size) ? s32Size : (gstFlash.u32PageSize - u32WriteOffset);

		if(u32BlockAddr >= u32NandTotalValidBlocks){
			return eERROR_FAILURE;
		}
		if(u32WriteOffset){
			//LOG_DEBUG("[%s]read page ba: 0x%x, pa: 0x%x\n",__FUNCTION__, u32BlockAddr, u32PageAddr);
			PPDRV_SNAND_FLASH_ReadPageMemc(gpReadBuf, gstBATL.u32LUT[u32BlockAddr], u32PageAddr, 0, gstFlash.u32PageSize, 0);
			//LOG_DEBUG("[%s]copy data offset: 0x%x",__FUNCTION__, u32WriteOffset);
			memcpy(&gpReadBuf[u32WriteOffset], pu8Buf, s32WriteSize);
			//LOG_DEBUG("[%s]write page ba: 0x%x, pa: 0x%x\n",__FUNCTION__, u32BlockAddr, u32PageAddr);
			PPDRV_SNAND_FLASH_WritePage(gpReadBuf, gstBATL.u32LUT[u32BlockAddr], u32PageAddr, s32WriteSize);
		}else{
			//LOG_DEBUG("[%s]write page ba: 0x%x, pa: 0x%x\n",__FUNCTION__, u32BlockAddr, u32PageAddr);
			PPDRV_SNAND_FLASH_WritePage(pu8Buf, gstBATL.u32LUT[u32BlockAddr], u32PageAddr, gstFlash.u32PageSize);
		}

		pu8Buf += s32WriteSize;
		u32Addr += s32WriteSize;
		s32Size -= s32WriteSize;

	}

	return eSUCCESS;
}

static PP_RESULT_E snand_read(PP_U8* OUT pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size, PP_U8 IN u8DMA)
{
	PP_U32 u32ReadOffset;
	PP_U32 u32BlockAddr;
	PP_U32 u32PageAddr;
	PP_S32 s32ReadSize;

	while(s32Size){
		u32ReadOffset = u32Addr & (gstFlash.u32PageSize - 1);
		u32BlockAddr = (u32Addr - u32ReadOffset) / gstFlash.u32EraseBlockSize;
		u32PageAddr = ((u32Addr - u32ReadOffset) & (gstFlash.u32EraseBlockSize-1))/gstFlash.u32PageSize;
		s32ReadSize = ((gstFlash.u32PageSize - u32ReadOffset) > s32Size) ? s32Size : (gstFlash.u32PageSize - u32ReadOffset);

		if(u32BlockAddr >= u32NandTotalValidBlocks){
			return eERROR_FAILURE;
		}

		if(u32ReadOffset){
			//LOG_DEBUG("[%s]read page ba: 0x%x, pa: 0x%x\n",__FUNCTION__, u32BlockAddr, u32PageAddr);
			PPDRV_SNAND_FLASH_ReadPageMemc(gpReadBuf, gstBATL.u32LUT[u32BlockAddr], u32PageAddr, 0, gstFlash.u32PageSize, u8DMA);
			//LOG_DEBUG("[%s]copy data offset: 0x%x",__FUNCTION__, u32ReadOffset);
			memcpy(pu8Buf, &gpReadBuf[u32ReadOffset], s32ReadSize);
		}else{
			//LOG_DEBUG("[%s]read page ba: 0x%x, pa: 0x%x\n",__FUNCTION__, u32BlockAddr, u32PageAddr, gstFlash);
			PPDRV_SNAND_FLASH_ReadPageMemc(pu8Buf, gstBATL.u32LUT[u32BlockAddr], u32PageAddr, u32ReadOffset, s32ReadSize, u8DMA);
		}

		pu8Buf += s32ReadSize;
		u32Addr += s32ReadSize;
		s32Size -= s32ReadSize;
	}

	return eSUCCESS;
}



//PP_RESULT_E PPAPI_FLASH_Initialize(PP_U32 IN u32PageSize, PP_U32 IN u32EraseBlockSize, PP_FLASH_TYPE_E IN enFlashType)
PP_RESULT_E PPAPI_FLASH_Initialize(PP_FLASH_TYPE_E IN enFlashType)
{
	SPI_MEM_FR_INFO fr_info;
	PP_RESULT_E ret = eSUCCESS;

	if(enFlashType == eFLASH_TYPE_NOR){
		if(PPDRV_FLASH_Initialize(u32APBClk/((SPI_CTRL_DIV+1)*2), 0, 1) < 0){
			ret = eERROR_FAILURE;
			goto END_FT;
		}
		PPDRV_FLASH_WriteProtect(0);

		gstFlash.u32Div = FLASH_CTRL_DIV;
		gstFlash.u32Dly = FLASH_CTRL_DELAY;
		gstFlash.u32PageSize = gstFlashID.u32PageSize;
		gstFlash.u32EraseBlockSize = gstFlashID.u32EraseBlockSize;
		gstFlash.u32PagesBlocks = gstFlashID.u32EraseBlockSize/gstFlashID.u32PageSize;
		gstFlash.enFlashType = enFlashType;

	}else{
		if(PPDRV_SNAND_FLASH_Initialize(u32APBClk/((SPI_CTRL_DIV+1)*2), 0, 1)){
			ret = eERROR_FAILURE;
			goto END_FT;
		}
		PPDRV_SNAND_FLASH_UnlockBlock();

		gstFlash.u32Div = FLASH_CTRL_DIV;
		gstFlash.u32Dly = FLASH_CTRL_DELAY;
		gstFlash.u32PageSize = gstFlashNandID.u32PageSize;
		gstFlash.u32EraseBlockSize = gstFlashNandID.u32EraseBlockSize;
		gstFlash.u32PagesBlocks = gstFlashNandID.u32EraseBlockSize/gstFlashNandID.u32PageSize;
		gstFlash.enFlashType = enFlashType;

	}

	memset(&fr_info, 0, sizeof(fr_info));
	fr_info.flash_type = gstFlash.enFlashType;

	if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
		fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_8;
		fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_VAL;
		fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
	}else{
		fr_info.mode_size = SPI_MEMCTRL_MODE_SIZE_0;
		fr_info.mode_out_state = SPI_MEMCTRL_MODE_STATE_NONE;

		if(gstFlashNandID.u8ManufacturerID == NAND_MANID_GIGADEVICE){
			fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_8;
		}else{	// micron, winbond
			fr_info.dummy_size = SPI_MEMCTRL_DUMMY_SIZE_16;
		}

	}
	fr_info.mode_value = 0;
	fr_info.read_type = SPI_MEMCTRL_READ_QUAD_IO;

	if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
		if(gstFlashID.u8AddrMode4Byte){
			fr_info.addr_4byte = 1;
			spi_memctrl_set(gstFlash.u32Div, gstFlash.u32Dly, 0xEC, fr_info);
		}else{
			spi_memctrl_set(gstFlash.u32Div, gstFlash.u32Dly, 0xEB, fr_info);
		}
	}else{
		spi_memctrl_set(gstFlash.u32Div, gstFlash.u32Dly, 0xEB, fr_info);
	}

	gpReadBuf = OSAL_malloc(gstFlash.u32EraseBlockSize);

	if(gstFlash.enFlashType == eFLASH_TYPE_NAND){
		snand_batl_init();
	}

END_FT:
	return ret;
}

PP_VOID PPAPI_FLASH_Erase(PP_U32 IN u32Addr, PP_U32 IN u32Size)
{
	PP_U32 i;
	PP_U32 u32BlockAddr;
	PP_U32 u32BlockCnt;

	u32Size += (u32Addr % gstFlash.u32EraseBlockSize);
	u32Size = ((u32Size + (gstFlash.u32EraseBlockSize - 1)) & (~(gstFlash.u32EraseBlockSize - 1)));
	u32Addr -= (u32Addr % gstFlash.u32EraseBlockSize);

	if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
		//LOG_DEBUG("[%s]addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, u32Size);
		PPDRV_FLASH_Erase(u32Addr, u32Size, gstFlash.u32EraseBlockSize);
	}else{

		configASSERT(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) == 0);
		u32BlockAddr = u32Addr / gstFlash.u32EraseBlockSize;
		u32BlockCnt = u32Size / gstFlash.u32EraseBlockSize;
		for(i=0;i<u32BlockCnt;i++){
			if((u32BlockAddr + i) < u32NandTotalValidBlocks){
				PPDRV_SNAND_FLASH_EraseBlock(gstBATL.u32LUT[u32BlockAddr + i]);
			}
		}
	}
}

// partial writing possible
// read block -> partial write -> erase block -> write block
PP_VOID PPAPI_FLASH_Write(PP_U8 IN *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	PP_U32 woffset;
	PP_U32 wsize;
	PP_U32 block_size;
	PP_U8 *wptr;

	//block_size = ( gstFlash.enFlashType == eFLASH_TYPE_NOR ? FLASH_ERASE_SECTOR : gstFlash.u32EraseBlockSize );
	block_size = gstFlash.u32EraseBlockSize;

	if(gstFlash.enFlashType == eFLASH_TYPE_NAND){
		configASSERT(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) == 0);
	}

	while(s32Size > 0){
		woffset = u32Addr % block_size;
		wsize = ( ( block_size - woffset ) > s32Size ? s32Size : ( block_size - woffset ) );

		if(woffset == 0 && wsize == block_size){
			wptr = pu8Buf;
		}else{
#if VPU_AVAILABLE
			PPAPI_FLASH_ReadQDMA(gpReadBuf, u32Addr - woffset, block_size);
#else
			PPAPI_FLASH_Read(gpReadBuf, u32Addr - woffset, block_size);
#endif
/*
			if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
				//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n",__FUNCTION__, (SPI_MEM_BASE_ADDR + (u32Addr - woffset )), block_size);
				memcpy(gpReadBuf, (PP_U32 *)(SPI_MEM_BASE_ADDR + (u32Addr - woffset )), block_size);
			}else{
				//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n",__FUNCTION__, u32Addr - woffset, block_size);
				snand_read(gpReadBuf, u32Addr - woffset, block_size, 0);
			}
*/
			//LOG_DEBUG("[%s]copy data offset: 0x%x, size: 0x%x\n",__FUNCTION__, woffset, wsize);
			memcpy(&gpReadBuf[woffset], pu8Buf, wsize);
			wptr = gpReadBuf;
		}
		//LOG_DEBUG("[%s]erase addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr-woffset, block_size);
		PPAPI_FLASH_Erase(u32Addr-woffset, block_size);

		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]write addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr - woffset, block_size);
			PPDRV_FLASH_Write(wptr, u32Addr - woffset, block_size);
			//PPDRV_FLASH_WriteDMA(wptr, u32Addr - woffset, block_size);
		}else{
			//LOG_DEBUG("[%s]write addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr - woffset, block_size);
			snand_write(wptr, u32Addr - woffset, block_size);
		}

		u32Addr += wsize;
		s32Size -= wsize;
		pu8Buf += wsize;
	}


}

PP_VOID PPAPI_FLASH_WriteErasedBlock(PP_U8 IN *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{

#if 0
	PP_U32 woffset;
	PP_U32 wsize;
	PP_U32 block_size;
	PP_U8 *wptr;

	if(gstFlash.enFlashType == eFLASH_TYPE_NOR)
		PPDRV_FLASH_Write(pu8Buf, u32Addr, s32Size);
	else
		snand_flash_write(pu8Buf, u32Addr, s32Size);

#else
	PP_U32 woffset;
	PP_U32 wsize;
	PP_U32 page_size;
	PP_U8 *wptr;

	if(gstFlash.enFlashType == eFLASH_TYPE_NAND){
		configASSERT(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) == 0);
	}
	//block_size = ( gstFlash.enFlashType == eFLASH_TYPE_NOR ? FLASH_ERASE_SECTOR : gstFlash.u32EraseBlockSize );
	page_size = gstFlash.u32PageSize;

	while(s32Size > 0){
		woffset = u32Addr % page_size;
		wsize = ( ( page_size - woffset ) > s32Size ? s32Size : ( page_size - woffset ) );

		if(woffset == 0 && wsize == page_size){
			wptr = pu8Buf;
		}else{
			//LOG_DEBUG("[%s]copy data offset: 0x%x, size: 0x%x\n", __FUNCTION__, woffset, wsize);
			memset(gpReadBuf, 0xff, page_size);
			memcpy(&gpReadBuf[woffset], pu8Buf, wsize);

			wptr = gpReadBuf;
		}

		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]write addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr - woffset, page_size);
			PPDRV_FLASH_Write(wptr, u32Addr - woffset, page_size);
			//PPDRV_FLASH_WriteDMA(wptr, u32Addr - woffset, page_size);
		}else{
			//LOG_DEBUG("[%s]write addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr - woffset, page_size);
			snand_write(wptr, u32Addr - woffset, page_size);
		}

		u32Addr += wsize;
		s32Size -= wsize;
		pu8Buf += wsize;
	}

#endif

}


PP_VOID PPAPI_FLASH_Read(PP_U8 OUT *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	spi_memctrl_crc_en(0);
	spi_memctrl_crc_en(1);
	if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
		//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, (SPI_MEM_BASE_ADDR + u32Addr), s32Size);
		//memcpy(pu8Buf, (PP_U32 *)(SPI_MEM_BASE_ADDR + u32Addr), s32Size);
		spi_memctrl_read(u32Addr, pu8Buf, s32Size);
	}else{
		configASSERT(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) == 0);
		//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, s32Size);
		snand_read(pu8Buf, u32Addr, s32Size, 0);
	}

}

#ifdef FPGA_ASIC_TOP //only for fpga suppport code.
# if ((VIDEO_IN_TYPE==VIDEO_IN_TYPE_MIPI_BAYER) || (VIDEO_IN_TYPE==VIDEO_IN_TYPE_MIPI_YUV) )
PP_RESULT_E PPAPI_FLASH_ReadQDMA(PP_U8 OUT *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN u32Size)
{
	PPAPI_FLASH_Read(pu8Buf, u32Addr, u32Size);
	return eSUCCESS;
}
# else 


PP_RESULT_E PPAPI_FLASH_ReadQDMA(PP_U8 OUT *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	PP_U32 u32StartAddr;
	PP_U32 u32ReadOffset;
	PP_S32 s32ReadSize;
	PP_U32 Alignment;

	if(((PP_U32)pu8Buf & 0xf) || (u32Addr & 0xf) || (s32Size & 0xf) ){
		LOG_DEBUG("[%s]Invalid alignment. pu8Buf: 0x%x, u32Addr: 0x%x, s32Size: 0x%x\n", __FUNCTION__, ((PP_U32)pu8Buf & 0xf), (u32Addr & 0xf), (s32Size & 0xf) );
	}

	if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
		Alignment = 256;
	}else{
		Alignment = gstFlash.u32PageSize;
		configASSERT(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) == 0);
	}
	spi_memctrl_crc_en(0);
	spi_memctrl_crc_en(1);

	PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, s32Size);
	PPAPI_SYS_CACHE_Invalidate((PP_U32 *)gpReadBuf, Alignment);

	// first part. read non-aligned address if exist.
	if(u32Addr & (Alignment - 1)){
		u32ReadOffset = (u32Addr & (Alignment - 1));
		u32StartAddr = u32Addr - u32ReadOffset;
		s32ReadSize = ((Alignment - u32ReadOffset) > s32Size) ? s32Size : (Alignment - u32ReadOffset);
		//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)gpReadBuf, Alignment);
		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32StartAddr, Alignment);
			spi_memctrl_read_qdma(u32StartAddr, gpReadBuf, Alignment);
		}else{
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32StartAddr, Alignment);
			snand_read(gpReadBuf, u32StartAddr, Alignment, 1);
		}

		//LOG_DEBUG("[%s]copy data offset: 0x%x, size: 0x%x\n", __FUNCTION__, u32ReadOffset, s32ReadSize);
		memcpy(ADDR_NON_CACHEABLE((uint32)pu8Buf), ADDR_NON_CACHEABLE((uint32)&gpReadBuf[u32ReadOffset]), s32ReadSize );

		pu8Buf += s32ReadSize;
		s32Size -= s32ReadSize;
		u32Addr += s32ReadSize;
	}

	// middle part. read aligned address if exist.
	s32ReadSize = (s32Size & (~(Alignment - 1)));
	if(s32ReadSize > 0){
		//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, s32ReadSize);
		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, s32ReadSize);
			spi_memctrl_read_qdma(u32Addr, pu8Buf, s32ReadSize);
		}else{
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, s32ReadSize);
			snand_read(pu8Buf, u32Addr, s32ReadSize, 1);
		}

		pu8Buf += s32ReadSize;
		s32Size -= s32ReadSize;
		u32Addr += s32ReadSize;

	}

	// last part. read aligned address but size is not aligned if exist.
	if(s32Size > 0){
		//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)gpReadBuf, Alignment);
		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, Alignment);
			spi_memctrl_read_qdma(u32Addr, gpReadBuf, Alignment);
		}else{
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, Alignment);
			snand_read(gpReadBuf, u32Addr, Alignment, 1);
		}
		//LOG_DEBUG("[%s]copy data offset: 0, size: 0x%x\n", __FUNCTION__, s32Size);
		memcpy(ADDR_NON_CACHEABLE((uint32)pu8Buf), ADDR_NON_CACHEABLE((uint32)gpReadBuf), s32Size );

		pu8Buf += s32Size;
		s32Size -= s32Size;
		u32Addr += s32Size;

	}

	if(s32Size != 0){
		//LOG_DEBUG("[%s]Error!!! Size not zero(%d)\n", __FUNCTION__, s32Size);
	}
	//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, u32Size);

	return eSUCCESS;
}


# endif
#else //FPGA_ASIC_TOP
PP_RESULT_E PPAPI_FLASH_ReadQDMA(PP_U8 OUT *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size)
{
	PP_U32 u32StartAddr;
	PP_U32 u32ReadOffset;
	PP_S32 s32ReadSize;
	PP_U32 Alignment;

	if(((PP_U32)pu8Buf & 0xf) || (u32Addr & 0xf) || (s32Size & 0xf) ){
		LOG_DEBUG("[%s]Invalid alignment. pu8Buf: 0x%x, u32Addr: 0x%x, s32Size: 0x%x\n", __FUNCTION__, ((PP_U32)pu8Buf & 0xf), (u32Addr & 0xf), (s32Size & 0xf) );
	}

	if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
		Alignment = 256;
	}else{
		Alignment = gstFlash.u32PageSize;
		configASSERT(memcmp(gstBATL.s8Marker, gs8BATLMarker, sizeof(gs8BATLMarker)) == 0);
	}
	spi_memctrl_crc_en(0);
	spi_memctrl_crc_en(1);

	PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, s32Size);
	PPAPI_SYS_CACHE_Invalidate((PP_U32 *)gpReadBuf, Alignment);

	// first part. read non-aligned address if exist.
	if(u32Addr & (Alignment - 1)){
		u32ReadOffset = (u32Addr & (Alignment - 1));
		u32StartAddr = u32Addr - u32ReadOffset;
		s32ReadSize = ((Alignment - u32ReadOffset) > s32Size) ? s32Size : (Alignment - u32ReadOffset);
		//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)gpReadBuf, Alignment);
		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32StartAddr, Alignment);
			spi_memctrl_read_qdma(u32StartAddr, gpReadBuf, Alignment);
		}else{
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32StartAddr, Alignment);
			snand_read(gpReadBuf, u32StartAddr, Alignment, 1);
		}

		//LOG_DEBUG("[%s]copy data offset: 0x%x, size: 0x%x\n", __FUNCTION__, u32ReadOffset, s32ReadSize);
		memcpy((void *)ADDR_NON_CACHEABLE((uint32)pu8Buf), (void *)ADDR_NON_CACHEABLE((uint32)&gpReadBuf[u32ReadOffset]), s32ReadSize );

		pu8Buf += s32ReadSize;
		s32Size -= s32ReadSize;
		u32Addr += s32ReadSize;
	}

	// middle part. read aligned address if exist.
	s32ReadSize = (s32Size & (~(Alignment - 1)));
	if(s32ReadSize > 0){
		//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, s32ReadSize);
		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, s32ReadSize);
			spi_memctrl_read_qdma(u32Addr, pu8Buf, s32ReadSize);
		}else{
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, s32ReadSize);
			snand_read(pu8Buf, u32Addr, s32ReadSize, 1);
		}

		pu8Buf += s32ReadSize;
		s32Size -= s32ReadSize;
		u32Addr += s32ReadSize;

	}

	// last part. read aligned address but size is not aligned if exist.
	if(s32Size > 0){
		//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)gpReadBuf, Alignment);
		if(gstFlash.enFlashType == eFLASH_TYPE_NOR){
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, Alignment);
			spi_memctrl_read_qdma(u32Addr, gpReadBuf, Alignment);
		}else{
			//LOG_DEBUG("[%s]read addr: 0x%x, size: 0x%x\n", __FUNCTION__, u32Addr, Alignment);
			snand_read(gpReadBuf, u32Addr, Alignment, 1);
		}
		//LOG_DEBUG("[%s]copy data offset: 0, size: 0x%x\n", __FUNCTION__, s32Size);
		memcpy((void *)ADDR_NON_CACHEABLE((uint32)pu8Buf), (void *)ADDR_NON_CACHEABLE((uint32)gpReadBuf), s32Size );

		pu8Buf += s32Size;
		s32Size -= s32Size;
		u32Addr += s32Size;

	}

	if(s32Size != 0){
		//LOG_DEBUG("[%s]Error!!! Size not zero(%d)\n", __FUNCTION__, s32Size);
	}
	//PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, u32Size);

	return eSUCCESS;
}
#endif //FPGA_ASIC_TOP

PP_RESULT_E PPAPI_FLASH_ReadHeader(PP_VOID)
{
	PP_RESULT_E ret = eSUCCESS;

	PPAPI_FLASH_Read((PP_U8 *)&gstFlashHeader, 0, sizeof(PP_FLASH_HDR_S));

	if(gstFlashHeader.u32Sign != FLASH_HEADER_SIGNATURE)ret = eERROR_FAILURE;

	return ret;
}

#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
PP_RESULT_E PPAPI_FLASH_ReadFTLHeader(PP_VOID)
{
	PP_RESULT_E ret = eSUCCESS;

	PPAPI_FTL_Read((PP_U8 *)&gstFlashFTLHeader, 0, sizeof(PP_FLASH_HDR_S));

	if(gstFlashFTLHeader.u32Sign != FLASH_HEADER_SIGNATURE)ret = eERROR_FAILURE;

	return ret;
}
#endif

PP_U16 PPAPI_FLASH_GetCRC16(PP_VOID)
{
	return spi_memctrl_crc_get();

}
