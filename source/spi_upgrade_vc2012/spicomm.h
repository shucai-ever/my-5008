#ifndef _SPI_PROTOCOL_H_
#define _SPI_PROTOCOL_H_

#include "afxdialogex.h"

#define MAX_DATA_SIZE	512
#define TIMEOUT_MSEC	9000

enum RESP_CODE{
	RESP_OK = 0,
	RESP_ERROR_INVALID_HDR,
	RESP_ERROR_INVALID_SIZE,
	RESP_ERROR_CHECKSUM_FAIL,
	RESP_ERROR_UNKNOWN_PACKET,
	RESP_ERROR_FLASH_INIT_FAIL,
	RESP_ERROR_FLASH_WRITE_FAIL,
	eERROR_UPGRADE_REJECT,
	eERROR_UPGRADE_FAIL,

	ERROR_WRONG_RESP,
	ERROR_TIMEOUT_RESP,
	ERROR_FAILURE,

	NEED_MORE_DATA,	
};

typedef enum
{
	CLOCK_RATE_12M = 0,
	CLOCK_RATE_10M,
	CLOCK_RATE_7_5M,
	CLOCK_RATE_6M,
	CLOCK_RATE_5M,
	CLOCK_RATE_3_75M,
	CLOCK_RATE_3M,
	CLOCK_RATE_1_5M,
	CLOCK_RATE_750K,
	CLOCK_RATE_187K,
	CLOCK_RATE_46K,
	CLOCK_RATE_NUMBER,
}SPI_CLOCK_RATE;

int SPICOMM_Initialize(SPI_CLOCK_RATE SPIClk);
void SPICOMM_deInitialize(void);
int SPICOMM_ReceiveResponse(DWORD TimeOutMSEC);
int SPICOMM_UpgradeStart(DWORD write_size, BYTE Upgrade_Section, BYTE Verify, BYTE CurrCnt, BYTE TotalCnt, DWORD Version);
int SPICOMM_UpgradeDone(void);
int SPICOMM_UpgradeDataSend(BYTE *pBuf, DWORD DataSize);
int SPICOMM_ReadyCheck(void);
int SPICOMM_ReqResponse(void);

void SPICOMM_SendTest(void);

int SPICOMM_UpgradeReady(void);

WORD SPICOMM_CheckStatus(DWORD TimeOutMSEC);
int SPICOMM_FlashSendReady(void);

#endif