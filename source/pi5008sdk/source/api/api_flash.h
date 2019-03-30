/*
 * api_flash.h
 *
 *  Created on: 2018. 1. 10.
 *      Author: ihkong
 */

#include "type.h"
#include "error.h"
#include "spi_nand_flash.h"

#ifndef _FLASH_API_H_
#define _FLASH_API_H_

#define FLASH_HEADER_SIGNATURE		(0xABCD1234)
#define FLASH_HEADER_EMPTY			(0xFFFFFFFF)

#define NAND_BATL_ADDR				0x1000	//

#define FLASH_VER_ERROR_CHECK(x)		(x==0 || x==FLASH_HEADER_EMPTY)

typedef enum ppFLASH_TYPE_E
{
	eFLASH_TYPE_NOR = 0,
	eFLASH_TYPE_NAND,
}PP_FLASH_TYPE_E;

typedef enum ppNAND_AREA_E
{
	eNAND_AREA_RAW = 0,
	eNAND_AREA_FTL,
}PP_NAND_AREA_E;

typedef enum ppFLASH_SECT_E{
	eFLASH_SECT_BL1 = 0,		//0
	eFLASH_SECT_BL2,			//1
	eFLASH_SECT_IFW,			//2
	eFLASH_SECT_IFW1,			//3
	eFLASH_SECT_MFW,			//4
	eFLASH_SECT_MFW1,			//5
	eFLASH_SECT_SVM_LUT,		//6
	eFLASH_SECT_CAR_IMG,		//7
	eFLASH_SECT_MENU,			//8
	eFLASH_SECT_UI_IMG,			//9
	eFLASH_SECT_PGL_IMG,		//10
	eFLASH_SECT_ISP_DATA,		//11
	eFLASH_SECT_SENSOR_DATA,	//12
	eFLASH_SECT_CAM_CALIB_MAIN_TYPE,		//13
	eFLASH_SECT_CAM_CALIB_SUB_TYPE,		//14
	eFLASH_SECT_CAM_CAPTURE,	//15
	eFLASH_SECT_AUDIO,			//16
	eFLASH_SECT_STANDBY_M,		//17
	eFLASH_SECT_STANDBY_I,		//18
	eFLASH_SECT_MAX,			//19
}PP_FLASH_SECT_E;

typedef struct ppFLASH_SECT_S{
	PP_U32 u32Ver;
	PP_U32 u32FlashAddr;			// boot loader flash address
	PP_U32 u32LoadAddr;			// boot loader load address in ram
	PP_U32 u32Size;				// boot loader size
}PP_FLASH_SECT_S;



typedef struct ppFLASH_HDR_S
{
	PP_U32 u32Sign;			// 0x0

	PP_U32 u32FlashType;	// 0x4
	PP_U32 u32PageSize;		// 0x8
	PP_U32 u32PagesBlock;	// 0xc

	PP_U32 u32ClockDiv;		// 0x10, boot rom spi controller clock divider

	PP_FLASH_SECT_S stSect[eFLASH_SECT_MAX];	// 0x14 ~ 0x144

	PP_U32 u32Rsv[0x3AA];		// 0x144
	PP_U32 u32Max;				// 0xFEC
	PP_U32 u32CRC16;			// 0xFF0
	PP_U32 u32Rsv1[3];			// 0xFF4
}PP_FLASH_HDR_S;

extern PP_FLASH_HDR_S gstFlashHeader;
extern PP_FLASH_HDR_S gstFlashFTLHeader;

PP_RESULT_E PPAPI_FLASH_Initialize(PP_FLASH_TYPE_E IN enFlashType);
PP_VOID PPAPI_FLASH_Erase(PP_U32 IN u32Addr, PP_U32 IN u32Size);
PP_VOID PPAPI_FLASH_Write(PP_U8 IN *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_VOID PPAPI_FLASH_WriteErasedBlock(PP_U8 IN *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_VOID PPAPI_FLASH_Read(PP_U8 OUT *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_RESULT_E PPAPI_FLASH_ReadQDMA(PP_U8 OUT *pu8Buf, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_RESULT_E PPAPI_FLASH_ReadHeader(PP_VOID);
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
PP_RESULT_E PPAPI_FLASH_ReadFTLHeader(PP_VOID);
#endif
PP_U16 PPAPI_FLASH_GetCRC16(PP_VOID);

#endif
