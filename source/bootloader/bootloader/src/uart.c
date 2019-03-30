#include "system.h"
#include "type.h"
#include "uart.h"
#include "debug.h"

/*---------------------------------------------------------------------------*/
/* Global Variables */
/*---------------------------------------------------------------------------*/
static UART_REG_T *gpUart = UART0_CTRL_REG;

static uint8 uart_get_rxReady (void)
{
	return (gpUart->lsr & 1);
}

static uint8 uart_get_rxData (void)
{
	return (gpUart->rbr & 0xff);
}

static uint8 uart_get_txEmpty (void)
{
	return ((gpUart->lsr>>5)&1);
}

static void uart_set_txData (uint8 data)
{
	gpUart->thr = data;
}

void uart_init(uint32 baudrate, uint32 databit, uint32 stopbit, uint32 parity)
{
	uint16 	div = 0;

	//div = UART_CLK / (18 * baudrate);
	div = UART_CLK / (16 * baudrate);

	// Set DLAB to 1
	gpUart->lcr |= (1<<7);

	// Set OSCR (default value)
	gpUart->oscr  = 16; 
	//gpUart->oscr  = 18; // minimize error ratio at 27MHz, 115200baud
	
	// Set DLL for baudrate
	gpUart->dll = ((div >> 0) & 0xff);
	gpUart->dlm = ((div >> 8) & 0xff);

	// Set DLAB to 0
	gpUart->lcr &= ~(1<<7);

	// LCR: data bit, stop bit, parity
	gpUart->lcr = (databit | stopbit | parity);
}


void uart_outbyte(uint8 c)
{
	uint32 reg = 0;

	do {
		reg = uart_get_txEmpty();
	} while(!reg);

	uart_set_txData(c);
}

uint8 uart_inbyte(void)
{
	while(!uart_get_rxReady());

	//if(gpUart->lsr & 2)dbg("rx buffer overrun\n");

	return uart_get_rxData();
}



