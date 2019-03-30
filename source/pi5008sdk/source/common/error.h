/**
 * \file
 *
 * \brief	Error code
 *
 * \version 0.1
 *
 * Revision History
 *
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __ERROR_H__
#define __ERROR_H__

#include "type.h"


//#define SYS_OS_SUCCESS                  HAL_SUCCESS
//#define SYS_OS_FAILURE                  HAL_FAILURE
//
//#define SYS_OS_ERR_UNHANDLED_INTERRUPT  HAL_ERR_UNHANDLED_INTERRUPT
//#define SYS_OS_ERR_INVALID_POINTER      HAL_ERR_INVALID_POINTER
//#define SYS_OS_ERR_NOT_PRESENT          HAL_ERR_NOT_PRESENT
//#define SYS_OS_ERR_UNAVAILABLE          HAL_ERR_UNAVAILABLE
//#define SYS_OS_ERR_TIMEOUT              HAL_ERR_TIMEOUT
//#define SYS_OS_ERR_NO_MEMORY            HAL_ERR_NO_MEMORY
//#define SYS_OS_ERR_INVALID_ENTRY        HAL_ERR_INVALID_ENTRY
//#define SYS_OS_ERR_INVALID_OPERATION    HAL_ERR_INVALID_OPERATION
//#define SYS_OS_ERR_INVALID_DRIVER       HAL_ERR_INVALID_DRIVER
//#define SYS_OS_ERR_INVALID_START        HAL_ERR_INVALID_START

typedef enum ppRESULT_E
{
	eSUCCESS = 0,
	eERROR_FAILURE,
	eERROR_INVALID_ARGUMENT,											/* Invalid argument */
	eERROR_NO_MEM,														/* Out of memory */
	eERROR_BUSY,
	eERROR_NOT_SUPPORT,
	eERROR_UNHANDLED_INTERRUPT,
	eERROR_INVALID_POINTER,
	eERROR_UNAVAILABLE,
	eERROR_TIMEOUT,
	eERROR_INVALID_ENTRY,
	eERROR_INVALID_OPERATION,
	eERROR_INVALID_DRIVER,
	eERROR_INVALID_START,
	eERROR_NOT_FOUND,
	eERROR_INVALID_ALIGN,
	eERROR_NO_FLASH_MEM,												/* Data does not exist in flash memroy */

	eERROR_SVM_NOT_INITIALIZE = 100,									/* Not Initialize */
	eERROR_SVM_VIEWMODE_NOT_SELECTED,									/* Not selected viewmode */
	eERROR_SVM_LIMIT_VIEWMODE,											/* Too many view mode count */
	eERROR_SVM_NOT_CREATED_VIEWMODE,									/* Viewmode was not created. */
	eERROR_SVM_UNUSE_LUT_VIEWMODE,										/* Viewmode does not use LUT. */
	eERROR_SVM_UNUSE_IMG_VIEWMODE,										/* Viewmode does not use image. */
	eERROR_SVM_RUNNING,													/* Can not be set while SVM is running. */
	eERROR_SVM_MORPHING_RUNNING,										/* Can not be set while morphing is running. */
	eERROR_SVM_INVALID_LUT,												/* There is something without lut. (FB + LR + BC)*/
	eERROR_SVM_RESOUTION_P2I,											/* For P2I(progressive to interlace), the output height of svm must be twice the size of the du output height.*/
	eERROR_SVM_MORPHING,												/* BC LUT number for morphing is larger than 3. */
	eERROR_SVM_PAGEFLIPPING_CNT,										/* The number of page-flipping should be at least two. */
	eERROR_SVM_NOT_LOADING,

	eERROR_VPU_NOT_INITIALIZE = 200,									/* Not Initialize */
	eERROR_VPU_INVALIED_ARGUMENT_TOO_MAANY_LIMIT_FEATURE_COUNT,			/* Invalid argument */
	eERROR_VPU_TOO_MAANY_LIMIT_FEATURE_COUNT_1ST_FRAME,					/* Too many feature count (1st frame)*/
	eERROR_VPU_TOO_MAANY_LIMIT_FEATURE_COUNT_2ND_FRAME,					/* Too many feature count (2nd frame)*/
	eERROR_VPU_TOO_MAANY_LIMIT_FEATURE_COUNT_BOTH_FRAME,				/* Too many feature count (1st frame and 2nd frame)*/

	eERROR_SYS_DMA_INVALID_CHANNEL = 300,								/* Invalid DMA channel */
	eERROR_SYS_DMA_CHANNEL_IN_USE,										/* DMA channel is already in use */
	eERROR_SYS_DMA_ABORT,												/* DMA abort */
	eERROR_SYS_DMA_ERROR,												/* DMA error */

    eERROR_UART_INVALID_HEADER_SOF = 400,								/* UART Frame header SoF Error */
    eERROR_UART_INVALID_HEADER_COMMAND,									/* UART Frame header Unknown Command */
    eERROR_UART_INVALID_HEADER_LENGTH,									/* UART Frame length is over 256(max) */
    eERROR_UART_INVALID_HEADER_UNKNOWN,									/* UART Frame Unknown Error */
    eERROR_UART_CRC,													/* UART Frame CRC Error */

	eERROR_DU_NOT_INITIALIZE = 500,										/* Not Initialize */

	eERROR_EDGE_NOT_INITIALIZE = 600,									/* Not Initialize */

	eERROR_LUTMERGE_NOT_INITIALIZE = 700,								/* Not Initialize */
	eERROR_LUTMERGE_NOT_SET_SECTION,									/* Section is not set */
	eERROR_LUTMERGE_VIEW_COUNT,											/* View count is larger than what is use */
	eERROR_LUTMERGE_VIEW_INDEX,											/* View index does not exist in flash memroy */
	eERROR_LUTMERGE_MERGE_TYPE,											/* Merge type error */

	eERROR_FATFS_INVALIDDRIVE = 800,
	eERROR_FATFS_NOTFORMATTED,		//1
	eERROR_FATFS_INVALIDDIR,		//2
	eERROR_FATFS_INVALIDNAME,		//3
	eERROR_FATFS_NOTFOUND,          //5
	eERROR_FATFS_DUPLICATED,        //6
	eERROR_FATFS_NOMOREENTRY,       //7
	eERROR_FATFS_NOTOPEN,           //8
	eERROR_FATFS_EOF,               //9
	eERROR_FATFS_RESERVED,          //10
	eERROR_FATFS_NOTUSEABLE,        //11
	eERROR_FATFS_LOCKED,            //12
	eERROR_FATFS_ACCESSDENIED,      //13
	eERROR_FATFS_NOTEMPTY,          //14
	eERROR_FATFS_INITFUNC,          //15
	eERROR_FATFS_CARDREMOVED,       //16
	eERROR_FATFS_ONDRIVE,           //17
	eERROR_FATFS_INVALIDSECTOR,     //18
	eERROR_FATFS_READ,              //19
	eERROR_FATFS_WRITE,             //20
	eERROR_FATFS_INVALIDMEDIA,      //21
	eERROR_FATFS_BUSY,              //22
	eERROR_FATFS_WRITEPROTECT,      //23
	eERROR_FATFS_INVFATTYPE,        //24
	eERROR_FATFS_MEDIATOOSMALL,     //25
	eERROR_FATFS_MEDIATOOLARGE,     //26
	eERROR_FATFS_NOTSUPPSECTORSIZE, //27
	eERROR_FATFS_UNKNOWN,           //28
	eERROR_FATFS_DRVALREADYMNT,     //29
	eERROR_FATFS_TOOLONGNAME,       //30
	eERROR_FATFS_NOTFORREAD,        //31
	eERROR_FATFS_DELFUNC,           //32
	eERROR_FATFS_ALLOCATION,        //33
	eERROR_FATFS_INVALIDPOS,        //34
	eERROR_FATFS_NOMORETASK,        //35
	eERROR_FATFS_NOTAVAILABLE,      //36
	eERROR_FATFS_TASKNOTFOUND,      //37
	eERROR_FATFS_UNUSABLE,          //38
	eERROR_FATFS_CRCERROR,          //39
	eERROR_FATFS_CARDCHANGED,       //40

	eERROR_UPGRADE_REJECTED = 900,
} PP_RESULT_E;

PP_VOID PPAPI_SYSTEM_Fault(PP_VOID);

#endif // __ERROR_H__
