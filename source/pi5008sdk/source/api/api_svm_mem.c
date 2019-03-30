/**
 * \file
 *
 * \brief	SVM memory control APIs
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#include "api_flash.h"
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
#include "api_FAT_FTL.h"
#endif
#include "sys_api.h"
#include "dram.h"
#include "vin.h"
#include "api_vin.h"
#include "prj_config.h"
#include "dma.h"

#include "api_svm_mem.h"

/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#define SVMMEM_VIEW_SECTION_COMMON_BIT		(15)
#define VIEWSECTIONINDEX(x)					utilClearBit(x, SVMMEM_VIEW_SECTION_COMMON_BIT)
#define ISVIEWCOMMONSECTIONINDEX(x)			utilGetBit(x, SVMMEM_VIEW_SECTION_COMMON_BIT)
#define SVMMEM_PAGE_NULL					(0xFFFFFFFF)

/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
/************************************************************************************************
 * Merge
************************************************************************************************/
typedef enum ppSVMMEM_MERGE_TYPE_S
{
	eSVMMEM_MERGE_TYPE_FB = 0,
	eSVMMEM_MERGE_TYPE_LR,
	eSVMMEM_MERGE_TYPE_BC,
	eSVMMEM_MERGE_TYPE_MAX,
} PP_SVMMEM_MERGE_TYPE_E;

/************************************************************************************************
 * Others
************************************************************************************************/
typedef enum ppSVMMEM_PAGE_AREA_E
{
	eSVMMEM_PAGE_AREA_FLIPPING = 0,			// page flipping
	eSVMMEM_PAGE_AREA_CACHE,				// pre-loading page
	eSVMMEM_PAGE_AREA_MAX,
} PP_SVMMEM_PAGE_AREA_E;

typedef enum ppSVMMEM_LOAD_STATE_E
{
	eSVMMEM_LOAD_STATE_NULL = 0,
	eSVMMEM_LOAD_STATE_MERGE,				// lut merge
	eSVMMEM_LOAD_STATE_COPY,				// copy flash to dram
	eSVMMEM_LOAD_STATE_MAX,
} PP_SVMMEM_LOAD_STATE_E;

/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
/************************************************************************************************
 * Header
************************************************************************************************/
typedef struct ppSVMMEM_SECTION_DATA_S
{
	PP_U32 u32StoredAear;														// FTL or NON-FTL, Nor
	PP_U32 u32AddrOffset;														// offset address in flash
	PP_U32 u32DataSize;
} PP_SVMMEM_SECTION_DATA_S;

typedef struct ppSVMMEM_SECTION_HEADER_S
{
	PP_SIZE_S stSectionSize;													// rectangle of section. 8x8 sampling applied rectangel
	PP_RECT_S stImgRect;														// rectangle of image
	PP_SVMMEM_SECTION_DATA_S stData[eSVMMEM_SECTION_DATA_MAX];					// information of each data

	PP_U32 u32Reserved[1];
} PP_SVMMEM_SECTION_HEADER_S;

typedef struct ppSVMMEM_VIEW_HEADER_S
{
	PP_U32 u32SectionCnt;														// Section count of view
	PP_U16 u16SectionIndexList[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];			// Section index list of view. Maximum 4.
																				// The 7th bit is status of common setion. if the 15th bit is 1, this section index is common section
	PP_POS_S stSectionPos[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];				// Section position
	PP_SVMMEM_SECTION_DATA_S stBCData[eSVMMEM_VIEW_DATA_MAX];					// Data of BC LUT && BC Additional LUT 
	PP_U32 u32DataSize;															// data size without image of common view
	PP_U32 u32IsData;															// bit control for data usage

	PP_U32 u32Reserved[1];
} PP_SVMMEM_VIEW_HEADER_S;

typedef struct ppSVMMEM_TOTAL_HEADER_S
{
	PP_SIZE_S stViewSize;														// output size
	
	PP_U32 u32ViewHeaderOffset;													// Offset address of view header
	PP_U32 u32ViewCnt;															// Count of total view
	PP_U32 u32ViewMaxDataSize;													// Max data size of view without image of common section (LUTs + shadow image + pending)
	PP_U32 u32ViewMaxSectionCnt;												// max section count of view

	PP_U32 u32SectionHeaderOffset;												// Offset address of section header
	PP_U32 u32SectionCnt;														// Count of total section
	PP_U32 u32SectionMaxCntWithoutCommon;										// section max count without common section
	PP_U32 u32SectionMaxDataSize;												// Max once data size of section

	PP_U32 u32CommonSectionCnt;													// Count of total common section.
	PP_U16 u16CommonSectionIndexList[SVMMEM_HEADER_COMMON_SECTION_COUNT_MAX];	// Offset address in flash. Maximum 4
} PP_SVMMEM_TOTAL_HEADER_S;

/************************************************************************************************
 * Merge
************************************************************************************************/
typedef struct ppSVMMEM_DATA_S
{
	PP_U32 *pu32Address;
	PP_U32 u32DataSize;
} PP_SVMMEM_DATA_S;

typedef struct ppSVMMEM_MERGE_SECTION_S
{
	PP_RECT_S stRect;
	PP_SVMMEM_DATA_S stData;
} PP_SVMMEM_MERGE_SECTION_S;

/************************************************************************************************
 * Others
************************************************************************************************/
typedef struct ppSVMMEM_LOAD_DATA_S
{
	PP_FLASH_LOADING_TYPE_E enFlash;
	PP_U32 u32Size;
	PP_U32 u32Addr;
} PP_SVMMEM_LOAD_DATA_S;

typedef struct ppSVMMEM_LOAD_S
{
	PP_SVMMEM_LOAD_STATE_E enState;
	PP_SVMMEM_LOAD_DATA_S stSrc[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];
	PP_U32 u32MergeAddr[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];
	PP_U32 u32DstAddr;
} PP_SVMMEM_LOAD_S;

typedef struct ppSVMMEM_PAGE_S
{
	PP_U32 u32Cur;
	PP_U32 u32Cnt;
	PP_U32* pu32ViewList;
	PP_BOOL* bReload;
} PP_SVMMEM_PAGE_S;

typedef struct ppSVMMEM_CRTL_S
{
	PP_BOOL bInit;
	
	PP_U32* pu32Addr[eSVMMEM_DATA_SUBTYPE_MAX];

	PP_U32* pu32CacheViewList;
	PP_U32* pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_MAX];
	
	PP_SVMMEM_PAGE_S stPage[eSVMMEM_PAGE_AREA_MAX];

	PP_SIZE_S stLutSize;
	PP_U32 u32LutDataSize;
} PP_SVMMEM_CTRL_S;


/***************************************************************************************************************************************************************
 * Global Variable
***************************************************************************************************************************************************************/
STATIC PP_SVMMEM_TOTAL_HEADER_S *gpstTotalHD;
STATIC PP_SVMMEM_VIEW_HEADER_S *gpstViewHD;
STATIC PP_SVMMEM_SECTION_HEADER_S *gpstSectionHD;

STATIC PP_RECT_S gstImgRect;
STATIC PP_RECT_S gstSectionRect;

STATIC PP_SVMMEM_CTRL_S gstSVMMEMCtrl;

/*======================================================================================================================
 = Local function
======================================================================================================================*/
STATIC PP_U32 PPAPI_SVMMEM_GetPageArea(PP_U32 IN u32View)
{
	PP_U32 i;

	for ( i = 0; i < gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].u32Cnt; i++ )
	{
		if ( u32View == gstSVMMEMCtrl.pu32CacheViewList[i] )
			return eSVMMEM_PAGE_AREA_CACHE;
	}

	return eSVMMEM_PAGE_AREA_FLIPPING;
}

STATIC PP_BOOL PPAPI_SVMMEM_SetCurPageNum(PP_SVMMEM_PAGE_AREA_E IN enPageArea, PP_U32 IN u32View, PP_U32* IN u32CurPage)
{
	PP_U32 i;

	for ( i = 0; i < gstSVMMEMCtrl.stPage[enPageArea].u32Cnt; i++ )
	{
		if ( u32View == gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[i] )
		{
			*u32CurPage = gstSVMMEMCtrl.stPage[enPageArea].u32Cur = i;
			return PP_TRUE;
		}
	}

	if ( enPageArea == eSVMMEM_PAGE_AREA_FLIPPING )
	{
		if ( gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[gstSVMMEMCtrl.stPage[enPageArea].u32Cur] != VIEWMODE_NULL )
			gstSVMMEMCtrl.stPage[enPageArea].u32Cur++;
		if ( gstSVMMEMCtrl.stPage[enPageArea].u32Cur >= gstSVMMEMCtrl.stPage[enPageArea].u32Cnt )
			gstSVMMEMCtrl.stPage[enPageArea].u32Cur = 0;
		*u32CurPage = gstSVMMEMCtrl.stPage[enPageArea].u32Cur;
	}
	else
	{
		for ( i = 0; i < gstSVMMEMCtrl.stPage[enPageArea].u32Cnt; i++ )
		{
			if ( u32View == gstSVMMEMCtrl.pu32CacheViewList[i] )
				*u32CurPage = gstSVMMEMCtrl.stPage[enPageArea].u32Cur = i;
		}
	}

	return PP_FALSE;
}

