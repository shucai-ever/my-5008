
#include <stdio.h>

#include "osal.h"
#include "i2s.h"
#include "type.h"
#include "utils.h"
#include "debug.h"
#include "pi5008.h"
#include "interrupt.h"
#include "system.h"
#include "clock.h"
#include "i2c.h"
#include "dma.h"

#define MAX_I2C_BUF		4

typedef struct tagI2C_CTX_T{
	uint8 isMaster;
	uint8 isTx;
	uint8 buf[MAX_I2C_BUF];
	uint8 rsv;
	uint32 buf_size;
	uint32 cnt;
}I2C_CTX_T;

typedef struct tagI2C_REG_T
{
	vuint32 id;								/* 0x000 		- ID and revision register */
	vuint32 rsv0[3];						/* 0x004 ~ 0x00c	- reserved */
	vuint32 cfg;							/* 0x010			- Configuration register */
	vuint32 irq_enable;						/* 0x014			- Interrupt enable register */
	vuint32 status;							/* 0x018			- Status register */
	vuint32 addr;							/* 0x01c			- Address register */
	vuint32 data;							/* 0x020			- Data register */
	vuint32 ctrl;							/* 0x024			- Control register */
	vuint32 cmd;							/* 0x028			- Control register */
	vuint32 setup;							/* 0x02c			- Control register */

}I2CM_REG_T;

#define I2CM0_CTRL_REG			((I2CM_REG_T*)			(I2CM0_BASE_ADDR))
#define I2CM1_CTRL_REG			((I2CM_REG_T*)			(I2CM1_BASE_ADDR))


/*-----------------------------------------------
	I2CS Register
-----------------------------------------------*/
#define rI2CS_I2C_MODE			(I2CS_BASE_ADDR + 0x40)
#define rI2CS_SLV_ADDR			(I2CS_BASE_ADDR + 0x44)
#define rI2CS_FIFO_FLUSH		(I2CS_BASE_ADDR + 0x48)
#define rI2CS_INT_CLR			(I2CS_BASE_ADDR + 0x4c)
#define rI2CS_AHB_MASTER		(I2CS_BASE_ADDR + 0x50)
#define rI2CS_AHB_RDATA			(I2CS_BASE_ADDR + 0x54)
#define rI2CS_AHB_SLAVE_EN		(I2CS_BASE_ADDR + 0x58)
#define rI2CS_SLV_WDATA			(I2CS_BASE_ADDR + 0x60)
#define rI2CS_SLV_RDATA			(I2CS_BASE_ADDR + 0x60)

#define rI2CS_DBG_STATE			(I2CS_BASE_ADDR + 0x80)
#define rI2CS_DBG_ADDR			(I2CS_BASE_ADDR + 0x84)
#define rI2CS_DBG_RDATA			(I2CS_BASE_ADDR + 0x88)
#define rI2CS_DBG_WDATA			(I2CS_BASE_ADDR + 0x8c)

//-------------------------------------------------------------
#define I2CS_SLAVE_ADDR		(0x1A)
#define I2CS_FIFO_FLUSH		(0x1)


/**********************************
- for picasso, slave function
**********************************/
void i2cs_init(void)
{
	// i2c slave mode set, slave id = 0x34
	SetRegValue(rI2CS_FIFO_FLUSH, I2CS_FIFO_FLUSH);	// fifo flush
	SetRegValue(rI2CS_I2C_MODE, 0x46);				// fifo full intr en, fifo threshold 3, slave
	SetRegValue(rI2CS_SLV_ADDR, (unsigned char)(I2CS_SLAVE_ADDR<<1));
	// i2c slave BUS Master set
	SetRegValue(rI2CS_AHB_MASTER, 0x1);				// rd endian = 00, wr endian = 00, msb first = 0, addr_pos = 00, ahb master enable = 1)
}

/**********************************
- i2cm0/i2cm1, master function
**********************************/
static I2CM_REG_T *gpI2CM[] = {
	I2CM0_CTRL_REG,
	I2CM1_CTRL_REG,
//	I2CM2_CTRL_REG,
//	I2CM3_CTRL_REG
};



