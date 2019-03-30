//----------------------------------------------------------------------------------------------------------------------
// (C) Copyright 2005  Macro Image Technology Co., LTd. , All rights reserved
// 
// This source code is the property of Macro Image Technology and is provided
// pursuant to a Software License Agreement. This code's reuse and distribution
// without Macro Image Technology's permission is strictly limited by the confidential
// information provisions of the Software License Agreement.
//-----------------------------------------------------------------------------------------------------------------------
//
// File Name   		:	I2C.C
// Description 		:
// Ref. Docment		: 
// Revision History 	:

// ----------------------------------------------------------------------
// Include files
// ----------------------------------------------------------------------
#include	"..\common\board.h"
#include <stdio.h>

// ----------------------------------------------------------------------
// Struct/Union Types and define
// ----------------------------------------------------------------------
#define	TIMEOUT_MAX    0x1000 /*<! The value of the maximal timeout for I2C waiting loops */

// ----------------------------------------------------------------------
// Static Global Data section variables
// ----------------------------------------------------------------------

// ----------------------------------------------------------------------
// External Variable 
// ----------------------------------------------------------------------
BOOL I2C_OldSpeed[I2C_no] = {LOW, LOW};

// ----------------------------------------------------------------------
// Static Prototype Functions
// ----------------------------------------------------------------------
BYTE I2C_ID;
// ----------------------------------------------------------------------
// Static functions
// ----------------------------------------------------------------------

static void I2C_SpeedReset(BOOL I2C_Speed)
{
	I2C_InitTypeDef I2C_InitStructure;

	if (I2C_Speed == I2C_OldSpeed[(I2C_ID == I2C_1)? I2C_1 : I2C_2]) return;
	I2C_OldSpeed[(I2C_ID == I2C_1)? I2C_1 : I2C_2] = I2C_Speed;

	I2C_Cmd((I2C_ID == I2C_1)? I2C1 : I2C2, DISABLE);
	I2C_DeInit((I2C_ID == I2C_1)? I2C1 : I2C2);

	// I2C configuration
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x00;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	if (I2C_Speed == LOW)	I2C_InitStructure.I2C_ClockSpeed = 100000;
	else			I2C_InitStructure.I2C_ClockSpeed = 400000;

	/* I2C1 Peripheral Enable */
	I2C_Cmd((I2C_ID == I2C_1)? I2C1 : I2C2, ENABLE);
	I2C_Init((I2C_ID == I2C_1)? I2C1 : I2C2, &I2C_InitStructure);
	
}

static BYTE I2C_Check(uint32_t I2C_Event)
{
	WORD TimeOut = TIMEOUT_MAX;
	BYTE return_flag = I2C_OK;

	while(!I2C_CheckEvent((I2C_ID == I2C_1)? I2C1 : I2C2, I2C_Event))
	{
		if((TimeOut--) == 0)
		{
			switch (I2C_Event)
			{
				case I2C_EVENT_MASTER_MODE_SELECT :
					printf("Error : I2C cannot start\n\r");
					return_flag = I2C_NOT_START;
					break;
					
				case I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED :
					printf("Error : I2C No Ack\n\r");
					return_flag = I2C_HOST_NACK;
					break;
					
				case I2C_EVENT_MASTER_BYTE_TRANSMITTED :
					printf("Error : I2C data transfer not finish\n\r");
					return_flag = I2C_HOST_NACK;
					break;
			}
			I2C_GenerateSTOP(I2C1, ENABLE);
		}
	}

	return return_flag;
}

static BYTE I2C_CheckBusy(void)
{
	uint32_t TimeOut = TIMEOUT_MAX;

	while (I2C_GetFlagStatus((I2C_ID == I2C_1)?I2C1 : I2C2, I2C_FLAG_BUSY)) 
	{if((TimeOut--) == 0) return I2C_TIME_OUT;}
	return I2C_OK;
}

static BYTE I2C_Start(void)
{
	I2C_GenerateSTART((I2C_ID == I2C_1)?I2C1 : I2C2, ENABLE);
	if (I2C_Check(I2C_EVENT_MASTER_MODE_SELECT)) return I2C_NOT_FREE;
	
	return I2C_OK;
}

