
#include <stdio.h>
#include <nds32_intrinsic.h>

#include "osal.h"
#include "utils.h"
#include "spi.h"
#include "dma.h"
#include "debug.h"
#include "pi5008.h"
#include "interrupt.h"
#include "clock.h"
#include "gpio.h"
#include "pinmux.h"

#if MCU_CTRL_METHOD
#include "task_manager.h"
#endif

#define TRANSFER_ISR				0

#define SPI_TCR_WR           	(0x0 << 24)
#define SPI_TCR_WONLY          	(0x1 << 24)
#define SPI_TCR_RONLY          	(0x2 << 24)

#if defined(SYSTEM_BUS_DW)
#define MAX_SPI_DEV_NUM				4
#else
#define MAX_SPI_DEV_NUM				1
#endif

#define TX_INTR_FIFO_THRD			6
#define RX_INTR_FIFO_THRD			2

#define TX_DMA_FIFO_THRD			4
#define RX_DMA_FIFO_THRD			4

#if defined(TRANSFER_ISR)
typedef struct tagSPI_RESOURCE_T{
	uint32 *pTxBuf;
	uint32 *pRxBuf;
	uint32 TxSize;
	uint32 RxSize;
}SPI_RESOURCE_T;

#endif


typedef struct tagGPIO_PIN_CFG_T{
	uint32 port;
	uint32 pin;
	uint32 pinmux_group;
	uint32 pinmux_shift;
	uint32 pinmux_old;
	uint32 pinmux_sel;

}GPIO_PIN_CFG_T;


static SPI_REG_T *gpSPI[] = {
#if defined(SYSTEM_BUS_DW)
		SPI0_CTRL_REG,
		SPI1_CTRL_REG,
		SPI2_CTRL_REG,
#endif
		SPI_FLASH_CTRL_REG,
};

static SPI_ISR_CALLBACK gSPICallback[MAX_SPI_DEV_NUM];

static uint32 gDMARequestTx[MAX_SPI_DEV_NUM] = {
#if defined(SYSTEM_BUS_DW)
		SPI0_DMA_TX_REQ,
		SPI1_DMA_TX_REQ,
		SPI2_DMA_TX_REQ,
#endif
		SPI_FLASH_DMA_TX_REQ
};
static uint32 gDMARequestRx[MAX_SPI_DEV_NUM] = {
#if defined(SYSTEM_BUS_DW)
		SPI0_DMA_RX_REQ,
		SPI1_DMA_RX_REQ,
		SPI2_DMA_RX_REQ,
#endif
		SPI_FLASH_DMA_RX_REQ
};

//static vuint32 *gMISC_CS_PINMUX = (vuint32 *)(MISC_BASE_ADDR + 0xf8);
//static sint8 gCS_GPIOCh[MAX_SPI_DEV_NUM] = {3, 4, 5, 7 };
static GPIO_PIN_CFG_T gCS_GPIOCh[MAX_SPI_DEV_NUM] = {
		{
				.port = 0,
				.pin = 26,
				.pinmux_group = 1,
				.pinmux_shift = 20,
				.pinmux_old = 0,
				.pinmux_sel = PINMUX_1_GPIO0_26,

		},
		{
				.port = 0,
				.pin = 29,
				.pinmux_group = 1,
				.pinmux_shift = 26,
				.pinmux_old = 0,
				.pinmux_sel = PINMUX_1_GPIO0_29,

		},
		{
				.port = 0,
				.pin = 27,
				.pinmux_group = 1,
				.pinmux_shift = 22,
				.pinmux_old = 0,
				.pinmux_sel = PINMUX_1_GPIO0_27,

		},
		{
				.port = 1,
				.pin = 1,
				.pinmux_group = 2,
				.pinmux_shift = 2,
				.pinmux_old = 0,
				.pinmux_sel = PINMUX_2_GPIO1_1,

		},

};

static uint32 u32TxDMABurstSize[9] = {
		DMA_BSIZE_4,	// THRD Level 0 - N/A
		DMA_BSIZE_4,	// THRD Level 1
		DMA_BSIZE_4,	// THRD Level 2
		DMA_BSIZE_4,	// THRD Level 3
		DMA_BSIZE_4,	// THRD Level 4
		DMA_BSIZE_1,	// THRD Level 5
		DMA_BSIZE_1,	// THRD Level 6
		DMA_BSIZE_1,	// THRD Level 7
		DMA_BSIZE_1,	// THRD Level 8 - N/A
};

static uint32 u32RxDMABurstSize[9] = {
		DMA_BSIZE_1,	// THRD Level 0 - N/A
		DMA_BSIZE_1,	// THRD Level 1
		DMA_BSIZE_1,	// THRD Level 2
		DMA_BSIZE_1,	// THRD Level 3
		DMA_BSIZE_4,	// THRD Level 4
		DMA_BSIZE_4,	// THRD Level 5
		DMA_BSIZE_4,	// THRD Level 6
		DMA_BSIZE_4,	// THRD Level 7
		DMA_BSIZE_4,	// THRD Level 8 - N/A
};
uint8 spi_buf[255];
uint8 spi_count;
static SemaphoreHandle_t ghDMALock[MAX_SPI_DEV_NUM];
static uint8 gTransferDMAFlag[MAX_SPI_DEV_NUM];


PP_U8 spi_isr_buf[1024];
PP_U32 rx_index = 0;

#if MCU_CTRL_METHOD

