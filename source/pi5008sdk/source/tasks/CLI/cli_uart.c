#ifdef SUPPORT_DEBUG_CLI
#include <nds32_intrinsic.h>
#include <CLI/d1088_defs.h>
#include <CLI/d1088_regs.h>

#include "osal.h"

#define DEFAULT_BAUDRATE	115200

#define IN8(reg) 		(uint8_t)( (*(volatile unsigned long *)(reg)) & 0x000000FF )
#define READ_CLR(reg) 	(*(volatile unsigned long *)(reg))

extern PP_U32 gu32BaudRate[eBAUDRATE_MAX];
extern PP_U32 gu32OSCR[eBAUDRATE_MAX];

int drv_uart_set_baudrate(int baudrate)
{
	unsigned long baud_div;	/* baud rate divisor */
	unsigned long temp_word;

	/* Set the baud rate */
//#ifdef CONFIG_PLAT_D1088_2GB
//# define UARTC_CLOCK          37125000U           /* 37.125MHz . FPGA */
# define UARTC_CLOCK          27000000U           /* 27MHz . ASIC */

//#else
//# error "undefined uartc_clock"
//#endif

	baud_div = (UARTC_CLOCK / (((gu32OSCR[baudrate] == 0) ? 32 : gu32OSCR[baudrate]) * gu32BaudRate[baudrate]));

	/* Save LCR temporary */
	temp_word = IN8(STUARTC_BASE + UARTC_LCR_OFFSET);

	/* Setup dlab bit for baud rate setting */
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, (temp_word | UARTC_LCR_DLAB));

	/* Apply baud rate */
	OUT8(STUARTC_BASE + UARTC_DLM_OFFSET, (unsigned char)(baud_div >> 8));
	OUT8(STUARTC_BASE + UARTC_DLL_OFFSET, (unsigned char)baud_div);
	OUT8(STUARTC_BASE + UARTC_PSR_OFFSET, (unsigned char)1);

	/* Restore LCR */
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, temp_word);
	
	return 0;
}

int drv_uart_is_kbd_hit(void)
{
	return IN8(STUARTC_BASE + UARTC_LSR_OFFSET) & UARTC_LSR_RDR;
}

int drv_uart_get_char(void)
{
	while (!(IN8(STUARTC_BASE + UARTC_LSR_OFFSET) & UARTC_LSR_RDR))
	{
		OSAL_sleep(10);
	}

	return IN8(STUARTC_BASE + UARTC_RBR_OFFSET);
}

void drv_uart_put_char(int ch)
{
	while (!(IN8(STUARTC_BASE + UARTC_LSR_OFFSET) & UARTC_LSR_THRE))
	{
		OSAL_sleep(10);
	}

	OUT8(STUARTC_BASE + UARTC_THR_OFFSET, ch);
}

int drv_uart_init(void)
{
	/* Clear everything */
	OUT8(STUARTC_BASE + UARTC_IER_OFFSET, 0x0);
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, 0x0);

	/* Setup baud rate */
	drv_uart_set_baudrate(DEFAULT_BAUDRATE);

	/* Setup parity, data bits, and stop bits */
	OUT8(STUARTC_BASE + UARTC_LCR_OFFSET, (UARTC_LCR_PARITY_NONE |
				UARTC_LCR_BITS8 | UARTC_LCR_STOP1));

	return 0;
}
#endif /* SUPPORT_DEBUG_CLI */