static I2C_ISR_CALLBACK gI2CM_ISR[MAX_I2C_DEV_NUM];
static I2C_CTX_T gI2C_CTX[MAX_I2C_DEV_NUM];
static SemaphoreHandle_t ghLock[MAX_I2C_DEV_NUM];
static PP_RESULT_E genResult[MAX_I2C_DEV_NUM] = {eSUCCESS, eSUCCESS, };
static uint32 gTransComplete[MAX_I2C_DEV_NUM] = {1, 1,};

static I2C_SLAVE_READ OnReceive[MAX_I2C_DEV_NUM] = {NULL, NULL,};
static I2C_SLAVE_WRITE OnSend[MAX_I2C_DEV_NUM] = {NULL, NULL, };

static void rx_data(uint32 ch)
{

	if(gI2C_CTX[ch].buf){
		while((gpI2CM[ch]->status & STAT_FIFO_EMPTY) == 0){
			if(gI2C_CTX[ch].cnt >= gI2C_CTX[ch].buf_size)break;
			gI2C_CTX[ch].buf[gI2C_CTX[ch].cnt++] = gpI2CM[ch]->data;
		}
	}
}

static void tx_data(uint32 ch)
{
	uint32 i = 0;

	while(gI2C_CTX[ch].cnt > 0){

		if((gpI2CM[ch]->status & STAT_FIFO_FULL) == 0){
			gpI2CM[ch]->data = gI2C_CTX[ch].buf[i++];
			gI2C_CTX[ch].cnt--;
		}
	}
}

