/**
 * \file
 *
 * \brief	common struture & enumeration
 *
 * Copyright (c) 2016 Pixelplus Co.,Ltd. All rights reserved
 *
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "board_config.h"

/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppCHANNEL_E
{
	eCHANNEL_FRONT = 0,							// cam0
	eCHANNEL_LEFT,								// cam1
	eCHANNEL_RIGHT,								// cam2
	eCHANNEL_REAR,								// cam3
	eCHANNEL_MAX,
} PP_CHANNEL_E;

typedef enum ppFIELD_E
{
	eFIELD_NONE = 0,							// progressive
	eFIELD_ODD = 0,								// odd field of interlace
	eFIELD_EVEN,								// even field of interlace
	eFIELD_MAX,
} PP_FIELD_E;

typedef enum ppFLASH_LOADING_TYPE_E
{
#if (BD_FLASH_TYPE == FLASH_TYPE_NOR)
	eFLASH_LOADING_TYPE_NOR = 0,
#else
	eFLASH_LOADING_TYPE_FTL = 0,
	eFLASH_LOADING_TYPE_NONFTL,
#endif
	eFLASH_LOADING_TYPE_MAX,
} PP_FLASH_LOADING_TYPE_E;


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
typedef struct ppPOS_S
{
	PP_U16 u16X;
	PP_U16 u16Y;
} PP_POS_S;

typedef struct ppSIZE_S
{
	PP_S16 u16Width;
	PP_S16 u16Height;
} PP_SIZE_S;

typedef struct ppRECT_S
{
	PP_U16 u16X;
	PP_U16 u16Y;
	PP_U16 u16Width;
	PP_U16 u16Height;
} PP_RECT_S;

typedef struct ppVID_S
{
	PP_S8 s8VidPort;
	PP_S8 s8Status;
	PP_S8 s8Resv0;
	PP_S8 s8Resv1;
} PP_VID_S;

#define PP_UNUSED_ARG( arg ) (void)( arg )

#endif // __COMMON_H__
