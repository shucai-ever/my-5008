#ifdef SUPPORT_DEBUG_CLI
#include <stdio.h>
#include <CLI/cli_uart.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


unsigned char serial_getc(unsigned char *a_data) {
	*a_data = drv_uart_get_char();
	return 1;
}


void serial_putc(unsigned char *data, unsigned char a_len) {
	while (a_len--) {
		drv_uart_put_char(*data++);
	}
}
#endif /* SUPPORT_DEBUG_CLI */