static BYTE I2C_SendAddr(BYTE dAddr, BYTE mode)
{
	uint32_t TimeOut = TIMEOUT_MAX;
	// mode = 0 for transmitter, 1 for receiver

	if (mode == I2C_Direction_Transmitter)
	{
		I2C_Send7bitAddress((I2C_ID == I2C_1)?I2C1 : I2C2, dAddr, mode);
		if (I2C_Check(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)){
			printf("dAddr = %02X\n\r", dAddr);
			while (1);
			
//			return I2C_HOST_NACK;
		}
	}
	else
	{
		I2C_Send7bitAddress((I2C_ID == I2C_1)?I2C1 : I2C2, dAddr, I2C_Direction_Receiver);

		TimeOut = TIMEOUT_MAX;
		while(I2C_GetFlagStatus((I2C_ID == I2C_1)?I2C1 : I2C2, I2C_FLAG_ADDR) == RESET)
		{if((TimeOut--) == 0) return I2C_TIME_OUT;}
	}
	
	return I2C_OK;
}

static BYTE I2C_SendByte(BYTE Data, BOOL mode)
{
	if (mode == NACK) I2C_AcknowledgeConfig((I2C_ID == I2C_1)?I2C1 : I2C2, DISABLE);

	I2C_SendData((I2C_ID == I2C_1)?I2C1 : I2C2, Data);
	if (I2C_Check(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) return I2C_HOST_NACK;

	if (mode == NACK) I2C_AcknowledgeConfig((I2C_ID == I2C_1)?I2C1 : I2C2, ENABLE);
	return I2C_OK;
}

static BYTE I2C_GetByte(BOOL mode)
{
	BYTE read;

	if (mode == NACK) I2C_AcknowledgeConfig((I2C_ID == I2C_1)?I2C1 : I2C2, DISABLE);

	while(I2C_GetFlagStatus((I2C_ID == I2C_1)?I2C1 : I2C2, I2C_FLAG_RXNE) == RESET);
	read = I2C_ReceiveData((I2C_ID == I2C_1)?I2C1 : I2C2);

	if (mode == NACK) I2C_AcknowledgeConfig((I2C_ID == I2C_1)?I2C1 : I2C2, ENABLE);

	return read;
}

static BYTE I2C_Stop(void)
{
	I2C_GenerateSTOP((I2C_ID == I2C_1)?I2C1 : I2C2, ENABLE);
	return I2C_OK;
}

static void I2C_Clear(void)
{
	if (I2C_ID == I2C_1) {(void)I2C1->SR1;	(void)I2C1->SR2;}
	else {(void)I2C2->SR1;	(void)I2C2->SR2;}
}

//--------------------------------------------------------------------------------------------------------------------------
BYTE I2C_Write(BYTE ID, BOOL Speed, BYTE dAddr, BYTE rAddr, PBYTE pBuff, WORD bytes)
{
	WORD i;

	I2C_ID = ID;
	I2C_SpeedReset(Speed);
	if (I2C_CheckBusy()) return I2C_NOT_FREE;

	if (I2C_Start()) return I2C_NOT_START;
	if (I2C_SendAddr(dAddr, I2C_Direction_Transmitter)) return I2C_HOST_NACK;
	if (I2C_SendByte(rAddr, ACK)) return I2C_HOST_NACK;

	for (i=0; i<bytes-1; i++)
	{
		if (I2C_SendByte(pBuff[i], ACK)) return I2C_HOST_NACK;
	}

	if (I2C_SendByte(pBuff[i], NACK)) return I2C_HOST_NACK;
	
	I2C_Stop();
	I2C_Clear();

#if 0
	printf("WR(0x%02X, %d) : ", rAddr, bytes);
	for (i=0; i<bytes; i++) printf("%02X ", ((PBYTE)pBuff)[i]);
	printf("\n\r");
#endif	

	return I2C_OK;

}

//--------------------------------------------------------------------------------------------------------------------------
BYTE I2C_Read(BYTE ID, BOOL Speed, BYTE dAddr, BYTE rAddr, PBYTE pBuff, WORD bytes)
{
	WORD i;

	I2C_ID = ID;
	I2C_SpeedReset(Speed);
	if (I2C_CheckBusy()) return I2C_NOT_FREE;

	if (I2C_Start()) return I2C_NOT_START;
	if (I2C_SendAddr(dAddr, I2C_Direction_Transmitter)) return I2C_HOST_NACK;
	if (I2C_SendByte(rAddr, ACK)) return I2C_HOST_NACK;

	if (I2C_Start()) return I2C_NOT_START;
	if (I2C_SendAddr(dAddr, I2C_Direction_Receiver)) return I2C_HOST_NACK;

	__disable_irq();  I2C_Clear();	__enable_irq();  

	for (i=0; i<bytes-1; i++)
	{
		pBuff[i] = I2C_GetByte(ACK);
	}

	pBuff[i] = I2C_GetByte(NACK);
	I2C_Stop();
	I2C_Clear();
#if 0
	printf("RD(0x%02X, %d) : ", rAddr, bytes);
	for (i=0; i<bytes; i++) printf("%02X ", ((PBYTE)pBuff)[i]);
	printf("\n\r");
#endif	
	return I2C_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
BYTE I2C_Write16(BYTE ID, BOOL Speed, BYTE dAddr, WORD rAddr, PBYTE pBuff, WORD bytes)
{
	WORD i;

	I2C_ID = ID;
	I2C_SpeedReset(Speed);
	I2C_CheckBusy();

	if (I2C_Start()) return I2C_NOT_START;
	if (I2C_SendAddr(dAddr, I2C_Direction_Transmitter)) return I2C_HOST_NACK;
	if (I2C_SendByte(HIBYTE(rAddr), ACK)) return I2C_HOST_NACK;
	if (I2C_SendByte(LOBYTE(rAddr), ACK)) return I2C_HOST_NACK;

	for (i=0; i<bytes-1; i++)
	{
		if (I2C_SendByte(pBuff[i^1], ACK)) return I2C_HOST_NACK;
	}

	if (I2C_SendByte(pBuff[i^1], NACK)) return I2C_HOST_NACK;
	I2C_Stop();
	I2C_Clear();
#if 0
	printf("WR16(0x%04X, %d) : ", rAddr, bytes);
	for (i=0; i<bytes; i++) printf("%02X ", ((PBYTE)pBuff)[i]);
	printf("\n\r");
#endif	

	return I2C_OK;
}

//--------------------------------------------------------------------------------------------------------------------------
BYTE I2C_Read16(BYTE ID, BOOL Speed, BYTE dAddr, WORD rAddr, PBYTE pBuff, WORD bytes)
{
	WORD i;

	I2C_ID = ID;
	I2C_SpeedReset(Speed);
	I2C_CheckBusy();

	if (I2C_Start()) return I2C_NOT_START;
	if (I2C_SendAddr(dAddr, I2C_Direction_Transmitter)) return I2C_HOST_NACK;
	if (I2C_SendByte(HIBYTE(rAddr), ACK)) return I2C_HOST_NACK;
	if (I2C_SendByte(LOBYTE(rAddr), ACK)) return I2C_HOST_NACK;

	if (I2C_Start()) return I2C_NOT_START;
	if (I2C_SendAddr(dAddr, I2C_Direction_Receiver)) return I2C_HOST_NACK;

	__disable_irq();  I2C_Clear();	__enable_irq();  

	for (i=0; i<bytes-1; i++)
	{
		pBuff[i^0x01]  = I2C_GetByte(ACK);		// Receive a buffer data
	}
		
	pBuff[i^0x01]  = I2C_GetByte(NACK);		// Receive a buffer data
	I2C_Stop();
	I2C_Clear();
#if 0
	printf("RD16(0x%04X, %d) : ", rAddr, bytes);
	for (i=0; i<bytes; i++) printf("%02X ", ((PBYTE)pBuff)[i]);
	printf("\n\r");
#endif	
	return I2C_OK;
}


/*  FILE_END_HERE */
