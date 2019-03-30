/*
 * sdcard.c
 *
 *  Created on: 2017. 5. 1.
 *      Author: ihkong
 */

#include <stdio.h>
#include "spi.h"
#include "sdcard.h"
#include "debug.h"

#define SUPPORT_WRITE		0
#define SUPPORT_DMA			0

#define SPI_CTRL_SDIO_CH	eSPI_CHANNEL_0
#define NO_RELEASE 			0
#define RELEASE 			1

static uint8 SD_Type = 0; // SD card type

uint8 spi_sdio_idle_state(void)
{
	uint16 	i;
	uint8	r1=0;
	uint16 	retry;
	uint8	DATA[1];


	// generate pulses :: 74 pulses, so that SD card to complete their init
	DATA[0] = 0xFF;
	for(i=0; i<10; i++) spi_tx(SPI_CTRL_SDIO_CH, DATA, 1);

	retry=0;

	do
	{
		// send CMD0 :: until the SD card back 0x01, enter the IDLE state
		r1 = spi_sdio_send_command(CMD0, 0, 0x95);
		retry++;
	}while((r1!=0x01) && (retry<SD_MEMORY_RETRY_COUNT));

	// fail process
	if(retry == SD_MEMORY_RETRY_COUNT) return 1;

	return 0;

}

uint8 spi_sdio_send_command(uint8 cmd, uint32 arg, uint8 crc)
{
	uint8	r1;
	uint16  retry=0;
	uint8	sDATA[8];
	uint8	rDATA[1];
	uint8   count;

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);
	sDATA[0] = 0xFF;
	for(count=0; count<10;count++)
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

	//printf("spi_cs_activate.\n");
	spi_cs_activate(SPI_CTRL_SDIO_CH);

	count = 0;

	sDATA[count++] = cmd | 0x40;
	sDATA[count++] = arg >> 24;
	sDATA[count++] = arg >> 16;
	sDATA[count++] = arg >> 8;
	sDATA[count++] = arg;
	sDATA[count++] = (crc | 1);

	sDATA[count++] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, count);

	sDATA[0] = 0xFF;
	do
	{
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA, rDATA,1);

		//printf("CMD: %d return : spi_rx rDATA = %d.\n",cmd, rDATA[0]);

		retry++;
		if(retry > SD_MEMORY_RETRY_COUNT)
		{
			//LOG_DEBUG("CMD: %d timeout return : spi_rx rDATA = %d.\n",cmd, rDATA[0]);
			break;

		}

	}while(rDATA[0] == 0xFF);


	r1 = rDATA[0];

	return r1;

}

uint8 spi_sdio_send_command_nodeasssert(uint8 cmd, uint32 arg, uint8 crc)
{

	uint8	r1;
	uint16  retry=0;
	uint8	sDATA[8];
	uint8	rDATA[1];
	uint8   count;

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);
	sDATA[0] = 0xFF;
	for(count=0; count<10;count++)
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

	spi_cs_activate(SPI_CTRL_SDIO_CH);

	count = 0;
	sDATA[count++] = cmd | 0x40;
	sDATA[count++] = arg >> 24;
	sDATA[count++] = arg >> 16;
	sDATA[count++] = arg >> 8;
	sDATA[count++] = arg;
	sDATA[count++] = (crc|1);

	sDATA[count++] = 0xFF;

	spi_tx(SPI_CTRL_SDIO_CH, sDATA, count);

	sDATA[0] = 0xFF;
	do
	{
		spi_xfer(SPI_CTRL_SDIO_CH, sDATA, rDATA,1);
		//LOG_DEBUG("spi_rx rDATA = %d.\n", rDATA[0]);


		retry++;
		if(retry > SD_MEMORY_RETRY_COUNT)
			break;

	}while(rDATA[0] == 0xFF);

	r1 = rDATA[0];

	return r1;

}


