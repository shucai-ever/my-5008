#include <stdio.h>
#include <string.h>
#include "spi.h"
#include "debug.h"

#define SPI_TCR_WR           	(0x0 << 24)
#define SPI_TCR_WONLY          	(0x1 << 24)
#define SPI_TCR_RONLY          	(0x2 << 24)

#define MAX_SPI_DEV_NUM				5

typedef struct tagGPIO_PIN_T
{
	uint32 group;
	uint32 ch;
}GPIO_PIN_T;

typedef struct tagPINMUX_T
{
	uint32 reg;
	uint32 offset;
	uint32 val;
}PINMUX_T;



static SPI_REG_T *gpSPI[] = {
	SPI0_CTRL_REG,
	SPI1_CTRL_REG,
	SPI2_CTRL_REG,
	SPI_FLASH_CTRL_REG,
};

static PINMUX_T gCS_PinMux[] = {
	{MISC_BASE_ADDR + 0x5C, 20, 1},	// group 1, shift 20, val 1
	{MISC_BASE_ADDR + 0x5C, 26, 1},	// group 1, shift 26, val 1
	{MISC_BASE_ADDR + 0x5C, 22, 1},	// group 1, shift 22, val 1		
	{MISC_BASE_ADDR + 0x60, 2, 1},	// group 2, shift 2, val 1	
};


static GPIO_PIN_T gCS_GPIO[] = {
	{0, 26},
	{0, 29},
	{0, 27},
	{1, 1}
};



sint32 spi_initialize(sint32 ch, uint32 slave, uint32 freq, uint32 clk_mode, uint32 wordlen, SPI_CALLBACK callback)
{
	sint32 ret = 0;
	uint32 div;

	gpSPI[ch]->ctrl = 7;	// Tx/Rx FIFO reset, SPI reset
	while(gpSPI[ch]->ctrl & 7);
	
	gpSPI[ch]->trans_fmt = ( (((wordlen-1)&0x1f) << 8) | ((slave & 1)<<2) | (clk_mode&3) );

    gpSPI[ch]->ctrl = ( (2 << 16) |	(2 << 8) );	// Tx/Rx FIFO Threshold

	if(!slave){
		div = (PCLK / (freq * 2)) - 1;
		//gpSPI[ch]->timing = utilPutBits(gpSPI[ch]->timing, 0, 8, div);
		gpSPI[ch]->timing &= (~0xff);
		gpSPI[ch]->timing |= (div & 0xff);
	}else{
		gpSPI[ch]->trans_ctrl = SPI_TCR_WR;
	}
	
	return ret;
}


sint32 spi_tx(sint32 ch, const uint8 *dout, uint32 size)
{
	sint32 ret = 0;

	sint32 len_tx;
	sint32 n_bytes;
	uint32 data;
	
	n_bytes = (((gpSPI[ch]->trans_fmt>>8)&0x1f)+ 1 + 7 ) / 8;
	gpSPI[ch]->trans_ctrl = (SPI_TCR_WONLY | (((size-1)&0x1ff)<<12));
	gpSPI[ch]->cmd = 0;	// start transfer
	
	for(len_tx=0; len_tx < size;){
		if(len_tx < size && !((gpSPI[ch]->status>>23)&1) ){	// Tx not full
			memcpy(&data, dout, n_bytes);
			gpSPI[ch]->data = data;
			dout += n_bytes;
			len_tx++;
		}
	}

	while(gpSPI[ch]->status & 1);

	return ret;

}

sint32 spi_rx(sint32 ch, uint8 *din, uint32 size)
{
	sint32 ret = 0;

	sint32 len_rx;
	vuint32 val;
	sint32 n_bytes;
	uint32 data;

	n_bytes = (((gpSPI[ch]->trans_fmt>>8)&0x1f)+ 1 + 7 ) / 8;
	gpSPI[ch]->trans_ctrl = (SPI_TCR_RONLY | (((size-1)&0x1ff)<<0));

	gpSPI[ch]->cmd = 0;	// start transfer

	for(len_rx=0; len_rx < size;){
		if(!((gpSPI[ch]->status>>14)&1)){	// Rx not empty
			val = gpSPI[ch]->data;
			data = (uint8)val;
			memcpy(din, &data, n_bytes);
			din += n_bytes;
			len_rx++;
		}

	}

	while(gpSPI[ch]->status & 1);

	return ret;
}


