#ifndef _PI5008_FLASH_BOOT
#define _PI5008_FLASH_BOOT

#include "type.h"

#define FLASH_HEADER_EMPTY			(0xFFFFFFFF)
#define FLASH_VER_ERROR_CHECK(x)		(x==0 || x==FLASH_HEADER_EMPTY)

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
	eFLASH_SECT_CAM_CALIB,		//13
	eFLASH_SECT_CAM_CAPTURE,	//14
	eFLASH_SECT_AUDIO,			//15
	eFLASH_SECT_MAX,			//16
}PP_FLASH_SECT_E;

typedef struct ppFLASH_SECT_S{
	uint32 u32Ver;
	uint32 u32FlashAddr;			// boot loader flash address
	uint32 u32LoadAddr;			// boot loader load address in ram
	uint32 u32Size;				// boot loader size
}PP_FLASH_SECT_S;

typedef struct ppFLASH_HDR_S
{
	uint32 u32Sign;

	uint32 u32FlashType;
	uint32 u32PageSize;
	uint32 u32PagesBlock;

	uint32 u32ClockDiv;			// boot rom spi controller clock divider

	PP_FLASH_SECT_S stSect[eFLASH_SECT_MAX];

}PP_FLASH_HDR_S;


#if 0
typedef struct tagFLASH_HDR_T
{
	unsigned int sign;

	unsigned int flash_type;
	unsigned int page_size;
	unsigned int pages_block;

	unsigned int clock_div;			// boot rom spi controller clock divider
	
	unsigned int bl_ver;
	unsigned int bl_flash_addr;		// boot loader flash address
	unsigned int bl_load_addr;		// boot loader load address in ram
	unsigned int bl_size;			// boot loader size

	unsigned int ifw_ver;
	unsigned int ifw_flash_addr; 	// isp f/w flash address
	unsigned int ifw_load_addr;		// isp f/w load address in ram
	unsigned int ifw_size;			// isp f/w size

	unsigned int mfw_ver;
	unsigned int mfw_flash_addr;	// main f/w flash address
	unsigned int mfw_load_addr; 	// main f/w load address in ram
	unsigned int mfw_size;			// main f/w size

	unsigned int lut_ver;
	unsigned int lut_flash_addr;	// lut flash address
	unsigned int lut_size;			// lut size
	
	unsigned int car_img_ver;
	unsigned int car_img_flash_addr;	// car image flash address
	unsigned int car_img_size;			// car image size
	
	unsigned int menu_ver;
	unsigned int menu_flash_addr;	// menu flash address
	unsigned int menu_size;			// menu size

	unsigned int gui_ver;
	unsigned int gui_flash_addr;	// gui image flash address
	unsigned int gui_size;			// gui image size

	unsigned int pgl_ver;
	unsigned int pgl_flash_addr;	// pgl image flash address
	unsigned int pgl_size;			// pgl image size

	unsigned int isp_ver;
	unsigned int isp_flash_addr;	// isp init data flash address
	unsigned int isp_size;			// isp init data size

	unsigned int sensor_ver;
	unsigned int sensor_flash_addr;	// sensor init data flash address
	unsigned int sensor_size;		// sensor init data size
	
}FLASH_HDR_T;
#endif

uint32 spi_nor_flash_boot(void);
uint32 spi_nand_flash_boot(void);


#endif