void spi_interrupt_process(sint32 dev)
{
	//LOG_DEBUG("spi_interrupt_handle\n");
	int fifo_num = ((gpSPI[dev]->status >> 8)&0x1f);

	while( fifo_num )
	{
		spi_isr_buf[rx_index++] = PPDRV_SPI_RxSlave_Byte(dev);

		fifo_num--;
	}

#if 0 // Interrupt service performance test...
	int i = 0;

	for( i = 0;i<15;i++ )
		memcpy(test_buf, spi_isr_buf, 1024);
#endif

	if( rx_index > 11 )
	{
		AppTask_SendCmdFromISR(CMD_GET_REMOCON, TASK_MONITOR, TASK_MONITOR, 0, spi_isr_buf, rx_index);
		memset(spi_isr_buf, 0, sizeof(spi_isr_buf));
		rx_index = 0;
	}
}
#endif //MCU_CTRL_METHOD

void spi1_isr()
{
    memcpy((void *)&spi_buf[spi_count], (void *)gpSPI[eSPI_CHANNEL_1]->data, gpSPI[eSPI_CHANNEL_1]->slave_data_cnt);
    spi_count += gpSPI[eSPI_CHANNEL_1]->slave_data_cnt;

    

}

ISR(spi_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	sint32 dev = 0;
	BaseType_t xHigherPriorityTaskWoken;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

    if(IRQ_SPI_STATUS_MASK(0) == FLAG_SET)			// ch0 - sdio
    {
		dev = 0;
    }
    else if(IRQ_SPI_STATUS_MASK(1) == FLAG_SET)		// ch1 - can micom
    {
		dev = 1;
    }
    else if(IRQ_SPI_STATUS_MASK(2) == FLAG_SET)
    {
		dev = 2;
    }
    else if(IRQ_SPI_STATUS_MASK(3) == FLAG_SET)
    {
		dev = 3;
    }

    reg = gpSPI[dev]->irq_status;

    if(reg & SPI_INTR_TRANSFER_END){
    	if(gTransferDMAFlag[dev] == 1){
            //release semaphore.
            xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(ghDMALock[dev], &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR();
            }

    		gTransferDMAFlag[dev] = 0;
    	}
    	//printf("spi isr transfer done\n");
    }

    if(reg & SPI_INTR_RXFIFO_THRD){
    	//printf("rxfifo irq. spi status: 0x%x, rx fifo num: %d\n", gpSPI[dev]->status, ((gpSPI[dev]->status >> 8)&0x1f));
    }

    if(reg & SPI_INTR_RXFIFO_OVER){
    	PPDRV_SPI_BufferClear( dev );
    }

#if MCU_CTRL_METHOD
    if( dev == 1 )
    	spi_interrupt_process( dev );
#endif

    if(gSPICallback[dev])gSPICallback[dev](reg);



	gpSPI[dev]->irq_status = reg;

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);

}

ISR(spi_flash_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	BaseType_t xHigherPriorityTaskWoken;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	reg = gpSPI[eSPI_CHANNEL_FLASH]->irq_status;

	if(reg & SPI_INTR_TRANSFER_END){
    	if(gTransferDMAFlag[eSPI_CHANNEL_FLASH] == 1){
            //release semaphore.
            xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(ghDMALock[eSPI_CHANNEL_FLASH], &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR();
            }

    		gTransferDMAFlag[eSPI_CHANNEL_FLASH] = 0;
    	}
		//printf("spi isr transfer done\n");
	}

    if(gSPICallback[eSPI_CHANNEL_FLASH])gSPICallback[eSPI_CHANNEL_FLASH](reg);


	gpSPI[eSPI_CHANNEL_FLASH]->irq_status = reg;

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);

}

