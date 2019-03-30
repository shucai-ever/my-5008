#include <stdio.h>
#include <string.h>
#include "type.h"
#include "bootloader.h"
#include "spi.h"
#include "uart.h"
#include "ext_boot.h"
#include "cache.h"
#include "debug.h"
#include "common.h"

#define EXT_BOOT_SPI_CH		eSPI_CHANNEL_1
#define BAUD_RATE			115200
//#define BAUD_RATE			460800

#define PACKET_HDR_SIZE		8
#define EXT_BOOT_SOF		0xbc
//=================================
//	Packet CMD Define
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

#define CMD_UPGRADE_START	0x20
#define CMD_UPGRADE_DATA	0x21
#define CMD_UPGRADE_DONE	0x22
//=================================

typedef enum tagCMD_RESP_E
{
	eRESP_OK = 0,
	eERROR_INVALID_HDR,
	eERROR_INVALID_SIZE,
	eERROR_CHECKSUM_FAIL,
	eERROR_UNKNOWN_PACKET
}CMD_RESP_E;

extern void write_boot_status(uint8 status);

static uint8 *gBuf;
extern uint8 gGlobalBuf[MAX_DATA_SIZE];
int gRecvSize = 0;
static void ReceiveData(uint8 *pBuf, uint32 size, uint32 bootmode)
{
	uint32 i;
	
	if(bootmode == eBOOT_MODE_SPI){
		spi_slave_rx(EXT_BOOT_SPI_CH, pBuf, size);
	}else if(bootmode == eBOOT_MODE_UART){
		for(i=0;i<size;i++){
			pBuf[i] = uart_inbyte();
			gRecvSize = i;
			//dbg("inbyte[%d] 0x%x\n", i, pBuf[i]);
		}
	}
}

static uint8 ReceiveByte(uint32 bootmode)
{
	uint8 ret = 0;
	
	if(bootmode == eBOOT_MODE_SPI){
		ret = spi_slave_rx_byte(EXT_BOOT_SPI_CH);
	}else if(bootmode == eBOOT_MODE_UART){
		ret = uart_inbyte();
		//dbg("inbyte 0x%x\n", ret);
	}

	return ret;
}


static void send_response(sint32 resp, uint32 bootmode)
{
	uint32 *ptr;
	uint32 checksum_calc;
	sint32 i;

	memset(gBuf, 0, 16);
	gBuf[0] = EXT_BOOT_SOF;
	gBuf[1] = CMD_RESPONSE;
	gBuf[4] = 0x08;
	gBuf[8] = (resp&0xff);
	ptr = (unsigned int *)gBuf;

	checksum_calc = 0;
	for(i=0;i<3;i++){
		checksum_calc += *ptr;
		ptr++;
	}

	memcpy(&gBuf[12], &checksum_calc, 4);

	//for(i=0;i<16;i++)r_tx_data(buf[i]);

	// send 16 byte response packet
	if(bootmode == eBOOT_MODE_SPI){
		spi_wait(EXT_BOOT_SPI_CH);
		spi_slave_tx(EXT_BOOT_SPI_CH, gBuf, 16);
		
	}else if(bootmode == eBOOT_MODE_UART){
		for(i=0;i<16;i++){
			//dbg("send byte: %d\n", gBuf[i]);
			uart_outbyte(gBuf[i]);
		}
	}


}

uint32 external_boot(uint32 bootmode)
{

	uint32 i;
	uint32 *ptr;
	uint8 cmd;
	uint32 data_size;
	uint32 checksum, checksum_calc;
	sint32 response = 0;	// 0: no resp, 1: resp ok, 2: resp error
	uint32 jump_addr = (uint32)-1;

	gBuf = gGlobalBuf;

	memset(gBuf, 0, MAX_DATA_SIZE);

	if(bootmode == eBOOT_MODE_SPI){
		// SPI init
		spi_initialize(EXT_BOOT_SPI_CH, 1, 0, 0, 8, NULL);
		//dbg("spi init\n");
	}else if(bootmode == eBOOT_MODE_UART){
		// UART init - set to default
		//r_init_uart0(SERIAL_CLK, BAUD_RATE);
		uart_init(BAUD_RATE, 3 /*data 8bit*/, 0/*stop 1bit*/, 0/*no parity*/);
		//dbg("uart init\n");
	}else{
		// error
		return jump_addr;
	}

	while(1){
		//response = eRESP_OK;

		// Receive Header
		while((gBuf[0] = ReceiveByte(bootmode)) != EXT_BOOT_SOF);
		//dbg("SOF found\n");
		
		for(i=1;i<PACKET_HDR_SIZE;i++){
			gBuf[i] = ReceiveByte(bootmode);
		}

		
		
		cmd = gBuf[1];
		memcpy(&data_size, &gBuf[4], 4);

		if((data_size+PACKET_HDR_SIZE) >= MAX_DATA_SIZE){
			// Error handling
			response = eERROR_INVALID_SIZE;
			//dbg("invalid size\n");
			goto SEND_RESPONSE;
		}

		// Receive Data
		ReceiveData(&gBuf[PACKET_HDR_SIZE], data_size, bootmode);

		// Checksum check
		checksum_calc = 0;
		memcpy(&checksum, &gBuf[PACKET_HDR_SIZE + data_size - 4], 4);
		ptr = (uint32 *)gBuf;
		for(i=0;i<((PACKET_HDR_SIZE + data_size - 4)>>2);i++){
			checksum_calc += (*ptr);
			ptr++;
		}

		if(checksum_calc != checksum){
			response = eERROR_CHECKSUM_FAIL;
			//dbg("checksum fail\n");
			goto SEND_RESPONSE;
		}


		// CMD Action
		dbg("cmd: %d\n", cmd);
		if(cmd == CMD_BAUD_SET){
			int baud_rate;

			memcpy(&baud_rate, &gBuf[PACKET_HDR_SIZE], 4);
			// UART init
			uart_init(baud_rate, 3 /*data 8bit*/, 0/*stop 1bit*/, 0/*no parity*/);
			//dbg("baudrate set: %d\n", baud_rate);
			continue;	// no response
		}else if(cmd == CMD_READY_CHECK){
			response = eRESP_OK;
		}else if(cmd == CMD_REG_SET){
			int reg[2];
			vuint32 *addr;

			memcpy(reg, &gBuf[PACKET_HDR_SIZE], 8);
			addr = (vuint32 *)reg[0];
			*addr = (vuint32)reg[1];
			response = eRESP_OK;
		}else if(cmd == CMD_MEMORY_WRITE){

			unsigned int **addr;

			
			addr = (unsigned int **)&gBuf[PACKET_HDR_SIZE];

			//dbg("cmd mem write. addr:0x%x, size: %d\n", (unsigned int)*addr, data_size);
			memcpy(*addr, &gBuf[PACKET_HDR_SIZE+4], data_size - 4 - 4 );
			cache_wb_range(*addr,data_size - 4 - 4);
			response = eRESP_OK;
		}else if(cmd == CMD_PROG_JUMP){
			memcpy(&jump_addr, &gBuf[PACKET_HDR_SIZE], 4);
			response = eRESP_OK;
			send_response(response, bootmode);
			
			break;

		}else if(cmd == CMD_REQ_RESPONSE){
			dbg("Request Response!!\n");
		}else{	// unknown packet

			// Error handling
			response = eERROR_UNKNOWN_PACKET;
			goto SEND_RESPONSE;
		}



SEND_RESPONSE:
		send_response(response, bootmode);

	}
	
	return jump_addr;

}


