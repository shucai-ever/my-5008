#ifndef __PVIRX_DRVCOMMON_H__
#define __PVIRX_DRVCOMMON_H__

#include "pvirx_support.h"
#include "type.h"
/************************************************************/
/*                   App & Driver common 		    */
/************************************************************/
typedef enum {
	NTSC = 0,
	PAL,
	MAX_VIDEO_SYSTEM_TYPE
}PP_PVIRX_VIDEO_SYSTEM_E;

/***************************************************************************************************************/
/* error module */
typedef enum {
	PR_ERR_COMMON = 0x01,
	PR_ERR_VDEC = 0x02,
	PR_ERR_UTC = 0x03,
	PR_ERR_OSG = 0x04,

	MAX_PR_ERR_MODULE
}PR_ERR_MODULE_E;
/* error code */
typedef enum {
	/* common */
	PR_ERR_HANDLE = 0x001,
	PR_ERR_INVALID_DRVFD = 0x002,
	PR_ERR_ILLEGAL_PARAM = 0x003,
	PR_ERR_EXIST = 0x004,
	PR_ERR_UNEXIST = 0x005,
	PR_ERR_NULL_PTR = 0x006,
	PR_ERR_NOT_PERM = 0x007,
	PR_ERR_NOT_SUPPORT = 0x008,
	PR_ERR_NOT_CONFIG = 0x009,
	PR_ERR_NO_MEM = 0x00A,
	PR_ERR_NO_BUF = 0x00B,
	PR_ERR_FULL_BUF = 0x00C,
	PR_ERR_EMPTY_BUF = 0x00D,
	PR_ERR_NOT_READY = 0x00E,
	PR_ERR_BUSY = 0x00F,
	PR_ERR_INVALID_DEVID = 0x010,
	PR_ERR_INVALID_CHNID = 0x011,
	PR_ERR_LOCK = 0x012,
	PR_ERR_UNLOCK = 0x013,
	PR_ERR_PUT_QUEUE = 0x014,
	PR_ERR_GET_QUEUE = 0x015,

	MAX_PR_ERR_CODE
}PR_ERR_CODE_E;


/* error code */
#define PR_ERROR_CODE_TYPE_APP	(0x0)
#define PR_ERROR_CODE_TYPE_LIB	(0x1)
#define PR_ERROR_CODE_TYPE_DRV	(0x2)

#define PR_ERROR_CODE(type, module, code)	(0xF000F000|((type&0xF)<<24)|((module&0xFF)<<16)|((code)&0xFFF))
#define PR_ERROR_CODE_APP(module, code)		PR_ERROR_CODE(PR_ERROR_CODE_TYPE_APP, module, code)
#define PR_ERROR_CODE_LIB(module, code)		PR_ERROR_CODE(PR_ERROR_CODE_TYPE_LIB, module, code)
#define PR_ERROR_CODE_DRV(module, code)		PR_ERROR_CODE(PR_ERROR_CODE_TYPE_DRV, module, code)
/***************************************************************************************************************/

/********************************************************************/
/*				app & drv common struct								*/
/********************************************************************/
typedef struct
{
	uint8_t chanAddr;
	uint8_t vinMode;
}_stAttrChip;

typedef struct
{
	uint8_t u8Reg;
	uint8_t u8Mask;
	uint8_t u8Data;
}_stPrReg;

#ifdef SUPPORT_PVIRX_VID
typedef struct
{
	uint16_t u16HActive; //b[12:0]
	uint16_t u16HDelay; //b[12:0]
	uint16_t u16VActive; //b[10:0]
	uint16_t u16VDelay; //b[10:0]
	uint16_t u16HSCLRatio; //b[15:0] 0:skip write
}_stChnAttr;

typedef struct
{
	uint8_t u8CbGain;
	uint8_t u8CrGain;
	uint8_t u8CbOffset;
	uint8_t u8CrOffset;
}_stCscAttr;

typedef struct
{
	uint8_t u8Contrast;
}_stContrast;

typedef struct
{
	uint8_t u8Bright;
}_stBright;

typedef struct
{
	uint8_t u8Saturation;
}_stSaturation;

typedef struct
{
	uint8_t u8Hue;
}_stHue;

typedef struct
{
	uint8_t u8Sharpness; //b[3:0]
}_stSharpness;
#endif // SUPPORT_PVIRX_VID

#ifdef SUPPORT_PVIRX_UTC
#define MAX_PVIRX_UTC_BUF_SIZE           (64)
typedef struct
{
        uint8_t utcCmd[MAX_PVIRX_UTC_BUF_SIZE];
        uint8_t utcCmdLength;
}_stUTCCmd;
#endif // SUPPORT_PVIRX_UTC

#ifdef SUPPORT_PVIRX_CEQ

#endif // SUPPORT_PVIRX_CEQ

#endif /* __PVIRX_DRVCOMMON_H__ */
