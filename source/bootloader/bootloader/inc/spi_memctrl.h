/*
 * spi_memctrl.h
 *
 *  Created on: 2017. 7. 24.
 *      Author: ihkong
 */

#ifndef _SPI_MEMCTRL_H_
#define _SPI_MEMCTRL_H_

#include "type.h"
#include "system.h"

#define SPI_FLASH_TYPE_NOR			0
#define SPI_FLASH_TYPE_NAND			1

#define SPI_MEMC_QDMA_MODE			0
#define SPI_MEMC_MEMORY_MAP_MODE	1

#define SPI_MEMCTRL_READ_SINGLE		0x0
#define SPI_MEMCTRL_READ_FAST		0x1
#define SPI_MEMCTRL_READ_DUAL		0x2
#define SPI_MEMCTRL_READ_DUAL_IO	0xA
#define SPI_MEMCTRL_READ_QUAD		0x4
#define SPI_MEMCTRL_READ_QUAD_IO	0xc

#define SPI_MEMCTRL_MODE_STATE_NONE	0x0
#define SPI_MEMCTRL_MODE_STATE_HIZ	0x1
#define SPI_MEMCTRL_MODE_STATE_VAL	0x2

#define SPI_MEMCTRL_MODE_SIZE_0		0x0
#define SPI_MEMCTRL_MODE_SIZE_4		0x4
#define SPI_MEMCTRL_MODE_SIZE_8		0x8

#define SPI_MEMCTRL_DUMMY_SIZE_0	0x0
#define SPI_MEMCTRL_DUMMY_SIZE_2	0x2
#define SPI_MEMCTRL_DUMMY_SIZE_4	0x4
#define SPI_MEMCTRL_DUMMY_SIZE_8	0x8
#define SPI_MEMCTRL_DUMMY_SIZE_16	0x10


typedef struct tagSPI_MEM_REG_T
{
	vuint32 buffer[4];						/* 0x000~0x00c 	- Buffer */
	vuint32 ctrl;							/* 0x010		 	- control register */
	vuint32 divider;						/* 0x014		 	- divider register */
	vuint32 rx_delay;						/* 0x018		 	- delay rx clock register */
	vuint32 fr_cmd;							/* 0x01c		 	- Read command register */
	vuint32 fr_info;						/* 0x020		 	- Info register */
	vuint32 fr_addr;						/* 0x024			- flash read address register */
	vuint32 dst_addr[2];					/* 0x028~0x02c		- write address register */
	vuint32 dst_size[2];					/* 0x030~0x034		- write size or bulk size(dst_size[0] when memory mapped access mode) register */
	vuint32 buf_sel;						/* 0x038			- buf sel register */
	vuint32 buf_start;						/* 0x03c			- buf start register */
	vuint32 crc_en;							/* 0x040			- crc16 enable */
	vuint32 crc_result;						/* 0x044			- crc16 result */

}SPI_MEM_REG_T;


typedef struct tagSPI_MEM_FR_INFO_T
{
	uint32 dummy_size		: 7;
	uint32 rsv1				: 1;
	uint32 mode_size		: 4;
	uint32 mode_out_state	: 4;
	uint32 mode_value		: 8;
	uint32 read_type		: 4;
	uint32 addr_4byte		: 1;
	uint32 rsv2				: 2;
	uint32 flash_type		: 1;
}SPI_MEM_FR_INFO;

#define SPI_MEM_CTRL_REG		((SPI_MEM_REG_T*)		(QSPI_BASE_ADDR))

void spi_memctrl_set(uint32 div, uint32 rx_delay, uint8 read_cmd, SPI_MEM_FR_INFO fr_info);
int spi_memctrl_mode_set(uint8 mode, uint32 bulk_size);
int spi_memctrl_qdma_fill_buff(uint32 flash_addr, uint8 buf_idx, uint32 dst_addr, uint32 size);
int spi_memctrl_read_qdma(uint32 flash_addr, uint8 *buf, uint32 size);
void spi_memctrl_crc_en(uint32 en);
uint16 spi_memctrl_crc_get(void);

#endif /* _SPI_MEMCTRL_H_ */
