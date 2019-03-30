#ifndef _PI5008_SPI_H
#define _PI5008_SPI_H

#include "system.h"
#include "type.h"
#include "error.h"


//#define	SPI_CPHA	0x01			/* clock phase */
//#define	SPI_CPOL	0x02			/* clock polarity */

#define SPI_CH_1            (1)
#define SPI_CH_2            (2)
#define SPI_CH_3            (3)
#define SPI_CH_4            (4)
#define SPI_CH_0            (0)



typedef struct tagSPI_REG_T
{
	vuint32 id;								/* 0x000 		- ID and revision register */
	vuint32 rsv0[3];						/* 0x004 ~ 0x00c	- reserved */
	vuint32 trans_fmt;						/* 0x010 		- Transfer format register */
	vuint32 direct_io;						/* 0x014 		- Direct IO control register */
	vuint32 rsv1[2];						/* 0x018 ~ 0x01c	- reserved */
	vuint32 trans_ctrl;						/* 0x020 		- Transfer control register */
	vuint32 cmd;							/* 0x024 		- Command register */
	vuint32 addr;							/* 0x028 		- Address register */
	vuint32 data;							/* 0x02c 		- Data register */
	vuint32 ctrl;							/* 0x030 		- Control register */
	vuint32 status;							/* 0x034 		- Status register */
	vuint32 irq_enable;						/* 0x038 		- Interrupt enable register */
	vuint32 irq_status;						/* 0x03c 		- Interrupt status register */
	vuint32 timing;							/* 0x040 		- Interface timing register */
	vuint32 rsv2[3];						/* 0x044 ~ 0x04c	- reserved */
	vuint32 mem_ctrl;						/* 0x050 		- Memory access control register */
	vuint32 rsv3[3];						/* 0x054 ~ 0x05c	- reserved */
	vuint32 slave_status;					/* 0x060 		- Slave status register */
	vuint32 slave_data_cnt;					/* 0x064 		- Slave data count register */
	vuint32 rsv4[5];						/* 0x068 ~ 0x078	- reserved */
	vuint32 config;							/* 0x07c 		- Configuration register */
}SPI_REG_T;



typedef enum tagSPI_CHANNEL_E
{
	eSPI_CHANNEL_0 = 0,
	eSPI_CHANNEL_1,
	eSPI_CHANNEL_2,
//	eSPI_CHANNEL_3,
	eSPI_CHANNEL_FLASH,
} SPI_CHANNEL_E;


#define SPI_FLASH_CTRL_REG			((SPI_REG_T*)			(SPI_FLASH_BASE_ADDR))
#if defined(SYSTEM_BUS_DW)
#define SPI0_CTRL_REG			((SPI_REG_T*)			(SPI0_BASE_ADDR))
#define SPI1_CTRL_REG			((SPI_REG_T*)			(SPI1_BASE_ADDR))
#define SPI2_CTRL_REG			((SPI_REG_T*)			(SPI2_BASE_ADDR))
//#define SPI3_CTRL_REG			((SPI_REG_T*)			(SPI3_BASE_ADDR))
#endif



/*
#define SPI0_CTRL_REG			((SPI_REG_T*)			(SPI0_BASE_ADDR))
#if defined(SYSTEM_BUS_DW)
#define SPI1_CTRL_REG			((SPI_REG_T*)			(SPI1_BASE_ADDR))
#define SPI2_CTRL_REG			((SPI_REG_T*)			(SPI2_BASE_ADDR))
#define SPI3_CTRL_REG			((SPI_REG_T*)			(SPI3_BASE_ADDR))
#define SPI4_CTRL_REG			((SPI_REG_T*)			(SPI4_BASE_ADDR))
#endif
*/
#define SPI_MEM_CTRL_REG		((SPI_MEM_REG_T*)		(QSPI_BASE_ADDR))

#define SPI_INTR_RXFIFO_OVER        (1UL << 0)
#define SPI_INTR_TXFIFO_UNDER       (1UL << 1)
#define SPI_INTR_RXFIFO_THRD        (1UL << 2)
#define SPI_INTR_TXFIFO_THRD		(1UL << 3)
#define SPI_INTR_TRANSFER_END       (1UL << 4)
#define SPI_INTR_RECV_CMD           (1UL << 5)