PP_RESULT_E PPDRV_SPI_Initialize(PP_S32 IN s32Ch, PP_U32 IN u32IsSlave, PP_U32 IN u32Freq, PP_U32 IN u32ClkMode, PP_U32 IN u32WordLen)
{
	PP_RESULT_E ret = eSUCCESS;
	uint32 div;
    sys_os_isr_t *old = NULL;


    /* Register SPI ISR */
    if(s32Ch < eSPI_CHANNEL_FLASH){
		OSAL_register_isr(IRQ_SPI_VECTOR, spi_isr, old);
		INTC_irq_config(IRQ_SPI_VECTOR, IRQ_LEVEL_TRIGGER);
		INTC_irq_clean(IRQ_SPI_VECTOR);
		INTC_irq_enable(IRQ_SPI_VECTOR);
    }else{
		OSAL_register_isr(IRQ_SPI_FLASH_VECTOR, spi_flash_isr, old);
		INTC_irq_clean(IRQ_SPI_FLASH_VECTOR);
		INTC_irq_enable(IRQ_SPI_FLASH_VECTOR);
    }

    gSPICallback[s32Ch] = PP_NULL;

	gpSPI[s32Ch]->ctrl = 7;	// Tx/Rx FIFO reset, SPI reset

	while(gpSPI[s32Ch]->ctrl & 7);
	
	gpSPI[s32Ch]->trans_fmt = ( (((u32WordLen-1)&0x1f) << 8) | ((u32IsSlave & 1)<<2) | (u32ClkMode&3) );

    gpSPI[s32Ch]->ctrl = ( (TX_INTR_FIFO_THRD << 16) |	(RX_INTR_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold

    if( !u32IsSlave )
    {
    	if((u32Freq*2) > u32APBClk)
    		div = 0;
    	else
    		div = (u32APBClk / (u32Freq * 2)) - 1;

        gpSPI[s32Ch]->timing = utilPutBits(gpSPI[s32Ch]->timing, 0, 8, div);

    }else{
    	gpSPI[s32Ch]->slave_status &= (~0x1ffff);	// slave ready & status clear
    }


    if(ghDMALock[s32Ch] == NULL){
		if( (ghDMALock[s32Ch] = xSemaphoreCreateBinary()) == NULL)
		{
			LOG_DEBUG("ERROR! can't create lockDMA\n");
			return eERROR_FAILURE;
		}
    }

	gTransferDMAFlag[s32Ch] = 0;

	return ret;
}

PP_VOID PPDRV_SPI_SetISR(PP_S32 IN s32Ch, SPI_ISR_CALLBACK IN cbISR)
{
	gSPICallback[s32Ch] = cbISR;
}

PP_RESULT_E PPDRV_SPI_Tx(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size)
{
	PP_RESULT_E ret = eSUCCESS;

	sint32 len_tx;
	sint32 n_bytes;
	uint32 data;

	
	n_bytes = ( utilGetBits(gpSPI[s32Ch]->trans_fmt, 8, 5) + 1 + 7 ) / 8;
	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WONLY | (((u32Size-1)&0x1ff)<<12));

	if( utilGetBit(gpSPI[s32Ch]->trans_fmt, 2) != 1)
		gpSPI[s32Ch]->cmd = 0;	// start transfer
	
	for(len_tx=0; len_tx < u32Size;){
		if(len_tx < u32Size && !(utilGetBit(gpSPI[s32Ch]->status,23))){	// Tx not full
			memcpy(&data, pu8DOut, n_bytes);
			gpSPI[s32Ch]->data = data;
			pu8DOut += n_bytes;
			len_tx++;
		}
	}

	while(gpSPI[s32Ch]->status & 1);

	return ret;

}

PP_RESULT_E PPDRV_SPI_Rx(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size)
{
	PP_RESULT_E ret = eSUCCESS;

	sint32 len_rx;
	vuint32 val;
	sint32 n_bytes;
	uint32 data;

	n_bytes = ( utilGetBits(gpSPI[s32Ch]->trans_fmt, 8, 5) + 1 + 7 ) / 8;
	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_RONLY | (((u32Size-1)&0x1ff)<<0));

	if( utilGetBit(gpSPI[s32Ch]->trans_fmt, 2) != 1)
		gpSPI[s32Ch]->cmd = 0;	// start transfer

	for(len_rx=0; len_rx < u32Size;){
		if(!utilGetBit(gpSPI[s32Ch]->status, 14)){	// Rx not empty
			val = gpSPI[s32Ch]->data;
			data = (uint8)val;
			memcpy(pu8DIn, &data, n_bytes);
			pu8DIn += n_bytes;
			len_rx++;
		}

	}

	while(gpSPI[s32Ch]->status & 1);

	return ret;
}


PP_RESULT_E PPDRV_SPI_TxRx(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size)
{
	PP_RESULT_E ret = eSUCCESS;

	sint32 len_tx, len_rx;
	vuint32 val;
	sint32 n_bytes;
	uint32 data;
	
	n_bytes = ( utilGetBits(gpSPI[s32Ch]->trans_fmt, 8, 5) + 1 + 7 ) / 8;
	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WR | (((u32Size-1)&0x1ff)<<12) | (((u32Size-1)&0x1ff)<<0));

	gpSPI[s32Ch]->cmd = 0;	// start transfer
	
	for(len_tx=0, len_rx=0; len_rx < u32Size;){
		if(len_tx < u32Size && !(utilGetBit(gpSPI[s32Ch]->status,23))){	// Tx not full
			if(pu8DOut){
				memcpy(&data, pu8DOut, n_bytes);
				gpSPI[s32Ch]->data = data;
				pu8DOut += n_bytes;
			}else{
				gpSPI[s32Ch]->data = 0;
			}
			len_tx++;
		}

		if(!utilGetBit(gpSPI[s32Ch]->status, 14)){	// Rx not empty
			val = gpSPI[s32Ch]->data;
			if (pu8DIn){
				data = (uint8)val;
				memcpy(pu8DIn, &data, n_bytes);
				pu8DIn += n_bytes;
			}
			len_rx++;
		}

	}

	while(gpSPI[s32Ch]->status & 1);

	return ret;
}