STATIC PP_BOOL PPAPI_SVMMEM_LUTMerge_SetDiscontinuity(PP_SVMMEM_MERGE_SECTION_S* IN pstSection, PP_U8 IN u8SectionCnt,
																	PP_U8 IN u8SamplingWidth, PP_U8 IN u8SamplingHeight, PP_U32* IN pu32OutData)
{
	PP_U8 u8SectionIndex0, u8SectionIndex1;
	PP_U16 u16X, u16Y;
	PP_U16 u16StartX[2], u16EndX[2], u16StartY[2], u16EndY[2];
	PP_U16 u16DelPos[4];		// [0]endx of bottom&left [1]startx of bottom&right, [2]endy of right&up, [3]starty of right&down
	PP_S16 u16AdjustX, u16AdjustY;
	PP_U32 u32DataPos;
	PP_U32 u32Value;
	PP_U32 *pu32Pos = pu32OutData;
	PP_S32 s32VoutWidth, s32VoutHeight;
	_VID_RESOL enVoutResol;
	PP_BOOL bHalfSection = PP_FALSE;
	PP_U32 u32LutHalfWidth;

	PPAPI_VIN_GetResol(BD_SVM_OUT_FMT, &s32VoutWidth, &s32VoutHeight, &enVoutResol);

	u32LutHalfWidth = (PP_U8)(s32VoutWidth / 8) / 2;

	for ( u8SectionIndex0 = 0; u8SectionIndex0 < u8SectionCnt; u8SectionIndex0++ )
	{
		u16AdjustX = u16AdjustY = 0;
		
		u16StartX[0] = pstSection[u8SectionIndex0].stRect.u16X;
		u16EndX[0] = pstSection[u8SectionIndex0].stRect.u16X + pstSection[u8SectionIndex0].stRect.u16Width - 1;
		u16StartY[0] = pstSection[u8SectionIndex0].stRect.u16Y;
		u16EndY[0] = pstSection[u8SectionIndex0].stRect.u16Y + pstSection[u8SectionIndex0].stRect.u16Height - 1;

		if ( u16EndY[0] == u8SamplingHeight - 1 )
		{
			u16DelPos[0] = 0xFFFF;	// endx
			u16DelPos[1] = 0xFFFF;	// startx
		}
		else
		{
			u16DelPos[0] = u16EndX[0];	// endx
			u16DelPos[1] = u16EndX[0];	// startx
		}

		if ( u16EndX[0] == u8SamplingWidth - 2 )
		{
			u16DelPos[2] = 0xFFFF;		// endy
			u16DelPos[3] = 0xFFFF;		// starty
		}
		else
		{
			u16DelPos[2] = u16EndY[0];	// endy
			u16DelPos[3] = u16EndY[0];	// starty
		}

		for ( u8SectionIndex1 = 0; u8SectionIndex1 < u8SectionCnt; u8SectionIndex1++ )
		{
			if ( u8SectionIndex0 == u8SectionIndex1 ) continue;
			if ( u16DelPos[0] == 0xFFFF && u16DelPos[1] == 0xFFFF && u16DelPos[2] == 0xFFFF && u16DelPos[3] == 0xFFFF ) continue;

			u16StartX[1] = pstSection[u8SectionIndex1].stRect.u16X;
			u16EndX[1] = pstSection[u8SectionIndex1].stRect.u16X + pstSection[u8SectionIndex1].stRect.u16Width - 1;
			u16StartY[1] = pstSection[u8SectionIndex1].stRect.u16Y;
			u16EndY[1] = pstSection[u8SectionIndex1].stRect.u16Y + pstSection[u8SectionIndex1].stRect.u16Height - 1;

			if ( u16EndX[0] == u16StartX[1] && u16EndX[0] != u8SamplingWidth - 2 )
			{
				if ( u16EndY[1] == u8SamplingHeight - 1 && u16StartX[1] == u32LutHalfWidth )
					bHalfSection = PP_TRUE;
				
				if ( u16StartY[0] < u16StartY[1] && u16DelPos[2] != 0xFFFF )
				{
					if ( u16StartY[1] < u16DelPos[2] )
						u16DelPos[2] = u16StartY[1] - 1;
				}
				else
				{
					u16DelPos[2] = 0xFFFF;
				}

				if ( u16EndY[0] > u16EndY[1] && u16DelPos[3] != 0xFFFF )
				{
					if ( u16EndY[1] > u16DelPos[3] || u16DelPos[3] == u16EndY[0] )
						u16DelPos[3] = u16EndY[1];
				}
				else
				{
					u16DelPos[3] = 0xFFFF;
				}

				{
					PP_U16 u16DisPos[2] = {u16StartY[1], u16EndY[1]};	// starty, starty

					if ( u16StartY[0] > u16StartY[1] ) u16DisPos[0] = u16StartY[0];
					if ( u16EndY[0] < u16EndY[1] ) u16DisPos[1] = u16EndY[0];

					for ( u16Y = u16DisPos[0]; u16Y <= u16DisPos[1]; u16Y++ )
					{
						u32DataPos = (u16StartX[1] + u8SamplingWidth * u16Y);
						if ( *(pu32Pos + u32DataPos) == 0xFFFFFFFF )
							*(pu32Pos + u32DataPos) = 0x40000000;
					}
				}
				
				u16AdjustX = -1;
			}
			else if ( u16EndY[0] == u16StartY[1] && u16EndY[0] != u8SamplingHeight - 1 )
			{
				if ( u16StartX[0] < u16StartX[1] && u16DelPos[0] != 0xFFFF )
				{
					if ( u16StartX[1] > u16DelPos[0] || u16DelPos[0] == u16EndX[0] )
						u16DelPos[0] = u16StartX[1] - 1;
				}
				else
				{
					u16DelPos[0] = 0xFFFF;
				}

				if ( u16EndX[0] > u16EndX[1] && u16DelPos[1] != 0xFFFF )
				{
					if ( u16EndX[1] < u16DelPos[1] )
						u16DelPos[1] = u16EndX[1];
				}
				else
				{
					u16DelPos[1] = 0xFFFF;
				}

				{
					PP_U16 u16DisPos[2] = {u16StartX[1], u16EndX[1]};	// startx, startx

					if ( u16StartX[0] > u16StartX[1] )	u16DisPos[0] = u16StartX[0];
					if ( u16EndX[0] < u16EndX[1] ) u16DisPos[1] = u16EndX[0];
					
					for ( u16X = u16DisPos[0]; u16X <= u16DisPos[1]; u16X++ )
					{
						u32DataPos = (u16X + u8SamplingWidth * u16StartY[1]);
						if ( *(pu32Pos + u32DataPos) == 0xFFFFFFFF )
							*(pu32Pos + u32DataPos) = 0x40000000;
					}
				}
				
				u16AdjustY = -1;
			}
		}
		
		if ( u16DelPos[0] != 0xFFFF )
		{
			for ( u16X = u16StartX[0]; u16X <= u16DelPos[0] + u16AdjustX; u16X++ )
			{
				u32DataPos = (u16X + u8SamplingWidth * u16EndY[0]);
				u32Value = *(pu32Pos + u32DataPos);
				if ( u32Value == 0xFFFFFFFF )
					*(pu32Pos + u32DataPos) = 0x40000000;
			}
		}
		
		if ( u16DelPos[1] != 0xFFFF && u16DelPos[0] != u16EndX[0] )
		{
			for ( u16X = u16DelPos[1]; u16X <= u16EndX[0] + u16AdjustX; u16X++ )
			{
				u32DataPos = (u16X + u8SamplingWidth * u16EndY[0]);
				u32Value = *(pu32Pos + u32DataPos);
				if ( u32Value == 0xFFFFFFFF )
					*(pu32Pos + u32DataPos) = 0x40000000;
			}
		}
		
		if ( u16DelPos[2] != 0xFFFF )
		{
			for ( u16Y = u16StartY[0]; u16Y <= u16DelPos[2] + u16AdjustY; u16Y++ )
			{
				u32DataPos = (u16EndX[0] + u8SamplingWidth * u16Y);
				u32Value = *(pu32Pos + u32DataPos);
				if ( u32Value == 0xFFFFFFFF )
					*(pu32Pos + u32DataPos) = 0x40000000;
			}
		}
		
		if ( u16DelPos[3] != 0xFFFF && u16DelPos[2] != u16EndY[0] )
		{
			for ( u16Y = u16DelPos[3]; u16Y <= u16EndY[0] + u16AdjustY; u16Y++ )
			{
				u32DataPos = (u16EndX[0] + u8SamplingWidth * u16Y);
				u32Value = *(pu32Pos + u32DataPos);
				if ( u32Value == 0xFFFFFFFF )
					*(pu32Pos + u32DataPos) = 0x40000000;
			}
		}
	}

	if ( bHalfSection )
	{
		PP_U32 *pu32DstAdress = pu32Pos + (u32LutHalfWidth + (u8SamplingWidth * (u8SamplingHeight - 1)));
		PP_U32 value = *pu32DstAdress;
		*pu32DstAdress = utilClearBit(value, 30);
	}

	return PP_TRUE;
}

STATIC PP_BOOL PPAPI_SVMMEM_LUTMerge_SectionMerge(PP_SVMMEM_MERGE_TYPE_E IN enType, PP_SVMMEM_MERGE_SECTION_S* IN pstSection, PP_U8 IN u8SectionCnt,
																PP_U8 IN u8SamplingWidth, PP_U8 IN u8SamplingHeight, PP_U32* IN pu32OutData, PP_BOOL IN bCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX])
{
	PP_U32 *pu32SrcPos, *pu32DstPos;
	PP_U8 i;
	PP_U16 u16Y;
	PP_U16 u16StartX, u16StartY, u16Width, u16Height;
	PP_S8 s8AdjustWidth = 0, s8AdjustHeight = 0;

	pu32DstPos = pu32OutData;
	for ( i = 0; i < u8SectionCnt; i++ )
	{
		if ( bCopySection[i] == PP_FALSE ) continue;
		
		pu32SrcPos = pstSection[i].stData.pu32Address;

		u16StartX = pstSection[i].stRect.u16X;
		u16StartY = pstSection[i].stRect.u16Y;
		u16Width = pstSection[i].stRect.u16Width;
		u16Height = pstSection[i].stRect.u16Height;
		s8AdjustWidth = s8AdjustHeight = 0;

		if ( u16StartX + u16Width != u8SamplingWidth - 1 )	s8AdjustWidth = -1;
		if ( u16StartY + u16Height != u8SamplingHeight )	s8AdjustHeight = -1;

		for ( u16Y = 0; u16Y < (u16Height + s8AdjustHeight); u16Y++ )
		{
			memcpy(pu32DstPos + (u16StartX + u8SamplingWidth * (u16Y + u16StartY)), pu32SrcPos + (0 + u16Width * u16Y), (u16Width + s8AdjustWidth) * 4);
		}
	}

	return PP_TRUE;
}

