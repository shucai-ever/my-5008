/*
 * i2s.h
 *
 *  Created on: 2017. 4. 6.
 *      Author: ihkong
 */

#ifndef _PI5008_I2S_H_
#define _PI5008_I2S_H_

#include "system.h"
#include "type.h"

typedef void (*I2S_ISR_CALLBACK) (void);

typedef enum ppDAI_FORMAT_E
{
	eFORMAT_16BIT = 0,
	eFORMAT_32BIT = 2,
}PP_DAI_FORMAT_E;

typedef enum ppDAI_MLF_E
{
	eMLF_MSB_FIRST = 0,
	eMLF_LSB_FIRST,
}PP_DAI_MLF_E;

typedef enum ppDAI_BUFF_SIZE_E
{
	eBUFF_SIZE_4K = 0,
	eBUFF_SIZE_8K,
	eBUFF_SIZE_16K,
	eBUFF_SIZE_32K,
}PP_DAI_BUFF_SIZE_E;

typedef enum ppDAI_RFS_E
{
	eRFS_256FS = 0,			// fs = mclk / 256
	eRFS_384FS,				// fs = mclk / 384
	eRFS_512FS,				// fs = mclk / 512
	eRFS_768FS,				// fs = mclk / 768
}PP_DAI_RFS_E;

typedef struct ppI2S_INIT_S{
	PP_DAI_FORMAT_E enDataFormat;		// 0: 16, 2: 32 bit
	PP_DAI_MLF_E enMLBFirst;			// MSB/LSB first
	PP_DAI_BUFF_SIZE_E enBufferSize;	// PCM Buffer Size 0: 4K, 1: 8K, 2: 16K, 3: 32K
	PP_DAI_RFS_E enSampleFreqRatio;		// sample freq & mclk ratio - master only
	PP_U32 u32UnderMargin;				// Tx under-run margin - word align
	PP_U32 u32IsMaster;					// 0: slave, 1: master
}PP_I2S_INIT_S;

PP_RESULT_E PPDRV_I2S_Initialize(PP_I2S_INIT_S IN *pstInit);
PP_VOID PPDRV_I2S_SetISR(I2S_ISR_CALLBACK IN cbISR);
PP_VOID PPDRV_I2S_WriteEnable(PP_S32 IN s32Enable);
PP_VOID PPDRV_I2S_ResetBufPtr(PP_VOID);
PP_VOID PPDRV_I2S_WriteBuf(PP_U8 IN *pu8Buf, PP_U32 IN u32Size);
PP_S32 PPDRV_I2S_GetWriteBuffAvailSize(PP_VOID);


#endif