ISR(i2cm_isr, num)
{
	uint32 msk = (1 << num);
	uint32 reg;
	uint32 dev;
	uint32 size;
	BaseType_t xHigherPriorityTaskWoken;

	/* Mask and clear HW interrupt vector */
	__nds32__mtsr_dsb(__nds32__mfsr(NDS32_SR_INT_MASK2) & ~msk, NDS32_SR_INT_MASK2);
	__nds32__mtsr(msk, NDS32_SR_INT_PEND2);

	for(dev=0;dev<MAX_I2C_DEV_NUM;dev++){
		if(IRQ_I2C_STATUS_MASK(dev) == FLAG_SET)break;
	}


	reg = gpI2CM[dev]->status;
	gpI2CM[dev]->status = reg;

	//printf("i2c isr. dev: %d, status: 0x%x\n", dev, reg);

	if(gI2C_CTX[dev].isMaster){		// master mode
		if(reg & STAT_ADDR_HIT){
			//printf("[I2C ISR, dev: 0x%x]addr hit(0x%x)\n", dev, reg);
		}

		if(reg & STAT_BYTE_RCV){
			if(gpI2CM[dev]->irq_enable & INT_BYTE_RECV){	// ACK/NACK manually
				if((gpI2CM[dev]->ctrl & 0xff) == 0){	// received last byte -> NACK
					gpI2CM[dev]->cmd = CMD_RSP_NACK;
				}else{
					gpI2CM[dev]->cmd = CMD_RSP_ACK;
				}
			}
		}
		if(reg & STAT_CMPL){
			//printf("[I2C ISR, dev: 0x%x]Complete(0x%x)\n", dev, reg);
			genResult[dev] = eSUCCESS;
			gTransComplete[dev] = 1;

            xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(ghLock[dev], &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR();
            }

		}

		if(reg & INT_ARB_LOSE){
			//printf("[I2C ISR, dev: 0x%x]Arbitration Lose(0x%x)\n", dev, reg);
			genResult[dev] = eERROR_FAILURE;
			gTransComplete[dev] = 1;

			xHigherPriorityTaskWoken = pdFALSE;
            xSemaphoreGiveFromISR(ghLock[dev], &xHigherPriorityTaskWoken);
            if(xHigherPriorityTaskWoken == pdTRUE)
            {
                portYIELD_FROM_ISR();
            }

		}
	}else{							// slave mode

		if(reg & STAT_ADDR_HIT){
			//printf("[I2C ISR, dev: 0x%x]addr hit(0x%x)\n", dev, reg);
			if((gpI2CM[dev]->ctrl & PHASE_DIR_SLV_TRS) == PHASE_DIR_SLV_TRS){
				gI2C_CTX[dev].isTx = 1;
				gpI2CM[dev]->irq_enable |= INT_FIFO_EMPTY;
			}else{
				gI2C_CTX[dev].isTx = 0;
				gpI2CM[dev]->irq_enable |= INT_FIFO_HALF;
			}
		}
		if(reg & STAT_FIFO_FULL){
#if 0
			//printf("[I2C ISR, dev: 0x%x]FIFO FULL (0x%x)\n", dev, reg);
			if(gI2C_CTX[dev].isTx == 0){	// receive data
				rx_data(dev);
				if(OnReceive[dev])OnReceive[dev](dev, gI2C_CTX[dev].buf, gI2C_CTX[dev].cnt);
				gI2C_CTX[dev].cnt = 0;
			}
#endif
		}
		if(reg & STAT_BYTE_RCV){
			if(gpI2CM[dev]->irq_enable & INT_BYTE_RECV){	// ACK/NACK manually
				if(1){
					gpI2CM[dev]->cmd = CMD_RSP_ACK;
				}else{	// if error condition occur, send NACK
					gpI2CM[dev]->cmd = CMD_RSP_NACK;
				}
			}
		}

		if(reg & STAT_FIFO_HALF){
			//printf("[I2C ISR, dev: 0x%x]FIFO Half (0x%x)\n", dev, reg);
			if(gpI2CM[dev]->irq_enable & INT_FIFO_HALF){
				if(gI2C_CTX[dev].isTx == 0){	// receive data
					rx_data(dev);
					if(OnReceive[dev])OnReceive[dev](dev, gI2C_CTX[dev].buf, gI2C_CTX[dev].cnt);
					gI2C_CTX[dev].cnt = 0;
				}
			}

		}

		if(reg & STAT_FIFO_EMPTY){
			printf("[I2C ISR, dev: 0x%x]FIFO Empty(0x%x)\n", dev, reg);

			if( gpI2CM[dev]->irq_enable & INT_FIFO_EMPTY ){
				if((gI2C_CTX[dev].isTx == 1) && !(reg & STAT_STOP) ){	// if tx mode && xfer is not end, then send data
					if(OnSend[dev])size = OnSend[dev](dev, gI2C_CTX[dev].buf, gI2C_CTX[dev].buf_size - gI2C_CTX[dev].cnt);
					gI2C_CTX[dev].cnt += size;
					//printf("[I2C ISR, dev: 0x%x]send cnt: %d\n", dev, gI2C_CTX[dev].cnt);
					tx_data(dev);
					//printf("[I2C ISR, dev: 0x%x]left cnt: %d\n", dev, gI2C_CTX[dev].cnt);
				}
			}
		}

		if(reg & STAT_CMPL){
			//printf("[I2C ISR, dev: 0x%x]Complete(0x%x)\n", dev, reg);
			if(gI2C_CTX[dev].isTx == 0){
				rx_data(dev);
				if(OnReceive[dev] && gI2C_CTX[dev].cnt)OnReceive[dev](dev, gI2C_CTX[dev].buf, gI2C_CTX[dev].cnt);
				gI2C_CTX[dev].cnt = 0;

			}
			// irq clear
			gTransComplete[dev] = 1;
			gpI2CM[dev]->irq_enable &= (~(INT_FIFO_HALF | INT_FIFO_FULL | INT_FIFO_EMPTY));

			gpI2CM[dev]->cmd = CMD_CLEAR;

		}

	}

	if(gI2CM_ISR[dev])gI2CM_ISR[dev](dev, reg);

	__nds32__mtsr(__nds32__mfsr(NDS32_SR_INT_MASK2) | msk, NDS32_SR_INT_MASK2);

}