STATIC PP_BOOL PPAPI_SVMMEM_LUTMerge_Merge(PP_SVMMEM_DATA_SUBTYPE_E IN enSubType, PP_SVMMEM_MERGE_SECTION_S* IN pstSection, PP_U8 IN u8InSectionCnt, PP_U32* IN pu32OutData,
														PP_SVMMEM_PAGE_AREA_E enPageArea, PP_U32 IN u32CurPage, PP_U32 IN u32CopyPage, PP_BOOL IN bCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX], PP_U32 u32NotCopySectionCnt)
{
	PP_U32 i;
	PP_S32 s32VoutWidth, s32VoutHeight;
	_VID_RESOL enVoutResol;
	PP_SVMMEM_MERGE_TYPE_E enMergeType;
	PP_BOOL bData[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX] = {PP_FALSE, PP_FALSE, PP_FALSE, PP_FALSE};
	PP_BOOL bNewCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX] = {bCopySection[0], bCopySection[1], bCopySection[2], bCopySection[3]};
	PP_BOOL bNewload = PP_FALSE;

	if ( enSubType == eSVMMEM_DATA_SUBTYPE_FB_ODD || enSubType == eSVMMEM_DATA_SUBTYPE_FB_EVEN )
		enMergeType = eSVMMEM_MERGE_TYPE_FB;
	else if ( enSubType == eSVMMEM_DATA_SUBTYPE_LR_ODD || enSubType == eSVMMEM_DATA_SUBTYPE_LR_EVEN )
		enMergeType = eSVMMEM_MERGE_TYPE_LR;
	else if ( enSubType == eSVMMEM_DATA_SUBTYPE_BC_ODD || enSubType == eSVMMEM_DATA_SUBTYPE_BC_EVEN )
		enMergeType = eSVMMEM_MERGE_TYPE_BC;
	else
		return PP_FALSE;

	if ( !u8InSectionCnt || !pu32OutData ) return PP_FALSE;
	
	if ( pstSection )
	{
		for ( i = 0; i < u8InSectionCnt; i++ )
		{
			if ( pstSection[i].stData.pu32Address && pstSection[i].stData.u32DataSize )
				bData[i] = PP_TRUE;
			else
				bNewload = PP_TRUE;
		}

		if ( !bData[0] && !bData[1] && !bData[2] && !bData[3] )
			return PP_FALSE;

		if ( bNewload )
		{
			for ( i = 0; i < u8InSectionCnt; i++ )
			{
				if ( bData[i] )	bNewCopySection[i] = PP_TRUE;
				else			bNewCopySection[i] = PP_FALSE;
			}
		}
	}
	else
	{
		return PP_FALSE;
	}

	PPAPI_VIN_GetResol(BD_SVM_OUT_FMT, &s32VoutWidth, &s32VoutHeight, &enVoutResol);
	
	for ( i = 0; i < u8InSectionCnt; i++ )
	{
		pstSection[i].stRect.u16X = (PP_U8)(pstSection[i].stRect.u16X / 8);
		pstSection[i].stRect.u16Y = (PP_U8)(pstSection[i].stRect.u16Y / 8);
		pstSection[i].stRect.u16Width = (PP_U8)(pstSection[i].stRect.u16Width / 8) + 1;
		pstSection[i].stRect.u16Height = (PP_U8)(pstSection[i].stRect.u16Height / 8) + 1;
	}

	if ( u32CopyPage == SVMMEM_PAGE_NULL || bNewload )
	{
		if (  u32NotCopySectionCnt == 0 || bNewload )
		{
			if ( enMergeType == eSVMMEM_MERGE_TYPE_BC )
			{
				for ( i = 0; i < gstSVMMEMCtrl.stLutSize.u16Width * gstSVMMEMCtrl.stLutSize.u16Height; i++ )
					*(pu32OutData + i) = 0x09000000;
			}
			else
			{
				memset(pu32OutData, 0xFF, gstSVMMEMCtrl.stLutSize.u16Width * gstSVMMEMCtrl.stLutSize.u16Height * 4);
			}
		}
	}
	else
	{
		PP_U32 *pu32SrcAddr, *pu32DstAddr;
		
		if ( enPageArea == eSVMMEM_PAGE_AREA_FLIPPING )
			pu32SrcAddr = (PP_U32*)(gstDramReserved.u32SvmData[u32CopyPage] + ((PP_U32)pu32OutData - gstDramReserved.u32SvmData[u32CurPage]));
		else
			pu32SrcAddr = (PP_U32*)(gstDramReserved.u32SvmCacheData[u32CopyPage] + ((PP_U32)pu32OutData - gstDramReserved.u32SvmCacheData[u32CurPage]));
		pu32DstAddr = pu32OutData;

		if ( PPDRV_DMA_M2M_Word(DMA_MAX_CHANNEL - 1, pu32SrcAddr, pu32DstAddr, gstSVMMEMCtrl.stLutSize.u16Width * gstSVMMEMCtrl.stLutSize.u16Height * 4, 0))
		{
			printf("[SVMMEM] DMA Error\n");
			return PP_FALSE;
		}

		do {
			asm volatile("nop");
		} while (PPDRV_DMA_GetDone(DMA_MAX_CHANNEL - 1, PP_NULL) == 0 );
	}
	
	PPAPI_SVMMEM_LUTMerge_SectionMerge(enMergeType, pstSection, u8InSectionCnt, gstSVMMEMCtrl.stLutSize.u16Width, gstSVMMEMCtrl.stLutSize.u16Height, pu32OutData, bNewCopySection);

	if ( enMergeType != eSVMMEM_MERGE_TYPE_BC )
		PPAPI_SVMMEM_LUTMerge_SetDiscontinuity(pstSection, u8InSectionCnt, gstSVMMEMCtrl.stLutSize.u16Width, gstSVMMEMCtrl.stLutSize.u16Height, pu32OutData);

	PPAPI_SYS_CACHE_Writeback(pu32OutData, gstSVMMEMCtrl.u32LutDataSize);

	return PP_TRUE;
}

STATIC PP_BOOL PPAPI_SVMMEM_LUTMerge_CheckSectionRect(PP_U32 IN u32SetView, PP_U32 IN u32PageView, PP_BOOL OUT bCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX], PP_U32* OUT u32NotCopySectionCnt)
{
	PP_U32 i, k;
	PP_U32 u32SameLayoutCnt = 0;
	PP_U32 u32ViewSectionIndex, u32PageSectionIndex;

	for ( i = 0; i < SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX; i++ ) bCopySection[i] = PP_TRUE;
	*u32NotCopySectionCnt = 0;

	if ( u32SetView == VIEWMODE_NULL || u32PageView == VIEWMODE_NULL || gpstViewHD[u32SetView].u32SectionCnt != gpstViewHD[u32PageView].u32SectionCnt )
		return PP_FALSE;

	for ( i = eSVMMEM_DATA_SUBTYPE_FB_ODD; i <= eSVMMEM_DATA_SUBTYPE_BC_EVEN; i++ )
	{	
		if ( utilGetBit(gpstViewHD[u32SetView].u32IsData, i) != utilGetBit(gpstViewHD[u32PageView].u32IsData, i) )
			return PP_FALSE;
	}

	for ( i = 0; i < gpstViewHD[u32SetView].u32SectionCnt; i++ )
	{
		u32ViewSectionIndex = VIEWSECTIONINDEX(gpstViewHD[u32SetView].u16SectionIndexList[i]);
		
		for ( k = 0; k < gpstViewHD[u32PageView].u32SectionCnt; k++ )
		{
			u32PageSectionIndex = VIEWSECTIONINDEX(gpstViewHD[u32PageView].u16SectionIndexList[k]);
			
			if ( gpstViewHD[u32SetView].stSectionPos[i].u16X == gpstViewHD[u32PageView].stSectionPos[k].u16X &&
				 gpstViewHD[u32SetView].stSectionPos[i].u16Y == gpstViewHD[u32PageView].stSectionPos[k].u16Y &&
				 gpstSectionHD[u32ViewSectionIndex].stSectionSize.u16Width == gpstSectionHD[u32PageSectionIndex].stSectionSize.u16Width &&
				 gpstSectionHD[u32ViewSectionIndex].stSectionSize.u16Height == gpstSectionHD[u32PageSectionIndex].stSectionSize.u16Height )
			{
				u32SameLayoutCnt++;
				if ( u32ViewSectionIndex == u32PageSectionIndex )
				{
					bCopySection[i] = PP_FALSE;
					(*u32NotCopySectionCnt)++;
				}
				break;
			}
		}
	}

	if ( gpstViewHD[u32SetView].u32SectionCnt == u32SameLayoutCnt )
		return PP_TRUE;		// same layout

	for ( i = 0; i < SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX; i++ ) bCopySection[i] = PP_FALSE;

	return PP_FALSE;		// different layout
}

STATIC PP_VOID PPAPI_SVMMEM_LUTMerge_CheckSection(PP_U32 IN u32SetView, PP_SVMMEM_PAGE_AREA_E IN enPageArea, PP_U32 IN u32PageIndex, PP_U32* OUT u32CopyPage,
																PP_BOOL OUT bCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX], PP_U32* OUT u32NotCopySectionCnt)
{
	PP_U32 i;
	PP_U32 u32PageView;
	PP_U32 u32SetViewCommonSectionCnt = 0;
	PP_U32 u32TempNotCopySectionCnt = 0;
	PP_BOOL bTempCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];

	*u32CopyPage = SVMMEM_PAGE_NULL;
	*u32NotCopySectionCnt = 0;
	for ( i = 0; i < SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX; i++ ) bCopySection[i] = PP_TRUE;

	for ( i = 0; i < gpstViewHD[u32SetView].u32SectionCnt; i++ )
	{
		if ( ISVIEWCOMMONSECTIONINDEX(gpstViewHD[u32SetView].u16SectionIndexList[i]) )
			u32SetViewCommonSectionCnt++;
	}

	if ( !gstSVMMEMCtrl.stPage[enPageArea].bReload[u32PageIndex] && gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[u32PageIndex] != VIEWMODE_NULL )
	{
		u32PageView = gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[u32PageIndex];
		if ( u32PageView != VIEWMODE_NULL && PPAPI_SVMMEM_LUTMerge_CheckSectionRect(u32SetView, u32PageView, bTempCopySection, &u32TempNotCopySectionCnt) )
		{
			if ( u32TempNotCopySectionCnt == u32SetViewCommonSectionCnt )
			{
				memcpy(bCopySection, bTempCopySection, sizeof(PP_BOOL) * SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX);
				*u32NotCopySectionCnt = u32TempNotCopySectionCnt;
				return;
			}
		}
	}

	for ( i = 0; i < gstSVMMEMCtrl.stPage[enPageArea].u32Cnt; i++ )
	{
		if ( i == u32PageIndex ) continue;
		
		u32PageView = gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[i];
		if ( u32PageView == VIEWMODE_NULL ) return;

		if ( !gstSVMMEMCtrl.stPage[enPageArea].bReload[i] && PPAPI_SVMMEM_LUTMerge_CheckSectionRect(u32SetView, u32PageView, bTempCopySection, &u32TempNotCopySectionCnt) )
		{
			memcpy(bCopySection, bTempCopySection, sizeof(PP_BOOL) * SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX);
			*u32NotCopySectionCnt = u32TempNotCopySectionCnt;
			*u32CopyPage = i;
			
			if ( *u32NotCopySectionCnt == u32SetViewCommonSectionCnt ) return;
		}
	}
}

