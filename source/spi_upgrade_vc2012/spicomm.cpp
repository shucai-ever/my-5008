#include "stdafx.h"
#include "spicomm.h"
#include "spimaster.h"

//#define MAX_PACKET_SIZE		0x400	// MAX 1KB
#define MAX_PACKET_SIZE		0x40000	// 
#define PACKET_HDR_SIZE		8
#define SPI_DUMMY_SIZE		2
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
#define CMD_FLASH_SEND 		0x15

#define CMD_UPGRADE_START	0x20
#define CMD_UPGRADE_DATA	0x21
#define CMD_UPGRADE_DONE	0x22
#define CMD_UPGRADE_READY 	0x23

#define CMD_REMOCON_CONTROL	0x30
//=================================

static void *gphandle = NULL;

static DWORD CalcCheckSum(BYTE *pBuf, DWORD Size)
{
	DWORD *ptr = (DWORD *)pBuf;
	DWORD CheckSum = 0;
	DWORD i;

	Size /= 4;

	for(i=0;i<Size;i++){
		CheckSum += *ptr;
		ptr++;
	}

	return CheckSum;
}

static int SPICOMM_Write(void * handle, unsigned char *data, unsigned int size)
{
	int ret;
	int write_size;

	if(size > 8){
		if((ret = SPI_Write(handle, data, 8+2)) <= 0){
			return ret;
		}

		write_size = ret;

		if((ret = SPI_Write(handle, (BYTE *)(data+8), size-8)) <= 0){
			return ret;
		}
		write_size += ret;
		write_size -= 2;

	}else{
		if((ret = SPI_Write(handle, data, size)) <= 0){
			return ret;
		}

		write_size = ret;
	}

	return write_size;
}

static int SPICOMM_Read(void * handle, unsigned char *data, unsigned int size)
{
	int ret;
	
	ret = SPI_Read(handle, data, size);

	return ret;
}

/*
*	returns -1 on failure
*/

int SPICOMM_Initialize(SPI_CLOCK_RATE SPIClk)
{
	int ret = 0;

	gphandle = SPI_Init(SPIClk);

	TRACE("spicomm init(0x%x)\n", gphandle);

	if(gphandle == NULL)ret = -1;

	return ret;
}

void SPICOMM_deInitialize(void)
{
	TRACE("spicomm close(0x%x)\n", gphandle);
	SPI_deInit(gphandle);
	gphandle = NULL;
}

/*
*	returns -1 on failure
*/

/*
*	returns 0 on success, response code otherwise(refer to enum RESP_CODE)
*/
int SPICOMM_ReceiveResponse(DWORD TimeOutMSEC)
{
	BYTE buf[MAX_PACKET_SIZE];
	BYTE resp_buf[MAX_PACKET_SIZE];
	DWORD data_size;
	DWORD check_sum;
	DWORD stick;
	int ret;
	int i;
	int need_sof;
	int recv_size;
	int size;
	int retry = 0;

	need_sof = 1;
	size = 16 + 2;
	recv_size = 0;

	//TRACE("resp wait - %d\n", GetTickCount());
	stick = GetTickCount();
	do{
		retry = 0;
		do{
			//Sleep(1);
			if(SPICOMM_Read(gphandle, buf, size) <= 0){
				ret = ERROR_FAILURE;
				goto END_FT;
			}
		
			if(need_sof){
				for(i=0;i<size;i++){
					if(buf[i] == EXT_BOOT_SOF){
						memcpy(&resp_buf[recv_size], &buf[i], size - i );
						need_sof = 0;
						recv_size = size - i;
						//TRACE("\n", buf[i]);
						break;
					}else{
						//TRACE("0x%x ", buf[i]);
					}
				}
			}else{
				memcpy(&resp_buf[recv_size], buf, size);
				recv_size += size;
			}
		
			// need timeout
			if(TimeOutMSEC != INFINITE){
				if(GetTickCount() - stick > TimeOutMSEC){
					ret = ERROR_TIMEOUT_RESP;
					goto END_FT;
				}
			}
		}while(need_sof && recv_size < 16);

		memcpy(&data_size, &resp_buf[4], 4);
		if(data_size == 8){
			check_sum = CalcCheckSum(resp_buf,PACKET_HDR_SIZE + data_size - 4);

			//TRACE("checksum calc = %02x, resp_buf = %02x\n", check_sum, resp_buf[PACKET_HDR_SIZE + data_size - 4]);
			if(memcmp(&check_sum, &resp_buf[PACKET_HDR_SIZE + data_size - 4], 4) != 0){
				//Checksum fail retry.
				retry = 1;
			}
		}else{
			// invalid size retry.
			retry = 1;
		}

		if(retry){
			SPICOMM_ReqResponse();
			recv_size = 0;
			need_sof = 1;
			Sleep(1);
		}
	}while(retry);
	memcpy(&ret, &resp_buf[8], 4);

	//TRACE("resp done - %d\n", GetTickCount());
END_FT:
	return ret;

}