PP_RESULT_E PPDRV_SPI_TxDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size, PP_S32 s32DMACh, PP_U32 u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;
	PP_DMA_CONFIG_S cfg;
	sint32 n_bytes;
	uint32 width;

	if(gDMARequestTx[s32Ch] == (uint32)(-1))return eERROR_FAILURE;

	gTransferDMAFlag[s32Ch] = 1;

	n_bytes = ( utilGetBits(gpSPI[s32Ch]->trans_fmt, 8, 5) + 1 + 7 ) / 8;
	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WONLY | (((u32Size-1)&0x1ff)<<12));


	gpSPI[s32Ch]->ctrl |= (1<<4);	// DMA Tx Enable
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_DMA_FIFO_THRD << 16) |	(RX_DMA_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold

	gpSPI[s32Ch]->irq_enable = SPI_INTR_TRANSFER_END;

	u32Size = u32Size * n_bytes;
	//size = 1;

	if(n_bytes == 1)width = DMA_WIDTH_BYTE;
	else if(n_bytes == 2)width = DMA_WIDTH_HALFWORD;
	else width = DMA_WIDTH_WORD;

	memset(&cfg.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg.ctrl.ctl.priority = 0;
	//cfg.ctrl.ctl.src_burst_size = u32TxDMABurstSize[TX_DMA_FIFO_THRD];
	cfg.ctrl.ctl.src_burst_size = DMA_BSIZE_2;

	cfg.ctrl.ctl.src_width = width;
	cfg.ctrl.ctl.dst_width = width;
	cfg.ctrl.ctl.src_mode = 0;		// normal
	cfg.ctrl.ctl.dst_mode = 1;		// handshake
	cfg.ctrl.ctl.src_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.dst_addr_ctrl = 2;	// fixed
	cfg.ctrl.ctl.src_req_sel = 0;
	cfg.ctrl.ctl.dst_req_sel = gDMARequestTx[s32Ch];	// spi request ??????
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;
	cfg.u32SrcAddr = (uint32)(pu8DOut);
	cfg.u32DstAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg.u32TrasnferSize = (u32Size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;

	PPDRV_DMA_SetConfig(s32DMACh, &cfg);

	PPDRV_DMA_Start(s32DMACh, 0);
	gpSPI[s32Ch]->cmd = 0;	// start transfer
	if(PPDRV_DMA_Wait(s32DMACh, u32Timeout)){
		goto ERROR;
	}

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

	return ret;

ERROR:
	LOG_CRITICAL("ERROR! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;

}

PP_RESULT_E PPDRV_SPI_RxDMA(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 s32DMACh, PP_U32 u32Timeout)
{
	sint32 ret = 0;
	PP_DMA_CONFIG_S cfg;
	sint32 n_bytes;
	uint32 width;

	if(gDMARequestRx[s32Ch] == (uint32)(-1))return eERROR_FAILURE;
	gTransferDMAFlag[s32Ch] = 1;

	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_RONLY | (((u32Size-1)&0x1ff)<<0));
	gpSPI[s32Ch]->ctrl |= (1<<3);	// DMA Rx Enable
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_DMA_FIFO_THRD << 16) |	(RX_DMA_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold

	gpSPI[s32Ch]->irq_enable = SPI_INTR_TRANSFER_END;

	n_bytes = ( utilGetBits(gpSPI[s32Ch]->trans_fmt, 8, 5) + 1 + 7 ) / 8;
	u32Size = u32Size * n_bytes;

	if(n_bytes == 1)width = DMA_WIDTH_BYTE;
	else if(n_bytes == 2)width = DMA_WIDTH_HALFWORD;
	else width = DMA_WIDTH_WORD;

	memset(&cfg.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg.ctrl.ctl.priority = 0;
	//cfg.ctrl.ctl.src_burst_size = u32RxDMABurstSize[RX_DMA_FIFO_THRD];
	cfg.ctrl.ctl.src_burst_size = DMA_BSIZE_2;
	cfg.ctrl.ctl.src_width = width;
	cfg.ctrl.ctl.dst_width = width;
	cfg.ctrl.ctl.src_mode = 1;		// handshake
	cfg.ctrl.ctl.dst_mode = 0;		// normal
	cfg.ctrl.ctl.src_addr_ctrl = 2;	// fixed
	cfg.ctrl.ctl.dst_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.src_req_sel = gDMARequestRx[s32Ch];
	cfg.ctrl.ctl.dst_req_sel = 0;
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;
	cfg.u32SrcAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg.u32DstAddr = (uint32)(pu8DIn);
	cfg.u32TrasnferSize = (u32Size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;

	PPDRV_DMA_SetConfig(s32DMACh, &cfg);

	PPDRV_DMA_Start(s32DMACh, 0);
	gpSPI[s32Ch]->cmd = 0;	// start transfer

	if(PPDRV_DMA_Wait(s32DMACh, u32Timeout)){
		goto ERROR;
	}

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }
	return ret;

ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;
}

PP_RESULT_E PPDRV_SPI_TxRxDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMATxCh, PP_S32 IN s32DMARxCh, PP_U32 IN u32Timeout)
{
	sint32 ret = 0;

	PP_DMA_CONFIG_S cfg_tx;
	PP_DMA_CONFIG_S cfg_rx;
	sint32 n_bytes;
	uint32 width;

	if((gDMARequestRx[s32Ch] == (uint32)(-1)) || (gDMARequestTx[s32Ch] == (uint32)(-1)))return eERROR_FAILURE;
	gTransferDMAFlag[s32Ch] = 1;

	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WR | (((u32Size-1)&0x1ff)<<12) | (((u32Size-1)&0x1ff)<<0));
	gpSPI[s32Ch]->ctrl |= ( (1<<4) | (1<<3) );	// DMA Tx/Rx Enable
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_DMA_FIFO_THRD << 16) |	(RX_DMA_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold
	gpSPI[s32Ch]->irq_enable = SPI_INTR_TRANSFER_END;

	n_bytes = ( utilGetBits(gpSPI[s32Ch]->trans_fmt, 8, 5) + 1 + 7 ) / 8;
	u32Size = u32Size * n_bytes;

	if(n_bytes == 1)width = DMA_WIDTH_BYTE;
	else if(n_bytes == 2)width = DMA_WIDTH_HALFWORD;
	else width = DMA_WIDTH_WORD;

	memset(&cfg_tx.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg_tx.ctrl.ctl.priority = 0;
	//cfg_tx.ctrl.ctl.src_burst_size = u32TxDMABurstSize[TX_DMA_FIFO_THRD];
	cfg_tx.ctrl.ctl.src_burst_size = DMA_BSIZE_2;
	cfg_tx.ctrl.ctl.src_width = width;
	cfg_tx.ctrl.ctl.dst_width = width;
	cfg_tx.ctrl.ctl.src_mode = 0;		// normal
	cfg_tx.ctrl.ctl.dst_mode = 1;		// handshake
	cfg_tx.ctrl.ctl.src_addr_ctrl = 0;	// increment
	cfg_tx.ctrl.ctl.dst_addr_ctrl = 2;	// fixed
	cfg_tx.ctrl.ctl.src_req_sel = 0;
	cfg_tx.ctrl.ctl.dst_req_sel = gDMARequestTx[s32Ch];
	cfg_tx.ctrl.ctl.int_abort_mask = 0;
	cfg_tx.ctrl.ctl.int_err_mask = 0;
	cfg_tx.ctrl.ctl.int_tcm_mask = 0;
	cfg_tx.u32SrcAddr = (uint32)(pu8DOut);
	cfg_tx.u32DstAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg_tx.u32TrasnferSize = (u32Size >> cfg_tx.ctrl.ctl.src_width);
	cfg_tx.u32LLPPoint = 0;

	memset(&cfg_rx.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg_rx.ctrl.ctl.priority = 0;
	//cfg_rx.ctrl.ctl.src_burst_size = u32TxDMABurstSize[TX_DMA_FIFO_THRD];
	cfg_rx.ctrl.ctl.src_burst_size = DMA_BSIZE_2;
	cfg_rx.ctrl.ctl.src_width = width;
	cfg_rx.ctrl.ctl.dst_width = width;
	cfg_rx.ctrl.ctl.src_mode = 1;		// handshake
	cfg_rx.ctrl.ctl.dst_mode = 0;		// normal
	cfg_rx.ctrl.ctl.src_addr_ctrl = 2;	// fixed
	cfg_rx.ctrl.ctl.dst_addr_ctrl = 0;	// increment
	cfg_rx.ctrl.ctl.src_req_sel = gDMARequestRx[s32Ch];
	cfg_rx.ctrl.ctl.dst_req_sel = 0;
	cfg_rx.ctrl.ctl.int_abort_mask = 0;
	cfg_rx.ctrl.ctl.int_err_mask = 0;
	cfg_rx.ctrl.ctl.int_tcm_mask = 0;
	cfg_rx.u32SrcAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg_rx.u32DstAddr = (uint32)(pu8DIn);
	cfg_rx.u32TrasnferSize = (u32Size >> cfg_rx.ctrl.ctl.src_width);
	cfg_rx.u32LLPPoint = 0;


	PPDRV_DMA_SetConfig(s32DMATxCh, &cfg_tx);
	PPDRV_DMA_SetConfig(s32DMARxCh, &cfg_rx);

	PPDRV_DMA_Start(s32DMATxCh, 0);
	PPDRV_DMA_Start(s32DMARxCh, 0);

	gpSPI[s32Ch]->cmd = 0;	// start transfer

	if(PPDRV_DMA_Wait(s32DMATxCh, u32Timeout)){
		PPDRV_DMA_Wait(s32DMARxCh, u32Timeout);
		goto ERROR;
	}
	if(PPDRV_DMA_Wait(s32DMARxCh, u32Timeout)){
		goto ERROR;
	}

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

	return ret;

ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;
}

PP_RESULT_E PPDRV_SPI_DMAWait(PP_S32 IN s32Ch, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;

	if(PPDRV_DMA_Wait(s32DMACh, u32Timeout)){
		goto ERROR;
	}

	if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

    return ret;

ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;
}
PP_RESULT_E PPDRV_SPI_TxRx_DMAWait(PP_S32 IN s32Ch, PP_S32 IN s32DMATxCh, PP_S32 IN s32DMARxCh, PP_U32 IN u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;


	if(PPDRV_DMA_Wait(s32DMATxCh, u32Timeout)){
		PPDRV_DMA_Wait(s32DMARxCh, u32Timeout);
		goto ERROR;
	}
	if(PPDRV_DMA_Wait(s32DMARxCh, u32Timeout)){
		goto ERROR;
	}

	if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

    return ret;

ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;
}


// deprecated function
PP_RESULT_E PPDRV_SPI_DMAGetDone(PP_S32 IN s32Ch, PP_S32 IN s32DMACh)
{
	vuint32 reg;
	PP_RESULT_E ret = eSUCCESS;

	reg = gpSPI[s32Ch]->status;

	if(( reg & 1) == 0 && PPDRV_DMA_GetDone(s32DMACh, NULL))ret = eSUCCESS;
	else ret = eERROR_FAILURE;

	return ret;
}

PP_VOID PPDRV_SPI_GPIOCSEnable(PP_S32 IN s32Ch, PP_U8 u8Enable)
{

	if(u8Enable){
		PPDRV_GPIO_SetDir(gCS_GPIOCh[s32Ch].port, gCS_GPIOCh[s32Ch].pin, eDIR_OUT, 1);
		set_pinmux(gCS_GPIOCh[s32Ch].pinmux_group, gCS_GPIOCh[s32Ch].pinmux_shift, gCS_GPIOCh[s32Ch].pinmux_sel);


	}else{
		clear_pinmux(gCS_GPIOCh[s32Ch].pinmux_group, gCS_GPIOCh[s32Ch].pinmux_shift);

	}

}

PP_VOID PPDRV_SPI_CSActivate(PP_S32 IN s32Ch)
{

	PPDRV_GPIO_SetValue(gCS_GPIOCh[s32Ch].port, gCS_GPIOCh[s32Ch].pin, 0);

}

PP_VOID PPDRV_SPI_CSDeActivate(PP_S32 IN s32Ch)
{

	PPDRV_GPIO_SetValue(gCS_GPIOCh[s32Ch].port, gCS_GPIOCh[s32Ch].pin, 1);

}

PP_S32 PPDRV_SPI_CSGetLevel(PP_S32 IN s32Ch)
{
	return PPDRV_GPIO_GetOutValue(gCS_GPIOCh[s32Ch].port, gCS_GPIOCh[s32Ch].pin);
}


PP_RESULT_E PPDRV_SPI_TxSlave(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size)
{
	PP_RESULT_E ret = eSUCCESS;

	sint32 len_tx;

	//gpSPI[s32Ch]->trans_ctrl = ( SPI_TCR_WONLY | (((u32Size-1)&0x1ff)<<12));
	gpSPI[s32Ch]->trans_ctrl = ( SPI_TCR_WONLY );//| (((u32Size-1)&0x1ff)<<12));

	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->irq_enable = 0;

	PPDRV_SPI_SetSlaveReady(s32Ch);

	for(len_tx=0; len_tx < u32Size;){
		if(len_tx < u32Size && !((gpSPI[s32Ch]->status>>23)&1) ){	// Tx not full
			gpSPI[s32Ch]->data = *pu8DOut;
			pu8DOut++;
			len_tx++;
		}
	}

	return ret;

}
PP_RESULT_E PPDRV_SPI_RxSlave(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size)
{
	PP_RESULT_E ret = eSUCCESS;
	sint32 len_rx;

	gpSPI[s32Ch]->trans_ctrl = SPI_TCR_RONLY;
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->irq_enable = 0;

	PPDRV_SPI_SetSlaveReady(s32Ch);
	for(len_rx=0; len_rx < u32Size;){
		if(!((gpSPI[s32Ch]->status>>14)&1)){	// Rx not empty
			*pu8DIn = gpSPI[s32Ch]->data;
			pu8DIn ++;
			len_rx++;
		}

	}

	//while(gpSPI[s32Ch]->status & 1);

	return ret;
}


PP_U8 PPDRV_SPI_RxSlave_Byte(PP_S32 ch)
{
    uint8 ret;

    if( (gpSPI[ch]->trans_ctrl&SPI_TCR_RONLY) != SPI_TCR_RONLY ){
    	gpSPI[ch]->trans_ctrl = SPI_TCR_RONLY;
    }

    while((gpSPI[ch]->status>>14)&1);   // rx empty

    ret = gpSPI[ch]->data;

    return ret;
}

PP_RESULT_E PPDRV_SPI_TxSlaveDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U32 IN u32Size, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout)
{
	sint32 ret = 0;
	PP_DMA_CONFIG_S cfg;

	if(gDMARequestTx[s32Ch] == (uint32)(-1))return -1;

	gTransferDMAFlag[s32Ch] = 1;

	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WONLY | (((u32Size-1)&0x1ff)<<12));
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_DMA_FIFO_THRD << 16) |	(RX_DMA_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold
	gpSPI[s32Ch]->ctrl |= (1<<4);	// DMA Tx Enable

	gpSPI[s32Ch]->irq_enable = SPI_INTR_TRANSFER_END;

	memset(&cfg.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg.ctrl.ctl.priority = 0;
	cfg.ctrl.ctl.src_burst_size = u32TxDMABurstSize[TX_DMA_FIFO_THRD];
	cfg.ctrl.ctl.src_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.dst_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.src_mode = 0;		// normal
	cfg.ctrl.ctl.dst_mode = 1;		// handshake
	cfg.ctrl.ctl.src_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.dst_addr_ctrl = 2;	// fixed
	cfg.ctrl.ctl.src_req_sel = 0;
	cfg.ctrl.ctl.dst_req_sel = gDMARequestTx[s32Ch];	// spi request ??????
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;
	cfg.u32SrcAddr = (uint32)(pu8DOut);
	cfg.u32DstAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg.u32TrasnferSize = (u32Size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;

	PPDRV_DMA_SetConfig(s32DMACh, &cfg);
	PPDRV_DMA_Start(s32DMACh, 0);
	PPDRV_SPI_SetSlaveReady(s32Ch);
	if(PPDRV_DMA_Wait(s32DMACh, u32Timeout)){
		goto ERROR;
	}

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

	return ret;
ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;


}

PP_RESULT_E PPDRV_SPI_RxSlaveDMA(PP_S32 IN s32Ch, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMACh, PP_U32 IN u32Timeout)
{
	sint32 ret = 0;

	PP_DMA_CONFIG_S cfg;

	if(gDMARequestRx[s32Ch] == (uint32)(-1))return -1;

	gTransferDMAFlag[s32Ch] = 1;
	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_RONLY);

	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_DMA_FIFO_THRD << 16) |	(RX_DMA_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold
	gpSPI[s32Ch]->ctrl |= (1<<3);	// DMA Rx Enable

	gpSPI[s32Ch]->irq_enable = SPI_INTR_TRANSFER_END;

	memset(&cfg.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg.ctrl.ctl.priority = 0;
	cfg.ctrl.ctl.src_burst_size = u32RxDMABurstSize[RX_DMA_FIFO_THRD];
	cfg.ctrl.ctl.src_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.dst_width = DMA_WIDTH_BYTE;
	cfg.ctrl.ctl.src_mode = 1;		// handshake
	cfg.ctrl.ctl.dst_mode = 0;		// normal
	cfg.ctrl.ctl.src_addr_ctrl = 2;	// fixed
	cfg.ctrl.ctl.dst_addr_ctrl = 0;	// increment
	cfg.ctrl.ctl.src_req_sel = gDMARequestRx[s32Ch];
	cfg.ctrl.ctl.dst_req_sel = 0;
	cfg.ctrl.ctl.int_abort_mask = 0;
	cfg.ctrl.ctl.int_err_mask = 0;
	cfg.ctrl.ctl.int_tcm_mask = 0;
	cfg.u32SrcAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg.u32DstAddr = (uint32)(pu8DIn);
	cfg.u32TrasnferSize = (u32Size >> cfg.ctrl.ctl.src_width);
	cfg.u32LLPPoint = 0;

	PPDRV_DMA_SetConfig(s32DMACh, &cfg);
	PPDRV_DMA_Start(s32DMACh, 0);
	PPDRV_SPI_SetSlaveReady(s32Ch);
	if(PPDRV_DMA_Wait(s32DMACh, u32Timeout)){
		goto ERROR;
	}

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

	return ret;

ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;

}

PP_RESULT_E PPDRV_SPI_TxRxSlaveDMA(PP_S32 IN s32Ch, PP_U8 IN *pu8DOut, PP_U8 OUT *pu8DIn, PP_U32 IN u32Size, PP_S32 IN s32DMATxCh, PP_S32 IN s32DMARxCh, PP_U32 IN u32Timeout)
{
	sint32 ret = 0;

	PP_DMA_CONFIG_S cfg_tx, cfg_rx;

	if(gDMARequestRx[s32Ch] == (uint32)(-1))return -1;
	gTransferDMAFlag[s32Ch] = 1;

	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WR | (((u32Size-1)&0x1ff)<<12) | (((u32Size-1)&0x1ff)<<0));
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_DMA_FIFO_THRD << 16) |	(RX_DMA_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold
	gpSPI[s32Ch]->ctrl |= ( (1<<4) | (1<<3) );	// DMA Tx/Rx Enable

	gpSPI[s32Ch]->irq_enable = SPI_INTR_TRANSFER_END;

	// dma tx config
	memset(&cfg_tx.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg_tx.ctrl.ctl.priority = 0;
	cfg_tx.ctrl.ctl.src_burst_size = u32TxDMABurstSize[TX_DMA_FIFO_THRD];
	cfg_tx.ctrl.ctl.src_width = DMA_WIDTH_BYTE;
	cfg_tx.ctrl.ctl.dst_width = DMA_WIDTH_BYTE;
	cfg_tx.ctrl.ctl.src_mode = 0;		// normal
	cfg_tx.ctrl.ctl.dst_mode = 1;		// handshake
	cfg_tx.ctrl.ctl.src_addr_ctrl = 0;	// increment
	cfg_tx.ctrl.ctl.dst_addr_ctrl = 2;	// fixed
	cfg_tx.ctrl.ctl.src_req_sel = 0;
	cfg_tx.ctrl.ctl.dst_req_sel = gDMARequestTx[s32Ch];
	cfg_tx.ctrl.ctl.int_abort_mask = 0;
	cfg_tx.ctrl.ctl.int_err_mask = 0;
	cfg_tx.ctrl.ctl.int_tcm_mask = 0;
	cfg_tx.u32SrcAddr = (uint32)(pu8DOut);
	cfg_tx.u32DstAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg_tx.u32TrasnferSize = (u32Size >> cfg_tx.ctrl.ctl.src_width);
	cfg_tx.u32LLPPoint = 0;


	memset(&cfg_rx.ctrl, 0, sizeof(PP_DMA_CTL_U));
	cfg_rx.ctrl.ctl.priority = 0;
	cfg_rx.ctrl.ctl.src_burst_size = u32RxDMABurstSize[RX_DMA_FIFO_THRD];
	cfg_rx.ctrl.ctl.src_width = DMA_WIDTH_BYTE;
	cfg_rx.ctrl.ctl.dst_width = DMA_WIDTH_BYTE;
	cfg_rx.ctrl.ctl.src_mode = 1;		// handshake
	cfg_rx.ctrl.ctl.dst_mode = 0;		// normal
	cfg_rx.ctrl.ctl.src_addr_ctrl = 2;	// fixed
	cfg_rx.ctrl.ctl.dst_addr_ctrl = 0;	// increment
	cfg_rx.ctrl.ctl.src_req_sel = gDMARequestRx[s32Ch];
	cfg_rx.ctrl.ctl.dst_req_sel = 0;
	cfg_rx.ctrl.ctl.int_abort_mask = 0;
	cfg_rx.ctrl.ctl.int_err_mask = 0;
	cfg_rx.ctrl.ctl.int_tcm_mask = 0;
	cfg_rx.u32SrcAddr = (uint32)(&gpSPI[s32Ch]->data);
	cfg_rx.u32DstAddr = (uint32)(pu8DIn);
	cfg_rx.u32TrasnferSize = (u32Size >> cfg_rx.ctrl.ctl.src_width);
	cfg_rx.u32LLPPoint = 0;

	PPDRV_DMA_SetConfig(s32DMATxCh, &cfg_tx);
	PPDRV_DMA_SetConfig(s32DMARxCh, &cfg_rx);


	PPDRV_DMA_Start(s32DMATxCh, 0);
	PPDRV_DMA_Start(s32DMARxCh, 0);

	PPDRV_SPI_SetSlaveReady(s32Ch);

	if(PPDRV_DMA_Wait(s32DMATxCh, u32Timeout)){
		PPDRV_DMA_Wait(s32DMARxCh, u32Timeout);
		goto ERROR;
	}
	if(PPDRV_DMA_Wait(s32DMARxCh, u32Timeout)){
		goto ERROR;
	}

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghDMALock[s32Ch], u32Timeout) == pdFALSE )
		{
			goto ERROR;
		}
		gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
		gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));	// DMA disable
    }

	return ret;