STATIC PP_S32 PPAPI_SVMMEM_ViewTypeToSectionType(PP_SVMMEM_DATA_SUBTYPE_E enViewType)
{
	PP_S32 s32Ret = -1;
	
	switch (enViewType)
	{
	case eSVMMEM_DATA_SUBTYPE_FB_ODD:
	case eSVMMEM_DATA_SUBTYPE_FB_EVEN:
	case eSVMMEM_DATA_SUBTYPE_LR_ODD:
	case eSVMMEM_DATA_SUBTYPE_LR_EVEN:
	case eSVMMEM_DATA_SUBTYPE_BC_ODD:
	case eSVMMEM_DATA_SUBTYPE_BC_EVEN:
		s32Ret = enViewType;
		break;
	case eSVMMEM_DATA_SUBTYPE_IMG0_ODD:
	case eSVMMEM_DATA_SUBTYPE_IMG1_ODD:
		s32Ret = eSVMMEM_SECTION_DATA_IMG_ODD;
		break;
	case eSVMMEM_DATA_SUBTYPE_IMG0_EVEN:
	case eSVMMEM_DATA_SUBTYPE_IMG1_EVEN:
		s32Ret = eSVMMEM_SECTION_DATA_IMG_EVEN;
		break;
	default:
		break;
	}

	return s32Ret;
}

STATIC PP_S32 PPAPI_SVMMEM_ViewTypeToViewType(PP_SVMMEM_DATA_SUBTYPE_E enViewType)
{
	PP_S32 s32Ret = -1;
	
	switch (enViewType)
	{
	case eSVMMEM_DATA_SUBTYPE_BC_ODD:
	case eSVMMEM_DATA_SUBTYPE_BC_EVEN:
	case eSVMMEM_DATA_SUBTYPE_A0S0_ODD:
	case eSVMMEM_DATA_SUBTYPE_A0S0_EVEN:
	case eSVMMEM_DATA_SUBTYPE_A0S1_ODD:
	case eSVMMEM_DATA_SUBTYPE_A0S1_EVEN:
	case eSVMMEM_DATA_SUBTYPE_A1S0_ODD:
	case eSVMMEM_DATA_SUBTYPE_A1S0_EVEN:
	case eSVMMEM_DATA_SUBTYPE_A1S1_ODD:
	case eSVMMEM_DATA_SUBTYPE_A1S1_EVEN:
	case eSVMMEM_DATA_SUBTYPE_BGS0_ODD:
	case eSVMMEM_DATA_SUBTYPE_BGS0_EVEN:
	case eSVMMEM_DATA_SUBTYPE_BGS1_ODD:
	case eSVMMEM_DATA_SUBTYPE_BGS1_EVEN:
		s32Ret = enViewType - eSVMMEM_DATA_SUBTYPE_BC_ODD;
		break;
	default:
		break;
	}

	return s32Ret;
}

STATIC PP_BOOL PPAPI_SVMMEM_GetLoadDataInfo(PP_VIEWMODE_E enView, PP_SVMMEM_LOAD_S stLoad[eSVMMEM_DATA_SUBTYPE_MAX], PP_U32 u32DstAddr)
{
	PP_U32 i;
	PP_U32 u32Type, u32Section;
	PP_BOOL bBCAddLUT = PP_FALSE;
	PP_BOOL bCommonSection = PP_FALSE;
	PP_U16 u16CommonSectionDataIndex, u16ViewSection;
	PP_U32 u32FlashAddr[eFLASH_LOADING_TYPE_MAX];
	PP_S32 s32ViewType, s32SectionType;
	PP_U8 u8ImageCnt = 0;
	PP_U32 u32DstOffset = 0;
	PP_U8 u8MergeDataIndex = 0;
	PP_FLASH_LOADING_TYPE_E enFlash;
	PP_SVMMEM_PAGE_AREA_E enPageArea;

	memset(stLoad, 0, sizeof(PP_SVMMEM_LOAD_S) * eSVMMEM_DATA_SUBTYPE_MAX);

	enPageArea = PPAPI_SVMMEM_GetPageArea(enView);
	if ( enPageArea == eSVMMEM_PAGE_AREA_CACHE )
		u8MergeDataIndex += PPAPI_SVMMEM_GetSectionMergeMaxCount();

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	u32FlashAddr[eFLASH_LOADING_TYPE_NOR] = gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr;
#else
	u32FlashAddr[eFLASH_LOADING_TYPE_FTL] = gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr;
	u32FlashAddr[eFLASH_LOADING_TYPE_NONFTL] = gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr;
#endif

	bBCAddLUT =  gpstViewHD[enView].stBCData[eSVMMEM_VIEW_DATA_BC_ODD].u32DataSize;

	for ( u32Type = eSVMMEM_DATA_SUBTYPE_FB_ODD; u32Type <= eSVMMEM_DATA_SUBTYPE_IMG0_EVEN; u32Type++ )
	{
		s32ViewType = PPAPI_SVMMEM_ViewTypeToViewType(u32Type);
		s32SectionType = PPAPI_SVMMEM_ViewTypeToSectionType(u32Type);
		
		if ( utilGetBit(gpstViewHD[enView].u32IsData, u32Type) )
		{
			if ( bBCAddLUT && eSVMMEM_DATA_SUBTYPE_BC_ODD <= u32Type && u32Type <= eSVMMEM_DATA_SUBTYPE_BGS1_EVEN )
			{
				stLoad[u32Type].stSrc[0].u32Size = gpstViewHD[enView].stBCData[s32ViewType].u32DataSize;
				if ( stLoad[u32Type].stSrc[0].u32Size )
				{
					enFlash = gpstViewHD[enView].stBCData[s32ViewType].u32StoredAear;
					stLoad[u32Type].stSrc[0].enFlash = enFlash;
					stLoad[u32Type].stSrc[0].u32Addr = u32FlashAddr[enFlash] + gpstViewHD[enView].stBCData[s32ViewType].u32AddrOffset;
					stLoad[u32Type].enState = eSVMMEM_LOAD_STATE_COPY;
					stLoad[u32Type].u32DstAddr = u32DstAddr + u32DstOffset;
					u32DstOffset += stLoad[u32Type].stSrc[0].u32Size;
				}
				continue;
			}
			
			u8MergeDataIndex = 0;
			for ( u32Section = 0; u32Section < gpstViewHD[enView].u32SectionCnt; u32Section++ )
			{
				bCommonSection = ISVIEWCOMMONSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[u32Section]);
				u16ViewSection = VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[u32Section]);
				
				stLoad[u32Type].stSrc[u32Section].u32Size = gpstSectionHD[u16ViewSection].stData[s32SectionType].u32DataSize;
			
				if ( stLoad[u32Type].stSrc[u32Section].u32Size )
				{
					if ( u32Type == eSVMMEM_DATA_SUBTYPE_IMG0_ODD ) u8ImageCnt++;
					if ( u8ImageCnt > 2 ) continue;
					
					if ( bCommonSection )
					{
						if ( !gstDramReserved.u32SvmCommonSectionData ) return PP_FALSE;
						
						for ( i = 0; i < gpstTotalHD->u32CommonSectionCnt; i++ )
						{
							if ( gpstTotalHD->u16CommonSectionIndexList[i] == u16ViewSection )
							{
								u16CommonSectionDataIndex = i;
								break;
							}
						}
						
						if ( u32Type >= eSVMMEM_DATA_SUBTYPE_IMG0_ODD )
						{
							PP_U32 u32ImageType = u32Type;
							if ( u8ImageCnt == 2 ) u32ImageType = u32Type + 2;
							stLoad[u32ImageType].stSrc[u32Section].enFlash = eFLASH_LOADING_TYPE_MAX;
							stLoad[u32ImageType].stSrc[u32Section].u32Size = stLoad[u32Type].stSrc[u32Section].u32Size;
							stLoad[u32ImageType].stSrc[u32Section].u32Addr = gstDramReserved.u32SvmCommonSectionData[u16CommonSectionDataIndex][s32SectionType];
							stLoad[u32ImageType].u32DstAddr = gstDramReserved.u32SvmCommonSectionData[u16CommonSectionDataIndex][s32SectionType];
							stLoad[u32ImageType].enState = eSVMMEM_LOAD_STATE_NULL;
						}
						else
						{
							stLoad[u32Type].stSrc[u32Section].enFlash = eFLASH_LOADING_TYPE_MAX;
							stLoad[u32Type].stSrc[u32Section].u32Addr = gstDramReserved.u32SvmCommonSectionData[u16CommonSectionDataIndex][s32SectionType];
							stLoad[u32Type].enState = eSVMMEM_LOAD_STATE_MERGE;
						}
					}
					else
					{
						enFlash = gpstSectionHD[u16ViewSection].stData[s32SectionType].u32StoredAear;
						if ( u32Type >= eSVMMEM_DATA_SUBTYPE_IMG0_ODD )
						{
							PP_U32 u32ImageType = u32Type;
							if ( u8ImageCnt == 2 ) u32ImageType = u32Type + 2;
							stLoad[u32ImageType].stSrc[u32Section].enFlash = enFlash;
							stLoad[u32ImageType].stSrc[u32Section].u32Size = stLoad[u32Type].stSrc[u32Section].u32Size;
							stLoad[u32ImageType].stSrc[u32Section].u32Addr = u32FlashAddr[enFlash] + gpstSectionHD[u16ViewSection].stData[s32SectionType].u32AddrOffset;
							stLoad[u32ImageType].enState = eSVMMEM_LOAD_STATE_COPY;
							stLoad[u32ImageType].u32DstAddr = u32DstAddr + u32DstOffset;
							u32DstOffset += stLoad[u32Type].stSrc[u32Section].u32Size;
						}
						else
						{
							if ( !gstDramReserved.u32SVMSectionMergeData ) return PP_FALSE;
							
							stLoad[u32Type].stSrc[u32Section].enFlash = enFlash;
							stLoad[u32Type].stSrc[u32Section].u32Addr = u32FlashAddr[enFlash] + gpstSectionHD[u16ViewSection].stData[s32SectionType].u32AddrOffset;
							stLoad[u32Type].u32MergeAddr[u32Section] = gstDramReserved.u32SVMSectionMergeData[u8MergeDataIndex++];
							stLoad[u32Type].enState = eSVMMEM_LOAD_STATE_MERGE;
						}
					}
				}
			}

			if ( stLoad[u32Type].enState == eSVMMEM_LOAD_STATE_MERGE )
			{
				stLoad[u32Type].u32DstAddr = u32DstAddr + u32DstOffset;
				u32DstOffset += gstSVMMEMCtrl.u32LutDataSize;
			}
		}
		else
		{
			stLoad[u32Type].u32DstAddr = 0;
		}
	}

	return PP_TRUE;
}

