#include "system.h"
#include "debug.h"
#include "osal.h"
#include "task_manager.h"
#include "spi.h"
#include "task_fwdn.h"
#include "api_FAT_FTL.h"
#include "api_flash.h"
#include "spi_flash.h"
#include "sys_api.h"
#include "dma.h"
#include "reset.h"
#include "ring_buffer.h"

#if defined(USE_PP_GUI)
#include "application.h"
#endif

#define DBG_TIME_MEASURE	0
#define UPGRADE_SPI_TEST	0

#if(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
typedef enum tagCMD_RESP_E
{
    eRESP_OK = 0,
    eERROR_INVALID_HDR,
    eERROR_INVALID_SIZE,
    eERROR_CHECKSUM_FAIL,
    eERROR_UNKNOWN_PACKET,
    eERROR_FLASH_INIT_FAIL,
    eERROR_FLASH_WRITE_FAIL,
	eERROR_UPGRADE_REJECT,
	eERROR_UPGRADE_FAIL,
}CMD_RESP_E;

typedef enum
{
	SPIFLASH_NOR = 0,
	SPIFLASH_NAND_RAW,
	SPIFLASH_NAND_FTL
}SPIFLASH_TYPE;


#define SPICOMM_BUFF_SIZE	(32*1024 + 32)

#endif	//#if(UPGRADE_METHOD == UPGRADE_METHOD_SPI)


static PP_CHAR *szSect[eFLASH_SECT_MAX] = {
		"bootloader", 			// 0
		"second bootloader",	// 1
		"isp f/w",				//2
		"isp f/w - 1",			//3
		"main f/w",				//4
		"main f/w - 1",			//5
		"svm lut",				//6
		"car image",			//7
		"menu",					//8
		"gui image",			//9
		"pgl image",			//10
		"isp data",				//11
		"sensor data",			//12
		"calib_main",			//13
		"calib_sub",			//14
		"capture",				//15
		"audio"					//16
};

static PP_FLASH_SECT_E genUpdateSection[] = {
		eFLASH_SECT_IFW,
		eFLASH_SECT_MFW,
		eFLASH_SECT_SVM_LUT,
		eFLASH_SECT_MENU,
		eFLASH_SECT_UI_IMG,
		eFLASH_SECT_ISP_DATA,
		eFLASH_SECT_SENSOR_DATA,
		eFLASH_SECT_CAM_CALIB_MAIN_TYPE,
		eFLASH_SECT_CAM_CALIB_SUB_TYPE,
};

#define UPGRADE_RX_BUFF_SIZE	1024

static PP_U8 gRxBufISRData[UPGRADE_RX_BUFF_SIZE];
circ_bbuf_t gRxBufISR = {
	.buffer = gRxBufISRData,
	.head = 0,
	.tail = 0,
	.maxlen = UPGRADE_RX_BUFF_SIZE
};

uint32 gRecvIdx = 0;
static SemaphoreHandle_t ghLockRx;

PP_VOID ShowFlashInfo(PP_U8 u8FTL)
{
	PP_U8 u8Buf[16];
	PP_FLASH_HDR_S *pstHdr;
	PP_S32 i;

	if(!u8FTL)pstHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
	else pstHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;


	LOG_DEBUG("====================================================\n");
	LOG_DEBUG("FLASH HEADER INFO\n");
	LOG_DEBUG("====================================================\n");
	LOG_DEBUG("signature: 0x%x\n", pstHdr->u32Sign);
	LOG_DEBUG("flash type: 0x%x\n", pstHdr->u32FlashType);
	LOG_DEBUG("page size: 0x%x\n", pstHdr->u32PageSize);
	LOG_DEBUG("block size: 0x%x\n", pstHdr->u32PagesBlock);
	LOG_DEBUG("clock div: 0x%x\n", pstHdr->u32ClockDiv);

	for(i=0;i<eFLASH_SECT_MAX;i++){
		LOG_DEBUG("%s ver: 0x%x\n", szSect[i], pstHdr->stSect[i].u32Ver);
		LOG_DEBUG("%s flash addr: 0x%x\n", szSect[i], pstHdr->stSect[i].u32FlashAddr);
		LOG_DEBUG("%s load addr: 0x%x\n", szSect[i], pstHdr->stSect[i].u32LoadAddr);
		LOG_DEBUG("%s size: 0x%x\n", szSect[i], pstHdr->stSect[i].u32Size);
	}
	

	LOG_DEBUG("====================================================\n");
	LOG_DEBUG("FLASH DATA\n");
	LOG_DEBUG("====================================================\n");

	for(i=0;i<eFLASH_SECT_MAX;i++){
		if(pstHdr->stSect[i].u32Ver > 0 && pstHdr->stSect[i].u32Ver != 0xffffffff){
			PPAPI_FLASH_Read((PP_U8 *)u8Buf, pstHdr->stSect[i].u32FlashAddr, 16);
			LOG_DEBUG("[%s]\n", szSect[i]);
			print_hex(u8Buf, 16);
		}

	}


}


static PP_S32 VerifyData(PP_U8 *src, PP_U8 *dst, PP_U32 size)
{
	PP_S32 ret = -1;
	PP_S32 i;

	for(i=0;i<size;i++){
		if(src[i] != dst[i]){
			ret = i;
			break;
		}
	}

	return ret;

}

static PP_S32 VerifyDataWord(PP_U8 *src, PP_U8 *dst, PP_U32 size)
{
	PP_S32 ret = -1;
	PP_S32 i;
	PP_U32 *pSrcPtr = (PP_U32 *)src;
	PP_U32 *pDstPtr = (PP_U32 *)dst;

	size /= 4;

	for(i=0;i<size;i++){
		if(pSrcPtr[i] != pDstPtr[i]){
			ret = i * 4;
			break;
		}
	}

	return ret;

}


static PP_RESULT_E DataWriteFlash(PP_U8 *pBuf, PP_U32 u32Addr, PP_S32 s32Size, PP_U8 u8FTL)
{
	PP_RESULT_E enRet = eSUCCESS;

	if(u8FTL == 0){	// nor flash, nand non FTL
		PPAPI_FLASH_WriteErasedBlock(pBuf, u32Addr, s32Size);
	}else{	// nand FTL
		if(PPAPI_FTL_Write(pBuf, u32Addr, s32Size) != 0)
			enRet = eERROR_FAILURE;
	}

	return enRet;
}

