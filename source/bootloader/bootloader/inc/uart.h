/* uart.h */
#ifndef __UART_H__
#define __UART_H__

#include "type.h"
#include "system.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagUART_REG_T
{
	vuint32 id;					/* 0x000 		- id */
	vuint32 rsv[3];				/* 0x004 ~ 0x00c	- reserved */
	vuint32 cfg;				/* 0x010			- HW configure regisster */	
	vuint32 oscr;				/* 0x014			- Over sample control register */	
	vuint32 rsv1[2];			/* 0x018 ~ 0x01c	- reserved */
	union {
		vuint32 rbr;			/* 0x020			- Receiver Buffer Register for Read */
		vuint32 thr;			/* 0x020 		- Transmitter Holding Register for Write */
		vuint32 dll;			/* 0x020 		- Divisor latch LSB */
	};
	union {
		vuint32 ier;			/* 0x024 		- Interrupt enable register */
		vuint32 dlm;			/* 0x024 		- Divisor latch MSB */
	};
	union {
		vuint32 iir;			/* 0x028 		- Interrupt identification register */
		vuint32 fcr;			/* 0x028 		- FIFO control register */
	};
	vuint32 lcr;				/* 0x02c 		- Line control register */
	vuint32 mcr;				/* 0x030 		- Modem control register */
	vuint32 lsr;				/* 0x034 		- Line status register */
	vuint32 msr;				/* 0x038 		- Modem status register */
	vuint32 scr;				/* 0x03c 		- Scratch Register */
}UART_REG_T;

#define UART0_CTRL_REG			((UART_REG_T*)			(UART0_BASE_ADDR + 0x0000U))
#define UART1_CTRL_REG			((UART_REG_T*)		(UART1_BASE_ADDR + 0x0000U))

/* Interface */

extern void uart_init(uint32 baudrate, uint32 databit, uint32 stopbit, uint32 parity);
extern void uart_outbyte(uint8 c);
extern uint8 uart_inbyte(void);

#ifdef __cplusplus
}
#endif

#endif // __UART_H__
