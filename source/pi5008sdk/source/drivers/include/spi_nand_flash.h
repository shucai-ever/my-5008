#ifndef _PI5008_SPI_NAND_FLASH_H
#define _PI5008_SPI_NAND_FLASH_H

#include "type.h"
#include "error.h"

#define NAND_MANID_GIGADEVICE	0xC8
#define NAND_MANID_MICRON		0x2C
#define NAND_MANID_WINBOND		0xEF

typedef struct ppFLASH_NAND_ID_S
{
	uint8 u8ManufacturerID;
	uint8 u8DeviceID0;
	uint8 u8DeviceID1;
	uint8 u8FTLDeviceID;
	uint32 u32TotalSize;
	uint32 u32PageSize;
	uint32 u32EraseBlockSize;
}PP_FLASH_NAND_ID_S;

PP_RESULT_E PPDRV_SNAND_FLASH_Initialize(PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U8 IN u8QuadIO);
PP_RESULT_E PPDRV_SNAND_FLASH_ReadID(PP_U8* OUT pu8ID);
PP_U8 PPDRV_SNAND_FLASH_GetFeature(PP_U8 IN u8Addr);
PP_RESULT_E PPDRV_SNAND_FLASH_SetFeature(PP_U8 IN u8Addr, PP_U8 IN u8Feature);
PP_RESULT_E PPDRV_SNAND_FLASH_ReadPage(PP_U8* OUT pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32ColAddr, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SNAND_FLASH_ReadSpare(PP_U8* OUT pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Offset, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SNAND_FLASH_ReadPageMemc(PP_U8* OUT pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32ColAddr, PP_U32 IN u32Size, PP_U8 IN u8DMA);
PP_RESULT_E PPDRV_SNAND_FLASH_WritePage(PP_U8* IN pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SNAND_FLASH_WriteSpare(PP_U8* IN pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Offset, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SNAND_FLASH_WritePageDMA(PP_U8* IN pu8Buf, PP_U32 IN u32BlockAddr, PP_U32 IN u32PageAddr, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SNAND_FLASH_EraseBlock(PP_U32 u32BlockAddr);
PP_VOID PPDRV_SNAND_FLASH_UnlockBlock(PP_VOID);
PP_U8 PPDRV_SNAND_FLASH_CheckBad(PP_U32 u32BlockAddr);

extern PP_FLASH_NAND_ID_S gstFlashNandID;

#endif