static PP_RESULT_E DataReadFlash(PP_U8 *pBuf, PP_U32 u32Addr, PP_S32 s32Size, PP_U8 u8FTL)
{
	PP_RESULT_E enRet = eSUCCESS;

	if(u8FTL == 0){	// nor flash, nand non FTL
		PPAPI_FLASH_Read(pBuf, u32Addr, s32Size);
	}else{	// nand FTL
		if(PPAPI_FTL_Read(pBuf, u32Addr, s32Size) != 0)
			enRet = eERROR_FAILURE;
	}

	return enRet;
}

#if(UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)
static PP_RESULT_E FlashUpdateAllSDCard(const PP_CHAR *pBuf)
{
	FWDN_FLASH_UPDATE_S *pstFW = (FWDN_FLASH_UPDATE_S *)pBuf;
	void *fh = 0;
	PP_U32 u32FileSize;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_U32 u32FlashSize;
#endif
	PP_U32 u32WriteUnitSize;
	PP_U32 u32WrittenSize;
	PP_U32 u32FlashAddr;
	PP_U8 *pu8SrcBuf = NULL;
	PP_U8 *pu8DstBuf = NULL;
	PP_S32 i;
	PP_RESULT_E ret = eSUCCESS;

#if DBG_TIME_MEASURE
	PP_U32 stick, etick;
#endif

	PPAPI_FATFS_DelVolume();
	if(PPAPI_FATFS_InitVolume()){
		ret = eERROR_UPGRADE_REJECTED;
		printf("Filesystem initvolume fail(%d)\n", PPAPI_FATFS_GetLastError());
		goto END_FT;
	}
	if(!(fh = PPAPI_FATFS_Open(pstFW->szFileName, "r"))){
		printf("Error! file open fail(%s)\n", pstFW->szFileName);
		ret = eERROR_UPGRADE_REJECTED;
		goto END_FT;
	}


	if((pu8SrcBuf = (PP_U8 *)OSAL_malloc(512*1024)) == NULL){
		printf("Error! malloc fail\n");
		ret = eERROR_UPGRADE_REJECTED;
		goto END_FT;
	}

	if((pu8DstBuf = (PP_U8 *)OSAL_malloc(512*1024)) == NULL){
		printf("Error! malloc fail\n");
		ret = eERROR_UPGRADE_REJECTED;
		goto END_FT;
	}

	u32FileSize = PPAPI_FATFS_FileLength(pstFW->szFileName);
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	u32FlashSize = (u32FileSize + (gstFlashID.u32EraseBlockSize -1)) & (~(gstFlashID.u32EraseBlockSize-1));
#else
//	u32FlashSize = (u32FileSize + (gstFlashNandID.u32EraseBlockSize -1)) & (~(gstFlashNandID.u32EraseBlockSize-1));
#endif

	printf("file name: %s, size: 0x%x, ver: 0x%x\n", pstFW->szFileName, u32FileSize, pstFW->u32Version);
	//=======================================================================
	// flash erase
	//=======================================================================
//	if(!(pstFW->enFlashType == eFLASH_TYPE_NAND && pstFW->enNandArea == eNAND_AREA_FTL)){
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	{
		u32WrittenSize = 0;
		u32FlashAddr = 0;
		while(u32WrittenSize < u32FlashSize){
			if(u32FlashSize - u32WrittenSize > 512*1024)u32WriteUnitSize = 512*1024;
			else u32WriteUnitSize = u32FlashSize - u32WrittenSize;

			printf("Flash erase. 0x%08x/0x%08x\n", u32WrittenSize, u32FlashSize);
			PPAPI_FLASH_Erase(u32FlashAddr + u32WrittenSize, u32WriteUnitSize);

			u32WrittenSize += u32WriteUnitSize;

		}
	}
#endif
	//=======================================================================
	// flash write & verify
	//=======================================================================
	u32WrittenSize = 0;
	u32FlashAddr = 0;
	while(u32WrittenSize < u32FileSize){
		if(u32FileSize - u32WrittenSize > 512*1024)u32WriteUnitSize = 512*1024;
		else u32WriteUnitSize = u32FileSize - u32WrittenSize;

		printf("Flash write 0x%08x/0x%08x\n", u32WrittenSize, u32FileSize);
		if(PPAPI_FATFS_Read(pu8SrcBuf, 1, (PP_S32)u32WriteUnitSize, fh) != u32WriteUnitSize){
			printf("Error! file read fail(size: %d)\n", u32WriteUnitSize);
			ret = eERROR_FAILURE;
			goto END_FT;
		}



		if((ret = DataWriteFlash(pu8SrcBuf, u32FlashAddr + u32WrittenSize, u32WriteUnitSize, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
			printf("Error! data write fail(size: %d)\n", u32WriteUnitSize);
			goto END_FT;
		}

		if(pstFW->u32Verify){
			if((ret = DataReadFlash(pu8DstBuf, u32FlashAddr + u32WrittenSize, u32WriteUnitSize, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
				printf("Error! flash read fail\n");
				goto END_FT;
			}

			if(u32WriteUnitSize & 3){	// byte compare
				if((i=VerifyData(pu8SrcBuf, pu8DstBuf, u32WriteUnitSize)) >= 0){
					printf("Error! wrong flash data at 0x%x\n", u32FlashAddr + i);
					ret = eERROR_FAILURE;
					goto END_FT;
				}
			}else{	// word compare
				if((i=VerifyDataWord(pu8SrcBuf, pu8DstBuf, u32WriteUnitSize)) >= 0){
					printf("Error! wrong flash data at 0x%x\n", u32FlashAddr + i);
					ret = eERROR_FAILURE;
					goto END_FT;
				}

			}
		}
		u32WrittenSize += u32WriteUnitSize;

	}

	printf("Flash write done\n");


#if 0
	PPAPI_FLASH_ReadHeader();
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
	PPAPI_FLASH_ReadFTLHeader();
#endif

	ShowFlashInfo(BD_FLASH_TYPE == FLASH_TYPE_NAND);
#endif

END_FT:
	if(pu8SrcBuf)OSAL_free(pu8SrcBuf);
	if(pu8DstBuf)OSAL_free(pu8DstBuf);
	if(fh)PPAPI_FATFS_Close(fh);


	return ret;
}

static PP_RESULT_E FlashUpdateSectionSDCard(const PP_CHAR *pBuf, PP_FLASH_SECT_E enFlashSection)
{
	FWDN_FLASH_UPDATE_S *pstFW = (FWDN_FLASH_UPDATE_S *)pBuf;
	void *fh = 0;
	PP_U32 u32FileSize;
	PP_U32 u32FlashSize;
	PP_U32 u32WriteUnitSize;
	PP_U32 u32WrittenSize;
	PP_U32 u32FlashAddr;
	PP_U8 *pu8SrcBuf = NULL;
	PP_U8 *pu8DstBuf = NULL;
	PP_S32 i;
	PP_RESULT_E ret = eSUCCESS;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
#else
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;
#endif

#if DBG_TIME_MEASURE
	PP_U32 stick, etick;
#endif

	PPAPI_FATFS_DelVolume();
	if(PPAPI_FATFS_InitVolume()){
		ret = eERROR_UPGRADE_REJECTED;
		printf("Filesystem initvolume fail(%d)\n", PPAPI_FATFS_GetLastError());
		goto END_FT;
	}
	if(!(fh = PPAPI_FATFS_Open(pstFW->szFileName, "r"))){
		printf("Error! file open fail(%s)\n", pstFW->szFileName);
		ret = eERROR_UPGRADE_REJECTED;
		goto END_FT;
	}


	if((pu8SrcBuf = (PP_U8 *)OSAL_malloc(512*1024)) == NULL){
		printf("Error! malloc fail\n");
		ret = eERROR_UPGRADE_REJECTED;
		goto END_FT;
	}

	if((pu8DstBuf = (PP_U8 *)OSAL_malloc(512*1024)) == NULL){
		printf("Error! malloc fail\n");
		ret = eERROR_UPGRADE_REJECTED;
		goto END_FT;
	}


	u32FileSize = PPAPI_FATFS_FileLength(pstFW->szFileName);

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	u32FlashSize = (u32FileSize + (gstFlashID.u32EraseBlockSize -1)) & (~(gstFlashID.u32EraseBlockSize-1));
#else
	u32FlashSize = (u32FileSize + (gstFlashNandID.u32EraseBlockSize -1)) & (~(gstFlashNandID.u32EraseBlockSize-1));
#endif
	printf("file name: %s, size: 0x%x, ver: 0x%x\n", pstFW->szFileName, u32FileSize, pstFW->u32Version);
	//=======================================================================
	// check updata available
	//=======================================================================
	// 1. check if setion is exist
	if(FLASH_VER_ERROR_CHECK(pstFlashHdr->stSect[enFlashSection].u32Ver)){
		ret = eERROR_UPGRADE_REJECTED;
		printf("Flash section(%d) not exist\n", enFlashSection);
		goto END_FT;
	}

	// 2. check if size is available
	for(i=enFlashSection+1;i<eFLASH_SECT_MAX;i++){
		if(!FLASH_VER_ERROR_CHECK(pstFlashHdr->stSect[i].u32Ver)){
			break;
		}
	}

	if(i== eFLASH_SECT_MAX){	// next section not found
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
		if((pstFlashHdr->stSect[enFlashSection].u32FlashAddr + u32FlashSize) > gstFlashID.u32TotalSize){
			ret = eERROR_UPGRADE_REJECTED;
			goto END_FT;
		}
#else
		if((pstFlashHdr->stSect[enFlashSection].u32FlashAddr + u32FlashSize) > (gstFlashNandID.u32TotalSize - SPI_NAND_NON_FTL_BLOCKS * gstFlashNandID.u32EraseBlockSize)){
			ret = eERROR_UPGRADE_REJECTED;
			goto END_FT;
		}

#endif
	}else{						// next section found
		if((pstFlashHdr->stSect[enFlashSection].u32FlashAddr + u32FlashSize) > pstFlashHdr->stSect[i].u32FlashAddr){
			ret = eERROR_UPGRADE_REJECTED;
			goto END_FT;
		}
	}

	//=======================================================================
	// flash erase
	//=======================================================================
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	{
		u32WrittenSize = 0;
		u32FlashAddr = pstFlashHdr->stSect[enFlashSection].u32FlashAddr;
		while(u32WrittenSize < u32FlashSize){
			if(u32FlashSize - u32WrittenSize > 512*1024)u32WriteUnitSize = 512*1024;
			else u32WriteUnitSize = u32FlashSize - u32WrittenSize;

			printf("Flash erase. 0x%08x/0x%08x\n", u32WrittenSize, u32FlashSize);
			PPAPI_FLASH_Erase(u32FlashAddr + u32WrittenSize, u32WriteUnitSize);

			u32WrittenSize += u32WriteUnitSize;

		}
	}
#endif

	if(pstFW->u32UpdateSection != FLASH_UPDATE_ALL){
		if(enFlashSection == eFLASH_SECT_IFW || enFlashSection == eFLASH_SECT_MFW){
			pstFlashHdr->stSect[enFlashSection].u32Ver = 0;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
			PPAPI_FLASH_Write((PP_U8 *)pstFlashHdr, 0, sizeof(PP_FLASH_HDR_S));
#else
			PPAPI_FTL_Write((PP_U8 *)pstFlashHdr, 0, sizeof(PP_FLASH_HDR_S));
#endif
		}
	}

	//=======================================================================
	// flash write & verify
	//=======================================================================
	u32WrittenSize = 0;
	u32FlashAddr = pstFlashHdr->stSect[enFlashSection].u32FlashAddr;
	while(u32WrittenSize < u32FileSize){
		if(u32FileSize - u32WrittenSize > 512*1024)u32WriteUnitSize = 512*1024;
		else u32WriteUnitSize = u32FileSize - u32WrittenSize;

		printf("Flash write 0x%08x/0x%08x\n", u32WrittenSize, u32FileSize);

		if(PPAPI_FATFS_Read(pu8SrcBuf, 1, (PP_S32)u32WriteUnitSize, fh) != u32WriteUnitSize){
			printf("Error! file read fail(size: %d)\n", u32WriteUnitSize);
			ret = eERROR_FAILURE;
			goto END_FT;
		}


		if((ret = DataWriteFlash(pu8SrcBuf, u32FlashAddr + u32WrittenSize, u32WriteUnitSize, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
			printf("Error! data write fail(size: %d)\n", u32WriteUnitSize);
			ret = eERROR_FAILURE;
			goto END_FT;
		}

		if(pstFW->u32Verify){
			if((ret = DataReadFlash(pu8DstBuf, u32FlashAddr + u32WrittenSize, u32WriteUnitSize, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
				printf("Error! flash read fail\n");
				ret = eERROR_FAILURE;
				goto END_FT;
			}

			if(u32WriteUnitSize & 3){	// byte compare
				if((i=VerifyData(pu8SrcBuf, pu8DstBuf, u32WriteUnitSize)) >= 0){
					printf("Error! wrong flash data at 0x%x\n", u32FlashAddr + i);
					ret = eERROR_FAILURE;
					goto END_FT;
				}
			}else{	// word compare
				if((i=VerifyDataWord(pu8SrcBuf, pu8DstBuf, u32WriteUnitSize)) >= 0){
					printf("Error! wrong flash data at 0x%x\n", u32FlashAddr + i);
					ret = eERROR_FAILURE;
					goto END_FT;
				}

			}
		}
		u32WrittenSize += u32WriteUnitSize;

	}


	//=======================================================================
	// update header & write
	//=======================================================================

	pstFlashHdr->stSect[enFlashSection].u32Size = u32FileSize;
	pstFlashHdr->stSect[enFlashSection].u32Ver = pstFW->u32Version;
	// add crc
	//printf("crc data size: 0x%x\n", (PP_U32)&pstFlashHdr->u32CRC16 - (PP_U32)pstFlashHdr);
	pstFlashHdr->u32CRC16 = CalcCRC16IBM((PP_U8 *)pstFlashHdr, (PP_U32)&pstFlashHdr->u32CRC16 - (PP_U32)pstFlashHdr);

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PPAPI_FLASH_Erase(0, 4096);
#endif
	if((ret = DataWriteFlash((PP_U8 *)pstFlashHdr, 0, 4096, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
		printf("Error! data write fail(size: %d)\n", u32WriteUnitSize);
		goto END_FT;
	}
	printf("Flash write done\n");


#if 0
	PPAPI_FLASH_ReadHeader();
#if (BD_FLASH_TYPE == FLASH_TYPE_NAND)
	PPAPI_FLASH_ReadFTLHeader();
#endif

	ShowFlashInfo(pstFW->enNandArea == eNAND_AREA_FTL);
#endif

END_FT:
	if(pu8SrcBuf)OSAL_free(pu8SrcBuf);
	if(pu8DstBuf)OSAL_free(pu8DstBuf);
	if(fh)PPAPI_FATFS_Close(fh);


	return ret;
}
#elif(UPGRADE_METHOD == UPGRADE_METHOD_SPI)

//------------------------------------------------------------------------------------------------------------
// upgrade - SPI
//------------------------------------------------------------------------------------------------------------
static inline PP_U32 ClacChecksum(PP_U32 *pBuf, PP_U32 u32WordSize)
{
	PP_U32 u32CheckSum = 0;
	PP_U32 i;

	for(i=0;i<u32WordSize;i++){
		u32CheckSum += pBuf[i];
	}

	return u32CheckSum;
}

static void SetStatus(PP_U16 u16Status)
{
	PPDRV_SPI_SetSlaveStatus(eSPI_CHANNEL_1, u16Status);
}

static PP_RESULT_E UpgradeStart(const FWDN_FLASH_UPDATE_S *pstFW)
{
	PP_U32 u32FileSize;
	PP_U32 u32FlashSize;
	PP_FLASH_SECT_E enFlashSection;

	PP_S32 i;
	PP_RESULT_E ret = eSUCCESS;

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
#else
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;
#endif

#if DBG_TIME_MEASURE
	PP_U32 stick, etick;
#endif


	u32FileSize = pstFW->u32WriteSize;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	u32FlashSize = (u32FileSize + (gstFlashID.u32EraseBlockSize -1)) & (~(gstFlashID.u32EraseBlockSize-1));
#else
	u32FlashSize = (u32FileSize + (gstFlashNandID.u32EraseBlockSize -1)) & (~(gstFlashNandID.u32EraseBlockSize-1));
#endif

	if(pstFW->u32UpdateSection != FLASH_UPDATE_ALL){
		PP_FLASH_SECT_S stSectZero;

		memset(&stSectZero, 0, sizeof(PP_FLASH_SECT_S));

		enFlashSection = genUpdateSection[pstFW->u32UpdateSection];

		//=======================================================================
		// check updata available
		//=======================================================================
		// 1. check if setion is exist
		//if(FLASH_VER_ERROR_CHECK(pstFlashHdr->stSect[enFlashSection].u32Ver)){
		if((pstFlashHdr->stSect[enFlashSection].u32Ver == 0xffffffff) || (memcmp( &pstFlashHdr->stSect[enFlashSection], &stSectZero, sizeof(PP_FLASH_SECT_S)) == 0) ){
			ret = eERROR_FAILURE;
			printf("Flash section(%d) not exist\n", enFlashSection);
			goto END_FT;
		}

		// 2. check if size is available
		for(i=enFlashSection+1;i<eFLASH_SECT_MAX;i++){
			if(!FLASH_VER_ERROR_CHECK(pstFlashHdr->stSect[i].u32Ver)){
				break;
			}
		}

		if(i== eFLASH_SECT_MAX){	// next section not found
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
			if((pstFlashHdr->stSect[enFlashSection].u32FlashAddr + u32FlashSize) > gstFlashID.u32TotalSize){
				ret = eERROR_FAILURE;
				goto END_FT;
			}
#else
			if((pstFlashHdr->stSect[enFlashSection].u32FlashAddr + u32FlashSize) > (gstFlashNandID.u32TotalSize - SPI_NAND_NON_FTL_BLOCKS * gstFlashNandID.u32EraseBlockSize)){
				ret = eERROR_FAILURE;
				goto END_FT;
			}

#endif
		}else{						// next section found
			if((pstFlashHdr->stSect[enFlashSection].u32FlashAddr + u32FlashSize) > pstFlashHdr->stSect[i].u32FlashAddr){
				ret = eERROR_FAILURE;
				goto END_FT;
			}
		}
	}
	//=======================================================================
	// flash erase
	//=======================================================================
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	{
		u32WrittenSize = 0;
		if(pstFW->u32UpdateSection != FLASH_UPDATE_ALL)
			u32FlashAddr = pstFlashHdr->stSect[enFlashSection].u32FlashAddr;
		else
			u32FlashAddr = 0;

		while(u32WrittenSize < u32FlashSize){
			if(u32FlashSize - u32WrittenSize > 512*1024)u32WriteUnitSize = 512*1024;
			else u32WriteUnitSize = u32FlashSize - u32WrittenSize;

			printf("Flash erase. 0x%08x/0x%08x\n", u32WrittenSize, u32FlashSize);
			PPAPI_FLASH_Erase(u32FlashAddr + u32WrittenSize, u32WriteUnitSize);

			u32WrittenSize += u32WriteUnitSize;

		}
	}
#endif

#if !UPGRADE_SPI_TEST
	if(pstFW->u32UpdateSection != FLASH_UPDATE_ALL){
		if(enFlashSection == eFLASH_SECT_IFW || enFlashSection == eFLASH_SECT_MFW){
			pstFlashHdr->stSect[enFlashSection].u32Ver = 0;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
			PPAPI_FLASH_Write((PP_U8 *)pstFlashHdr, 0, sizeof(PP_FLASH_HDR_S));
#else
			PPAPI_FTL_Write((PP_U8 *)pstFlashHdr, 0, sizeof(PP_FLASH_HDR_S));
#endif
		}
	}
#endif

END_FT:
	return ret;
}

static PP_RESULT_E UpgradeData(const FWDN_FLASH_UPDATE_S *pstFW, PP_U8 *pu8Buf, PP_U32 u32Size)
{
	PP_U32 u32FileSize;
	PP_FLASH_SECT_E enFlashSection;
	PP_U32 u32FlashAddr;
	PP_U8 *pu8DstBuf = NULL;
	PP_S32 i;
	PP_RESULT_E ret = eSUCCESS;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
#else
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;
#endif

	if((pu8DstBuf = (PP_U8 *)OSAL_malloc(512*1024)) == NULL){
		printf("Error! malloc fail\n");
		ret = eERROR_FAILURE;
		goto END_FT;
	}

	if(pstFW->u32UpdateSection != FLASH_UPDATE_ALL)
		enFlashSection = genUpdateSection[pstFW->u32UpdateSection];
	u32FileSize = pstFW->u32WriteSize;
	//=======================================================================
	// flash write & verify
	//=======================================================================
	if(pstFW->u32UpdateSection != FLASH_UPDATE_ALL)
		u32FlashAddr = pstFlashHdr->stSect[enFlashSection].u32FlashAddr + pstFW->u32WrittenSize;
	else
		u32FlashAddr = pstFW->u32WrittenSize;

	if(pstFW->u32WrittenSize + u32Size > u32FileSize){
		printf("size error. written: %d, size: %d, total: %d\n", pstFW->u32WrittenSize, u32Size, u32FileSize);
		ret = eERROR_FAILURE;
		goto END_FT;
	}

	//printf("Flash write 0x%08x/0x%08x @0x%x\n", pstFW->u32WrittenSize, u32FileSize, u32FlashAddr);
#if UPGRADE_SPI_TEST
	goto END_FT;
#endif
	if((ret = DataWriteFlash(pu8Buf, u32FlashAddr, u32Size, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
		printf("Error! data write fail(size: %d)\n", u32Size);
		ret = eERROR_FAILURE;
		goto END_FT;
	}

	if(pstFW->u32Verify){
		if((ret = DataReadFlash(pu8DstBuf, u32FlashAddr, u32Size, (PP_U8)(BD_FLASH_TYPE == FLASH_TYPE_NAND))) != eSUCCESS){
			printf("Error! flash read fail\n");
			ret = eERROR_FAILURE;
			goto END_FT;
		}

		if(u32Size & 3){	// byte compare
			if((i=VerifyData(pu8Buf, pu8DstBuf, u32Size)) >= 0){
				printf("Error! wrong flash data at 0x%x\n", u32FlashAddr);
				ret = eERROR_FAILURE;
				goto END_FT;
			}
		}else{	// word compare
			if((i=VerifyDataWord(pu8Buf, pu8DstBuf, u32Size)) >= 0){
				printf("Error! wrong flash data at 0x%x\n", u32FlashAddr);
				ret = eERROR_FAILURE;
				goto END_FT;
			}

		}
	}

END_FT:

	if(pu8DstBuf)OSAL_free(pu8DstBuf);

	return ret;
}

static PP_RESULT_E UpgradeHeader(const FWDN_FLASH_UPDATE_S *pstFW)
{
	PP_FLASH_SECT_E enFlashSection;
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashHeader;
#else
	PP_FLASH_HDR_S *pstFlashHdr = (PP_FLASH_HDR_S *)&gstFlashFTLHeader;
#endif

	if(pstFW->u32UpdateSection == FLASH_UPDATE_ALL)
		return eSUCCESS;

	enFlashSection = genUpdateSection[pstFW->u32UpdateSection];
	pstFlashHdr->stSect[enFlashSection].u32Size = pstFW->u32WriteSize;
	pstFlashHdr->stSect[enFlashSection].u32Ver = pstFW->u32Version;

	pstFlashHdr->u32CRC16 = CalcCRC16IBM((PP_U8 *)pstFlashHdr, (PP_U32)&pstFlashHdr->u32CRC16 - (PP_U32)pstFlashHdr);
	
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	PPAPI_FLASH_Write((PP_U8 *)pstFlashHdr, 0, sizeof(PP_FLASH_HDR_S));
#else
	PPAPI_FTL_Write((PP_U8 *)pstFlashHdr, 0, sizeof(PP_FLASH_HDR_S));
#endif

	return eSUCCESS;
}


static void SPIIntrHandler(PP_U32 u32Status)
{
	PP_S32 s32Ret;
	PP_S32 s32Loop;
	BaseType_t xHigherPriorityTaskWoken;


	if(u32Status & SPI_INTR_RXFIFO_THRD){
		s32Loop = SPIDRV_GET_RX_FIFO_NUM(eSPI_CHANNEL_1);
		while(s32Loop--){
			s32Ret = PPUTIL_CIRCBUF_PUSH(&gRxBufISR, SPIDRV_GET_RX_FIFO(eSPI_CHANNEL_1));
			if(s32Ret){
				LOG_DEBUG_ISR("ERROR!!! ISR buffer full - 0\n");
			}
			gRecvIdx++;
		}
	}

	if(u32Status & SPI_INTR_TRANSFER_END){
		s32Loop = SPIDRV_GET_RX_FIFO_NUM(eSPI_CHANNEL_1);
		while(s32Loop--){
			s32Ret = PPUTIL_CIRCBUF_PUSH(&gRxBufISR, SPIDRV_GET_RX_FIFO(eSPI_CHANNEL_1));
			if(s32Ret){
				LOG_DEBUG_ISR("ERROR!!! ISR buffer full - 1\n");
			}
			gRecvIdx++;
		}

		// work around because of FTTH driver
		// FTTH driver makes transfer end interrupt without any data when it is initialized.
		if(gRecvIdx == 0){
			PPDRV_SPI_SetSlaveReady(eSPI_CHANNEL_1);
		}

		//release semaphore.
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(ghLockRx, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken == pdTRUE)
		{
			portYIELD_FROM_ISR();
		}

	    gRecvIdx = 0;
	}

}

static PP_U32 SPIRxIntr(PP_U8 *pu8Buf)
{
	PP_S32 s32Response = 0;
	PP_U32 u32DataSize;
	PP_S32 s32Ret;
	int i;

	PPDRV_SPI_SetRxSlaveIntr(eSPI_CHANNEL_1);
	xSemaphoreTake(ghLockRx, portMAX_DELAY);

	pu8Buf[0] = 0;

	// find SOF
	while(pu8Buf[0] != EXT_BOOT_SOF){
		s32Ret = PPUTIL_CIRCBUF_POP(&gRxBufISR, (uint8_t *)&pu8Buf[0]);
		if(s32Ret){
			// buffer empty
			LOG_DEBUG("Buffer empty-0\n");
			xSemaphoreTake(ghLockRx, portMAX_DELAY);
		}
	}

	// receive header
	i=1;
	while(i < PACKET_HDR_SIZE){
		s32Ret = PPUTIL_CIRCBUF_POP(&gRxBufISR, (uint8_t *)&pu8Buf[i]);
		if(s32Ret){

			// buffer empty
			LOG_DEBUG("Buffer empty-1\n");
			xSemaphoreTake(ghLockRx, portMAX_DELAY);

		}else{
			i++;
		}
	};


	memcpy( &u32DataSize, &pu8Buf[4], 4 );
	if(( u32DataSize + PACKET_HDR_SIZE ) >= SPICOMM_BUFF_SIZE)
	{
		LOG_DEBUG("data_size = %d\n", u32DataSize);
		s32Response = eERROR_INVALID_SIZE;
		goto END_FT;
	}

	// receive data
	i=0;
	while(i < u32DataSize){
		//if(circ_bbuf_pop(&gRxBufISR, &gBuf[PACKET_HDR_SIZE+i])){
		s32Ret = PPUTIL_CIRCBUF_POP(&gRxBufISR, (uint8_t *)&pu8Buf[PACKET_HDR_SIZE+i]);
		if(s32Ret){

			// buffer empty
			LOG_DEBUG("Buffer empty-2\n");
			xSemaphoreTake(ghLockRx, portMAX_DELAY);

		}else{
			i++;
		}
	};

	// Receive 1 packet done. Stop receive.
	PPDRV_SPI_IRQClear(eSPI_CHANNEL_1, (SPI_INTR_TRANSFER_END | SPI_INTR_RXFIFO_THRD));

END_FT:
	return s32Response;
}

static PP_S32 SPIRxFlashUpdate(FWDN_FLASH_UPDATE_S *pstFW, PP_U8 *pu8Buf)
{
	PP_U32 u32WriteUnitSize;
	PP_U32 u32CheckSum;
	PP_U8 u8LoopFlag = 1;
	PP_S32 s32Response;

	PPDRV_SPI_SetISR(eSPI_CHANNEL_1, NULL);
	while(u8LoopFlag){
		if( pstFW->u32WriteSize - pstFW->u32WrittenSize > pstFW->u32SendSize )
		{
			u32WriteUnitSize = pstFW->u32SendSize;
		}
		else
		{
			u32WriteUnitSize = pstFW->u32WriteSize - pstFW->u32WrittenSize;
			u8LoopFlag = 0;
		}

		PPDRV_SPI_RxSlaveDMA( eSPI_CHANNEL_1, pu8Buf, u32WriteUnitSize + 4, eDMA_CHANNEL_SPI1_RX, 3000 );
		PPAPI_SYS_CACHE_Invalidate((PP_U32 *)pu8Buf, u32WriteUnitSize + 4);

		u32CheckSum = ClacChecksum((PP_U32 *)pu8Buf, (u32WriteUnitSize)>>2);
		if(memcmp((void *)((PP_U32)pu8Buf + u32WriteUnitSize), &u32CheckSum, 4)){

			u8LoopFlag = 1;
			s32Response = eERROR_CHECKSUM_FAIL;
			//LOG_DEBUG("checksum_calc = %02x\n", u32CheckSum);
			LOG_DEBUG("checksum fail\n");
			SetStatus(s32Response);
			continue;
		}

		if(UpgradeData(pstFW, pu8Buf, u32WriteUnitSize)){
			printf("firmware upgrade failed\n");
			s32Response = eERROR_UPGRADE_FAIL;
			u8LoopFlag = 0;
			SetStatus(s32Response);

			goto ERROR;
		}
		pstFW->u32WrittenSize += u32WriteUnitSize;

		s32Response = eRESP_OK;

		SetStatus(s32Response);

	}

	PPDRV_SPI_SetISR(eSPI_CHANNEL_1, SPIIntrHandler);
	printf("Flash Send DONE!!!!!!!!\n");

	return s32Response;
ERROR:
	return eERROR_UPGRADE_FAIL;
}

PP_RESULT_E FlashUpdateSPI(PP_VOID)
{
	PP_S32 s32Response = 0;
	PP_U8 *pu8Buf;
	PP_U8 *pu8VerifyBuf;
	PP_RESULT_E enRet = eERROR_FAILURE;
	PP_U8 u8Cmd;
	PP_U32 u32DataSize;
	PP_U32 u32CheckSum;
	PP_U32 **ppU32Addr;
	PP_U8 u8UpgradeFlag = 1;
	FWDN_FLASH_UPDATE_S stFlashUpSPI;

	if( (ghLockRx = xSemaphoreCreateBinary()) == NULL)
	{
		LOG_DEBUG("ERROR! can't create lock\n");
	}

	PPDRV_SPI_Initialize(eSPI_CHANNEL_1, 1, 0, 0, 8);
    PPDRV_SPI_SetISR(eSPI_CHANNEL_1, SPIIntrHandler);

	if((pu8Buf = OSAL_malloc(SPICOMM_BUFF_SIZE)) == NULL){
		LOG_DEBUG("Error!!! Update malloc fail\n");
		goto END_FT;
	}

	if((pu8VerifyBuf = OSAL_malloc(SPICOMM_BUFF_SIZE)) == NULL){
		LOG_DEBUG("Error!!! Update malloc fail\n");
		goto END_FT;
	}

	while(u8UpgradeFlag){

    		if((s32Response = SPIRxIntr(pu8Buf))){
    			goto SEND_RESPONSE;
    		}
    		u8Cmd = pu8Buf[1];
    		memcpy( &u32DataSize, &pu8Buf[4], 4 );

		memcpy( &u32CheckSum, &pu8Buf[PACKET_HDR_SIZE + u32DataSize - 4], 4 );

		if( ClacChecksum((PP_U32 *)pu8Buf, (( PACKET_HDR_SIZE + u32DataSize - 4 ) >> 2)) != u32CheckSum){
			s32Response = eERROR_CHECKSUM_FAIL;
			LOG_DEBUG("checksum fail(0x%x)\n", stFlashUpSPI.u32WrittenSize);
			goto SEND_RESPONSE;
		}
		switch( u8Cmd )
		{

		case CMD_READY_CHECK:
			s32Response = eRESP_OK;
			printf("===Ready Check===\n");
			break;

		case CMD_MEMORY_WRITE:

			ppU32Addr = (PP_U32 **)&pu8Buf[8];

			LOG_DEBUG("cmd mem write. addr:0x%x, size: %d\n", (PP_U32)*ppU32Addr, u32DataSize);
			memcpy( *ppU32Addr, &pu8Buf[PACKET_HDR_SIZE + 4], u32DataSize - 4 - 4 );
			//cache_wb_range( *ppU32Addr, u32DataSize - 4 - 4 );
			s32Response = eRESP_OK;
			break;

		case CMD_UPGRADE_START:
			memcpy(&stFlashUpSPI.u32WriteSize, &pu8Buf[PACKET_HDR_SIZE], 4);
			stFlashUpSPI.u32UpdateSection = pu8Buf[PACKET_HDR_SIZE+4];
			stFlashUpSPI.u32Verify = pu8Buf[PACKET_HDR_SIZE+5];
			stFlashUpSPI.u32CurrUpgradeCnt = pu8Buf[PACKET_HDR_SIZE+6];
			stFlashUpSPI.u32TotalUpgradeCnt = pu8Buf[PACKET_HDR_SIZE+7];
			memcpy(&stFlashUpSPI.u32Version, &pu8Buf[PACKET_HDR_SIZE+8], 4);

			// add send size
			memcpy(&stFlashUpSPI.u32SendSize, &pu8Buf[PACKET_HDR_SIZE+12], 4);

			stFlashUpSPI.u32WrittenSize = 0;
			if(stFlashUpSPI.u32UpdateSection == FLASH_UPDATE_ALL)
				printf("firmware upgrade start. [all image], size: %d\n", stFlashUpSPI.u32WriteSize);
			else
				printf("firmware upgrade start. [%s], size: %d\n", szSect[genUpdateSection[stFlashUpSPI.u32UpdateSection]], stFlashUpSPI.u32WriteSize);

#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
			PPAPI_FLASH_ReadHeader();
#else
			PPAPI_FLASH_ReadFTLHeader();
#endif

			if(UpgradeStart(&stFlashUpSPI)){
				s32Response = eERROR_UPGRADE_REJECT;
				u8UpgradeFlag = 0;
				printf("firmware upgrade rejected\n");
				enRet = eERROR_UPGRADE_REJECTED;
				goto SEND_RESPONSE;
			}
			//s32Response = eRESP_OK;

			s32Response = SPIRxFlashUpdate(&stFlashUpSPI, pu8Buf);
			if(s32Response != eRESP_OK){
				u8UpgradeFlag = 0;
				printf("firmware upgrade failed\n");
			}

			continue;
			break;


		case CMD_UPGRADE_DONE:
			if(stFlashUpSPI.u32WriteSize != stFlashUpSPI.u32WrittenSize){
				printf("firmware upgrade failed. invalid size %d/%d\n", stFlashUpSPI.u32WriteSize, stFlashUpSPI.u32WrittenSize);
				s32Response = eERROR_UPGRADE_FAIL;
				u8UpgradeFlag = 0;
				goto SEND_RESPONSE;
			}

			if(UpgradeHeader(&stFlashUpSPI)){
				printf("firmware upgrade failed. update header fail\n");
				s32Response = eERROR_UPGRADE_FAIL;
				u8UpgradeFlag = 0;
				goto SEND_RESPONSE;
			}else{
				printf("firmware upgrade success\n");
				s32Response = eRESP_OK;
				if(stFlashUpSPI.u32CurrUpgradeCnt == stFlashUpSPI.u32TotalUpgradeCnt){
					enRet = eSUCCESS;
					u8UpgradeFlag = 0;
				}
			}
			break;
		case CMD_REQ_RESPONSE:
			break;
		default:
			s32Response = eERROR_UNKNOWN_PACKET;
			printf("Unknown Packet : %02x\n", u8Cmd);
			goto SEND_RESPONSE;
		}

SEND_RESPONSE:
		if(s32Response != eRESP_OK){
			printf("response error. (%d)\n", s32Response);
		}

    	SetStatus(s32Response);

	}

END_FT:
	if(pu8Buf)OSAL_free(pu8Buf);
	if(pu8VerifyBuf)OSAL_free(pu8VerifyBuf);

	return enRet;
}

//-----------------------------------------------------------------------------------------------------
#endif



void FlashUpdateSetClkPD(void)
{
	PP_U32 u32PowerUpBit;
	PP_U32 u32PowerDownBit;

	u32PowerUpBit =
		CLK_ENNUM_QSPI          |
        CLK_ENNUM_TIMERS        |
		CLK_ENNUM_UART          |
        CLK_ENNUM_DU            |
		0;

	u32PowerDownBit = (PP_U32)~u32PowerUpBit;

	clock_powerdown(u32PowerUpBit, u32PowerDownBit);

}
static PP_RESULT_E FlashUpdate(const PP_CHAR *pBuf, PP_U32 u32Size)
{
#if(UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)
	FWDN_FLASH_UPDATE_S *pstFW = (FWDN_FLASH_UPDATE_S *)pBuf;
	PP_S32 s32NumElement;
#endif
	PP_RESULT_E enRet = eSUCCESS;
	PP_S32 i;

	OSAL_sleep(1000);

	// suspend task

	for(i=0;i<MAX_TASK;i++){
		if( (i != TASK_FWDN) && (i != TASK_UI)
#ifdef CACHE_VIEW_USE
			&& (i != TASK_CACHE)
#endif
		){
			if(gHandle[i].phTask)
				vTaskSuspend(gHandle[i].phTask);
		}
	}
	printf("task suspended\n");

	//FlashUpdateSetClkPD();
	//printf("clock power down\n");

#if(UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)

	s32NumElement = u32Size / sizeof(FWDN_FLASH_UPDATE_S);

	if(pstFW == NULL || u32Size == 0){
		enRet = eERROR_FAILURE;
	}else{
		for(i=0;i<s32NumElement;i++){
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
			PPAPI_FLASH_ReadHeader();
#else
			PPAPI_FLASH_ReadFTLHeader();
#endif
			if(pstFW[i].u32UpdateSection == FLASH_UPDATE_ALL){		// All Update
				enRet = FlashUpdateAllSDCard(pBuf);
			}else{											// Partial Update - Todo: Not implemented yet
				printf("update section: %s\n", szSect[genUpdateSection[pstFW[i].u32UpdateSection]]);
				enRet = FlashUpdateSectionSDCard((PP_CHAR *)&pstFW[i], genUpdateSection[pstFW[i].u32UpdateSection]);
			}

			if(enRet)break;
		}

	}
#elif(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
	enRet = FlashUpdateSPI();
#endif

	// set default clock power down
	//SetClockPD();

	for(i=0;i<MAX_TASK;i++){
		if((i != TASK_FWDN) && (i != TASK_UI)
#ifdef CACHE_VIEW_USE
			&& (i != TASK_CACHE)
#endif

		){
			if(gHandle[i].phTask)
				vTaskResume(gHandle[i].phTask);
		}
	}

	return enRet;
}

PP_VOID vTaskFWDN(PP_VOID *pvData)
{
    int myHandleNum = TASK_FWDN;
    int timeOut = 1000; //msec
    EventBits_t eventWaitBits;
    EventBits_t eventResultBits;
    PP_RESULT_E result;
    TaskParam_t *pstParam = (TaskParam_t *)pvData;

	int i;

	LOG_DEBUG("Start Task(%s), Priority:%d\n", __FUNCTION__, (int)uxTaskPriorityGet(NULL));

    PPAPI_FATFS_EnterFS();
    for(;;)
    {
    	pstParam->u32TaskLoopCnt++;

	    //LOG_DEBUG("WaitBits(%s)\n", __FUNCTION__);
        if(gHandle[myHandleNum].phEventGroup)
        {
            eventWaitBits = 0x00FFFFFF; //0x00FFFFFF all bit
            eventResultBits = OSAL_EVENTGROUP_CMD_WaitBits(gHandle[myHandleNum].phEventGroup, eventWaitBits, pdTRUE, pdFALSE, timeOut);

            if ( eventResultBits == 0 )
            {
                //LOG_DEBUG("timeout\n");
                continue;
            }
            else
            {
                // process event bit
                if(eventResultBits & (1<<EVENT_FWDN_INIT))
                {/*{{{*/
                    //Called once when start task.
                    //No message Queue.
                    //printf("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_FWDN_INIT, psEventNameFWDN[EVENT_FWDN_INIT]);
                }/*}}}*/
                if(eventResultBits & (1<<EVENT_FWDN_MSG))
                {/*{{{*/
                    LOG_DEBUG("%s:Rcv Event[%dbit(%s)]\n", psTaskName[myHandleNum], EVENT_FWDN_MSG, psEventNameFWDN[EVENT_FWDN_MSG]);
                    {/*{{{*/
                        if(gHandle[myHandleNum].phQueue)
                        {
                            int queueCnt = 0;
                            stQueueItem queueItem;
                            if( (queueCnt = OSAL_QUEUE_CMD_GetCount(gHandle[myHandleNum].phQueue)) > 0)
                            {
                                //LOG_DEBUG("get Queue cnt:%d\n", queueCnt);
                                for(i = 0; i < queueCnt; i++)
                                {
                                    if( OSAL_QUEUE_CMD_Receive(gHandle[myHandleNum].phQueue, &queueItem, 0) == eSUCCESS )
                                    {
                                        /*
                                           LOG_DEBUG("%s:Rcv cmd:%08x, sender:%d(%s), attr:%x, length:%d\n", psTaskName[myHandleNum],
                                           queueItem.u32Cmd, queueItem.u16Sender, psTaskName[queueItem.u16Sender], queueItem.u16Attr, queueItem.u32Length);
                                           */
                                        if(queueItem.u32Cmd == CMD_FWDN_ALIVE)
                                        {						
                                        }
                                        else if(queueItem.u32Cmd == CMD_FWDN_FLASH_UPDATE){
                                        	result = FlashUpdate(queueItem.pData, queueItem.u32Length);
                                        }

                                        if(queueItem.u16Attr)
                                        {
                                            if(queueItem.u16Attr & (1<<QUEUE_CMDATTR_BIT_REQACK)) 
                                            {
                                                if(queueItem.u32Cmd == CMD_FWDN_ALIVE)
                                                {						
                                                    AppTask_SendCmd(CMD_ACK, TASK_FWDN, queueItem.u16Sender, 0, &pstParam->u32TaskLoopCnt, sizeof(pstParam->u32TaskLoopCnt), 1000);
                                                }
                                                else
                                                {
                                                    if(result == eSUCCESS)
                                                        AppTask_SendCmd(CMD_ACK, TASK_FWDN, queueItem.u16Sender, 0, NULL, 0, 1000);
                                                    else
                                                        AppTask_SendCmd(CMD_NACK, TASK_FWDN, queueItem.u16Sender, 0, &result, sizeof(result), 1000);
                                                }

                                                //AppTask_SendCmd(CMD_DISPLAY_POPUP_ON, TASK_FWDN, TASK_DISPLAY, ePopUp_ShutDown, NULL_PTR, 0, 1000);
                                            }
                                        }

                                        if( (queueItem.u32Length > 0) && (queueItem.pData != NULL) )
                                        {
                                            OSAL_free(queueItem.pData);
                                        }

                                    }
                                }
                            }
                            else
                            {
                                //ignore
                                //LOG_DEBUG("Don't remain queue.\n");
                            }
                        }
                        else
                        {
                            LOG_CRITICAL("Invalid handle.\n");
                        }

                    }/*}}}*/
                }/*}}}*/
            }
        }
    }

    PPAPI_FATFS_ReleaseFS();

    return;
}


