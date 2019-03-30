#ifdef SUPPORT_DEBUG_CLI
#ifndef __CLI_UART_H__
#define __CLI_UART_H__

extern int drv_uart_init(void);
extern int drv_uart_set_baudrate(int baudrate);
extern int drv_uart_is_kbd_hit(void);
extern int drv_uart_get_char(void);
extern void drv_uart_put_char(int ch);

#endif /* __CLI_UART_H__ */
#endif /* SUPPORT_DEBUG_CLI */