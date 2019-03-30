#ifndef _PI5008_DMA_H
#define _PI5008_DMA_H

#include "system.h"
#include "type.h"
#include "error.h"

#define DMA_MAX_CHANNEL 	(8)

#define DMA_BSIZE_1            	(0)  // Burst size = 1
#define DMA_BSIZE_2             (1)  // Burst size = 2
#define DMA_BSIZE_4             (2)  // Burst size = 4
#define DMA_BSIZE_8             (3)  // Burst size = 8
#define DMA_BSIZE_16            (4)  // Burst size = 16
#define DMA_BSIZE_32            (5)  // Burst size = 32
#define DMA_BSIZE_64            (6)  // Burst size = 64
#define DMA_BSIZE_128           (7)  // Burst size = 128

#define DMA_WIDTH_BYTE          (0)  // Width = 1 byte
#define DMA_WIDTH_HALFWORD      (1)  // Width = 2 bytes
#define DMA_WIDTH_WORD          (2)  // Width = 4 bytes

#define DMA_EVENT_DONE			(0)
#define DMA_EVENT_ABORT			(1)
#define DMA_EVENT_ERROR			(2)

#define DMA_ADDR_INC			(0)
#define DMA_ADDR_DEC			(1)
#define DMA_ADDR_FIX			(2)


typedef struct tagDMA_CH_REG_T
{
	vuint32 ctrl;							/* 0x044			- Channel0 control register */
	vuint32 src;							/* 0x048			- Channel0 source address register */
	vuint32 dst;							/* 0x04c			- Channel0 destination address register */
	vuint32 size;							/* 0x050			- Channel0 control register */
	vuint32 llp;							/* 0x054			- Channel0 control register */

}DMA_CH_REG_T;

typedef struct tagDMA_REG_T
{
	vuint32 id;								/* 0x000 		- ID and revision register */
	vuint32 rsv0[3];						/* 0x004 ~ 0x00c	- reserved */
	vuint32 cfg;							/* 0x010			- Configuration register */
	vuint32 rsv1[3];						/* 0x014 ~ 0x01c	- reserved */
	vuint32 ctrl;							/* 0x020			- Control register */
	vuint32 rsv2[3];						/* 0x024 ~ 0x02c	- reserved */
	vuint32 irq_status;						/* 0x030			- Interrupt status register */
	vuint32 ch_enable;						/* 0x034			- Channel enable register - RO */
	vuint32 rsv3[2];						/* 0x038 ~ 0x03c	- reserved */
	vuint32 ch_abort;						/* 0x040			- Channel abort register - WO */

	DMA_CH_REG_T ch_reg[DMA_MAX_CHANNEL];	/* 0x044 ~ 0x0E0	- Channel register */

}DMA_REG_T;

enum tagDMA_CHANNEL_E
{
	eDMA_CHANNEL_NUM0 = 0,
	eDMA_CHANNEL_FLASH_TX,
	eDMA_CHANNEL_SPI0_TX,
	eDMA_CHANNEL_SPI0_RX,
	eDMA_CHANNEL_SPI1_TX,
	eDMA_CHANNEL_SPI1_RX,
	eDMA_CHANNEL_UART0,
	eDMA_CHANNEL_UART1,
};

typedef union ppDMA_CTL_U
{
	PP_U32 value;
	struct{
		PP_U32 enable			: 1,	// 0: disable, 1: enable
			int_tcm_mask		: 1,	// 0: allow the terminal count irq to be trigered, 1: disable the terminal count irq
			int_err_mask		: 1,	// 0: allow the error irq to be trigered, 1: disable the error irq
			int_abort_mask		: 1,	// 0: allow the abort irq to be trigered, 1: disable the abort irq
			dst_req_sel			: 4,	// source DMA request select
			src_req_sel			: 4,	// source DMA request select
			dst_addr_ctrl		: 2,	// 0: increment, 1: decrement, 2: fixed address
			src_addr_ctrl		: 2,	// 0: increment, 1: decrement, 2: fixed address
			dst_mode			: 1,	// 0: normal, 1: handshake mode
			src_mode			: 1,	// 0: normal, 1: handshake mode
			dst_width			: 2,	// 0: byte, 1: half, 2: word transfer
			src_width			: 2,	// 0: byte, 1: half, 2: word transfer
			src_burst_size		: 3,	// 0: 1, 1: 2, 2: 4, 3: 8, ... 7: 128 transfer ( 2^x )
			rsv1				: 4,	// reserved
			priority			: 1,	// 0: low, 1: high priority
			rsv2				: 2;	// reserved
	}ctl;
}PP_DMA_CTL_U;

typedef struct ppDMA_CONFIG_S
{
	PP_DMA_CTL_U ctrl;
	PP_U32 u32SrcAddr;
	PP_U32 u32DstAddr;
	PP_U32 u32TrasnferSize;
	PP_U32 u32LLPPoint;
}PP_DMA_CONFIG_S;

typedef void (*DMA_ISR_CALLBACK) (PP_S32 s32Ch, PP_U32 u32Event);

#define DMA_CTRL_REG			((DMA_REG_T*)			(DMA_BASE_ADDR + 0x0000))

PP_VOID PPDRV_DMA_Initialize(PP_VOID);
PP_VOID PPDRV_DMA_SetConfig(PP_S32 IN s32Ch, PP_DMA_CONFIG_S IN *pstCfg);
PP_RESULT_E PPDRV_DMA_Start(PP_S32 IN s32Ch, PP_U32 IN u32Timeout);
PP_RESULT_E PPDRV_DMA_Wait(PP_S32 IN s32Ch, PP_U32 IN u32Timeout);
PP_VOID PPDRV_DMA_Abort(PP_S32 IN s32Ch);
PP_U32 PPDRV_DMA_GetDone(PP_S32 IN s32Ch, PP_RESULT_E OUT *penResult);

PP_RESULT_E PPDRV_DMA_M2M_Word(PP_S32 IN s32Ch, PP_U32 IN *SrcAddr, PP_U32 IN *DstAddr, PP_U32 IN u32Size, PP_U32 IN u32Timeout);

#endif