uint8 spi_sdio_init(uint32 freq, uint32 clk_mode)
{
	uint8	r1;
	uint8	sDATA[1];
	uint8	rDATA[1];
	uint16  retry=0;
	uint8	rBUF[6]={0,0,0,0,0};

	// 400KHz Clock Speed
	//spi_initialize(SPI_CTRL_SDIO_CH, 0, 400000, clk_mode, 8, NULL);
	//spi_initialize(SPI_CTRL_SDIO_CH, 0, 300000, clk_mode, 8, NULL);
	spi_initialize(SPI_CTRL_SDIO_CH, 0, PCLK/93, clk_mode, 8, NULL);
	spi_direct_cs_enable(SPI_CTRL_SDIO_CH, 1);

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);

	spi_sdio_idle_state();
	//if (spi_sdio_idle_state())
		//LOG_CRITICAL("\r\n SD Idle Fail");
	//else
		//LOG_DEBUG("\r\n SD Idle Success");

	// get the sd card version
	r1 = spi_sdio_send_command_nodeasssert(8, 0x1AA, 0x87);

	if(r1 == 0x05)
	{
		SD_Type = SD_TYPE_V1;

		sDATA[0] = 0xFF;
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

		// init SD Card, MMC Card
		retry = 0;
		do
		{
			r1= spi_sdio_send_command(CMD55, 0, 0);
			if(r1 == 0xFF){
				spi_cs_deactivate(SPI_CTRL_SDIO_CH);
				return r1;
			}

			r1= spi_sdio_send_command(ACMD41, 0, 0);

			retry++;

		}while((r1 != 0x00) && (retry < SD_MEMORY_RETRY_COUNT));

		if(retry == SD_MEMORY_RETRY_COUNT)
		{
			//LOG_DEBUG("MMC Card detected.\n");
			SD_Type = SD_TYPE_MMC;
		}


		// set spi hi speed mode
		// send 0xFF
		sDATA[0] = 0xFF;
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);


		// disable CRC check
		r1 = spi_sdio_send_command(CMD59, 0, 0x95);
		if(r1 != 0x00){
			spi_cs_deactivate(SPI_CTRL_SDIO_CH);
			return r1;
		}

		// set sector size
		r1 = spi_sdio_send_command(CMD16, 512, 0x95);
		if(r1 != 0x00){
			spi_cs_deactivate(SPI_CTRL_SDIO_CH);
			return r1;
		}

	}
	else if(r1 == 0x01)
	{
		//LOG_DEBUG("\r\n Ver 2");
		sDATA[0] = 0xFF;
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[0] = rDATA[0];	// 0x00
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[1] = rDATA[0];	// 0x00
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[2] = rDATA[0];	// 0x01
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[3] = rDATA[0];	// 0xAA

		//LOG_DEBUG("\r\n1. rBUF[0] = %d, rBUF[0] = %d, rBUF[0] = %d, rBUF[0] = %d\n", rBUF[0], rBUF[1], rBUF[2], rBUF[3]);

		sDATA[0] = 0xFF;
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
		spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

		retry = 0;
		do
		{
			r1 = spi_sdio_send_command(CMD55, 0, 0);
			//LOG_DEBUG("\n CMD55 return = %d\n", r1);
			if(r1 != 0x01){
				spi_cs_deactivate(SPI_CTRL_SDIO_CH);
				return r1;
			}

			r1 = spi_sdio_send_command(ACMD41, 0x40000000, 0);
			if(r1 != 0){
				if(++retry > SD_MEMORY_RETRY_COUNT){
					dbg("acmd41 fail. Init with cmd1\n");
					break;
				}
			}

		}while(r1 != 0);

		retry = 0;
		while(r1 != 0){
			r1 = spi_sdio_send_command(CMD1, 0x40000000, 0);
			if(r1 != 0x0){
				if(++retry > SD_MEMORY_RETRY_COUNT){
					dbg("\r\n ERROR CMD1");
					return r1;
				}
			}
		}

		r1 = spi_sdio_send_command_nodeasssert(CMD58, 0, 0);
		if(r1 != 0)
		{

			spi_cs_deactivate(SPI_CTRL_SDIO_CH);
			return r1;
		}

		sDATA[0] = 0xFF;
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[0] = rDATA[0];
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[1] = rDATA[0];
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[2] = rDATA[0];
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1); rBUF[3] = rDATA[0];


		//LOG_DEBUG("\r\n2.  rBUF[0] = %d, rBUF[0] = %d, rBUF[0] = %d, rBUF[0] = %d\n", rBUF[0], rBUF[1], rBUF[2], rBUF[3]);


		sDATA[0] = 0xFF; spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

		if(rBUF[0] & 0x40)
		{
			SD_Type = SD_TYPE_V2HC;
			//LOG_DEBUG("\r\n SD_TYPE_V2HC");
		}
		else
		{
			SD_Type = SD_TYPE_V2;
			//LOG_DEBUG("\r\n SD_TYPE_V2");
		}

	}

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);
	spi_initialize(SPI_CTRL_SDIO_CH, 0, freq, clk_mode, 8, NULL);

	return r1;

}