ERROR:
	LOG_CRITICAL("Timeout! DMA.\n");
	gTransferDMAFlag[s32Ch] = 0;
	gpSPI[s32Ch]->irq_enable &= (~SPI_INTR_TRANSFER_END);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	// tx fifo reset
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<1)));					// Wait until fifo clear

	return eERROR_FAILURE;

}

PP_RESULT_E PPDRV_SPI_SetTxSlaveIntr(PP_S32 IN s32Ch)
{
	sint32 ret = 0;

	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_WONLY);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_INTR_FIFO_THRD << 16) |	(RX_INTR_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold

	//gpSPI[s32Ch]->irq_enable |= (SPI_INTR_TXFIFO_THRD);
	gpSPI[s32Ch]->irq_enable = (SPI_INTR_TXFIFO_THRD);
	PPDRV_SPI_SetSlaveReady(s32Ch);

	return ret;
}

PP_RESULT_E PPDRV_SPI_SetRxSlaveIntr(PP_S32 IN s32Ch)
{
	sint32 ret = 0;

	gpSPI[s32Ch]->trans_ctrl = (SPI_TCR_RONLY);
	gpSPI[s32Ch]->ctrl &= (~((1<<4) | (1<<3)));						// Tx/Rx DMA disable
	gpSPI[s32Ch]->ctrl |= ((1<<2) | (1<<1));						// Tx/Rx fifo clear
	while(gpSPI[s32Ch]->ctrl & ((1<<2) | (1<<2)));					// Wait until fifo clear
	gpSPI[s32Ch]->ctrl &= (~((0x1f<<16) | (0x1f<<8)));
	gpSPI[s32Ch]->ctrl |= ( (TX_INTR_FIFO_THRD << 16) |	(RX_INTR_FIFO_THRD << 8) );	// Tx/Rx FIFO Threshold

	//gpSPI[s32Ch]->irq_enable |= (SPI_INTR_TRANSFER_END | SPI_INTR_RXFIFO_THRD);
	gpSPI[s32Ch]->irq_enable = (SPI_INTR_TRANSFER_END | SPI_INTR_RXFIFO_THRD | SPI_INTR_RECV_CMD);
	PPDRV_SPI_SetSlaveReady(s32Ch);

	return ret;
}

