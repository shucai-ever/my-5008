#include "stdafx.h"
#include "spimaster.h"
#include "spicomm.h"
#include "ftd2xx.h"
#include "LibFT4222.h"

/** ****************
Data Definitions
*** **************** */     

typedef struct
{
	FT4222_ClockRate clk_rate;
	FT4222_SPIClock  spi_clk;
} SPI_CLOCK_TABLE;

SPI_CLOCK_TABLE g_spi_clock_table[] = {
	{ SYS_CLK_48, CLK_DIV_4 }, // SPI CLOCK 12MHz
	{ SYS_CLK_80, CLK_DIV_8 }, // SPI CLOCK 10MHz
	{ SYS_CLK_60, CLK_DIV_8 }, // SPI CLOCK 7.5MHz
	{ SYS_CLK_48, CLK_DIV_8 }, // SPI CLOCK 6MHz
	{ SYS_CLK_80, CLK_DIV_16 }, // SPI CLOCK 5MHz
	{ SYS_CLK_60, CLK_DIV_16 }, // SPI CLOCK 3.75MHz
	{ SYS_CLK_48, CLK_DIV_16 }, // SPI CLOCK 3MHz
	{ SYS_CLK_24, CLK_DIV_16 }, // SPI CLOCK 1.5MHz
	{ SYS_CLK_24, CLK_DIV_32 }, // SPI CLOCK 750KHz
	{ SYS_CLK_24, CLK_DIV_128 }, // SPI CLOCK 187KHz

};


void* SPI_Init(SPI_CLOCK_RATE SPIClk /*Hz*/)
{
	FT_HANDLE handle = NULL;
	FT_STATUS ftStatus;

	//FT4222_ClockRate clk;

	ftStatus = FT_Open(0, &handle);
    if (FT_OK != ftStatus)
    {
        TRACE("Open a FT4222 device failed!\n");
		goto ERROR_RET;   
    }
	
	ftStatus = FT4222_SetClock( handle, g_spi_clock_table[SPIClk].clk_rate );

	if (FT_OK != ftStatus)
    {
        TRACE("Open a FT4222 device failed!\n");
		goto ERROR_RET;   
    }

/*
	ftStatus = FT4222_GetClock( handle, &clk );
	if (FT_OK != ftStatus)
    {
        TRACE("Open a FT4222 device failed!\n");
		goto ERROR_RET;   
    }
	else if(FT_OK == ftStatus )
	{
		TRACE("Clock Rate = %d\n", clk);
	}
*/

	ftStatus = FT4222_SPIMaster_Init(handle, SPI_IO_SINGLE, g_spi_clock_table[SPIClk].spi_clk, CLK_IDLE_LOW, CLK_LEADING, 0x01);

/*
	TRACE("SPIClk = %d\n", SPIClk);
	TRACE("g_spi_clock_table[SPIClk].clk_rate = %d\n", g_spi_clock_table[SPIClk].clk_rate);
	TRACE("g_spi_clock_table[SPIClk].spi_clk = %d\n", g_spi_clock_table[SPIClk].spi_clk);
*/

    if (FT_OK != ftStatus)
    {
        TRACE("Init FT4222 as SPI master device failed!\n");
		goto ERROR_RET;    
	}

	return (void *)handle;

ERROR_RET:

	if(handle){
	    FT4222_UnInitialize(handle);
		FT_Close(handle);
	}

	return (void *)NULL;

}

void SPI_deInit(void * handle)
{
	if(handle){
	    FT4222_UnInitialize(handle);
		FT_Close(handle);
	}
}

int SPI_Read(void * handle, unsigned char *data, unsigned int size)
{
	FT4222_STATUS rStatus;
	uint16		  sizeOfRead;
	int			  ret;

	if(handle == NULL || data == NULL || size == 0)return 0;

	if((rStatus = FT4222_SPIMaster_SingleRead (handle, data, size, &sizeOfRead, true)) != FT4222_OK){
		TRACE("SPI read fail. ERROR: %d\n", rStatus);
		ret = -1;
		goto END_FT;
	}

	ret = sizeOfRead;

END_FT:
	return ret;
}

int SPI_Write(void * handle, unsigned char *data, unsigned int size)
{
	FT4222_STATUS rStatus;
	uint16		  sizeTransferred;
	int			  ret;

	if(handle == NULL || data == NULL || size == 0)return 0;

	if((rStatus = FT4222_SPIMaster_SingleWrite (handle, data, size, &sizeTransferred, true)) != FT4222_OK){
		TRACE("SPI write fail. ERROR: %d\n", rStatus);
		ret = -1;
		goto END_FT;
	}

	ret = sizeTransferred;

END_FT:
	return ret;
}

int SPI_ReadWrite(void * handle, unsigned char *rdata, unsigned char *wdata, unsigned int wsize)
{
	FT4222_STATUS rStatus;
	uint16		  sizeOfRead;
	int			  ret;

	if(handle == NULL || rdata == NULL || wsize == 0 || wdata == NULL)return 0;

	if((rStatus = FT4222_SPIMaster_SingleReadWrite (handle, rdata, wdata, wsize, &sizeOfRead, true)) != FT4222_OK){
		TRACE("SPI read fail. ERROR: %d\n", rStatus);
		ret = -1;
		goto END_FT;
	}

	ret = sizeOfRead;

END_FT:
	return ret;
}