#define SPIDRV_GET_TX_FIFO_NUM(CH)			(((*(vuint32 *)(SPI0_BASE_ADDR + 0x1000*CH + 0x34))>>16)&0x1f)
#define SPIDRV_IS_TX_FIFO_FULL(CH)			(((*(vuint32 *)(SPI0_BASE_ADDR + 0x1000*CH + 0x34))>>23)&0x1)
#define SPIDRV_PUT_TX_FIFO(CH, DATA)		(*(vuint32 *)(SPI0_BASE_ADDR + 0x1000*CH + 0x2C) = DATA)

#define SPIDRV_GET_RX_FIFO_NUM(CH)			(((*(vuint32 *)(SPI0_BASE_ADDR + 0x1000*CH + 0x34))>>8)&0x1f)
#define SPIDRV_IS_RX_FIFO_EMPTY(CH)			(((*(vuint32 *)(SPI0_BASE_ADDR + 0x1000*CH + 0x34))>>14)&1)
#define SPIDRV_GET_RX_FIFO(CH)				*(vuint32 *)(SPI0_BASE_ADDR + 0x1000*CH + 0x2C)

typedef void (*SPI_ISR_CALLBACK) (PP_U32 IN u32Status);

PP_RESULT_E PPDRV_SPI_Initialize(PP_S32 IN s32Ch, PP_U32 IN u32IsSlave, PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U32 IN u32WordLen);
PP_VOID PPDRV_SPI_IRQEnable(PP_S32 s32Ch, PP_U32 u32Irq);
PP_VOID PPDRV_SPI_IRQClear(PP_S32 s32Ch, PP_U32 u32Irq);
PP_U32 PPDRV_SPI_GetFreq(PP_S32 s32Ch);
PP_VOID PPDRV_SPI_SetISR(PP_S32 IN s32Ch, SPI_ISR_CALLBACK IN cbISR);

PP_RESULT_E PPDRV_SPI_Tx(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SPI_Rx(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SPI_TxRx(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size);

PP_RESULT_E PPDRV_SPI_TxDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_SPI_RxDMA(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_SPI_TxRxDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMATxCh, PP_S32 IN s32DMARxCh, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_SPI_DMAGetDone(PP_S32 IN s32Ch, PP_S32 IN s32DMACh);
PP_RESULT_E PPDRV_SPI_DMAWait(PP_S32 IN s32Ch, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_SPI_TxRx_DMAWait(PP_S32 IN s32Ch, PP_S32 IN s32DMATxCh, PP_S32 IN s32DMARxCh, PP_U32 IN u32Timeout);

PP_VOID PPDRV_SPI_GPIOCSEnable(PP_S32 IN s32Ch, PP_U8 u8Enable);
PP_VOID PPDRV_SPI_CSActivate(PP_S32 IN s32Ch);
PP_VOID PPDRV_SPI_CSDeActivate(PP_S32 IN s32Ch);
PP_S32 PPDRV_SPI_CSGetLevel(PP_S32 IN s32Ch);

PP_RESULT_E PPDRV_SPI_TxSlave(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size);
PP_RESULT_E PPDRV_SPI_RxSlave(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size);
PP_U8 PPDRV_SPI_RxSlave_Byte(PP_S32 IN ch);

PP_RESULT_E PPDRV_SPI_TxSlaveDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_SPI_RxSlaveDMA(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_SPI_TxRxSlaveDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMATxCh, PP_S32 IN s32DMARxCh, PP_U32 IN u32Timeout);

PP_VOID PPDRV_SPI_Wait(PP_S32 IN s32Ch);
PP_VOID PPDRV_SPI_BufferClear(PP_S32 IN s32Ch);

PP_RESULT_E PPDRV_SPI_SetTxSlaveIntr(PP_S32 IN s32Ch);
PP_RESULT_E PPDRV_SPI_SetRxSlaveIntr(PP_S32 IN s32Ch);

PP_VOID PPDRV_SPI_SetSlaveReady(PP_S32 IN s32Ch);
PP_VOID PPDRV_SPI_SetSlaveStatus(PP_S32 IN s32Ch, PP_U16 IN u16Status);
PP_U16 PPDRV_SPI_GetSlaveStatus(PP_S32 IN s32Ch);

#endif
