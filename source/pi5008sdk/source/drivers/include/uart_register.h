/* uart_register.h */
#ifndef __UART_REGISTER_H__
#define __UART_REGISTER_H__

#include "type.h"
#include "system.h"

#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/
//----------------------------------------------------------------------------
//	uart register
//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//  uart register map
//----------------------------------------------------------------------------
typedef volatile struct ppUART_REG_S {
	PP_U32 id;					// 0x00		- ID and Revision Register
	PP_U32 reserved_0x04;		// 0x04		- Reserved
	PP_U32 reserved_0x08;		// 0x08		- Reserved
	PP_U32 reserved_0x0c;		// 0x0C		- Reserved
	PP_U32 cfg;					// 0x10		- HW configure regisster
	PP_U32 oscr;				// 0x14		- Over sample control register
	PP_U32 reserved_0x18;		// 0x18		- Reserved
	PP_U32 reserved_0x1c;		// 0x1C		- Reserved
	union {						
		PP_U32 rbr;				// 0x20		- (DLAB = 0) Receiver Buffer Register (Read only)
		PP_U32 thr;				// 0x20 	- (DLAB = 0) Transmitter Holding Register (Write only)
		PP_U32 dll;				// 0x20 	- (DLAB = 1) Divisor latch LSB
	};
	union {
		PP_U32 ier;				// 0x24 	- (DLAB = 0) Interrupt enable register
		PP_U32 dlm;				// 0x24 	- (DLAB = 1) Divisor latch MSB
	};
	union {
		PP_U32 iir;				// 0x28 	- Interrupt identification register (Read only)
		PP_U32 fcr;				// 0x28 	- FIFO control register (Write only)
	};
	PP_U32 lcr;					// 0x2C 	- Line control register
	PP_U32 mcr;					// 0x30 	- Modem control register
	PP_U32 lsr;					// 0x34 	- Line status register
	PP_U32 msr;					// 0x38 	- Modem status register
	PP_U32 scr;					// 0x3C 	- Scratch Register
} PP_UART_REG_S;


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/



/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/


#ifdef __cplusplus
}
#endif

#endif  // __UART_REGISTER_H__
