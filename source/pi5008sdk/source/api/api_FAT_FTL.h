#ifndef _API_FAT_FTL_H_
#define _API_FAT_FTL_H_

#include "type.h"

#ifdef FAT_FTL_LIB_BUILD
typedef PP_S32 PP_RESULT_E;
#endif

/* attribute file/directory bitpattern definitions */
#define FATFS_ATTR_ARC          0x20
#define FATFS_ATTR_DIR          0x10
#define FATFS_ATTR_VOLUME       0x08
#define FATFS_ATTR_SYSTEM       0x04
#define FATFS_ATTR_HIDDEN       0x02
#define FATFS_ATTR_READONLY     0x01

//////////////////////////////////////////////////////////////
//				FAT Filesystem Structure
//////////////////////////////////////////////////////////////
typedef struct ppFNAME_S
{
	PP_S32	s32DriveNum;			/*  drive number 0-A 1-B 2-C  */
	PP_CHAR	szPath[256];    		/*  pathname /directory1/dir2/   */
	PP_CHAR szLName[256];   		/*  long file name   */
}PP_FNAME_S;

typedef struct ppFPOS_S
{
	PP_U32	u32Cluster;            	/* which cluster is used */
	PP_U32  u32PrevCluster;        	/* previous cluster for bad block handling */
	PP_U32  u32SectorBegin;        	/* calculated sector start */
	PP_U32  u32Sector;             	/* current sector */
	PP_U32  u32SectorEnd;          	/* last saector position of the cluster */
	PP_U32  u32Pos;                	/* current position */
}PP_FPOS_S;


typedef struct ppFNFIND_S
{
	PP_CHAR		szFileName[256];	/* file name+ext */
	PP_CHAR     szName[8];          /* file name */
	PP_CHAR     szExt[3];           /* file extension */
	PP_U8   	pu8Attr;            /* attribute of the file */

	PP_U16  	u16CTime;           /* creation time */
	PP_U16  	u16CDate;           /* creation date */
	PP_U32      u32FileSize;        /* length of file */

	PP_U32      u32Cluster;         /* current file starting position */
	PP_FNAME_S    stFindFSName;       /* find properties */
	PP_FPOS_S     stPos;              /* position of the current list */
}PP_FNFIND_S;



typedef struct ppFNSTAT_S
{
	PP_U32	u32FileSize;
	PP_U16  u16CreateDate;
	PP_U16  u16CreateTime;
	PP_U16  u16ModifiedDate;
	PP_U16  u16ModifiedTime;
	PP_U16  u16LastAccessDate;
	PP_U8   u8Attr;                 
	PP_S32  s32DriveNum;
}PP_FNSTAT_S;

typedef struct ppFNSPACE_S
{
	PP_U32  total;
	PP_U32  free;
	PP_U32  used;
	PP_U32  bad;

	PP_U32  total_high;
	PP_U32  free_high;
	PP_U32  used_high;
	PP_U32  bad_high;
}PP_FNSPACE_S;


//////////////////////////////////////////////////////////////
//				FTL Low Level Structure
//////////////////////////////////////////////////////////////
typedef struct ppFTL_LL_PHY_S
{
  PP_U32  u32Blocks;           /* total number of blocks */
  PP_U32  u32PagePerBlock;     /* pages per block */
  PP_U32  u32PageDataSize;        /* page data size */
  PP_U32  u32ReservedBlocks;   /* number of reserved blocks */
  PP_U32  u32FreeBlocks;       /* total number of management blocks including map, log
                                   and spare blocks */
  PP_U32  u32LogBlocks;        /* no. of log blocks */
  PP_U32  u32MapBlocks;        /* no. of map blocks */
  PP_U32  u32MapBlockShadow;  /* no. of shadow blocks for each map block */
  PP_U32  u32WearStaticLimit;  /* limit for difference between maximum and minimum wear count */
  PP_U32  u32WearStaticCount;  /* static wear checking must run after so many merge operations */
  PP_U32  u32RewriteInterval;   /* rewrite check must run after so many read/write operations */

}PP_FTL_LL_PHY_S;