sint32 spi_xfer(sint32 ch, const uint8 *dout, uint8 *din, uint32 size)
{
	sint32 ret = 0;

	sint32 len_tx, len_rx;
	vuint32 val;
	sint32 n_bytes;
	uint32 data;
	
	n_bytes = (((gpSPI[ch]->trans_fmt>>8)&0x1f)+ 1 + 7 ) / 8;
	gpSPI[ch]->trans_ctrl = (SPI_TCR_WR | (((size-1)&0x1ff)<<12) | (((size-1)&0x1ff)<<0));

	gpSPI[ch]->cmd = 0;	// start transfer
	
	for(len_tx=0, len_rx=0; len_rx < size;){
		if(len_tx < size && !((gpSPI[ch]->status>>23)&1)){	// Tx not full
			if(dout){
				memcpy(&data, dout, n_bytes);
				gpSPI[ch]->data = data;
				dout += n_bytes;
			}else{
				gpSPI[ch]->data = 0;
			}
			len_tx++;
		}

		if(!((gpSPI[ch]->status>>14)&1)){	// Rx not empty
			val = gpSPI[ch]->data;
			if (din){
				data = (uint8)val;
				memcpy(din, &data, n_bytes);
				din += n_bytes;
			}
			len_rx++;
		}

	}

	while(gpSPI[ch]->status & 1);

	return ret;
}


// support only 8 bit
sint32 spi_slave_tx(sint32 ch, const uint8 *dout, uint32 size)
{
	sint32 ret = 0;

	sint32 len_tx;
	
	for(len_tx=0; len_tx < size;){
		if(len_tx < size && !((gpSPI[ch]->status>>23)&1) ){	// Tx not full
			gpSPI[ch]->data = *dout;
			dout++;
			len_tx++;
		}
	}

	return ret;

}
void spi_slave_tx_byte(sint32 ch, const uint8 dout)
{
	while((gpSPI[ch]->status>>23)&1);	// tx full
	gpSPI[ch]->data = dout;

}

sint32 spi_slave_rx(sint32 ch, uint8 *din, uint32 size)
{
	sint32 ret = 0;
	sint32 len_rx;

	for(len_rx=0; len_rx < size;){
		if(!((gpSPI[ch]->status>>14)&1)){	// Rx not empty
			*din = gpSPI[ch]->data;
			din ++;
			len_rx++;
		}

	}

	return ret;
}

uint8 spi_slave_rx_byte(sint32 ch)
{
	uint8 ret;
	
	while((gpSPI[ch]->status>>14)&1);	// rx empty

	ret = gpSPI[ch]->data;
	
	return ret;
}


sint32 spi_direct_cs_enable(sint32 ch, uint8 en)
{
	if(gCS_GPIO[ch].ch == -1)return -1;
	
	if(en){
		*(vuint32 *)(GPIO0_BASE_ADDR + 0x1000*gCS_GPIO[ch].group + 0x24) |= (1<<gCS_GPIO[ch].ch);	// gpio output value  set
		*(vuint32 *)(GPIO0_BASE_ADDR + 0x1000*gCS_GPIO[ch].group + 0x28) |= (1<<gCS_GPIO[ch].ch);	// gpio dir out 
		
		//*(vuint32 *)gCS_PinMux[ch].reg &= ~(0xff << gCS_PinMux[ch].offset);
		//*(vuint32 *)gCS_PinMux[ch].reg |= (1 << gCS_PinMux[ch].offset);
		*(vuint32 *)gCS_PinMux[ch].reg &= ~(0x3 << gCS_PinMux[ch].offset);
		*(vuint32 *)gCS_PinMux[ch].reg |= (gCS_PinMux[ch].val << gCS_PinMux[ch].offset);

	}else{
		//*(vuint32 *)gCS_PinMux[ch].reg &= ~(0xff << gCS_PinMux[ch].offset);
		*(vuint32 *)gCS_PinMux[ch].reg &= ~(0x3 << gCS_PinMux[ch].offset);
		
	}

	return 1;

}

sint32 spi_cs_activate(sint32 ch)
{

	if(gCS_GPIO[ch].ch == -1)return -1;

	*(vuint32 *)(GPIO0_BASE_ADDR + 0x1000*gCS_GPIO[ch].group + 0x24) &= (~(1<<gCS_GPIO[ch].ch));	// gpio output value  set 0

	return 1;
}

sint32 spi_cs_deactivate(sint32 ch)
{
	if(gCS_GPIO[ch].ch == -1)return -1;

	*(vuint32 *)(GPIO0_BASE_ADDR + 0x1000*gCS_GPIO[ch].group + 0x24) |= (1<<gCS_GPIO[ch].ch);	// gpio output value  set 1

	return 1;
}

void spi_wait(sint32 ch)
{
	while(gpSPI[ch]->status & 1);
}



