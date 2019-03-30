#ifndef _PI5008_SPI_FLASH_H
#define _PI5008_SPI_FLASH_H

#include "type.h"
#include "error.h"

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

PP_RESULT_E PPDRV_FLASH_Initialize(PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U8 IN u8QuadIO);
PP_VOID PPDRV_FLASH_WriteProtect(PP_U8 IN u8On);
PP_RESULT_E PPDRV_FLASH_ReadID(PP_U8* OUT pu8ID);
PP_U8 PPDRV_FLASH_GetStatus(PP_VOID);
PP_RESULT_E PPDRV_FLASH_SetStatus(PP_U8 IN u8Status);
PP_U8 PPDRV_FLASH_GetConfig(PP_VOID);
PP_RESULT_E PPDRV_FLASH_SetConfig(PP_U8 IN u8Config);
PP_RESULT_E PPDRV_FLASH_Read(PP_U8* OUT pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_RESULT_E PPDRV_FLASH_Write(PP_U8* IN pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_RESULT_E PPDRV_FLASH_WriteDMA(PP_U8* IN pu8Buf, PP_U32 IN u32Addr, PP_S32 s32Size);
PP_RESULT_E PPDRV_FLASH_Erase(PP_U32 IN u32Addr, PP_S32 IN s32Len, PP_S32 IN s32EraseUnitSize);

extern PP_FLASH_ID_S gstFlashID;

#endif