#if SUPPORT_WRITE
uint8 spi_sdio_write_single_block(uint32 sector, const uint8 *data)
{
	uint8 r1;
	uint16 i;
	uint16 retry;
	uint8	sDATA[1];
	uint8	rDATA[1];


	LOG_DEBUG("write sector : %d +++++\t\r", sector);


	if (SD_Type != SD_TYPE_V2HC)
	{
		sector = sector <<9;
	}

card_busy:


	//printf("CMD24. C...");

	r1 = spi_sdio_send_command(CMD24, sector, 0);
	if (r1 != 0x00)
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		goto card_busy;
		//printf("\n. spi_sdio_write_single_block 1 = %d\n", r1);
		return r1;
	}

	sDATA[0] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

	sDATA[0] = 0xFE;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);



	//printf("CMD24.D\n");


#if 0
	// Put data in a sector
	for (i = 0; i <512; i++)
	{
		//printf("\n. spi_sdio_write_single_block data = 0x%02x\n", *data);
		spi_tx(SPI_CTRL_SDIO_CH, data, 1);
		data++;
	}
#else
	spi_tx(SPI_CTRL_SDIO_CH, data, 512);
#endif
	sDATA[0] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

	spi_xfer(SPI_CTRL_SDIO_CH,sDATA, rDATA,1);
	if ((rDATA[0] & 0x1F) != 0x05)
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		LOG_DEBUG("\n. spi_sdio_write_single_block 3 = %d\n", rDATA[0]);
	    return rDATA[0];
	}




	retry = 0;
	sDATA[0] = 0xFF;
	do
	{
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA,rDATA,1);
	    retry++;
	    if (retry> 0xfffe) // if not done for a long time to write, error exit
	    {
			spi_cs_deactivate(SPI_CTRL_SDIO_CH);
			LOG_DEBUG("\n. spi_sdio_write_single_block 4 = %d\n", rDATA[0]);
	        return 1; // write timeout return 1
	    }
	}while(!rDATA[0]);

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);

	//printf("CMD24.END\n");

	return 0;

}
#endif

#if SUPPORT_DMA
uint8 spi_sdio_write_single_block_dma(uint32 sector, const uint8 *data)
{
	uint8 r1;
	uint16 i;
	uint16 retry;
	uint8	sDATA[1];
	uint8	rDATA[1];

	if (SD_Type != SD_TYPE_V2HC)
	{
		sector = sector <<9;
	}

card_busy:

	r1 = spi_sdio_send_command(CMD24, sector, 0);
	if (r1 == 130)
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		goto card_busy;

	}
	if (r1 != 0x00) return r1;

	LOG_DEBUG("\n. spi_sdio_write_single_block 1 = %d\n", r1);

	sDATA[0] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);

	sDATA[0] = 0xFE;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);


#if 0
	// Put data in a sector
	for (i = 0; i <512; i++)
	{
		//printf("\n. spi_sdio_write_single_block data = 0x%02x\n", *data);
		spi_tx(SPI_CTRL_SDIO_CH, data, 1);
		data++;
	}
#endif
	spi_tx_dma(SPI_CTRL_SDIO_CH, data, 512, 3);
	do{
		OSAL_sleep(1);
	}while(!spi_dma_get_done(SPI_CTRL_SDIO_CH));

	sDATA[0] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);



	spi_xfer(SPI_CTRL_SDIO_CH,sDATA, rDATA,1);
	if ((rDATA[0] & 0x1F) != 0x05)
	{
	    return rDATA[0];
	}


	LOG_DEBUG("\n. spi_sdio_write_single_block 3 = %d\n", rDATA[0]);


	retry = 0;
	sDATA[0] = 0xFF;
	do
	{
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA, rDATA,1);
	    retry++;
	    if (retry> 0xfffe) // if not done for a long time to write, error exit
	    {
	        return 1; // write timeout return 1
	    }
	}while(!rDATA[0]);


	LOG_DEBUG("\n. spi_sdio_write_single_block 4 = %d\n", rDATA[0]);

	return 0;

}
#endif