PP_VOID PPDRV_I2C_Initialize(PP_VOID)
{
	uint32 i;

    OSAL_register_isr(IRQ_I2CM_VECTOR, i2cm_isr, NULL);
    INTC_irq_config(IRQ_I2CM_VECTOR, IRQ_LEVEL_TRIGGER);
    INTC_irq_clean(IRQ_I2CM_VECTOR);
    INTC_irq_enable(IRQ_I2CM_VECTOR);

    for(i=0;i<MAX_I2C_DEV_NUM;i++){
    	gI2CM_ISR[i] = NULL;
        if( (ghLock[i] = xSemaphoreCreateBinary()) == NULL)
        {
            LOG_DEBUG("ERROR! can't create lock\n");
        }

    }
}


PP_VOID PPDRV_I2C_Setup(PP_S32 IN s32Ch, PP_U32 IN u32DevAddr, PP_I2C_SPEED_E IN enSpeed, PP_I2C_MODE_E IN enMode, PP_I2C_ADDRBIT_E IN enAddrBit)
{
	uint32 t_pclk;	// pclk time ns
	sint32 t_sp;	// pulse width of spikes
	sint32 t_sudat;	// data setup time
	sint32 t_hddat; // data hold time
	sint32 t_high;	// scl high period
	sint32 t_lowratio;	// scl low period ratio
	uint32 i2c_mode;
	uint32 i2c_addrbit;

	uint32 sp_time[3] = { 50, 50, 50, };
	uint32 sudat_time[3] = {250, 100, 50,};
	uint32 hddat_time[3] = {300, 300, 0,};
	uint32 high_time[3] = {5000, 1250, 500,};
	uint32 low_time[3] = {5000, 1250, 500,};


	t_pclk = ( 1000000000 / u32APBClk);
	t_sp = sp_time[enSpeed] / t_pclk;
	t_sudat = sudat_time[enSpeed]/t_pclk - t_sp - 4;
	if(t_sudat < 0)t_sudat = 0;
	t_hddat = hddat_time[enSpeed]/t_pclk - t_sp - 4;
	if(t_hddat < 0)t_hddat = 0;
	t_high = high_time[enSpeed]/t_pclk - t_sp - 4;
	if(t_high < 0)t_high = 0;
	t_lowratio = low_time[enSpeed]/high_time[enSpeed] - 1 ;
	if(t_lowratio > 1)t_lowratio = 1;

	//LOG_NOTICE("t_pclk: %d, t_sp: %d, t_sudat: %d, t_hddat: %d, t_high: %d, t_lowratio: %d\n", t_pclk, t_sp, t_sudat, t_hddat, t_high, t_lowratio);

	i2c_mode = (enMode == eI2C_MODE_MASTER) ? MASTER_MODE : SLAVE_MODE;
	i2c_addrbit = (enAddrBit == eI2C_ADDRESS_7BIT) ? ADDRESSING_7BIT : ADDRESSING_10BIT;

	gpI2CM[s32Ch]->cmd = CMD_RESET;
	gpI2CM[s32Ch]->setup = (( (t_sudat<<24) | (t_sp<<21) | (t_hddat<<16) | (t_lowratio<<13) | (t_high<<4) | DMA_DISABLE | i2c_mode | i2c_addrbit) | (gpI2CM[s32Ch]->setup & 1) );
	gpI2CM[s32Ch]->addr = u32DevAddr;

	memset(&gI2C_CTX[s32Ch], 0, sizeof(gI2C_CTX[s32Ch]));
	gI2C_CTX[s32Ch].isMaster = (enMode == eI2C_MODE_MASTER) ? 1 : 0;
	gI2C_CTX[s32Ch].buf_size = MAX_I2C_BUF;

	gTransComplete[s32Ch] = 1;
	genResult[s32Ch] = eSUCCESS;
	gpI2CM[s32Ch]->irq_enable |= (INT_CMPL | INT_ADDR_HIT | INT_ARB_LOSE | INT_STOP);

}

PP_VOID PPDRV_I2C_Enable(PP_S32 IN s32Ch, PP_U32 IN u32Enable)
{
	if(u32Enable){
		gpI2CM[s32Ch]->irq_enable |= (INT_CMPL | INT_ADDR_HIT);
		gpI2CM[s32Ch]->setup |= I2C_ENABLE;
	}else{
		gpI2CM[s32Ch]->setup &= (~I2C_ENABLE);
	}

}

