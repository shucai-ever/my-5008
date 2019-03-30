#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "spi_flash.h"
#include "debug.h"


#define CMD_READ_ID					0x9f

#define CMD_WRITE_STATUS			0x01
#define CMD_WRITE_DISABLE			0x04
#define CMD_READ_STATUS				0x05
#define CMD_WRITE_ENABLE			0x06

PP_FLASH_ID_S gstFlashIDTable[] = {
		// MXIC
		{ .u8ManufacturerID = NOR_MANID_MXIC, .u8DeviceID0 = 0x20, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// MXIC, 3.3V
		{ .u8ManufacturerID = NOR_MANID_MXIC, .u8DeviceID0 = 0x25, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// MXIC, 1.8V
		// ISSI
		{ .u8ManufacturerID = NOR_MANID_ISSI, .u8DeviceID0 = 0x60, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// ISSI, 3.3V
		{ .u8ManufacturerID = NOR_MANID_ISSI, .u8DeviceID0 = 0x70, .u32PageSize = 256, .u32EraseBlockSize = 4*1024  },	// ISSI, 1.8V

};
PP_FLASH_ID_S gstFlashID;

static sint32 flash_cmd_wait_ready(uint32 timeout)
{
	sint32 status;
	
	do{
		status = flash_read_status();
		//printf("status: 0x%x\n", status);
	}while(status & 1);

	return 1;

}



sint32 flash_init(uint32 freq, uint32 clk_mode, uint8 quad_io)
{
	sint32 status;
	
    spi_initialize(eSPI_CHANNEL_FLASH, 0, freq, clk_mode, 8, NULL);

    //get flash id
#if 0
    {
        uint8 id[8];
        flash_read_id(id);
        //printf("flash id: 0x%x, 0x%x, 0x%x, 0x%x\n", id[0], id[1], id[2], id[3]);
    }
#endif
	memset(&gstFlashID, 0, sizeof(gstFlashID));
	if(flash_find_id(&gstFlashID)){
		return -1;

	}

	status = flash_read_status();
	if( ((status>>6)&1) != quad_io){	// need quad mode set

		flash_write_enable(1);
		do{
			status = flash_read_status();
		}while(!(status & 2));
	
		if(quad_io)
			status |= (1<<6);
		else
			status &= (~(1<<6));

		flash_write_status(status);
		flash_cmd_wait_ready(1);

		flash_write_enable(0);
	}

	return 0;
}

sint32 flash_find_id(PP_FLASH_ID_S *pstID)
{
	uint8 id[8];
	uint32 i;

	// find flash manufacturer id
	flash_read_id(id);
	for(i=0;i<sizeof(gstFlashIDTable)/sizeof(gstFlashIDTable[0]);i++){
		if(gstFlashIDTable[i].u8ManufacturerID == id[1] && gstFlashIDTable[i].u8DeviceID0 == id[2]){
			memcpy(pstID, &gstFlashIDTable[i], sizeof(PP_FLASH_ID_S));
			pstID->u32TotalSize = 1024 * 1024 * (1<<((id[3]&0xf)-1)) / 8;
			if(pstID->u32TotalSize >= (32*1024*1024)){
				pstID->u8AddrMode4Byte = 1;
			}else{
				pstID->u8AddrMode4Byte = 0;
			}
			break;
		}
	}
	if(pstID->u8ManufacturerID == 0){
		dbg("error! unsupported flash id(0x%x)\n", id[1]);
		return -1;
	}

	dbg("Serial NOR Flash. Id: 0x%x, 0x%x, 0x%x, 0x%x\n", id[0], id[1], id[2], id[3]);
	dbg("Flash size: 0x%x, Page size: 0x%x, Block size: 0x%x\n", pstID->u32TotalSize, pstID->u32PageSize, pstID->u32EraseBlockSize );

	return 0;
}

sint32 flash_read_id(uint8 *id)
{
	uint8 cmd[4];
	uint8 buf[4];
	sint32 size;

	size = 4;
	memset(cmd, 0, sizeof(cmd));
	cmd[0] = CMD_READ_ID;
	

	spi_xfer(eSPI_CHANNEL_FLASH, cmd, buf, size);

	memcpy(id, buf, 4);

	return 0;	
}

sint32 flash_read_status(void)
{
	uint8 cmd[2];
	uint8 status[2];
	
	cmd[0] = CMD_READ_STATUS;
	
	spi_xfer(eSPI_CHANNEL_FLASH, cmd, status, 2);

	return (uint32)status[1];

}

sint32 flash_write_status(uint8 status)
{
	uint8 cmd[2];
	
	cmd[0] = CMD_WRITE_STATUS;
	cmd[1] = status;
	
	//spi_xfer(cmd, NULL, 2);
	spi_tx(eSPI_CHANNEL_FLASH, cmd, 2);

	return 0;

}


sint32 flash_write_enable(uint8 enable)
{
	uint8 cmd;

	if(enable)cmd = CMD_WRITE_ENABLE;
	else cmd = CMD_WRITE_DISABLE;
	
	//spi_xfer(&cmd, NULL, 1);
	spi_tx(eSPI_CHANNEL_FLASH, &cmd, 1);

	return 0;

}