/*======================================================================================================================
 = Export function
======================================================================================================================*/
PP_U32* PPAPI_SVMMEM_GetInputFrameBufferAddr(PP_CHANNEL_E IN enChannel)
{
	return (PP_U32*)gstDramReserved.u32SvmInBuff[enChannel];
}

PP_U32* PPAPI_SVMMEM_GetOutputFrameBufferAddr(PP_SVMMEM_OUT_FRAMEBUF_NUM_E IN enOutFBNum)
{
	if ( enOutFBNum >= SVMMEM_OUT_BUFFER_COUNT ) return PP_NULL;
	return (PP_U32*)gstDramReserved.u32SvmOutBuff[enOutFBNum];
}

PP_U32* PPAPI_SVMMEM_GetLoadedFBLUTAddress(PP_FIELD_E IN enField)
{
	if ( enField == eFIELD_ODD )
		return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_FB_ODD];
	else
		return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_FB_EVEN];
}

PP_U32* PPAPI_SVMMEM_GetLoadedLRLUTAddress(PP_FIELD_E IN enField)
{
	if ( enField == eFIELD_ODD )
		return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_LR_ODD];
	else
		return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_LR_EVEN];
}

PP_U32* PPAPI_SVMMEM_GetLoadedBCLUTAddress(PP_FIELD_E IN enField)
{
	if ( enField == eFIELD_ODD )
		return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_BC_ODD];
	else
		return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_BC_EVEN];
}

PP_U32* PPAPI_SVMMEM_GetLoadedBCAddLUTAddress(PP_SVMMEM_BC_ADDITIONAL_LUT_E IN enType, PP_SVMMEM_BC_ADDITIONAL_LUT_SUBCORE_E IN enSubCore, PP_FIELD_E IN enField)
{
	PP_U32 u32Pos;

	switch ( enType )
	{
	case eSVMMEM_BC_ADD_LUT_ALPHA_0:
		u32Pos = eSVMMEM_DATA_SUBTYPE_A0S0_ODD;
		break;
	case eSVMMEM_BC_ADD_LUT_ALPHA_1:
		u32Pos = eSVMMEM_DATA_SUBTYPE_A1S0_ODD;
		break;
	case eSVMMEM_BC_ADD_LUT_GRADIENT:
		u32Pos = eSVMMEM_DATA_SUBTYPE_BGS0_ODD;
		break;
	default:
		return PP_NULL;
	}

	if ( enSubCore == eSVMMEM_BC_ADD_LUT_SUBCORE_1 )
		u32Pos += 2;

	if ( enField == eFIELD_EVEN )
		u32Pos += 1;
	
	return gstSVMMEMCtrl.pu32Addr[u32Pos];
}

PP_U32* PPAPI_SVMMEM_GetLoadedImageAddress(PP_SVMDRV_IMG_NUMBER_E enImgNum, PP_FIELD_E IN enField)
{
	if ( enImgNum == eSVMDRV_IMG_NUM_0 )
	{
		if ( enField == eFIELD_ODD )
			return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_IMG0_ODD];
		else
			return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_IMG0_EVEN];
	}
	else if ( enImgNum == eSVMDRV_IMG_NUM_1 )
	{
		if ( enField == eFIELD_ODD )
			return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_IMG1_ODD];
		else
			return gstSVMMEMCtrl.pu32Addr[eSVMMEM_DATA_SUBTYPE_IMG1_EVEN];
	}

	return PP_NULL;
}

PP_BOOL PPAPI_SVMMEM_LoadCommonSectionData(PP_S32 u32SectionNum, PP_BOOL bOnlyViewLUT)
{
	PP_U32 i, j;
	PP_U32 u32CommonSectionCnt = 0;
	PP_U16* u16CommonSectionList;
	PP_U32 u32SectionIndex;
	PP_U32 u32SrcAddr, u32SrcSize;
	PP_U8 *pu8DstAddr;
	STATIC PP_BOOL bFirstCall = PP_TRUE;
		
	if ( gstDramReserved.u32SvmCommonSectionData )
	{
		u32CommonSectionCnt = PPAPI_SVMMEM_GetCommonSectionCount();
		u16CommonSectionList = PPAPI_SVMMEM_GetCommonSectionList();
		for ( i = 0; i < u32CommonSectionCnt; i++ )
		{
			if ( gstDramReserved.u32SvmCommonSectionData[i] )
			{
				u32SectionIndex = u16CommonSectionList[i];
				if ( u32SectionNum >= 0 && u32SectionIndex != u32SectionNum ) continue;
				
				for ( j = eSVMMEM_SECTION_DATA_FB_ODD; j < eSVMMEM_SECTION_DATA_MAX; j++ )
				{
					if ( bOnlyViewLUT && j >= eSVMMEM_SECTION_DATA_BC_ODD ) continue;
					
					u32SrcSize = gpstSectionHD[u32SectionIndex].stData[j].u32DataSize;
					pu8DstAddr = (PP_U8*)gstDramReserved.u32SvmCommonSectionData[i][j];
					if ( u32SrcSize )
					{
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
						u32SrcAddr = gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstSectionHD[u32SectionIndex].stData[j].u32AddrOffset;
						PPAPI_FLASH_ReadQDMA(pu8DstAddr, u32SrcAddr, u32SrcSize);
						
#else
						if ( gpstSectionHD[u32SectionIndex].stData[j].u32StoredAear == eFLASH_LOADING_TYPE_FTL )
						{
							u32SrcAddr = gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstSectionHD[u32SectionIndex].stData[j].u32AddrOffset;
							if ( PPAPI_FTL_Read(pu8DstAddr, u32SrcAddr, u32SrcSize) != 0  )
							{
								LOG_CRITICAL("[SVMMEM] (%s, %d) : Copy failed\n", __FUNCTION__, __LINE__);
								return PP_FALSE;
							}
							PPAPI_SYS_CACHE_Writeback((PP_U32*)pu8DstAddr, u32SrcSize);
						}
						else
						{
							u32SrcAddr = gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstSectionHD[u32SectionIndex].stData[j].u32AddrOffset;
							PPAPI_FLASH_ReadQDMA(pu8DstAddr, u32SrcAddr, u32SrcSize);
						}
#endif
					}
				}
			}
		}

		if ( !bFirstCall )
		{
			for ( i = eSVMMEM_PAGE_AREA_FLIPPING; i < eSVMMEM_PAGE_AREA_MAX; i++ )
			{
				for ( j = 0; j < gstSVMMEMCtrl.stPage[i].u32Cnt; j++ )
					gstSVMMEMCtrl.stPage[i].bReload[j] = PP_TRUE;
			}
		}
		
		bFirstCall = PP_FALSE;
	}

	return PP_TRUE;
}