PP_RESULT_E PPDRV_I2C_Write(PP_S32 s32Ch, PP_U8 IN *pu8Data, PP_U8 IN u8DataSize, PP_U32 IN u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;

	if(gpI2CM[s32Ch]->status & (STAT_BUSY | STAT_ARB_LOSE))return 1;

	gTransComplete[s32Ch] = 1;
	genResult[s32Ch] = eSUCCESS;
	//gpI2CM[ch]->cmd = CMD_RESET;
	gpI2CM[s32Ch]->cmd = CMD_CLEAR;
	gpI2CM[s32Ch]->ctrl = ( PHASE_START | PHASE_ADDR | PHASE_DATA | PHASE_STOP | PHASE_DIR_MST_TRS | u8DataSize );
	//gpI2CM[s32Ch]->irq_enable |= (INT_CMPL | INT_ADDR_HIT | INT_ARB_LOSE);

	while(u8DataSize > 0){
		if( (gpI2CM[s32Ch]->status & STAT_FIFO_FULL) == 0){
			gpI2CM[s32Ch]->data = *(pu8Data++);
			u8DataSize--;
		}
		if(u8DataSize == 0)break;
	}
	gpI2CM[s32Ch]->cmd = CMD_TRANSACTION;

    if(u32Timeout == SYS_OS_SUSPEND)
    {
        u32Timeout = portMAX_DELAY;
    }
    else
    {
        u32Timeout = OSAL_ms2ticks(u32Timeout);
    }

    if(u32Timeout){
		if( xSemaphoreTake(ghLock[s32Ch], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			ret = eERROR_TIMEOUT;
		}else{
			ret = genResult[s32Ch];
		}
    }

	return ret;
}


PP_RESULT_E PPDRV_I2C_Read(PP_S32 s32Ch, PP_U8 OUT *pu8Data, PP_U8 IN u8DataSize, PP_U32 IN u32Timeout)
{
	PP_RESULT_E ret = eSUCCESS;

	if(gpI2CM[s32Ch]->status & (STAT_BUSY | STAT_ARB_LOSE))return 1;

	gTransComplete[s32Ch] = 1;
	genResult[s32Ch] = eSUCCESS;
	gpI2CM[s32Ch]->cmd = CMD_CLEAR;
	gpI2CM[s32Ch]->ctrl = ( PHASE_START | PHASE_ADDR | PHASE_DATA | PHASE_STOP | PHASE_DIR_MST_RCV | u8DataSize );
	//gpI2CM[s32Ch]->irq_enable |= (INT_CMPL | INT_ADDR_HIT | INT_ARB_LOSE);

	gpI2CM[s32Ch]->cmd = CMD_TRANSACTION;

	while(u8DataSize > 0){
		if( (gpI2CM[s32Ch]->status & STAT_FIFO_EMPTY) == 0){
			*(pu8Data++) = gpI2CM[s32Ch]->data;
			u8DataSize--;
		}
		if(u8DataSize == 0)break;
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
		if( xSemaphoreTake(ghLock[s32Ch], u32Timeout) == pdFALSE )
		{
			LOG_CRITICAL("Timeout!\n");
			ret = eERROR_TIMEOUT;
		}else{
			ret = genResult[s32Ch];
		}
    }

	return ret;
}

PP_U32 PPDRV_I2C_GetDone(PP_S32 IN s32Ch, PP_RESULT_E OUT *penResult)
{
	PP_U32 u32Ret;

	u32Ret = gTransComplete[s32Ch];
	if(penResult)*penResult = genResult[s32Ch];

	return u32Ret;
}

PP_VOID PPDRV_I2C_SlaveSetCallback(PP_S32 IN s32Ch, I2C_SLAVE_READ IN cbRecv, I2C_SLAVE_WRITE IN cbSend)
{
	OnReceive[s32Ch] = cbRecv;
	OnSend[s32Ch] = cbSend;
}

PP_VOID PPDRV_I2C_Reset(PP_S32 IN s32Ch)
{
	gpI2CM[s32Ch]->cmd = CMD_RESET;
}
