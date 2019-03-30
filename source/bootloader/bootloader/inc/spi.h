#ifndef _PI5008_SPI_H
#define _PI5008_SPI_H

#include "system.h"
#include "type.h"


//#define	SPI_CPHA	0x01			/* clock phase */
//#define	SPI_CPOL	0x02			/* clock polarity */

typedef void (*SPI_CALLBACK) (uint32 status);

typedef struct tagSPI_REG_T
{
	vuint32 id;								/* 0x000 		- ID and revision register */
	vuint32 rsv0[3];						/* 0x004 ~ 0x00c	- reserved */
	vuint32 trans_fmt;						/* 0x010 		- Transfer format register */
	vuint32 direct_io;						/* 0x014 		- Direct IO control register */
	vuint32 rsv1[2];						/* 0x018 ~ 0x01c	- reserved */
	vuint32 trans_ctrl;						/* 0x020 		- Transfer control register */
	vuint32 cmd;							/* 0x024 		- Command register */
	vuint32 addr;							/* 0x028 		- Address register */
	vuint32 data;							/* 0x02c 		- Data register */
	vuint32 ctrl;							/* 0x030 		- Control register */
	vuint32 status;							/* 0x034 		- Status register */
	vuint32 irq_enable;						/* 0x038 		- Interrupt enable register */
	vuint32 irq_status;						/* 0x03c 		- Interrupt status register */
	vuint32 timing;							/* 0x040 		- Interface timing register */
	vuint32 rsv2[3];						/* 0x044 ~ 0x04c	- reserved */
	vuint32 mem_ctrl;						/* 0x050 		- Memory access control register */
	vuint32 rsv3[3];						/* 0x054 ~ 0x05c	- reserved */
	vuint32 slave_status;					/* 0x060 		- Slave status register */
	vuint32 slave_data_cnt;					/* 0x064 		- Slave data count register */
	vuint32 rsv4[5];						/* 0x068 ~ 0x078	- reserved */
	vuint32 config;							/* 0x07c 		- Configuration register */
}SPI_REG_T;


typedef enum tagSPI_CHANNEL_E
{
	eSPI_CHANNEL_0 = 0,
	eSPI_CHANNEL_1,
	eSPI_CHANNEL_2,
	eSPI_CHANNEL_FLASH,
} SPI_CHANNEL_E;

#define SPI_FLASH_CTRL_REG		((SPI_REG_T*)			(SPI_FLASH_BASE_ADDR))
#define SPI0_CTRL_REG			((SPI_REG_T*)			(SPI0_BASE_ADDR))
#define SPI1_CTRL_REG			((SPI_REG_T*)			(SPI1_BASE_ADDR))
#define SPI2_CTRL_REG			((SPI_REG_T*)			(SPI2_BASE_ADDR))


#define SPI_INTR_RXFIFO_OVER        (1UL << 0)
#define SPI_INTR_TXFIFO_UNDER       (1UL << 1)
#define SPI_INTR_RXFIFO_THRD        (1UL << 2)
#define SPI_INTR_TXFIFO_THRD		(1UL << 3)
#define SPI_INTR_TRANSFER_END       (1UL << 4)
#define SPI_INTR_RECV_CMD           (1UL << 5)


sint32 spi_initialize(sint32 ch, uint32 slave, uint32 freq, uint32 clk_mode, uint32 wordlen, SPI_CALLBACK callback);
sint32 spi_tx(sint32 ch, const uint8 *dout, uint32 size);
sint32 spi_rx(sint32 ch, uint8 *din, uint32 size);
sint32 spi_xfer(sint32 ch, const uint8 *dout, uint8 *din, uint32 size);
sint32 spi_slave_tx(sint32 ch, const uint8 *dout, uint32 size);
sint32 spi_slave_rx(sint32 ch, uint8 *din, uint32 size);
void spi_slave_tx_byte(sint32 ch, const uint8 dout);
uint8 spi_slave_rx_byte(sint32 ch);

sint32 spi_direct_cs_enable(sint32 ch, uint8 en);
sint32 spi_cs_activate(sint32 ch);
sint32 spi_cs_deactivate(sint32 ch);

void spi_wait(sint32 ch);




#endif