PP_SVMMEM_LOAD_RET_E PPAPI_SVMMEM_LoadData(PP_VIEWMODE_E IN enView, PP_BOOL IN bLoading)
{
	PP_U32 i, j;
	PP_SVMMEM_PAGE_AREA_E enPageArea;
	PP_BOOL bExistence = PP_FALSE;

	PP_U16 u16SectionIndex;
	PP_U32 u32PageAddr = 0;
	PP_U32 u32SrcAddr, u32SrcSize, u32DstAddr;
	PP_SVMMEM_MERGE_SECTION_S stSectionMerge[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];
	PP_SVMMEM_LOAD_S stLoad[eSVMMEM_DATA_SUBTYPE_MAX];
	PP_U32 u32CurPage, u32PageIndex;
	PP_U32 u32CopyPage = 0;
	PP_BOOL bCopySection[SVMMEM_HEADER_VIEW_SECTION_COUNT_MAX];
	PP_U32 u32NotCopySectionCnt = 0;
	
	if ( !gpstTotalHD ) return eSVMMEM_LOAD_RET_FAILURE;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt || !gpstViewHD[enView].u32SectionCnt ) return eSVMMEM_LOAD_RET_FAILURE;
	if ( !gpstSectionHD ) return eSVMMEM_LOAD_RET_FAILURE;

	if ( enView >= eVIEWMODE_LOAD_MAX ) return eSVMMEM_LOAD_RET_FAILURE;

	enPageArea = PPAPI_SVMMEM_GetPageArea(enView);
	bExistence = PPAPI_SVMMEM_SetCurPageNum(enPageArea, enView, &u32CurPage);
	
	if ( gstSVMMEMCtrl.stPage[enPageArea].bReload[u32CurPage] )
	{
		bLoading = PP_TRUE;
		bExistence = PP_FALSE;
	}
	
	if ( !bExistence && !bLoading )
	{
		printf("[SVMMEM] (%s, %d) : %d viewmode data does not exist in dram memroy\n", __FUNCTION__, __LINE__, enView);
		return eSVMMEM_LOAD_RET_FAILURE;
	}
	else if ( bExistence )
	{
		return eSVMMEM_LOAD_RET_SUCCESS_STORE;
	}
	
	if ( enPageArea == eSVMMEM_PAGE_AREA_FLIPPING )
	{
		if ( !gstDramReserved.u32SvmData[u32CurPage] )
		{
			printf("[SVMMEM] (%s, %d) : Page address is NULL\n", __FUNCTION__, __LINE__);
			return eSVMMEM_LOAD_RET_FAILURE;
		}
		u32PageAddr = gstDramReserved.u32SvmData[u32CurPage];
		u32PageIndex = u32CurPage;
	}
	else
	{
		if ( !gstDramReserved.u32SvmCacheData )
		{
			printf("[SVMMEM] (%s, %d) : Page address is NULL\n", __FUNCTION__, __LINE__);
			return eSVMMEM_LOAD_RET_FAILURE;
		}
		u32PageAddr = gstDramReserved.u32SvmCacheData[u32CurPage];
		u32PageIndex = enView - gstSVMMEMCtrl.pu32CacheViewList[0];
	}

	if ( !PPAPI_SVMMEM_GetLoadDataInfo(enView, stLoad, u32PageAddr) )
	{
		printf("[SVMMEM] (%s, %d) : Reserved memory does not exist in dram memroy\n", __FUNCTION__, __LINE__);
		return eSVMMEM_LOAD_RET_FAILURE;
	}

	if ( !bExistence && bLoading )
	{
		PPAPI_SVMMEM_LUTMerge_CheckSection(enView, enPageArea, u32PageIndex, &u32CopyPage, bCopySection, &u32NotCopySectionCnt);
		
		for ( i = eSVMMEM_DATA_SUBTYPE_FB_ODD; i < eSVMMEM_DATA_SUBTYPE_MAX; i++ )
		{
			if ( stLoad[i].enState == eSVMMEM_LOAD_STATE_MERGE )
			{
				for ( j = 0; j < gpstViewHD[enView].u32SectionCnt; j++ )
				{
					u16SectionIndex = VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[j]);
					
					stSectionMerge[j].stRect.u16X = gpstViewHD[enView].stSectionPos[j].u16X;
					stSectionMerge[j].stRect.u16Y = gpstViewHD[enView].stSectionPos[j].u16Y;
					stSectionMerge[j].stRect.u16Width = gpstSectionHD[u16SectionIndex].stSectionSize.u16Width;
					stSectionMerge[j].stRect.u16Height = gpstSectionHD[u16SectionIndex].stSectionSize.u16Height;

					stSectionMerge[j].stData.u32DataSize = stLoad[i].stSrc[j].u32Size;
					
					if ( stLoad[i].stSrc[j].enFlash != eFLASH_LOADING_TYPE_MAX )
					{
						stSectionMerge[j].stData.pu32Address = (PP_U32*)stLoad[i].u32MergeAddr[j];
						
						u32SrcSize = stLoad[i].stSrc[j].u32Size;
						u32SrcAddr = stLoad[i].stSrc[j].u32Addr;
						u32DstAddr = stLoad[i].u32MergeAddr[j];
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
						PPAPI_FLASH_ReadQDMA((PP_U8*)u32DstAddr, u32SrcAddr, u32SrcSize);
#else
						if ( stLoad[i].stSrc[j].enFlash == eFLASH_LOADING_TYPE_FTL )
						{
							if ( PPAPI_FTL_Read((PP_U8*)u32DstAddr, u32SrcAddr, u32SrcSize) != 0 )
							{
								printf("[SVMMEM] (%s, %d) : Copy failed\n", __FUNCTION__, __LINE__);
								return eSVMMEM_LOAD_RET_FAILURE;
							}
							PPAPI_SYS_CACHE_Writeback((PP_U32*)u32DstAddr, u32SrcSize);
						}
						else
						{
							PPAPI_FLASH_ReadQDMA((PP_U8*)u32DstAddr, u32SrcAddr, u32SrcSize);
						}
#endif
					}
					else
					{
						stSectionMerge[j].stData.pu32Address = (PP_U32*)stLoad[i].stSrc[j].u32Addr;
					}
				}
				
				PPAPI_SVMMEM_LUTMerge_Merge(i, stSectionMerge, gpstViewHD[enView].u32SectionCnt, (PP_U32*)stLoad[i].u32DstAddr, enPageArea, u32PageIndex, u32CopyPage, bCopySection, u32NotCopySectionCnt);
			}
			else if ( stLoad[i].enState == eSVMMEM_LOAD_STATE_COPY )
			{
				for ( j = 0; j < gpstViewHD[enView].u32SectionCnt; j++ )
				{
					u32SrcSize = stLoad[i].stSrc[j].u32Size;
					u32SrcAddr = stLoad[i].stSrc[j].u32Addr;
					u32DstAddr = stLoad[i].u32DstAddr;
					if ( u32SrcSize && u32SrcAddr && u32DstAddr )
					{
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
						PPAPI_FLASH_ReadQDMA((PP_U8*)u32DstAddr, u32SrcAddr, u32SrcSize);
#else
						if ( stLoad[i].stSrc[j].enFlash == eFLASH_LOADING_TYPE_FTL )
						{
							if ( PPAPI_FTL_Read((PP_U8*)u32DstAddr, u32SrcAddr, u32SrcSize) != 0 )
							{
								printf("[SVMMEM] (%s, %d) : Copy failed\n", __FUNCTION__, __LINE__);
								return eSVMMEM_LOAD_RET_FAILURE;
							}
							PPAPI_SYS_CACHE_Writeback((PP_U32*)u32DstAddr, u32SrcSize);
						}
						else
						{
							PPAPI_FLASH_ReadQDMA((PP_U8*)u32DstAddr, u32SrcAddr, u32SrcSize);
						}
#endif
					}
				}
			}
		}

		gstSVMMEMCtrl.stPage[enPageArea].bReload[u32CurPage] = PP_FALSE;
	}

	if ( enPageArea == eSVMMEM_PAGE_AREA_CACHE && bLoading )
	{
		gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[u32PageIndex] = enView;
		for ( i = eSVMMEM_DATA_SUBTYPE_FB_ODD; i < eSVMMEM_DATA_SUBTYPE_MAX; i++ )
			gstSVMMEMCtrl.pu32CacheViewAddr[i] = (PP_U32*)stLoad[i].u32DstAddr;
	}
	else
	{
		for ( i = eSVMMEM_DATA_SUBTYPE_FB_ODD; i < eSVMMEM_DATA_SUBTYPE_MAX; i++ )
			gstSVMMEMCtrl.pu32Addr[i] = (PP_U32*)stLoad[i].u32DstAddr;
		
		gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[u32PageIndex] = enView;
	}
	
	return eSVMMEM_LOAD_RET_SUCCESS_LOAD;
}

PP_U32 PPAPI_SVMMEM_GetSectionCount(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32SectionCnt;
}

PP_S32 PPAPI_SVMMEM_GetSectionNumber(PP_VIEWMODE_E IN enView, PP_RECT_S IN stSectionRect)
{
	PP_U32 i;
	PP_U16 u16SectionIndex = 0;

	if ( !gpstTotalHD ) return -1;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return -1;
	if ( !gpstSectionHD ) return -1;

	for ( i = 0; i < gpstViewHD[enView].u32SectionCnt; i++ )
	{
		u16SectionIndex = VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[i]);

		if ( gpstViewHD[enView].stSectionPos[i].u16X == stSectionRect.u16X &&
			 gpstViewHD[enView].stSectionPos[i].u16Y == stSectionRect.u16Y &&
			 gpstSectionHD[u16SectionIndex].stSectionSize.u16Width == stSectionRect.u16Width &&
			 gpstSectionHD[u16SectionIndex].stSectionSize.u16Height == stSectionRect.u16Height )
		{
			return u16SectionIndex;
		}
	}
	
	return -1;
}

PP_U32 PPAPI_SVMMEM_GetSectionDataSize(PP_U32 IN u32SectionIndex, PP_SVMMEM_SECTION_DATA_E IN enType)
{
	if ( !gpstTotalHD ) return 0;
	if ( !gpstSectionHD || u32SectionIndex >= gpstTotalHD->u32SectionCnt ) return 0;
	return gpstSectionHD[u32SectionIndex].stData[enType].u32DataSize;
}

PP_U32* PPAPI_SVMMEM_GetSectionAddress(PP_U32 IN u32SectionIndex, PP_SVMMEM_SECTION_DATA_E IN enType)
{
	if ( !gpstTotalHD ) return PP_NULL;
	if ( !gpstSectionHD || u32SectionIndex >= gpstTotalHD->u32SectionCnt ) return PP_NULL;
	if ( !gpstSectionHD[u32SectionIndex].stData[enType].u32DataSize ) return PP_NULL;
	return (PP_U32*)gpstSectionHD[u32SectionIndex].stData[enType].u32AddrOffset;
}

PP_U32 PPAPI_SVMMEM_GetSectionMaxDataSize(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32SectionMaxDataSize;
}

PP_U32 PPAPI_SVMMEM_GetSectionMergeMaxCount(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32SectionMaxCntWithoutCommon;
}

PP_U32 PPAPI_SVMMEM_GetCommonSectionCount(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32CommonSectionCnt;
}

PP_U16* PPAPI_SVMMEM_GetCommonSectionList(PP_VOID)
{
	if ( !gpstTotalHD ) return PP_NULL;
	return gpstTotalHD->u16CommonSectionIndexList;
}

PP_BOOL PPAPI_SVMMEM_IsCommonSection(PP_U32 IN u32SectionIndex)
{
	PP_U32 i;
	
	if ( !gpstTotalHD ) return PP_FALSE;
	if ( u32SectionIndex >= gpstTotalHD->u32SectionCnt ) return PP_FALSE;
	
	for ( i = 0; i < gpstTotalHD->u32CommonSectionCnt; i++ )
	{
		if ( u32SectionIndex == gpstTotalHD->u16CommonSectionIndexList[i] )
			return PP_TRUE;
	}

	return PP_FALSE;
}

PP_U32 PPAPI_SVMMEM_GetViewMaxSectionCount(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32ViewMaxSectionCnt;
}

PP_U32 PPAPI_SVMMEM_GetViewCount(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32ViewCnt;
}

PP_U32 PPAPI_SVMMEM_GetViewTotalSize(PP_VIEWMODE_E IN enView)
{
	if ( !gpstTotalHD ) return 0;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return 0;
	return gpstViewHD[enView].u32DataSize;
}

PP_U32 PPAPI_SVMMEM_GetViewMaxSize(PP_VOID)
{
	if ( !gpstTotalHD ) return 0;
	return gpstTotalHD->u32ViewMaxDataSize;
}

PP_U32 PPAPI_SVMMEM_GetViewDataSize(PP_VIEWMODE_E IN enView, PP_SVMMEM_VIEW_DATA_E IN enLutType)
{
	if ( !gpstTotalHD ) return 0;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return 0;

	return gpstViewHD[enView].stBCData[enLutType].u32DataSize;
}

