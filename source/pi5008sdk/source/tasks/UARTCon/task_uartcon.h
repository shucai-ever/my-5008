#ifndef __APP_UARTCON_H__
#define __APP_UARTCON_H__

#include "type.h"
#include "error.h"
#include "uart_drv.h"

PP_VOID vTaskUARTCon(PP_VOID *pvData);

PP_VOID AppUARTCon_Initialize(PP_UART_BAUDRATE_E baudrate, PP_UART_DATABIT_E databit, PP_UART_STOP_BIT_E stopbit, PP_UART_PARITY_E parity);
PP_VOID AppUARTCon_DebugPrint(PP_CHAR *pszFormat, ...);
PP_VOID AppUARTCon_DebugPrintFromISR(PP_CHAR *pszFormat, ...);
#endif // __APP_UARTCON_H__
