/*----------------------------------------------------------------
//                                                              //
// Copyright (C) 2010 Authors and OPENCORES.ORG                 //
//                                                              //
// This source file may be used and distributed without         //
// restriction provided that this copyright statement is not    //
// removed from the file and that any derivative work contains  //
// the original copyright notice and the associated disclaimer. //
//                                                              //
// This source file is free software; you can redistribute it   //
// and/or modify it under the terms of the GNU Lesser General   //
// Public License as published by the Free Software Foundation; //
// either version 2.1 of the License, or (at your option) any   //
// later version.                                               //
//                                                              //
// This source is distributed in the hope that it will be       //
// useful, but WITHOUT ANY WARRANTY; without even the implied   //
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      //
// PURPOSE.  See the GNU Lesser General Public License for more //
// details.                                                     //
//                                                              //
// You should have received a copy of the GNU Lesser General    //
// Public License along with this source; if not, download it   //
// from http://www.opencores.org/lgpl.shtml                     //
//                                                              //
----------------------------------------------------------------*/

#include <FreeRTOS.h>
#include <timers.h>

#include "system.h"
#include "clock.h"
#include "type.h"
#include "error.h"
#include "pi5008.h"
#include "uart_register.h"
#include "uart_drv.h"

#include "utils.h"
#include "osal.h"
#include "interrupt.h"
#include "dma.h"
#include "proc.h"

#include "task_manager.h"

/*---------------------------------------------------------------------------*/
/* Defines */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function Prototypes */
/*---------------------------------------------------------------------------*/
ISR(uart_isr0, num);
ISR(uart_isr1, num);
ISR(uart_isr2, num);

/*---------------------------------------------------------------------------*/
/* Global Variables */
/*---------------------------------------------------------------------------*/
STATIC PP_UART_REG_S *gpUart[eUART_MAX] = {
	(PP_UART_REG_S *)UART0_BASE_ADDR,
#if defined(SYSTEM_BUS_DW)
	(PP_UART_REG_S *)UART1_BASE_ADDR,
	(PP_UART_REG_S *)UART2_BASE_ADDR,
#endif
};

STATIC PP_U32 gUARTRequestTx[eUART_MAX] = {
	UART0_DMA_TX_REQ,
#if defined(SYSTEM_BUS_DW)
	UART1_DMA_TX_REQ,
	UART2_DMA_TX_REQ,
#endif
};

STATIC PP_U32 gUARTRequestRx[eUART_MAX] = {
	UART0_DMA_RX_REQ,
#if defined(SYSTEM_BUS_DW)
	UART1_DMA_RX_REQ,
	UART2_DMA_RX_REQ,
#endif
};

STATIC PP_VOID *gUARTISR[eUART_MAX] = {
		uart_isr0,
		uart_isr1,
		uart_isr2
};

STATIC PP_UartChannel_S uartBufInfo[eUART_MAX];

PP_U32 gu32BaudRate[eBAUDRATE_MAX] = { 2400,	4800, 9600,	19200, 38400, 57600, 100000, 115200, 230400, 460800 };
PP_U32 gu32OSCR[eBAUDRATE_MAX] = { 10, 8, 14, 9, 8, 18, 10, 18, 8, 10 };
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
STATIC PP_U32 gu32UARTIntcCnt[eUART_MAX] = {0};

//PROC irq
PP_RESULT_E UART0_PROC_IRQ(PP_S32 argc, CONST PP_CHAR **argv);
PP_RESULT_E UART1_PROC_IRQ(PP_S32 argc, CONST PP_CHAR **argv);
PP_RESULT_E UART2_PROC_IRQ(PP_S32 argc, CONST PP_CHAR **argv);

struct proc_irq_struct stUART0_PROC_irqs[] = {	{ .fn = UART0_PROC_IRQ,    .irqNum = IRQ_UART0_VECTOR, .next = (PP_VOID*)0, },	};
struct proc_irq_struct stUART1_PROC_irqs[] = {	{ .fn = UART1_PROC_IRQ,    .irqNum = IRQ_UART1_VECTOR, .next = (PP_VOID*)0, },	};
struct proc_irq_struct stUART2_PROC_irqs[] = {	{ .fn = UART2_PROC_IRQ,    .irqNum = IRQ_UART2_VECTOR, .next = (PP_VOID*)0, },	};

//PROC device
PP_RESULT_E UART0_PROC_DEVICE(PP_S32 argc, CONST PP_CHAR **argv);
PP_RESULT_E UART1_PROC_DEVICE(PP_S32 argc, CONST PP_CHAR **argv);
PP_RESULT_E UART2_PROC_DEVICE(PP_S32 argc, CONST PP_CHAR **argv);

