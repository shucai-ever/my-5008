/*
 * dram.c
 */

#include <stdio.h>
#include <unistd.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "debug.h"
#include "system.h"

#include "dram.h"

_gstDramReserved gstDramReserved;
 
extern PP_U8 __pi5008_rsv_area; //Defined pi5008_sdk.sag

void init_dram_reserved(void)
{
#ifdef ALLOC_DRAM_RESERVED

    PP_U32 i, j;
    PP_U32 u32Addr;
	
	PP_BOOL bSVMMemInit = PPAPI_SVMMEM_IsInitialize();
	PP_U32 u32SVMViewSize = 0;
	PP_U32* u32CacheViewList = PP_NULL;
	PP_U32 u32CacheViewListCnt = 0;
	PP_U16* u16SVMCommonSectionList = PP_NULL;
	PP_U32 u32SVMCommonSectionCnt = 0;
	PP_U32 u32SVMTotalCommonSectionSize = 0;

    PP_U8 *pDramRsvBaseAddr = (PP_U8 *)&__pi5008_rsv_area;

	memset(&gstDramReserved, 0, sizeof(_gstDramReserved));

    gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr);
    gstDramReserved.u32AllocatedSize = 0;
    LOG_DEBUG("DramRsvBase addr: 0x%08x\n", (PP_U32)gstDramReserved.u32AllocStartAddr);

	gstDramReserved.u32AddrSVM = (PP_U32)((PP_U8 *)gstDramReserved.u32AllocStartAddr);
	gstDramReserved.u32SizeSVM = 0;
	
	u32Addr = gstDramReserved.u32AddrSVM;
	for(i = 0; i < 4; i++) //4ch
	{
		gstDramReserved.u32SvmInBuff[i] = u32Addr + (SVM_RESERVED_IN_BUFFERSIZE*(i));
		gstDramReserved.u32AllocatedSize += SVM_RESERVED_IN_BUFFERSIZE;
		gstDramReserved.u32SizeSVM += SVM_RESERVED_IN_BUFFERSIZE;
		//LOG_DEBUG("   SVM In%d Buff, addr:0x%08x, size:0x%08x(%u)\n", i, gstDramReserved.u32SvmInBuff[i], SVM_RESERVED_IN_BUFFERSIZE, SVM_RESERVED_IN_BUFFERSIZE);
	}
	
	u32Addr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
	for(i = 0; i < SVMMEM_OUT_BUFFER_COUNT; i++)
	{
		gstDramReserved.u32SvmOutBuff[i] = u32Addr + (SVM_RESERVED_OUT_BUFFERSIZE*(i));
		gstDramReserved.u32AllocatedSize += SVM_RESERVED_OUT_BUFFERSIZE;
		gstDramReserved.u32SizeSVM += SVM_RESERVED_OUT_BUFFERSIZE;
		//LOG_DEBUG("   SVM Out%d Buff, addr:0x%08x, size:0x%08x(%u)\n", i, gstDramReserved.u32SvmOutBuff[i], SVM_RESERVED_OUT_BUFFERSIZE, SVM_RESERVED_OUT_BUFFERSIZE);
	}

	u32Addr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
	if ( bSVMMemInit )
	{
		// reserved memory for page fliping
		u32SVMViewSize = PPAPI_SVMMEM_GetViewMaxSize();
		if ( u32SVMViewSize )
		{
			u32SVMViewSize = ALIGN_128BIT(u32SVMViewSize);
			for ( i = 0; i < SVMMEM_VIEW_PAGE_FLIPPING_COUNT; i++ )
			{
				gstDramReserved.u32SvmData[i] = u32Addr  + (u32SVMViewSize*(i));
				gstDramReserved.u32AllocatedSize += u32SVMViewSize;
				gstDramReserved.u32SizeSVM += u32SVMViewSize;
				//LOG_DEBUG("   SVM Data%d, addr:0x%08x, size:0x%08x(%u)\n", i, gstDramReserved.u32SvmData[i], u32SVMViewSize, u32SVMViewSize);
			}
		}

		// reserved memory for common section
		u32Addr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
		u16SVMCommonSectionList = PPAPI_SVMMEM_GetCommonSectionList();
		u32SVMCommonSectionCnt = PPAPI_SVMMEM_GetCommonSectionCount();
		if ( u16SVMCommonSectionList != PP_NULL && u32SVMCommonSectionCnt != 0 )
		{
			gstDramReserved.u32SvmCommonSectionData = (PP_U32**)OSAL_malloc(sizeof(PP_U32) * u32SVMCommonSectionCnt);
			for ( i = 0; i < u32SVMCommonSectionCnt; i++ )
			{
				gstDramReserved.u32SvmCommonSectionData[i] = (PP_U32*)OSAL_malloc(sizeof(PP_U32) * eSVMMEM_SECTION_DATA_MAX);

				for ( j = eSVMMEM_SECTION_DATA_FB_ODD; j < eSVMMEM_SECTION_DATA_MAX; j++ )
				{
					u32SVMViewSize = PPAPI_SVMMEM_GetSectionDataSize(u16SVMCommonSectionList[i], j);
					if ( u32SVMViewSize )
					{
						u32SVMViewSize = ALIGN_128BIT(u32SVMViewSize);
						gstDramReserved.u32SvmCommonSectionData[i][j] = u32Addr + u32SVMTotalCommonSectionSize;
						gstDramReserved.u32AllocatedSize += u32SVMViewSize;
						gstDramReserved.u32SizeSVM += u32SVMViewSize;
						u32SVMTotalCommonSectionSize += u32SVMViewSize;
						//LOG_DEBUG("   SVM common section Data%d, addr:0x%08x, size:0x%08x(%u)\n", i, gstDramReserved.u32SvmCommonSectionData[i][j], u32SVMViewSize, u32SVMViewSize);
					}
				}
			}
		}

		// reserved memory for section merge
		u32Addr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
		u32SVMViewSize = PPAPI_SVMMEM_GetSectionMaxDataSize();
		if ( u32SVMViewSize )
		{
			PP_U8 u8MaxCnt = PPAPI_SVMMEM_GetSectionMergeMaxCount();
			if ( PPAPI_SVMMEM_GetCacheViewListCount() ) u8MaxCnt *= 2;

			u32SVMViewSize = ALIGN_128BIT(u32SVMViewSize);
			
			gstDramReserved.u32SVMSectionMergeData = (PP_U32*)OSAL_malloc(sizeof(PP_U32) * u8MaxCnt);
			for ( i = 0; i < u8MaxCnt; i++ )
			{
				gstDramReserved.u32SVMSectionMergeData[i] = u32Addr + (u32SVMViewSize*(i));
				gstDramReserved.u32AllocatedSize += u32SVMViewSize;
				gstDramReserved.u32SizeSVM += u32SVMViewSize;
				//LOG_DEBUG("   SVM section Data%d, addr:0x%08x, size:0x%08x(%u)\n", i, gstDramReserved.u32SVMSectionMergeData[i], u32SVMViewSize, u32SVMViewSize);
			}
		}
	}

	gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
	LOG_DEBUG(" SVM base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)gstDramReserved.u32AddrSVM, gstDramReserved.u32SizeSVM, gstDramReserved.u32SizeSVM);

	gstDramReserved.u32AddrDisplay = (PP_U32)(gstDramReserved.u32AllocStartAddr);
	gstDramReserved.u32SizeDisplay = (PP_U32)DISPLAY_RESERVED_SIZE;
	gstDramReserved.u32AllocatedSize += gstDramReserved.u32SizeDisplay;
	gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
	LOG_DEBUG(" DISPLAY base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)gstDramReserved.u32AddrDisplay, gstDramReserved.u32SizeDisplay, gstDramReserved.u32SizeDisplay);

    gstDramReserved.u32AddrQUAD = (PP_U32)(gstDramReserved.u32AllocStartAddr);
    gstDramReserved.u32SizeQUAD = (PP_U32)QUAD_RESERVED_SIZE;
    gstDramReserved.u32AllocatedSize += gstDramReserved.u32SizeQUAD;
	gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
    LOG_DEBUG(" QUAD base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)gstDramReserved.u32AddrQUAD, gstDramReserved.u32SizeQUAD, gstDramReserved.u32SizeQUAD);
    for(i = 0; i < 4; i++) //4ch
    {
        gstDramReserved.u32QuadRsvCh[i] = gstDramReserved.u32AddrQUAD + (QUAD_RESERVED_CHSIZE*(i)); 
        *(PP_VU32 *)(VIN_BASE_ADDR + (0x0140+(i*4))) = gstDramReserved.u32QuadRsvCh[i];
        //LOG_DEBUG("   Quad Ch%d, addr:0x%08x\n", i, gstDramReserved.u32QuadRsvCh[i]);
    }

    gstDramReserved.u32AddrVPU = (PP_U32)(gstDramReserved.u32AllocStartAddr);
    gstDramReserved.u32SizeVPU = (PP_U32)VPU_RESERVED_SIZE;
    gstDramReserved.u32AllocatedSize += gstDramReserved.u32SizeVPU;
	gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
    LOG_DEBUG(" VPU base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)gstDramReserved.u32AddrVPU, gstDramReserved.u32SizeVPU, gstDramReserved.u32SizeVPU);
    for(i = 0; i < 1; i++)
    {
        gstDramReserved.u32CaptureRsvBuff[i] = gstDramReserved.u32AddrVPU + (VPU_RESERVED_BUFFSIZE*(i)); 
        //LOG_DEBUG("   Capture Buff%d, addr:0x%08x\n", i, gstDramReserved.u32CaptureRsvBuff[i]);
    }
    u32Addr = gstDramReserved.u32AddrVPU + (VPU_RESERVED_BUFFSIZE*(i)); 
    for(i = 0; i < eVPU_FRAME_MAX; i++)
    {
        gstDramReserved.u32VpuBufFast[i] = u32Addr + (VPU_BUF_FAST_SIZE*(i)); 
        //LOG_DEBUG("   VPU BufFast Frame%d, addr:0x%08x\n", i, gstDramReserved.u32VpuBufFast[i]);
    }
    u32Addr += (VPU_BUF_FAST_SIZE*(i)); 
    for(i = 0; i < eVPU_FRAME_MAX; i++)
    {
        gstDramReserved.u32VpuBufBrief[i] = u32Addr + (VPU_BUF_BRIEF_SIZE*(i)); 
        //LOG_DEBUG("   VPU BufBrief Frame%d, addr:0x%08x\n", i, gstDramReserved.u32VpuBufBrief[i]);
    }
    u32Addr += (VPU_BUF_BRIEF_SIZE*(i)); 
    for(i = 0; i < eVPU_FRAME_MAX; i++)
    {
        gstDramReserved.u32VpuBufFC[i] = u32Addr + (VPU_BUF_FC_SIZE*(i)); 
        //LOG_DEBUG("   VPU BufFC Frame%d, addr:0x%08x\n", i, gstDramReserved.u32VpuBufFC[i]);
    }
    u32Addr += (VPU_BUF_FC_SIZE*(i)); 
    for(i = 0; i < eVPU_ZONE_MAX; i++)
    {
        gstDramReserved.u32VpuBufHDMatch[i] = u32Addr + (VPU_BUF_HDMATCHZONE_SIZE*(i)); 
        //LOG_DEBUG("   VPU BufHDMatch Zone%d, addr:0x%08x\n", i, gstDramReserved.u32VpuBufHDMatch[i]);
    }

    
#ifdef CALIB_LIB_USE
		gstDramReserved.u32AddrCalibrationLib = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
		gstDramReserved.u32SizeCalibrationLib = (PP_U32)CALIB_RESERVED_BUFFSIZE;
		LOG_DEBUG(" CALIB LIB base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)gstDramReserved.u32AddrCalibrationLib, gstDramReserved.u32SizeCalibrationLib, gstDramReserved.u32SizeCalibrationLib);
#endif
	
	if ( bSVMMemInit )
	{
		// reserved memory for cache
		u32Addr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
		u32CacheViewList = (PP_U32*)PPAPI_SVMMEM_GetCacheViewList();
		u32CacheViewListCnt = PPAPI_SVMMEM_GetCacheViewListCount();
		if ( u32CacheViewList != PP_NULL && u32CacheViewListCnt != 0 )
		{
			gstDramReserved.u32SvmCacheData = (PP_U32*)OSAL_malloc(sizeof(PP_U32) * u32CacheViewListCnt);
			for ( i = 0; i < u32CacheViewListCnt; i++ )		// reserved memory for page fliping
			{
				u32SVMViewSize = PPAPI_SVMMEM_GetViewTotalSize(u32CacheViewList[i]);
				gstDramReserved.u32SvmCacheData[i] = u32Addr  + gstDramReserved.u32SizeSvmCacheData;
				gstDramReserved.u32SizeSvmCacheData += u32SVMViewSize;
				gstDramReserved.u32AllocatedSize += u32SVMViewSize;
			}
			gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
			LOG_DEBUG(" SVM Cache base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)u32Addr, gstDramReserved.u32SizeSvmCacheData, gstDramReserved.u32SizeSvmCacheData);
		}
	}

	gstDramReserved.u32AddrSwingCar = (PP_U32)(gstDramReserved.u32AllocStartAddr);
	gstDramReserved.u32SizeSwingCar = (PP_U32)SWING_CAR_RESERVED_SIZE;
	gstDramReserved.u32AllocatedSize += gstDramReserved.u32SizeSwingCar;
	gstDramReserved.u32AllocStartAddr = (PP_U32)((PP_U8 *)pDramRsvBaseAddr) + gstDramReserved.u32AllocatedSize;
	LOG_DEBUG(" Swing Car base addr:0x%08x, size:0x%08x(%u)\n", (PP_U32)gstDramReserved.u32AddrSwingCar, gstDramReserved.u32SizeSwingCar, gstDramReserved.u32SizeSwingCar);


    LOG_DEBUG("Total Dram Reserved base addr:0x%08x, size:0x%08x(%u), end addr: 0x%08x\n", (PP_U32)((PP_U8 *)pDramRsvBaseAddr), (PP_U32)gstDramReserved.u32AllocatedSize, (PP_U32)gstDramReserved.u32AllocatedSize,(PP_U32)((PP_U8 *)pDramRsvBaseAddr)+(PP_U32)gstDramReserved.u32AllocatedSize);

#endif // ALLOC_DRAM_RESERVED
}