uint8 spi_sdio_get_response(uint8 response)
{
	uint16 count = 0xFFF;
	uint8	rDATA[1];
	uint8	sDATA[1];

	sDATA[0] = 0xff;
	do
	{
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA, rDATA,1);
		count--;
	}while((rDATA[0] != response) && count);

	if(count == 0)
		return MSD_RESPONSE_FAILURE;
	else
		return MSD_RESPONSE_NO_ERROR;
}

uint8 spi_sdio_read_data(uint8 *data, uint16 len, uint8 release)
{
	uint8	sDATA[1];


	if (spi_sdio_get_response(0xFE))
	{
		//spi_sdio_cs_disable();
		return 1;
	}

	//printf("\n. spi_sdio_read_data 1\n");

	sDATA[0] = 0xFF;
	while (len--)
	{
		spi_xfer(SPI_CTRL_SDIO_CH,sDATA, data,1);
		data++;
	}

	sDATA[0] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);


	if(release == RELEASE)
	{
		sDATA[0] = 0xFF; spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	}

	return 0;
}

#if SUPPORT_DMA
uint8 spi_sdio_read_data_dma(uint8 *data, uint16 len, uint8 release)
{
	uint8	sDATA[1];


	if (spi_sdio_get_response(0xFE))
	{
		//spi_sdio_cs_disable();
		return 1;
	}


	LOG_DEBUG("\n. spi_sdio_read_data 1\n");


#if 0
	while (len--)
	{
		spi_rx(SPI_CTRL_SDIO_CH,data,1);
		//LOG_DEBUG("\n. spi_sdio_read_data data 0x%02x\n", *data);

		data++;
	}
#endif

	gTransferEnd = 0;
	spi_rx_dma(SPI_CTRL_SDIO_CH,data,len, 2);
	do{
		OSAL_sleep(1);
	}while(!spi_dma_get_done(SPI_CTRL_SDIO_CH));

	sDATA[0] = 0xFF;
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);


	if(release == RELEASE)
	{
		sDATA[0] = 0xFF; spi_tx(SPI_CTRL_SDIO_CH, sDATA, 1);
	}

	return 0;

}
#endif

uint8 spi_sdio_read_single_block(uint32 sector, uint8 *buffer)
{
	uint8 r1;

	//LOG_DEBUG("read sector : %d +++++\t\r", sector);
	if(SD_Type != SD_TYPE_V2HC) sector = sector << 9;

card_busy:

	//printf("CMD17. C...");
	r1 = spi_sdio_send_command(CMD17, sector, 0);
	if(r1 != 0x00)
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		goto card_busy;
	}


	//printf("CMD17. D...\n");
	r1 = spi_sdio_read_data(buffer, 512, NO_RELEASE);
	if (r1 != 0)
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		return r1; // read data error!

	}
	else
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		//printf("CMD17. END\n");
		return 0;
	}

}

#if SUPPORT_DMA
uint8 spi_sdio_read_single_block_dma(uint32 sector, uint8 *buffer)
{
	uint8 r1;

	if(SD_Type != SD_TYPE_V2HC) sector = sector << 9;



card_busy:
	r1 = spi_sdio_send_command(CMD17, sector, 0);
	LOG_DEBUG("\n. spi_sdio_read_single_block 1 = %d\n", r1);

	if (r1 == 130)
	{
		spi_cs_deactivate(SPI_CTRL_SDIO_CH);
		goto card_busy;

	}
	if(r1 != 0x00) return r1;


	r1 = spi_sdio_read_data_dma(buffer, 512, NO_RELEASE);

	LOG_DEBUG("\n. spi_sdio_read_single_block 2 = %d\n", r1);


	if (r1 != 0)
		return r1; // read data error!
	else
		return 0;
}
#endif

uint8 spi_sdio_get_csd(uint8 *buf)
{
	uint8 r1;


	r1 = spi_sdio_send_command(CMD9, 0, 0xff);
	//LOG_DEBUG("\r\nspi_sdio_get_csd return = %d", r1);

	if(r1)	return r1;

	spi_sdio_read_data(buf, 16, NO_RELEASE);

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);

	return 0;

}

uint8 spi_sdio_get_cid(uint8 *buf)
{
	uint8 r1;

	r1 = spi_sdio_send_command(CMD10, 0, 0xff);
	//LOG_DEBUG("\r\nspi_sdio_get_cid return = %d", r1);

	if(r1 != 0x00) return r1;

	spi_sdio_read_data(buf, 16, NO_RELEASE);

	spi_cs_deactivate(SPI_CTRL_SDIO_CH);

	return 0;


}