PP_VOID PPDRV_SPI_SetSlaveReady(PP_S32 IN s32Ch)
{
	gpSPI[s32Ch]->slave_status |= (1<<16);
}

PP_VOID PPDRV_SPI_SetSlaveStatus(PP_S32 IN s32Ch, PP_U16 IN u16Status)
{
	gpSPI[s32Ch]->slave_status &= (~0xffff);
	gpSPI[s32Ch]->slave_status |= (u16Status&0xffff);
}

PP_U16 PPDRV_SPI_GetSlaveStatus(PP_S32 IN s32Ch)
{
	return (gpSPI[s32Ch]->slave_status & 0xffff);
}


PP_VOID PPDRV_SPI_IRQEnable(PP_S32 s32Ch, PP_U32 u32Irq)
{
	gpSPI[s32Ch]->irq_enable |= u32Irq;
}

PP_VOID PPDRV_SPI_IRQClear(PP_S32 s32Ch, PP_U32 u32Irq)
{
	gpSPI[s32Ch]->irq_enable &= (~u32Irq);
}


PP_U32 PPDRV_SPI_GetFreq(PP_S32 s32Ch)
{
	uint32 div;

	div = utilGetBits( gpSPI[s32Ch]->timing, 0, 8 );

	return (u32APBClk/((div+1)*2));
}

PP_VOID PPDRV_SPI_Wait(PP_S32 s32Ch)
{
    while(gpSPI[s32Ch]->status & 1);
}

PP_VOID PPDRV_SPI_BufferClear(PP_S32 s32Ch)
{
	int i = 0;

	int fifo_num = ((gpSPI[s32Ch]->status >> 8)&0x1f);

	for( i = 0; i < fifo_num; i ++ )
	{
		spi_isr_buf[i] = PPDRV_SPI_RxSlave_Byte(s32Ch);
	}

	memset(spi_isr_buf, 0, sizeof(spi_isr_buf));
	rx_index = 0;
}