//////////////////////////////////////////////////////////////
//				FTL Structure
//////////////////////////////////////////////////////////////
typedef struct ppFTL_STATS_S
{
  PP_U32  u32BlocksTotal;
  PP_U32  u32BlocksReserved;
  PP_U32  u32BlocksBad;
  PP_U32  u32PagesPerBlock;
  PP_U32  u32BytesPerPages;;
}PP_FTL_STATS_S;


//////////////////////////////////////////////////////////////
//				FTL defines
//////////////////////////////////////////////////////////////
#define NAND_DEVICE_GD1G2G		0
#define NAND_DEVICE_MT29FXG01	1
#define NAND_DEVICE_W25NXG		2

//////////////////////////////////////////////////////////////
//				FTL Low Level Functions - Do not use
//////////////////////////////////////////////////////////////
PP_U32 PPAPI_FTL_LL_Initialize(PP_U8 IN u8DeviceType);
PP_U32 PPAPI_FTL_LL_GetPhy(PP_FTL_LL_PHY_S* OUT pstPhy);
PP_U32 PPAPI_FTL_LL_Read(PP_U32 IN u32BlockAddr, PP_U32 IN u32PageOffset, PP_U8* OUT pu8Buf);
PP_U32 PPAPI_FTL_LL_ReadPart(PP_U32 IN u32BlockAddr, PP_U32 IN u32PageOffset, PP_U8* OUT pu8Buf, PP_U16 IN u16Index);
PP_U32 PPAPI_FTL_LL_Write(PP_U32 IN u32BlockAddr, PP_U32 IN u32PageOffset, PP_U8* IN pu8Buf, PP_U8* IN pu8Spare);
PP_U32 PPAPI_FTL_LL_WriteDouble(PP_U32 IN u32BlockAddr, PP_U32 IN u32PageOffset, PP_U8* IN pu8Buf0, PP_U8* IN pu8Buf1);
PP_U32 PPAPI_FTL_LL_Erase(PP_U32 IN u32BlockAddr);
PP_U32 PPAPI_FTL_LL_IsBadBlock(PP_U32 IN u32BlockAddr);
PP_U32 PPAPI_FTL_LL_ReadOneByte(PP_U32 IN u32BlockAddr, PP_U32 IN u32PageOffset, PP_U8 IN u8SparePos, PP_U8* OUT pu8Buf);


//////////////////////////////////////////////////////////////
//				FTL Functions
//////////////////////////////////////////////////////////////
PP_RESULT_E PPAPI_FTL_Initialize(PP_U8 IN u8DeviceType,PP_FTL_STATS_S* OUT pstStats);
PP_VOID PPAPI_FTL_Release(PP_VOID);
PP_RESULT_E PPAPI_FTL_Write(PP_VOID* IN pvData, PP_U32 IN u32Addr, PP_S32 IN s32Size);
PP_RESULT_E PPAPI_FTL_Read(PP_VOID* OUT pvData, PP_U32 IN u32Addr, PP_S32 IN s32Size);

//////////////////////////////////////////////////////////////
//				FAT Filesystem Functions
//////////////////////////////////////////////////////////////
PP_RESULT_E PPAPI_FATFS_Initialize(PP_VOID);
PP_RESULT_E PPAPI_FATFS_DeInitialize(PP_VOID);

PP_RESULT_E PPAPI_FATFS_EnterFS(PP_VOID);
PP_VOID PPAPI_FATFS_ReleaseFS(PP_VOID);
PP_RESULT_E PPAPI_FATFS_GetLastError(PP_VOID);

PP_RESULT_E PPAPI_FATFS_InitVolume(PP_VOID);
PP_RESULT_E PPAPI_FATFS_DelVolume(PP_VOID);
PP_RESULT_E PPAPI_FATFS_CheckVolume(PP_VOID);
PP_RESULT_E PPAPI_FATFS_GetFreeSpace(PP_FNSPACE_S* OUT pstSpace);
PP_RESULT_E PPAPI_FATFS_GetLabel(PP_CHAR* OUT szLabel, PP_S32 IN s32Len);
PP_RESULT_E PPAPI_FATFS_SetLabel(const PP_CHAR* IN szLabel);

