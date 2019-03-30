//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2010  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	MDINI2C.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------

#include "..\common\board.h"
#include "..\common\common.h"
#include <stdio.h>
#include "stm32f10x.h"
#include "..\Common\i2c.h"

#include "pi5008k_test.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
volatile int spinLockI2C = 0;

#ifdef PIXELPLUS_PR1000
// only for make register table.
//#undef SET_MANUAL_I2C_DATA_DEBUG //DEBUGGING for table loading.
#define PRINT_I2C_DATA  //also set pr1000_driver/pr1000_func.c 
#ifdef PRINT_I2C_DATA
int gPrintI2C = 0;
static unsigned char curPage = 0xFF;
#endif // PRINT_I2C_DATA
extern BYTE I2C_Write(BYTE ID, BOOL Speed, BYTE dAddr, BYTE rAddr, PBYTE pBuff, WORD bytes);
extern BYTE I2C_Read(BYTE ID, BOOL Speed, BYTE dAddr, BYTE rAddr, PBYTE pBuff, WORD bytes);

int PR1000_SetPage(const int fd, unsigned char slave, unsigned char page)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;
	ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);
	spinLockI2C = 0;

#ifdef PRINT_I2C_DATA
	if(gPrintI2C)
	{
		if(curPage != page)
		{
			curPage = page;
			//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, 0xFF, page);
			printf("{0x%02x, 0x%02x},\n\r", 0xFF, page);
		}
		else
		{
			//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, 0xFF, page);
		}
	}
#endif // PRINT_I2C_DATA

        return(ret);
}
int PR1000_Read(const int fd, unsigned char slave, unsigned char reg, unsigned char *pRet)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;
	ret = I2C_Read(I2C_1, HIGH, (slave), reg, pRet, 1);
	spinLockI2C = 0;

        return(ret);
}
int PR1000_Write(const int fd, unsigned char slave, unsigned char reg, unsigned char value)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;
	ret = I2C_Write(I2C_1, HIGH, (slave), reg, (PBYTE)&value, 1);
	spinLockI2C = 0;

#ifdef PRINT_I2C_DATA
	if(gPrintI2C)
	{
		if(reg == 0xFF)
		{
			if(curPage != value)
			{
				curPage = value;
				//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, reg, value);
				printf("{0x%02x, 0x%02x},\n\r", reg, value);
			}
			
		}
		else
		{
			//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, reg, value);
			printf("{0x%02x, 0x%02x},\n\r", reg, value);
		}
	}
#endif // PRINT_I2C_DATA


        return(ret);
}
int PR1000_PageRead(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char *pRet)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;

	if(page>=0) ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);
	ret = I2C_Read(I2C_1, HIGH, (slave), reg, pRet, 1);

	spinLockI2C = 0;
        return(ret);
}
int PR1000_PageWrite(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char value)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;

	if(page>=0) ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);
	ret = I2C_Write(I2C_1, HIGH, (slave), reg, &value, 1);

	spinLockI2C = 0;

#ifdef PRINT_I2C_DATA
	if(gPrintI2C)
	{
		if(curPage != (BYTE)page)
		{
			curPage = (BYTE)page;
			//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, 0xFF, (BYTE)page);
			printf("{0x%02x, 0x%02x}, \n\r", 0xFF, (BYTE)page);
		}
		//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, reg, value);
		printf("{0x%02x, 0x%02x},\n\r", reg, value);
	}
#endif // PRINT_I2C_DATA

        return(ret);
}
int PR1000_PageReadBurst(const int fd, unsigned char slave, int page, unsigned char reg, unsigned short length, unsigned char *pRetData)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;

	if(page>=0) ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);
	ret = I2C_Read(I2C_1, HIGH, (slave), reg, pRetData, length);

	spinLockI2C = 0;
        return(ret);
}
int PR1000_PageWriteBurst(const int fd, unsigned char slave, int page, unsigned char reg, unsigned short length, const unsigned char *pData)
{
        int ret = 0; 

	while(spinLockI2C==1); spinLockI2C = 1;

	if(page>=0) ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);
	ret = I2C_Write(I2C_1, HIGH, (slave), reg, (PBYTE)pData, length);

	spinLockI2C = 0;

#ifdef PRINT_I2C_DATA
	if(gPrintI2C)
	{
		int i;
		if(curPage != (BYTE)page)
		{
			curPage = (BYTE)page;
			printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, 0xFF, (BYTE)page);
		}
		printf("WrS S:0x%02x R:0x%02x ", slave, reg);
		for(i = 0; i < length; i++)
		{
			printf("D:0x%02x ", pData[i]);
		}
		printf("\n\r");
	}
#endif // PRINT_I2C_DATA


        return(ret);
}
int PR1000_ReadMaskBit(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char regMaskBit, unsigned char *pRet)
{
        int ret = 0; 
	uint8_t data = 0;

	while(spinLockI2C==1); spinLockI2C = 1;

	if(page>=0) ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);

	ret = I2C_Read(I2C_1, HIGH, (slave), reg, &data, 1); 
	*pRet = data & regMaskBit;

	spinLockI2C = 0;
        return(ret);
}
int PR1000_WriteMaskBit(const int fd, unsigned char slave, int page, unsigned char reg, unsigned char regMaskBit, unsigned char value)
{
        int ret = 0; 
	uint8_t data = 0;

	while(spinLockI2C==1); spinLockI2C = 1;

	if(page>=0) ret = I2C_Write(I2C_1, HIGH, (slave), 0xFF, (PBYTE)&page, 1);

	ret = I2C_Read(I2C_1, HIGH, (slave), reg, &data, 1); 
	data &= ~(regMaskBit);
	data |= (value & regMaskBit);
	ret = I2C_Write(I2C_1, HIGH, (slave), reg, &data, 1);

	spinLockI2C = 0;

#ifdef PRINT_I2C_DATA
	if(gPrintI2C)
	{
		if(curPage != (BYTE)page)
		{
			curPage = (BYTE)page;
			//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, 0xFF, (BYTE)page);
			printf("{0x%02x, 0x%02x}, \n\r", 0xFF, (BYTE)page);
		}
		//printf("WrS S:0x%02x R:0x%02x D:0x%02x\n\r", slave, reg, data);
		printf("{0x%02x, 0x%02x},\n\r", reg, data);
	}
#endif // PRINT_I2C_DATA



        return(ret);
}
#endif
// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------
	
// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------
// Drive Function for delay (usec and msec)
// You must make functions which is defined below.
//--------------------------------------------------------------------------------------------------------------------------
void U10secDelay(int delay)
{
	Delay_10us(delay);
}

//--------------------------------------------------------------------------------------------------------------------------
void MDelay(int delay)
{
	Delay_ms(delay);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// PI5008K Pin Mapping
// PA5 -> PR1000K RESET pin assign
// PB0 -> PR1000K IRQ pin assign

// PB6 -> PR1000K SCL pin assign
// PB7 -> PR1000K SDA pin assign

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void PI5008K_Reset_Con(void)
{	
	PI5008K_POWER_RESET(LOW);	
	MDelay(670); 

	PI5008K_POWER_RESET(HIGH);
	MDelay(670);	
}

