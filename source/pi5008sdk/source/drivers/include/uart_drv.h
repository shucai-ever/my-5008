/* uart_drv.h */
#ifndef __UART_DRV_H__
#define __UART_DRV_H__

#include "type.h"
#include "system.h"

#ifdef __cplusplus
EXTERN "C" {
#endif


/***************************************************************************************************************************************************************
 * Declaration
***************************************************************************************************************************************************************/
#define UART_PNT_CH			(0) // for PC Debug
#define UART_PRT_CH			(1) // for MCU communication
#define UART_PRT1_CH		(2) // for communication

#define UART_ID_REV			(0x02011002)

/* HK */
#define UART_BUF_SIZE_MAX	(256)
#define UART_NUM_OF_BUF		(2)

#define FRAME_SOF			(0x79)
#define FRAME_COMMAND_ALL	(0x4)

#define UART_IER_EMSI		(1<<3)
#define UART_IER_ELSI		(1<<2)
#define UART_IER_ETHEI		(1<<1)
#define UART_IER_ERBI		(1<<0)

/***************************************************************************************************************************************************************
 * Enumeration
***************************************************************************************************************************************************************/
typedef enum ppUART_CHANNEL_E {
    eUART_CH_0,		// For Debug
    eUART_CH_1,		// CAN MCU <-> PI5008 CAN Data
    eUART_CH_2,

	eUART_MAX
} PP_UART_CHANNEL_E;

/*
typedef enum ppUART_BAUDRATE_E {
	eBAUDRATE_2400		= 2400,
	eBAUDRATE_4800		= 4800,
	eBAUDRATE_9600		= 9600,
	eBAUDRATE_19200		= 19200,
	eBAUDRATE_38400		= 38400,
	eBAUDRATE_57600		= 57600,
	eBAUDRATE_100000	= 100000,
	eBAUDRATE_115200	= 115200,
	eBAUDRATE_230400	= 230400,
	eBAUDRATE_460800	= 460800,

	eBAUDRATE_MAX		= 999999
} PP_UART_BAUDRATE_E;
*/

typedef enum ppUART_BAUDRATE_E {
	eBAUDRATE_2400= 0,
	eBAUDRATE_4800,
	eBAUDRATE_9600,
	eBAUDRATE_19200,
	eBAUDRATE_38400,
	eBAUDRATE_57600,
	eBAUDRATE_100000,
	eBAUDRATE_115200,
	eBAUDRATE_230400,
	eBAUDRATE_460800,
	eBAUDRATE_MAX
} PP_UART_BAUDRATE_E;

typedef enum ppUART_DATABIT_E {
	eDATA_BIT_5,
	eDATA_BIT_6,
	eDATA_BIT_7,
	eDATA_BIT_8,

	eDATA_BIT_MAX
} PP_UART_DATABIT_E;

typedef enum ppUART_STOP_BIT_E {
	eSTOP_BIT_1,
	eSTOP_BIT_1_5,
	eSTOP_BIT_2,

	eSTOP_BIT_MAX
} PP_UART_STOP_BIT_E;

typedef enum ppUART_PARITY_E {
	ePARITY_NONE,
	ePARITY_ODD,
	ePARITY_EVEN,
	ePARITY_MARK,
	ePARITY_SPACE,

	ePARITY_MAX
} PP_UART_PARITY_E;

/* RXFIFO Trigger Level(FIFO Depth = 16) */
typedef enum ppUART_RX_LEVEL {
	eRX_LEVEL_0,	// Not empty
    eRX_LEVEL_1,	// More than 3
    eRX_LEVEL_2,	// More than 7
    eRX_LEVEL_3,	// More than 13

	eRX_LEVEL_MAX
} PP_UART_RX_LEVEL;

/* TXFIFO Trigger Level(FIFO Depth = 16) */
typedef enum ppUART_TX_LEVEL {
	eTX_LEVEL_0,	// Not full
    eTX_LEVEL_1,	// Less than 12
    eTX_LEVEL_2,	// Less than 8
    eTX_LEVEL_3,	// Less than 4
    
    eTX_LEVEL_MAX
} PP_UART_TX_LEVEL;


/* HK */
typedef enum ppUartPacketHeaderCommandInfo_E
{
    eUART_COMMAND_ALL = 0x4,
    eUART_COMMAND_MAX
} PP_UartPacketHeaderCommandInfo_E;

typedef enum ppUartPacketHeaderFrameInfo_E
{
    eUART_HEADER_SOF = 0,
    eUART_HEADER_COMMAND,
    eUART_HEADER_LENGTH,
    eUART_HEADER_END
} PP_UartPacketHeaderFrameInfo_E;


/***************************************************************************************************************************************************************
 * Structure
***************************************************************************************************************************************************************/

/* HK */
typedef struct ppUartData_S
{
    PP_U8 status;
    PP_U8 data[UART_BUF_SIZE_MAX+5];
    PP_U16 dataLength;
    PP_U16 dataCount;
} PP_UartData_S;

typedef struct ppUartChannel_S
{
    PP_UartData_S tx;
    
    PP_UartData_S rx[UART_NUM_OF_BUF]; // Dual buffering
    PP_U8 rxBufferIndex;
} PP_UartChannel_S;

typedef struct ppUartFifoOpt_S
{
    PP_U8 txEnable;
    PP_U8 rxEnable;
    PP_U8 txLevel;
    PP_U8 rxLevel;
} PP_UartFifoOpt_S;

typedef struct ppUartQueueMsg_S
{
    PP_U16 length;
    PP_U8 *buffer;
} PP_UartQueueMsg_S;


/***************************************************************************************************************************************************************
 * Function
***************************************************************************************************************************************************************/
PP_VOID PPDRV_UART_SWreset (PP_VOID);
PP_VOID PPDRV_UART_Init (PP_UART_CHANNEL_E IN ch, PP_UART_BAUDRATE_E IN baudrate, PP_UART_DATABIT_E IN databit, PP_UART_STOP_BIT_E IN stopbit, PP_UART_PARITY_E IN parity);
PP_VOID PPDRV_UART_InitInterrupt (PP_UART_CHANNEL_E IN ch, PP_UartFifoOpt_S IN opt);
PP_VOID PPDRV_UART_OutByte (PP_U8 IN ch, PP_U8 IN c);
PP_U8 PPDRV_UART_InByte (PP_U8 IN ch);
PP_U32 PPDRV_UART_GetID( PP_U32 IN ch);
PP_U8 PPDRV_UART_GetRxReady (PP_U8 IN ch);
PP_U8 PPDRV_UART_GetRxData (PP_U8 IN ch);
PP_U8 PPDRV_UART_GetTxEmpty (PP_U8 IN ch);
PP_VOID PPDRV_UART_SetTxData (PP_U8 IN ch, PP_U8 IN data);
PP_VOID PPDRV_UART_EnableLoopback (PP_U8 IN ch, PP_BOOL IN enable);
PP_RESULT_E PPDRV_UART_SetDmaTx (PP_UART_CHANNEL_E IN ch, PP_UART_TX_LEVEL IN level, CONST PP_U8* IN din, PP_U32 IN size, PP_U32 IN dma_ch, PP_U32 u32Timeout);
PP_RESULT_E PPDRV_UART_SetDmaRx(PP_UART_CHANNEL_E IN ch, PP_UART_RX_LEVEL IN level, PP_U8* OUT dout, PP_U32 IN size, PP_U32 IN dma_ch, PP_U32 u32Timeout);
PP_U32 PPDRV_UART_GetDmaDone(PP_U32 IN dma_ch);
PP_VOID PPDRV_UART_SetIER(PP_U8 IN ch, PP_U32 IN mask);
#ifdef __cplusplus
}
#endif

#endif // __UART_DRV_H__