struct proc_device_struct stUART0_PROC_devices[] = {	{ .pName = "UART0",  .fn = UART0_PROC_DEVICE,    .next = (PP_VOID*)0, },	};
struct proc_device_struct stUART1_PROC_devices[] = {	{ .pName = "UART1",  .fn = UART1_PROC_DEVICE,    .next = (PP_VOID*)0, },	};
struct proc_device_struct stUART2_PROC_devices[] = {	{ .pName = "UART2",  .fn = UART2_PROC_DEVICE,    .next = (PP_VOID*)0, },	};

PP_RESULT_E UART0_PROC_IRQ(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("UART0", IRQ_UART0_VECTOR, gu32UARTIntcCnt[eUART_CH_0]);
	return(eSUCCESS);
}

PP_RESULT_E UART1_PROC_IRQ(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("UART1", IRQ_UART1_VECTOR, gu32UARTIntcCnt[eUART_CH_1]);
	return(eSUCCESS);
}

PP_RESULT_E UART2_PROC_IRQ(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
//	PRINT_PROC_IRQ(NAME, NUM, CNT)
	PRINT_PROC_IRQ("UART2", IRQ_UART2_VECTOR, gu32UARTIntcCnt[eUART_CH_2]);
	return(eSUCCESS);
}

PP_RESULT_E UART0_PROC_DEVICE(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
	PP_U32 ch = eUART_CH_0;

	if( (argc) && (strcmp(argv[0], stUART0_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}
	printf("\n%s Device Info -------------\n", stUART0_PROC_devices[0].pName);

	printf("### UART%d info(Driver ID : 0x%08X) ### \n", ch, gpUart[ch]->id);

	printf("------------------ HW info -------------------------\n");
	printf("FIFO_DEPTH : ");
	if((gpUart[ch]->cfg&0x3) == 0)
		printf("16byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 1)
		printf("32byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 2)
		printf("64byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 3)
		printf("128byte FIFO\n");
	printf("OSC : %d\n", gpUart[ch]->oscr&0x1F);
	
	printf("------------------ interrupt enable reg -------------------------\n");
	printf("ERBI : %s\n", (gpUart[ch]->ier&0x1)?"Enable":"Disable");
	printf("ETHEI : %s\n", ((gpUart[ch]->ier>>1)&0x1)?"Enable":"Disable");
	printf("ELSI : %s\n", ((gpUart[ch]->ier>>2)&0x1)?"Enable":"Disable");
	printf("EMSI : %s\n", ((gpUart[ch]->ier>>3)&0x1)?"Enable":"Disable");

	printf("------------------ interrupt identification reg -------------------------\n");
	printf("FIFOED : %s\n", (((gpUart[ch]->iir>>6)&0x3) == 0x3)?"Enable":"Disable");
	printf("Interrupt Type : ");
	if((gpUart[ch]->iir&0xF) == 0x0)
		printf("Modem status\n");
	else if((gpUart[ch]->iir&0xF) == 0x1)
		printf("None\n");
	else if((gpUart[ch]->iir&0xF) == 0x2)
		printf("Transmitter Holding Register empty\n");
	else if((gpUart[ch]->iir&0xF) == 0x4)
		printf("Received data available\n");
	else if((gpUart[ch]->iir&0xF) == 0x6)
		printf("Received line status\n");
	else if((gpUart[ch]->iir&0xF) == 0xC)
		printf("Character timeout\n");

	printf("------------------ line control reg -------------------------\n");
	printf("WLS : %d bits\n", 5+(gpUart[ch]->lcr&0x3));
	printf("STB : ");
	if(((gpUart[ch]->lcr>>2)&0x1) == 1)
	{
		if((gpUart[ch]->lcr&0x3) == 0)
			printf("1.5 bits\n");
		else
			printf("2 bits\n");
	}
	else
	{
		printf("1 bits\n");
	}
	printf("Parity bit : ");
	if(((gpUart[ch]->lcr>>3)&0x1) == 1)
	{
		if(((gpUart[ch]->lcr>>5)&0x1) == 1)
		{
			if(((gpUart[ch]->lcr>>4)&0x1) == 1)
				printf("Parity bit is always 0\n");
			else
				printf("Parity bit is always 1\n");
		}
		else
		{
			if(((gpUart[ch]->lcr>>4)&0x1) == 1)
				printf("Parity is even\n");
			else
				printf("Parity is odd\n");
		}
	}
	else
	{
		printf("No parity bit\n");
	}
	printf("BC : %s\n", ((gpUart[ch]->lcr>>6)&0x1)?"Enable":"Disable");

	printf("------------------ line status reg -------------------------\n");
	printf("DR : %s\n", (gpUart[ch]->lsr&0x1)?"O":"X");
	printf("OE : %s\n", ((gpUart[ch]->lsr>>1)&0x1)?"O":"X");
	printf("PE : %s\n", ((gpUart[ch]->lsr>>2)&0x1)?"O":"X");
	printf("FE : %s\n", ((gpUart[ch]->lsr>>3)&0x1)?"O":"X");
	printf("LBreak : %s\n", ((gpUart[ch]->lsr>>4)&0x1)?"O":"X");
	printf("THRE : %s\n", ((gpUart[ch]->lsr>>5)&0x1)?"O":"X");
	printf("TEMT : %s\n", ((gpUart[ch]->lsr>>6)&0x1)?"O":"X");
	printf("ERRF : %s\n", ((gpUart[ch]->lsr>>7)&0x1)?"O":"X");

	printf("--------------------------------------------------------------\n");
	return(eSUCCESS);
}

PP_RESULT_E UART1_PROC_DEVICE(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
	PP_U32 ch = eUART_CH_1;

	if( (argc) && (strcmp(argv[0], stUART1_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}
	printf("\n%s Device Info -------------\n", stUART1_PROC_devices[0].pName);

	printf("### UART%d info(Driver ID : 0x%08X) ### \n", ch, gpUart[ch]->id);

	printf("------------------ HW info -------------------------\n");
	printf("FIFO_DEPTH : ");
	if((gpUart[ch]->cfg&0x3) == 0)
		printf("16byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 1)
		printf("32byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 2)
		printf("64byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 3)
		printf("128byte FIFO\n");
	printf("OSC : %d\n", gpUart[ch]->oscr&0x1F);
	
	printf("------------------ interrupt enable reg -------------------------\n");
	printf("ERBI : %s\n", (gpUart[ch]->ier&0x1)?"Enable":"Disable");
	printf("ETHEI : %s\n", ((gpUart[ch]->ier>>1)&0x1)?"Enable":"Disable");
	printf("ELSI : %s\n", ((gpUart[ch]->ier>>2)&0x1)?"Enable":"Disable");
	printf("EMSI : %s\n", ((gpUart[ch]->ier>>3)&0x1)?"Enable":"Disable");

	printf("------------------ interrupt identification reg -------------------------\n");
	printf("FIFOED : %s\n", (((gpUart[ch]->iir>>6)&0x3) == 0x3)?"Enable":"Disable");
	printf("Interrupt Type : ");
	if((gpUart[ch]->iir&0xF) == 0x0)
		printf("Modem status\n");
	else if((gpUart[ch]->iir&0xF) == 0x1)
		printf("None\n");
	else if((gpUart[ch]->iir&0xF) == 0x2)
		printf("Transmitter Holding Register empty\n");
	else if((gpUart[ch]->iir&0xF) == 0x4)
		printf("Received data available\n");
	else if((gpUart[ch]->iir&0xF) == 0x6)
		printf("Received line status\n");
	else if((gpUart[ch]->iir&0xF) == 0xC)
		printf("Character timeout\n");

	printf("------------------ line control reg -------------------------\n");
	printf("WLS : %d bits\n", 5+(gpUart[ch]->lcr&0x3));
	printf("STB : ");
	if(((gpUart[ch]->lcr>>2)&0x1) == 1)
	{
		if((gpUart[ch]->lcr&0x3) == 0)
			printf("1.5 bits\n");
		else
			printf("2 bits\n");
	}
	else
	{
		printf("1 bits\n");
	}
	printf("Parity bit : ");
	if(((gpUart[ch]->lcr>>3)&0x1) == 1)
	{
		if(((gpUart[ch]->lcr>>5)&0x1) == 1)
		{
			if(((gpUart[ch]->lcr>>4)&0x1) == 1)
				printf("Parity bit is always 0\n");
			else
				printf("Parity bit is always 1\n");
		}
		else
		{
			if(((gpUart[ch]->lcr>>4)&0x1) == 1)
				printf("Parity is even\n");
			else
				printf("Parity is odd\n");
		}
	}
	else
	{
		printf("No parity bit\n");
	}
	printf("BC : %s\n", ((gpUart[ch]->lcr>>6)&0x1)?"Enable":"Disable");

	printf("------------------ line status reg -------------------------\n");
	printf("DR : %s\n", (gpUart[ch]->lsr&0x1)?"O":"X");
	printf("OE : %s\n", ((gpUart[ch]->lsr>>1)&0x1)?"O":"X");
	printf("PE : %s\n", ((gpUart[ch]->lsr>>2)&0x1)?"O":"X");
	printf("FE : %s\n", ((gpUart[ch]->lsr>>3)&0x1)?"O":"X");
	printf("LBreak : %s\n", ((gpUart[ch]->lsr>>4)&0x1)?"O":"X");
	printf("THRE : %s\n", ((gpUart[ch]->lsr>>5)&0x1)?"O":"X");
	printf("TEMT : %s\n", ((gpUart[ch]->lsr>>6)&0x1)?"O":"X");
	printf("ERRF : %s\n", ((gpUart[ch]->lsr>>7)&0x1)?"O":"X");

	printf("--------------------------------------------------------------\n");
	return(eSUCCESS);
}

PP_RESULT_E UART2_PROC_DEVICE(PP_S32 IN argc, CONST PP_CHAR** IN argv)
{
	PP_U32 ch = eUART_CH_2;

	if( (argc) && (strcmp(argv[0], stUART2_PROC_devices[0].pName) && strcmp(argv[0], "ALL")) )
	{
		return(eERROR_INVALID_ARGUMENT);
	}
	printf("\n%s Device Info -------------\n", stUART2_PROC_devices[0].pName);

	printf("### UART%d info(Driver ID : 0x%08X) ### \n", ch, gpUart[ch]->id);

	printf("------------------ HW info -------------------------\n");
	printf("FIFO_DEPTH : ");
	if((gpUart[ch]->cfg&0x3) == 0)
		printf("16byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 1)
		printf("32byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 2)
		printf("64byte FIFO\n");
	else if((gpUart[ch]->cfg&0x3) == 3)
		printf("128byte FIFO\n");
	printf("OSC : %d\n", gpUart[ch]->oscr&0x1F);
	
	printf("------------------ interrupt enable reg -------------------------\n");
	printf("ERBI : %s\n", (gpUart[ch]->ier&0x1)?"Enable":"Disable");
	printf("ETHEI : %s\n", ((gpUart[ch]->ier>>1)&0x1)?"Enable":"Disable");
	printf("ELSI : %s\n", ((gpUart[ch]->ier>>2)&0x1)?"Enable":"Disable");
	printf("EMSI : %s\n", ((gpUart[ch]->ier>>3)&0x1)?"Enable":"Disable");

	printf("------------------ interrupt identification reg -------------------------\n");
	printf("FIFOED : %s\n", (((gpUart[ch]->iir>>6)&0x3) == 0x3)?"Enable":"Disable");
	printf("Interrupt Type : ");
	if((gpUart[ch]->iir&0xF) == 0x0)
		printf("Modem status\n");
	else if((gpUart[ch]->iir&0xF) == 0x1)
		printf("None\n");
	else if((gpUart[ch]->iir&0xF) == 0x2)
		printf("Transmitter Holding Register empty\n");
	else if((gpUart[ch]->iir&0xF) == 0x4)
		printf("Received data available\n");
	else if((gpUart[ch]->iir&0xF) == 0x6)
		printf("Received line status\n");
	else if((gpUart[ch]->iir&0xF) == 0xC)
		printf("Character timeout\n");

	printf("------------------ line control reg -------------------------\n");
	printf("WLS : %d bits\n", 5+(gpUart[ch]->lcr&0x3));
	printf("STB : ");
	if(((gpUart[ch]->lcr>>2)&0x1) == 1)
	{
		if((gpUart[ch]->lcr&0x3) == 0)
			printf("1.5 bits\n");
		else
			printf("2 bits\n");
	}
	else
	{
		printf("1 bits\n");
	}
	printf("Parity bit : ");
	if(((gpUart[ch]->lcr>>3)&0x1) == 1)
	{
		if(((gpUart[ch]->lcr>>5)&0x1) == 1)
		{
			if(((gpUart[ch]->lcr>>4)&0x1) == 1)
				printf("Parity bit is always 0\n");
			else
				printf("Parity bit is always 1\n");
		}
		else
		{
			if(((gpUart[ch]->lcr>>4)&0x1) == 1)
				printf("Parity is even\n");
			else
				printf("Parity is odd\n");
		}
	}
	else
	{
		printf("No parity bit\n");
	}
	printf("BC : %s\n", ((gpUart[ch]->lcr>>6)&0x1)?"Enable":"Disable");

	printf("------------------ line status reg -------------------------\n");
	printf("DR : %s\n", (gpUart[ch]->lsr&0x1)?"O":"X");
	printf("OE : %s\n", ((gpUart[ch]->lsr>>1)&0x1)?"O":"X");
	printf("PE : %s\n", ((gpUart[ch]->lsr>>2)&0x1)?"O":"X");
	printf("FE : %s\n", ((gpUart[ch]->lsr>>3)&0x1)?"O":"X");
	printf("LBreak : %s\n", ((gpUart[ch]->lsr>>4)&0x1)?"O":"X");
	printf("THRE : %s\n", ((gpUart[ch]->lsr>>5)&0x1)?"O":"X");
	printf("TEMT : %s\n", ((gpUart[ch]->lsr>>6)&0x1)?"O":"X");
	printf("ERRF : %s\n", ((gpUart[ch]->lsr>>7)&0x1)?"O":"X");

	printf("--------------------------------------------------------------\n");
	return(eSUCCESS);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------*/
/* Function Prototypes */
/*---------------------------------------------------------------------------*/

PP_VOID PPDRV_UART_SWreset (PP_VOID)
{
	(*((PP_VU32 *)(0xF0000040))) &= ~(1<<7);
	(*((PP_VU32 *)(0xF0000040))) |= (1<<7);
}

STATIC PP_VOID uart0_tx_isr (PP_VOID)
{
    // dummy ISR
	EXTERN PP_VOID uart_dbg_tx_isr(PP_VOID);
	uart_dbg_tx_isr();
}

STATIC PP_VOID uart0_rx_isr (PP_VOID)
{
    // dummy ISR
}

STATIC PP_VOID uart1_tx_isr (PP_VOID)
{
    // dummy ISR
}

STATIC PP_VOID uart1_rx_isr (PP_VOID)
{
	EXTERN PP_VOID uart_rx_rem_isr (PP_VOID);
	uart_rx_rem_isr();
}


#define UART_CH				UART_PRT_CH		// UART_PRT_CH
#define DEFAULT_BAUDRATE	eBAUDRATE_115200
#define DEFAULT_DATABIT		eDATA_BIT_8
#define DEFAULT_STOPBIT		eSTOP_BIT_1
#define DEFAULT_PARITY		ePARITY_NONE

#define BUF_SIZE 1024
PP_U8 uart_buf[BUF_SIZE] = {0};
PP_U32 rx_idx = 0;
PP_U32 tx_idx = 0;
PP_U32 rx_byte_cnt = 0;
PP_U32 tx_byte_cnt = 0;
PP_U32 uart_int_test_flag = 0;


PP_VOID uart_rx_rem_isr (PP_VOID)
{
	PP_RESULT_E result;
	
	while( PPDRV_UART_GetRxReady(UART_CH) )
	{
		uart_buf[rx_idx++] = PPDRV_UART_GetRxData(UART_CH);
		if(rx_idx >= BUF_SIZE)
			rx_idx = 0;
	}

	if(uart_buf[rx_idx-1]=='\n') //end of remocon protocol
	{
		result=AppTask_SendCmdFromISR(CMD_GET_REMOCON, TASK_MONITOR, TASK_MONITOR, 0, uart_buf, rx_idx);
		if(result)printf("circ buffer overflow \n");
		memset(uart_buf,0,BUF_SIZE);
		rx_idx=0;		
	}
	
	
	
}

PP_VOID uart_rx_weltrend_isr (PP_VOID)
{
	while( PPDRV_UART_GetRxReady(UART_CH) )
	{
		uart_buf[rx_idx++] = PPDRV_UART_GetRxData(UART_CH);
		if(rx_idx >= BUF_SIZE)
			rx_idx = 0;
	}

	if(uart_buf[rx_idx-1]==0xF5) //end of weltrend protocol
	{
		AppTask_SendCmdFromISR(CMD_GET_WELTREND, TASK_MONITOR, TASK_MONITOR, 0, uart_buf, rx_idx);
		memset(uart_buf,0,BUF_SIZE);
		rx_idx=0;		
	}
	
	
	
}

#if 0 //TODO delete by OHLee
STATIC PP_VOID uart1_tx_isr (PP_VOID)
{
    // dummy ISR
}

/* ISR bottom half*/
PP_VOID uart1_put_queue(PP_VOID * buf, PP_U32_t arg2)
{
    PP_UartQueueMsg_S *msg;
    sys_os_queue_t *queue = GET_QUEUE_HANDLE(QUEUE_HCI);
    PP_UartData_S *bufInfo;

    bufInfo = (PP_UartData_S *)buf;

    if(bufInfo != NULL)
    {
        msg = pvPortMalloc(sizeof(PP_UartQueueMsg_S));
        msg->buffer = pvPortMalloc(sizeof(PP_U8) * bufInfo->dataCount);
        memcpy(msg->buffer, bufInfo->data, bufInfo->dataCount);
        msg->length = bufInfo->dataCount;
        
        queue->msg = (PP_VOID *)msg;
        OSAL_post_queue(queue);
        bufInfo->dataCount = 0;
        bufInfo->dataLength = 255;
    }
}

STATIC PP_VOID uart1_rx_isr (PP_VOID)
{
    PP_U8 bufferIndex = uartBufInfo[UART_PRT_CH].rxBufferIndex;
    PP_U16 dataIndex = uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount;
    PP_U8 prevBufferIndex;
    BaseType_t xHigherPriorityTaskWoken;

	while(PPDRV_UART_GetRxReady(UART_PRT_CH))
    {
		uartBufInfo[UART_PRT_CH].rx[bufferIndex].data[dataIndex] = PPDRV_UART_GetRxData(UART_PRT_CH);
        if(dataIndex < eUART_HEADER_END)
        {
            /* HEADER Parsing */
            switch(dataIndex)
            {
                case ((PP_U8)eUART_HEADER_SOF) :
                    if( uartBufInfo[UART_PRT_CH].rx[bufferIndex].data[(PP_U8)eUART_HEADER_SOF] == FRAME_SOF)
                    {
                        uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount++;
                        dataIndex++;
                    }
                    else
                    {
                        printf("SOF ERROR \n");
                        uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount = 0;
                        dataIndex = 0;
                    }
                    break;
                case ((PP_U8)eUART_HEADER_COMMAND) :
                    if( uartBufInfo[UART_PRT_CH].rx[bufferIndex].data[(PP_U8)eUART_HEADER_COMMAND] == FRAME_COMMAND_ALL)
                    {
                        uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount++;
                        dataIndex++;
                    }
                    else
                    {
                        printf("COMMAND ERROR\n");
                        uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount = 0;
                        dataIndex = 0;    
                    }
                    break;
                case ((PP_U8)eUART_HEADER_LENGTH) :
                    if(uartBufInfo[UART_PRT_CH].rx[bufferIndex].data[(PP_U8)eUART_HEADER_LENGTH] < UART_BUF_SIZE_MAX)
                    {
                        uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount++;
                        dataIndex++;
                        uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataLength = uartBufInfo[UART_PRT_CH].rx[bufferIndex].data[(PP_U8)eUART_HEADER_LENGTH]
                                                                        + dataIndex + 2/*CRC*/;
                    }
                    else
                    {
                        /* Size is over 255 */
                    }
                    break;
                default :
                     printf("Unknwon ERROR\n");
                    uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount = 0;
                    dataIndex = 0;   
                    break;
            }
        }
        else
        {
            /* Data Parsing */
            uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount++;
            dataIndex++;

            if(dataIndex >= uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataLength)
            {
            	uartBufInfo[UART_PRT_CH].rx[bufferIndex].dataCount = 0;
            	dataIndex = 0;
                /* Double Buffer Switching */
                uartBufInfo[UART_PRT_CH].rxBufferIndex ^= 0x01U;
                prevBufferIndex = bufferIndex;
                bufferIndex ^= 0x01U;
                xHigherPriorityTaskWoken = pdFALSE;
                xTimerPendFunctionCallFromISR(uart1_put_queue, (PP_VOID *)&uartBufInfo[UART_PRT_CH].rx[prevBufferIndex], 0, &xHigherPriorityTaskWoken);
                if(xHigherPriorityTaskWoken == pdTRUE)
                {
                    portYIELD_FROM_ISR(/*xHigherPriorityTaskWoken*/);
                }
            }
        }
    }
	
}
#endif

STATIC PP_VOID uart2_tx_isr (PP_VOID)
{
    // dummy ISR
}

STATIC PP_VOID uart2_rx_isr (PP_VOID)
{
    // dummy ISR
}


ISR(uart_isr0, num)
{
    PP_U32 msk = (1 << num);

    PP_U8 interruptId;

    INTC_irq_clean(num);
    //INTC_irq_enable(num);

    __nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
    __nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	interruptId = gpUart[0]->iir & 0x0FU;

	switch (interruptId)
	{
		case 0b0100: /* rx */
			uart0_rx_isr();
			break;
		case 0b0010: /* tx */
			uart0_tx_isr();
			break;
		default:
			/* NOP */
			break;
	}

	gu32UARTIntcCnt[eUART_CH_0]++;

    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

ISR(uart_isr1, num)
{
    PP_U32 msk = (1 << num);

    PP_U8 interruptId;

    INTC_irq_clean(num);
    //INTC_irq_enable(num);

    __nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
    __nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	interruptId = gpUart[1]->iir & 0x0FU;

	switch (interruptId)
	{
		case 0b0100: /* rx */
			uart1_rx_isr();
			break;
		case 0b0010: /* tx */
			uart1_tx_isr();
			break;
		default:
			/* NOP */
			break;
	}

	gu32UARTIntcCnt[eUART_CH_1]++;

    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);
}

ISR(uart_isr2, num)
{
    PP_U32 msk = (1 << num);

    PP_U8 interruptId;

    INTC_irq_clean(num);
    //INTC_irq_enable(num);

    __nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
    __nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	interruptId = gpUart[2]->iir & 0x0FU;

	switch (interruptId)
	{
		case 0b0100: /* rx */
			uart2_rx_isr();
			break;
		case 0b0010: /* tx */
			uart2_tx_isr();
			break;
		default:
			/* NOP */
			break;
	}

	gu32UARTIntcCnt[eUART_CH_2]++;

    __nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);

}

PP_U32 PPDRV_UART_GetID(PP_U32 IN ch)
{
    return gpUart[ch]->id;
}

PP_U8 PPDRV_UART_GetRxReady (PP_U8 IN ch)
{
	return utilGetBits(gpUart[ch]->lsr, 0, 1);
}

PP_U8 PPDRV_UART_GetRxData (PP_U8 IN ch)
{
	return utilGetBits(gpUart[ch]->rbr, 0, 8);
}

PP_U8 PPDRV_UART_GetTxEmpty (PP_U8 IN ch)
{
	return utilGetBits(gpUart[ch]->lsr, 5, 1);
}

PP_VOID PPDRV_UART_SetTxData (PP_U8 IN ch, PP_U8 IN data)
{
	gpUart[ch]->thr = data;
}

PP_VOID PPDRV_UART_EnableLoopback (PP_U8 IN ch, PP_BOOL IN enable)
{
	if(enable)
		gpUart[ch]->mcr |= (1<<4);
	else
		gpUart[ch]->mcr &= ~(1<<4);
}

PP_VOID PPDRV_UART_Init (PP_UART_CHANNEL_E IN ch, PP_UART_BAUDRATE_E IN baudrate, PP_UART_DATABIT_E IN databit, PP_UART_STOP_BIT_E IN stopbit, PP_UART_PARITY_E IN parity)
{
	PP_U16 	div = 0;

	// Set OSCR (default value)
	gpUart[ch]->oscr  = (gu32OSCR[baudrate] == 32 ? 0 : gu32OSCR[baudrate]);

	div = u32UARTClk / ( ((gpUart[ch]->oscr == 0) ? 32 : gpUart[ch]->oscr)  * gu32BaudRate[baudrate]);

	// Set LCR initialize
	gpUart[ch]->lcr = 0;

	// Set DLAB to 1
	gpUart[ch]->lcr |= (1<<7);
	
	// Set DLL for baudrate
	gpUart[ch]->dll = ((div >> 0) & 0xff);
	gpUart[ch]->dlm = ((div >> 8) & 0xff);

	// Set DLAB to 0
	gpUart[ch]->lcr &= ~(1<<7);

	// LCR : data bit
	if(databit == eDATA_BIT_5)
		gpUart[ch]->lcr |= 0;
	else if(databit == eDATA_BIT_6)
		gpUart[ch]->lcr |= 1;
	else if(databit == eDATA_BIT_7)
		gpUart[ch]->lcr |= 2;
	else if(databit == eDATA_BIT_8)
		gpUart[ch]->lcr |= 3;
			
	// LCR : stop bit
	if (stopbit == eSTOP_BIT_1)
	{
		gpUart[ch]->lcr |= (0<<2);
	}
	else if (stopbit == eSTOP_BIT_1_5)
	{
		gpUart[ch]->lcr |= (1<<2);
		gpUart[ch]->lcr &= ~(1<<0); 
	}
	else if (stopbit == eSTOP_BIT_2)
	{
		gpUart[ch]->lcr |= (1<<2);
	}
		
	// LCR : parity
	if(parity == ePARITY_NONE)
	{
		gpUart[ch]->lcr |= (0<<3);
	}
	else if(parity == ePARITY_ODD)
	{
		gpUart[ch]->lcr |= (1<<3) | (0<<4) | (0<<5);
	}
	else if(parity == ePARITY_EVEN)
	{
		gpUart[ch]->lcr |= (1<<3) | (1<<4) | (0<<5);
	}
	else if(parity == ePARITY_MARK)
	{
		gpUart[ch]->lcr |= (1<<3) | (0<<4) | (1<<5);
	}
	else if(parity == ePARITY_SPACE)
	{
		gpUart[ch]->lcr |= (1<<3) | (1<<4) | (1<<5);
	}

	if(ch == eUART_CH_0)
	{
		SYS_PROC_addDevice(stUART0_PROC_devices);
	}
	else if(ch == eUART_CH_1)
	{
		SYS_PROC_addDevice(stUART1_PROC_devices);
	}
	else if(ch == eUART_CH_2)
	{
		SYS_PROC_addDevice(stUART2_PROC_devices);
	}
}

PP_VOID PPDRV_UART_InitInterrupt (PP_UART_CHANNEL_E IN ch, PP_UartFifoOpt_S IN opt)
{
    sys_os_isr_t* old = NULL_PTR;
    PP_U8 bufferIndex = 0;
    PP_U16 dataIndex = 0;

    /* IER: Interrupt enable register. */
	gpUart[ch]->ier = (opt.rxEnable << 0) | /* rx */
                      (opt.txEnable << 1); /* tx */

    /* FCR: Enable FIFO, reset TX and RX. */
    gpUart[ch]->fcr = (opt.rxLevel<<6) | (opt.txLevel<<4) | (1<<2) | (1<<1) | (1<<0);


    /* Initailize Uart Buffer */
    uartBufInfo[ch].rxBufferIndex = 0;
    for(bufferIndex = 0; bufferIndex < UART_NUM_OF_BUF; bufferIndex++)
    {
        uartBufInfo[ch].rx[bufferIndex].status = 0;
        uartBufInfo[ch].rx[bufferIndex].dataCount = 0U;
        uartBufInfo[ch].rx[bufferIndex].dataLength = 255U;
        for(dataIndex = 0; dataIndex < UART_BUF_SIZE_MAX; dataIndex++)
        {
            uartBufInfo[ch].rx[bufferIndex].data[dataIndex] = 0U;
        }
    }

	OSAL_register_isr(IRQ_UART0_VECTOR + ch, gUARTISR[ch], old);
	INTC_irq_clean(IRQ_UART0_VECTOR + ch);
	INTC_irq_enable(IRQ_UART0_VECTOR + ch);

	if(ch == eUART_CH_0)
	{
		SYS_PROC_addIrq(stUART0_PROC_irqs);
	}
	else if(ch == eUART_CH_1)
	{
		SYS_PROC_addIrq(stUART1_PROC_irqs);
	}
	else if(ch == eUART_CH_2)
	{
		SYS_PROC_addIrq(stUART2_PROC_irqs);
	}
}

PP_RESULT_E PPDRV_UART_SetDmaTx (PP_UART_CHANNEL_E IN ch, PP_UART_TX_LEVEL IN level, CONST PP_U8* IN din, PP_U32 IN size, PP_U32 IN dma_ch, PP_U32 u32Timeout)
{
	PP_DMA_CONFIG_S cfg;
	PP_RESULT_E ret = eSUCCESS;
	
	if(gUARTRequestTx[ch] == (PP_U32)(-1))return eERROR_FAILURE;

	// init dma
	memset(&cfg.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg.ctrl.ctl.priority = 0;
	cfg.ctrl.ctl.src_burst_size = DMA_BSIZE_1;
	cfg.ctrl.ctl.src_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.dst_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.src_mode = 0;		// normal
	cfg.ctrl.ctl.dst_mode = 1;		// handshake
	cfg.ctrl.ctl.src_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.dst_addr_ctrl = 2;	// fixed
	cfg.ctrl.ctl.src_req_sel = 0;
	cfg.ctrl.ctl.dst_req_sel = gUARTRequestTx[ch];
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;
	cfg.u32SrcAddr = (PP_U32)(din);
	cfg.u32DstAddr = (PP_U32)(&gpUart[ch]->thr);
	cfg.u32TrasnferSize = (size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;
	PPDRV_DMA_SetConfig(dma_ch, &cfg);

	// FCR : fifo en. tx/rx fifo clr. dma en. set tx/rx fifo lv.
	//gpUart[ch]->fcr = 0xFF;// (gpUart[ch]->fcr&0xF0) | 0xF;
	// 0F(X) 5F(O) AF(0) FF(0)
	gpUart[ch]->fcr = (level<<4) | (1<<3) | (1<<2) | (1<<1) | (1<<0);

	ret = PPDRV_DMA_Start(dma_ch, u32Timeout);
	
	return ret;
}

PP_RESULT_E PPDRV_UART_SetDmaRx(PP_UART_CHANNEL_E IN ch, PP_UART_RX_LEVEL IN level, PP_U8* OUT dout, PP_U32 IN size, PP_U32 IN dma_ch, PP_U32 u32Timeout)
{
	PP_DMA_CONFIG_S cfg;
	PP_RESULT_E ret = eSUCCESS;

	if(gUARTRequestRx[ch] == (PP_U32)(-1))return eERROR_FAILURE;

	// init dma
	memset(&cfg.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg.ctrl.ctl.priority = 0;
	cfg.ctrl.ctl.src_burst_size = DMA_BSIZE_1;
	cfg.ctrl.ctl.src_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.dst_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.src_mode = 1;		// handshake
	cfg.ctrl.ctl.dst_mode = 0;		// normal
	cfg.ctrl.ctl.src_addr_ctrl = 2;	// fixed
	cfg.ctrl.ctl.dst_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.src_req_sel = gUARTRequestRx[ch];
	cfg.ctrl.ctl.dst_req_sel = 0;
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;
	cfg.u32SrcAddr = (PP_U32)(&gpUart[ch]->rbr);
	cfg.u32DstAddr = (PP_U32)(dout);
	cfg.u32TrasnferSize = (size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;
	PPDRV_DMA_SetConfig(dma_ch, &cfg);

	// FCR : fifo en. tx/rx fifo clr. dma en. set tx/rx fifo lv.
	//gpUart[ch]->fcr = (gpUart[ch]->fcr&0xF0) | 0xF;
	gpUart[ch]->fcr = (level<<6) | (1<<3) | (1<<2) | (1<<1) | (1<<0);

	ret = PPDRV_DMA_Start(dma_ch, u32Timeout);

	return ret;
}

PP_U32 PPDRV_UART_GetDmaDone(PP_U32 IN dma_ch)
{
	return PPDRV_DMA_GetDone(dma_ch, NULL);
}

PP_VOID PPDRV_UART_OutByte (PP_U8 IN ch, PP_U8 IN c)
{
#if 1
	PP_U32 reg = 0;

	do {
		reg = PPDRV_UART_GetTxEmpty(ch);
	} while(!reg);

	PPDRV_UART_SetTxData(ch, c);
#endif
}

PP_U8 PPDRV_UART_InByte (PP_U8 IN ch)
{
	if( !PPDRV_UART_GetRxReady(ch) )
	{
		return 0;
	}

	return PPDRV_UART_GetRxData(ch);
}

PP_VOID PPDRV_UART_SetIER(PP_U8 IN ch, PP_U32 IN mask)
{
	gpUart[ch]->ier = mask;
}