PP_RESULT_E PPAPI_FATFS_MakeDir(const PP_CHAR* IN szDirName);	
PP_RESULT_E PPAPI_FATFS_ChangeDir(const PP_CHAR* IN szDirName);
PP_RESULT_E PPAPI_FATFS_RemoveDir(const PP_CHAR* IN szDirName);
PP_RESULT_E PPAPI_FATFS_GetCWD(PP_CHAR* OUT szBuffer, PP_S32 IN s32MaxLen);

PP_VOID* PPAPI_FATFS_Open(const PP_CHAR* IN szFileName, const PP_CHAR* IN szMode);
PP_RESULT_E PPAPI_FATFS_Close(PP_VOID* IN pvHandle);
PP_S32 PPAPI_FATFS_Read(PP_VOID* IN pvBuf, PP_S32 IN s32Size, PP_S32 IN s32NumItem, PP_VOID* IN pvHandle);
PP_S32 PPAPI_FATFS_Write(const PP_VOID* IN pvBuf, PP_S32 IN s32Size, PP_S32 IN s32NumItem, PP_VOID* IN pvHandle);
PP_CHAR PPAPI_FATFS_GetChar(PP_VOID* IN pvHandle);
PP_CHAR PPAPI_FATFS_PutChar(PP_CHAR IN cChar, PP_VOID* IN pvHandle);
PP_RESULT_E PPAPI_FATFS_EOF(PP_VOID* IN pvHandle);
PP_S32 PPAPI_FATFS_Tell(PP_VOID* IN pvHandle);
PP_RESULT_E PPAPI_FATFS_Seek(PP_VOID* IN pvHandle, PP_S32 IN s32Offset, PP_S32 IN s32Whence);

PP_RESULT_E PPAPI_FATFS_Delete(const PP_CHAR* IN szFileName);
PP_RESULT_E PPAPI_FATFS_FindFirst(const PP_CHAR* IN szFileName, PP_FNFIND_S* OUT pstFind);
PP_RESULT_E PPAPI_FATFS_FindNext(PP_FNFIND_S* OUT pstFind);
PP_RESULT_E PPAPI_FATFS_Move(const PP_CHAR* IN szFileName, const PP_CHAR* IN szNewName);
PP_RESULT_E PPAPI_FATFS_Rename(const PP_CHAR* IN szOldName, const PP_CHAR* IN szNewName);
PP_RESULT_E PPAPI_FATFS_GetAttr(const PP_CHAR* IN szFileName, PP_U8* OUT pu8Attr);
PP_RESULT_E PPAPI_FATFS_SetAttr(const PP_CHAR* IN szFileName, PP_U8 IN u8Attr);
PP_RESULT_E PPAPI_FATFS_GetTimeDate(const PP_CHAR* IN szFileName, PP_U16* OUT pu16CTime, PP_U16* OUT pu16CDate);
PP_RESULT_E PPAPI_FATFS_SetTimeDate(const PP_CHAR* IN szFileName, PP_U16 IN u16CTime, PP_U16 IN u16CDate);
PP_RESULT_E PPAPI_FATFS_Flush(PP_VOID* IN pvHandle);
PP_RESULT_E PPAPI_FATFS_Format(PP_VOID);
PP_RESULT_E PPAPI_FATFS_Stat(const PP_CHAR* IN szFileName, PP_FNSTAT_S* OUT pstStat);
PP_S32 PPAPI_FATFS_FileLength(const PP_CHAR* IN szFileName);


//////////////////////////////////////////////////////////////
//				TEST Functions - Do not use
//////////////////////////////////////////////////////////////
PP_VOID PPAPI_FATFS_DoTest(PP_VOID);
PP_U32 PPAPI_FTL_DrvTest(PP_VOID);
PP_S32 PPAPI_FTL_FATTest(PP_VOID);


#endif