PP_U32	PPAPI_SVMMEM_GetViewSectionCount(PP_VIEWMODE_E IN enView)
{
	if ( !gpstTotalHD ) return 0;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return 0;

	return gpstViewHD[enView].u32SectionCnt;
}

PP_S32 PPAPI_SVMMEM_GetViewSectionIndex(PP_VIEWMODE_E IN enView, PP_U32 IN u32viewSectionIndex)
{
	if ( !gpstTotalHD ) return -1;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt || u32viewSectionIndex >= gpstViewHD[enView].u32SectionCnt ) return -1;

	return VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[u32viewSectionIndex]);
}

PP_RECT_S* PPAPI_SVMMEM_GetViewSectionRect(PP_VIEWMODE_E IN enView, PP_U32 u32ViewSectionIndex)
{
	PP_U16 u16SectionIndex;
	
	if ( !gpstTotalHD ) return PP_NULL;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return PP_NULL;
	if ( !gpstSectionHD ) return PP_NULL;
	if ( u32ViewSectionIndex >= gpstViewHD[enView].u32SectionCnt ) return PP_NULL;

	u16SectionIndex = VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[u32ViewSectionIndex]);
	gstSectionRect.u16X = gpstViewHD[enView].stSectionPos[u32ViewSectionIndex].u16X;
	gstSectionRect.u16Y = gpstViewHD[enView].stSectionPos[u32ViewSectionIndex].u16Y;
	gstSectionRect.u16Width = gpstSectionHD[u16SectionIndex].stSectionSize.u16Width;
	gstSectionRect.u16Height = gpstSectionHD[u16SectionIndex].stSectionSize.u16Height;

	return &gstSectionRect;
}

PP_BOOL	PPAPI_SVMMEM_CheckViewSectionRect(PP_VIEWMODE_E IN enView, PP_RECT_S* IN stSectionRect, PP_U32 u32SectionCnt)
{
	PP_U32 i, j;
	PP_U16 u16SectionIndex;
	PP_BOOL bExist;

	if ( !gpstTotalHD ) return PP_FALSE;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return PP_FALSE;
	if ( !gpstSectionHD ) return PP_FALSE;

	if ( u32SectionCnt != gpstViewHD[enView].u32SectionCnt ) return PP_FALSE;

	for ( i = 0; i < gpstViewHD[enView].u32SectionCnt; i++ )
	{
		u16SectionIndex = VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[i]);

		bExist = PP_FALSE;
		for ( j = 0; j < u32SectionCnt; j++ )
		{
			if ( gpstViewHD[enView].stSectionPos[i].u16X == stSectionRect[j].u16X &&
				 gpstViewHD[enView].stSectionPos[i].u16Y == stSectionRect[j].u16Y &&
				 gpstSectionHD[u16SectionIndex].stSectionSize.u16Width == stSectionRect[j].u16Width &&
				 gpstSectionHD[u16SectionIndex].stSectionSize.u16Height == stSectionRect[j].u16Height )
			{
				bExist = PP_TRUE;
				break;
			}
		}

		if ( !bExist )
			return PP_FALSE;
	}

	return PP_TRUE;
}

PP_BOOL PPAPI_SVMMEM_IsData(PP_VIEWMODE_E IN enView, PP_SVMMEM_DATA_SUBTYPE_E IN enLutType)
{
	if ( !gpstTotalHD ) return 0;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return PP_FALSE;

	return (PP_BOOL)utilGetBit(gpstViewHD[enView].u32IsData, enLutType);
}

PP_RECT_S* PPAPI_SVMMEM_GetImageRect(PP_VIEWMODE_E IN enView, PP_SVMDRV_IMG_NUMBER_E enImgNum)
{
	PP_U32 i;
	PP_U32 u32ImgSize;
	PP_U16 u16SectionIndex;
	PP_U8 u8ImgSectionCnt = 0;

	if ( !gpstTotalHD ) return PP_NULL;
	if ( !gpstViewHD || enView >= gpstTotalHD->u32ViewCnt ) return PP_NULL;
	if ( !gpstSectionHD ) return PP_NULL;

	for ( i = 0; i < gpstViewHD[enView].u32SectionCnt; i++ )
	{
		u16SectionIndex = VIEWSECTIONINDEX(gpstViewHD[enView].u16SectionIndexList[i]);
		
		u32ImgSize = gpstSectionHD[u16SectionIndex].stData[eSVMMEM_SECTION_DATA_IMG_ODD].u32DataSize;
		if ( u32ImgSize )
		{
			if ( enImgNum == eSVMDRV_IMG_NUM_1 && u8ImgSectionCnt == 0 )
			{
				u8ImgSectionCnt++;
				continue;
			}
			
			memcpy(&gstImgRect, &gpstSectionHD[u16SectionIndex].stImgRect, sizeof(PP_RECT_S));
			gstImgRect.u16X += gpstViewHD[enView].stSectionPos[i].u16X;
			gstImgRect.u16Y += gpstViewHD[enView].stSectionPos[i].u16Y;

			return &gstImgRect;
		}
	}

	return PP_NULL;
}

PP_U32 PPAPI_SVMMEM_GetCacheViewListCount(PP_VOID)
{
	return gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].u32Cnt;
}

PP_VIEWMODE_E*	PPAPI_SVMMEM_GetCacheViewList(PP_VOID)
{
	return (PP_VIEWMODE_E*)gstSVMMEMCtrl.pu32CacheViewList;
}

PP_BOOL	PPAPI_SVMMEM_CheckCacheView(PP_VIEWMODE_E enView)
{
	if ( PPAPI_SVMMEM_GetPageArea(enView) == eSVMMEM_PAGE_AREA_CACHE )
		return PP_TRUE;
	return PP_FALSE;
}

PP_U32* PPAPI_SVMMEM_GetCacheViewLoadedFBLUTAddress(PP_FIELD_E IN enField)
{
	if ( enField == eFIELD_ODD )
		return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_FB_ODD];
	else
		return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_FB_EVEN];
}

PP_U32* PPAPI_SVMMEM_GetCacheViewLoadedLRLUTAddress(PP_FIELD_E IN enField)
{
	if ( enField == eFIELD_ODD )
		return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_LR_ODD];
	else
		return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_LR_EVEN];
}

PP_U32* PPAPI_SVMMEM_GetCacheViewLoadedBCLUTAddress(PP_FIELD_E IN enField)
{
	if ( enField == eFIELD_ODD )
		return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_BC_ODD];
	else
		return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_BC_EVEN];
}

PP_U32* PPAPI_SVMMEM_GetCacheViewLoadedBCAddLUTAddress(PP_SVMMEM_BC_ADDITIONAL_LUT_E IN enType, PP_SVMMEM_BC_ADDITIONAL_LUT_SUBCORE_E IN enSubCore, PP_FIELD_E IN enField)
{
	PP_U32 u32Pos;

	switch ( enType )
	{
	case eSVMMEM_BC_ADD_LUT_ALPHA_0:
		u32Pos = eSVMMEM_DATA_SUBTYPE_A0S0_ODD;
		break;
	case eSVMMEM_BC_ADD_LUT_ALPHA_1:
		u32Pos = eSVMMEM_DATA_SUBTYPE_A1S0_ODD;
		break;
	case eSVMMEM_BC_ADD_LUT_GRADIENT:
		u32Pos = eSVMMEM_DATA_SUBTYPE_BGS0_ODD;
		break;
	default:
		return PP_NULL;
	}

	if ( enSubCore == eSVMMEM_BC_ADD_LUT_SUBCORE_1 )
		u32Pos += 2;

	if ( enField == eFIELD_EVEN )
		u32Pos += 1;
	
	return gstSVMMEMCtrl.pu32CacheViewAddr[u32Pos];
}

PP_U32* PPAPI_SVMMEM_GetCacheViewLoadedImageAddress(PP_SVMDRV_IMG_NUMBER_E enImgNum, PP_FIELD_E IN enField)
{
	if ( enImgNum == eSVMDRV_IMG_NUM_0 )
	{
		if ( enField == eFIELD_ODD )
			return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_IMG0_ODD];
		else
			return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_IMG0_EVEN];
	}
	else if ( enImgNum == eSVMDRV_IMG_NUM_1 )
	{
		if ( enField == eFIELD_ODD )
			return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_IMG1_ODD];
		else
			return gstSVMMEMCtrl.pu32CacheViewAddr[eSVMMEM_DATA_SUBTYPE_IMG1_EVEN];
	}

	return PP_NULL;
}

PP_BOOL PPAPI_SVMMEM_IsLoaded(PP_VIEWMODE_E IN enView)
{
	PP_U32 i;
	PP_SVMMEM_PAGE_AREA_E enPageArea;

	if ( enView == VIEWMODE_NULL || enView >= eVIEWMODE_TOTAL_MAX ) return PP_FALSE;
	if ( eVIEWMODE_BASIC_FRONT_BYPASS <= enView && enView <= eVIEWMODE_BASIC_QUAD ) return PP_TRUE;
	
	enPageArea = PPAPI_SVMMEM_GetPageArea(enView);

	for ( i = 0; i < gstSVMMEMCtrl.stPage[enPageArea].u32Cnt; i++ )
	{
		if ( enView == gstSVMMEMCtrl.stPage[enPageArea].pu32ViewList[i] )
		{
			return PP_TRUE;
		}
	}
	
	return PP_FALSE;
}

PP_BOOL PPAPI_SVMMEM_IsInitialize(PP_VOID)
{
	return gstSVMMEMCtrl.bInit;
}