int SPICOMM_UpgradeStart(DWORD write_size, BYTE Upgrade_Section, BYTE Verify, BYTE CurrCnt, BYTE TotalCnt, DWORD Version)
{
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;
	DWORD send_size = 32 * 1024;

	data_size = 4 + 1 + 1 + 1 + 1 + 4 + 4 + 4; // write size(4) + upgrade section(1) + verify(1) + curr cnt(1) + total cnt(1) + version(4) + check sum(4) + send_size(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_UPGRADE_START;
	memcpy(&ptr[4], &data_size, 4);
	memcpy(&ptr[8], &write_size, 4);
	ptr[12] = Upgrade_Section;
	ptr[13] = Verify;
	ptr[14] = CurrCnt;
	ptr[15] = TotalCnt;
	memcpy(&ptr[16], &Version, 4);
	memcpy(&ptr[20], &send_size, 4);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4); // except checksum size

	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if( SPICOMM_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0 ){
		ret = -1;
	}
	
	return ret;
}

int SPICOMM_UpgradeDone(void)
{
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;

	data_size = 4;	// check sum(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_UPGRADE_DONE;
	memcpy(&ptr[4], &data_size, 4);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4);	// except checksum size
	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if(SPICOMM_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0){
		ret = -1;
	}

	return ret;
}

int SPICOMM_UpgradeDataSend(BYTE *pBuf, DWORD DataSize)
{
#if 0
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;
	
	data_size = DataSize + 4;	// data(DataSize) + check sum(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_UPGRADE_DATA;
	memcpy(&ptr[4], &data_size, 4);
	memcpy(&ptr[8], pBuf, DataSize);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4);	// except checksum size
	
	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if(SPICOMM_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0){
		ret = -1;
	}
	return ret;
#else
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;
	
	data_size = DataSize + 4;
	memset( buf, 0 , MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];
	memcpy(&ptr[0], pBuf, DataSize);

	check_sum = CalcCheckSum(ptr, data_size - 4);	// except checksum size

	memcpy(&ptr[data_size - 4], &check_sum, 4);

	if(SPI_Write(gphandle, buf, SPI_DUMMY_SIZE + data_size) <= 0){
		ret = -1;
	}
	//TRACE("prog done - %d\n", GetTickCount());
	return ret;
#endif
}

int SPICOMM_ReadyCheck(void)
{
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;

	data_size = 4;	// check sum(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_READY_CHECK;
	memcpy(&ptr[4], &data_size, 4);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4);	// except checksum size

	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if(SPICOMM_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0){
		ret = -1;
	}

	return ret;
}

int SPICOMM_ReqResponse(void)
{
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;

	data_size = 4;	// check sum(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_REQ_RESPONSE;
	memcpy(&ptr[4], &data_size, 4);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4);	// except checksum size

	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if(SPICOMM_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0){
		ret = -1;
	}

	return ret;
}


static void make_pattern(unsigned int *pMem, unsigned int size, unsigned int initval)
{
	unsigned int i;

	for(i=0;i<size/4;i++){
		*(unsigned int *)(pMem+i) = (initval + i*4);
	}

}

void SPICOMM_SendTest(void)
{
	unsigned char *pBuf;
	unsigned int size = 32*1024;

	pBuf = (unsigned char *)malloc(size + 4);

	make_pattern((unsigned int *)&pBuf[4], size, 0);

	if(SPICOMM_Write(gphandle, &pBuf[2], size+2) <= 0){
		TRACE("Error\n");
	}

	free(pBuf);
}

int SPICOMM_UpgradeReady(void)
{
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;

	data_size = 4;	// check sum(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_UPGRADE_READY;
	memcpy(&ptr[4], &data_size, 4);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4);	// except checksum size

	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if(SPICOMM_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0){
		ret = -1;
	}

	return ret;
}

WORD SPICOMM_CheckStatus(DWORD TimeOutMSEC)
{
	WORD ret;
	BYTE write_buf[MAX_PACKET_SIZE];
	BYTE resp_buf[MAX_PACKET_SIZE];
	int read_size;
	int write_size;
	DWORD stick;

	write_buf[0] = 0x05;

	write_size = 6;

	stick = GetTickCount();
	while(1)
	{
		read_size = SPI_ReadWrite(gphandle, resp_buf, write_buf, write_size);

		if( read_size == 6 )
		{
			if( ((resp_buf[3] >> 0) & 1 ) == 1 )
			{
				ret = resp_buf[4] << 8;
				ret += resp_buf[5];
				break;
			}
		}

		if(TimeOutMSEC != INFINITE){
			if(GetTickCount() - stick > TimeOutMSEC){
			ret = ERROR_TIMEOUT_RESP;
			goto END_FT;
			}
		}
	}
END_FT:
	
	return ret;
}

int SPICOMM_FlashSendReady(void)
{
	TRACE("SPICOMM_FlashSend!!!!\n");
	int ret = 0;
	BYTE buf[MAX_PACKET_SIZE];
	BYTE *ptr;
	DWORD data_size;
	DWORD check_sum;
	DWORD send_size = 32 * 1024;

	data_size = 8;	// check sum(4)
	memset(buf, 0, MAX_PACKET_SIZE);

	ptr = &buf[SPI_DUMMY_SIZE];

	ptr[0] = EXT_BOOT_SOF;
	ptr[1] = CMD_FLASH_SEND;
	memcpy(&ptr[4], &data_size, 4);
	memcpy(&ptr[8], &send_size, 4);

	check_sum = CalcCheckSum(ptr, PACKET_HDR_SIZE + data_size - 4);	// except checksum size
	//TRACE("check_sum = %02x\n", check_sum);
	memcpy(&ptr[PACKET_HDR_SIZE + data_size - 4], &check_sum, 4);

	if(SPI_Write(gphandle, buf, SPI_DUMMY_SIZE + PACKET_HDR_SIZE + data_size) <= 0){
		ret = -1;
	}

	return ret;
}
