#ifndef __APP_FWDN_H__
#define __APP_FWDN_H__

#include "type.h"
#include "error.h"
#include "api_flash.h"
#include "board_config.h"

/*----------------------------------------------
 *	FEATURE DEFINE
 */

#if 0
#define FLASH_UPDATE_ALL				0xFFFFFFFF
#define FLASH_UPDATE_BOOTLOADER			(1<<0)
#define FLASH_UPDATE_SECOND_BOOTLOADER	(1<<1)
#define FLASH_UPDATE_ISP_FW				(1<<2)
#define FLASH_UPDATE_MAIN_FW			(1<<4)
#define FLASH_UPDATE_SVM_LUT			(1<<6)
#define FLASH_UPDATE_CAR_IMG			(1<<7)
#define FLASH_UPDATE_MENU				(1<<8)
#define FLASH_UPDATE_GUI_IMG			(1<<9)
#define FLASH_UPDATE_PGL_IMG			(1<<10)
#define FLASH_UPDATE_ISP_DATA			(1<<11)
#define FLASH_UPDATE_SENSOR_DATA		(1<<12)
#define FLASH_UPDATE_CALIB				(1<<13)
#endif

#define FLASH_UPDATE_ALL				0xFF
#define FLASH_UPDATE_ISP_FW				0
#define FLASH_UPDATE_MAIN_FW			1
#define FLASH_UPDATE_SVM_LUT			2
#define FLASH_UPDATE_MENU				3
#define FLASH_UPDATE_GUI_IMG			4
#define FLASH_UPDATE_ISP_DATA			5
#define FLASH_UPDATE_SENSOR_DATA		6
#define FLASH_UPDATE_CALIB_MAIN			7
#define FLASH_UPDATE_CALIB_SUB			8
#define FLASH_UPDATE_SECTION_MAX		9

#define PACKET_HDR_SIZE     8
#define EXT_BOOT_SOF        0xbc
//=================================
//  Packet CMD Define
//=================================
#define CMD_BAUD_SET        0x01
#define CMD_READY_CHECK     0x02
#define CMD_RESPONSE        0x03
#define CMD_REG_SET         0x04
#define CMD_MEMORY_WRITE    0x05
#define CMD_PROG_JUMP       0x06
#define CMD_REQ_RESPONSE	0x07

#define CMD_FLASH_SETUP     0x10
#define CMD_FLASH_ERASE     0x11
#define CMD_FLASH_PROG      0x12
#define CMD_FLASH_PROG_DONE 0x13
#define CMD_FLASH_ERASE2 	0x14
#define CMD_FLASH_SEND 		0x15

#define CMD_UPGRADE_START	0x20
#define CMD_UPGRADE_DATA	0x21
#define CMD_UPGRADE_DONE	0x22
#define CMD_UPGRADE_READY 	0x23

#define CMD_REMOCON_CONTROL	0x30
//=================================

//=================================
//	REMOCON  CONTROL Define
//=================================
#define REMOCON_UP 			0x01
#define REMOCON_UPLONG 		0x02
#define REMOCON_DOWN 		0x03
#define REMOCON_DOWNLONG 	0x04
#define REMOCON_LEFT		0x05
#define REMOCON_LEFTLONG 	0x06
#define REMOCON_RIGHT 		0x07
#define REMOCON_RIGHTLONG 	0x08
#define REMOCON_CENTER 		0x09
#define REMOCON_CENTERLONG 	0x0A
#define REMOCON_MENU 		0x0B
#define REMOCON_MENULONG 	0x0C
#define REMOCON_NAVI 		0x0D
#define REMOCON_NAVILONG 	0x0E
//=================================

#if 1

#if(UPGRADE_METHOD == UPGRADE_METHOD_SDCARD)
typedef struct ppFWDN_FLASH_UPDATE_S
{
	PP_CHAR szFileName[256];
	PP_U32 u32UpdateSection;
	PP_U32 u32Verify;
	PP_U32 u32Version;
}FWDN_FLASH_UPDATE_S;
#elif(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
typedef struct ppFWDN_FLASH_UPDATE_S
{
	PP_U32 u32WriteSize;			// Total data size
	PP_U32 u32UpdateSection;		// Upgrade section
	PP_U32 u32Verify;				// Verify flag
	PP_U32 u32Version;				// Version
	PP_U32 u32SendSize;				// SPI rx size at a time
	PP_U32 u32WrittenSize;			// Data written size to flash
	PP_U32 u32CurrUpgradeCnt;		// The number of current upgrade count
	PP_U32 u32TotalUpgradeCnt;		// The number of total upgrade count
}FWDN_FLASH_UPDATE_S;

#endif

#else
typedef struct ppFWDN_FLASH_UPDATE_S
{
	PP_UPDATE_SRC_E enSrcMedia;
	union {
		struct {
			PP_U32 u32ADDR;
			PP_U32 u32Size;
		}dram;
		struct{
			PP_CHAR szFileName[256];
		}file;
	}src;

	PP_FLASH_TYPE_E enFlashType;
	PP_U32 u32PageSize;
	PP_U32 u32EraseBlockSize;

	PP_U32 u32UpdateSection;
	PP_U32 u32Verify;

	PP_NAND_AREA_E enNandArea;

}FWDN_FLASH_UPDATE_S;


#endif

#if(UPGRADE_METHOD == UPGRADE_METHOD_SPI)
PP_RESULT_E FlashUpdateSPI(PP_VOID);
#endif

PP_VOID vTaskFWDN(PP_VOID *pvData);
#endif // __APP_FWDN_H__
