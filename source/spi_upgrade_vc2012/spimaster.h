#ifndef _SPI_MASTER_H_
#define _SPI_MASTER_H_

#include "spicomm.h"

void* SPI_Init(SPI_CLOCK_RATE SPIClk /*Hz*/);
void SPI_deInit(void * handle);
int SPI_Read(void * handle, unsigned char *data, unsigned int size);
int SPI_Write(void * handle, unsigned char *data, unsigned int size);
int SPI_ReadWrite(void * handle, unsigned char *rdata, unsigned char *wdata, unsigned int wsize);
#endif