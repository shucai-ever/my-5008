/*
 * sdcard.h
 *
 *  Created on: 2017. 5. 1.
 *      Author: ihkong
 */

#ifndef _PI5008_SDCARD_H_
#define _PI5008_SDCARD_H_
#include "type.h"

#define SDCARD_BLOCK_SIZE            512

// SD Card Type Definition
#define	SD_TYPE_MMC		1
#define SD_TYPE_V1		2
#define SD_TYPE_V2		3
#define	SD_TYPE_V2HC	4

// SD Card instruction sheet
#define CMD0 	0 	// Card Reset
#define CMD1 	1
#define CMD9 	9 	// command 9, CSD data read
#define CMD10 	10 	// Command 10, read CID data
#define CMD12 	12 	// command 12, to stop data transmission
#define CMD16 	16 	// Command 16, set SectorSize should return 0x00
#define CMD17 	17 	// Command 17, read sector
#define CMD18 	18 	// Command 18, read Multi sector
#define ACMD23 	23 	// Command 23, set the multi-sector erase writing in advance of a block N
#define CMD24 	24 	// Command 24, write sector
#define CMD25 	25 	// Command 25, write Multi sector
#define ACMD41 	41 	// command to 41, should return 0x00
#define CMD55 	55 	// command to 55, should return 0x01
#define CMD58 	58 	// Command 58, read OCR information
#define CMD59 	59 	// command to 59, enables / disables the CRC, should return 0x00

// Write data to respond to the word meaning
#define MSD_DATA_OK 			0x05
#define MSD_DATA_CRC_ERROR 		0x0B
#define MSD_DATA_WRITE_ERROR 	0x0D
#define MSD_DATA_OTHER_ERROR 	0xFF

// SD card labeled word response
#define MSD_RESPONSE_NO_ERROR 		0x00
#define MSD_IN_IDLE_STATE 			0x01
#define MSD_ERASE_RESET 			0x02
#define MSD_ILLEGAL_COMMAND 		0x04
#define MSD_COM_CRC_ERROR 			0x08
#define MSD_ERASE_SEQUENCE_ERROR 	0x10
#define MSD_ADDRESS_ERROR 			0x20
#define MSD_PARAMETER_ERROR 		0x40
#define MSD_RESPONSE_FAILURE 		0xFF

#define SD_MEMORY_RETRY_COUNT 		255


uint8 spi_sdio_send_command(uint8 cmd, uint32 arg, uint8 crc);
uint8 spi_sdio_send_command_nodeasssert(uint8 cmd, uint32 arg, uint8 crc);
uint8 spi_sdio_idle_state(void);
uint8 spi_sdio_read_data(uint8 *data, uint16 len, uint8 release);
uint8 spi_sdio_read_data_dma(uint8 *data, uint16 len, uint8 release);
uint8 spi_sdio_get_response(uint8 response);
uint8 spi_sdio_get_csd(uint8 *buf);
uint8 spi_sdio_get_cid(uint8 *buf);

uint8 spi_sdio_init(uint32 freq, uint32 clk_mode);
uint8 spi_sdio_read_single_block(uint32 sector, uint8 *buffer);
uint8 spi_sdio_read_single_block_dma(uint32 sector, uint8 *buffer);
uint8 spi_sdio_write_single_block(uint32 sector, const uint8 *data);
uint8 spi_sdio_write_single_block_dma(uint32 sector, const uint8 *data);


#endif /* _PI5008_SDCARD_H_ */