PP_RESULT_E PPAPI_SVMMEM_Initialize(PP_VIEWMODE_E IN enCacheStartView, PP_VIEWMODE_E IN enCacheEndView)
{
	PP_U32 i, j;
	PP_S32 s32VoutWidth, s32VoutHeight;
	_VID_RESOL enVoutResol;
	
	memset(&gstSVMMEMCtrl, 0, sizeof(PP_SVMMEM_CTRL_S));

	PPAPI_VIN_GetResol(BD_SVM_OUT_FMT, &s32VoutWidth, &s32VoutHeight, &enVoutResol);

	if ( (enCacheStartView == VIEWMODE_NULL && enCacheEndView != VIEWMODE_NULL) || 
		 (enCacheStartView != VIEWMODE_NULL && enCacheEndView == VIEWMODE_NULL) )
	{
		LOG_WARNING("[SVMMEM] (%s, %d) : Cache view mode argument error\n", __FUNCTION__, __LINE__);
		enCacheStartView = enCacheEndView = VIEWMODE_NULL;
	}

	if ( enCacheEndView != VIEWMODE_NULL && enCacheEndView >= eVIEWMODE_TOTAL_MAX )
		enCacheEndView = eVIEWMODE_TOTAL_MAX - 1;

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	if ( FLASH_VER_ERROR_CHECK(gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32Ver) == 0 )
	{
		gpstTotalHD = OSAL_malloc(sizeof(PP_SVMMEM_TOTAL_HEADER_S));
		if ( !gpstTotalHD ) return eERROR_NO_MEM;
		
		PPAPI_FLASH_Read((PP_U8*)gpstTotalHD, gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr, sizeof(PP_SVMMEM_TOTAL_HEADER_S));

		if ( gpstTotalHD->u32ViewCnt )
		{
			gpstViewHD = OSAL_malloc(sizeof(PP_SVMMEM_VIEW_HEADER_S) * gpstTotalHD->u32ViewCnt);
			if ( !gpstViewHD ) return eERROR_NO_MEM;
			
			PPAPI_FLASH_Read((PP_U8*)gpstViewHD, gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstTotalHD->u32ViewHeaderOffset,
							sizeof(PP_SVMMEM_VIEW_HEADER_S) * gpstTotalHD->u32ViewCnt);

			if ( gpstTotalHD->u32SectionCnt )
			{
				gpstSectionHD = OSAL_malloc(sizeof(PP_SVMMEM_SECTION_HEADER_S) * gpstTotalHD->u32SectionCnt);
				if ( !gpstSectionHD ) return eERROR_NO_MEM;
				
				PPAPI_FLASH_Read((PP_U8*)gpstSectionHD, gstFlashHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstTotalHD->u32SectionHeaderOffset,
								sizeof(PP_SVMMEM_SECTION_HEADER_S) * gpstTotalHD->u32SectionCnt);
			}
		}
	}
#else
	// FTL header
	if ( FLASH_VER_ERROR_CHECK(gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32Ver) == 0 )
	{
		gpstTotalHD = OSAL_malloc(sizeof(PP_SVMMEM_TOTAL_HEADER_S));
		if ( !gpstTotalHD ) return eERROR_NO_MEM;
		
		if ( PPAPI_FTL_Read((PP_U8*)gpstTotalHD, gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr, sizeof(PP_SVMMEM_TOTAL_HEADER_S)) != 0 ) return eERROR_NO_FLASH_MEM;;

		if ( gpstTotalHD->u32ViewCnt )
		{
			gpstViewHD = OSAL_malloc(sizeof(PP_SVMMEM_VIEW_HEADER_S) * gpstTotalHD->u32ViewCnt);
			if ( !gpstViewHD ) return eERROR_NO_MEM;
			
			if ( PPAPI_FTL_Read((PP_U8*)gpstViewHD, gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstTotalHD->u32ViewHeaderOffset,
								sizeof(PP_SVMMEM_VIEW_HEADER_S) * gpstTotalHD->u32ViewCnt) != 0 )
			{
				return eERROR_NO_FLASH_MEM;
			}

			if ( gpstTotalHD->u32SectionCnt )
			{
				gpstSectionHD = OSAL_malloc(sizeof(PP_SVMMEM_SECTION_HEADER_S) * gpstTotalHD->u32SectionCnt);
				if ( !gpstSectionHD ) return eERROR_NO_MEM;
				
				if ( PPAPI_FTL_Read((PP_U8*)gpstSectionHD, gstFlashFTLHeader.stSect[eFLASH_SECT_SVM_LUT].u32FlashAddr + gpstTotalHD->u32SectionHeaderOffset,
									sizeof(PP_SVMMEM_SECTION_HEADER_S) * gpstTotalHD->u32SectionCnt) != 0 )
				{
					return eERROR_NO_FLASH_MEM;
				}
			}
		}
	}
#endif

	if ( !gpstTotalHD || !gpstViewHD || !gpstSectionHD )
	{
		if ( gpstTotalHD )
		{
			OSAL_free(gpstTotalHD);
			gpstTotalHD = PP_NULL;
		}

		if ( gpstViewHD )
		{
			OSAL_free(gpstViewHD);
			gpstViewHD = PP_NULL;
		}

		if ( gpstSectionHD )
		{
			OSAL_free(gpstSectionHD);
			gpstSectionHD = PP_NULL;
		}

		LOG_CRITICAL("###############################################################################################################\n");
		LOG_CRITICAL("# There is no SVM data in flash memory.\n");
		LOG_CRITICAL("###############################################################################################################\n");
	}
	else if ( gpstTotalHD->stViewSize.u16Width != s32VoutWidth || gpstTotalHD->stViewSize.u16Height != s32VoutHeight )
	{
		if ( gpstTotalHD )
		{
			OSAL_free(gpstTotalHD);
			gpstTotalHD = PP_NULL;
		}

		if ( gpstViewHD )
		{
			OSAL_free(gpstViewHD);
			gpstViewHD = PP_NULL;
		}

		if ( gpstSectionHD )
		{
			OSAL_free(gpstSectionHD);
			gpstSectionHD = PP_NULL;
		}

		LOG_CRITICAL("###############################################################################################################\n");
		LOG_CRITICAL("# The SVM data in flash memory is different from the current resolution.\n");
		LOG_CRITICAL("###############################################################################################################\n");
	}
	else if ( gpstTotalHD && gpstTotalHD->u32ViewCnt != eVIEWMODE_LOAD_MAX )
	{
		LOG_CRITICAL("###############################################################################################################\n");
		LOG_CRITICAL("# [SVMMEM] SVM view mode count(%d) of flash and eVIEWMODE_LOAD_MAX(%d) of viewmode_config.h are not the same. #\n", gpstTotalHD->u32ViewCnt, eVIEWMODE_LOAD_MAX);
		LOG_CRITICAL("###############################################################################################################\n");
	}

	if ( !gpstTotalHD )
	{
		enCacheStartView = enCacheEndView = VIEWMODE_NULL;
	}
	else if ( enCacheEndView != VIEWMODE_NULL && enCacheEndView > (gpstTotalHD->u32ViewCnt + eVIEWMODE_TOTAL_MAX - eVIEWMODE_LOAD_MAX - 1) )
		enCacheEndView = (gpstTotalHD->u32ViewCnt + eVIEWMODE_TOTAL_MAX - eVIEWMODE_LOAD_MAX - 1) - 1;
	
	// Default page flipping area
	gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].pu32ViewList = (PP_VIEWMODE_E*)OSAL_malloc(sizeof(PP_VIEWMODE_E) * SVMMEM_VIEW_PAGE_FLIPPING_COUNT);
	gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].bReload = (PP_BOOL*)OSAL_malloc(sizeof(PP_BOOL) * SVMMEM_VIEW_PAGE_FLIPPING_COUNT);
	if ( !gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].pu32ViewList || !gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].bReload )
	{
		LOG_CRITICAL("[SVMMEM] (%s_%d) : eERROR_NO_MEM\n", __FUNCTION__, __LINE__);
		return eERROR_NO_MEM;
	}
	gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].u32Cnt = SVMMEM_VIEW_PAGE_FLIPPING_COUNT;
	for ( i = 0; i < gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].u32Cnt; i++ )
	{
		gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].pu32ViewList[i] = VIEWMODE_NULL;
		gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].bReload[i] = PP_FALSE;
	}
	gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_FLIPPING].u32Cur = 0;

	if ( enCacheStartView != VIEWMODE_NULL && enCacheEndView != VIEWMODE_NULL && (enCacheStartView >= eVIEWMODE_TOTAL_MAX || enCacheStartView > enCacheEndView) )
	{
		LOG_WARNING("[SVMMEM] (%s, %d) : Cache view mode argument error\n", __FUNCTION__, __LINE__);
		enCacheStartView = VIEWMODE_NULL;
	}

	// Pre-loading page flipping area
	if ( enCacheStartView != VIEWMODE_NULL )
	{
		PP_U32 u32CacheViewCnt;

		if ( enCacheEndView == VIEWMODE_NULL )
			u32CacheViewCnt = 1;
		else
			u32CacheViewCnt = enCacheEndView - enCacheStartView + 1;
		
		gstSVMMEMCtrl.pu32CacheViewList = (PP_VIEWMODE_E*)OSAL_malloc(sizeof(PP_VIEWMODE_E) * u32CacheViewCnt);
		gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].pu32ViewList = (PP_VIEWMODE_E*)OSAL_malloc(sizeof(PP_VIEWMODE_E) * u32CacheViewCnt);
		gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].bReload = (PP_BOOL*)OSAL_malloc(sizeof(PP_BOOL) * u32CacheViewCnt);
		if ( !gstSVMMEMCtrl.pu32CacheViewList || !gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].pu32ViewList || !gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].bReload )
		{
			LOG_WARNING("[SVMMEM] (%s_%d) : eERROR_NO_MEM\n", __FUNCTION__, __LINE__);
			return eERROR_NO_MEM;
		}
		gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].u32Cnt = u32CacheViewCnt;

		for ( i = 0, j = enCacheStartView; i < gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].u32Cnt; i++, j++ )
		{
			gstSVMMEMCtrl.pu32CacheViewList[i] = j;
			gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].pu32ViewList[i] = VIEWMODE_NULL;
			gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].bReload[i] = PP_FALSE;
		}
	}
	gstSVMMEMCtrl.stPage[eSVMMEM_PAGE_AREA_CACHE].u32Cur = 0;

	gstSVMMEMCtrl.bInit = PP_TRUE;
	
	gstSVMMEMCtrl.stLutSize.u16Width = (PP_U8)(s32VoutWidth / 8) + 1;
	if ( gstSVMMEMCtrl.stLutSize.u16Width % 2 != 0 ) gstSVMMEMCtrl.stLutSize.u16Width++;
	gstSVMMEMCtrl.stLutSize.u16Height = (PP_U8)(s32VoutHeight / 8) + 1;
	
	gstSVMMEMCtrl.u32LutDataSize = gstSVMMEMCtrl.stLutSize.u16Width * gstSVMMEMCtrl.stLutSize.u16Height * 4;
	if ( gstSVMMEMCtrl.u32LutDataSize % 16 )
		gstSVMMEMCtrl.u32LutDataSize += (16 - (gstSVMMEMCtrl.u32LutDataSize % 16));
	
	return eSUCCESS;
}
